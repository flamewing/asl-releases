		ifndef	__regusartc0inc
__regusartc0inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSKEL.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for USRT0 @ 0xc0 in SRAM/SFR space *
;*                                                                          *
;****************************************************************************

UDR0		sfr	0xc6		; USART0 I/O Data Register

UCSR0A		sfr	0xc0		; USART0 Control & Status Register A
MPCM0		equ	0		; USART0 multi processor communication mode
U2X0		equ	1		; USART0 double transmission speed
UPE0		equ	2		; USART0 parity error
DOR0		equ	3		; USART0 Overrun
FE0		equ	4		; USART0 Framing Error
UDRE0		equ	5		; USART0 Data Register Empty
TXC0		equ	6		; USART0 Transmit Complete
RXC0		equ	7		; USART0 Receive Complete

UCSR0B		sfr	0xc1		; USART0 Control & Status Register B
TXB80		equ     0		; USART0 transmit bit 8
RXB80		equ     1		; USART0 receive bit 8
UCSZ02		equ     2		; USART0 character size
TXEN0		equ     3		; USART0 enable transmitter
RXEN0		equ     4		; USART0 enable receiver
UDRIE0		equ     5		; USART0 enable data register empty interrupt
TXCIE0		equ     6		; USART0 enable transmit complete interrupt
RXCIE0		equ     7		; USART0 enable receive complete interrupt

UCSR0C		sfr	0xc2		; USART0 Control & Status Register C
UCPOL0		equ	0		; USART0 clock polarity
UCSZ00		equ	1		; USART0 character size
UCSZ01		equ	2
USBS0		equ	3		; USART0 stop bit select
UPM00		equ	4		; USART0 parity mode : odd/even
UPM01		equ	5		; USART0 parity mode : enable/disable
		ifdef	__USART0_SPI__
UMSEL00		 equ	6		; USART0 mode select
UMSEL01		 equ	7
		elseif
UMSEL0		  equ	6		; USART0 mode select
		endif

UBRR0H		sfr	0xc5		; USART0 baud rate register high
UBRR0L		sfr	0xc4		; USART0 baud rate register low

		restore			; re-enable listing

		endif			; __regusartc0inc
