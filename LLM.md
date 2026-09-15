# MultiDore 64 — LLM Reference

Complete reference for writing C64 games in C with the MultiDore 64 engine. Everything below is sufficient to write correct, compiling programs.

## 1. Toolchain and build

- Compiler: **oscar64** (whole-program optimizing C→6502). cc65 does NOT work.
- Build command (all sources must be listed):

```bash
oscar64 -O3 -Ox -Op -tm=c64 -tf=prg -o=dist/main.prg \
    src/main.c \
    src/multidore64/renderlib.c \
    src/multidore64/soundlib.c \
    src/multidore64/soundlib_asm.c \
    src/multidore64/controllerlib.c \
    src/multidore64/utilslib.c \
    src/multidore64/filelib.c
# optional modules: append src/multidore64/render3d.c and/or src/multidore64/netlib.c
```

- Output flag REQUIRES `=`: `-o=FILE` works, `-o FILE` fails.
- `-tm=c64 -tf=prg` always. Output: `dist/main.prg` and `dist/main.d64`.
- Repo layout: `src/main.c` (your code), `src/multidore64/*` (engine), `src/song.bin` (music). Includes use `#include "multidore64/NAME.h"` (paths relative to `src/`).
- Or run `./compile.sh` / `compile.bat` (bundles `main.prg` + `song.bin` into `dist/main.d64`; pass extra files as arguments: `./compile.sh extra.prg:extra data.bin:data`).
- Test with VICE: `x64 -autostart dist/main.d64` or `x64 dist/main.prg`.

## 2. Program skeleton (mandatory structure)

```c
#include <stdio.h>
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    renderlib_init();          // ALWAYS first: sets up VIC-II text mode

    // ... your setup ...

    while (1)                  // ALWAYS end with infinite loop:
    {                          // returning from main drops to BASIC
        // game loop: input -> logic -> soundlib_update() if music -> draw
    }
}
```

Init order: `renderlib_init()` → `soundlib_init()` → `controller_init()` → (`render3d_init()` if 3D).

## 3. renderlib — display, drawing, text, sprites

`#include "multidore64/renderlib.h"`

Modes (`renderlib_setmode(mode)`, after init):

| Constant | Resolution | Use |
|---|---|---|
| `RMODE_TEXT` (default) | 40x25 chars | text/UI |
| `RMODE_HIRES` | 320x200 px | 2D bitmap, render3d polygons |
| `RMODE_MC_CHAR` | 40x25 | multicolor chars |
| `RMODE_MC_BITMAP` | 160x200 px | multicolor bitmap |
| `RMODE_ECM` | 40x25 | extended bg color |

Dimensions: `RMODE_TEXT_W/H`=40/25, `RMODE_HIRES_W/H`=320/200, `RMODE_MC_W/H`=160/200; runtime `renderlib_screen_w()/h()`. Palette = 16 entries (`RPALETTE_SIZE`); sprites = 8 (`RSPIRTE_COUNT`).

Text (character modes; x=0-39 column, y=0-24 row):

```c
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, const char *str);
void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c);
unsigned char renderlib_getchar(unsigned char x, unsigned char y);
void renderlib_setcharset(const unsigned char *data);  // 2048 bytes
```

Primitives (bitmap modes; pixel coords, multicolor halves x-resolution):

```c
void renderlib_plot(unsigned char x, unsigned char y, unsigned char color);
unsigned char renderlib_getpixel(unsigned char x, unsigned char y);
void renderlib_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color);
void renderlib_rect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color);       // outline
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color);
void renderlib_circle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);
void renderlib_fillcircle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);
void renderlib_ellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color);
void renderlib_fillellipse(unsigned char cx, unsigned char cy, unsigned char rx, unsigned char ry, unsigned char color);
void renderlib_polygon(const struct RPoint *pts, unsigned char n, unsigned char color);       // struct RPoint { unsigned char x, y; }
void renderlib_fillpolygon(const struct RPoint *pts, unsigned char n, unsigned char color);
void renderlib_blit(unsigned char x, unsigned char y, unsigned char w, unsigned char h, const unsigned char *data); // 1-bpp bitmap
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor);
void renderlib_invert(unsigned char x, unsigned char y, unsigned char w, unsigned char h);
void renderlib_copy(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char w, unsigned char h);
```

