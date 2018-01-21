		ifndef	__spim2cinc
__spim2cinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSPI2C.INC                                            *
;*                                                                          *
;*   Contains SPI Bit & Register Definitions for Newer ATmegas              *
;*                                                                          *
;****************************************************************************

SPCR		port	0x2c		; SPI Control Register
SPR0		avrbit	SPCR,0		; Clock Select
SPR1		avrbit	SPCR,1
CPHA		avrbit	SPCR,2		; Clock phase
CPOL		avrbit	SPCR,3		; Clock polarity
MSTR		avrbit	SPCR,4		; Master/Slave Selection
DORD		avrbit	SPCR,5		; Bit Order
SPE		avrbit	SPCR,6		; Enable SPI
SPIE		avrbit	SPCR,7		; SPI Interrupt Enable

SPSR		port	0x2d		; SPI Status Register
SPI2X		avrbit	SPSR,0		; Double Speed Mode
WCOL		avrbit	SPSR,6		; Write Collision
SPIF		avrbit	SPSR,7		; SPI Interrupt Occured?

SPDR		port	0x2e		; SPI Data Register

		restore			; re-enable listing

		endif			; __spim2cinc
