#define GB_IMPLEMENTATION
#include "../../lib/gb.h"

typedef enum { COUNT_LINES, COUNT_WORDS, COUNT_BYTES, FLAGS_COUNT } FLAGS;
const char* flags_str = "wlc";

gb_global gbBitset flags[GB_FLAGS_ARRAY_SIZE(FLAGS_COUNT)] = { 0 };

int main(int argc, char** argv)
{

    // parsing the args with the flags
    int opt;
    while ((opt = getopt(argc, argv, flags_str)) != -1) {
        switch (opt) {
        case 'w':
            GB_FLAGS_SET(flags, COUNT_WORDS);
            break;
        case 'l':
            GB_FLAGS_SET(flags, COUNT_LINES);
            break;
        case 'c':
            GB_FLAGS_SET(flags, COUNT_BYTES);
            break;
        default:
            gb_printf_err("Usage: %s [-%s] [file]\n", argv[0], flags_str);
            gb_exit(1);
        }
    }

    if (gb_flags_is_empty(flags, FLAGS_COUNT))
        gb_flags_activate_all(flags, FLAGS_COUNT);

    int* p_optind = &optind;

    gbString buffer = {};
    defer(gb_string_free(buffer));
    if (*p_optind >= argc) {
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
        auto file = gb_file_read_contents(gb_heap_allocator(), 1, argv[*p_optind]);
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
    if (GB_FLAGS_TEST(flags, COUNT_LINES))
        printf("total lignes: %ld\n", total_lignes);
    if (GB_FLAGS_TEST(flags, COUNT_BYTES))
        printf("total bytes: %ld\n", total_bytes);
    if (GB_FLAGS_TEST(flags, COUNT_WORDS))
        printf("total words: %ld\n", total_words);
    return 0;
}
