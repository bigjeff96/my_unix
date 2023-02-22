#define GB_IMPLEMENTATION
#include "../../lib/gb.h"

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

    isize max_new_lines = 11;
    char* start_point = NULL;

    for (isize i = gb_string_length(buffer) - 1; i > 0; i--) {
        if (max_new_lines == 0) {
            start_point = &buffer[i + 1];
            break;
        }

        if (buffer[i] == '\n')
            max_new_lines--;
    }

    if (max_new_lines != 0) {
        //TODO: allow to use program with text files with less than 10 lines
        gb_printf_err("ERROR: file with less than 10 newlines at end\n");
        gb_exit(1);
    }

    char* end_point = &buffer[gb_string_length(buffer) - 1];
    while (start_point++ != end_point) {
        //TODO: not very efficient, but since its only 10 lines, doesn't really matter I guess
        printf("%c", *start_point);
    }

    return 0;
}
