/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include "utilslib.h"
#include <c64/vic.h>

void sleep(unsigned int frames)
{
	/* Block for `frames` video frames (50 Hz on PAL machines). Each frame is
	   one full VIC raster pass, so the delay is real time, not CPU cycles.
	   This still polls the raster register - a truly sleeping CPU would need
	   an interrupt timer, which the engine deliberately does not install. */
	while (frames--)
		vic_waitFrame();
}
