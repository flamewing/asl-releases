		ifndef	__reguart90inc
__reguart90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGUART90.INC                                           *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90Sxxxx UART                 *
;*                                                                          *
;****************************************************************************

UDR		port	0x0c		; UART I/O Data Register

USR		port	0x0b		; UART Status Register
OR		avrbit	USR,3		; Overrun
FE		avrbit	USR,4		; Framing Error
UDRE		avrbit	USR,5		; Data Register Empty
TXC		avrbit	USR,6		; Transmit Complete
RXC		avrbit	USR,7		; Receive Complete

UCR		port	0x0a		; UART Control Register
TXB8		avrbit	UCR,0		; Transmit Bit 8
RXB8		avrbit	UCR,1		; Receive Bit 8
CHR9		avrbit	UCR,2		; 9 bit Characters
TXEN		avrbit	UCR,3		; Enable Transmitter
RXEN		avrbit	UCR,4		; Enable Receiver
UDRIE		avrbit	UCR,5		; Enable Data Register Empty Interrupt
TXCIE		avrbit	UCR,6		; Enable Transmit Complete Interrupt
RXCIE		avrbit	UCR,7		; Enable Receive Complete Interrupt

UBRR		port	0x09		; Baud Rate Register

		restore			; re-enable listing

		endif			; __reguart90inc
