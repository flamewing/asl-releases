		ifndef	__regm32u2inc
__regm32u2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM32U2.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega32U2                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	1023
RAMSTART	equ	0x100,data
RAMEND		equ	0x4ff,data
FLASHEND	label	0x7fff

		include	"avr/regmxu2.inc"

		restore			; re-enable listing

		endif			; __regm32u2inc
