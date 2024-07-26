/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
--------------------------21--------------------------------
(c) 2024 by Malte06
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soundlib.h"
#include <c64.h>
#include <peekpoke.h>

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

unsigned int SIDSIZE = 6144;
// unsigned char SIDBAK[6144];

extern void SIDINIT(); 
extern void SIDPLAY();
extern void SIDSTOP();

void soundlib_init()
{
	
}

void soundlib_play(char FILEDATA[]){
	// memcpy((void*)(SIDBAK),(void*)SIDLOAD,SIDSIZE);
	memcpy((void*)(SIDLOAD),(void*)FILEDATA,SIDSIZE);
    SIDINIT();
    SIDPLAY();
}

void soundlib_play_raw(const int freqs[])
{
    unsigned int i;
	unsigned char t, song_length = sizeof(freqs) / 2;
	
	SID.v1.ad    = 0x58; 	// Attack/decay voice 1
	SID.v1.sr    = 0x59; 	// Sustain/release voice 1
	SID.amp      = 0xFF; 	// Volume (0x1F)
	SID.v1.pw	 = 0x0F0F; 	// Pulse width voice 1
	
	for (t = 0 ; t < song_length ; t += 2) {
		SID.v1.freq  = freqs[t];	// Frequency 
		SID.v1.ctrl  = 0x41;	// Control voice 1
		for (i = 0 ; i < freqs[t + 1] * 3; i++) {}
		SID.v1.ctrl  = 0x40;
		for (i = 0 ; i < 1000 ; i++) {}
	}		
}

void soundlib_stop()
{
	// Stop the SID
	SIDSTOP();
	SID.v1.ctrl  = 0x40;
	SID.v2.ctrl  = 0x40;
	SID.v3.ctrl  = 0x40;
	// Restore the original SID data
	// memcpy((void*)(SIDLOAD),(void*)(SIDBAK),SIDSIZE);
}