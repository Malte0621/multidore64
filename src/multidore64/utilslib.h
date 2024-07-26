/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023 by Malte0621
*/

#ifndef UTILSLIB_H
#define UTILSLIB_H

#define FIXED_PI 31415
#define FIXED_2_PI 62831
#define FIXED_PI_DIV_2 15707

void sleep(unsigned int ns);
int cos(int angle);
int sin(int angle);

#define TABLE_SIZE 360
extern int cos_table[TABLE_SIZE];
extern int sin_table[TABLE_SIZE];

#endif