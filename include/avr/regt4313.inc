		ifndef	__regt4313inc
__regt4313inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT4313.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny4313                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     255		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x15f,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

		include "regtx313.inc"

		restore

		endif			; __regt4313inc
