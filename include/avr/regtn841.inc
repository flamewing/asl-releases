		ifndef	__regtn841inc
__regtn841inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN841.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny841                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     511		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x2ff,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

		include "regtnx41.inc"

		restore

		endif			; __regtn841inc
