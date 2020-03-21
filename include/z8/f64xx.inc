		ifndef	__regf64xxinc
__regf64xxinc	equ	1
                save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File F64xx.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z8encore F64xx                 *
;*   Source: Z8 Encore! XP F64xx Series Product Specification, PS019926-1114*
;*                                                                          *
;****************************************************************************

		include	"ez8com.inc"

;----------------------------------------------------------------------------
; System Control


;----------------------------------------------------------------------------
; Flash Options

OPTIONS0	label	0000h
WDT_RES		__z8cbit OPTIONS0,7	;  Watchdog Timer Reset
WDT_AO		__z8cbit OPTIONS0,6	;  Watchdog Timer Always On
OSC_SEL		__z8cbfield OPTIONS0,4,2 ;  Oscillator Mode Selection
VBO_AO		__z8cbit OPTIONS0,3	;  Voltage Brown-Out Protection Always On
FRP		__z8cbit OPTIONS0,2	;  (Flash) Read Protect
FWP		__z8cbit OPTIONS0,0	;  Flash Write Protect
OPTIONS1	label	0001h

;----------------------------------------------------------------------------
; Interrupts Vectors

RESET_vect	label	0002h		; Reset (not an interrupt)
WDT_vect	label	0004h		; Watchdog Timer
ILL_INST_vect	label	0006h		; Illegal Instruction Trap (not an interrupt)
TIMER2_vect	label	0008h		; Timer 2
TIMER1_vect	label	000ah		; Timer 1
TIMER0_vect	label	000ch		; Timer 0
UART0_RX_vect	label	000eh		; UART0 Receiver
UART0_TX_vect	label	0010h		; UART0 Transmitter
I2C_vect	label	0012h		; I2C
SPI_vect	label	0014h		; SPI
ADC_vect	label	0016h		; ADC
A7_vect		label	0018h		; Port A7 or Port D7, selectable rising or falling input edge
A6_vect		label	001ah		; Port A6 or Port D6, selectable rising or falling input edge
A5_vect		label	001ch		; Port A5 or Port D5, selectable rising or falling input edge
A4_vect		label	001eh		; Port A4 or Port D4, selectable rising or falling input edge
A3_vect		label	0020h		; Port A3 or Port D3, selectable rising or falling input edge
A2_vect		label	0022h		; Port A2 or Port D2, selectable rising or falling input edge
A1_vect		label	0024h		; Port A1 or Port D1, selectable rising or falling input edge
A0_vect		label	0026h		; Port A0 or Port D0, selectable rising or falling input edge
		if	__hastimer3
TIMER3_vect	label	0028h		; Timer 3
		endif
UART1_RX_vect	label	002ah		; UART0 Receiver
UART1_TX_vect	label	002ch		; UART0 Transmitter
DMA_vect	label	002eh		; DMA
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

T2ENH		__z8bit	IRQ0ENH,7	;  Timer 1 Interrupt Enable & Priority
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

PA7DI		__z8bit	IRQ1,7		;  Port A7 or Port D7 Interrupt Request
PA6DI		__z8bit	IRQ1,6		;  Port A6 or Port D6 Interrupt Request
PA5DI		__z8bit	IRQ1,5		;  Port A5 or Port D5 Interrupt Request
PA4DI		__z8bit	IRQ1,4		;  Port A4 or Port D4 Interrupt Request
PA3DI		__z8bit	IRQ1,3		;  Port A3 or Port D3 Interrupt Request
PA2DI		__z8bit	IRQ1,2		;  Port A2 or Port D2 Interrupt Request
PA1DI		__z8bit	IRQ1,1		;  Port A1 or Port D1 Interrupt Request
PA0DI		__z8bit	IRQ1,0		;  Port A0 or Port D0 Interrupt Request

PAD7ENH		__z8bit	IRQ1ENH,7	;  Port A7 or Port D7 Interrupt Enable & Priority
PAD7ENL		__z8bit	IRQ1ENL,7
PAD6ENH		__z8bit	IRQ1ENH,6	;  Port A6 or Port D6 Interrupt Enable & Priority
PAD6ENL		__z8bit	IRQ1ENL,6
PAD5ENH		__z8bit	IRQ1ENH,5	;  Port A5 or Port D5 Interrupt Enable & Priority
PAD5ENL		__z8bit	IRQ1ENL,5
PAD4ENH		__z8bit	IRQ1ENH,4	;  Port A4 or Port D4 Interrupt Enable & Priority
PAD4ENL		__z8bit	IRQ1ENL,4
PAD3ENH		__z8bit	IRQ1ENH,3	;  Port A3 or Port D3 Interrupt Enable & Priority
PAD3ENL		__z8bit	IRQ1ENL,3
PAD2ENH		__z8bit	IRQ1ENH,2	;  Port A2 or Port D2 Interrupt Enable & Priority
PAD2ENL		__z8bit	IRQ1ENL,2
PAD1ENH		__z8bit	IRQ1ENH,1	;  Port A1 or Port D1 Interrupt Enable & Priority
PAD1ENL		__z8bit	IRQ1ENL,1
PAD0ENH		__z8bit	IRQ1ENH,0	;  Port A0 or Port D0 Interrupt Enable & Priority
PAD0ENL		__z8bit	IRQ1ENL,0

