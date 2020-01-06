		ifndef	__regtn44inc
__regtn44inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN44.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny44(A)                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x15f,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

		include "regtnx4.inc"

		restore

		endif			; __regtn44inc
