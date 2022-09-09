CFLAGS='-Wall -std=c11 -pedantic -ggdb'
LFLAGS='-lm'

set -xe
cc $CFLAGS -o ag ascii_gen.c $LFLAGS