T3I		__z8bit	IRQ2,7		;  Timer 3 Interrupt Request
U1RXI		__z8bit	IRQ2,6		;  UART 1 Receiver Interrupt Request
U1TXI		__z8bit	IRQ2,5		;  UART 1 Transmitter Interrupt Request
DMAI		__z8bit	IRQ2,4		;  DMA Interrupt Request
PC3I		__z8bit	IRQ2,3		;  Port C3 Interrupt Request
PC2I		__z8bit	IRQ2,2		;  Port C2 Interrupt Request
PC1I		__z8bit	IRQ2,1		;  Port C1 Interrupt Request
PC0I		__z8bit	IRQ2,0		;  Port C0 Interrupt Request

T3ENH		__z8bit	IRQ2ENH,7	;  Timer 3 Interrupt Enable & Priority
T3ENL		__z8bit	IRQ2ENL,7
U1RENH		__z8bit	IRQ2ENH,6	;  UART 1 Receive Interrupt Enable & Priority
U1RENL		__z8bit	IRQ2ENL,6
U1TENH		__z8bit	IRQ2ENH,5	;  UART 1 Transmit Interrupt Enable & Priority
U1TENL		__z8bit	IRQ2ENL,5
DMAENH		__z8bit	IRQ2ENH,4	;  DMA Interrupt Enable & Priority
DMAENL		__z8bit	IRQ2ENL,4
C3ENH		__z8bit	IRQ2ENH,3	;  Port C3 Interrupt Enable & Priority
C3ENL		__z8bit	IRQ2ENL,3
C2ENH		__z8bit	IRQ2ENH,2	;  Port C2 Interrupt Enable & Priority
C2ENL		__z8bit	IRQ2ENL,2
C1ENH		__z8bit	IRQ2ENH,1	;  Port C1 Interrupt Enable & Priority
C1ENL		__z8bit	IRQ2ENL,1
C0ENH		__z8bit	IRQ2ENH,0	;  Port C0 Interrupt Enable & Priority
C0ENL		__z8bit	IRQ2ENL,0

IRQES		sfr	0fcdh		; Interrupt Edge Select
IRQPS		sfr	0fceh		; Interrupt Port Select Register
PAD7S		__z8bit	IRQPS,7		; PAx/PDx Selection
PAD6S		__z8bit	IRQPS,6
PAD5S		__z8bit	IRQPS,5
PAD4S		__z8bit	IRQPS,4
PAD3S		__z8bit	IRQPS,3
PAD2S		__z8bit	IRQPS,2
PAD1S		__z8bit	IRQPS,1
PAD0S		__z8bit	IRQPS,0
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
		__defgpio "F",0fe4h
		__defgpio "G",0fe8h
		__defgpio "H",0fech

;----------------------------------------------------------------------------
; Timer

		__deftimer "0",0f00h,0,1
		__deftimer "1",0f08h,0,1
		__deftimer "2",0f10h,0,1
		if	__hastimer3
		__deftimer "3",0f18h,0,1
		endif

;----------------------------------------------------------------------------
; UART

		__defuart "0",0f40h
		__defuart "1",0f48h

;----------------------------------------------------------------------------
; I2C

		__defi2c 0f50h

;----------------------------------------------------------------------------
; SPI

		__defspi 0f60h

;----------------------------------------------------------------------------
; Analog/Digital Converter

ADCCTL		sfr	0f70h		; ADC Control 0
CEN		__z8bit	ADCCTL,7	;  Conversion Enable
VREF		__z8bit	ADCCTL,5	;  Voltage Reference
CONT		__z8bit ADCCTL,4	;  Continuous Conversion
ANAIN		__z8bfield ADCCTL,0,3	;  Analog Input Select
ADCD_H		sfr	0f72h		; ADC Data High Byte
ADCD_L		sfr	0f73h		; ADC Data Low Bits
OVF		__z8bit	ADCD_L,0	;  Overflow Status
ADCD		sfr	ADCD_H

;----------------------------------------------------------------------------
; DMA

__defdma	macro	NUM,Base
DMA{NUM}CTL	sfr	Base+0		; DMA Control
DMA{NUM}DEN	__z8bit	DMA{NUM}CTL,7	;  DMA Enable
DMA{NUM}DLE	__z8bit	DMA{NUM}CTL,6	;  DMA Loop Enable
DMA{NUM}DDIR	__z8bit	DMA{NUM}CTL,5	;  DMA Data Transfer Direction
DMA{NUM}IRQEN	__z8bit	DMA{NUM}CTL,4	;  DMA Interrupt Enable
DMA{NUM}WSEL	__z8bit	DMA{NUM}CTL,3	;  Word Select
DMA{NUM}RSS	__z8bfield DMA{NUM}CTL,0,3 ;  Request Trigger Source Select
DMA{NUM}IO	sfr	Base+1		; DMA I/O Address
DMA{NUM}H	sfr	Base+2		; DMA End/Start Address High Nibble
DMA{NUM}_END_H	__z8bfield DMA{NUM}H,4,4 ;  DMA End Address High Nibble  
DMA{NUM}_START_H __z8bfield DMA{NUM}H,0,4 ;  DMA Start/Current Address High Nibble
DMA{NUM}START	sfr	Base+3		; DMA Start Address Low Byte
DMA{NUM}END	sfr	Base+4		; DMA End Address Low Byte
		endm
		
		__defdma "0",0fb0h
		__defdma "1",0fb8h

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCTL		sfr	0ff0h		; Watchdog Timer Control
WDTU		sfr	0ff1h		; Watchdog Timer Reload Upper Byte
WDTH		sfr	0ff2h		; Watchdog Timer Reload High Byte
WDTL		sfr	0ff3h		; Watchdog Timer Reload Low Byte

;----------------------------------------------------------------------------

		restore

		endif			; __regf64xxinc
