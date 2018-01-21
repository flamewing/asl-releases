		ifndef	__regusartc8inc
__regusartc8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File USARTC8.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for USRT0 @ 0xc8 in SRAM/SFR Space *
;*                                                                          *
;****************************************************************************

UDR1		sfr	0xce		; USART1 I/O Data Register

UCSR1A		sfr	0xc8		; USART1 Control & Status Register A
MPCM1		avrbit	UCSR1A,0	; USART1 Multi Processor Communication Mode
U2X1		avrbit	UCSR1A,1	; USART1 Double Transmission Speed
UPE1		avrbit	UCSR1A,2	; USART1 Parity Error
DOR1		avrbit	UCSR1A,3	; USART1 Overrun
FE1		avrbit	UCSR1A,4	; USART1 Framing Error
UDRE1		avrbit	UCSR1A,5	; USART1 Data Register Empty
TXC1		avrbit	UCSR1A,6	; USART1 Transmit Complete
RXC1		avrbit	UCSR1A,7	; USART1 Receive Complete

UCSR1B		sfr	0xc9		; USART1 Control & Status Register B
TXB81		avrbit	UCSR1B,0	; USART1 Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; USART1 Receive Bit 8
UCSZ12		avrbit	UCSR1B,2	; USART1 Character Size
TXEN1		avrbit	UCSR1B,3	; USART1 Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; USART1 Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; USART1 Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; USART1 Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; USART1 Enable Receive Complete Interrupt

UCSR1C		sfr	0xca		; USART1 Control & Status Register C
UCPOL1		avrbit	UCSR1C,0	; USART1 Clock Polarity
UCSZ10		avrbit	UCSR1C,1	; USART1 Character Size
UCSZ11		avrbit	UCSR1C,2
USBS1		avrbit	UCSR1C,3	; USART1 Stop Bit Select
UPM10		avrbit	UCSR1C,4	; USART1 Parity Mode : Odd/Even
UPM11		avrbit	UCSR1C,5	; USART1 Parity Mode : Enable/Disable
UMSEL10		avrbit	UCSR1C,6	; USART1 USART Mode Select
UMSEL11		avrbit	UCSR1C,7

UBRR1H		sfr	0xcd		; USART1 Baud Rate Register MSB
UBRR1L		sfr	0xcc		; USART1 Baud Rate Register LSB

		restore			; re-enable listing

		endif			; __regusartc8inc
