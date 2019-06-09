		ifndef	__regtn441inc
__regtn441inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN441.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny441                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     255		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x1ff,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

		include "regtnx41.inc"

		restore

		endif			; __regtn441inc
