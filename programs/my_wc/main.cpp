#include <cstdio>
#define GB_IMPLEMENTATION
#include "../../lib/bitset.h"
#include "../../lib/gb.h"
#include <unistd.h>

#define NORMAL_COLOR "\x1B[0m"
#define GREEN "\x1B[32m"
#define BLUE "\x1B[34m"
#define RED "\x1B[31m"
#define BOLD "\x1B[1m"
#define RED_BACKGROUND "\x1B[41m"
#define DEFAULT_BACKGROUND "\x1B[49m"
typedef enum { COUNT_LINES, COUNT_WORDS, COUNT_BYTES, FLAGS_COUNT} FLAGS;

gb_global b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};

int main(int argc, char** argv) {

    // TODO: if no file is passed as arg, read stdin
    if (argc == 1) {
        gb_printf_err("Usage: %s [file]\n", argv[0]);
        gb_exit(1);
    }

    auto file = gb_file_read_contents(gb_heap_allocator(), 1, argv[1]);
    defer(gb_file_free_contents(&file));

    isize total_lignes = 0;
    isize total_bytes = file.size;
    isize total_words = 0;
    isize total_white_space = 0;

    for (isize i = 0; i < file.size; i++) {
        char* current_character = (char*)file.data + i;
        
        if (*current_character == '\n')
            total_lignes++;

        //TODO: make better method, since this ignores things like '(bob is dead)' and '}' is a word
        if (gb_char_is_space(*current_character))
            total_white_space++;
    }

    if (file.size > 0) {
        total_words = total_white_space > 0 ? total_white_space - 1 : 1;
    }

    printf("total lignes: %ld\n", total_lignes);
    printf("total bytes: %ld\n", total_bytes);
    printf("total words: %ld\n", total_words);
    return 0;
}

// int opt;
// while ((opt = getopt(argc, argv, "als")) != -1) {
//     switch (opt) {
//     case 'a':
//         BITSET(flags, SHOW_DOT_FILES);
//         break;
//     case 's':
//         BITSET(flags, SHOW_FILE_SIZE);
//         break;
//     case 'l':
//         BITSET(flags, SHOW_LAST_MOD_DATE);
//         BITSET(flags, SHOW_FILE_SIZE);
//         break;
//     default:
//         gb_printf_err("Usage: %s [-asl] [directory...]\n", argv[0]);
//         gb_exit(1);
//     }
// }
// int* p_optind = &optind;
// if (*p_optind >= argc) {
//     d = opendir(".");
// } else {
//     d = opendir(argv[*p_optind]);
// }
// if (!d) {
//     gb_printf_err("%s%s\n", NORMAL_COLOR, strerror(errno));
//     gb_exit(1);
// }


