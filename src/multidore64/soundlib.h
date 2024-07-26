/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2024 by Malte0621
*/

#ifndef SOUNDLIB_H
#define SOUNDLIB_H

#include <stdint.h>

void soundlib_init();
void soundlib_play(char FILEDATA[]);
void soundlib_stop();
void soundlib_play_raw(const int freqs[]);

#endif