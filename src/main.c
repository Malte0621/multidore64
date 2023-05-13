/*
----------------------------------------------------------
This file is a part of MultiDore 64 and was made to
demonstrate the library in action.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#define RENDERLIB3D_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "renderlib.h"
#include "soundlib.h"
#include "colorlib.h"
#include "controllerlib.h"
#include <conio.h>

char max_x = 39, max_y = 24;

extern char SIDFILE[];

void replaceColor(unsigned char color, unsigned char color2)
{
    unsigned char x;
    unsigned char y;
    for (x = 0; x < 40; x++)
    {
        for (y = 0; y < 25; y++)
        {
            if (renderlib_getpixel(x, y) == color)
            {
                draw(x, y, color2);
            }
        }
    }
}

void findAndFloodFill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor)
{
    unsigned char cx;
    unsigned char cy;
    // Get the center of the shape
    if (renderlib_findcenter(x, y, &cx, &cy))
    {
        // Flood fill the shape
        renderlib_floodfill(cx, cy, color, stopColor);
    }
    replaceColor(stopColor, color);
}

void handleInput(unsigned char port)
{
}

int main(void)
{
    char debug = 1;
    int timeLeft = 240;
    int i;
    int j;
    long ticks;

    renderlib_init();
    soundlib_init();
    controller_init();

    sleep(50);
    renderlib_drawstring(max_x / 2 - 16, max_y / 2, color_white, "press space/fire button to start");
    soundlib_play(SIDFILE);
    while (1)
    {
        if (controller_ispressed(0x20) || controller_joy_fire(0) || controller_joy_fire(1))
        {
            break;
        }
    }
    soundlib_stop();
    renderlib_clear();
    
    renderlib3d_init();

    // Game Loop
    while (1)
    {
        if (debug)
        {
            char *timeLeftStr = malloc(24);
            char *p1state = malloc(20);
            char *p1nd = malloc(16);

            sprintf(timeLeftStr, "time left: %i", timeLeft);
            renderlib_drawstring(0, 1, color_white, timeLeftStr);
        }

        // check if the letter "Q" was pressed
        if (controller_ispressed(0x51)) // TODO: Correct this.
        {
            // if so, exit the program
            break;
        }

        // Handle Player Input
        handleInput(0);
        // handleInput(1);

        if (timeLeft > 0)
        {
            timeLeft--;
        }
        else
        {
            break;
        }

        sleep(1);
        ticks++;
    }
    renderlib_unload();
    return EXIT_SUCCESS;
}