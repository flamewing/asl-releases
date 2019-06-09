		ifndef	__regm644rfr2inc
__regm644rfr2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM644RFR2.INC                                         *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega644RFR2                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x200,data
RAMEND		equ	0x21ff,data
FLASHEND	label	0xffff

		include "regx4rfr2.inc"

		restore			; re-enable listing

		endif			; __regm644rfr2inc
