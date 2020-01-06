		ifndef	__regtn167inc
__regtn167inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN167.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny167                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     511		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x2ff,data	; end address SRAM
FLASHEND	label   16383		; end address Flash

		include "regtnx7.inc"

		restore

		endif			; __regtn167inc
