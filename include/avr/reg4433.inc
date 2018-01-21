		ifndef	__reg4433inc
__reg4433inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG4433.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S4433                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	255
RAMSTART	equ	0x60,data
RAMEND		equ	0xdf,data
FLASHEND	label	0xfff

		include	"avr/regxx33.inc"

		restore

		endif			; __reg4433inc
