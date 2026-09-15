/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soundlib.h"
#include <c64/sid.h>
#include <c64/vic.h>
#include "utilslib.h"

struct SIDHeader
{
    char   magic[4];       // Magic number ("PSID")
    uint16_t version;      // Version number
    uint16_t dataOffset;   // Offset to data block
    uint16_t loadAddress;  // Load address
    uint16_t initAddress;  // Init address
    uint16_t playAddress;  // Play address
    uint16_t songs;        // Number of songs
    uint16_t startSong;    // Starting song
    int32_t speed;         // Speed
    char   name[32];       // Song name
    char   author[32];     // Author
    char   released[32];   // Released
	uint16_t flags;        // Flags
	uint16_t startPage;    // Start page
	uint16_t pageLength;   // Page length
	uint16_t secondSIDAddress; // Second SID address
	uint16_t reserved;     // Reserved
	char   thirdSIDAddress; // Third SID address
	char   reserved2;      // Reserved
	char   reserved3[6];   // Reserved
};

#define BE(x) ((x >> 8) | (x << 8))

#define SIDDATA(x) ((struct SIDHeader*)(x))


unsigned int SIDLOAD = 0x4000;

/* Must match the #embed size in soundlib_asm.c (song.bin from offset 126) */
unsigned int SIDSIZE = 1339;
// unsigned char SIDBAK[6144];

extern void SIDINIT();
extern void SIDSTEP();

void soundlib_init()
{

}

void soundlib_play(char FILEDATA[]){
	memcpy((void *)(SIDLOAD),(void *)FILEDATA,SIDSIZE);
    SIDINIT();
}

/* Step the SID player once per VIC frame. Non-blocking: returns immediately
   when no new frame has started. Never call from an IRQ (the tune keeps work
   data in the CPU stack page). */
void soundlib_update()
{
	static char wasBottom = 0;
	char bottom = (vic.ctrl1 & VIC_CTRL1_RST8) != 0;
	char newFrame = bottom && !wasBottom;
	wasBottom = bottom;
	if (newFrame)
		SIDSTEP();
}

void soundlib_stop()
{
	// Stop the SID
	sid.voices[0].ctrl  = SID_CTRL_RECT;
	sid.voices[1].ctrl  = SID_CTRL_RECT;
	sid.voices[2].ctrl  = SID_CTRL_RECT;
	// Restore the original SID data
	// memcpy((void*)(SIDLOAD),(void*)(SIDBAK),SIDSIZE);
}
