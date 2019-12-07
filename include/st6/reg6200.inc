		ifndef	__reg6200inc	; avoid multiple inclusion
__reg6200inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG6200.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6200/01/03                      *
;*                                                                          *
;*   Source: ST6200/01/03C Data Sheet, Rev. 3.2, July 2001                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	sfr	0080h		; Start Address Internal RAM
RAMEND		sfr	00bfh		; End Address Internal RAM

		switch  MOMCPUNAME
		case	"ST6200","ST6201"
ROMSTART	label	0880h		; End     "        "     ROM
__hasadc	eval	1
		case	"ST6203"
ROMSTART	label	0ba0h		; End     "        "     ROM
__hasadc	eval	0
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

		restore
		endif			; __reg6200inc
