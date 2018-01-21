		ifndef	__regtn48inc
__regtn48inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN48.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny48                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x1ff,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

		include "avr/regtnx8.inc"

		restore

		endif			; __regtn48inc
