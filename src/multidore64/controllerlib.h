#include <joystick.h>

void controller_init(void);
unsigned char controller_joy_up(unsigned char port);
unsigned char controller_joy_down(unsigned char port);
unsigned char controller_joy_left(unsigned char port);
unsigned char controller_joy_right(unsigned char port);
unsigned char controller_joy_fire(unsigned char port);
unsigned char controller_joy_ispressed(unsigned char port, unsigned char button);
unsigned char controller_ispressed(unsigned char button);