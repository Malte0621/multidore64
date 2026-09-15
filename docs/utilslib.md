# Utilities - utilslib

Small helpers shared by the engine and your game.

```c
#include "multidore64/utilslib.h"
```

## API

```c
void sleep(unsigned int frames);
```

Blocks for `frames` **video frames** - one frame = 1/50 s on PAL machines (1/60 s NTSC). The delay is paced against the VIC raster, so it is real time, not CPU cycles:

```c
renderlib_drawstring(0, 0, color_white, "BOOTING...");
sleep(50);                       // one second on PAL
```

## Notes

- `sleep()` still polls the raster register between frames. A genuinely sleeping CPU (zero-power idle) would require an interrupt timer, which the engine deliberately does not install - see [memory map](memory.html) for why interrupts are kept minimal.
- For in-game pacing without blocking (e.g. stepping music while polling input), use the pattern from [soundlib](soundlib.html): detect the frame boundary, act once, return.
