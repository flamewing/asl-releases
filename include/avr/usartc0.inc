		ifndef	__regusartc0inc
__regusartc0inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File USARTC0.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for USRT0 @ 0xc0 in SRAM/SFR Space *
;*                                                                          *
;****************************************************************************

UDR0		sfr	0xc6		; USART0 I/O Data Register

UCSR0A		sfr	0xc0		; USART0 Control & Status Register A
MPCM0		avrbit	UCSR0A,0	; USART0 Multi Processor Communication Mode
U2X0		avrbit	UCSR0A,1	; USART0 Double Transmission Speed
UPE0		avrbit	UCSR0A,2	; USART0 Parity Error
DOR0		avrbit	UCSR0A,3	; USART0 Overrun
FE0		avrbit	UCSR0A,4	; USART0 Framing Error
UDRE0		avrbit	UCSR0A,5	; USART0 Data Register Empty
TXC0		avrbit	UCSR0A,6	; USART0 Transmit Complete
RXC0		avrbit	UCSR0A,7	; USART0 Receive Complete

UCSR0B		sfr	0xc1		; USART0 Control & Status Register B
TXB80		avrbit	UCSR0B,0	; USART0 Transmit Bit 8
RXB80		avrbit	UCSR0B,1	; USART0 Receive Bit 8
UCSZ02		avrbit	UCSR0B,2	; USART0 Character Size
TXEN0		avrbit	UCSR0B,3	; USART0 Enable transmitter
RXEN0		avrbit	UCSR0B,4	; USART0 Enable receiver
UDRIE0		avrbit	UCSR0B,5	; USART0 Enable Data Register Empty Interrupt
TXCIE0		avrbit	UCSR0B,6	; USART0 Enable transmit Complete Interrupt
RXCIE0		avrbit	UCSR0B,7	; USART0 Enable receive Complete Interrupt

UCSR0C		sfr	0xc2		; USART0 Control & Status Register C
UCPOL0		avrbit	UCSR0C,0	; USART0 Clock Polarity
UCSZ00		avrbit	UCSR0C,1	; USART0 Character Size
UCSZ01		avrbit	UCSR0C,2
USBS0		avrbit	UCSR0C,3	; USART0 Stop Bit Select
UPM00		avrbit	UCSR0C,4	; USART0 Parity Mode : Odd/Even
UPM01		avrbit	UCSR0C,5	; USART0 Parity Mode : Enable/Disable
		ifdef	__USART0_SPI__
UMSEL00		 avrbit	UCSR0C,6	; USART0 Mode Select
UMSEL01		 avrbit	UCSR0C,7
		elseif
UMSEL0		 avrbit	UCSR0C,6	; USART0 Mode Select
		endif

UBRR0H		sfr	0xc5		; USART0 Baud Rate Register MSB
UBRR0L		sfr	0xc4		; USART0 Baud Rate Register LSB

		restore			; re-enable listing

		endif			; __regusartc0inc
