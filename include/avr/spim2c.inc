		ifndef	__spim2cinc
__spim2cinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPI2C.INC                                            *
;*                                                                          *
;*   Contains SPI bit & register definitions for newer ATmegas              *
;*                                                                          *
;****************************************************************************

SPCR		port	0x2c		; SPI control register
SPR0		equ	0		; clock select
SPR1		equ	1
CPHA		equ	2		; clock phase
CPOL		equ	3		; clock polarity
MSTR		equ	4		; master/slave selection
DORD		equ	5		; bit order
SPE		equ	6		; enable SPI
SPIE		equ	7		; SPI interrupt enable

SPSR		port	0x2d		; SPI status register
SPI2X		equ	0		; double speed mode
WCOL		equ	6		; write collision
SPIF		equ	7		; SPI interrupt occured?

SPDR		port	0x2e		; SPI data register

		restore			; re-enable listing

		endif			; __spim2cinc
