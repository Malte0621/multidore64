#!/bin/bash
# MultiDore 64 build script (oscar64)
set -e

rm -rf dist
mkdir -p dist

oscar64 -O3 -Ox -Op \
    -tm=c64 \
    -tf=prg \
    -o=dist/main.prg \
    src/main.c \
    src/multidore64/renderlib.c \
    src/multidore64/soundlib.c \
    src/multidore64/soundlib_asm.c \
    src/multidore64/controllerlib.c \
    src/multidore64/utilslib.c \
    src/multidore64/filelib.c

# Build standard 1541 disk image (dist/main.d64)
# Default bundle: main PRG and song.bin. Pass extra files via script arguments:
#   ./compile.sh extra.prg:extra level1.dat:level1
python3 tools/make_d64.py dist/main.d64 multidore64 dist/main.prg:main src/song.bin:song.bin "$@"

echo "Build successful! Generated dist/main.prg and dist/main.d64"