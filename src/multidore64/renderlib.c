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
#include <cbm.h>
#include "config.h"
#include "renderlib.h"
#include <tgi.h>
// #include "colorlib.h"

#define BASE 0x3c00 // 8192

const unsigned char color_black1 = 0x00;
const unsigned char color_white1 = 0x01;
const unsigned char color_light_blue1 = 0x0E;
const unsigned char color_blue1 = 0x06;

unsigned short renderlib_screen_width = 40, renderlib_screen_height = 25;

char hasBeenInitialized = 0;
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
unsigned char renderMode = 0;

const unsigned char renderlib_mode_text = 0;
const unsigned char renderlib_mode_graphics = 1;

unsigned char drawPage = 0;
#endif

unsigned short get_renderlib_screen_width()
{
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        return tgi_getmaxx();
    }
#endif

    return renderlib_screen_width;
}

unsigned short get_renderlib_screen_height()
{
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        return tgi_getmaxy();
    }
#endif
    return renderlib_screen_height;
}

#define TABLE_SIZE 360
int cos_table[TABLE_SIZE] = {1000, 999, 998, 997, 995, 994, 992, 990, 988, 986, 984, 982, 980, 978, 975, 973, 970, 968, 965, 962, 960, 957, 954, 951, 948, 945, 942, 939, 936, 933, 930, 927, 923, 920, 917, 913, 910, 906, 903, 899, 896, 892, 888, 885, 881, 877, 873, 869, 865, 861, 857, 853, 849, 845, 841, 837, 833, 829, 825, 821, 817, 813, 808, 804, 800, 796, 791, 787, 783, 778, 774, 769, 765, 760, 756, 751, 747, 742, 738, 733, 729, 724, 719, 715, 710, 705, 701, 696, 691, 687, 682, 677, 672, 668, 663, 658, 653, 648, 643, 639, 634, 629, 624, 619, 614, 609, 604, 599, 594, 589, 584, 579, 574, 569, 564, 559, 554, 549, 544, 539, 534, 529, 524, 519, 514, 509, 504, 499, 494, 489, 484, 479, 474, 469, 464, 459, 454, 449, 444, 439, 434, 429, 424, 419, 414, 409, 404, 399, 394, 389, 384, 379, 374, 369, 364, 359, 354, 349, 344, 339, 334, 329, 324, 319, 314, 309, 304, 299, 294, 289, 284, 279, 274, 269, 264, 259, 254, 249, 244, 239, 234, 229, 224, 219, 214, 209, 204, 199, 194, 189, 184, 179, 174, 169, 164, 159, 154, 149, 144, 139, 134, 129, 124, 119, 114, 109, 104, 99, 94, 89, 84, 79, 74, 69, 64, 59, 54, 49, 44, 39, 34, 29, 24, 19, 14, 9, 4, 0, -4, -9, -14, -19, -24, -29, -34, -39, -44, -49, -54, -59, -64, -69, -74, -79, -84, -89, -94, -99, -104, -109, -114, -119, -124, -129, -134, -139, -144, -149, -154, -159, -164, -169, -174, -179, -184, -189, -194, -199, -204, -209, -214, -219, -224, -229, -234, -239, -244, -249, -254, -259, -264, -269, -274, -279, -284, -289, -294, -299, -304, -309, -314, -319, -324, -329, -334, -339, -344, -349, -354, -359, -364, -369, -374, -379, -384, -389, -394, -399, -404, -409, -414, -419, -424, -429, -434, -439, -444, -449, -454, -459, -464, -469, -474, -479, -484, -489, -494, -499, -504, -509, -514, -519, -524, -529, -534, -539, -544, -549, -554, -559, -564, -569, -574, -579, -584, -589, -594, -599, -604, -609, -614, -619, -624, -629, -634, -639, -643, -648, -653, -658};
int sin_table[TABLE_SIZE] = {0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 63, 69, 75, 81, 87, 93, 99, 105, 111, 117, 123, 128, 134, 140, 145, 151, 156, 162, 167, 172, 177, 182, 187, 192, 197, 202, 206, 211, 215, 219, 224, 228, 232, 235, 239, 243, 246, 250, 253, 256, 259, 262, 265, 267, 270, 272, 274, 276, 278, 280, 282, 283, 285, 286, 287, 288, 289, 290, 290, 291, 291, 291, 291, 291, 291, 291, 290, 290, 289, 288, 287, 286, 285, 283, 282, 280, 278, 276, 274, 272, 270, 267, 265, 262, 259, 256, 253, 250, 246, 243, 239, 235, 232, 228, 224, 219, 215, 211, 206, 202, 197, 192, 187, 182, 177, 172, 167, 162, 156, 151, 145, 140, 134, 128, 123, 117, 111, 105, 99, 93, 87, 81, 75, 69, 63, 56, 50, 44, 38, 31, 25, 19, 13, 6, 0, -6, -13, -19, -25, -31, -38, -44, -50, -56, -63, -69, -75, -81, -87, -93, -99, -105, -111, -117, -123, -128, -134, -140, -145, -151, -156, -162, -167, -172, -177, -182, -187, -192, -197, -202, -206, -211, -215, -219, -224, -228, -232, -235, -239, -243, -246, -250, -253, -256, -259, -262, -265, -267, -270, -272, -274, -276, -278, -280, -282, -283, -285, -286, -287, -288, -289, -290, -290, -291, -291, -291, -291, -291, -291, -291, -290, -290, -289, -288, -287, -286, -285, -283, -282, -280, -278, -276, -274, -272, -270, -267, -265, -262, -259, -256, -253, -250, -246, -243, -239, -235, -232, -228, -224, -219, -215, -211, -206, -202, -197, -192, -187, -182, -177, -172, -167, -162, -156, -151, -145, -140, -134, -128, -123, -117, -111, -105, -99, -93, -87, -81, -75, -69, -63, -56, -50, -44, -38, -31, -25, -19, -13, -6, 0, 6, 13, 19, 25, 31, 38, 44, 50, 56, 63, 69, 75, 81, 87, 93, 99, 105, 111, 117, 123, 128, 134, 140, 145, 151, 156, 162, 167, 172, 177, 182, 187, 192, 197, 202, 206, 211, 215, 219, 224, 228, 232, 235, 239, 243, 246, 250, 253, 256, 259, 262, 265, 267, 270, 272, 274, 276, 278, 280, 282, 283, 285, 286, 287, 288, 289, 290, 290, 291, 291, 291};

