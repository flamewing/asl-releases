		ifndef	__stm8lspi01inc	; avoid multiple inclusion
__stm8lspi01inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI01.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 SPI                      *
;*                                                                          *
;****************************************************************************

__defspi01	macro	NAME,Base
__NS		set	"\{NAME}_"
{__NS}CR1	label	Base+$00	; control register 1
{__NS}LSBFIRST	bit	{__NS}CR1,7	;  Frame format
{__NS}SPE	bit	{__NS}CR1,6	;  enable
{__NS}BR	bfield	{__NS}CR1,3,3	;  Baud rate control
{__NS}MSTR	bit	{__NS}CR1,2	;  Master selection
{__NS}CPOL	bit	{__NS}CR1,1	;  Clock polarity
{__NS}CPHA	bit	{__NS}CR1,0	;  Clock phase
{__NS}CR2	label	Base+$01	; control register 2
{__NS}BDM	bit	{__NS}CR2,7	;  Bidirectional data mode enable
{__NS}BDOE	bit	{__NS}CR2,6	;  Input/Output enable in bidirectional mode
{__NS}CRCEN	bit	{__NS}CR2,5	;  Hardware CRC calculation enable
{__NS}CRCNEXT	bit	{__NS}CR2,4	;  Transmit CRC next
{__NS}RXONLY	bit	{__NS}CR2,2	;  Receive only
{__NS}SSM	bit	{__NS}CR2,1	;  Software slave management
{__NS}SSI	bit	{__NS}CR2,0	;  Internal slave select
{__NS}ICR	label	Base+$02	; interrupt control register
{__NS}TXIE	bit	{__NS}ICR,7	;  Tx buffer empty interrupt enable
{__NS}RXIE	bit	{__NS}ICR,6	;  RX buffer not empty interrupt enable
{__NS}ERRIE	bit	{__NS}ICR,5	;  Error interrupt enable
{__NS}WKIE	bit	{__NS}ICR,4	;  Wakeup interrupt enable
{__NS}TXDMAEN	bit	{__NS}ICR,1	;  Tx Buffer DMA Enable
{__NS}RXDMAEN	bit	{__NS}ICR,0	;  Rx Buffer DMA Enable
{__NS}SR	label	Base+$03	; status register
{__NS}BSY	bit	{__NS}SR,7	;  Busy flag
{__NS}OVR	bit	{__NS}SR,6	;  Overrun flag
{__NS}MODF	bit	{__NS}SR,5	;  Mode fault
{__NS}CRCERR	bit	{__NS}SR,4	;  CRC error flag
{__NS}WKUP	bit	{__NS}SR,3	;  Wakeup flag
{__NS}TXE	bit	{__NS}SR,1	;  Transmit buffer empty
{__NS}RXNE	bit	{__NS}SR,0	;  Receive buffer not empty
{__NS}DR	label	Base+$04	; data register
		endm

		restore
		endif			; __stm8lspi01inc
