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
#include <c64/kernalio.h>
#include "utilslib.h"

struct SIDHeader
{
    char           magic[4];          // Magic number ("PSID" or "RSID")
    unsigned short version;           // Version number
    unsigned short dataOffset;        // Offset to data block (big endian)
    unsigned short loadAddress;       // Load address (0 = first 2 bytes of data)
    unsigned short initAddress;       // Init address
    unsigned short playAddress;       // Play address
    unsigned short songs;             // Number of songs
    unsigned short startSong;         // Starting song
    long           speed;             // Speed
    char           name[32];          // Song name
    char           author[32];        // Author
    char           released[32];      // Released
};

#define BE(x) (((unsigned short)(x) >> 8) | ((unsigned short)(x) << 8))

#define SID_STAGE   0x4000      /* staging address where the player runs */
#define SID_MAXLEN  6144        /* stay well below screen and heap */

extern void SIDINIT();
extern void SIDSTEP();

static unsigned char soundlib_isPlaying = 0;

void soundlib_init(void)
{
    soundlib_isPlaying = 0;
}

unsigned char soundlib_play(const char *tune, unsigned int len)
{
    if (!tune || len == 0)
        return 0;

    // Check if buffer contains a PSID or RSID file with a header
    if (len >= sizeof(struct SIDHeader) &&
        (memcmp(tune, "PSID", 4) == 0 || memcmp(tune, "RSID", 4) == 0))
    {
        const struct SIDHeader *hdr = (const struct SIDHeader *)tune;
        unsigned int offset = BE(hdr->dataOffset);
        if (hdr->loadAddress == 0)
            offset += 2;        // skip 2-byte C64 load address in data block
        if (offset < len)
        {
            tune += offset;
            len -= offset;
        }
    }

    if (len > SID_MAXLEN)
        len = SID_MAXLEN;

    memcpy((void *)SID_STAGE, tune, len);
    SIDINIT();
    soundlib_isPlaying = 1;
    return 1;
}

unsigned char soundlib_play_file(const char *filename)
{
    if (!filename || !filename[0])
        return 0;

    char cbm_name[36];
    int j = 0;
    if (filename[0] != '@' && strchr(filename, ':') == NULL)
    {
        cbm_name[j++] = '0';
        cbm_name[j++] = ':';
    }
    for (int i = 0; filename[i] && j < 32; i++)
    {
        char c = filename[i];
        if (c >= 'a' && c <= 'z')
            c -= 32;
        cbm_name[j++] = c;
    }
    cbm_name[j] = 0;

    krnio_setnam(cbm_name);
    if (!krnio_open(2, 8, 2))
        return 0;

    int bytes = krnio_read(2, (char *)SID_STAGE, SID_MAXLEN);
    krnio_close(2);

    if (bytes <= 0)
        return 0;

    return soundlib_play((const char *)SID_STAGE, (unsigned int)bytes);
}

/* Step the SID player once per VIC frame. Non-blocking: returns immediately
   when no new frame has started. Safely does nothing if no song is playing. */
void soundlib_update(void)
{
    if (!soundlib_isPlaying)
        return;

    static char wasBottom = 0;
    char bottom = (vic.ctrl1 & VIC_CTRL1_RST8) != 0;
    char newFrame = bottom && !wasBottom;
    wasBottom = bottom;
    if (newFrame)
        SIDSTEP();
}

void soundlib_stop(void)
{
    soundlib_isPlaying = 0;
    sid.voices[0].ctrl = SID_CTRL_RECT;
    sid.voices[1].ctrl = SID_CTRL_RECT;
    sid.voices[2].ctrl = SID_CTRL_RECT;
}
