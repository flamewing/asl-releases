		ifndef	__regm88inc
__regm88inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM88.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega88                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x4ff,data
FLASHEND	label	0x1fff

		include "regmx8.inc"

		restore			; re-enable listing

		endif			; __regm88inc
