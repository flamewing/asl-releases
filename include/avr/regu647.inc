		ifndef	__regu647inc
__regu647inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGU647.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90USB647                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x100,data
RAMEND		equ	0x10ff,data
FLASHEND	label	0xffff

__USBHOSTMODE   equ     1
		include	"avr/regmxu6.inc"

		restore			; re-enable listing

		endif			; __regu647inc
