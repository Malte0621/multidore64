# Memory map

What a compiled MultiDore 64 program does with the C64's 64 KB. Knowing this map explains several engine rules - like why the music must be stepped from your main loop and never from an IRQ.

## Layout

```
$0000-$0001   CPU port (DDR / data direction)
$0002-$00FF   zero page - compiler temporaries, pointers, software stack pointer
$0100-$01FF   CPU stack page - CALL/IRQ frames AND the music sequencer's work data
$0200-$03FF   kernal/editor workspace ($0314-$031B IRQ vector lives here)
$0400-$07E7   screen RAM (40x25 characters + color RAM at $D800)
$0800-$0FFF   your program: code, rodata (incl. the embedded tune), BSS
$1000-$1FFF   charset area - VIC sees char ROM at $1000 in bank 0
$2000-$3FFF   free RAM
$4000-$453B   SID tune staging area (written by soundlib_play)
$453C-$8FFF   free RAM - your level data, heap
$9000-$9FFF   compiler software stack (grows down from $9FFE)
$A000-$BFFF   BASIC ROM (banked)
$C000-$CFFF   free RAM
$D000-$DFFF   char ROM or I/O (VIC-II, SID, CIA) - banked via $01
$E000-$FFFF   kernal ROM (banked), hardware vectors at $FFFA-$FFFF
```

Exact boundaries for a given build come from the `.map` file oscar64 writes next to the PRG.

## The music staging area at $4000

`soundlib_play()` copies the PSID payload to `$4000` and jumps to the tune's init entry there. While music plays, `$4000-$453B` belongs to the player:

```c
soundlib_play_file("song.bin"); // or soundlib_play(buf, len); $4000-$453B in use
...
soundlib_stop();           // area reusable again
```

Do not place level data, bitmaps or sprites in that range while music is playing.

## The stack page and interrupts

This is the one rule that bites people:

!!! danger
    The bundled tune keeps its sequencer tables in the CPU stack page (`$0100-$01FF`). That page is also where the 6502 pushes call and interrupt frames. Stepping the player from an interrupt handler interleaves IRQ frames with the tune's data - the return address a later `RTI` pops is then garbage, the CPU ends up executing data, and the machine drops to BASIC.

Consequences the engine enforces for you:

- `soundlib_update()` is a **main-loop** call. It contains no IRQ hooking; it only reads the raster to detect frame boundaries.
- The engine installs no interrupt handler of its own, and `soundlib_play()` does not touch the `$0314` vector. Your program starts with the kernal's default IRQ behavior intact.

If you install your own raster IRQ for gameplay effects, keep the handler minimal (save registers, acknowledge `$D019`, `RTI`) and never call `soundlib_update()` or `jsr $4003` from it.

## Screen and charset

- Screen RAM sits at `$0400`, color RAM at `$D800` - renderlib manages both.
- The default charset is the char ROM visible to the VIC-II at bank offset `$1000`; `renderlib_setcharset()` points `$D018` at a RAM charset instead (2 KB, must reside inside the selected 16 KB VIC bank).

## Banked ROM and the $01 port

The engine leaves the kernal and BASIC mapped in (`$01 = $37`) so `printf`, the keyboard and the default IRQ keep working. renderlib only programs CIA2 (`$DD00`) to select the VIC bank. If you flip `$01` yourself, restore it before calling back into the engine.
