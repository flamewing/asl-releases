		ifndef	__regf1680inc
__regf1680inc	equ	1
                save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File F1680.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z8encore F1680                 *
;*   Source: Z8 Encore! XP F1680 Series Product Specification, PS025016-1013*
;*                                                                          *
;****************************************************************************

		include	"ez8com.inc"

;----------------------------------------------------------------------------
; System Control

PWRCTL0		sfr	0f80h		; Power Control 0
TRAM		__z8bit	PWRCTL0,7	;  Low-Power Operational Amplifier Disable
LVD_VBO		__z8bit	PWRCTL0,4	;  Low-Voltage Detection/Voltage Brown-Out Detector Disable
TEMP		__z8bit	PWRCTL0,3	;  Temperature Sensor Disable
COMP0		__z8bit	PWRCTL0,1	;  Comparator 0 Disable
COMP1		__z8bit	PWRCTL0,0	;  Comparator 1 Disable

OSCCTL0		sfr	0f86h		; Oscillator Control 0
INTEN		__z8bit	OSCCTL0,7	;  Internal Precision Oscillator Enable
XTLEN		__z8bit	OSCCTL0,6	;  Crystal Oscillator Enable
WDTEN		__z8bit	OSCCTL0,5	;  Watchdog Timer Oscillator Enable
POFEN		__z8bit	OSCCTL0,4	;  Primary Oscillator Failure Detection Enable
WDFEN		__z8bit	OSCCTL0,3	;  Watchdog Timer Oscillator Failure Detection Enable
SCKSEL		__z8bfield OSCCTL0,0,3	;  System Clock Oscillator Select
OSCCTL1		sfr	0f87h		; Oscillator Control 1
SECEN		__z8bit	OSCCTL1,7	;  Secondary Oscillator Enable
SECRDY		__z8bit	OSCCTL1,6	;  Secondary Oscillator Ready Flag
INTSEL		__z8bfield OSCCTL1,0,3	;  Internal Precision Oscillator Frequency Select

TRMADR		sfr	0ff6h		; Trim Bit Address
TRMDR		sfr	0ff7h		; Trim Data

;----------------------------------------------------------------------------
; Flash Options

OPTIONS0	label	0000h
WDT_RES		__z8cbit OPTIONS0,7	;  Watchdog Timer Reset
WDT_AO		__z8cbit OPTIONS0,6	;  Watchdog Timer Always On
OSC_SEL		__z8cbfield OPTIONS0,4,2 ;  Oscillator Mode Selection
VBO_AO		__z8cbit OPTIONS0,3	;  Voltage Brown-Out Protection Always On
FRP		__z8cbit OPTIONS0,2	;  (Flash) Read Protect
PRAM_M		__z8cbit OPTIONS0,1	;  On-Chip Program RAM Mode Select
FWP		__z8cbit OPTIONS0,0	;  Flash Write Protect
OPTIONS1	label	0001h
EXTLTMG		__z8cbfield OPTIONS1,6,2 ;  External Crystal Reset Timing
FLASH_WR_PRO_EN	__z8cbit OPTIONS1,5	;  Flash Write Operation Protect
EXTL_AO		__z8cbit OPTIONS1,4	;  External Crystal Always ON
X2_Mode		__z8cbit OPTIONS1,1	;  Secondary Crystal Mode Select
X2TL_AO		__z8cbit OPTIONS1,0	;  Secondary Crystal Always On

;----------------------------------------------------------------------------
; Interrupts Vectors

