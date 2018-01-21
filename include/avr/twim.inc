		ifndef	__twiminc
__twiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTWIM.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for older ATmega TWI Interface     *
;*                                                                          *
;****************************************************************************

TWBR		port	0x00		; Bit Rate Register

TWCR		port	0x36		; Control Register
TWINT		avrbit	TWCR,7		; Interrupt Flag
TWEA		avrbit	TWCR,6		; Enable acknowledge bit
TWSTA		avrbit	TWCR,5		; Start Condition
TWSTO		avrbit	TWCR,4		; Stop Condition
TWWC		avrbit	TWCR,3		; Write Collision Flag
TWEN		avrbit	TWCR,2		; Enable bit
TWIE		avrbit	TWCR,0		; Interrupt Enable

TWSR		port	0x01		; Status Register
TWS7		avrbit	TWSR,7		; Status
TWS6		avrbit	TWSR,6
TWS5		avrbit	TWSR,5
TWS4		avrbit	TWSR,4
TWS3		avrbit	TWSR,3
TWPS1		avrbit	TWSR,1		; Prescaler
TWPS0		avrbit	TWSR,0

TWDR		port	0x03		; Data Register

TWAR		port	0x02		; (Slave) Address Register
TWGCE		avrbit	TWAR,0		; General Call Recognition Bit
TWA0		avrbit	TWAR,1		; Slave Address	
TWA1		avrbit	TWAR,2
TWA2		avrbit	TWAR,3
TWA3		avrbit	TWAR,4
TWA4		avrbit	TWAR,5
TWA5		avrbit	TWAR,6
TWA6		avrbit	TWAR,7

		restore			; re-enable listing

		endif			; __twiminc
