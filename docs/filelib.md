# filelib - C64 disk filesystem library

Disk I/O library for reading, writing, appending, and deleting files on Commodore 64 disk drives (1541 / 1571 / 1581 / SD2IEC).

## API

```c
#include "multidore64/filelib.h"

void filelib_init(void);
void filelib_set_device(unsigned char device); // default: 8
unsigned char filelib_get_device(void);

// Write buffer to file, replacing existing file if present.
// Returns bytes written, or -1 on error.
int filelib_write(const char *filename, const char *buffer, unsigned int len);

// Append buffer to file (creates file if not yet existing).
// Returns bytes appended, or -1 on error.
int filelib_append(const char *filename, const char *buffer, unsigned int len);

// Read up to max_bytes from file into buffer.
// Returns bytes read, or -1 on error.
int filelib_read(const char *filename, char *buffer, unsigned int max_bytes);

// Check if a file exists on disk. Returns 1 if found, 0 otherwise.
unsigned char filelib_exists(const char *filename);

// Delete (scratch) a file from disk. Returns 1 on success, 0 on failure.
unsigned char filelib_delete(const char *filename);
```

## Quick start

### Saving and loading a high score

```c
#include "multidore64/filelib.h"

struct SaveData {
    unsigned int score;
    char initials[4];
};

void save_game(unsigned int score, const char *initials)
{
    struct SaveData data;
    data.score = score;
    strncpy(data.initials, initials, 4);

    filelib_write("HIGHSCORE.DAT", (const char *)&data, sizeof(data));
}

unsigned int load_game(void)
{
    struct SaveData data;
    if (filelib_exists("HIGHSCORE.DAT"))
    {
        if (filelib_read("HIGHSCORE.DAT", (char *)&data, sizeof(data)) == sizeof(data))
        {
            return data.score;
        }
    }
    return 0; // default if no save file exists
}
```

### Appending to a log or replay file

```c
filelib_append("GAME.LOG", "START\n", 6);
filelib_append("GAME.LOG", "P1_WIN\n", 7);
```

## How it works

- **Native KERNAL I/O**: `filelib` uses the Commodore 64 KERNAL serial bus routines via `<c64/kernalio.h>`, avoiding standard library overhead.
- **Safe overwrite**: `filelib_write` automatically formats filenames with `@0:` and `,s,w` mode so that the 1541 DOS replaces any existing file cleanly without throwing a `FILE EXISTS` (error 63) drive fault.
- **Append mode**: `filelib_append` uses CBM DOS `,s,a` mode. If the file does not exist, the drive automatically creates it.
- **Flexible read**: `filelib_read` opens sequential mode first (`,s,r`), and falls back to untyped read if necessary, enabling both SEQ and PRG file reads.
- **Scratch (delete)**: `filelib_delete` opens command channel 15 and sends the CBM DOS `s0:filename` scratch command.
- **Multi-drive support**: The target device defaults to 8 (the standard primary floppy drive), but can be redirected to drive 9, 10, or 11 with `filelib_set_device()`.
