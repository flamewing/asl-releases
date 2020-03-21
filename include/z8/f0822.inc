		ifndef	__regf0822inc
__regf0822inc	equ	1
                save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File F0822.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z8encore F0822                 *
;*   Source: Z8 Encore! XP F0822 Series Product Specification, PS022518-1011*
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
TIMER1_vect	label	000ah		; Timer 1
TIMER0_vect	label	000ch		; Timer 0
UART_RX_vect	label	000eh		; UART Receiver
UART_TX_vect	label	0010h		; UART Transmitter
I2C_vect	label	0012h		; I2C
SPI_vect	label	0014h		; SPI
		if	__hasadc
ADC_vect	label	0016h		; ADC
		endif
A7_vect		label	0018h		; Port A7, selectable rising or falling input edge
A6_vect		label	001ah		; Port A6, selectable rising or falling input edge or Comparator Output
A5_vect		label	001ch		; Port A5, selectable rising or falling input edge
A4_vect		label	001eh		; Port A4, selectable rising or falling input edge
A3_vect		label	0020h		; Port A3 or Port D3, selectable rising or falling input edge
A2_vect		label	0022h		; Port A2 or Port D2, selectable rising or falling input edge
A1_vect		label	0024h		; Port A1, selectable rising or falling input edge
A0_vect		label	0026h		; Port A0, selectable rising or falling input edge
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

T1I		__z8bit IRQ0,6		;  Timer 1 Interrupt Request
T0I		__z8bit IRQ0,5		;  Timer 0 Interrupt Request
U0RXI		__z8bit	IRQ0,4		;  UART 0 Receiver Interrupt Request
U0TXI		__z8bit	IRQ0,3		;  UART 0 Transmitter Interrupt Request
I2CI		__z8bit	IRQ0,2		;  I2C Interrupt Request
SPII		__z8bit	IRQ0,1		;  SPI Interrupt Request
		if	__hasadc
ADCI		__z8bit IRQ0,0		;  ADC Interrupt Request
		endif

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
		if	__hasadc
ADCENH		__z8bit	IRQ0ENH,0	;  ADC Interrupt Enable & Priority
ADCENL		__z8bit	IRQ0ENL,0
		endif

PA7I		__z8bit	IRQ1,7		;  Port A7 Interrupt Request
PA6I		__z8bit	IRQ1,6		;  Port A6 Interrupt Request
PA5I		__z8bit	IRQ1,5		;  Port A5 Interrupt Request
PA4I		__z8bit	IRQ1,4		;  Port A4 Interrupt Request
PA3I		__z8bit	IRQ1,3		;  Port A3 Interrupt Request
PA2I		__z8bit	IRQ1,2		;  Port A2 Interrupt Request
PA1I		__z8bit	IRQ1,1		;  Port A1 Interrupt Request
PA0I		__z8bit	IRQ1,0		;  Port A0 Interrupt Request

PA7ENH		__z8bit	IRQ1ENH,7	;  Port A7 Interrupt Enable & Priority
PA7ENL		__z8bit	IRQ1ENL,7
PA6ENH		__z8bit	IRQ1ENH,6	;  Port A6 Interrupt Enable & Priority
PA6ENL		__z8bit	IRQ1ENL,6
PA5ENH		__z8bit	IRQ1ENH,5	;  Port A5 Interrupt Enable & Priority
PA5ENL		__z8bit	IRQ1ENL,5
PA4ENH		__z8bit	IRQ1ENH,4	;  Port A4 Interrupt Enable & Priority
PA4ENL		__z8bit	IRQ1ENL,4
PA3ENH		__z8bit	IRQ1ENH,3	;  Port A3 Interrupt Enable & Priority
PA3ENL		__z8bit	IRQ1ENL,3
PA2ENH		__z8bit	IRQ1ENH,2	;  Port A2 Interrupt Enable & Priority
PA2ENL		__z8bit	IRQ1ENL,2
PA1ENH		__z8bit	IRQ1ENH,1	;  Port A1 Interrupt Enable & Priority
PA1ENL		__z8bit	IRQ1ENL,1
PA0ENH		__z8bit	IRQ1ENH,0	;  Port A0 Interrupt Enable & Priority
PA0ENL		__z8bit	IRQ1ENL,0

PC3I		__z8bit	IRQ2,3		;  Port C3 Interrupt Request
PC2I		__z8bit	IRQ2,2		;  Port C2 Interrupt Request
PC1I		__z8bit	IRQ2,1		;  Port C1 Interrupt Request
PC0I		__z8bit	IRQ2,0		;  Port C0 Interrupt Request

C3ENH		__z8bit	IRQ2ENH,3	;  Port C3 Interrupt Enable & Priority
C3ENL		__z8bit	IRQ2ENL,3
C2ENH		__z8bit	IRQ2ENH,2	;  Port C2 Interrupt Enable & Priority
C2ENL		__z8bit	IRQ2ENL,2
C1ENH		__z8bit	IRQ2ENH,1	;  Port C1 Interrupt Enable & Priority
C1ENL		__z8bit	IRQ2ENL,1
C0ENH		__z8bit	IRQ2ENH,0	;  Port C0 Interrupt Enable & Priority
C0ENL		__z8bit	IRQ2ENL,0

IRQES		sfr	0fcdh		; Interrupt Edge Select
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

;----------------------------------------------------------------------------
; Timer

		__deftimer "0",0f00h,0,1
		__deftimer "1",0f08h,0,1

;----------------------------------------------------------------------------
; UART

		__defuart "0",0f40h

;----------------------------------------------------------------------------
; I2C

		__defi2c 0f50h

;----------------------------------------------------------------------------
; SPI

		__defspi 0f60h

;----------------------------------------------------------------------------
; Analog/Digital Converter

		if	__hasadc
ADCCTL		sfr	0f70h		; ADC Control 0
CEN		__z8bit	ADCCTL,7	;  Conversion Enable
VREF		__z8bit	ADCCTL,5	;  Voltage Reference
CONT		__z8bit ADCCTL,4	;  Continuous Conversion
ANAIN		__z8bfield ADCCTL,0,3	;  Analog Input Select
ADCD_H		sfr	0f72h		; ADC Data High Byte
ADCD_L		sfr	0f73h		; ADC Data Low Bits
OVF		__z8bit	ADCD_L,0	;  Overflow Status
ADCD		sfr	ADCD_H
ADCSST		sfr	0f74h		; ADC Sample Settling Time
SST		__z8bfield ADCSST,0,3	;  Sample Settling Time
ADCST		sfr	0f75h		; ADC sample time
ST		__z8bfield ADCST,0,6	;  Sample Time
		endif			; __hasadc

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCTL		sfr	0ff0h		; Watchdog Timer Control
WDTU		sfr	0ff1h		; Watchdog Timer Reload Upper Byte
WDTH		sfr	0ff2h		; Watchdog Timer Reload High Byte
WDTL		sfr	0ff3h		; Watchdog Timer Reload Low Byte

;----------------------------------------------------------------------------

		restore

		endif			; __regf0822inc
