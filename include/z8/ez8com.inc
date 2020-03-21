		ifndef	__ez8cominc
__ez8cominc	equ	1
                save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File F0830.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions valid for all Z8encore CPUs        *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; CPU Core

FLAGS		sfr	0ffch		; Flags
RP		sfr	0ffdh		; Register pointer
SPH		sfr	0ffeh		; Stack pointer high byte
SPL		sfr	0fffh		; Stack pointer low byte
SP		sfr	SPH

;----------------------------------------------------------------------------
; GPIO

__defgpio	macro	NUM,Base
P{NUM}ADDR	sfr	Base+0		; Port n (subregister) Address
P{NUM}CTL	sfr	Base+1		; Port n (subregister) Control
P{NUM}DD	sfr	P{NUM}CTL	; Port n Data Direction (subregister 1)
P{NUM}AF	sfr	P{NUM}CTL	; Port n Alternate Function (subregister 2)
P{NUM}OC	sfr	P{NUM}CTL	; Port n Output Control (Open-Drain) (subregister 3)
P{NUM}HDE	sfr	P{NUM}CTL	; Port n High Drive Enable (subregister 4)
P{NUM}SMRE	sfr	P{NUM}CTL	; Port n Stop Mode Recovery Source Enable (subregister 5)
P{NUM}PUE	sfr	P{NUM}CTL	; Port n Pull-Up Enable (subregister 6)
P{NUM}AFS1	sfr	P{NUM}CTL	; Port n Alternate Function Set 1 (subregister 7)
P{NUM}AFS2	sfr	P{NUM}CTL	; Port n Alternate Function Set 2 (subregister 8)
P{NUM}IN	sfr	Base+2		; Port n Input Data
P{NUM}OUT	sfr	Base+3		; Port n Output Data
		endm

;----------------------------------------------------------------------------
; Timer

__deftimer	macro	NUM,Base,HasInpCap,Chain
T{NUM}H		sfr	Base		; Timer High Byte
T{NUM}L		sfr	Base+1		; Timer Low Byte
T{NUM}		sfr	T{NUM}H
T{NUM}RH	sfr	Base+2		; Timer Reload High Byte
T{NUM}RL	sfr	Base+3		; Timer Reload Low Byte
T{NUM}R		sfr	T{NUM}RH
T{NUM}PWMH	sfr	Base+4		; Timer PWM High Byte
T{NUM}PWML	sfr	Base+5		; Timer PWM Low Byte
T{NUM}PWM	sfr	T{NUM}PWMH
T{NUM}CTL0	sfr	Base+6		; Timer Control 0
		if	HasInpCap
T{NUM}MODEHI	 __z8bit    T{NUM}CTL0,7;  Timer Mode
T{NUM}ICONFIG	 __z8bfield T{NUM}CTL0,5,2 ;  Timer Interrupt Configuration
T{NUM}PWMD	 __z8bfield T{NUM}CTL0,1,3 ;  Timer PWM Delay Value
T{NUM}INPCAP	 __z8bit    T{NUM}CTL0,0;  Timer Input Capture Event
		endif
		if	Chain
T{NUM}CSC	 __z8bit    T{NUM}CTL0,4;  Cascade Timers
		endif
T{NUM}CTL1	sfr	Base+7		; Timer Control 1
T{NUM}TEN	__z8bit	T{NUM}CTL1,7	;  Timer Enable
T{NUM}TPOL	__z8bit	T{NUM}CTL1,6	;  Timer Input/Output Polarity
T{NUM}PRES	__z8bfield T{NUM}CTL1,3,3 ;  Prescale Value
T{NUM}TMODE	__z8bfield T{NUM}CTL1,0,3 ;  Timer Mode
		endm

;----------------------------------------------------------------------------
; Multi-Channel Timer

