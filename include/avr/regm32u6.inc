		ifndef	__regm32u6inc
__regm32u6inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM32U6.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega32U6                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	1023
RAMSTART	equ	0x100,data
RAMEND		equ	0xaff,data
FLASHEND	label	0x7fff

__USBHOSTMODE	equ	0
		include	"regmxu6.inc"

		restore			; re-enable listing

		endif			; __regm32u6inc
