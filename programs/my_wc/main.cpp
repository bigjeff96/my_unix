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
typedef enum { COUNT_LINES, COUNT_WORDS, COUNT_BYTES, FLAGS_COUNT } FLAGS;

gb_global b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};

int main(int argc, char** argv)
{
    gbString buffer = {};
    defer(gb_string_free(buffer));
    if (argc == 1) {
        auto input_file = gb_file_get_standard(gbFileStandard_Input);
        isize file_size = gb_file_size(input_file);

        if (!file_size) {
            // FIXME: can't read piped data like 'head main.cpp | ./my_wc.exe', I get 0 file size
            printf("ERROR: empty stdin\n");
            gb_exit(1);
        }

        buffer = gb_string_make_reserve(gb_heap_allocator(), file_size);
        b32 good = gb_file_read(input_file, buffer, gb_string_capacity(buffer));
        GB_STRING_HEADER(buffer)->length = file_size;
        if (!good) {
            printf("ERROR:\n");
            gb_exit(1);
        }
    } else {
        auto file = gb_file_read_contents(gb_heap_allocator(), 1, argv[1]);
        buffer = cast(char*) file.data;
        GB_STRING_HEADER(buffer)->allocator = file.allocator;
        GB_STRING_HEADER(buffer)->length = file.size;
        GB_STRING_HEADER(buffer)->capacity = file.size;
    }

    isize total_lignes = 0;
    isize total_bytes = gb_string_length(buffer);
    isize total_words = 0;
    isize total_white_space = 0;
    bool in_word = false;

    for (isize i = 0; i < gb_string_length(buffer); i++) {
        if (!gb_char_is_space(buffer[i])) {
            in_word = true;
        } else {
            if (buffer[i] == '\n')
                total_lignes++;
            if (in_word) {
                total_words++;
                in_word = false;
            }
        }
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
