		ifndef	__mcf5473inc		; avoid multiple inclusion
__mcf5473inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5473.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5473                  *
;*                                                                          *
;****************************************************************************

__has_usb	equ	1
__has_sec	equ	1
__has_fec1	equ	0

		include	"mcf547x.inc"
		
		restore
		
		endif				; __mcf5473inc
