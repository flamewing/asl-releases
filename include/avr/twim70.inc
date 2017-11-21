		ifndef	__twim70inc
__twiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTWIM70.INC                                           *
;*                                                                          *
;* Contains bit & register definitions for newer ATmega TWI Interface @0x70 *
;*                                                                          *
;****************************************************************************

TWBR		sfr	0x70		; bit rate register
TWBR0		equ	0
TWBR1		equ	1
TWBR2		equ	2
TWBR3		equ	3
TWBR4		equ	4
TWBR5		equ	5
TWBR6		equ	6
TWBR7		equ	7

TWCR		sfr	0x74		; control register
TWINT		equ	7		; interrupt flag
TWEA		equ	6		; enable acknowledge bit
TWSTA		equ	5		; start condition
TWSTO		equ	4		; stop condition
TWWC		equ	3		; write collision flag
TWEN		equ	2		; enable bit
TWIE		equ	0		; interupt enable

TWSR		sfr	0x71		; status register
TWS7		equ	7		; status
TWS6		equ	6
TWS5		equ	5
TWS4		equ	4
TWS3		equ	3
TWPS1		equ	1		; prescaler
TWPS0		equ	0

TWDR		sfr	0x73		; data register
TWD0		equ	0
TWD1		equ	1
TWD2		equ	2
TWD3		equ	3
TWD4		equ	4
TWD5		equ	5
TWD6		equ	6
TWD7		equ	7

TWAR		sfr	0x72		; (slave) address register
TWGCE		equ	0		; general call recognition bit
TWA0		equ	1		; slave address	
TWA1		equ	2
TWA2		equ	3
TWA3		equ	4
TWA4		equ	5
TWA5		equ	6
TWA6		equ	7

		restore			; re-enable listing

		endif			; __twiminc
