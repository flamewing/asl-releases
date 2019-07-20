		ifndef	__st7i2cinc	; avoid multiple inclusion
__st7i2cinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File I2C.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 I2C                           *
;*                                                                          *
;****************************************************************************

__defi2c	macro	Base
I2CCR		label	Base+$00	; Control Register
PE		bit	I2CCR,5		;  Peripheral enable
ENGC		bit	I2CCR,4		;  Enable General Call
START		bit	I2CCR,3		;  Generation of a Start condition
ACK		bit	I2CCR,2		;  Acknowledge enable
STOP		bit	I2CCR,1		;  Generation of a Stop condition
ITE		bit	I2CCR,0		;  Interrupt enable
I2CSR1		label	Base+$01	; Status Register 1
EVF		bit	I2CSR1,7	;  Event flag
ADD10		bit	I2CSR1,6	;  10-bit addressing in Master mode
TRA		bit	I2CSR1,5	;  Transmitter/Receiver
BUSY		bit	I2CSR1,4	;  Bus busy
BTF		bit	I2CSR1,3	;  Byte transfer finished
ADSL		bit	I2CSR1,2	;  Address matched (Slave mode)
M_SL		bit	I2CSR1,1	;  Master/Slave
SB		bit	I2CSR1,0	;  Start bit (Master mode)
I2CSR2		label	Base+$02	; Status Register 2
AF		bit	I2CSR2,4	;  Acknowledge failure
STOPF		bit	I2CSR2,3	;  Stop detection (Slave mode)
ARLO		bit	I2CSR2,2	;  Arbitration lost
BERR		bit	I2CSR2,1	;  Bus error
GCAL		bit	I2CSR2,0	;  General Call (Slave mode)
I2CCCR		label	Base+$03	; Clock Control Register
FM_SM		bit	I2CCCR,7	;  Fast/Standard I2C mode
I2COAR1		label	Base+$04	; Own Address Register 1
I2COAR2		label	Base+$05	; Own Address Register 2
FR		bfield	I2COAR2,6,2	;  Frequency bits
I2CDR		label	Base+$06	; Data Register
		endm

		restore
		endif			; __st7i2cinc
