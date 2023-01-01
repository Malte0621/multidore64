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

void sleep(unsigned int ms);

void renderlib_init(); // Initialize the renderlib
void renderlib_unload(); // Unload the renderlib
void renderlib_clear(); // Clear the screen
void renderlib_setcolor(unsigned char background, unsigned char foreground); // Set a background/foreground color
void renderlib_setpixel(unsigned char x, unsigned char y, unsigned char color); // Set a pixel
unsigned char renderlib_getpixel(unsigned char x, unsigned char y); // Get a pixel
void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor); // Floodfill a color

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c); // Draw a character
unsigned char renderlib_getchar(unsigned char x, unsigned char y); // Get a drawn character
void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, char* str); // Draw a string

void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char thickness, unsigned char color); // Draw a line
void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color); // Fill a rectangle
void renderlib_fillsphere(unsigned char x, unsigned char y, unsigned char r, unsigned char color); // Fill a sphere
void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char* sprite); // Draw a sprite

#endif