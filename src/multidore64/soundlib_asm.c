/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

// The SID is stepped from the main loop (soundlib_update): the tune's
// sequencer keeps work data in CPU stack page $0100-$01FF, so it must not
// run from a raster IRQ handler (IRQ frames corrupt its data and vice versa).

// The engine does not embed any tune. Games either embed their own song data
// or load one from disk; see soundlib_play() / soundlib_play_file().

// Step the SID: jump to the SID play routine at 0x4003 and return.
__native void SIDSTEP(void)
{
    __asm volatile {
        jsr 0x4003
        rts
    }
}

// Init the SID: call the SID init routine at 0x4000 with A=X=Y=0.
__native void SIDINIT(void)
{
    __asm volatile {
        lda #0
        tax
        tay
        jsr 0x4000
        rts
    }
}