RESET_vect	label	0002h		; Reset (not an interrupt)
WDT_vect	label	0004h		; Watchdog Timer
ILL_INST_vect	label	0006h		; Illegal Instruction Trap (not an interrupt)
TIMER2_vect	label	0008h		; Timer 2
TIMER1_vect	label	000ah		; Timer 1
TIMER0_vect	label	000ch		; Timer 0
UART0_RX_vect	label	000eh		; UART 0 Receiver
UART0_TX_vect	label	0010h		; UART 0 Transmitter
I2C_vect	label	0012h		; I2C
SPI_vect	label	0014h		; SPI
ADC_vect	label	0016h		; ADC
A7_vect		label	0018h		; Port A7, selectable rising or falling input edge, or LVD
A6_vect		label	001ah		; Port A6, selectable rising or falling input edge or Comparator 0 Output
A5_vect		label	001ch		; Port A5, selectable rising or falling input edge or Comparator 1 Output
A4_vect		label	001eh		; Port A4 or Port D4, selectable rising or falling input edge
A3_vect		label	0020h		; Port A3 or Port D3, selectable rising or falling input edge
A2_vect		label	0022h		; Port A2 or Port D2, selectable rising or falling input edge
A1_vect		label	0024h		; Port A1 or Port D1, selectable rising or falling input edge
A0_vect		label	0026h		; Port A0, selectable rising or falling input edge
MCT_vect	label	002ah		; Multi-Channel Timer
UART1_RX_vect	label	002ch		; UART 1 Receiver
UART1_TX_vect	label	002eh		; UART 1 Transmitter
C3_vect		label	0030h		; Port C3, both input edges
C2_vect		label	0032h		; Port C2, both input edges
C1_vect		label	0034h		; Port C1, both input edges
C0_vect		label	0036h		; Port C0, both input edges

;----------------------------------------------------------------------------
; Interrupts

__defirq	macro	NUM,Base
IRQ{NUM}	sfr	Base+0		; Interrupt Request n
IRQ{NUM}ENH	sfr	Base+1		; IRQn Enable High Bit
IRQ{NUM}ENL	sfr	Base+2		; IRQn Enable Low Bit
		endm

		__defirq "0",0fc0h
		__defirq "1",0fc3h
		__defirq "2",0fc6h

T2I		__z8bit	IRQ0,7		;  Timer 2 Interrupt Request
T1I		__z8bit IRQ0,6		;  Timer 1 Interrupt Request
T0I		__z8bit IRQ0,5		;  Timer 0 Interrupt Request
U0RXI		__z8bit	IRQ0,4		;  UART 0 Receiver Interrupt Request
U0TXI		__z8bit	IRQ0,3		;  UART 0 Transmitter Interrupt Request
I2CI		__z8bit	IRQ0,2		;  I2C Interrupt Request
SPII		__z8bit	IRQ0,1		;  SPI Interrupt Request
ADCI		__z8bit IRQ0,0		;  ADC Interrupt Request

T2ENH		__z8bit	IRQ0ENH,7	;  Timer 2 Interrupt Enable & Priority
T2ENL		__z8bit	IRQ0ENL,7
T1ENH		__z8bit	IRQ0ENH,6	;  Timer 1 Interrupt Enable & Priority
T1ENL		__z8bit	IRQ0ENL,6
T0ENH		__z8bit	IRQ0ENH,5	;  Timer 0 Interrupt Enable & Priority
T0ENL		__z8bit	IRQ0ENL,5
U0RENH		__z8bit	IRQ0ENH,4	;  UART 0 Receive Interrupt Enable & Priority
U0RENL		__z8bit	IRQ0ENL,4
U0TENH		__z8bit	IRQ0ENH,3	;  UART 0 Transmit Interrupt Enable & Priority
U0TENL		__z8bit	IRQ0ENL,3
I2CENH		__z8bit	IRQ0ENH,2	;  I2C Interrupt Enable & Priority
I2CENL		__z8bit	IRQ0ENL,2
SPIENH		__z8bit	IRQ0ENH,1	;  SPI Interrupt Enable & Priority
SPIENL		__z8bit	IRQ0ENL,1
ADCENH		__z8bit	IRQ0ENH,0	;  ADC Interrupt Enable & Priority
ADCENL		__z8bit	IRQ0ENL,0

