		ifndef	__spim02cinc
__spim0inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPIM02C.INC                                          *
;*                                                                          *
;*   Contains SPI0 bit & register definitions for newer ATmegas             *
;*                                                                          *
;****************************************************************************

SPCR0		port	0x2c		; SPI0 control register
SPR00		equ	0		; clock select
SPR01		equ	1
CPHA0		equ	2		; clock phase
CPOL0		equ	3		; clock polarity
MSTR0		equ	4		; master/slave selection
DORD0		equ	5		; bit order
SPE0		equ	6		; enable SPI
SPIE0		equ	7		; SPI interrupt enable

SPSR0		port	0x2d		; SPI0 status register
SPI2X0		equ	0		; double speed mode
WCOL0		equ	6		; write collision
SPIF0		equ	7		; SPI0 interrupt occured?

SPDR0		port	0x2e		; SPI0 data register

		restore			; re-enable listing

		endif			; __spim02cinc
