/*
----------------------------------------------------------
This file is a part of MultiDore 64 and was made to
demonstrate the library in action.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "multidore64/include.h"
#include <conio.h>

unsigned short max_x, max_y;

extern char SIDFILE[];

void handleInput(unsigned char port)
{
}

int main()
{
    unsigned int i;
    unsigned int j;

    char debug = 1;
    int timeLeft = 240;
    long ticks;

    max_x = get_renderlib_screen_width() - 1;
    max_y = get_renderlib_screen_height() - 1;

    renderlib_init();
    soundlib_init();
    controller_init();

    sleep(50);
    /*
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
    */

    renderlib_setmode(1);

    // renderlib_fillrect(0, 0, 50, 20, color_red);
    // renderlib_fillcircle(get_renderlib_screen_width() / 2, get_renderlib_screen_height() / 2, 50, color_green);
    renderlib_drawstring(get_renderlib_screen_width() / 2, 0, color_blue, "Hello World!");
    renderlib_draw();

    while (1)
    {
        if (controller_ispressed(0x51)) // TODO: Correct this.
        {
            // if so, exit the program
            break;
        }
        sleep(1);
    }

    /*
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

        if (timeLeft > 0 && ticks % 750 == 0)
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
    */
    
    renderlib_unload();
    return EXIT_SUCCESS;
}