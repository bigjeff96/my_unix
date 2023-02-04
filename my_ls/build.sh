#!/usr/bin/bash

CC="g++"
DFLAGS="-Wall -Wextra -Wno-implicit-fallthrough -Wno-unused-variable -Wno-unused-parameter -Wno-ignored-qualifiers -Wno-parentheses -Wno-missing-field-initializers -ggdb"
MEM="-fsanitize=address"

set -e
time $CC  $DFLAGS -O0  -I . *.cpp -o my_ls
echo "--OUTPUT--"
./my_ls  -s 