__defmct	macro
MCTH		sfr	0fa0h		; MCT High Byte
MCTL		sfr	0fa1h		; MCT Low Byte
MCT		sfr	MCTH
MCTRH		sfr	0fa2h		; MCT Reload High Byte
MCTRL		sfr	0fa3h		; MCT Reload Low Byte
MCTR		sfr	MCTRH
MCTSA		sfr	0fa4h		; MCT Subaddress
MCTSR0		sfr	0fa5h		; MCT Subregister 0
TCTST		__z8bit	MCTSR0,7	;  Timer Count Status (MCTCTL0, MCTSA=00)
CHST		__z8bit	MCTSR0,6	;  Channel Status (MCTCTL0, MCTSA=00)
TCIEN		__z8bit	MCTSR0,5	;  Timer Count Interrupt Enable (MCTCTL0, MCTSA=00)
TCLKS		__z8bit	MCTSR0,0,3	;  Timer Clock Source (MCTCTL0, MCTSA=00)
CHDEO		__z8bit	MCTSR0,3	;  Channel D Event Flag Overrun (MCTCHS0, MCTSA=01)
CHCEO		__z8bit	MCTSR0,2	;  Channel C Event Flag Overrun (MCTCHS0, MCTSA=01)
CHBEO		__z8bit	MCTSR0,1	;  Channel B Event Flag Overrun (MCTCHS0, MCTSA=01)
CHAEO		__z8bit	MCTSR0,0	;  Channel A Event Flag Overrun (MCTCHS0, MCTSA=01)
CHDH		__z8bfield MCTSR0,0,8	;  Channel D Cap/Com Value High (MCTCHDH, MCTSA=05)
CHCH		__z8bfield MCTSR0,0,8	;  Channel C Cap/Com Value High (MCTCHCH, MCTSA=04)
CHBH		__z8bfield MCTSR0,0,8	;  Channel B Cap/Com Value High (MCTCHBH, MCTSA=03)
CHAH		__z8bfield MCTSR0,0,8	;  Channel A Cap/Com Value High (MCTCHAH, MCTSA=02)
MCTSR1		sfr	0fa6h		; MCT Subregister 1
TEN		__z8bit	MCTSR1,7	;  Timer Enable (MCTCTL1, MCTSA=00)
PRES		__z8bfield MCTSR1,3,3	;  Prescale Value (MCTCTL1, MCTSA=00)
TMODE		__z8bfield MCTSR1,0,2	;  Timer Mode (MCTCTL1, MCTSA=00)
CHDEF		__z8bit	MCTSR1,3	;  Channel D Event Flag (MCTCHS1, MCTSA=01)
CHCEF		__z8bit	MCTSR1,2	;  Channel C Event Flag (MCTCHS1, MCTSA=01)
CHBEF		__z8bit	MCTSR1,1	;  Channel B Event Flag (MCTCHS1, MCTSA=01)
CHAEF		__z8bit	MCTSR1,0	;  Channel A Event Flag (MCTCHS1, MCTSA=01)
CHDL		__z8bfield MCTSR0,0,8	;  Channel D Cap/Com Value Low (MCTCHDL, MCTSA=05)
CHCL		__z8bfield MCTSR0,0,8	;  Channel C Cap/Com Value Low (MCTCHCL, MCTSA=04)
CHBL		__z8bfield MCTSR0,0,8	;  Channel B Cap/Com Value Low (MCTCHBL, MCTSA=03)
CHAL		__z8bfield MCTSR0,0,8	;  Channel A Cap/Com Value Low (MCTCHAL, MCTSA=02)
MCTSR2		sfr	0fa7h		; MCT Subregister 2
CHEN		__z8bit	MCTSR2,7	;  Channel Enable (MCTCHyCTL, MCTSA=02...05)
CHPOL		__z8bit	MCTSR2,6	;  Channel Input/Output Polarity (MCTCHyCTL, MCTSA=02...05)
CHIEN		__z8bit	MCTSR2,5	;  Channel Interrupt Enable (MCTCHyCTL, MCTSA=02...05)
CHUE		__z8bit	MCTSR2,4	;  Channel Update Enable (MCTCHyCTL, MCTSA=02...05)
CHOP		__z8bfield MCTSR2,0,3	;  Channel Operation Method (MCTCHyCTL, MCTSA=02...05)
		endm

;----------------------------------------------------------------------------
; UART

