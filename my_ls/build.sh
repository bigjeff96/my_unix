#!/usr/bin/bash

CC="g++"
DFLAGS="-Wall -Wextra -ggdb"
MEM="-fsanitize=address"

set -e
time $CC  $DFLAGS  -I . *.cpp -o my_ls.exe 2<&1 | grep -v -i gb.h
echo "--OUTPUT--"
time ./my_ls.exe -sa ~