PA7VI		__z8bit	IRQ1,7		;  Port A7 or LVD Interrupt Request
PA6CI		__z8bit	IRQ1,6		;  Port A6 or Comparator 0 Interrupt Request
PA5CI		__z8bit	IRQ1,5		;  Port A5 or Comparator 1 Interrupt Request
PAD4I		__z8bit	IRQ1,4		;  Port A4 or Port D4 Interrupt Request
PAD3I		__z8bit	IRQ1,3		;  Port A3 or Port D3 Interrupt Request
PAD2I		__z8bit	IRQ1,2		;  Port A2 or Port D2 Interrupt Request
PAD1I		__z8bit	IRQ1,1		;  Port A1 or Port D1 Interrupt Request
PA0I		__z8bit	IRQ1,0		;  Port A0 Interrupt Request

PA7VENH		__z8bit	IRQ1ENH,7	;  Port A7 or LVD Interrupt Enable & Priority
PA7VENL		__z8bit	IRQ1ENL,7
PA6ENH		__z8bit	IRQ1ENH,6	;  Port A6 or Comparator 0 Interrupt Enable & Priority
PA6ENL		__z8bit	IRQ1ENL,6
PA5ENH		__z8bit	IRQ1ENH,5	;  Port A5 or Comparator 1 Interrupt Enable & Priority
PA5ENL		__z8bit	IRQ1ENL,5
PA4ENH		__z8bit	IRQ1ENH,4	;  Port A4 or Port D4 Interrupt Enable & Priority
PA4ENL		__z8bit	IRQ1ENL,4
PA3ENH		__z8bit	IRQ1ENH,3	;  Port A3 or Port D3 Interrupt Enable & Priority
PA3ENL		__z8bit	IRQ1ENL,3
PA2ENH		__z8bit	IRQ1ENH,2	;  Port A2 or Port D2 Interrupt Enable & Priority
PA2ENL		__z8bit	IRQ1ENL,2
PA1ENH		__z8bit	IRQ1ENH,1	;  Port A1 or Port D1 Interrupt Enable & Priority
PA1ENL		__z8bit	IRQ1ENL,1
PA0ENH		__z8bit	IRQ1ENH,0	;  Port A0 Interrupt Enable & Priority
PA0ENL		__z8bit	IRQ1ENL,0

MCTI		__z8bit	IRQ2,6		;  Multi-Channel Timer Interrupt Request
U1RXI		__z8bit	IRQ2,5		;  UART 1 Receiver Interrupt Request
U1TXI		__z8bit	IRQ2,4		;  UART 1 Transmitter Interrupt Request
PC3I		__z8bit	IRQ2,3		;  Port C3 Interrupt Request
PC2I		__z8bit	IRQ2,2		;  Port C2 Interrupt Request
PC1I		__z8bit	IRQ2,1		;  Port C1 Interrupt Request
PC0I		__z8bit	IRQ2,0		;  Port C0 Interrupt Request

MCTENH		__z8bit	IRQ2ENH,6	;  Multi-Channel Timer Interrupt Enable & Priority
MCTENL		__z8bit	IRQ2ENL,6
U1RENH		__z8bit	IRQ2ENH,5	;  UART 1 Receive Interrupt Enable & Priority
U1RENL		__z8bit	IRQ2ENL,5
U1TENH		__z8bit	IRQ2ENH,4	;  UART 1 Transmit Interrupt Enable & Priority
U1TENL		__z8bit	IRQ2ENL,4
C3ENH		__z8bit	IRQ2ENH,3	;  Port C3 Interrupt Enable & Priority
C3ENL		__z8bit	IRQ2ENL,3
C2ENH		__z8bit	IRQ2ENH,2	;  Port C2 Interrupt Enable & Priority
C2ENL		__z8bit	IRQ2ENL,2
C1ENH		__z8bit	IRQ2ENH,1	;  Port C1 Interrupt Enable & Priority
C1ENL		__z8bit	IRQ2ENL,1
C0ENH		__z8bit	IRQ2ENH,0	;  Port C0 Interrupt Enable & Priority
C0ENL		__z8bit	IRQ2ENL,0

