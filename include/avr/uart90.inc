		ifndef	__reguart90inc
__reguart90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGUART90.INC                                           *
;*                                                                          *
;*   Contains bit & register definitions for AT90Sxxxx UART                 *
;*                                                                          *
;****************************************************************************

UDR		port	0x0c		; UART I/O Data Register

USR		port	0x0b		; UART Status Register
OR		equ	3		; Overrun
FE		equ	4		; Framing Error
UDRE		equ	5		; Data Register Empty
TXC		equ	6		; Transmit Complete
RXC		equ	7		; Receive Complete

UCR		port	0x0a		; UART Control Register
TXB8		equ     0		; transmit bit 8
RXB8		equ     1		; receive bit 8
CHR9		equ     2		; 9 bit characters
TXEN		equ     3		; enable transmitter
RXEN		equ     4		; enable receiver
UDRIE		equ     5		; enable data register empty interrupt
TXCIE		equ     6		; enable transmit complete interrupt
RXCIE		equ     7		; enable receive complete interrupt

UBRR		port	0x09		; baud rate register

		restore			; re-enable listing

		endif			; __reguart90inc
