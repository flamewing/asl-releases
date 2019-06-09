		ifndef	__regtn861inc
__regtn861inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN861.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny861(A)                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     511		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x25f,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

		include "regtnx61.inc"

		restore

		endif			; __regtn861inc