int cos(int angle)
{
    // Normalize the angle to be within the range of 0 to 359 degrees
    angle = angle % TABLE_SIZE;
    if (angle < 0)
    {
        angle += TABLE_SIZE;
    }

    // Return the cosine value from the lookup table
    return cos_table[angle];
}

int sin(int angle)
{
    // Normalize the angle to be within the range of 0 to 359 degrees
    angle = angle % TABLE_SIZE;
    if (angle < 0)
    {
        angle += TABLE_SIZE;
    }

    // Return the sine value from the lookup table
    return sin_table[angle];
}

void sleep(unsigned int ms)
{
    unsigned int i;
    for (i = 0; i < ms; i++)
    {
        __asm__("nop");
    }
}

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

void renderlib_clear()
{
    unsigned char color = PEEK(53280);
    if (hasBeenInitialized == 0)
    {
        printf("%c", 0x93);
        return;
    }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        tgi_clear();
    }
    else
    {
#endif

        renderlib_fillrect(0, 0, 39, 23, color);

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
}

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
void renderlib_draw()
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    if (renderMode == renderlib_mode_graphics)
    {
        drawPage = !drawPage;
        tgi_setdrawpage(drawPage);
        tgi_setviewpage(!drawPage);
    }
}

void renderlib_setmode(unsigned char state)
{
    if (state == 0)
    {
        // Revert to text mode
        tgi_uninstall();
        tgi_unload();

        renderlib_screen_width = 40;
        renderlib_screen_height = 25;
    }
    else
    {
        // Enter graphics mode
        tgi_install(tgi_static_stddrv);
        tgi_init();
        tgi_clear();

        tgi_setdrawpage(drawPage);
        tgi_setviewpage(!drawPage);

        // renderlib_setcolor(color_black1, color_white1);

        renderlib_screen_width = tgi_getxres();
        renderlib_screen_height = tgi_getyres();
    }
    renderMode = state;
}
#endif

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
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    unsigned char *c2 = malloc(2);
    c2[0] = c;
    c2[1] = '\0';
