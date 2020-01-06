		ifndef	__stm8li2c01inc	; avoid multiple inclusion
__stm8li2c01inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File I2C01.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 I2C                      *
;*                                                                          *
;****************************************************************************

__defi2c01	macro	NAME,Base,HasDual
__NS		set	"\{NAME}_"
{__NS}CR1	label	Base+0		; control register 1
{__NS}NOSTRETCH	bit	{__NS}CR1,7	;  Clock stretching disable (Slave mode)
{__NS}ENGC	bit	{__NS}CR1,6	;  General call enable
{__NS}ENPEC	bit	{__NS}CR1,5	;  PEC enable
{__NS}ENARP	bit	{__NS}CR1,4	;  ARP enable
{__NS}SMBTYPE	bit	{__NS}CR1,3	;  SMBus type
{__NS}SMBUS	bit	{__NS}CR1,1	;  SMBus mode
{__NS}PE	bit	{__NS}CR1,0	;  Peripheral enable
{__NS}CR2	label	Base+1		; control register 2
{__NS}SWRST	bit	{__NS}CR2,7	;  Software reset
{__NS}ALERT	bit	{__NS}CR2,5	;  SMBus alert
{__NS}PEC	bit	{__NS}CR2,4	;  Packet error checking
{__NS}POS	bit	{__NS}CR2,3	;  Acknowledge position (for data reception)
{__NS}ACK	bit	{__NS}CR2,2	;  Acknowledge enable
{__NS}STOP	bit	{__NS}CR2,1	;  Stop generation
{__NS}START	bit	{__NS}CR2,0	;  Start generation
{__NS}FREQR	label	Base+2		; frequency register
		if	HasDual
{__NS}OAR1L	 label	Base+3		; own address register low
{__NS}OAR1H	 label	Base+4		; own address register high
{__NS}ADDMODE	 bit	{__NS}OAR1H,7	;  Address mode configuration
{__NS}ADDCONF 	 bit	{__NS}OAR1H,6	;  Addressing mode (Slave mode)
{__NS}OAR2	 label	Base+5		; own address register for dual mode
{__NS}ENDUAL	 bit	{__NS}OAR2,0	;  Dual addressing mode enable
		elseif
{__NS}OARL	 label	Base+3		; own address register low
{__NS}OARH	 label	Base+4		; own address register high
{__NS}ADDMODE	 bit	{__NS}OARH,7	;  Address mode configuration
{__NS}ADDCONF 	 bit	{__NS}OARH,6	;  Addressing mode (Slave mode)
		endif
{__NS}DR	label	Base+6		; data register
{__NS}SR1	label	Base+7		; status register 1
{__NS}TXE	bit	{__NS}SR1,7	;  Data register empty (transmitters)
{__NS}RXNE	bit	{__NS}SR1,6	;  Data register not empty (receivers)
{__NS}STOPF	bit	{__NS}SR1,4	;  Stop detection (Slave mode)
{__NS}ADD10	bit	{__NS}SR1,3	;  10-bit header sent (Master mode)
{__NS}BTF	bit	{__NS}SR1,2	;  Byte transfer finished
{__NS}ADDR	bit	{__NS}SR1,1	;  Address sent (master mode)/matched (slave mode)
{__NS}SB	bit	{__NS}SR1,0	;  Start bit (Master mode)
{__NS}SR2	label	Base+8		; status register 2
{__NS}SMBALERT	bit	{__NS}SR2,7	;  SMBus alert
{__NS}TIMEOUT	bit	{__NS}SR2,6	;  Timeout or Tlow error
{__NS}WUFH	bit	{__NS}SR2,5	;  Wakeup from Halt
{__NS}PECERR	bit	{__NS}SR2,4	;  PEC Error in reception
{__NS}OVR	bit	{__NS}SR2,3	;  Overrun/underrun
{__NS}AF	bit	{__NS}SR2,2	;  Acknowledge failure
{__NS}ARLO	bit	{__NS}SR2,1	;  Arbitration lost (master mode)
{__NS}BERR	bit	{__NS}SR2,0	;  Bus error
{__NS}SR3	label	Base+9		; status register 3
{__NS}DUALF	bit	{__NS}SR3,7	;  Dual flag (Slave mode)
{__NS}SMBHOST	bit	{__NS}SR3,6
{__NS}SMBDEFAULT bit	{__NS}SR3,5
{__NS}GENCALL	bit	{__NS}SR3,4	;  General call header (Slave mode)
{__NS}TRA	bit	{__NS}SR3,2	;  Transmitter/Receiver
{__NS}BUSY	bit	{__NS}SR3,1	;  Bus busy
{__NS}MSL	bit	{__NS}SR3,0	;  Master/Slave
{__NS}ITR	label	Base+$a		; interrupt control register
{__NS}LAST	bit	{__NS}ITR,4	;  DMA last transfer
{__NS}DMAEN	bit	{__NS}ITR,3	;  DMA requests enable
{__NS}ITBUFEN	bit	{__NS}ITR,2	;  Buffer interrupt enable
{__NS}ITEVTEN	bit	{__NS}ITR,1	;  Event interrupt enable
{__NS}ITERREN	bit	{__NS}ITR,0	;  Error interrupt enable
{__NS}CCRL	label	Base+$b		; clock control register low
{__NS}CCRH	label	Base+$c		; clock control register high
{__NS}FS	bit	{__NS}CCRH,7	;  master mode selection
{__NS}DUTY	bit	{__NS}CCRH,6	;  Fast mode duty cycle
{__NS}TRISER	label	Base+$d		; TRISE register
		endm

		restore
		endif			; __stm8li2c01inc
