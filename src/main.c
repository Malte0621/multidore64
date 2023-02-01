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
#include "renderlib.h"
#include "soundlib.h"
#include "colorlib.h"
#include "controllerlib.h"
#include <conio.h>

char max_x = 39, max_y = 24;

char p1_x = 0, p1_y = 0,
     p2_x = 0, p2_y = 0;

char p1_lastDir = 0, p2_lastDir = 0;
char p1_bo = 0, p2_bo = 0; // Black Over (How many times the player has gone over a black pixel)
char p1_nd = 0, p2_nd = 0; // Near Death (When the player is near death and has a last chance to survive)

char nearDeathSaveTicks = 2; // how many ticks the player has to survive after being near death

// Make a table where old colors are stored

unsigned char map[40][25];

unsigned char player1_color = 0x02;
unsigned char player1_character_color = 0x0A;
unsigned char player2_color = 0x06;
unsigned char player2_character_color = 0x0E;

extern char SIDFILE[];

void draw(unsigned char x, unsigned char y, unsigned char color)
{
    if (color != player1_character_color && color != player2_character_color)
        map[x][y] = color;
    renderlib_setpixel(x, y, color);
}

unsigned char isInColor(unsigned char x, unsigned char y, unsigned char color)
{
    return (map[x][y] == color);
}

unsigned char isCollidingWith(unsigned char x, unsigned char y, unsigned char color)
{
    return renderlib_getpixel(x, y) == color;
}

void resetGame()
{
    unsigned char x;
    unsigned char y;
    for (x = 0; x < 40; x++)
    {
        for (y = 0; y < 25; y++)
        {
            map[x][y] = 0;
        }
    }
    p1_x = 0;
    p1_y = 0;
    p2_x = 0;
    p2_y = 0;
    p1_lastDir = 0;
    p2_lastDir = 0;
    renderlib_clear();
}

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

