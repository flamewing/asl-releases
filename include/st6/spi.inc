		ifndef	__spiinc	; avoid multiple inclusion
__spiinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI.INC                                                 *
;*                                                                          *
;*  Contains SFR and Bit Definitions for ST62xx Serial Peripheral Interface *
;*                                                                          *
;****************************************************************************

SDSR		sfr	0ddh		; SPI Data/Shift Register
SIDR		sfr	0dch		; SPI Interrupt Disable Register

		restore
		endif			; __spiinc
