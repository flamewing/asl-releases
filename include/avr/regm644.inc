		ifndef	__regm644inc
__regm644inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM644.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega644                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x100,data
RAMEND		equ	0x10ff,data
FLASHEND	label	0xffff

		include "avr/regmxx4.inc"

		restore			; re-enable listing

		endif			; __regm644inc
