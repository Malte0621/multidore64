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
    src/multidore64/utilslib.c

echo "Build successful!"