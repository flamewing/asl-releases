		ifndef	__reg1284rfr2inc
__reg1284rfr2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM644RFR2.INC                                         *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega1284RFR2                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x200,data
RAMEND		equ	0x41ff,data
FLASHEND	label	0x1ffff

		include "regx4rfr2.inc"

		restore			; re-enable listing

		endif			; __reg1284rfr2inc
