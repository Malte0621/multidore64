/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#include <stdio.h>
#include <string.h>
#include "filelib.h"
#include <c64/kernalio.h>

#define FILELIB_FNUM     2      /* logical file number for data */
#define FILELIB_CHANNEL  2      /* secondary address for data */
#define FILELIB_CMD_CHAN 15     /* command / status channel */

static unsigned char filelib_device = 8;

void filelib_init(void)
{
    filelib_device = 8;
}

void filelib_set_device(unsigned char device)
{
    filelib_device = device;
}

unsigned char filelib_get_device(void)
{
    return filelib_device;
}

static void build_cbm_name(char *dest, const char *filename, const char *prefix, const char *suffix)
{
    dest[0] = 0;
    if (prefix && prefix[0])
    {
        // Only prepend prefix if filename does not already specify drive/replace
        if (filename[0] != '@' && strchr(filename, ':') == NULL)
        {
            strcat(dest, prefix);
        }
    }
    strcat(dest, filename);
    if (suffix && suffix[0])
    {
        // Only append type/mode if filename does not already contain a comma
        if (strchr(filename, ',') == NULL)
        {
            strcat(dest, suffix);
        }
    }
}

int filelib_write(const char *filename, const char *buffer, unsigned int len)
{
    char cbm_name[36];
    if (!filename || !filename[0])
        return -1;

    build_cbm_name(cbm_name, filename, "@0:", ",s,w");

    krnio_setnam(cbm_name);
    if (!krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
        return -1;

    int written = 0;
    if (buffer && len > 0)
    {
        written = krnio_write(FILELIB_FNUM, buffer, (int)len);
    }
    krnio_close(FILELIB_FNUM);
    return written;
}

int filelib_append(const char *filename, const char *buffer, unsigned int len)
{
    char cbm_name[36];
    if (!filename || !filename[0])
        return -1;

    build_cbm_name(cbm_name, filename, "0:", ",s,a");

    krnio_setnam(cbm_name);
    if (!krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
        return -1;

    int written = 0;
    if (buffer && len > 0)
    {
        written = krnio_write(FILELIB_FNUM, buffer, (int)len);
    }
    krnio_close(FILELIB_FNUM);
    return written;
}

int filelib_read(const char *filename, char *buffer, unsigned int max_bytes)
{
    char cbm_name[36];
    if (!filename || !filename[0] || !buffer || max_bytes == 0)
        return -1;

    // First attempt: SEQ read mode
    build_cbm_name(cbm_name, filename, "0:", ",s,r");
    krnio_setnam(cbm_name);
    if (!krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
    {
        // Second attempt: raw read (works for PRG or untyped files)
        build_cbm_name(cbm_name, filename, "0:", "");
        krnio_setnam(cbm_name);
        if (!krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
        {
            return -1;
        }
    }

    int bytes = krnio_read(FILELIB_FNUM, buffer, (int)max_bytes);
    krnio_close(FILELIB_FNUM);
    return bytes;
}

unsigned char filelib_exists(const char *filename)
{
    char cbm_name[36];
    if (!filename || !filename[0])
        return 0;

    build_cbm_name(cbm_name, filename, "0:", ",s,r");
    krnio_setnam(cbm_name);
    if (krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
    {
        krnio_close(FILELIB_FNUM);
        return 1;
    }

    build_cbm_name(cbm_name, filename, "0:", "");
    krnio_setnam(cbm_name);
    if (krnio_open(FILELIB_FNUM, filelib_device, FILELIB_CHANNEL))
    {
        krnio_close(FILELIB_FNUM);
        return 1;
    }

    return 0;
}

unsigned char filelib_delete(const char *filename)
{
    char cmd[36];
    if (!filename || !filename[0])
        return 0;

    cmd[0] = 0;
    strcat(cmd, "s0:");
    strcat(cmd, filename);

    krnio_setnam(cmd);
    if (!krnio_open(FILELIB_CMD_CHAN, filelib_device, FILELIB_CMD_CHAN))
        return 0;

    krnio_close(FILELIB_CMD_CHAN);
    return 1;
}
