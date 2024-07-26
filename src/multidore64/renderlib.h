/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#ifndef RENDERLIB_H
#define RENDERLIB_H

#include "config.h"

unsigned short get_renderlib_screen_width();
unsigned short get_renderlib_screen_height();

void renderlib_init();   // Initialize the renderlib
void renderlib_unload(); // Unload the renderlib
void renderlib_clear();  // Clear the screen
void renderlib_setcolor(
    unsigned char background,
    unsigned char foreground); // Set a background/foreground color
void renderlib_setpixel(unsigned char x, unsigned char y,
                        unsigned char color); // Set a pixel
unsigned char renderlib_getpixel(unsigned char x,
                                 unsigned char y); // Get a pixel
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color,
                         unsigned char stopColor); // Floodfill a color
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX,
                          unsigned char *outY); // Find the center of a shape

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color,
                        unsigned char c); // Draw a character
unsigned char renderlib_getchar(unsigned char x,
                                unsigned char y); // Get a drawn character
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color,
                          char *str); // Draw a string

void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2,
                        unsigned char y2, unsigned char thickness,
                        unsigned char color); // Draw a line
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w,
                        unsigned char h,
                        unsigned char color); // Fill a rectangle
void renderlib_fillcircle(unsigned char x, unsigned char y, unsigned char r,
                          unsigned char color); // Fill a sphere
void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w,
                          unsigned char h,
                          unsigned char *sprite); // Draw a sprite

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
void renderlib_setmode(unsigned char state);
void renderlib_draw();
#endif

#ifdef RENDERLIB3D_INCLUDED
struct renderlib3d_vector3 {
  int x;
  int y;
  int z;
};

/*
SHAPES:
0 = Cube
1 = Pyramid (WIP)
2 = Sphere (WIP)
3 = Cylinder (WIP)
4 = Cone (WIP)
5 = Plane (WIP)
*/
struct renderlib3d_object {
  unsigned char shape;
  unsigned char id; // MAKE SURE TO ADJUST WITH MAX_OBJECTS.
  unsigned char color;
  /*
  // WIP
  unsigned char fillColor;
  unsigned char lineColor;
  */
  struct renderlib3d_vector3 *pos;
  struct renderlib3d_vector3 *rot;
  struct renderlib3d_vector3 *scl;
};

struct renderlib3d_camera {
  struct renderlib3d_vector3 *pos;
  struct renderlib3d_vector3 *rot;
  struct renderlib3d_vector3 *scl;
};

void renderlib3d_setcam(struct renderlib3d_camera *cam);
struct renderlib3d_camera *renderlib3d_getcam();

void renderlib3d_clear();
void renderlib3d_reset();
void renderlib3d_init();

struct renderlib3d_object *renderlib3d_createobj(unsigned char shape);
unsigned char renderlib3d_addobj(struct renderlib3d_object *obj);

void renderlib3d_render();
#endif

#endif