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
#include "utilslib.h"

void sleep(unsigned int ns)
{
    unsigned int i;
    for (i = 0; i < ns; i++)
    {
        __asm volatile { nop }
    }
}
