		ifndef	__regm16u2inc
__regm16u2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM16U2.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega16U2                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x2ff,data
FLASHEND	label	0x3fff

		include	"regmxu2.inc"

		restore			; re-enable listing

		endif			; __regm16u2inc
