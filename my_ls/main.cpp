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
typedef enum FLAGS { SHOW_DOT_FILES, SHOW_FILE_SIZE, FLAGS_COUNT } FLAGS;

void print_file(dirent* file, Stat* sb);

gb_global b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};

int main(int argc, char** argv)
{
    DIR* d = NULL;
    gbArray(dirent) files = NULL;
    gb_array_init(files, gb_heap_allocator());
    defer(gb_array_free(files));

    int opt;
    int* p_optind = &optind;

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
    // TODO: with multiple directories, do a listing of each directory one after the other
    if (*p_optind >= argc) {
        d = opendir(".");
    } else {
        d = opendir(argv[*p_optind]);
    }

    GB_ASSERT_NOT_NULL(d);
    defer(closedir(d));

    dirent* current_d = NULL;
    while (current_d = readdir(d)) {
        gb_array_append(files, *current_d);
    }
    // TODO: put the directories on top of the list (and "." and ".." always on top)
    gb_sort_array(files, gb_array_count(files), gb_str_cmp(gb_offset_of(dirent, d_name)));

    defer(printf("%s", NORMAL_COLOR));
    for (int i = 0; i < gb_array_count(files); i++) {

        Stat sb = {};
        stat(files[i].d_name, &sb);

        if ((files[i].d_name[0] == '.' && !BITTEST(flags, SHOW_DOT_FILES)))
            continue;
        print_file(&files[i], &sb);
    }
    return 0;
}

//TODO: Align the columns correctly (especially for file sizes)
void print_file(dirent* file, Stat* sb)
{
    char color[50] = NORMAL_COLOR;
    char file_size[255] = {};

    if (file->d_type == DT_DIR) {
        gb_memcopy(color, BLUE, sizeof(BLUE));
    } else if (sb->st_mode & S_IEXEC) {
        gb_memcopy(color, GREEN, sizeof(GREEN));
    } 

    if (BITTEST(flags, SHOW_FILE_SIZE)) {
        gb_snprintf(file_size, 100, "%s%0.1f KB ", NORMAL_COLOR, f32(sb->st_size / 1024.));
    }
    gb_printf("%s%s%s%s\n", file_size, color, BOLD, file->d_name);
}
// printf("Last file modification:   %s", ctime(&sb.st_mtime));
