		ifndef	__mcf5472inc		; avoid multiple inclusion
__mcf5472inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5472.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5472                  *
;*                                                                          *
;****************************************************************************

__has_usb	equ	1
__has_sec	equ	0
__has_fec1	equ	0

		include	"mcf547x.inc"
		
		restore
		
		endif				; __mcf5472inc
