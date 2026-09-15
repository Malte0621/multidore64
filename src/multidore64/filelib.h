/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

#ifndef FILELIB_H
#define FILELIB_H

/* Initialize the filesystem library. Defaults to device 8. */
void filelib_init(void);

/* Set the default device number (usually 8, 9, 10, or 11). */
void filelib_set_device(unsigned char device);

/* Get the current device number. */
unsigned char filelib_get_device(void);

/* Write a buffer to a file on disk. Overwrites any existing file of the same name.
   Returns the number of bytes written, or a negative number on error (-1 = open error). */
int filelib_write(const char *filename, const char *buffer, unsigned int len);

/* Append a buffer to a file on disk. Creates the file if it does not exist.
   Returns the number of bytes written, or a negative number on error (-1 = open error). */
int filelib_append(const char *filename, const char *buffer, unsigned int len);

/* Read up to max_bytes from a file on disk into buffer.
   Returns the number of bytes read, or a negative number on error (-1 = open error). */
int filelib_read(const char *filename, char *buffer, unsigned int max_bytes);

/* Check whether a file exists on disk.
   Returns 1 if the file exists and can be opened, 0 otherwise. */
unsigned char filelib_exists(const char *filename);

/* Delete (scratch) a file from disk.
   Returns 1 if the command was sent successfully, 0 on failure. */
unsigned char filelib_delete(const char *filename);

#endif
