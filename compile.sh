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

# Build raw binary (shared by prg and tap)
build_bin() {
    $OSCAR64 $FLAGS -tf=bin -o=../dist/main.bin $sources
}

build_prg() {
    echo "Building PRG..."
    build_bin
    python3 -c "
import struct
prog = open('../dist/main.bin', 'rb').read()
# C64 BASIC stub: '10 SYS <addr>'
# Stub is 12 bytes, loaded at \$0801. Program binary follows.
sys_addr = 0x0801 + 12  # \$080D
stub = bytes([
    0x00, 0x00,   # end-of-program / first-line pointer
    0x0A, 0x00,   # line 10
    0x00, 0x00,   # next line pointer (none)
    0x9A,         # SYS token
    0x00,         # space
    sys_addr & 0xFF, (sys_addr >> 8) & 0xFF,
    0x00,         # end of line
    0x00,         # end of program
])
load_addr = 0x0801
total_len = len(stub) + len(prog)
header = struct.pack('<HH', load_addr, total_len)
with open('../dist/main.prg', 'wb') as f:
    f.write(header)
    f.write(stub)
    f.write(prog)
"
    rm -f ../dist/main.bin
    echo "  -> dist/main.prg ($(stat -c '%s' ../dist/main.prg) bytes)"
}

build_crt() {
    echo "Building cartridge..."
    $OSCAR64 $FLAGS -tf=crt -cname="$CARTRIDGE_NAME" -cid=$CARTRIDGE_ID -csub=$CARTRIDGE_SUB -o=../dist/main.crt $sources
    echo "  -> dist/main.crt ($(stat -c '%s' ../dist/main.crt) bytes)"
}

build_tap() {
    echo "Building tape image..."
    build_bin
    python3 -c "
import struct
data = open('../dist/main.bin', 'rb').read()
addr = 0x0801
name = b'MAIN          '

def crc16(buf):
    crc = 0
    for b in buf:
        crc ^= b
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0x8408
            else:
                crc >>= 1
    return crc

# Message block (optional, shown on screen before load)
msg_data = b'\x00\x00\x00\x00' + b'\x00\x00\x00\x00' + b'\x00' + bytes([len(name)]) + name + b'\x00'
msg_hdr = struct.pack('<I', 0xA596271F) + struct.pack('<H', 4) + msg_data[:4] + struct.pack('<H', 0)
msg_blk = struct.pack('<I', 0xA596271F) + struct.pack('<H', len(msg_data)) + msg_data + struct.pack('<H', 0)

# Program header block
hdr_data = bytes([0x00, addr & 0xFF, (addr >> 8) & 0xFF, (len(data) >> 8) & 0xFF])
hdr_blk = struct.pack('<I', 0xA596271F) + struct.pack('<H', 4) + hdr_data + struct.pack('<H', 0)

# Program data block
data_blk = struct.pack('<I', 0xA5271FA5) + struct.pack('<H', len(data)) + data + struct.pack('<H', 0)

with open('../dist/main.tap', 'wb') as f:
    f.write(msg_blk)
    f.write(hdr_blk)
    f.write(data_blk)
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