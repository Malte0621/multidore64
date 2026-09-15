# Input - controllerlib

`controllerlib` reads the keyboard and both joystick ports.

```c
#include "multidore64/controllerlib.h"
```

## API

```c
void controller_init(void);

/* Joysticks (port 0 = port 1 on the C64, port 1 = port 2) */
void controller_poll(unsigned char port);
unsigned char controller_joy_up(unsigned char port);
unsigned char controller_joy_down(unsigned char port);
unsigned char controller_joy_left(unsigned char port);
unsigned char controller_joy_right(unsigned char port);
unsigned char controller_joy_fire(unsigned char port);
unsigned char controller_joy_ispressed(unsigned char port, unsigned char button);

/* Keyboard */
unsigned char controller_ispressed(unsigned char button);
```

## Keyboard

`controller_ispressed()` takes a **PETSCII/scancode-style key code** and returns 1 while the key is down:

```c
controller_init();

if (controller_ispressed(0x20))       /* space */
    start_game();
if (controller_ispressed(0x51))       /* Q - quit */
    return 0;
```

Common codes used in the engine: `0x20` space, `0x51` Q, `0x0D` return, `0x41`-`0x5A` A-Z.

## Joysticks

```c
controller_init();

while (1)
{
    if (controller_joy_fire(0))
        shoot();

    if (controller_joy_left(0))
        player_x--;
    else if (controller_joy_right(0))
        player_x++;

    if (controller_joy_up(1))         /* second player */
        enemy_y--;
}
```

- Ports are `0` and `1` (C64 control ports 1 and 2).
- Direction and fire functions return 1 while held - poll every frame from your game loop.
- `controller_poll(port)` refreshes a port's state explicitly; the direction/fire helpers read the latched values.

## Complete example

```c
#include "multidore64/renderlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/colorlib.h"

int main(void)
{
    unsigned char x = 18, y = 12;

    renderlib_init();
    controller_init();
    renderlib_drawstring(0, 0, color_white, "MOVE WITH JOYSTICK 1");

    while (!controller_joy_fire(0))
    {
        if (controller_joy_left(0)  && x > 0)  x--;
        if (controller_joy_right(0) && x < 39) x++;
        if (controller_joy_up(0)    && y > 1)  y--;
        if (controller_joy_down(0)  && y < 24) y++;

        renderlib_drawchar(x, y, color_yellow, '*');
    }
    return 0;
}
```
