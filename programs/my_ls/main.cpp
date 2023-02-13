#define GB_IMPLEMENTATION
#include "../../lib/gb.h"
#include <string.h>
#include <dirent.h>

#define NORMAL_COLOR "\x1B[0m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define RED "\x1B[31m"
#define BOLD "\x1B[1m"
#define RED_BACKGROUND "\x1B[41m"
#define DEFAULT_BACKGROUND "\x1B[49m"
typedef struct stat Stat;
typedef enum { SHOW_DOT_FILES, SHOW_FILE_SIZE, SHOW_LAST_MOD_DATE, FLAGS_COUNT } FLAGS;

void print_file(dirent* file, Stat* sb);
int sort_files_and_dirs(const void* a, const void* b);

gb_global gbBitset flags[GB_FLAGS_ARRAY_SIZE(FLAGS_COUNT)] = {};

int main(int argc, char** argv)
{
    DIR* d = NULL;
    gbArray(dirent) files = NULL;
    gb_array_init(files, gb_heap_allocator());
    defer(gb_array_free(files));

    int opt;
    while ((opt = getopt(argc, argv, "als")) != -1) {
        switch (opt) {
        case 'a':
            GB_FLAGS_SET(flags, SHOW_DOT_FILES);
            break;
        case 's':
            GB_FLAGS_SET(flags, SHOW_FILE_SIZE);
            break;
        case 'l':
            GB_FLAGS_SET(flags, SHOW_LAST_MOD_DATE);
            GB_FLAGS_SET(flags, SHOW_FILE_SIZE);
            break;
        default:
            gb_printf_err("Usage: %s [-asl] [directory...]\n", argv[0]);
            gb_exit(1);
        }
    }
    int* p_optind = &optind;
    if (*p_optind >= argc) {
        d = opendir(".");
    } else {
        d = opendir(argv[*p_optind]);
    }
    if (!d) {
        gb_printf_err("%s%s\n", NORMAL_COLOR, strerror(errno));
        gb_exit(1);
    }
    defer(closedir(d));

    dirent* current_d = NULL;
    while ((current_d = readdir(d))) {
        gb_array_append(files, *current_d);
    }
    gb_sort_array(files, gb_array_count(files), sort_files_and_dirs);

    defer(printf("%s", NORMAL_COLOR));
    for (int i = 0; i < gb_array_count(files); i++) {
        if ((files[i].d_name[0] == '.' && !GB_FLAGS_TEST(flags, SHOW_DOT_FILES)))
            continue;

        gb_local_persist gbString full_path = gb_string_make_reserve(gb_heap_allocator(), 256);
        defer(gb_string_clear(full_path));
        gb_string_appendc(full_path, argv[*p_optind]);

        // make sure that the last character in full_path is "/"
        auto length = gb_string_length(full_path);
        if (full_path[length - 1] != '/' && *p_optind < argc) {
            gb_string_appendc(full_path, "/");
        }
        gb_string_appendc(full_path, files[i].d_name);

        Stat sb = {};
        auto error = stat(full_path, &sb);
        if (error != 0) {
            const char* no_time_str = "         null ";
            gb_local_persist auto zero_bytes = gb_string_make_reserve(gb_heap_allocator(), 100);
            if (GB_FLAGS_TEST(flags, SHOW_FILE_SIZE))
                zero_bytes = gb_string_append_fmt(zero_bytes, "%5d  B", 0);

            if (GB_FLAGS_TEST(flags, SHOW_LAST_MOD_DATE))
                gb_printf("%s%s%s%s%s", zero_bytes, no_time_str, BOLD, RED_BACKGROUND, files[i].d_name);
            else
                gb_printf("%s%s%s%s", zero_bytes, BOLD, RED_BACKGROUND, files[i].d_name);

            gb_printf("%s%s\n", DEFAULT_BACKGROUND, NORMAL_COLOR);
            continue;
        }

        print_file(&files[i], &sb);
    }
    return 0;
}

