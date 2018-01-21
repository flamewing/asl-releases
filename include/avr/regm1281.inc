		ifndef	__regm1281inc
__regm1281inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM1281.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega1281                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x200,data
RAMEND		equ	0x21ff,data
FLASHEND	label	0x1ffff

ATmegaxx1	equ	1
		include	"avr/regmxx01.inc"

		restore			; re-enable listing

		endif			; __regm1281inc
