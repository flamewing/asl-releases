		ifndef	__stm8si2cinc	; avoid multiple inclusion
__stm8si2cinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File I2C.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S I2C                         *
;*                                                                          *
;****************************************************************************

__defi2c	macro	Base
I2C_CR1		label	Base+0		; I2C control register 1
I2C_NOSTRETCH	bit	I2C_CR1,7	;  Clock stretching disable (Slave mode)
I2C_ENGC	bit	I2C_CR1,6	;  General call enable
I2C_PE		bit	I2C_CR1,0	;  Peripheral enable
I2C_CR2		label	Base+1		; I2C control register 2
I2C_SWRST	bit	I2C_CR2,7	;  Software reset
I2C_POS		bit	I2C_CR2,3	;  Acknowledge position (for data reception)
I2C_ACK		bit	I2C_CR2,2	;  Acknowledge enable
I2C_STOP	bit	I2C_CR2,1	;  Stop generation
I2C_START	bit	I2C_CR2,0	;  Start generation
I2C_FREQR	label	Base+2		; I2C frequency register
I2C_OARL	label	Base+3		; I2C own address register low
I2C_OARH	label	Base+4		; I2C own address register high
I2C_ADDMODE	bit	I2C_OARH,7	;  Address mode configuration
I2C_ADDCONF	bit	I2C_OARH,6	;  Addressing mode (Slave mode)
I2C_DR		label	Base+6		; I2C data register
I2C_SR1		label	Base+7		; I2C status register 1
I2C_TXE		bit	I2C_SR1,7	;  Data register empty (transmitters)
I2C_RXNE	bit	I2C_SR1,6	;  Data register not empty (receivers)
I2C_STOPF	bit	I2C_SR1,4	;  Stop detection (Slave mode)
I2C_ADD10	bit	I2C_SR1,3	;  10-bit header sent (Master mode)
I2C_BTF		bit	I2C_SR1,2	;  Byte transfer finished
I2C_ADDR	bit	I2C_SR1,1	;  Address sent (master mode)/matched (slave mode)
I2C_SB		bit	I2C_SR1,0	;  Start bit (Master mode)
I2C_SR2		label	Base+8		; I2C status register 2
I2C_WUFH	bit	I2C_SR2,5	;  Wakeup from Halt
I2C_OVR		bit	I2C_SR2,3	;  Overrun/underrun
I2C_AF		bit	I2C_SR2,2	;  Acknowledge failure
I2C_ARLO	bit	I2C_SR2,1	;  Arbitration lost (master mode)
I2C_BERR	bit	I2C_SR2,0	;  Bus error
I2C_SR3		label	Base+9		; I2C status register 3
I2C_DUALF	bit	I2C_SR3,7	;  Dual flag (Slave mode)
I2C_GENCALL	bit	I2C_SR3,4	;  General call header (Slave mode)
I2C_TRA		bit	I2C_SR3,2	;  Transmitter/Receiver
I2C_BUSY	bit	I2C_SR3,1	;  Bus busy
I2C_MSL		bit	I2C_SR3,0	;  Master/Slave
I2C_ITR		label	Base+$a		; I2C interrupt control register
I2C_ITBUFEN	bit	I2C_ITR,2	;  Buffer interrupt enable
I2C_ITEVTEN	bit	I2C_ITR,1	;  Event interrupt enable
I2C_ITERREN	bit	I2C_ITR,0	;  Error interrupt enable
I2C_CCRL	label	Base+$b		; I2C clock control register low
I2C_CCRH	label	Base+$c		; I2C clock control register high
I2C_FS		bit	I2C_CCRH,7	;  I2C master mode selection
I2C_DUTY	bit	I2C_CCRH,6	;  Fast mode duty cycle
I2C_TRISER	label	Base+$d		; I2C TRISE register
I2C_PECR	label	Base+$e		; I2C packet error checking register
		endm

		restore
		endif			; __stm8si2cinc
