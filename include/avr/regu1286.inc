		ifndef	__regu1286inc
__regu1286inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGU1286.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90USB1286                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x100,data
RAMEND		equ	0x20ff,data
FLASHEND	label	0x1ffff

__USBHOSTMODE   equ     0
		include	"avr/regmxu6.inc"

		restore			; re-enable listing

		endif			; __regu1286inc
