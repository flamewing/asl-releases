		ifndef	__spiminc
__spiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPIM.INC                                             *
;*                                                                          *
;*  Contains bit & register definitions for ATmega SPI Interface (incl. 2x) *
;*                                                                          *
;****************************************************************************

		include	"avr/spi90.inc"

SPI2X		equ	0		; double speed mode

		restore			; re-enable listing

		endif			; __spiminc
