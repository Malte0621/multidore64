/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2024 by Malte0621
*/

#ifndef UTILSLIB_H
#define UTILSLIB_H

#define M_PI 31415

// defining FIXED_POINT_SHIFT as 9 is more accurate and will give you a better
// result. (WIP - scaling)
// #define FIXED_POINT_SHIFT 9
#define FIXED_POINT_SHIFT 8
#define FIXED_POINT_SCALE (1 << FIXED_POINT_SHIFT)

void sleep(unsigned int ns);
int cos(int angle);
int sin(int angle);

#define TABLE_SIZE 360
extern int cos_table[TABLE_SIZE];
extern int sin_table[TABLE_SIZE];

#endif