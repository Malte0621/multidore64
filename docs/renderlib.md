# Rendering - renderlib

`renderlib` drives the VIC-II. It supports five display modes, drawing primitives for bitmap modes, text output for character modes, the eight hardware sprites and full palette control.

```c
#include "multidore64/renderlib.h"
```

## Initialization

```c
void renderlib_init(void);
```

Call `renderlib_init()` once before anything else. It programs the VIC-II for standard text mode (40x25), clears the screen, installs the default charset and resets colors. Every program in these docs starts with it.

```c
void renderlib_setmode(unsigned char mode);
void renderlib_getmode(void);
void renderlib_unload(void);
```

| Mode | Constant | Resolution | Type |
|---|---|---|---|
| Text | `RMODE_TEXT` | 40x25 | character |
| Hires bitmap | `RMODE_HIRES` | 320x200 | bitmap |
| Multicolor char | `RMODE_MC_CHAR` | 40x25 | character |
| Multicolor bitmap | `RMODE_MC_BITMAP` | 160x200 | bitmap |
| Extended bg color | `RMODE_ECM` | 40x25 | character |

Screen dimensions per mode are available as `RMODE_TEXT_W/H`, `RMODE_HIRES_W/H` and `RMODE_MC_W/H`, or at runtime via `renderlib_screen_w()` / `renderlib_screen_h()`.

!!! note
    Switching modes changes how VIC-II registers `$D011/$D016/$D018` and the CIA2 bank bits are programmed. Always go through `renderlib_setmode()`, never poke the registers directly alongside the library.

## Clearing

```c
void renderlib_clear(unsigned char color);
```

Fills the whole screen with `color` (a [palette index](colorlib.html)). In bitmap modes every pixel is set; in text modes the screen is filled with spaces.

## Drawing primitives (bitmap modes)

`renderlib_plot` and the shape functions address pixels; in multicolor modes the horizontal resolution halves and each pixel doubles in width.

```c
void renderlib_plot(unsigned char x, unsigned char y, unsigned char color);
unsigned char renderlib_getpixel(unsigned char x, unsigned char y);

void renderlib_line(unsigned char x1, unsigned char y1,
                    unsigned char x2, unsigned char y2, unsigned char color);

void renderlib_rect(unsigned char x, unsigned char y,
                    unsigned char w, unsigned char h, unsigned char color);
void renderlib_fillrect(unsigned char x, unsigned char y,
                        unsigned char w, unsigned char h, unsigned char color);

void renderlib_circle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);
void renderlib_fillcircle(unsigned char cx, unsigned char cy, unsigned char r, unsigned char color);

void renderlib_ellipse(unsigned char cx, unsigned char cy,
                       unsigned char rx, unsigned char ry, unsigned char color);
void renderlib_fillellipse(unsigned char cx, unsigned char cy,
                           unsigned char rx, unsigned char ry, unsigned char color);

void renderlib_polygon(const struct RPoint *pts, unsigned char n, unsigned char color);
void renderlib_fillpolygon(const struct RPoint *pts, unsigned char n, unsigned char color);
```
A twinkling starfield, one pixel at a time:

```c
renderlib_fillrect(0, 0, 320, 200, color_black);

unsigned int seed = 0x2b4c;
for (unsigned char i = 0; i < 120; i++)
{
    seed = seed * 25173 + 13849;              // 16-bit LCG
    renderlib_plot((seed >> 8) % 320, seed % 200, color_white);
}
```

## Blitting and regions

```c
void renderlib_blit(unsigned char x, unsigned char y,
                    unsigned char w, unsigned char h, const unsigned char *data);

void renderlib_floodfill(unsigned char x, unsigned char y,
                         unsigned char color, unsigned char stopColor);
char renderlib_findcenter(unsigned char x, unsigned char y,
                          unsigned char *outX, unsigned char *outY);
void renderlib_invert(unsigned char x, unsigned char y, unsigned char w, unsigned char h);
void renderlib_copy(unsigned char x1, unsigned char y1,
                    unsigned char x2, unsigned char y2,
                    unsigned char w, unsigned char h);
```