Sprites (hardware, 24x21 px, index n = 0-7):

```c
void renderlib_sprite_enable(unsigned char n, unsigned char enabled);        // 1/0
void renderlib_sprite_pos(unsigned char n, unsigned char x, unsigned char y); // x 0-311(ish), y 0-229
void renderlib_sprite_color(unsigned char n, unsigned char color);
void renderlib_sprite_multicolor(unsigned char n, unsigned char enabled);
void renderlib_sprite_expand(unsigned char n, unsigned char x2, unsigned char y2); // 1=2x stretch
void renderlib_sprite_data(unsigned char n, unsigned char pointer);          // VIC pointer: (offset of 64-byte block in VIC bank)/64 — NOT a C pointer
void renderlib_sprite_all_enable(unsigned char enabled);
```

Screen/palette:

```c
void renderlib_clear(unsigned char color);
void renderlib_setbg(unsigned char color);
void renderlib_setborder(unsigned char color);
void renderlib_setcolor(unsigned char background, unsigned char foreground);
void renderlib_setpalette(unsigned char index, unsigned char color);
unsigned char renderlib_getpalette(unsigned char index);
void renderlib_scroll(unsigned char dir);              // 0=up 1=down 2=left 3=right
void renderlib_toggle_rendering(unsigned char state);  // 0=blank screen for tear-free bulk update, 1=resume
unsigned char renderlib_getmode(void);
void renderlib_unload(void);
```

## 4. colorlib — palette constants

`#include "multidore64/colorlib.h"` — plain `unsigned char` constants:

`color_black`=0, `color_white`=1, `color_red`=2, `color_cyan`=3, `color_purple`=4, `color_green`=5, `color_blue`=6, `color_yellow`=7, `color_orange`=8, `color_brown`=9, `color_light_red`=10, `color_dark_grey`=11, `color_grey`=12, `color_light_green`=13, `color_light_blue`=14, `color_light_grey`=15.

Every `color` parameter in the engine takes one of these (or 0-15).

## 5. soundlib — PSID music

`#include "multidore64/soundlib.h"`. Decoupled: no tune embedded in the engine.

```c
void soundlib_init(void);
unsigned char soundlib_play(const char *tune, unsigned int len); // play buffer (raw or PSID)
unsigned char soundlib_play_file(const char *filename);          // load from disk & play
void soundlib_update(void);                                      // one play tick; call EVERY main-loop iteration
void soundlib_stop(void);                                        // silence; after this $4000-$453B is reusable
```

- `soundlib_update()` is NON-BLOCKING: it self-paces to 50 Hz via raster detection and returns immediately when no new frame started. Just call it unconditionally at the top of your loop.
- **NEVER call `soundlib_update()` (or anything touching the tune) from an interrupt/IRQ handler.** The tune keeps sequencer data in the CPU stack page ($0100-$01FF); IRQ frames corrupt it → machine crashes to BASIC. Main loop only.
- Calling it faster/slower than once per frame distorts tempo (it self-limits, so extra calls are harmless no-ops).
- While music plays, `$4000-$453B` is owned by the player — no other data there.
- Don't write SID registers for sound effects while the tune plays (the player rewrites them each frame). Stop music first if you need SID effects.
- `soundlib_stop()` only silences voices; only call `soundlib_update()` while music should play.

## 6. controllerlib — input

`#include "multidore64/controllerlib.h"`

```c
void controller_init(void);
unsigned char controller_ispressed(unsigned char button);                    // keyboard; key code
unsigned char controller_joy_up/down/left/right/fire(unsigned char port);    // port 0 or 1; 1 = held
unsigned char controller_joy_ispressed(unsigned char port, unsigned char button);
void controller_poll(unsigned char port);                                    // explicit refresh
```

Key codes: `0x20` space, `0x51` Q, `0x0D` return, `0x41`-`0x5A` = A-Z. All return 1 while held → poll every loop iteration.

