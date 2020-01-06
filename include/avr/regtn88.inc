		ifndef	__regtn88inc
__regtn88inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN88.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny88                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x2ff,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

		include "regtnx8.inc"

		restore

		endif			; __regtn88inc
