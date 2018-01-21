		ifndef	__reg2233inc
__reg2233inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG2233.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S2233                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	127
RAMSTART	equ	0x60,data
RAMEND		equ	0xdf,data
FLASHEND	label	0x7ff

		include	"avr/regxx33.inc"

		restore

		endif			; __reg2233inc
