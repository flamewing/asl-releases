		ifndef	__regtn85inc
__regtn85inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN85.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny85                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     511		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x25f,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

		include "avr/regtnx5.inc"

		restore

		endif			; __regtn85inc
