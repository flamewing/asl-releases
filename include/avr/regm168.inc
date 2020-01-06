		ifndef	__regm168inc
__regm168inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM168.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega168                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x4ff,data
FLASHEND	label	0x3fff

		include "regmx8.inc"

		restore			; re-enable listing

		endif			; __regm168inc
