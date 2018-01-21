		ifndef	__regusartminc
__regusartminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGUSARTM.INC                                           *
;*                                                                          *
;*   Contains Bit & Register Definitions for Early ATmega Single USART      *
;*                                                                          *
;****************************************************************************

UDR		port	0x0c		; I/O Data Register

UCSRA		port	0x0b		; Control & Status Register A
MPCM		avrbit	UCSRA,0		; Multi Processor Communication Mode
U2X		avrbit	UCSRA,1		; Double Transmission Speed
PE		avrbit	UCSRA,2		; Parity Error
DOR		avrbit	UCSRA,3		; Overrun
FE		avrbit	UCSRA,4		; Framing Error
UDRE		avrbit	UCSRA,5		; Data Register Empty
TXC		avrbit	UCSRA,6		; Transmit Complete
RXC		avrbit	UCSRA,7		; Receive Complete

UCSRB		port	0x0a		; Control & Status Register B
TXB8		avrbit	UCSRB,0		; Transmit Bit 8
RXB8		avrbit	UCSRB,1		; Receive Bit 8
UCSZ2		avrbit	UCSRB,2		; Character Size
TXEN		avrbit	UCSRB,3		; Enable Transmitter
RXEN		avrbit	UCSRB,4		; Enable Receiver
UDRIE		avrbit	UCSRB,5		; Enable Data Register Empty Interrupt
TXCIE		avrbit	UCSRB,6		; Enable Transmit Complete Interrupt
RXCIE		avrbit	UCSRB,7		; Enable Receive Complete Interrupt

UCSRC		port	0x20		; Control & Status Register C
UCPOL		avrbit	UCSRC,0		; Clock Polarity
UCSZ0		avrbit	UCSRC,1		; Character Size
UCSZ1		avrbit	UCSRC,2
USBS		avrbit	UCSRC,3		; Stop Bit Select
UPM0		avrbit	UCSRC,4		; Parity Mode : Odd/Even
UPM1		avrbit	UCSRC,5		; Parity Mode : Enable/Disable
UMSEL		avrbit	UCSRC,6		; USART Mode Select
URSEL		avrbit	UCSRC,7		; Register Select (1 for UCSRC)

UBRRH		port	0x20		; Baud Rate Register MSB (Overlayed with UCSRC)
UBRRL		port	0x09		; Baud Rate Register LSB

		restore			; re-enable listing

		endif			; __regusartminc
