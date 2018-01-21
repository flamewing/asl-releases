		ifndef	__regtn12inc
__regtn12inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN12.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny12                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	63		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM (i.e. no SRAM)
FLASHEND	label   1023		; end address Flash

		include	"avr/regtn112.inc"

		restore

		endif			; __regtn12inc
