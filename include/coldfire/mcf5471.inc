		ifndef	__mcf5471inc		; avoid multiple inclusion
__mcf5471inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5471.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5471                  *
;*                                                                          *
;****************************************************************************

__has_usb	equ	0
__has_sec	equ	1
__has_fec1	equ	1

		include	"mcf547x.inc"
		
		restore
		
		endif				; __mcf5471inc
