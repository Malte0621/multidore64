# Building

Everything you need to know about compiling a MultiDore 64 project.

## Build scripts

The repository ships two equivalent scripts:

| Script | Platform | Output |
|---|---|---|
| `compile.sh` | Linux / macOS / WSL | `dist/main.prg`, `dist/main.d64` |
| `compile.bat` | Windows | `dist\main.prg`, `dist\main.d64` |
Both wipe `dist/` first, so the output directory always reflects the current sources.

```bash
./compile.sh      # Linux
compile.bat       # Windows (from a cmd prompt)
```

## Compiler invocation

The scripts call oscar64 directly, which is all a build needs:

```bash
oscar64 -O3 -Ox -Op -tm=c64 -tf=prg -o=dist/main.prg \
    src/main.c \
    src/multidore64/renderlib.c \
    src/multidore64/soundlib.c \
    src/multidore64/soundlib_asm.c \
    src/multidore64/controllerlib.c \
    src/multidore64/utilslib.c \
    src/multidore64/filelib.c

| Flag | Meaning |
|---|---|
| `-tm=c64` | Target the Commodore 64 |
| `-tf=prg` | Emit a `PRG` file with a BASIC loader stub |
| `-o=FILE` | Output path (**the equals sign is required**) |
| `-O3` | Full optimization |
| `-Ox` | Extra size optimization |
| `-Op` | Optimize for page-zero usage |

!!! warning
    `-o dist/main.prg` (space) is rejected by oscar64. Always write `-o=dist/main.prg`.

## Creating D64 disk images

The compile scripts automatically bundle `dist/main.prg` (as `main`) and `src/song.bin` (as `song.bin`) into `dist/main.d64`.

To bundle extra files or PRGs onto the disk image, simply pass them as arguments:

```bash
# Linux / macOS
./compile.sh extra.prg:extra level1.dat:level1

# Windows
compile.bat extra.prg:extra level1.dat:level1
```

You can also call the disk creator directly:

```bash
python3 tools/make_d64.py dist/main.d64 multidore64 dist/main.prg:main src/song.bin:song.bin extra.dat:extra
```

`make_d64.py` uses `c1541` if available in `PATH`, or falls back to an internal pure-Python 1541 allocator if not. No external dependencies required.

The base scripts build `main.c` plus the core modules. To use **render3d** or **netlib**, add their sources to the command line (see also [3D rendering](render3d.html) and [Networking](netlib.html)):

```bash
oscar64 -O3 -Ox -Op -tm=c64 -tf=prg -o=dist/main.prg \
    src/main.c \
    src/multidore64/renderlib.c \
    src/multidore64/soundlib.c \
    src/multidore64/soundlib_asm.c \
    src/multidore64/controllerlib.c \
    src/multidore64/utilslib.c \
    src/multidore64/render3d.c \
    src/multidore64/netlib.c
```

## Adding your own sources

Add each new `.c` file to the compile line. Headers live next to their modules; include them from `src/` with the `multidore64/` prefix:

```c
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
```

## Debug builds

Drop the optimization flags for readable code and an emitted assembly listing:

```bash
oscar64 -tm=c64 -tf=prg -o=dist/debug.prg src/main.c \
    src/multidore64/*.c
```

With optimization enabled oscar64 also writes a `.asm` listing and a `.map` file next to the output, which show the final 6502 code and every symbol's address - invaluable when debugging at the metal.

## Running in VICE

```bash
x64 dist/main.prg                 # autostart the PRG
x64 -remotemonitor dist/main.prg  # plus a remote monitor on port 6510
```

See [Memory map](memory.html) for what the compiled program expects from C64 memory.
