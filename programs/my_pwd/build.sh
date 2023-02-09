#!/usr/bin/bash

CC="g++"
DFLAGS="-Wall -Wextra -ggdb -Wno-unused-parameter -Wno-parentheses -Wno-unused-variable -Wno-missing-field-initializers -Wno-implicit-fallthrough -Wno-ignored-qualifiers"
MEM="-fsanitize=undefined"
PROG_NAME="my_pwd.exe"

set -e
time $CC  $DFLAGS -I . *.cpp -o $PROG_NAME  
echo "--OUTPUT--"
eval "time ./$PROG_NAME" 