IRQES		sfr	0fcdh		; Interrupt Edge Select
IRQSS		sfr	0fceh		; Shared Interrupt Select Register
PA7VS		__z8bit	IRQSS,7		;  PA7/LVD Selection
PA6CS		__z8bit	IRQSS,6		;  PA6/Comparator 0 Selection
PA5CS		__z8bit	IRQSS,5		;  PA5/Comparator 1 Selection
PAD4S		__z8bit	IRQSS,4		;  PA4/PD4 Selection
PAD3S		__z8bit	IRQSS,3		;  PA3/PD3 Selection
PAD2S		__z8bit	IRQSS,2		;  PA2/PD2 Selection
PAD1S		__z8bit	IRQSS,1		;  PA1/PD1 Selection
IRQCTL		sfr	0fcfh		; Interrupt Control
IRQE		__z8bit	IRQCTL,7	;  Interrupt Request Enable

;----------------------------------------------------------------------------
; Flash Memory Control

FCTL		sfr	0ff8h		; Flash Control
FCMD		__z8bfield FCTL,0,8	;  Flash Command
FSTAT		sfr	0ff8h		; Flash Status
FPS		sfr	0ff9h		; Flash Page Select
INFO_EN		__z8bit	FPS,7		;  Information Area Enable
PAGE		__z8bfield FPS,0,7	;  Page Select
FPROT		sfr	0ff9h		; Flash Sector Protect
FFREQH		sfr	0ffah		; Flash Programming Frequency High Byte
FFREQL		sfr	0ffbh		; Flash Programming Frequency Low Byte
FFREQ		sfr	FFREQH

;----------------------------------------------------------------------------
; GPIO

		__defgpio "A",0fd0h
		__defgpio "B",0fd4h
		__defgpio "C",0fd8h
		__defgpio "D",0fdch
		__defgpio "E",0fe0h

;----------------------------------------------------------------------------
; LED Controller

LEDEN		sfr	0f82h		; LED Drive Enable
LEDLVLH		sfr	0f83h		; LED Drive Level High
LEDLVLL		sfr	0f84h		; LED Drive Level Low

;----------------------------------------------------------------------------
; Timer

__defmytimer	macro	NUM,Base,Base2,Base3
		__deftimer NUM,Base,1,1
T{NUM}MODE3	equ	T{NUM}MODEHI
T{NUM}PWM1H	sfr	Base2+0		; Timer PWM1 High Byte
T{NUM}PWM1L	sfr	Base2+1		; Timer PWM1 Low Byte
T{NUM}PWM1	sfr	T{NUM}PWM1H
T{NUM}CTL2	sfr	Base2+2		; Timer Control 2
T{NUM}PWM0UE	__z8bit	T{NUM}CTL2,5	;  PWM0 Update Enable
T{NUM}TPOLHI	__z8bit	T{NUM}CTL2,4	;  Timer Input/Output Polarity High Bit
T{NUM}TCLKS	__z8bit	T{NUM}CTL2,0	;  Timer Clock Source
T{NUM}STAT	sfr	Base2+3		; Timer Status
T{NUM}NEF	__z8bit	T{NUM}STAT,7	;  Noise Event Flag
T{NUM}PWM1EO	__z8bit	T{NUM}STAT,5	;  PWM 1 Event Overrun
T{NUM}PWM0EO	__z8bit	T{NUM}STAT,4	;  PWM 0 Event Overrun
T{NUM}RTOEF	__z8bit	T{NUM}STAT,3	;  Reload Time-Out Event Flag
T{NUM}PWM1EF	__z8bit	T{NUM}STAT,1	;  PWM 1 Event Flag
T{NUM}PWM0EF	__z8bit	T{NUM}STAT,0	;  PWM 0 Event Flag
T{NUM}NFC	sfr	Base3+0		; Timer Noise Filter Control
T{NUM}NFEN	__z8bit	T{NUM}NFC,7	;  Noise Filter Enable
T{NUM}NFCTL	__z8bfield T{NUM}NFC,4,3 ;  Noise Filter Control
		endm

		__defmytimer "0",0f00h,0f20h,0f2ch
		__defmytimer "1",0f08h,0f24h,0f2dh
		__defmytimer "2",0f10h,0f28h,0f2eh

