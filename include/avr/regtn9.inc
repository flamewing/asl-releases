		ifndef	__regtn9inc
__regtn9inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN4.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny4                        *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x40,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM
FLASHEND	label   1023		; end address Flash

__HAS_ADC__	equ	0
		include "regtx459.inc"

		restore

		endif			; __regtn9inc
