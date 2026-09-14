#!/bin/bash
# MultiDore 64 build script (oscar64)
# oscar64 is a whole-program optimizing C compiler for the 6502.
# It compiles, assembles, and links in a single pass.
#
# Usage:
#   ./compile.sh          - build .prg only (default)
#   ./compile.sh prg      - build .prg
#   ./compile.sh crt      - build cartridge (.crt)
#   ./compile.sh tap      - build tape image (.tap)
#   ./compile.sh all      - build all formats

set -e

# Find oscar64 - check for working installation (binary + includes)
OSCAR64="${OSCAR64:-}"
if [ -z "$OSCAR64" ]; then
    for candidate in oscar64 /bin/oscar64 /usr/bin/oscar64 /usr/local/bin/oscar64 /tmp/oscar64/bin/oscar64; do
        if command -v "$candidate" &>/dev/null || [ -x "$candidate" ]; then
            OSCAR64="$candidate"
            break
        fi
    done
fi
if [ -z "$OSCAR64" ]; then
    echo "Error: oscar64 not found."
    echo "Install oscar64 or set OSCAR64 env var to the binary path."
    exit 1
fi

# Cartridge settings
CARTRIDGE_NAME="MULTIDORE64"
CARTRIDGE_ID=0x01
CARTRIDGE_SUB=0x00

# Clean previous build artifacts
rm -rf build
rm -rf dist
mkdir -p dist

cd src

# Collect all source files (main.c + multidore64/*.c)
sources="main.c"
for file in multidore64/*.c; do
    sources="$sources $file"
done

# oscar64 common flags
FLAGS="-O3 -Oo -tm=c64"

build_prg() {
    echo "Building PRG..."
    $OSCAR64 $FLAGS -tf=prg -o=../dist/main.prg $sources
    echo "  -> dist/main.prg ($(stat -c '%s' ../dist/main.prg) bytes)"
}

build_crt() {
    echo "Building cartridge..."
    $OSCAR64 $FLAGS -tf=crt -cname="$CARTRIDGE_NAME" -cid=$CARTRIDGE_ID -csub=$CARTRIDGE_SUB -o=../dist/main.crt $sources
    echo "  -> dist/main.crt ($(stat -c '%s' ../dist/main.crt) bytes)"
}

build_tap() {
    echo "Building tape image..."
    $OSCAR64 $FLAGS -tf=bin -o=../dist/main.bin $sources
    python3 -c "
data = open('../dist/main.bin', 'rb').read()
name = b'MAIN          '
addr = 0x0801
msg = b'\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00' + bytes([len(name)]) + name + b'\x00'
hdr = b'\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x10' + bytes([addr & 0xFF, (addr >> 8) & 0xFF]) + b'\x00'
with open('../dist/main.tap', 'wb') as f:
    f.write(msg)
    f.write(hdr)
    f.write(data)
"
    rm -f ../dist/main.bin
    echo "  -> dist/main.tap ($(stat -c '%s' ../dist/main.tap) bytes)"
}

# Determine what to build
target="${1:-prg}"

case "$target" in
    prg)
        build_prg
        ;;
    crt)
        build_crt
        ;;
    tap)
        build_tap
        ;;
    all)
        build_prg
        build_crt
        build_tap
        ;;
    *)
        echo "Unknown target: $target"
        echo "Usage: $0 [prg|crt|tap|all]"
        exit 1
        ;;
esac

echo "Build successful!"