		ifndef	__reg2564rfr2inc
__reg2564rfr2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM644RFR2.INC                                         *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega2564RFR2                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	8191
RAMSTART	equ	0x200,data
RAMEND		equ	0x81ff,data
FLASHEND	label	0x3ffff

		include "regx4rfr2.inc"

		restore			; re-enable listing

		endif			; __reg2564rfr2inc
