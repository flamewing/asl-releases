		ifndef	__stm8lspiinc	; avoid multiple inclusion
__stm8lspiinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L SPI                         *
;*                                                                          *
;****************************************************************************

		include	"stm8/stm8l/spi01.inc"
__defspi	macro	NAME,Base
		__defspi01 NAME,Base
__NS		set	"\{NAME}_"
{__NS}CRCPR	label	Base+$05	; CRC polynomial register
{__NS}RXCRCR	label	Base+$06	; Rx CRC register
{__NS}TXCRCR	label	Base+$07	; Tx CRC register
		endm

		restore
		endif			; __stm8Lspiinc
