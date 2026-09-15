/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#ifndef SOUNDLIB_H
#define SOUNDLIB_H

void soundlib_init(void);

/* Play a SID tune from a memory buffer (raw 6502 binary or unstripped PSID/RSID file).
   Staged at $4000 and initialized. Returns 1 on success, 0 on invalid buffer. */
unsigned char soundlib_play(const char *tune, unsigned int len);

/* Load a tune from disk (device 8) and initialize playback.
   Supports both raw binary files and unstripped PSID/RSID files.
   Returns 1 on success, 0 if the file could not be read. */
unsigned char soundlib_play_file(const char *filename);

/* Step the SID player once per frame from the main loop.
   Safely does nothing if no song is currently playing. */
void soundlib_update(void);

/* Silence all SID voices and stop playback. */
void soundlib_stop(void);

#endif
