.export _SIDINIT
.export _SIDFILE
.export _SIDPLAY
.export _SIDSTOP
_SIDFILE: .INCBIN "song.bin",$7e

SIDLOADPOS = $4000;
SIDPLAYPOS = $4003;

SIDSTEP: 		;step SID
	jsr SIDPLAYPOS  ;jump to SIDPLAYPOS, return
	rts 

_SIDINIT:		;init SID 
	lda #0
	tax		;transfer accumilator to x register
	tay		;transfer accumilator to y register
	jsr SIDLOADPOS ;jump to SIDLOADPOS, return
	rts 

_SIDPLAY:
	LDX #<IRQ	;lo-byte
	LDY #>IRQ	;hi-byte
	STX $0314
	STY $0315

	LDA #$00	;raster pos
	STA $D012

	LDA #$7F	;cia enable interrupt
	STA $DC0D
	LDA #$1B	
	STA $D011	;vertical pos 
	LDA #$01
	STA $D01A	;raster input signals from vic (sync)
	LDA #$00
	CLI
	RTS

_SIDSTOP:
	; Uninstall IRQ
	LDA #$00
	STA $0314
	LDA #$00
	STA $0315

	; Disable CIA interrupts
	LDA #$00
	STA $DC0D

	; Disable raster interrupt
	LDA #$00
	STA $D01A
	
	SEI
	RTS

IRQ:
	INC $D019
	LDA #$00
	STA $D012	

	JSR SIDSTEP	;step sid
	JMP $EA31	;jump to kernal stndard input service routines