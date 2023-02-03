#include <cstdio>
#define GB_IMPLEMENTATION
#include "../lib/gb.h"
#include "../lib/bitset.h"
#include <dirent.h>
#include <sys/stat.h>

#define NORMAL_COLOR "\x1B[0m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define RED "\x1B[31m"
#define BOLD "\x1B[1m"
typedef struct stat Stat;

typedef enum FLAGS {
    IGNORE_DOT_FILES,
    FLAGS_COUNT
} FLAGS;

int main(int argc, char** argv)
{
    b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};
    // BITSET(flags, IGNORE_DOT_FILES);
    DIR* d = NULL;
    gbArray(dirent) things = NULL;
    gb_array_init(things, gb_heap_allocator());
    defer(gb_array_free(things));

    // very cool change right now

    //TODO: Learn how to fail the program with grace

    //TODO: dealings with flags like -a and -sh
    //TODO: with multiple directories, do a listing of each directory one after the other
    if (argc == 1) {
        d = opendir(".");
    } else if (argc == 2) {
        d = opendir(argv[1]);
    } else {
        GB_PANIC("TODO\n");
    }
    GB_ASSERT_NOT_NULL(d);
    defer(closedir(d));

    dirent* current_d = NULL;
    while (current_d = readdir(d)) {
        gb_array_append(things, *current_d);
    }
    //TODO: put the directories on top of the list (and "." and ".." always on top)
    gb_sort_array(things, gb_array_count(things), gb_str_cmp(gb_offset_of(dirent, d_name)));

    for (int i = 0; i < gb_array_count(things); i++) {
        
        if ((things[i].d_name[0] == '.' && BITTEST(flags, IGNORE_DOT_FILES)))
            continue;
        
        if (things[i].d_type == DT_DIR) {
            printf("%s", BOLD);
            printf("%s%s\n", BLUE, things[i].d_name);
        } else {
            Stat sb = {};
            stat(things[i].d_name,&sb);
            if (sb.st_mode & S_IEXEC) {
                printf("%s", BOLD);
                printf("%s%s\n", GREEN, things[i].d_name);
            } else {
                printf("%s", BOLD);
                printf("%s%s\n", NORMAL_COLOR, things[i].d_name);
            }
        }
    }
    printf("%s", NORMAL_COLOR);
    return 0;
}

// printf("File size:                %jd bytes\n", (isize)sb.st_size);
// printf("Last file access:         %s", ctime(&sb.st_atime));
// printf("Last file modification:   %s", ctime(&sb.st_mtime));