## 7. render3d — software 3D

`#include "multidore64/render3d.h"`. Requires `render3d.c` in build + `RMODE_HIRES` + `renderlib_clear()` each frame. All coords 16-bit int; angles degrees 0-359; trig returns 8.8 fixed-point (÷256 for pixels, `R3D_DEG2FIX`=256).

```c
void render3d_init(void);                       // once: builds sin/cos tables
void render3d_cam_init(void);
void render3d_cam_pos(int x, int y, int z);
void render3d_cam_rot(int pitch, int yaw, int roll);
void render3d_cam_focal(int focal);             // ~256 default
struct Camera3D render3d_cam_get(void);
int render3d_sin(int angle); int render3d_cos(int angle);
void render3d_rot_x/y/z(struct V3 *v, int angle);          // struct V3 { int x, y, z; }
void render3d_transform(const struct V3 *world, struct V3 *cam_space, const struct Camera3D *cam);
unsigned char render3d_project(const struct V3 *cam_space, int *sx, int *sy, const struct Camera3D *cam); // 0 = behind camera
void render3d_line(int x1,int y1,int z1,int x2,int y2,int z2,unsigned char color);
void render3d_poly_outline(const int *verts, int n, unsigned char color);  // verts = flat x,y,z triples
void render3d_poly_fill(const int *verts, int n, unsigned char color);     // backface culled
void render3d_triangle(int x1,int y1,int z1,int x2,int y2,int z2,int x3,int y3,int z3,unsigned char color);
void render3d_box(int cx,int cy,int cz,int hx,int hy,int hz,unsigned char color);   // half-extents
void render3d_sphere(int cx,int cy,int cz,int r,unsigned char color);
void render3d_scene_clear(void); void render3d_scene_render(void);
```

Raycaster (first-person; map bytes 0=empty >0=wall; renders 320x200):

```c
void render3d_ray_init(void);
void render3d_ray_render(const unsigned char *map, int map_w, int map_h,
                         int player_x, int player_y, int player_dir);
```

Limits: `R3D_MAX_POLY`=8 verts, `R3D_MAX_OBJECTS`=32.

## 8. netlib — serial link (optional)

`#include "multidore64/netlib.h"`. Requires `netlib.c` in build. CIA2 serial.

```c
unsigned char netlib_init(unsigned int baud);          // NBAUD_300..NBAUD_19200; 1=ok
void netlib_shutdown(void);
void netlib_send(unsigned char b); unsigned char netlib_recv(void);   // recv blocks
unsigned char netlib_recv_timeout(unsigned int ms);    // 0xFF = timeout
unsigned char netlib_available(void);
void netlib_send_str(const char *s); unsigned int netlib_recv_str(char *buf, unsigned int max, unsigned int ms);
void netlib_send_data(const unsigned char *d, unsigned int len);
unsigned int netlib_recv_data(unsigned char *buf, unsigned int max, unsigned int ms);

// reliable packet layer (ACK/NAK, payload <= NPROT_MAX_DATA=250)
unsigned char netlib_connect(unsigned int ms);         // initiator
unsigned char netlib_accept(unsigned int ms);          // receiver
unsigned char netlib_connected(void); unsigned char netlib_state(void);   // NSTATE_DISCONNECTED/CONNECTING/CONNECTED
unsigned char netlib_send_packet(unsigned char type, const unsigned char *data, unsigned int len, unsigned int ms);
unsigned char netlib_recv_packet(unsigned char *buf, unsigned int max, unsigned int *out_len, unsigned int ms);
void netlib_send_input(unsigned char port, unsigned char buttons, unsigned char joy_x, unsigned char joy_y);
unsigned char netlib_recv_input(unsigned char *port, unsigned char *buttons, unsigned char *joy_x, unsigned char *joy_y, unsigned int ms);
void netlib_send_state(const unsigned char *state, unsigned int len);
unsigned char netlib_recv_state(unsigned char *state, unsigned int max, unsigned int *out_len, unsigned int ms);
void netlib_disconnect(void);
```

