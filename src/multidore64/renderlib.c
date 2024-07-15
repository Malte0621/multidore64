/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#include <stdio.h>
#include <stdlib.h>
#include <c64.h>
#include <conio.h>
#include <string.h>
#include <peekpoke.h>
#include "renderlib.h"
#include "utilslib.h"

char hasBeenInitialized = 0;

unsigned char modeByte1;
unsigned char modeByte2;

void msg(char *msg)
{
    printf("[MD64]: %s", msg);
}

void initErrorMsg()
{
    msg("Renderlib has not been initialized!");
}

void initErrorMsg2()
{
    msg("Renderlib has already been initialized!");
}

void renderlib_clear(void)
{
    unsigned char color = PEEK(53280);
    if (hasBeenInitialized == 0)
    {
        printf("%c", 0x93);
        return;
    }
    renderlib_fillrect(0, 0, 39, 23, color);
}

void renderlib_toggle_rendering(char state)
{
    if (state == 0)
    {
        // Disable rendering
    }
    else
    {
        // Enable rendering
    }
}

void renderlib_setcolor(unsigned char background, unsigned char foreground)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    // Set the color of the pixel at position (x, y) to the desired color
    POKE(53280, background);
    POKE(53281, foreground);
}

void renderlib_drawchar(unsigned char x, unsigned char y, unsigned char color, unsigned char c)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    // Set the color of the pixel at position (x, y) to the desired color
    POKE(55296 + y * 40 + x, color);
    // Draw the character
    POKE(1024 + y * 40 + x, c);
}

unsigned char renderlib_getchar(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return 0;
    }

    // Get the character at position (x, y)
    return PEEK(1024 + y * 40 + x);
}

void renderlib_setpixel(unsigned char x, unsigned char y, unsigned char color)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    // Set the color of the pixel at position (x, y) to the desired color
    renderlib_drawchar(x, y, color, 224);
}

unsigned char renderlib_getpixel(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return 0;
    }

    // Get the color of the pixel at position (x, y)
    return PEEK(55296 + y * 40 + x);
}

void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, char *str)
{
    // Get the length of the string
    unsigned char len = strlen(str);
    // Loop through the string
    unsigned char i = 0;
    while (i < len)
    {
        // Draw the character
        renderlib_drawchar(x + i, y, color, str[i]);
        i++;
    }
}

void renderlib_floodfill(unsigned char x, unsigned char y, unsigned char color, unsigned char stopColor)
{
    // Get the shape around x, y
    // Fill outwards until we reach the edge of the shape

    // Get the color of the pixel at position (x, y)
    unsigned char currentColor = renderlib_getpixel(x, y);

    // Check if the color is the same as the stop color
    if (currentColor == stopColor)
    {
        // Return
        return;
    }

    // Set the color of the pixel at position (x, y) to the desired color
    renderlib_setpixel(x, y, color);

    // Check if the pixel is on the left edge
    if (x > 0)
    {
        // Fill the pixel to the left
        renderlib_floodfill(x - 1, y, color, stopColor);
    }

    // Check if the pixel is on the right edge
    if (x < 40)
    {
        // Fill the pixel to the right
        renderlib_floodfill(x + 1, y, color, stopColor);
    }

    // Check if the pixel is on the top edge
    if (y > 0)
    {
        // Fill the pixel above
        renderlib_floodfill(x, y - 1, color, stopColor);
    }

    // Check if the pixel is on the bottom edge
    if (y < 25)
    {
        // Fill the pixel below
        renderlib_floodfill(x, y + 1, color, stopColor);
    }
}