void print_file(dirent* file, Stat* sb)
{
    gb_local_persist char* buffer = (char*)gb_malloc(gb_kilobytes(3));
    gbArena arena = {};
    gb_arena_init_from_memory(&arena, buffer, gb_kilobytes(3));
    auto allocator = gb_arena_allocator(&arena);
    auto temp = gb_temp_arena_memory_begin(&arena);
    defer(gb_temp_arena_memory_end(temp));

    auto color_str = gb_string_make_reserve(allocator, 20);
    auto unit_str = gb_string_make_reserve(allocator, 5);
    auto file_size_str = gb_string_make_reserve(allocator, 256);
    auto last_mod_full_str = gb_string_make_reserve(allocator, 256);

    if (file->d_type == DT_DIR) {
        color_str = gb_string_appendc(color_str, BLUE);
        color_str = gb_string_appendc(color_str, BOLD);
    } else if (sb->st_mode & S_IEXEC) {
        color_str = gb_string_appendc(color_str, GREEN);
        color_str = gb_string_appendc(color_str, BOLD);
    } else {
        color_str = gb_string_appendc(color_str, NORMAL_COLOR);
    }
    if (GB_FLAGS_TEST(flags, SHOW_FILE_SIZE)) {
        f32 file_size = sb->st_size;
        unit_str = gb_string_appendc(unit_str, " B");
        defer(gb_string_clear(unit_str));

        if (file_size > gb_kilobytes(1) && file_size < gb_megabytes(1)) {
            gb_string_clear(unit_str);
            unit_str = gb_string_appendc(unit_str, "KB");
            file_size /= gb_kilobytes(1);
        } else if (file_size > gb_megabytes(1) && file_size < gb_gigabytes(1)) {
            gb_string_clear(unit_str);
            unit_str = gb_string_appendc(unit_str, "MB");
            file_size /= gb_megabytes(1);
        } else if (file_size > gb_gigabytes(1) && file_size < gb_terabytes(1)) {
            gb_string_clear(unit_str);
            unit_str = gb_string_appendc(unit_str, "GB");
            file_size /= gb_gigabytes(1);
        } else if (file_size > gb_terabytes(1)) {
            gb_string_clear(unit_str);
            unit_str = gb_string_appendc(unit_str, "TB");
            file_size /= gb_terabytes(1);
        }
        file_size_str = gb_string_append_fmt(file_size_str, "%s%5.1f %s ", NORMAL_COLOR, file_size, unit_str);
    }

    gbString last_mod_str = NULL;

    if (GB_FLAGS_TEST(flags, SHOW_LAST_MOD_DATE)) {
        last_mod_full_str = gb_string_append_fmt(last_mod_full_str, "%s", ctime(&sb->st_mtime));
        last_mod_full_str = gb_string_trim(last_mod_full_str, "\n");
        auto length = gb_string_length(last_mod_full_str);
        last_mod_str = gb_string_make_reserve(allocator, 50);
        last_mod_str = gb_string_append_length(last_mod_str, last_mod_full_str + 4, length - 12);
        last_mod_str = gb_string_appendc(last_mod_str, " ");
    } else {
        last_mod_str = gb_string_make(allocator, "");
    }

    gb_printf("%s%s%s%s\n", file_size_str, last_mod_str, color_str, file->d_name);
}

int sort_files_and_dirs(const void* a, const void* b)
{
    dirent* a_file = (dirent*)a;
    dirent* b_file = (dirent*)b;
    i32 a_dot_shift = 0;
    i32 b_dot_shift = 0;

    if (a_file->d_name[0] == '.')
        a_dot_shift++;
    if (b_file->d_name[0] == '.')
        b_dot_shift++;

    if (a_file->d_type == DT_DIR && b_file->d_type == DT_DIR) {
        return gb_strcmp(a_file->d_name + a_dot_shift, b_file->d_name + b_dot_shift);
    }

    if (a_file->d_type != DT_DIR && b_file->d_type != DT_DIR) {
        return gb_strcmp(a_file->d_name + a_dot_shift, b_file->d_name + b_dot_shift);
    }

    if (a_file->d_type == DT_DIR) {
        return -1;
    } else {
        return +1;
    }

    GB_PANIC("Shouldn't be here\n");
    return 0;
}
