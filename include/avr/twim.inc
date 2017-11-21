		ifndef	__twiminc
__twiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTWIM.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for older ATmega TWI Interface     *
;*                                                                          *
;****************************************************************************

TWBR		port	0x00		; bit rate register

TWCR		port	0x36		; control register
TWINT		equ	7		; interrupt flag
TWEA		equ	6		; enable acknowledge bit
TWSTA		equ	5		; start condition
TWSTO		equ	4		; stop condition
TWWC		equ	3		; write collision flag
TWEN		equ	2		; enable bit
TWIE		equ	0		; interupt enable

TWSR		port	0x01		; status register
TWS7		equ	7		; status
TWS6		equ	6
TWS5		equ	5
TWS4		equ	4
TWS3		equ	3
TWPS1		equ	1		; prescaler
TWPS0		equ	0

TWDR		port	0x03		; data register

TWAR		port	0x02		; (slave) address register
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
