		ifndef	__regtn11inc
__regtn11inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN11.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny11                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM (i.e. no SRAM)
FLASHEND	label   1023		; end address Flash

		include	"regtn112.inc"

		restore

		endif			; __regtn11inc
