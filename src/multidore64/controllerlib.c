#include "controllerlib.h"
#include <conio.h>

// NOTE: Port 1 is 0 and port 2 is 1. It starts at 0 not 1!

void controller_init(void)
{
    joy_install(joy_static_stddrv);
}

unsigned char controller_joy_up(unsigned char port)
{
    return (joy_read(port) & 0x01);
}

unsigned char controller_joy_down(unsigned char port)
{
    return (joy_read(port) & 0x02);
}

unsigned char controller_joy_left(unsigned char port)
{
    return (joy_read(port) & 0x04);
}

unsigned char controller_joy_right(unsigned char port)
{
    return (joy_read(port) & 0x08);
}

unsigned char controller_joy_fire(unsigned char port)
{
    return (joy_read(port) & 0x10);
}

unsigned char controller_joy_ispressed(unsigned char port, unsigned char button)
{
    return (joy_read(port) & button);
}

unsigned char controller_ispressed(unsigned char button)
{
    // Check if button is pressed on keyboard
    return (kbhit() && (cgetc() == button));
}