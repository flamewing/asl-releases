		ifndef	__regu1287inc
__regu1287inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGU1287.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90USB1287                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x100,data
RAMEND		equ	0x20ff,data
FLASHEND	label	0x1ffff

__USBHOSTMODE   equ     1
		include	"avr/regmxu6.inc"

		restore			; re-enable listing

		endif			; __regu1287inc
