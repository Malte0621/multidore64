# Audio - soundlib

`soundlib` plays PSID music through the SID chip. You copy a tune into memory, initialize it once, and then **step it once per frame from your main loop**.

```c
#include "multidore64/soundlib.h"
```

## API

```c
void soundlib_init(void);
unsigned char soundlib_play(const char *tune, unsigned int len); // play buffer (raw or PSID)
unsigned char soundlib_play_file(const char *filename);          // load from disk & play
void soundlib_update(void);                                      // one play tick per frame
void soundlib_stop(void);                                        // silence all voices

## Quick start

```c
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/colorlib.h"
int main(void)
{
    renderlib_init();
    soundlib_init();

    // Load tune from disk and start playback (no embedding needed)
    soundlib_play_file("song.bin");

    while (1)
    {
        soundlib_update();          // exactly one step per frame
        // ... game logic ...
    }
}
```

## How it works
- `soundlib_play_file(filename)` loads a tune directly from disk (device 8) into `$4000` and starts playback.
- `soundlib_play(buf, len)` stages an in-memory tune at `$4000` and starts playback. Both raw 6502 code and unstripped PSID/RSID files are supported (headers are automatically detected and stripped).
- `soundlib_update()` calls the tune's play routine once per 50 Hz frame, paced by the VIC raster. It **never blocks**: on frames it has already seen it returns immediately.
- `soundlib_stop()` sets all three voices to release mode and stops updates.

## Rules and pitfalls

!!! danger
    Never call `soundlib_update()` from an interrupt handler. The bundled tune keeps its sequencer work data in the CPU stack page (`$0100-$01FF`); IRQ frames collide with it and the machine crashes to BASIC. Step the player from your main loop only - that is exactly what `soundlib_update()` is designed for.

- **Call it once per frame, every frame.** Calling it faster speeds the music up; skipping frames slows it down or makes it stutter.
- One tune at a time: `$4000-$453B` is owned by the player while music plays (see [memory map](memory.html)).
- Don't poke SID registers for sound effects while the tune plays - the player rewrites most registers every frame. Plan effects after `soundlib_stop()` or on voices the tune leaves alone.
- The engine embeds no tune binaries. Load songs from disk via `soundlib_play_file()` or pass a memory buffer to `soundlib_play()`.

## Stopping music

```c
soundlib_stop();
// ... SID is silent, $4000 area can be reused ...
```

After `soundlib_stop()` the `$4000` staging area is free for your own data again.
