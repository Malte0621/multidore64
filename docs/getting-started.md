# Getting started

This guide takes you from an empty directory to a running MultiDore 64 program in an emulator.

## 1. Install the toolchain

MultiDore 64 is built with [oscar64](https://github.com/drmortalwombat/oscar64), a whole-program optimizing C compiler targeting the 6502.

===

**Linux (Debian/Ubuntu)**

```bash
sudo apt install make git
git clone https://github.com/drmortalwombat/oscar64.git
cd oscar64 && make -j && sudo make install
```

**Linux (Debian/Ubuntu)**

```bash
sudo apt install make git
git clone https://github.com/drmortalwombat/oscar64.git
cd oscar64 && make -j && sudo make install
```

**Windows:** download a release bundle from the
[oscar64 releases](https://github.com/drmortalwombat/oscar64/releases) page,
unpack it, and add its `bin` directory to your `PATH`.

Verify the installation:

```bash
oscar64 --version
```

## 2. Get the engine

```bash
git clone https://github.com/Malte0621/multidore64.git
cd multidore64
```

The project layout:

```
multidore64/
├── compile.sh           # Linux build script
├── compile.bat          # Windows build script
├── src/
│   ├── main.c           # your game entry point
│   ├── song.bin         # default music (PSID)
│   └── multidore64/     # the engine modules
│       ├── renderlib.c/.h
│       ├── render3d.c/.h
│       ├── soundlib.c/.h
│       ├── soundlib_asm.c
│       ├── controllerlib.c/.h
│       ├── netlib.c/.h
│       ├── colorlib.h
│       ├── utilslib.c/.h
│       └── filelib.c/.h
└── dist/                # build output (created by the build)
```

## 3. Build

**Linux:**

```bash
./compile.sh
```

**Windows:**

```bat
compile.bat
```

Both scripts produce `dist/main.prg`, a runnable C64 program.

## 4. Run it

In [VICE](https://vice-emu.sourceforge.io/):

```bash
x64 dist/main.prg
```

or attach the PRG to any emulator, or write it to real floppy/SD2IEC media and `LOAD"*",8,1` + `RUN` it on real hardware.

## 5. Your first program

Replace the contents of `src/main.c` with:

```c
#include <stdio.h>
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    renderlib_init();

    renderlib_setborder(color_black);
    renderlib_setbg(color_blue);
    renderlib_drawstring(4, 12, color_white, "HELLO MULTIDORE 64");

    while (1)
    {
        // keep the program alive - falling out of main returns to BASIC
    }
}
```

Build and run again:

```bash
./compile.sh && x64 dist/main.prg
```

You should see the greeting text on a blue screen with a black border.

!!! note
    Keep a `while (1)` loop at the end of your program. Returning from `main` hands control back to the BASIC interpreter.

## 6. Next steps

- [Building](build.html) - flags, scripts and how to add your own source files
- [Rendering](renderlib.html) - modes, primitives, sprites and text
- [Examples](examples.html) - complete runnable programs for every module
