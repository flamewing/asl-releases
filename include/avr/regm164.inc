		ifndef	__regm164inc
__regm164inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM164.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega164                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x4ff,data
FLASHEND	label	0x3fff

		include	"regmxx4.inc"
	
		restore			; re-enable listing

		endif			; __regm164inc
