/*
----------------------------------------------------------
This file is a part of MultiDore 64.
----------------------------------------------------------
MultiDore 64 - A decent game engine for the commodore 64!
----------------------------------------------------------
(c) 2023-2026 by Malte0621
*/

// This file is the oscar64 replacement for the old cc65 soundlib.s.
// oscar64 is a whole-program C compiler and cannot assemble raw .s files,
// so the SID player is written as C functions with inline 6502 assembly.
// The emitted instructions match the original soundlib.s exactly.

// Embedded SID song data (PSID). The original used
//   _SIDFILE: .INCBIN "song.bin",$7e
// which imports song.bin from byte $7e (126) to the end of the file.
// song.bin is 1465 bytes, so 1465 - 126 = 1339 bytes are embedded.
__export char SIDFILE[] = {
    #embed 1339 126 "../song.bin"
};

// Forward declaration so SIDPLAY can install the handler below.
__native void irq_handler(void);

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

// Install the raster IRQ that steps the SID, then arm the VIC raster
// interrupt. The handler address is written to the user IRQ vector at
// $0314/$0315 (the location the kernal IRQ routine jumps to).
__native void SIDPLAY(void)
{
    *(void **)0x0314 = irq_handler;
    __asm volatile {
        lda #0
        sta 0xd012
        lda #$7f
        sta 0xdc0d
        lda #$1b
        sta 0xd011
        lda #1
        sta 0xd01a
        lda #0
        cli
    }
}

// Remove the raster IRQ and disable the CIA/VIC interrupts.
__native void SIDSTOP(void)
{
    *(void **)0x0314 = nullptr;
    __asm volatile {
        lda #0
        sta 0xdc0d
        lda #0
        sta 0xd01a
        sei
    }
}

// Raster IRQ handler: advance the raster counter, step the SID, then
// hand control back to the kernal IRQ routine at $EA31.
//
// This is entered via JMP from the kernal (no return address is pushed),
// so it ends with JMP $EA31 rather than RTS. It is deliberately NOT a
// __hwinterrupt: the original handler clobbers A and does not save any
// CPU registers, and a __hwinterrupt prologue would leak saved registers
// onto the stack because the handler never reaches its rti epilogue.
__native void irq_handler(void)
{
    __asm volatile {
        inc 0xd019
        lda #0
        sta 0xd012
        jsr SIDSTEP
        jmp 0xea31
    }
}