void restoreFromMap(unsigned char color)
{
    unsigned char x;
    unsigned char y;
    unsigned char ocolor;
    unsigned char temp;
    for (x = 0; x < 40; x++)
    {
        for (y = 0; y < 25; y++)
        {
            ocolor = map[x][y];
            temp = renderlib_getpixel(x, y);
            if (temp == color)
            {
                draw(x, y, ocolor);
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

void respawn(unsigned char port, unsigned char color)
{
    if (port == 0)
    {
        draw(p1_x, p1_y, map[p1_x][p1_y]);
        replaceColor(player1_color, color);
        restoreFromMap(player1_character_color);
        p1_x = rand() % 37;
        p1_y = rand() % 22;
        while (p1_x == p2_x && p1_y == p2_y)
        {
            p1_x = rand() % 37;
            p1_y = rand() % 22;
        }
        // Draw A 3X3 square around the player
        draw(p1_x, p1_y, player1_color);
        draw(p1_x + 1, p1_y, player1_color);
        draw(p1_x - 1, p1_y, player1_color);
        draw(p1_x, p1_y + 1, player1_color);
        draw(p1_x, p1_y - 1, player1_color);
        draw(p1_x + 1, p1_y + 1, player1_color);
        draw(p1_x - 1, p1_y - 1, player1_color);
        draw(p1_x + 1, p1_y - 1, player1_color);
        draw(p1_x - 1, p1_y + 1, player1_color);
        p1_lastDir = 1;
        p1_bo = 0;
        p1_nd = 0;
    }
    else
    {
        draw(p2_x, p2_y, map[p2_x][p2_y]);
        replaceColor(player2_color, color);
        restoreFromMap(player2_character_color);
        // Do not spawn on the same position as player 1
        p2_x = rand() % 37;
        p2_y = rand() % 22;
        while (p2_x == p1_x && p2_y == p1_y)
        {
            p2_x = rand() % 37;
            p2_y = rand() % 22;
        }
        // Draw A 3X3 square around the player
        draw(p2_x, p2_y, player2_color);
        draw(p2_x + 1, p2_y, player2_color);
        draw(p2_x - 1, p2_y, player2_color);
        draw(p2_x, p2_y + 1, player2_color);
        draw(p2_x, p2_y - 1, player2_color);
        draw(p2_x + 1, p2_y + 1, player2_color);
        draw(p2_x - 1, p2_y - 1, player2_color);
        draw(p2_x + 1, p2_y - 1, player2_color);
        draw(p2_x - 1, p2_y + 1, player2_color);
        p2_lastDir = 1;
        p2_bo = 0;
        p2_nd = 0;
    }
}

void handleInput(unsigned char port)
{
    unsigned char prevX, prevY, prevLastDir;
    if (port == 0)
    {
        prevX = p1_x;
        prevY = p1_y;
        prevLastDir = p1_lastDir;
        if (controller_joy_up(port) || controller_ispressed(0x17) || p1_lastDir == 2)
        {
            p1_y--;
            p1_lastDir = 2;
        }
        if (controller_joy_down(port) || controller_ispressed(0x13) || p1_lastDir == 3)
        {
            p1_y++;
            p1_lastDir = 3;
        }
        if (controller_joy_left(port) || controller_ispressed(0x01) || p1_lastDir == 4)
        {
            p1_x--;
            p1_lastDir = 4;
        }
        if (controller_joy_right(port) || controller_ispressed(0x04) || p1_lastDir == 5)
        {
            p1_x++;
            p1_lastDir = 5;
        }
        if (prevLastDir == 0 && p1_lastDir != 0)
        {
            respawn(0, 0);
            return;
        }
        if (p1_lastDir == 0 || p1_lastDir == 1)
        {
            return;
        }
        if (isCollidingWith(p1_x, p1_y, player2_character_color))
        {
            // Player 1 dies by player 2.
            respawn(0, player2_color);
            return;
        }
        if (isCollidingWith(p1_x, p1_y, player1_character_color))
        {
            p1_x = prevX;
            p1_y = prevY;
            if (p1_nd >= nearDeathSaveTicks)
            {
                // Player 1 dies.
                respawn(0, 0);
            }
            else
            {
                p1_nd++;
            }
            return;
        }else{
            p1_nd = 0;
        }
        if (isCollidingWith(prevX, prevY, 0))
        {
            p1_bo++;
        }
        if (isCollidingWith(p1_x, p1_y, player1_color) && p1_bo > 0)
        {
            // Fill the connected area with the player 1 color.
            findAndFloodFill(prevX, prevY, player1_color, player1_character_color);
            p1_bo = 0;
        }
        if (p1_x < 0)
        {
            p1_x = 0;
            p1_lastDir = 1;
        }
        else if (p1_x > max_x)
        {
            p1_x = max_x;
            p1_lastDir = 1;
        }
        if (p1_y < 0)
        {
            p1_y = 0;
            p1_lastDir = 1;
        }
        else if (p1_y > max_y)
        {
            p1_y = max_y;
            p1_lastDir = 1;
        }
        if (p2_lastDir == 0)
        {
            p2_x = p1_x;
            p2_y = p1_y;
        }
        if (controller_joy_fire(port) || controller_ispressed(0x20))
        {
            p1_lastDir = 1;
        }
        draw(p1_x, p1_y, player1_character_color);
    }
    else
    {
        prevX = p2_x;
        prevY = p2_y;
        prevLastDir = p2_lastDir;
        if (controller_joy_up(port) || p2_lastDir == 3)
        {
            p2_y--;
            p2_lastDir = 3;
        }
        if (controller_joy_down(port) || p2_lastDir == 4)
        {
            p2_y++;
            p2_lastDir = 4;
        }
        if (controller_joy_left(port) || p2_lastDir == 5)
        {
            p2_x--;
            p2_lastDir = 5;
        }
        if (controller_joy_right(port) || p2_lastDir == 6)
        {
            p2_x++;
            p2_lastDir = 6;
        }
        if (prevLastDir == 0 && p2_lastDir != 0)
        {
            respawn(1, 0);
            return;
        }
        if (p2_lastDir == 0 || p2_lastDir == 1)
        {
            return;
        }
        if (isCollidingWith(p2_x, p2_y, player1_character_color))
        {
            // Player 2 dies by player 1.
            respawn(1, player1_color);
            return;
        }
        if (isCollidingWith(p2_x, p2_y, player2_character_color))
        {
            p2_x = prevX;
            p2_y = prevY;
            if (p2_nd >= nearDeathSaveTicks)
            {
                // Player 2 dies.
                respawn(1, 0);
            }
            else
            {
                p2_nd++;
            }
            return;
        }else{
            p2_nd = 0;
        }
        if (isCollidingWith(prevX, prevY, 0))
        {
            p2_bo++;
        }
        if (isCollidingWith(p2_x, p2_y, player2_color) && p2_bo > 0)
        {
            // Fill the connected area with the player 2 color.
            findAndFloodFill(prevX, prevY, player2_color, player2_character_color);
            p2_bo = 0;
        }
        if (p2_x < 0)
        {
            p2_x = 0;
            p2_lastDir = 1;
        }
        else if (p2_x > max_x)
        {
            p2_x = max_x;
            p2_lastDir = 1;
        }
        if (p2_y < 0)
        {
            p2_y = 0;
            p2_lastDir = 1;
        }
        else if (p2_y > max_y)
        {
            p2_y = max_y;
            p2_lastDir = 1;
        }
        if (p1_lastDir == 0)
        {
            p1_x = p2_x;
            p1_y = p2_y;
        }
        if (controller_joy_fire(port))
        {
            p2_lastDir = 1;
        }
        draw(p2_x, p2_y, player2_character_color);
    }
}

int main(void)
{
    char debug = 1;
    int timeLeft = 240;
    int p1Score = 0;
    int p2Score = 0;
    int i;
    int j;

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

    resetGame();

    // Game Loop
    while (1)
    {
        if (debug){
            char *timeLeftStr = malloc(100);
            char *p1state = malloc(100);
            char *p1nd = malloc(50);

            sprintf(timeLeftStr, "time left: %i", timeLeft);
            renderlib_drawstring(0, 1, color_white, timeLeftStr);

            sprintf(p1state, "player 1 state: %i", p1_lastDir);
            renderlib_drawstring(0, 2, color_white, p1state);

            sprintf(p1nd, "player 1 nd: %i", p1_nd);
            renderlib_drawstring(0, 3, color_white, p1nd);
        }

        // check if the letter "Q" was pressed
        if (controller_ispressed(0x51)) // TODO: Correct this.
        {
            // if so, exit the program
            break;
        }

        // Handle Player Input
        handleInput(0);
        handleInput(1);

        sleep(750);

        if (timeLeft > 0)
        {
            timeLeft--;
        }
        else
        {
            // Game over
            // Check who won
            for (i = 0; i < 320; i++)
            {
                for (j = 0; j < 200; j++)
                {
                    if (renderlib_getpixel(i, j) == player1_color)
                    {
                        p1Score++;
                    }
                    else if (renderlib_getpixel(i, j) == player2_color)
                    {
                        p2Score++;
                    }
                }
            }
            renderlib_clear();
            if (p1Score > p2Score)
            {
                // Player 1 wins
                // Draw in the center of the screen (use max_x and max_y)
                renderlib_drawstring(max_x / 2 - 7, max_y / 2, color_white, "Player 1 wins!");
            }
            else if (p2Score > p1Score)
            {
                // Player 2 wins
                renderlib_drawstring(max_x / 2 - 7, max_y / 2, color_white, "Player 2 wins!");
            }
            else
            {
                // Draw
                renderlib_drawstring(max_x / 2 - 2, max_y / 2, color_white, "Draw!");
            }
            sleep(1000);
            resetGame();
            // break;
        }
    }
    renderlib_unload();
    return EXIT_SUCCESS;
}