#endif

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

    // Set the color of the pixel at position (x, y) to the desired color
    POKE(55296 + y * 40 + x, color);

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Draw the character using pixels
        /*
        unsigned char i, j;
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                if ((font8x8_basic[c][i] >> j) & 1)
                {
                    renderlib_setpixel(x * 8 + j, y * 8 + i, color);
                }
            }
        }
        */
        tgi_setcolor(color);
        tgi_outtextxy(x, y, c2);
    }
    else
    {
#endif
        // Draw the character
        POKE(1024 + y * 40 + x, c);
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
}

unsigned char renderlib_getchar(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return 0;
    }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        msg("Cannot get character in graphics mode!");
        return 0;
    }
#endif

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

    if (x >= renderlib_screen_width || y >= renderlib_screen_height)
        return;

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Set the color of the pixel at position (x, y) to the desired color
        // POKE(55296 + y * 40 + x, color);
        tgi_setcolor(color);
        tgi_setpixel(x, y);
        return;
    }
    else
    {

#endif

        // Set the color of the pixel at position (x, y) to the desired color
        renderlib_drawchar(x, y, color, 224);

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
}

unsigned char renderlib_getpixel(unsigned char x, unsigned char y)
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return 0;
    }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Get the color of the pixel at position (x, y)
        return tgi_getpixel(x, y);
    }
    else
    {
#endif

        // Get the color of the pixel at position (x, y)
        return PEEK(55296 + y * 40 + x);
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
}

void renderlib_drawstring(unsigned char x, unsigned char y, unsigned char color, char *str)
{
    // Get the length of the string
    unsigned char len = strlen(str);
    // Loop through the string
    unsigned char i = 0;

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        tgi_setcolor(color);
        tgi_outtextxy(x, y, str);
        return;
    }
#endif

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

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Draw a filled rectangle
        tgi_setcolor(color);
        tgi_bar(x, y, x + w, y + h);
    }
    else
    {
#endif

        for (i = 0; i < w; i++)
        {
            for (j = 0; j < h; j++)
            {
                renderlib_setpixel(x + i, y + j, color);
            }
        }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
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

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Draw a line
        tgi_setcolor(color);
        tgi_line(x1, y1, x2, y2);
    }
    else
    {
#endif

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

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
}

void renderlib_fillcircle(unsigned char x, unsigned char y, unsigned char r, unsigned char color)
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

void renderlib_drawcirlce(unsigned char x, unsigned char y, unsigned char r, unsigned char color)
{
    unsigned char i = 0;
    unsigned char j = 0;
    int radiusSquared;

    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }

#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        // Draw a circle
        tgi_setcolor(color);
        tgi_circle(x, y, r);
    }
    else
    {
#endif

        radiusSquared = r * r;
        for (i = 0; i <= r; i++)
        {
            for (j = 0; j <= r; j++)
            {
                int distanceSquared = (i - r) * (i - r) + (j - r) * (j - r);
                if (distanceSquared >= radiusSquared - 2 * r && distanceSquared <= radiusSquared + 2 * r)
                {
                    renderlib_setpixel(x + i, y + j, color);
                    renderlib_setpixel(x - i, y + j, color);
                    renderlib_setpixel(x + i, y - j, color);
                    renderlib_setpixel(x - i, y - j, color);
                }
            }
        }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    }
#endif
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

void renderlib_unload()
{
    if (hasBeenInitialized == 0)
    {
        initErrorMsg();
        return;
    }
#ifdef RENDERLIB_GRAPHICSMODE_INCLUDED
    if (renderMode == renderlib_mode_graphics)
    {
        renderlib_setmode(renderlib_mode_text);
    }
#endif
    renderlib_setcolor(color_light_blue1, color_blue1);
    hasBeenInitialized = 0;
    renderlib_clear();
}

void renderlib_init()
{
    if (hasBeenInitialized == 1)
    {
        initErrorMsg2();
        return;
    }
    // renderlib_setmode(renderlib_mode_text);
    renderlib_clear();
    hasBeenInitialized = 1;
    renderlib_setcolor(color_black1, color_black1);
}

#ifdef RENDERLIB3D_INCLUDED
// RenderLib 3D Implementation //

int cpos[3] = {0, 0, 0};
int crot[3] = {0, 0, 0};
int cscl[3] = {5, 5, 15}; // 10 = 1m

void renderlib3d_setcpos(int x, int y, int z)
{
    cpos[0] = x;
    cpos[1] = y;
    cpos[2] = z;
}

void renderlib3d_setcrot(int x, int y, int z)
{
    crot[0] = x;
    crot[1] = y;
    crot[2] = z;
}

