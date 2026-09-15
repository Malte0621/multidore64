# MultiDore 64

**A game engine for the Commodore 64, written in C for the [oscar64](https://github.com/drmortalwombat/oscar64) compiler.**

MultiDore 64 gives you a complete set of modules for building C64 games in C: a 2D renderer with text, bitmap and sprite support, a software 3D renderer with a raycasting engine, digital audio playback, joystick and keyboard input, and a serial link protocol for two-machine play.

```c
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    renderlib_init();
    renderlib_setborder(color_blue);
    renderlib_drawstring(12, 12, color_white, "HELLO WORLD");
    while (1) { }
}
```

## Feature overview

| Module | What it gives you |
|---|---|
| **renderlib** | 5 display modes, drawing primitives, flood fill, blitting, text, 8 hardware sprites, palette control |
| **soundlib** | PSID music playback with a main-loop stepping API |
| **render3d** | Integer-math 3D: camera, projection, wireframe and flat-shaded polygons, raycast first-person rendering |
| **controllerlib** | Keyboard and dual joystick input |
| **netlib** | Serial link (CIA2) with a reliable ACK/NAK packet protocol |
| **colorlib** | The 16-color VIC-II palette as named constants |
| **utilslib** | Timing helpers |
| **filelib** | Commodore 64 disk filesystem (read, write, append, delete) |

## Documentation

| Page | Description |
|---|---|
| [Getting started](getting-started.html) | Install the toolchain and build your first program |
| [Building](build.html) | Build scripts, compiler flags and project layout |
| [Rendering](renderlib.html) | Display modes, drawing, text and sprites |
| [Audio](soundlib.html) | Playing PSID music |
| [Input](controllerlib.html) | Keyboard and joysticks |
| [3D rendering](render3d.html) | Camera, polygons and the raycaster |
| [Networking](netlib.html) | Two-player serial link |
| [Colors](colorlib.html) | The 16-color palette |
| [Utilities](utilslib.html) | Sleep and timing |
| [Filesystem](filelib.html) | C64 disk file read/write/append |
| [Memory map](memory.html) | How the engine lays out C64 memory |
| [Examples](examples.html) | Complete, runnable programs |

## Requirements

- **oscar64** compiler (the engine is oscar64-native; cc65 is not supported)
- A Commodore 64, or an emulator such as [VICE](https://vice-emu.sourceforge.io/)

## License and credits

MultiDore 64 is (c) 2023-2026 by Malte0621. The engine plays standard PSID side-1 tunes through its own player bridge.