- `renderlib_blit` copies a packed 1-bpp bitmap of `w x h` pixels to the screen - the way to draw a logo or a big font.
- `renderlib_floodfill` recolors a bounded region, stopping at any pixel of `stopColor`.
- `renderlib_copy` duplicates a screen region, e.g. for parallax tiles.

## Text (character modes)

```c
void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c);
unsigned char renderlib_getchar(unsigned char x, unsigned char y);
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, const char *str);
void renderlib_setcharset(const unsigned char *data);
```

Coordinates are **character cells**, 0-39 across and 0-24 down:

```c
renderlib_drawstring(0, 0, color_white, "SCORE 000000");
renderlib_drawchar(39, 24, color_yellow, '*');
```

`renderlib_setcharset()` installs a custom 2 KB charset (256 glyphs x 8 bytes). It must live in RAM visible to the VIC-II; see [memory map](memory.html).

## Hardware sprites

The VIC-II overlays eight 24x21 pixel sprites on top of the screen. MultiDore 64 numbers them 0-7 (`RSPIRTE_COUNT`).

```c
void renderlib_sprite_enable(unsigned char n, unsigned char enabled);
void renderlib_sprite_pos(unsigned char n, unsigned char x, unsigned char y);
void renderlib_sprite_color(unsigned char n, unsigned char color);
void renderlib_sprite_multicolor(unsigned char n, unsigned char enabled);
void renderlib_sprite_expand(unsigned char n, unsigned char x2, unsigned char y2);
void renderlib_sprite_data(unsigned char n, unsigned char pointer);
void renderlib_sprite_all_enable(unsigned char enabled);
```

A minimal sprite:

```c
// 21 x 3 = 63 bytes of sprite pixels, stored somewhere in RAM
const unsigned char ball[64] = {
    // ... rows of 3 bytes each ...
};

renderlib_sprite_data(0, (unsigned char)(((unsigned)ball >> 6) & 0xff)); // see below
renderlib_sprite_color(0, color_white);
renderlib_sprite_pos(0, 100, 100);
renderlib_sprite_enable(0, 1);
```

`renderlib_sprite_data(n, pointer)` takes the VIC-II **sprite pointer value** (0-255, one unit = 64 bytes within the current VIC bank), not a C pointer - divide your array's offset into the VIC bank by 64.

!!! tip
    Call `renderlib_sprite_enable(0, 0)` before reusing a sprite's data buffer to avoid one frame of garbage.

## Palette and colors

```c
void renderlib_setpalette(unsigned char index, unsigned char color);
unsigned char renderlib_getpalette(unsigned char index);
void renderlib_setbg(unsigned char color);
void renderlib_setborder(unsigned char color);
void renderlib_setcolor(unsigned char background, unsigned char foreground);
```

Colors are VIC-II palette indices 0-15 - see [Colors](colorlib.html) for the named constants (`color_red`, `color_light_blue`, ...).

## Screen operations

```c
void renderlib_scroll(unsigned char dir);       /* 0=up, 1=down, 2=left, 3=right */
void renderlib_toggle_rendering(unsigned char state);
```

`renderlib_scroll(dir)` shifts the whole screen one cell/pixel in a direction. `renderlib_toggle_rendering(0)` blanks the display (set DEN off) for tear-free bulk updates; turn it back on with `renderlib_toggle_rendering(1)`.

## Complete example

```c
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    renderlib_init();
    renderlib_setmode(RMODE_TEXT);

    renderlib_setborder(color_black);
    renderlib_setbg(color_blue);

    renderlib_drawstring(15,  8, color_white,  "MULTIDORE");
    renderlib_drawstring(13, 10, color_yellow, "PRESS ANY KEY");

    // sprite cursor
    renderlib_sprite_pos(0, 160, 150);
    renderlib_sprite_color(0, color_red);
    renderlib_sprite_enable(0, 1);

    while (1)
        ;
}
```

More in [Examples](examples.html).
