		ifndef	__spi90inc
__spi90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI90.INC                                               *
;*                                                                          *
;*   Contains bit & register definitions for AT90Sxx SPI Interface          *
;*                                                                          *
;****************************************************************************

SPCR		port	0x0d		; SPI control register
SPR0		equ	0		; clock select
SPR1		equ	1
CPHA		equ	2		; clock phase
CPOL		equ	3		; clock polarity
MSTR		equ	4		; master/slave selection
DORD		equ	5		; bit order
SPE		equ	6		; enable SPI
SPIE		equ	7		; SPI interrupt enable

SPSR		port	0x0e		; SPI status register
WCOL		equ	6		; write collision
SPIF		equ	7		; SPI interrupt occured?

SPDR		port	0x0f		; SPI data register

		restore			; re-enable listing

		endif			; __spi90inc
