		ifndef	__regu646inc
__regu646inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGU646.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90USB646                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x100,data
RAMEND		equ	0x10ff,data
FLASHEND	label	0xffff

__USBHOSTMODE   equ     0
		include	"regmxu6.inc"

		restore			; re-enable listing

		endif			; __regu646inc