void renderlib3d_setcscl(int x, int y, int z)
{
    cscl[0] = x;
    cscl[1] = y;
    cscl[2] = z;
}

unsigned char cobj = 0;
#define MAX_OBJECTS 32
// unsigned char changeObjects[MAX_OBJECTS]; // Objects that have changed and need to be redrawn.
unsigned char redrawRequired = 0; // If 1, redraw all objects.
int objects[MAX_OBJECTS][11];     // TODO: Maybe increase or dynamiclly allocate somehow?

void renderlib3d_clear()
{
    unsigned char i;

    // Default values reset //
    cobj = 0;
    for (i = 0; i < MAX_OBJECTS; i++)
    {
        memset(objects[i], 0, 44); // objects[11] --> 11 --> 11 * 4 (int) = 44
    }

    redrawRequired = 1;
}

void renderlib3d_reset()
{
    renderlib3d_clear();
    renderlib3d_setcpos(0, 0, 0);
    renderlib3d_setcrot(0, 0, 0);
    renderlib3d_setcscl(5, 5, 15);
}

void renderlib3d_init()
{
    if (hasBeenInitialized == 0)
    {
        renderlib_init();
    }
    renderlib_setmode(renderlib_mode_graphics);

    renderlib3d_reset();
}

/*
SHAPES:
0 = Cube
1 = Pyramid (WIP)
2 = Sphere (WIP)
3 = Cylinder (WIP)
4 = Cone (WIP)
5 = Plane (WIP)
*/
unsigned char renderlib3d_draw(unsigned char shape, int posx, int posy, int posz, int rotx, int roty, int rotz, int sclx, int scly, int sclz, unsigned char color)
{
    int values[10];
    cobj++;
    values[0] = shape; // TODO: Can we reduce the amount of bytes used for this? (Maybe 1 byte for shape somehow? (instead of 4))
    values[1] = posx;
    values[2] = posy;
    values[3] = posz;
    values[4] = rotx;
    values[5] = roty;
    values[6] = rotz;
    values[7] = sclx;
    values[8] = scly;
    values[9] = sclz;
    values[10] = color;
    memcpy(objects[cobj], values, 9);
    return cobj;
}

void enqueue_redraw(unsigned char obj)
{
    redrawRequired = 1;
}

void renderlib3d_setpos(unsigned char obj, int x, int y, int z)
{
    objects[obj][1] = x;
    objects[obj][2] = y;
    objects[obj][3] = z;
    enqueue_redraw(obj);
}

void renderlib3d_setrot(unsigned char obj, int x, int y, int z)
{
    objects[obj][4] = x;
    objects[obj][5] = y;
    objects[obj][6] = z;
    enqueue_redraw(obj);
}

void renderlib3d_setscl(unsigned char obj, int x, int y, int z)
{
    objects[obj][7] = x;
    objects[obj][8] = y;
    objects[obj][9] = z;
    enqueue_redraw(obj);
}

void renderlib3d_setshape(unsigned char obj, unsigned char shape)
{
    objects[obj][0] = shape;
    enqueue_redraw(obj);
}

void renderlib3d_render()
{
    int *obj;
    unsigned char shape;
    int posx, posy, posz;
    int rotx, roty, rotz;
    int sclx, scly, sclz;
    unsigned char color;

    // Loop through all objects
    unsigned char i;

    if (!redrawRequired)
    {
        return;
    }

    for (i = 0; i < cobj; i++)
    {
        // Get the object
        obj = objects[i];
        // Get the shape
        shape = obj[0];
        // Get the position
        posx = obj[1];
        posy = obj[2];
        posz = obj[3];
        // Get the rotation
        rotx = obj[4];
        roty = obj[5];
        rotz = obj[6];
        // Get the scale
        sclx = obj[7];
        scly = obj[8];
        sclz = obj[9];
        // Get the color
        color = obj[10];

        // Draw it!
        switch (shape)
        {
        case 0: // Cube
            // void renderlib_drawline(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char thickness, unsigned char color)
            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            renderlib_drawline(posx, posy, posx + sclx, posy, 1, color);
            renderlib_drawline(posx, posy, posx, posy + scly, 1, color);
            renderlib_drawline(posx, posy + scly, posx + sclx, posy + scly, 1, color);
            renderlib_drawline(posx + sclx, posy, posx + sclx, posy + scly, 1, color);

            break;
        default:
            //
            break;
        }
    }
}

// End of section //
#endif