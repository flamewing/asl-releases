		ifndef	__regtn25inc
__regtn25inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN25.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny25                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0xdf,data	; end address SRAM
FLASHEND	label   2047		; end address Flash

		include "avr/regtnx5.inc"

		restore

		endif			; __regtn25inc
