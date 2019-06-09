		ifndef	__regtn5inc
__regtn5inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN5.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny5                        *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x40,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM
FLASHEND	label   511		; end address Flash

__HAS_ADC__	equ	1
		include "regtx459.inc"

		restore

		endif			; __regtn5inc
