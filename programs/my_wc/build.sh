#!/usr/bin/bash

DEBUG=1
SANITIZE=0

CC="g++"
PROG_NAME="my_wc.exe"

NOTHING=" "
DFLAGS="-Wall -Wextra -ggdb -Wno-unused-parameter -Wno-parentheses -Wno-unused-variable -Wno-missing-field-initializers -Wno-implicit-fallthrough -Wno-ignored-qualifiers"
OPFLAGS="-O2 -march=native -Wall -Wextra -ggdb -Wno-unused-parameter -Wno-parentheses -Wno-unused-variable -Wno-missing-field-initializers -Wno-implicit-fallthrough -Wno-ignored-qualifiers -Wno-strict-aliasing"
UNDEF_FLAG="-fsanitize=undefined"

MEM_FLAG=$([ $SANITIZE == 1 ] && echo $UNDEF_FLAG || echo $NOTHING)
FLAGS=$([ $DEBUG == 1 ] && echo $DFLAGS || echo $OPFLAGS)

set -e
time $CC $FLAGS $MEM_FLAG  -I . *.cpp -o $PROG_NAME
echo "--OUTPUT--"
eval "time ./$PROG_NAME "
