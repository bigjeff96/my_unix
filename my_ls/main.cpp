#include <errno.h>
#include <string.h>
#define GB_IMPLEMENTATION
#include "../lib/bitset.h"
#include "../lib/gb.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#define NORMAL_COLOR "\x1B[0m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define RED "\x1B[31m"
#define BOLD "\x1B[1m"
typedef struct stat Stat;
typedef enum { SHOW_DOT_FILES, SHOW_FILE_SIZE, FLAGS_COUNT } FLAGS;

void print_file(dirent* file, Stat* sb);
int sort_files_and_dirs(const void* a, const void* b);

gb_global b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};

int main(int argc, char** argv)
{
    DIR* d = NULL;
    gbArray(dirent) files = NULL;
    gb_array_init(files, gb_heap_allocator());
    defer(gb_array_free(files));

    int opt;
    while ((opt = getopt(argc, argv, "as")) != -1) {
        switch (opt) {
        case 'a':
            BITSET(flags, SHOW_DOT_FILES);
            break;
        case 's':
            BITSET(flags, SHOW_FILE_SIZE);
            break;
        default:
            gb_printf_err("Usage: %s [-as] [directory...]\n", argv[0]);
            gb_exit(1);
        }
    }
    int* p_optind = &optind;
    // TODO: with multiple directories, do a listing of each directory one after the other
    if (*p_optind >= argc) {
        d = opendir(".");
    } else {
        d = opendir(argv[*p_optind]);
    }
    if (!d) {
        fprintf(stderr, "%s%s\n", NORMAL_COLOR, strerror(errno));
        gb_exit(1);
    }
    defer(closedir(d));

    dirent* current_d = NULL;
    while (current_d = readdir(d)) {
        gb_array_append(files, *current_d);
    }
    gb_sort_array(files, gb_array_count(files), sort_files_and_dirs);

    defer(printf("%s", NORMAL_COLOR));
    for (int i = 0; i < gb_array_count(files); i++) {
        if ((files[i].d_name[0] == '.' && !BITTEST(flags, SHOW_DOT_FILES)))
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
            fprintf(stderr, "%s%s: %s\n", NORMAL_COLOR, strerror(errno), full_path);
            gb_exit(1);
        }

        print_file(&files[i], &sb);
    }
    return 0;
}

void print_file(dirent* file, Stat* sb)
{
    gb_local_persist gbString color_str = gb_string_make_reserve(gb_heap_allocator(), 256);
    gb_local_persist gbString file_size_str = gb_string_make_reserve(gb_heap_allocator(), 256);
    defer(gb_string_clear(color_str));
    defer(gb_string_clear(file_size_str));

    if (file->d_type == DT_DIR) {
        gb_string_appendc(color_str, BLUE);
        gb_string_appendc(color_str, BOLD);
    } else if (sb->st_mode & S_IEXEC) {
        gb_string_appendc(color_str, GREEN);
        gb_string_appendc(color_str, BOLD);
    } else {
        gb_string_appendc(color_str, NORMAL_COLOR);
    }
    if (BITTEST(flags, SHOW_FILE_SIZE)) {
        f32 file_size = sb->st_size;
        gb_local_persist gbString unit_str = gb_string_make_reserve(gb_heap_allocator(), 5);
        gb_string_appendc(unit_str, " B");
        defer(gb_string_clear(unit_str));

        if (file_size > gb_kilobytes(1) && file_size < gb_megabytes(1)) {
            gb_string_clear(unit_str);
            gb_string_appendc(unit_str, "KB");
            file_size /= gb_kilobytes(1);
        } else if (file_size > gb_megabytes(1) && file_size < gb_gigabytes(1)) {
            gb_string_clear(unit_str);
            gb_string_appendc(unit_str, "MB");
            file_size /= gb_megabytes(1);
        } else if (file_size > gb_gigabytes(1) && file_size < gb_terabytes(1)) {
            gb_string_clear(unit_str);
            gb_string_appendc(unit_str, "GB");
            file_size /= gb_gigabytes(1);
        } else if (file_size > gb_terabytes(1)) {
            gb_string_clear(unit_str);
            gb_string_appendc(unit_str, "TB");
            file_size /= gb_terabytes(1);
        }

        gb_string_append_fmt(file_size_str, "%s%5.1f %s ", NORMAL_COLOR, file_size, unit_str);
    }
    gb_printf("%s%s%s\n", file_size_str, color_str, file->d_name);
}
// printf("Last file modification:   %s", ctime(&sb.st_mtime));

int sort_files_and_dirs(const void* a, const void* b)
{
    dirent* a_file = (dirent*)a;
    dirent* b_file = (dirent*)b;

    if (a_file->d_type == DT_DIR && b_file->d_type == DT_DIR) {
        return gb_strcmp(a_file->d_name, b_file->d_name);
    }

    if (a_file->d_type != DT_DIR && b_file->d_type != DT_DIR) {
        return gb_strcmp(a_file->d_name, b_file->d_name);
    }

    if (a_file->d_type == DT_DIR) {
        return -1;
    } else {
        return +1;
    }

    GB_PANIC("Shouldn't be here\n");
    return 0;
}
