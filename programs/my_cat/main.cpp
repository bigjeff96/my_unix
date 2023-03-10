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
typedef enum { SHOW_DOT_FILES, SHOW_FILE_SIZE, SHOW_LAST_MOD_DATE,FLAGS_COUNT } FLAGS;

gb_global b8 flags[BITNSLOTS(FLAGS_COUNT)] = {};

int main(int argc, char** argv) {

    if (argc == 1) {
        gb_printf_err("Usage: %s [file]\n", argv[0]);
        gb_exit(1);
    }

    auto file = gb_file_read_contents(gb_heap_allocator(), 1, argv[1]);
    defer(gb_file_free_contents(&file));

    printf("%s\n", (char*)(file.data));

    return 0;
}



