		ifndef	__reg6208inc	; avoid multiple inclusion
__reg6208inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6208.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6208/09/10/20                   *
;*                                                                          *
;*   Source: ST6208/09/10/20C Data Sheet, Rev. 4, January 2009              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	sfr	0080h		; Start Address Internal RAM
RAMEND		sfr	00bfh		; End Address Internal RAM

		switch  MOMCPUNAME
		case	"ST6208"
ROMSTART	label	0ba0h		; End     "        "     ROM
__hasadc	eval	0
		case	"ST6209"
ROMSTART	label	0ba0h		; End     "        "     ROM
__hasadc	eval	1
		case	"ST6210"
ROMSTART	label	0880h		; End     "        "     ROM
__hasadc	eval	1
		case	"ST6220"
ROMSTART	label	0080h		; End     "        "     ROM
__hasadc	eval	1
		elsecase
		fatal	"Huh?"
		endcase

;----------------------------------------------------------------------------
; Interrupt Vectors

		if	__hasadc
ADC_vect	 label	0ff0h		; End Of Conversion
		endif
TIMER_vect	label	0ff2h		; Timer Underflow
PORTB_vect	label	0ff4h		; Ext. Interrupt Port B
PORTA_vect	label	0ff6h		; Ext. Interrupt Port A
NMI_vect	label	0ffch		; Non Maskable Interrupt
RESET_vect	label	0ffeh		; RESET

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "A",0c0h
		__defgpio "B",0c1h

;----------------------------------------------------------------------------
; CPU

		include	"ior.inc"

;----------------------------------------------------------------------------
; Watchdog

		include	"wdg.inc"

;----------------------------------------------------------------------------
; Analog/Digital Converter

                if      __hasadc
                include	"adc.inc"
                elseif
ADCR		sfr	0d1h
		endif
OSCOFF		bit	2,ADCR		;  Main Oscillator Off

;----------------------------------------------------------------------------
; Timer 1

		include	"timer.inc"
		__deftimer 0d2h,""
TOUT		bit	5,TSCR		;  Timer Output Control
DOUT		bit	4,TSCR		;  Data Output

		restore
		endif			; __reg6208inc
