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

// Embedded SID song data (PSID). The original used
//   _SIDFILE: .INCBIN "song.bin",$7e
// which imports song.bin from byte $7e (126) to the end of the file.
// song.bin is 1465 bytes, so 1465 - 126 = 1339 bytes are embedded.
__export char SIDFILE[] = {
    #embed 1339 126 "../song.bin"
};


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
