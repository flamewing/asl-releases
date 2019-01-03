		ifndef	__mcf5470inc		; avoid multiple inclusion
__mcf5470inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5470.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5470                  *
;*                                                                          *
;****************************************************************************

__has_usb	equ	0
__has_sec	equ	0
__has_fec1	equ	1

		include	"coldfire/mcf547x.inc"
		
		restore

		endif				; __mcf5470inc
