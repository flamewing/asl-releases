		ifndef	__regt13ainc
__regt13ainc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN13A.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny13A                      *
;*                                                                          *
;****************************************************************************

		include	"regtn13.inc"	; upward compatible from ATtiny13

;----------------------------------------------------------------------------
; Chip Configuration

BODCR		port	0x30		; Brown-Out Detection Control Register
BODSE		avrbit	BODCR,0		; BOD Sleep Enable
BODS		avrbit	BODCR,1		; BOD Sleep

PRR		port	0x25		; Power Reduction Register
PRTIM0		avrbit	PRR,1		; Power Reduction Timer 0
PRADC		avrbit	PRR,0		; Power Reduction A/D Converter

		restore

		endif			; __regt13ainc
