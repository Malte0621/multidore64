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

unsigned short get_renderlib_screen_width();
unsigned short get_renderlib_screen_height();

void sleep(unsigned int ms);
int cos(int angle);
int sin(int angle);

void renderlib_init(); // Initialize the renderlib
void renderlib_unload(); // Unload the renderlib
void renderlib_clear(); // Clear the screen
void renderlib_setcolor(unsigned char background, unsigned char foreground); // Set a background/foreground color
void renderlib_setpixel(unsigned char x, unsigned char y, unsigned char color); // Set a pixel
unsigned char renderlib_getpixel(unsigned char x, unsigned char y); // Get a pixel
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor); // Floodfill a color
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX, unsigned char *outY); // Find the center of a shape

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c); // Draw a character
unsigned char renderlib_getchar(unsigned char x, unsigned char y); // Get a drawn character
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, char* str); // Draw a string

void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char thickness, unsigned char color); // Draw a line
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color); // Fill a rectangle
void renderlib_fillcircle(unsigned char x, unsigned char y, unsigned char r, unsigned char color); // Fill a sphere
void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char* sprite); // Draw a sprite

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
void renderlib_setmode(unsigned char state);
void renderlib_draw();
#endif

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

#endif