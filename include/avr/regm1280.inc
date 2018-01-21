		ifndef	__regm1280inc
__regm1280inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM1280.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega1280                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x200,data
RAMEND		equ	0x21ff,data
FLASHEND	label	0x1ffff

ATmegaxx0	equ	1
		include	"avr/regmxx01.inc"

		restore			; re-enable listing

		endif			; __regm1280inc
