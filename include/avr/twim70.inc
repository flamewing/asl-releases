		ifndef	__twim70inc
__twiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTWIM70.INC                                           *
;*                                                                          *
;* Contains Bit & Register Definitions for newer ATmega TWI Interface @0x70 *
;*                                                                          *
;****************************************************************************

TWBR		sfr	0x70		; Bit Rate Register
TWBR0		avrbit	TWBR,0
TWBR1		avrbit	TWBR,1
TWBR2		avrbit	TWBR,2
TWBR3		avrbit	TWBR,3
TWBR4		avrbit	TWBR,4
TWBR5		avrbit	TWBR,5
TWBR6		avrbit	TWBR,6
TWBR7		avrbit	TWBR,7

TWCR		sfr	0x74		; Control Register
TWINT		avrbit	TWCR,7		; Interrupt Flag
TWEA		avrbit	TWCR,6		; Enable Acknowledge Bit
TWSTA		avrbit	TWCR,5		; Start Condition
TWSTO		avrbit	TWCR,4		; Stop Condition
TWWC		avrbit	TWCR,3		; Write Collision Flag
TWEN		avrbit	TWCR,2		; Enable Bit
TWIE		avrbit	TWCR,0		; Interrupt Enable

TWSR		sfr	0x71		; Status Register
TWS7		avrbit	TWSR,7		; Status
TWS6		avrbit	TWSR,6
TWS5		avrbit	TWSR,5
TWS4		avrbit	TWSR,4
TWS3		avrbit	TWSR,3
TWPS1		avrbit	TWSR,1		; Prescaler
TWPS0		avrbit	TWSR,0

TWDR		sfr	0x73		; Data Register
TWD0		avrbit	TWDR,0
TWD1		avrbit	TWDR,1
TWD2		avrbit	TWDR,2
TWD3		avrbit	TWDR,3
TWD4		avrbit	TWDR,4
TWD5		avrbit	TWDR,5
TWD6		avrbit	TWDR,6
TWD7		avrbit	TWDR,7

TWAR		sfr	0x72		; (Slave) Address Register
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
