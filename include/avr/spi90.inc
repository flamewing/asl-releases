		ifndef	__spi90inc
__spi90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI90.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90Sxx SPI Interface          *
;*                                                                          *
;****************************************************************************

SPCR		port	0x0d		; SPI Control Register
SPR0		avrbit	SPCR,0		; Clock Select
SPR1		avrbit	SPCR,1
CPHA		avrbit	SPCR,2		; Clock Phase
CPOL		avrbit	SPCR,3		; Clock Polarity
MSTR		avrbit	SPCR,4		; Master/Slave Selection
DORD		avrbit	SPCR,5		; Bit Order
SPE		avrbit	SPCR,6		; Enable SPI
SPIE		avrbit	SPCR,7		; SPI Interrupt Enable

SPSR		port	0x0e		; SPI Status Register
WCOL		avrbit	SPSR,6		; Write Collision
SPIF		avrbit	SPSR,7		; SPI Interrupt Occured?

SPDR		port	0x0f		; SPI Data Register

		restore			; re-enable listing

		endif			; __spi90inc
