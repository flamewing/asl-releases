		ifndef	__regt2313inc
__regt2313inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT2313.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny2313(A)                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0xdf,data	; end address SRAM
FLASHEND	label   2047		; end address Flash

		include "regtx313.inc"

		restore

		endif			; __regt2313inc
