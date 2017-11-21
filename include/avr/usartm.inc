		ifndef	__regusartminc
__regusartminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGUSARTM.INC                                           *
;*                                                                          *
;*   Contains bit & register definitions for early ATmega single USART      *
;*                                                                          *
;****************************************************************************

UDR		port	0x0c		; I/O Data Register

UCSRA		port	0x0b		; Control & Status Register A
MPCM		equ	0		; multi processor communication mode
U2X		equ	1		; double transmission speed
PE		equ	2		; parity error
DOR		equ	3		; Overrun
FE		equ	4		; Framing Error
UDRE		equ	5		; Data Register Empty
TXC		equ	6		; Transmit Complete
RXC		equ	7		; Receive Complete

UCSRB		port	0x0a		; Control & Status Register B
TXB8		equ     0		; transmit bit 8
RXB8		equ     1		; receive bit 8
UCSZ2		equ     2		; character size
TXEN		equ     3		; enable transmitter
RXEN		equ     4		; enable receiver
UDRIE		equ     5		; enable data register empty interrupt
TXCIE		equ     6		; enable transmit complete interrupt
RXCIE		equ     7		; enable receive complete interrupt

UCSRC		port	0x20		; Control & Status Register C
UCPOL		equ	0		; clock polarity
UCSZ0		equ	1		; character size
UCSZ1		equ	2
USBS		equ	3		; stop bit select
UPM0		equ	4		; parity mode : odd/even
UPM1		equ	5		; parity mode : enable/disable
UMSEL		equ	6		; USART mode select
URSEL		equ	7		; register select (1 for UCSRC)

UBRRH		port	0x20		; baud rate register high (overlayed with UCSRC)
UBRRL		port	0x09		; baud rate register low

		restore			; re-enable listing

		endif			; __regusartminc
