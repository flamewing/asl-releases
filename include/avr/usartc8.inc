		ifndef	__regusartc8inc
__regusartc8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSKEL.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for USRT0 @ 0xc8 in SRAM/SFR space *
;*                                                                          *
;****************************************************************************

UDR1		sfr	0xce		; USART1 I/O Data Register

UCSR1A		sfr	0xc8		; USART1 Control & Status Register A
MPCM1		equ	0		; USART1 multi processor communication mode
U2X1		equ	1		; USART1 double transmission speed
UPE1		equ	2		; USART1 parity error
DOR1		equ	3		; USART1 Overrun
FE1		equ	4		; USART1 Framing Error
UDRE1		equ	5		; USART1 Data Register Empty
TXC1		equ	6		; USART1 Transmit Complete
RXC1		equ	7		; USART1 Receive Complete

UCSR1B		sfr	0xc9		; USART1 Control & Status Register B
TXB81		equ     0		; USART1 transmit bit 8
RXB81		equ     1		; USART1 receive bit 8
UCSZ12		equ     2		; USART1 character size
TXEN1		equ     3		; USART1 enable transmitter
RXEN1		equ     4		; USART1 enable receiver
UDRIE1		equ     5		; USART1 enable data register empty interrupt
TXCIE1		equ     6		; USART1 enable transmit complete interrupt
RXCIE1		equ     7		; USART1 enable receive complete interrupt

UCSR1C		sfr	0xca		; USART1 Control & Status Register C
UCPOL1		equ	0		; USART1 clock polarity
UCSZ10		equ	1		; USART1 character size
UCSZ11		equ	2
USBS1		equ	3		; USART1 stop bit select
UPM10		equ	4		; USART1 parity mode : odd/even
UPM11		equ	5		; USART1 parity mode : enable/disable
UMSEL10		equ	6		; USART1 USART mode select
UMSEL11		equ	7

UBRR1H		sfr	0xcd		; USART1 baud rate register high
UBRR1L		sfr	0xcc		; USART1 baud rate register low

		restore			; re-enable listing

		endif			; __regusartc8inc