;----------------------------------------------------------------------------
; Multi-Channel Timer

		__defmct

;----------------------------------------------------------------------------
; LIN UART

__deflinuart	macro	NUM,Base
U{NUM}TXD	sfr	Base+0		; LIN UART Transmit Data
U{NUM}RXD	sfr	Base+0		; LIN UART Receive Data
U{NUM}STAT0	sfr	Base+1		; LIN UART Status 0
U{NUM}RDA	__z8bit	U{NUM}STAT0,7	;  Receive Data Available (LIN+UART)
U{NUM}PE	__z8bit	U{NUM}STAT0,6	;  Parity Error (UART)
U{NUM}PLE	__z8bit	U{NUM}STAT0,6	;  Physical Layer Error (LIN)
U{NUM}OE	__z8bit	U{NUM}STAT0,5	;  Overrun Error (LIN+UART)
U{NUM}FE	__z8bit	U{NUM}STAT0,4	;  Framing Error (LIN+UART)
U{NUM}BRKD	__z8bit	U{NUM}STAT0,3	;  Break Detect (LIN+UART)
U{NUM}TDRE	__z8bit	U{NUM}STAT0,2	;  Transmitter Data Register Empty (LIN+UART)
U{NUM}TXE	__z8bit	U{NUM}STAT0,1	;  Transmitter Empty (LIN+UART)
U{NUM}CTS	__z8bit	U{NUM}STAT0,0	;  Clear to Send Signal (UART)
U{NUM}ATB	__z8bit	U{NUM}STAT0,0	;  LIN Slave Autobaud Complete (LIN)
U{NUM}CTL0	sfr	Base+2		; LIN UART Control 0
U{NUM}TEN	__z8bit	U{NUM}CTL0,7	;  Transmit Enable
U{NUM}REN	__z8bit	U{NUM}CTL0,6	;  Receive Enable
U{NUM}CTSE	__z8bit	U{NUM}CTL0,5	;  Clear To Send Enable
U{NUM}PEN	__z8bit	U{NUM}CTL0,4	;  Parity Enable
U{NUM}PSEL	__z8bit	U{NUM}CTL0,3	;  Parity Select
U{NUM}SBRK	__z8bit	U{NUM}CTL0,2	;  Send Break
U{NUM}STOP	__z8bit	U{NUM}CTL0,1	;  Stop Bit Select
U{NUM}LBEN	__z8bit	U{NUM}CTL0,0	;  Loop Back Enable
U{NUM}CTL1	sfr	Base+3		; LIN UART Control 1
U{NUM}MPMD1	__z8bit	U{NUM}CTL1,7	;  Multiprocessor Mode (MSEL=000)
U{NUM}MPEN	__z8bit	U{NUM}CTL1,6	;  Multiprocessor Enable (MSEL=000)
U{NUM}MPMD0	__z8bit	U{NUM}CTL1,5	;  Multiprocessor Mode (MSEL=000)
U{NUM}MPBT	__z8bit	U{NUM}CTL1,4	;  Multiprocessor Bit Transmit (MSEL=000)
U{NUM}DEPOL	__z8bit	U{NUM}CTL1,3	;  Driver Enable Polarity (MSEL=000)
U{NUM}BRGCTL	__z8bit	U{NUM}CTL1,2	;  Baud Rate Generator Control (MSEL=000)
U{NUM}RDAIRQ	__z8bit	U{NUM}CTL1,1	;  Receive Data Interrupt (MSEL=000)
U{NUM}IREN	__z8bit	U{NUM}CTL1,0	;  Loop Back Enable (MSEL=000)
U{NUM}NFEN	__z8bit	U{NUM}CTL1,7	;  Noise Filter Enable (MSEL=001)
U{NUM}NFCTL	__z8bfield U{NUM}CTL1,4,3 ;  Noise Filter Control (MSEL=001)
U{NUM}LMST	__z8bit	U{NUM}CTL1,7	;  LIN MASTER Mode (MSEL=010)
U{NUM}LSLV	__z8bit	U{NUM}CTL1,6	;  LIN SLAVE Mode (MSEL=010)
U{NUM}ABEN	__z8bit	U{NUM}CTL1,5	;  Autobaud Enable (MSEL=010)
U{NUM}ABIEN	__z8bit	U{NUM}CTL1,4	;  Autobaud Interrupt Enable (MSEL=010)
U{NUM}LinState	__z8bfield U{NUM}CTL1,2,2 ;  LIN State Machine (MSEL=010)
U{NUM}TxBreakLength __z8bfield U{NUM}CTL1,0,2 ;  TxBreakLength (MSEL=010)
U{NUM}MDSTAT	sfr	Base+4		; LIN UART Mode Select and Status
U{NUM}MSEL	__z8bfield U{NUM}MDSTAT,5,3 ;  Mode Select
U{NUM}MODESTATUS __z8bfield U{NUM}MDSTAT,0,5 ;  Mode Status
U{NUM}ADDR	sfr	Base+5		; UART Address Compare
U{NUM}BRH	sfr	Base+6		; UART Baud Rate High Byte
U{NUM}BRL	sfr	Base+7		; UART Baud Rate Low Byte
U{NUM}BR	sfr	U{NUM}BRH
		endm

		__deflinuart "0",0f40h
		__deflinuart "1",0f48h

