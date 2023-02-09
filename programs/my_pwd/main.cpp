#define GB_IMPLEMENTATION
#include "../../lib/gb.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    gbBuffer(char) buffer = NULL;
    gb_buffer_init(buffer, gb_heap_allocator(), gb_kilobytes(1));
    defer(gb_buffer_free(buffer, gb_heap_allocator()));
    buffer = getcwd(buffer, gb_buffer_capacity(buffer));
    
    if (!buffer) {
        gb_printf_err("ERROR: %s\n", strerror(errno));
        gb_exit(1);
    }
    
    gb_printf("%s\n", buffer);

    return 0;
}

