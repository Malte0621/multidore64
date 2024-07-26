/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2024 by Malte0621
*/

#include <stdlib.h>

const unsigned char color_black = 0x00;
const unsigned char color_white = 0x01;
const unsigned char color_red = 0x02;
const unsigned char color_cyan = 0x03;
const unsigned char color_purple = 0x04;
const unsigned char color_green = 0x05;
const unsigned char color_blue = 0x06;
const unsigned char color_yellow = 0x07;
const unsigned char color_orange = 0x08;
const unsigned char color_brown = 0x09;
const unsigned char color_light_red = 0x0A;
const unsigned char color_dark_grey = 0x0B;
const unsigned char color_grey = 0x0C;
const unsigned char color_light_green = 0x0D;
const unsigned char color_light_blue = 0x0E;
const unsigned char color_light_grey = 0x0F;

unsigned char color_random()
{
    return rand() % 16;
}