;----------------------------------------------------------------------------
; I2C

I2CDATA		sfr	0f50h		; I2C Data
I2CSTAT		sfr	0f51h		; I2C Status
I2C_TDRE	__z8bit	I2CSTAT,7	;  Transmit Data Register Empty
RDRF		__z8bit	I2CSTAT,6	;  Receive Data Register Full
SAM		__z8bit	I2CSTAT,5	;  Slave Address Match
GCA		__z8bit	I2CSTAT,4	;  General Call Address
RD		__z8bit	I2CSTAT,3	;  Read
ARBLST		__z8bit	I2CSTAT,2	;  Arbitration Lost
SPRS		__z8bit	I2CSTAT,1	;  Stop/Restart Condition Interrupt
NCKI		__z8bit	I2CSTAT,0	;  NACK Interrupt
I2CCTL		sfr	0f52h		; I2C Control
IEN		__z8bit	I2CCTL,7	;  I2C Enable
SSTART		__z8bit	I2CCTL,6	;  Send Start Condition
SSTOP		__z8bit	I2CCTL,5	;  Send Stop Condition
I2C_BIRQ	__z8bit	I2CCTL,4	;  Baud Rate Generator Interrupt Request
TXI		__z8bit	I2CCTL,3	;  Enable TDRE interrupts
NAK		__z8bit	I2CCTL,2	;  Send NAK
FLUSH		__z8bit	I2CCTL,1	;  Flush Data
FILTEN		__z8bit	I2CCTL,0	;  I2C Signal Filter Enable
I2CBRH		sfr	0f53h		; I2C Baud Rate High Byte
I2CBRL		sfr	0f54h		; I2C Baud Rate Low Byte
I2CBR		sfr	I2CBRH
I2CSTATE	sfr	0f55h		; I2C State
I2CSTATE_H	__z8bfield I2CSTATE,4,4	;  I2C State (DIAG=1)
I2CSTATE_L	__z8bfield I2CSTATE,0,4	;  Least Significant Nibble of the I2C State Machine (DIAG=1)
ACKV		__z8bit	I2CSTATE,7	;  ACK Valid (DIAG=0)
ACK		__z8bit	I2CSTATE,6	;  Acknowledge (DIAG=0)
AS		__z8bit	I2CSTATE,5	;  Address State (DIAG=0)
DS		__z8bit	I2CSTATE,4	;  Data State (DIAG=0)
I10B		__z8bit	I2CSTATE,3	;  10B (DIAG=0)
RSTR		__z8bit	I2CSTATE,2	;  RESTART (DIAG=0)
SCLOUT		__z8bit	I2CSTATE,1	;  Serial Clock Output (DIAG=0)
BUSY		__z8bit	I2CSTATE,0	;  I2C Bus Busy (DIAG=0)
I2CMODE		sfr	0f56h		; I2C Mode
MODE		__z8bfield I2CMODE,5,2	;  Selects the I2C Controller Operational Mode
IRM		__z8bit	I2CMODE,4	;  Interactive Receive Mode
GCE		__z8bit	I2CMODE,3	;  General Call Address Enable
SLA		__z8bfield I2CMODE,1,2	;  Slave Address Bits 9 and 8
DIAG		__z8bit	I2CMODE,0	;  Diagnostic Mode
I2CSLVAD	sfr	0f57h		; I2C Slave Address Register

