		ifndef	__mcf5475inc		; avoid multiple inclusion
__mcf5475inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5475.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5475                  *
;*                                                                          *
;****************************************************************************

__has_usb	equ	1
__has_sec	equ	1
__has_fec1	equ	1

		include	"mcf547x.inc"
		
		restore
		
		endif				; __mcf5475inc
