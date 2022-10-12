CFLAGS='-Wall -pedantic -g'
LFLAGS='-lm'

set -xe
cc $CFLAGS -o ag ascii_gen.c $LFLAGS