__defuart	macro	NUM,Base
U{NUM}TXD	sfr	Base		; UART Transmit Data Register
U{NUM}RXD	sfr	Base		; UART Receive Data Register
U{NUM}STAT0	sfr	Base+1		; UART Status 0 Register
U{NUM}RDA	__z8bit	U{NUM}STAT0,7	;  Receive Data Available
U{NUM}PE	__z8bit	U{NUM}STAT0,6	;  Parity Error
U{NUM}OE	__z8bit	U{NUM}STAT0,5	;  Overrun Error
U{NUM}FE	__z8bit	U{NUM}STAT0,4	;  Framing Error
U{NUM}BRKD	__z8bit	U{NUM}STAT0,3	;  Break Detect
U{NUM}TDRE	__z8bit	U{NUM}STAT0,2	;  Transmitter Data Register Empty
U{NUM}TXE	__z8bit	U{NUM}STAT0,1	;  Transmitter Empty
U{NUM}CTS	__z8bit	U{NUM}STAT0,0	;  /CTS Signal
U{NUM}STAT1	sfr	Base+4		; UART Status 1 Register
U{NUM}NEWFRM	__z8bit	U{NUM}STAT1,1	;  New Frame
U{NUM}MPRX	__z8bit	U{NUM}STAT1,0	;  Multiprocessor Receive
U{NUM}CTL0	sfr	Base+2		; UART Control 0 Register
U{NUM}TEN	__z8bit	U{NUM}CTL0,7	;  Transmit Enable
U{NUM}REN	__z8bit	U{NUM}CTL0,6	;  Receive Enable
U{NUM}CTSE	__z8bit	U{NUM}CTL0,5	;  CTSE-CTS Enable
U{NUM}PEN	__z8bit	U{NUM}CTL0,4	;  Parity Enable
U{NUM}PSEL	__z8bit	U{NUM}CTL0,3	;  Parity Select
U{NUM}SBRK	__z8bit	U{NUM}CTL0,2	;  Send Break
U{NUM}USTOP	__z8bit	U{NUM}CTL0,1	;  Stop Bit Select
U{NUM}LBEN	__z8bit	U{NUM}CTL0,0	;  Loop Back Enable
U{NUM}CTL1	sfr	Base+3		; UART Control 1 Register
U{NUM}MPMD1	__z8bit	U{NUM}CTL1,7	;  Multiprocessor Mode
U{NUM}MPEN	__z8bit	U{NUM}CTL1,6	;  Multiprocessor (9-bit) Enable
U{NUM}MPMD0	__z8bit	U{NUM}CTL1,5	;  Multiprocessor Mode
U{NUM}MPBT	__z8bit	U{NUM}CTL1,4	;  Multiprocessor Bit Transmit
U{NUM}DEPOL	__z8bit	U{NUM}CTL1,3	;  Driver Enable Polarity
U{NUM}BRGCTL	__z8bit	U{NUM}CTL1,2	;  Baud Rate Control
U{NUM}RDAIRQ	__z8bit	U{NUM}CTL1,1	;  Receive Data Interrupt Enable
U{NUM}IREN	__z8bit	U{NUM}CTL1,0	;  Infrared Encoder/Decoder Enable
U{NUM}ADDR	sfr	Base+5		; UART Address Compare Register
U{NUM}BRH	sfr	Base+6		; UART Baud Rate High Byte Register
U{NUM}BRL	sfr	Base+7		; UART Baud Rate Low Byte Register
U{NUM}BR	sfr	U{NUM}BRH
		endm
		
;----------------------------------------------------------------------------
; I2C

