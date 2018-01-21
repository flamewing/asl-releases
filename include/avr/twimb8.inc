		ifndef	__twimb8inc
__twiminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTWIMB8.INC                                           *
;*                                                                          *
;* Contains Bit & Register Definitions for newer ATmega TWI Interface @0xb8 *
;*                                                                          *
;****************************************************************************

TWBR		sfr	0xb8		; Bit Rate Register
TWBR0		avrbit	TWBR,0
TWBR1		avrbit	TWBR,1
TWBR2		avrbit	TWBR,2
TWBR3		avrbit	TWBR,3
TWBR4		avrbit	TWBR,4
TWBR5		avrbit	TWBR,5
TWBR6		avrbit	TWBR,6
TWBR7		avrbit	TWBR,7

TWCR		sfr	0xbc		; Control Register
TWINT		avrbit	TWCR,7		; Interrupt Flag
TWEA		avrbit	TWCR,6		; Enable Acknowledge Bit
TWSTA		avrbit	TWCR,5		; Start Condition
TWSTO		avrbit	TWCR,4		; Stop Condition
TWWC		avrbit	TWCR,3		; Write Collision Flag
TWEN		avrbit	TWCR,2		; Enable Bit
TWIE		avrbit	TWCR,0		; Interupt Enable

TWSR		sfr	0xb9		; Status Register
TWS7		avrbit	TWSR,7		; Status
TWS6		avrbit	TWSR,6
TWS5		avrbit	TWSR,5
TWS4		avrbit	TWSR,4
TWS3		avrbit	TWSR,3
TWPS1		avrbit	TWSR,1		; Prescaler
TWPS0		avrbit	TWSR,0

TWDR		sfr	0xbb		; Data Register
TWD0		avrbit	TWDR,0
TWD1		avrbit	TWDR,1
TWD2		avrbit	TWDR,2
TWD3		avrbit	TWDR,3
TWD4		avrbit	TWDR,4
TWD5		avrbit	TWDR,5
TWD6		avrbit	TWDR,6
TWD7		avrbit	TWDR,7

TWAR		sfr	0xba		; (Slave) Address Register
TWGCE		avrbit	TWAR,0		; General Call Recognition Bit
TWA0		avrbit	TWAR,1		; Slave Address	
TWA1		avrbit	TWAR,2
TWA2		avrbit	TWAR,3
TWA3		avrbit	TWAR,4
TWA4		avrbit	TWAR,5
TWA5		avrbit	TWAR,6
TWA6		avrbit	TWAR,7

TWAMR		sfr	0xbd		; (Slave) Address Mask Register
TWAM0		avrbit	TWAMR,1
TWAM1		avrbit	TWAMR,2
TWAM2		avrbit	TWAMR,3
TWAM3		avrbit	TWAMR,4
TWAM4		avrbit	TWAMR,5
TWAM5		avrbit	TWAMR,6
TWAM6		avrbit	TWAMR,7

		restore			; re-enable listing

		endif			; __twiminc
