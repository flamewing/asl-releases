		ifndef	__spim02cinc
__spim0inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPIM02C.INC                                          *
;*                                                                          *
;*   Contains SPI0 bit & Register definitions for newer ATmegas             *
;*                                                                          *
;****************************************************************************

SPCR0		port	0x2c		; SPI0 Control Register
SPR00		avrbit	SPCR0,0		; Clock Select
SPR01		avrbit	SPCR0,1
CPHA0		avrbit	SPCR0,2		; Clock Phase
CPOL0		avrbit	SPCR0,3		; Clock Polarity
MSTR0		avrbit	SPCR0,4		; Master/Slave Selection
DORD0		avrbit	SPCR0,5		; Bit Order
SPE0		avrbit	SPCR0,6		; Enable SPI
SPIE0		avrbit	SPCR0,7		; SPI Interrupt Enable

SPSR0		port	0x2d		; SPI0 Status Register
SPI2X0		avrbit	SPSR0,0		; Double Speed Mode
WCOL0		avrbit	SPSR0,6		; Write Collision
SPIF0		avrbit	SPSR0,7		; SPI0 Interrupt Occured?

SPDR0		port	0x2e		; SPI0 Data Register

		restore			; re-enable listing

		endif			; __spim02cinc
