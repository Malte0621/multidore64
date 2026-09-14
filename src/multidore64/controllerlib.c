#include "controllerlib.h"
#include <conio.h>

// NOTE: Port 1 is 0 and port 2 is 1. It starts at 0 not 1!

void controller_init(void)
{
    // oscar64 reads the joystick on demand via joy_poll(); there is no
    // driver to install the way cc65 required joy_install().
}

/* Poll the joystick once per frame. Call this before reading direction
   state. The individual direction functions read from the cached
   joyx[]/joyy[]/joyb[] globals without re-polling hardware. */
void controller_poll(unsigned char port)
{
    joy_poll(port);
}

unsigned char controller_joy_up(unsigned char port)
{
    return (joyy[port] == -1);
}

unsigned char controller_joy_down(unsigned char port)
{
    return (joyy[port] == 1);
}

unsigned char controller_joy_left(unsigned char port)
{
    return (joyx[port] == -1);
}

unsigned char controller_joy_right(unsigned char port)
{
    return (joyx[port] == 1);
}

unsigned char controller_joy_fire(unsigned char port)
{
    return (joyb[port]);
}

unsigned char controller_joy_ispressed(unsigned char port, unsigned char button)
{
    joy_poll(port);
    if (button & 0x01) return (joyy[port] == -1);
    if (button & 0x02) return (joyy[port] == 1);
    if (button & 0x04) return (joyx[port] == -1);
    if (button & 0x08) return (joyx[port] == 1);
    if (button & 0x10) return (joyb[port]);
    return 0;
}

unsigned char controller_ispressed(unsigned char button)
{
    // Check if button is pressed on keyboard
    return (kbhit() && (getch() == button));
}