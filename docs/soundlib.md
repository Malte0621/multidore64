# Audio - soundlib

`soundlib` plays PSID music through the SID chip. You copy a tune into memory, initialize it once, and then **step it once per frame from your main loop**.

```c
#include "multidore64/soundlib.h"
```

## API

```c
void soundlib_init(void);
void soundlib_play(char FILEDATA[]);   // copy tune + initialize SID
void soundlib_update(void);            // one play tick (call once per frame)
void soundlib_stop(void);              // silence all voices
```

## Quick start

```c
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/colorlib.h"

extern char SIDFILE[];   // provided by the engine (src/song.bin)

int main(void)
{
    renderlib_init();
    soundlib_init();
    renderlib_drawstring(12, 12, color_white, "NOW PLAYING...");

    soundlib_play(SIDFILE);

    while (1)
    {
        soundlib_update();          // exactly one step per frame
        // ... game logic ...
    }
}
```

## How it works

- `soundlib_play()` copies the PSID payload to `$4000` and calls the tune's init routine.
- `soundlib_update()` calls the tune's play routine once per 50 Hz frame, paced by the VIC raster. It **never blocks**: on frames it has already seen it returns immediately.
- `soundlib_stop()` sets all three voices to release mode.

## Rules and pitfalls

!!! danger
    Never call `soundlib_update()` from an interrupt handler. The bundled tune keeps its sequencer work data in the CPU stack page (`$0100-$01FF`); IRQ frames collide with it and the machine crashes to BASIC. Step the player from your main loop only - that is exactly what `soundlib_update()` is designed for.

- **Call it once per frame, every frame.** Calling it faster speeds the music up; skipping frames slows it down or makes it stutter.
- One tune at a time: `$4000-$453B` is owned by the player while music plays (see [memory map](memory.html)).
- Don't poke SID registers for sound effects while the tune plays - the player rewrites most registers every frame. Plan effects after `soundlib_stop()` or on voices the tune leaves alone.
- The embedded tune lives in `src/song.bin` and is exposed as `extern char SIDFILE[]`. Replace the file to change the music; keep the PSID format (header + tune) intact.

## Stopping music

```c
soundlib_stop();
// ... SID is silent, $4000 area can be reused ...
```

After `soundlib_stop()` the `$4000` staging area is free for your own data again.
