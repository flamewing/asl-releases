		ifndef	__regmc2inc	; avoid multiple inclusion
__regmc2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMC2.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7MC2xx                          *
;*                                                                          *
;*   Source: ST7MC Data Sheet, Rev. 13, April 2009                          *
;*                                                                          *
;****************************************************************************

		include "regmc1.inc"

;----------------------------------------------------------------------------
; Interrupt Vectors

TB_vect		label	$ffe6		; Timer B Peripheral Interrupts
SPI_vect	label	$ffea		; SPI Interrupt

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0021

;----------------------------------------------------------------------------
; Timer

		__deftimer "TB",$0040
TBTIMD		bit	TASR,2		;  Timer A Disable

		restore
		endif			; __regmc2inc