// Find the center of a filled shape using one pixel from the edge of it.
char renderlib_findcenter(unsigned char x, unsigned char y, unsigned char *outX, unsigned char *outY)
{
    unsigned char diff;
    unsigned char tmp;
    char useTemp;
    unsigned char i;
    unsigned char currentColor = renderlib_getpixel(x, y);

    for (i = x; i < 40; i++)
    {
        // Check if the color of the pixel at position (i, j) is the same as the current color
        if (renderlib_getpixel(i, y) == currentColor)
        {
            break;
        }
        else
        {
            diff++;
        }
    }
    tmp = diff;
    useTemp = 1;
    for (i = x; i > 0; i--)
    {
        // Check if the color of the pixel at position (i, j) is the same as the current color
        if (renderlib_getpixel(i, y) == currentColor)
        {
            break;
        }
        else
        {
            if (tmp <= 0)
            {
                return 0;
            }
            else
            {
                tmp--;
            }
        }
    }
    (*outX) = x + diff - tmp;
    // Do the same for the y axis
    diff = 0;
    tmp = 0;
    useTemp = 1;
    for (i = y; i < 25; i++)
    {
        // Check if the color of the pixel at position (i, j) is the same as the current color
        if (renderlib_getpixel(x, i) == currentColor)
        {
            break;
        }
        else
        {
            diff++;
        }
    }
    tmp = diff;
    for (i = y; i > 0; i--)
    {
        // Check if the color of the pixel at position (i, j) is the same as the current color
        if (renderlib_getpixel(x, i) == currentColor)
        {
            break;
        }
        else
        {
            if (tmp <= 0)
            {
                return 0;
            }
            else
            {
                tmp--;
            }
        }
    }
    (*outY) = y + diff - tmp;
    return 1;
}

void renderlib_fillrect(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char color)
{
    unsigned char i = 0;
    unsigned char j = 0;

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    for (i = 0; i < w; i++)
    {
        for (j = 0; j < h; j++)
        {
            renderlib_setpixel(x + i, y + j, color);
        }
    }
}

void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char thickness, unsigned char color)
{
    // Calculate the difference between the two points
    unsigned char dx = x2 - x1;
    unsigned char dy = y2 - y1;

    // Calculate the slope without using a float (as it is not supported by CC65)
    unsigned char m = dy / dx;

    // Calculate the thickness
    unsigned char t = thickness / 2;

    // Calculate the thickness
    unsigned char i = 0;
    unsigned char j = 0;

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    // Loop through the thickness
    for (i = 0; i < t; i++)
    {
        // Loop through the x-axis
        for (j = 0; j < dx; j++)
        {
            // Calculate the y-axis
            unsigned char y = m * j + y1;
            // Draw the pixel
            renderlib_setpixel(x1 + j, y + i, color);
            renderlib_setpixel(x1 + j, y - i, color);
        }
    }
}

void renderlib_fillsphere(unsigned char x, unsigned char y, unsigned char r, unsigned char color)
{
    unsigned char i = 0;
    unsigned char j = 0;

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    for (i = 0; i < r; i++)
    {
        for (j = 0; j < r; j++)
        {
            if (i * i + j * j <= r * r)
            {
                renderlib_setpixel(x + i, y + j, color);
                renderlib_setpixel(x - i, y + j, color);
                renderlib_setpixel(x + i, y - j, color);
                renderlib_setpixel(x - i, y - j, color);
            }
        }
    }
}

void renderlib_drawsprite(unsigned char x, unsigned char y, unsigned char w, unsigned char h, unsigned char *sprite)
{
    unsigned char i = 0;
    unsigned char j = 0;

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    for (i = 0; i < w; i++)
    {
        for (j = 0; j < h; j++)
        {
            renderlib_setpixel(x + i, y + j, sprite[i + j * w]);
        }
    }
}

void renderlib_unload(void)
{
    if (hasBeenInitialized == 0){
        initErrorMsg();
        return;
    }
    renderlib_setcolor(0x0E, 0x06);
    hasBeenInitialized = 0;
    renderlib_clear();
}

void renderlib_init(void)
{
    if (hasBeenInitialized == 1){
        initErrorMsg2();
        return;
    }
    renderlib_clear();
    hasBeenInitialized = 1;
    renderlib_setcolor(0, 0);
}