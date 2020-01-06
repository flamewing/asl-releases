		ifndef	__uartinc	; avoid multiple inclusion
__uartinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File UART.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx UART                       *
;*                                                                          *
;****************************************************************************

UARTDR		sfr	0d6h		; UART Data Register
UARTCR		sfr	0d7h		; UART Control Register
RXRDY		bit	7,UARTCR	;  Receiver Ready
TXMT		bit	6,UARTCR	;  Transmitter Empty
RXIEN		bit	5,UARTCR	;  Receive Interrupt Enable
TXIEN		bit	4,UARTCR	;  Transmit Interrupt Enable
BR		bfield	UARTCR,1,3	;  Baudrate Select
PTYEN		bit	0,UARTCR	;  Parity/Data Bit 8

		restore
		endif			; __uartinc