;----------------------------------------------------------------------------
; ESPI

ESPIDATA	sfr	0f60h		; ESPI Data
ESPITDCR	sfr	0f61h		; ESPI Transmit Data Command
CRDR		__z8bit	ESPITDCR,7	;  Clear Receive Data Register
RDFLAG		__z8bfield ESPITDCR,5,2	;  Receive Data Buffer Flag
TEOF		__z8bit	ESPITDCR,1	;  Transmit End of Frame
SSV		__z8bit	ESPITDCR,0	;  Slave Select Value
ESPICTL		sfr	0f62h		; ESPI Control
DIRQE		__z8bit	ESPICTL,7	;  Data Interrupt Request Enable
ESPIEN1		__z8bit	ESPICTL,6	;  ESPI Enable and Direction Control
BRGCTL		__z8bit	ESPICTL,5	;  Baud Rate Generator Control
PHASE		__z8bit	ESPICTL,4	;  Phase Select
CLKPOL		__z8bit	ESPICTL,3	;  Clock Polarity
WOR		__z8bit	ESPICTL,2	;  Wire-OR (Open-Drain) Mode Enabled
MMEN		__z8bit	ESPICTL,1	;  ESPI MASTER Mode Enable
ESPIEN0		__z8bit	ESPICTL,0	;  ESPI Enable and Direction Control
ESPIMODE	sfr	0f63h		; ESPI Mode
SSMD		__z8bfield ESPIMODE,5,3	;  Slave Select Mode
NUMBITS		__z8bfield ESPIMODE,2,3	;  Number of Data Bits Per Character to Transfer
SSIO		__z8bit	ESPIMODE,1	;  Slave Select I/O
SSPO		__z8bit	ESPIMODE,0	;  Slave Select Polarity
ESPISTAT	sfr	0f64h		; ESPI Status
TDRE		__z8bit	ESPISTAT,7	;  Transmit Data Register Empty
TUND		__z8bit	ESPISTAT,6	;  Transmit Underrun
COL		__z8bit	ESPISTAT,5	;  Collision
ABT		__z8bit	ESPISTAT,4	;  SLAVE Mode Transaction Abort
ROVR		__z8bit	ESPISTAT,3	;  Receive Overrun
RDRNE		__z8bit	ESPISTAT,2	;  Receive Data Register Not Empty
TFST		__z8bit	ESPISTAT,1	;  Transfer Status
SLAS		__z8bit	ESPISTAT,0	;  Slave Select
ESPISTATE	sfr	0f65h		; ESPI State
SCKI		__z8bit	ESPISTATE,7	;  Serial Clock Input
SDI		__z8bit	ESPISTATE,6	;  Serial Data Input
ESPIBRH		sfr	0f66h		; ESPI Baud Rate High Byte
ESPIBRL		sfr	0f67h		; ESPI Baud Rate Low Byte
ESPIBR		sfr	ESPIBRH		; 

