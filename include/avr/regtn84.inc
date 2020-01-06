		ifndef	__regtn84inc
__regtn84inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN84.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny84(A)                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x25f,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

		include "regtnx4.inc"

		restore

		endif			; __regtn84inc
