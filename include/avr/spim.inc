		ifndef	__spiminc
__spiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPIM.INC                                             *
;*                                                                          *
;*  Contains Bit & Register Definitions for ATmega SPI Interface (incl. 2x) *
;*                                                                          *
;****************************************************************************

		include	"avr/spi90.inc"

SPI2X		avrbit	SPSR,0		; Double Speed Mode

		restore			; re-enable listing

		endif			; __spiminc
