# Examples

Complete, runnable programs. Each one is a full `src/main.c` - paste, build with `./compile.sh`, run `dist/main.prg`.

## Hello, MultiDore 64

The smallest useful program: init the renderer, draw text, stay alive.

```c
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    renderlib_init();
    renderlib_setborder(color_black);
    renderlib_setbg(color_blue);
    renderlib_drawstring(11, 12, color_white, "HELLO MULTIDORE 64");

    while (1)
        ;
}
```

## Animated starfield (bitmap mode)

Random pixels in hires mode with a fading trail.

```c
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    unsigned int seed = 0xace1;

    renderlib_init();
    renderlib_setmode(RMODE_HIRES);
    renderlib_setborder(color_black);
    renderlib_clear(color_black);

    while (1)
    {
        for (unsigned char i = 0; i < 8; i++)
        {
            seed = seed * 25173 + 13849;
            renderlib_plot((seed >> 8) % 320, seed % 200,
                           (seed & 3) ? color_white : color_yellow);
        }
        renderlib_scroll(0);   // drift the sky upward
    }
}
```

## Title screen with music and input

The canonical game opening: prompt, music stepping in the loop, wait for fire, stop music.

```c
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/colorlib.h"

extern char SIDFILE[];

int main(void)
{
    renderlib_init();
    soundlib_init();
    controller_init();

    renderlib_setborder(color_black);
    renderlib_setbg(color_purple);
    renderlib_drawstring(11, 10, color_white, "MULTIDORE 64");
    renderlib_drawstring(9, 14, color_yellow, "PRESS FIRE TO START");

    soundlib_play(SIDFILE);

    while (!controller_joy_fire(0) && !controller_ispressed(0x20))
        soundlib_update();        // one music tick per frame, non-blocking

    soundlib_stop();
    renderlib_clear(0);
    return 0;
}
```

## Sprite player character

Move a hardware sprite with joystick 1, clamped to the screen.

```c
#include "multidore64/renderlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/colorlib.h"

const unsigned char hero[64] =
{
    0x00,0x18,0x00,  0x00,0x3c,0x00,  0x00,0x7e,0x00,
    0x00,0xdb,0x00,  0x00,0xff,0x00,  0x00,0x24,0x00,
    0x00,0x24,0x00,  0x00,0x24,0x00,  0x00,0x42,0x00,
    0x00,0x00,0x00,  0x00,0x00,0x00,  0x00,0x00,0x00,
    0x00,0x00,0x00,  0x00,0x00,0x00,  0x00,0x00,0x00,
    0x00,0x00,0x00,  0x00,0x00,0x00,  0x00,0x00,0x00,
    0x00,0x00,0x00,  0x00,0x00,0x00,  0x00,0x00,0x00,
    0x00,0x00,0x00,  0x00,0x00,0x00,  0x00,0x00,0x00,
};

int main(void)
{
    unsigned char x = 160, y = 120;

    renderlib_init();
    controller_init();

    renderlib_sprite_data(0, 200);          // VIC sprite pointer slot 200
    renderlib_sprite_color(0, color_light_green);
    renderlib_sprite_pos(0, x, y);
    renderlib_sprite_enable(0, 1);

    while (1)
    {
        if (controller_joy_left(0)  && x > 8)   x--;
        if (controller_joy_right(0) && x < 311) x++;
        if (controller_joy_up(0)    && y > 8)   y--;
        if (controller_joy_down(0)  && y < 229) y++;

        renderlib_sprite_pos(0, x, y);
    }
}
```

!!! note
    `renderlib_sprite_data()` expects the VIC pointer index (offset of the 64-byte sprite block inside the VIC bank, divided by 64), not a C pointer.

## Wireframe cube (render3d)

A spinning cube. Add `src/multidore64/render3d.c` to the build line.

```c
#include "multidore64/renderlib.h"
#include "multidore64/render3d.h"
#include "multidore64/colorlib.h"

int main(void)
{
    int yaw = 0;
    const int size = 60;

    renderlib_init();
    renderlib_setmode(RMODE_HIRES);
    render3d_init();
    render3d_cam_pos(0, 0, -300);
    render3d_cam_focal(256);

    while (1)
    {
        renderlib_clear(color_black);
        render3d_box(0, 0, 0, size, size, size, color_cyan);
        render3d_cam_rot(0, yaw, 0);
        yaw = (yaw + 3) & 0xff;
    }
}
```

## Two-player serial link (netlib)

Machine A connects, machine B accepts; both exchange a state byte. Add `src/multidore64/netlib.c` to the build line.

```c
#include "multidore64/renderlib.h"
#include "multidore64/netlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    unsigned char ok, state = 0;

    renderlib_init();
    ok = netlib_init(NBAUD_2400);

#ifdef HOST_A
    ok = ok && netlib_connect(5000);
#else
    ok = ok && netlib_accept(5000);
#endif

    renderlib_drawstring(0, 0, ok ? color_green : color_red,
                         ok ? "LINKED" : "NO PEER");

    while (netlib_connected())
    {
        state++;
        netlib_send_state(&state, 1);
        renderlib_drawchar(0, 2, color_white, '0' + (state & 0x0f));
    }
    netlib_disconnect();
    return 0;
}
```

Build machine A with `-DHOST_A` and machine B without it.

## Palette cycling

Reprogram the hardware color registers for a plasma-ish border effect.

```c
#include "multidore64/renderlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    unsigned char phase = 0;
    const unsigned char ramp[8] = {
        color_blue, color_purple, color_red, color_orange,
        color_yellow, color_orange, color_red, color_purple
    };

    renderlib_init();

    while (1)
    {
        for (unsigned char i = 0; i < 8; i++)
            renderlib_setpalette(i + 8, ramp[(i + phase) & 7]);
        phase++;
    }
}
```
