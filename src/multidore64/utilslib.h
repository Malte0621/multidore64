/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#ifndef UTILSLIB_H
#define UTILSLIB_H

/** Block for `frames` video frames (one frame = 1/50 s on PAL).
 *  Paces against the VIC raster, so the delay is real time. */
void sleep(unsigned int frames);

#endif
