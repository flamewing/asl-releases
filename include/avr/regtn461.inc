		ifndef	__regtn461inc
__regtn461inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN461.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny461(A)                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     255		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x15f,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

		include "avr/regtnx61.inc"

		restore

		endif			; __regtn461inc