__defi2c	macro	Base
I2CDATA		sfr	Base+0		; I2C Data
I2CSTAT		sfr	Base+1		; I2C Status
I2C_TDRE	__z8bit	I2CSTAT,7	;  Transmit Data Register Empty
RDRF		__z8bit	I2CSTAT,6	;  Receive Data Register Full
ACK		__z8bit	I2CSTAT,5	;  Acknowledge
I10B		__z8bit	I2CSTAT,4	;  10-Bit Address
RD		__z8bit	I2CSTAT,3	;  Read
TAS		__z8bit	I2CSTAT,2	;  Transmit Address State
DSS		__z8bit	I2CSTAT,1	;  Data Shift State
NCKI		__z8bit	I2CSTAT,0	;  NACK Interrupt
I2CCTL		sfr	Base+2		; I2C Control
IEN		__z8bit	I2CCTL,7	;  I2C Enable
START		__z8bit	I2CCTL,6	;  Send Start Condition
STOP		__z8bit	I2CCTL,5	;  Send Stop Condition
I2C_BIRQ	__z8bit	I2CCTL,4	;  Baud Rate Generator Interrupt Request
TXI		__z8bit	I2CCTL,3	;  Enable TDRE interrupts
NAK		__z8bit	I2CCTL,2	;  Send NAK
FLUSH		__z8bit	I2CCTL,1	;  Flush Data
FILTEN		__z8bit	I2CCTL,0	;  I2C Signal Filter Enable
I2CBRH		sfr	Base+3		; I2C Baud Rate High Byte
I2CBRL		sfr	Base+4		; I2C Baud Rate Low Byte
I2CBR		sfr	I2CBRH
I2CDST		sfr	Base+5		; I2C Diagnostic State
SCLIN		__z8bit	I2CDST,7	;  Serial Clock Input
SDAIN		__z8bit	I2CDST,6	;  Serial Data Input
STPCNT		__z8bit	I2CDST,5	;  Stop Count
TXRXSTATE	__z8bfield I2CDST,0,5	;  Internal State
I2CDIAG		sfr	Base+6		; I2C Diagnostic Control
I2C_DIAG	__z8bit	I2CDIAG,0	;  Diagnostic Control Bit
		endm

;----------------------------------------------------------------------------
; SPI

__defspi	macro	Base
SPIDATA		sfr	Base+0		; SPI Data
SPICTL		sfr	Base+1		; SPI Control
SPI_IRQE	__z8bit	SPICTL,7	;  Interrupt Request Enable
STR		__z8bit	SPICTL,6	;  Start an SPI Interrupt Request
SPI_BIRQ	__z8bit	SPICTL,5	;  BRG Timer Interrupt Request
PHASE		__z8bit	SPICTL,4	;  Phase Select
CLKPOL		__z8bit	SPICTL,3	;  Clock Polarity
WOR		__z8bit	SPICTL,2	;  Wire-OR (Open-Drain) Mode Enabled
MMEN		__z8bit	SPICTL,1	;  SPI MASTER Mode Enable
SPIEN		__z8bit	SPICTL,0	;  SPI Enable
SPISTAT		sfr	Base+2		; SPI Status
IRQ		__z8bit	SPISTAT,7	;  Interrupt Request
OVR		__z8bit	SPISTAT,6	;  Overrun
COL		__z8bit	SPISTAT,5	;  Collision
ABT		__z8bit	SPISTAT,4	;  SLAVE Mode Transaction Abort
TXST		__z8bit	SPISTAT,1	;  Transmit Status
SLAS		__z8bit	SPISTAT,0	;  Slave Select
SPIMODE		sfr	Base+3		; SPI Mode
SPI_DIAG	__z8bit	SPIMODE,5	;  Diagnostic Mode Control Bit
NUMBITS		__z8bfield SPIMODE,2,3	;  Number of Data Bits Per Character to Transfer
SSIO		__z8bit	SPIMODE,1	;  Slave Select I/O
SSV		__z8bit	SPIMODE,0	;  Slave Select Value
SPIDST		sfr	Base+4		; SPI Diagnostic State
SCKEN		__z8bit	SPIDST,7	;  Shift Clock Enable
TCKEN		__z8bit	SPIDST,6	;  Transmit Clock Enable
SPISTATE	__z8bfield SPIDST,0,6	;  SPI State Machine
SPIBRH		sfr	Base+6		; SPI Baud Rate High Byte
SPIBRL		sfr	Base+7		; SPI Baud Rate Low Byte
SPIBR		sfr	SPIBRH
		endm

;----------------------------------------------------------------------------

		restore

		endif			; __ez8cominc
