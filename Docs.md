# MultiDore64 Library Documentation

This documentation provides an overview of the MultiDore 64 library, detailing the functions and constants available in the various header files.

---

## `multidore64/colorlib.h`

This header file defines constants for various colors used in the MultiDore 64 engine.

### Color Constants

```c
const unsigned char color_black       = 0x00;
const unsigned char color_white       = 0x01;
const unsigned char color_red         = 0x02;
const unsigned char color_cyan        = 0x03;
const unsigned char color_purple      = 0x04;
const unsigned char color_green       = 0x05;
const unsigned char color_blue        = 0x06;
const unsigned char color_yellow      = 0x07;
const unsigned char color_orange      = 0x08;
const unsigned char color_brown       = 0x09;
const unsigned char color_light_red   = 0x0A;
const unsigned char color_dark_grey   = 0x0B;
const unsigned char color_grey        = 0x0C;
const unsigned char color_light_green = 0x0D;
const unsigned char color_light_blue  = 0x0E;
const unsigned char color_light_grey  = 0x0F;
```

### Example Usage

```c
renderlib_setcolor(color_black, color_white);
```

---

## `multidore64/controllerlib.h`

This header file provides functions for initializing and handling controller input.

### Functions

```c
void controller_init(void);
unsigned char controller_joy_up(unsigned char port);
unsigned char controller_joy_down(unsigned char port);
unsigned char controller_joy_left(unsigned char port);
unsigned char controller_joy_right(unsigned char port);
unsigned char controller_joy_fire(unsigned char port);
unsigned char controller_joy_ispressed(unsigned char port, unsigned char button);
unsigned char controller_ispressed(unsigned char button);
```

### Example Usage

```c
controller_init();
if (controller_joy_up(0)) {
    // Move player up
}
```

---

## `multidore64/renderlib.h`

This header file provides functions for rendering graphics on the screen.

### Functions

```c
void renderlib_init(); 
void renderlib_unload(); 
void renderlib_clear(); 
void renderlib_setcolor(unsigned char background, unsigned char foreground); 
void renderlib_setpixel(unsigned char x, unsigned char y, unsigned char color); 
unsigned char renderlib_getpixel(unsigned char x, unsigned char y); 
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor); 
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX, unsigned char *outY); 
void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c); 
unsigned char renderlib_getchar(unsigned char x, unsigned char y); 
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, char* str); 
void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char thickness, unsigned char color); 
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color); 
void renderlib_fillsphere(unsigned char x, unsigned char y, unsigned char r, unsigned char color); 
void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char* sprite); 

#ifdef RENDERLIB3D_INCLUDED
void renderlib3d_setcpos(int x, int y, int z);
void renderlib3d_setcrot(int x, int y, int z);
void renderlib3d_setcscl(int x, int y, int z);

void renderlib3d_clear();
void renderlib3d_reset();
void renderlib3d_init();

unsigned char renderlib3d_draw(unsigned char shape, int posx, int posy, int posz, int rotx, int roty, int rotz, int sclx, int scly, int sclz, unsigned char color);

void renderlib3d_setpos(unsigned char obj, int x, int y, int z);
void renderlib3d_setrot(unsigned char obj, int x, int y, int z);
void renderlib3d_setscl(unsigned char obj, int x, int y, int z);
void renderlib3d_setshape(unsigned char obj, unsigned char shape);

void renderlib3d_render();
#endif
```

### Example Usage

```c
renderlib_init();
renderlib_clear();
renderlib_setcolor(color_black, color_white);
renderlib_drawstring(10, 10, color_red, "Hello, World!");
renderlib_unload();
```

---

## `multidore64/soundlib.h`

This header file provides functions for initializing and controlling sound playback.

### Functions

```c
void soundlib_init();
void soundlib_play(char FILEDATA[]);
void soundlib_stop();
```

### Example Usage

```c
soundlib_init();
soundlib_play(SIDFILE);
soundlib_stop();
```

---

## `multidore64/utilslib.h`

This header file provides utility functions.

### Functions

```c
void sleep(unsigned int ns);
int cos(int angle);
int sin(int angle);
```

### Example Usage

```c
sleep(1000);
int cosine = cos(45);
int sine = sin(45);
```

---

## Full Example Program

Here's a complete example program demonstrating the use of the MultiDore 64 library:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multidore64/renderlib.h"
#include "multidore64/soundlib.h"
#include "multidore64/colorlib.h"
#include "multidore64/controllerlib.h"
#include "multidore64/utilslib.h"

extern char SIDFILE[];

int main(void) {
    renderlib_init();
    soundlib_init();
    controller_init();

    // Display a start message
    renderlib_drawstring(10, 10, color_white, "Press fire to start");
    soundlib_play(SIDFILE);

    // Wait for user to press fire button
    while (!controller_joy_fire(0)) { }

    soundlib_stop();
    renderlib_clear();

    // Main game loop
    while (1) {
        // Handle player input and game logic here

        // Example: Draw a pixel and sleep for a short duration
        renderlib_setpixel(10, 10, color_red);
        sleep(500);

        // Check if the player wants to exit (e.g., by pressing a specific button)
        if (controller_ispressed(0x01)) {
            break;
        }
    }

    renderlib_unload();
    return EXIT_SUCCESS;
}
```

This example initializes the libraries, displays a start message, waits for user input, and enters a main game loop where it handles input and game logic.