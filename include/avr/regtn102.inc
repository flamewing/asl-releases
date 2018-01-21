		ifndef	__regtn102inc
__regtn102inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN102.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny102                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x40,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM
FLASHEND	label   1023		; end address Flash

		include "avr/regt1024.inc"

		restore

		endif			; __regtn102inc