Packet types: `NPROT_DATA/INPUT/STATE/HANDSHAKE/ACK/NAK/DISCONNECT`.

## 9. utilslib

`void sleep(unsigned int frames);` — blocks N video frames (1 frame = 1/50 s PAL). Real time, raster-paced.

## 10. filelib — disk filesystem

`#include "multidore64/filelib.h"`. C64 1541 disk I/O via native KERNAL calls.

```c
void filelib_init(void);                                  // default device 8
void filelib_set_device(unsigned char device);            // 8, 9, 10, 11
unsigned char filelib_get_device(void);
int filelib_write(const char *fn, const char *buf, unsigned int len); // overwrite file, returns bytes / -1
int filelib_append(const char *fn, const char *buf, unsigned int len);// append to file, returns bytes / -1
int filelib_read(const char *fn, char *buf, unsigned int max_bytes);  // read into buf, returns bytes / -1
unsigned char filelib_exists(const char *fn);             // 1=exists, 0=not found
unsigned char filelib_delete(const char *fn);             // scratch file via channel 15; 1=ok
```

## 11. Memory map (know before placing data)

```
$0400-$07E7  screen RAM        $0800-$0FFF  program code/data (incl. SIDFILE)
$1000-$1FFF  charset area      $4000-$453B  music staging (reserved while playing)
$2000-$3FFF, $453C-$8FFF, $C000-$CFFF  free RAM
$9000-$9FFF  compiler soft stack
```

## 11. Hard rules (violate = broken program)

1. `renderlib_init()` before ANY other engine call.
2. Infinite loop at end of main — no return to BASIC.
3. Music: `soundlib_update()` from the main loop ONLY — never from an IRQ.
4. `-o=FILE` with equals sign.
5. `renderlib_sprite_data()` takes VIC pointer index (64-byte-block offset ÷ 64), not a C pointer.
6. One music tune at a time; `$4000-$453B` reserved while playing.
7. Don't hand-edit VIC-II registers ($D011/$D016/$D018, $DD00) — use renderlib.
8. Don't write SID registers while music plays.
9. Colors are always palette indices 0-15 (use colorlib constants).

## 12. Canonical examples

Title screen with music and input:

```c
#include <stdio.h>
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/colorlib.h"



int main(void)
{
    renderlib_init();
    soundlib_init();
    controller_init();

    renderlib_setborder(color_black);
    renderlib_setbg(color_purple);
    renderlib_drawstring(11, 10, color_white, "MULTIDORE 64");
    renderlib_drawstring(9, 14, color_yellow, "PRESS FIRE TO START");

    soundlib_play_file("song.bin");

    while (!controller_joy_fire(0) && !controller_ispressed(0x20))
        soundlib_update();

    soundlib_stop();
    renderlib_clear(color_black);

    // game state
    unsigned char px = 18, py = 20;

    while (1)
    {
        soundlib_update();                    // remove if no music in-game

        if (controller_joy_left(0)  && px > 0)  px--;
        if (controller_joy_right(0) && px < 39) px++;
        if (controller_joy_up(0)    && py > 1)  py--;
        if (controller_joy_down(0)  && py < 24) py++;

        renderlib_drawchar(px, py, color_yellow, '*');
    }
}
```

Raycaster demo:

```c
const unsigned char maze[8][8] = {
    {1,1,1,1,1,1,1,1}, {1,0,0,0,0,0,0,1}, {1,0,1,0,2,2,0,1}, {1,0,1,0,0,2,0,1},
    {1,0,0,0,0,0,0,1}, {1,2,2,0,1,0,0,1}, {1,0,0,0,1,0,0,1}, {1,1,1,1,1,1,1,1} };

renderlib_setmode(RMODE_HIRES);
render3d_init();
render3d_ray_init();
render3d_ray_render(&maze[0][0], 8, 8, 4*64, 4*64, 0);
```

Serial link (A connects, B accepts):

```c
// A: netlib_init(NBAUD_2400) && netlib_connect(5000)
// B: netlib_init(NBAUD_2400) && netlib_accept(5000)
```