;----------------------------------------------------------------------------
; Analog Comparator

CMP0		sfr	0f90h		; Comparator 0 Control
INPSEL0		__z8bit	CMP0,7		;  Signal Select for Positive Input
INNSEL0		__z8bit	CMP0,6		;  Signal Select for Negative Input
REFLVL0		__z8bfield CMP0,2,4	;  Internal Reference Voltage Level
TIMTRG0		__z8bfield CMP0,0,2	;  Timer Trigger
CMP1		sfr	0f91h		; Comparator 1 Control
INPSEL1		__z8bit	CMP1,7		;  Signal Select for Positive Input
INNSEL1		__z8bit	CMP1,6		;  Signal Select for Negative Input
REFLVL1		__z8bfield CMP1,2,4	;  Internal Reference Voltage Level
TIMTRG1		__z8bfield CMP1,0,2	;  Timer Trigger

;----------------------------------------------------------------------------
; Analog/Digital Converter

ADCCTL0		sfr	0f70h		; ADC Control 0
START		__z8bit	ADCCTL0,7	;  ADC Start/Busy
INTREF_SEL	__z8bit	ADCCTL0,6	;  Internal Reference Select
REFEN		__z8bit	ADCCTL0,5	;  Reference Select
ADCEN		__z8bit	ADCCTL0,4	;  ADC Eneable
ANAIN		__z8bfield ADCCTL0,0,4	;  Analog Input Select
ADCRD_H		sfr	0f71h		; ADC Raw Data High Byte
ADCD_H		sfr	0f72h		; ADC Data High Byte
ADCD_L		sfr	0f73h		; ADC Data Low Bits
OVF		__z8bit	ADCD_L,0	;  Overflow Status
ADCD		sfr	ADCD_H
ADCSST		sfr	0f74h		; ADC Sample Settling Time
SST		__z8bfield ADCSST,0,4	;  Sample Settling Time
ADCST		sfr	0f75h		; ADC Sample Time
ST		__z8bfield ADCST,0,6	;  Sample Time
ADCCP		sfr	0f76h		; ADC Clock Prescale Register
DIV16		__z8bit	ADCCP,3		;  Divide by 16
DIV8		__z8bit	ADCCP,2		;  Divide by 8
DIV4		__z8bit	ADCCP,1		;  Divide by 4
DIV2		__z8bit	ADCCP,0		;  Divide by 2

;----------------------------------------------------------------------------
; Watchdog Timer

RSTSTAT		sfr	0ff0h		; Reset Status
POR_VBO		__z8bit	RSTSTAT,7	;  Power-On Initiated VBO Reset or General VBO Reset Indicator
STOP		__z8bit	RSTSTAT,6	;  Stop Mode Recovery Indicator
WDT		__z8bit	RSTSTAT,5	;  Watchdog Timer Time-Out Indicator
EXT		__z8bit	RSTSTAT,4	;  External Reset Indicator
LVD		__z8bit	RSTSTAT,0	;  Low-Voltage Detection Indicator
WDTH		sfr	0ff2h		; Watchdog Timer Reload High Byte
WDTL		sfr	0ff3h		; Watchdog Timer Reload Low Byte

;----------------------------------------------------------------------------

		restore

		endif			; __regf1680inc
