		ifndef	__stm8sspiinc	; avoid multiple inclusion
__stm8sspiinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S SPI                         *
;*                                                                          *
;****************************************************************************

__defspi	macro	Base
SPI_CR1		label	Base+$00	; SPI control register 1
SPI_LSBFIRST	bit	SPI_CR1,7	;  Frame format
SPI_SPE		bit	SPI_CR1,6	;  SPI enable
SPI_BR		bfield	SPI_CR1,3,3	;  Baud rate control
SPI_MSTR	bit	SPI_CR1,2	;  Master selection
SPI_CPOL	bit	SPI_CR1,1	;  Clock polarity
SPI_CPHA	bit	SPI_CR1,0	;  Clock phase
SPI_CR2		label	Base+$01	; SPI control register 2
SPI_BDM		bit	SPI_CR2,7	;  Bidirectional data mode enable
SPI_BDOE	bit	SPI_CR2,6	;  Input/Output enable in bidirectional mode
SPI_CRCEN	bit	SPI_CR2,5	;  Hardware CRC calculation enable
SPI_CRCNEXT	bit	SPI_CR2,4	;  Transmit CRC next
SPI_RXONLY	bit	SPI_CR2,2	;  Receive only
SPI_SSM		bit	SPI_CR2,1	;  Software slave management
SPI_SSI		bit	SPI_CR2,0	;  Internal slave select
SPI_ICR		label	Base+$02	; SPI interrupt control register
SPI_TXIE	bit	SPI_ICR,7	;  Tx buffer empty interrupt enable
SPI_RXIE	bit	SPI_ICR,6	;  RX buffer not empty interrupt enable
SPI_ERRIE	bit	SPI_ICR,5	;  Error interrupt enable
SPI_WKIE	bit	SPI_ICR,4	;  Wakeup interrupt enable
SPI_SR		label	Base+$03	; SPI status register
SPI_BSY		bit	SPI_SR,7	;  Busy flag
SPI_OVR		bit	SPI_SR,6	;  Overrun flag
SPI_MODF	bit	SPI_SR,5	;  Mode fault
SPI_CRCERR	bit	SPI_SR,4	;  CRC error flag
SPI_WKUP	bit	SPI_SR,3	;  Wakeup flag
SPI_TXE		bit	SPI_SR,1	;  Transmit buffer empty
SPI_RXNE	bit	SPI_SR,0	;  Receive buffer not empty
SPI_DR		label	Base+$04	; SPI data register
SPI_CRCPR	label	Base+$05	; SPI CRC polynomial register
SPI_RXCRCR	label	Base+$06	; SPI Rx CRC register
SPI_TXCRCR	label	Base+$07	; SPI Tx CRC register
		endm

		restore
		endif			; __stm8sspiinc
