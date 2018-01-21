		ifndef	__regm48inc
__regm48inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM48.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega48                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	255
RAMSTART	equ	0x100,data
RAMEND		equ	0x2ff,data
FLASHEND	label	0xfff

		include "avr/regmx8.inc"

		restore			; re-enable listing

		endif			; __regm48inc
