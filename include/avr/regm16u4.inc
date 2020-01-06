		ifndef	__regm16u4inc
__regm16u4inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM16U4.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega16U4                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x5ff,data
FLASHEND	label	0x3fff

		include	"regmxu4.inc"

		restore			; re-enable listing

		endif			; __regm16u4inc
