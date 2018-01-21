		ifndef	__regm8u2inc
__regm8u2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM8U2.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega8U2                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x2ff,data
FLASHEND	label	0x1fff

		include	"avr/regmxu2.inc"

		restore			; re-enable listing

		endif			; __regm8u2inc
