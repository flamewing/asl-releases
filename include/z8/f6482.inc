		ifndef	__regf6482inc
__regf6482inc	equ	1
                save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File F6482.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z8encore! XP F6482             *
;*   Source: Z8 Encore! XP F6482 Series Product Specification, PS029412-0618*
;*                                                                          *
;****************************************************************************

		include	"ez8com.inc"

;----------------------------------------------------------------------------
; System Control

PWRCTL0		sfr	0f80h		; Power Control 0
OpAmpB		__z8bit	PWRCTL0,7	;  Op Amp B Enable
OpAmpA		__z8bit	PWRCTL0,6	;  Op Amp A Enable
LCD		__z8bit	PWRCTL0,5	;  LCD Enable
LVDE		__z8bit	PWRCTL0,4	;  Low-Voltage Detection Enable
TEMP		__z8bit	PWRCTL0,3	;  Temperature Sensor Enable
FRECOV		__z8bit	PWRCTL0,2	;  Fast Recovery
COMP0		__z8bit	PWRCTL0,1	;  Comparator 0 Disable
COMP1		__z8bit	PWRCTL0,0	;  Comparator 1 Disable
PWRCTL1		sfr	0f81h		; Power Control 1
ADC		__z8bfield PWRCTL1,2,2	;  ADC and ADC Internal Voltage Reference Buffer Continous Enable
DAC		__z8bit	PWRCTL1,1	;  DAC Enable

CLKCTL0		sfr	0f82h		; Clock Control 0
CSTAT		__z8bit	CLKCTL0,7	;  Clock System Register Lock Status
SCKFEN		__z8bit	CLKCTL0,6	;  System Clock Failure Detection Enable
SCKDIV		__z8bfield CLKCTL0,3,3	;  System Clock Division Ratio
SCKSEL		__z8bfield CLKCTL0,0,3	;  System Clock Source Select
CLKCTL1		sfr	0f83h		; Clock Control 1
IPORDY		__z8bit	CLKCTL1,7	;  Internal Precision Oscillator (IPO) Ready Flag
WTOFEN		__z8bit	CLKCTL1,5	;  Watchdog Timer Oscillator Failure Detection Enable
PCKSEL		__z8bfield CLKCTL1,3,2	;  PCLK Source Select
PCKSM		__z8bit	CLKCTL1,2	;  PCLK Stop Mode Operation
LFXOEN		__z8bit	CLKCTL1,1	;  Low Frequency Crystal Oscillator (LFXO) Enable
IPOEN		__z8bit	CLKCTL1,0	;  Internal Precision Oscillator (IPO) Enable
CLKCTL2		sfr	0f84h		; Clock Control 2
HFXORDY		__z8bit	CLKCTL2,7	;  High Frequency Crystal Oscillator (HFXO) Ready Flag
HFXOBAND	__z8bfield CLKCTL2,2,2	;  High Frequency Crystal Oscillator (HFXO) Frequency Band
HFXOEN		__z8bit	CLKCTL2,0	;  High Frequency Crystal Oscillator (HFXO) Enable
CLKCTL3		sfr	0f85h		; Clock Control 3
FLLNDIVH	__z8bfield CLKCTL3,0,8	;  Frequency Locked Loop (FLL) N-Divider High Byte
CLKCTL4		sfr	0f86h		; Clock Control 4
FLLNDIVL	__z8bfield CLKCTL4,0,2	;  Frequency Locked Loop (FLL) N-Divider Low Byte
CLKCTL5		sfr	0f87h		; Clock Control 5
FLLIRQE		__z8bit	CLKCTL5,6	;  FLL Lost Lock Interrupt Request Enable
FLLLL		__z8bit	CLKCTL5,5	;  FLL Lost Lock
FLLRDY		__z8bit	CLKCTL5,4	;  FLL Ready
FLADONE		__z8bit	CLKCTL5,3	;  FLL Fast Locking Algorithm Done
DCOEN		__z8bit	CLKCTL5,2	;  Digitally Controlled Oscillator (DCO) Enable
SEEDSEL		__z8bit	CLKCTL5,1	;  DCO Seed Select
FLLEN		__z8bit	CLKCTL5,0	;  Frequency Locked Loop (FLL) Enable
CLKCTL6		sfr	0f88h		; Clock Control 6
DCOCTLH		__z8bfield CLKCTL6,0,8	;  Digitally Controlled Oscillator (DCO) Control Word High Byte
CLKCTL7		sfr	0f89h		; Clock Control 7
DCOCTLL		__z8bfield CLKCTL7,0,8	;  Digitally Controlled Oscillator (DCO) Control Word Low Byte
CLKCTL8		sfr	0f8ah		; Clock Control 8
DCOCTLCH	__z8bfield CLKCTL8,0,8	;  Digitally Controlled Oscillator (DCO) Converged Control Word High Byte
CLKCTL9		sfr	0f8bh		; Clock Control 9
DCOCTLCL	__z8bfield CLKCTL9,0,8	;  Digitally Controlled Oscillator (DCO) converged control word Low Byte
CLKCTLA		sfr	0f8ch		; Clock Control A
PLLNDIV		__z8bfield CLKCTLA,0,8	;  Phase Locked Loop (PLL) Feedback Division Ratio
CLKCTLB		sfr	0f8dh		; Clock Control B
PLLRDIV		__z8bfield CLKCTLB,4,4	;  Phase Locked Loop (PLL) Reference Division Ratio
PLLODIV		__z8bfield CLKCTLB,0,3	;  Phase Locked Loop (PLL) Output Division Ratio
CLKCTLC		sfr	0f8eh		; Clock Control C
PLLRDY		__z8bit	CLKCTLC,7	;  Phase Locked Loop (PLL) Ready Flag
PLLSEL		__z8bit	CLKCTLC,1	;  PLL Source Select
PLLEN		__z8bit	CLKCTLC,0	;  Phased Locked Loop (PLL) Enable

TRMADR		sfr	0ff6h		; Trim Bit Address
TRMDR		sfr	0ff7h		; Trim Data

;----------------------------------------------------------------------------
; Flash Options

OPTIONS0	label	0000h
WDT_RES		__z8cbit OPTIONS0,7	;  Watchdog Timer Reset
WDT_AO		__z8cbit OPTIONS0,6	;  Watchdog Timer Always On
VBOCTL		__z8cbfield OPTIONS0,2,2 ;  Voltage Brown-Out Protection Control
FRP		__z8cbit OPTIONS0,1	;  Flash Read Protect
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
UART0_RX_vect	label	000eh		; UART 0 Receiver
UART0_TX_vect	label	0010h		; UART 0 Transmitter
USB_vect	label	0012h		; USB
USB_RESUME_vect	label	0014h		; USB Resume
I2C_vect	label	0016h		; I2C
SPI1_vect	label	0018h		; SPI1
DAC_vect	label	001ah		; DAC
DMA1_vect	label	001ch		; DMA1
DMA0_vect	label	001eh		; DMA0
ADC_vect	label	0020h		; ADC
SPI0_vect	label	0022h		; SPI0
LCD_vect	label	0024h		; LCD
RTC_vect	label	0026h		; RTC
A7_vect		label	0028h		; Port A7, selectable rising or falling input edge, or LVD
A6_vect		label	002ah		; Port A6, selectable rising or falling input edge or Comparator 0 Output
A5_vect		label	002ch		; Port A5, selectable rising or falling input edge or Comparator 1 Output
A4_vect		label	002eh		; Port A4 or Port D4, selectable rising or falling input edge
A3_vect		label	0030h		; Port A3 or Port D3, selectable rising or falling input edge
A2_vect		label	0032h		; Port A2 or Port D2, selectable rising or falling input edge
A1_vect		label	0034h		; Port A1 or Port D1, selectable rising or falling input edge
A0_vect		label	0036h		; Port A0, selectable rising or falling input edge
AES_vect	label	0038h		; AES
MCT_vect	label	003ah		; Multi-Channel Timer
UART1_RX_vect	label	003ch		; UART 1 Receiver
UART1_TX_vect	label	003eh		; UART 1 Transmitter
C3_vect		label	0040h		; Port C3, both input edges
DMA3_vect	label	0040h		; DMA3
C2_vect		label	0042h		; Port C2, both input edges
DMA2_vect	label	0042h		; DMA2
C1_vect		label	0044h		; Port C1, both input edges
C0_vect		label	0046h		; Port C0, both input edges
SYSCLK_vect	label	0048h		; System Clock Fail (not an interrupt)
WDGOSC_vect	label	004ah		; Watchdog Timer Oscillator Fail (not an interrupt)

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
		__defirq "3",0fc9h

T2I		__z8bit	IRQ0,7		;  Timer 2 Interrupt Request
T1I		__z8bit IRQ0,6		;  Timer 1 Interrupt Request
T0I		__z8bit IRQ0,5		;  Timer 0 Interrupt Request
U0RXI		__z8bit	IRQ0,4		;  UART 0 Receiver Interrupt Request
U0TXI		__z8bit	IRQ0,3		;  UART 0 Transmitter Interrupt Request
USBI		__z8bit	IRQ0,2		;  USB Interrupt Request
USBRI		__z8bit	IRQ0,1		;  USB Resume Interrupt Request
I2CI		__z8bit	IRQ0,0		;  I2C Interrupt Request

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
USBENH		__z8bit	IRQ0ENH,2	;  USB Interrupt Enable & Priority
USBENL		__z8bit	IRQ0ENL,2
USBRENH		__z8bit	IRQ0ENH,1	;  USB Resume Interrupt Enable & Priority
USBRENL		__z8bit	IRQ0ENL,1
I2CENH		__z8bit	IRQ0ENH,0	;  I2C Interrupt Enable & Priority
I2CENL		__z8bit	IRQ0ENL,0

SPI1I		__z8bit	IRQ1,7		;  SPI1 Interrupt Request
DACI 		__z8bit	IRQ1,6		;  DAC Interrupt Request
DMA1I		__z8bit	IRQ1,5		;  DMA1 Interrupt Request
DMA0I		__z8bit	IRQ1,4		;  DMA0 Interrupt Request
ADCI 		__z8bit	IRQ1,3		;  ADC Interrupt Request
SPI0I		__z8bit	IRQ1,2		;  SPI0 Interrupt Request
LCDI 		__z8bit	IRQ1,1		;  LCD Interrupt Request
RTCI 		__z8bit	IRQ1,0		;  RTC Interrupt Request

SPI1ENH		__z8bit	IRQ1ENH,7	;  SPI1 Interrupt Enable & Priority
SPI1ENL		__z8bit	IRQ1ENL,7
DACENH		__z8bit	IRQ1ENH,6	;  DAC Interrupt Enable & Priority
DACENL		__z8bit	IRQ1ENL,6
DMA1ENH		__z8bit	IRQ1ENH,5	;  DMA1 Interrupt Enable & Priority
DMA1ENL		__z8bit	IRQ1ENL,5
DMA0ENH		__z8bit	IRQ1ENH,4	;  DMA0 Interrupt Enable & Priority
DMA0ENL		__z8bit	IRQ1ENL,4
ADCENH		__z8bit	IRQ1ENH,3	;  ADC Interrupt Enable & Priority
ADCENL		__z8bit	IRQ1ENL,3
SPI0ENH		__z8bit	IRQ1ENH,2	;  SPI0 Interrupt Enable & Priority
SPI0ENL		__z8bit	IRQ1ENL,2
LCDENH		__z8bit	IRQ1ENH,1	;  LCD Interrupt Enable & Priority
LCDENL		__z8bit	IRQ1ENL,1
RTCENH		__z8bit	IRQ1ENH,0	;  RTC Interrupt Enable & Priority
RTCENL		__z8bit	IRQ1ENL,0

PA7VI		__z8bit	IRQ2,7		;  Port A7 or LVD Interrupt Request
PA6CI		__z8bit	IRQ2,6		;  Port A6 or Comparator 0 Interrupt Request
PA5CI		__z8bit	IRQ2,5		;  Port A5 or Comparator 1 Interrupt Request
PA4DI		__z8bit	IRQ2,4		;  Port A4 or Port D4 Interrupt Request
PA3DI		__z8bit	IRQ2,3		;  Port A3 or Port D3 Interrupt Request
PA2DI		__z8bit	IRQ2,2		;  Port A2 or Port D2 Interrupt Request
PA1DI		__z8bit	IRQ2,1		;  Port A1 or Port D1 Interrupt Request
PA0I		__z8bit	IRQ2,0		;  Port A0 Interrupt Request

PA7VENH		__z8bit	IRQ2ENH,7	;  Port A7 or LVD Interrupt Enable & Priority
PA7VENL		__z8bit	IRQ2ENL,7
PA6C0ENH	__z8bit	IRQ2ENH,6	;  Port A6 or Comparator 0 Interrupt Enable & Priority
PA6C0ENL	__z8bit	IRQ2ENL,6
PA5C1ENH	__z8bit	IRQ2ENH,5	;  Port A5 or Comparator 1 Interrupt Enable & Priority
PA5C1ENL	__z8bit	IRQ2ENL,5
PAD4ENH		__z8bit	IRQ2ENH,4	;  Port A4 or Port D4 Interrupt Enable & Priority
PAD4ENL		__z8bit	IRQ2ENL,4
PAD3ENH		__z8bit	IRQ2ENH,3	;  Port A3 or Port D3 Interrupt Enable & Priority
PAD3ENL		__z8bit	IRQ2ENL,3
PAD2ENH		__z8bit	IRQ2ENH,2	;  Port A2 or Port D2 Interrupt Enable & Priority
PAD2ENL		__z8bit	IRQ2ENL,2
PAD1ENH		__z8bit	IRQ2ENH,1	;  Port A1 or Port D1 Interrupt Enable & Priority
PAD1ENL		__z8bit	IRQ2ENL,1
PAD0ENH		__z8bit	IRQ2ENH,0	;  Port A0 Interrupt Enable & Priority
PAD0ENL		__z8bit	IRQ2ENL,0

AESI		__z8bit	IRQ3,7		;  AES Interrupt Request
MCTI		__z8bit	IRQ3,6		;  Multi-Channel Timer Interrupt Request
U1RXI		__z8bit	IRQ3,5		;  UART 1 Receiver Interrupt Request
U1TXI		__z8bit	IRQ3,4		;  UART 1 Transmitter Interrupt Reques
PC3I		__z8bit	IRQ3,3		;  Port C3 or...
DMA3I		__z8bit	IRQ3,3		;  DMA 3 Interrupt Request
PC2I		__z8bit	IRQ3,2		;  Port C2 or...
DMA2I		__z8bit	IRQ3,2		;  DMA 2 Interrupt Request
PC1I		__z8bit	IRQ3,1		;  Port C1 Interrupt Request
PC0I		__z8bit	IRQ3,0		;  Port C0 Interrupt Request

AESENH		__z8bit	IRQ3ENH,7	;  AES Interrupt Enable & Priority
AESENL		__z8bit	IRQ3ENL,7
MCTENH		__z8bit	IRQ3ENH,6	;  Multi-Channel Timer Interrupt Enable & Priority
MCTENL		__z8bit	IRQ3ENL,6
U1RENH		__z8bit	IRQ3ENH,5	;  UART1 Receive Interrupt Enable & Priority
U1RENL		__z8bit	IRQ3ENL,5
U1TENH		__z8bit	IRQ3ENH,4	;  UART1 Transmit Interrupt Enable & Priority
U1TENL		__z8bit	IRQ3ENL,4
C3ENH		__z8bit	IRQ3ENH,3	;  Port C3 or...
C3ENL		__z8bit	IRQ3ENL,3
DMA3ENH		__z8bit	IRQ3ENH,3	;  DMA3  Interrupt Enable & Priority
DMA3ENL		__z8bit	IRQ3ENL,3
C2ENH		__z8bit	IRQ3ENH,2	;  Port C2 or...
C2ENL		__z8bit	IRQ3ENL,2
DMA2ENH		__z8bit	IRQ3ENH,2	;  DMA2 Interrupt Enable & Priority
DMA2ENL		__z8bit	IRQ3ENL,2
C1ENH		__z8bit	IRQ3ENH,1	;  Port C1 Interrupt Enable & Priority
C1ENL		__z8bit	IRQ3ENL,1
C0ENH		__z8bit	IRQ3ENH,0	;  Port C0 Interrupt Enable & Priority
C0ENL		__z8bit	IRQ3ENL,0

IRQES		sfr	0fcch		; Interrupt Edge Select
IRQSS0		sfr	0fcdh		; Shared Interrupt Select Register 0
PA7VS		__z8bit	IRQSS0,7	;  PA7/LVD Selection
PA6CS		__z8bit	IRQSS0,6	;  PA6/Comparator 0 Selection
PA5CS		__z8bit	IRQSS0,5	;  PA5/Comparator 1 Selection
PAD4S		__z8bit	IRQSS0,4	;  PA4/PD4 Selection
PAD3S		__z8bit	IRQSS0,3	;  PA3/PD3 Selection
PAD2S		__z8bit	IRQSS0,2	;  PA2/PD2 Selection
PAD1S		__z8bit	IRQSS0,1	;  PA1/PD1 Selection
IRQSS1		sfr	0fceh		; Shared Interrupt Select Register 1
PCDMA3S		__z8bit	IRQSS1,3	;  PC3/DMA3 Selection
PCDMA2S		__z8bit	IRQSS1,2	;  PC2/DMA2 Selection
IRQCTL		sfr	0fcfh		; Interrupt Control
IRQE		__z8bit	IRQCTL,7	;  Interrupt Request Enable

;----------------------------------------------------------------------------
; Event System

ESSSA		sfr	0f98h		; Event System Source Subaddress
ESSSD		sfr	0f99h		; Event System Source Subdata
CHSRCSEL	__z8bfield ESSSD,0,7	;  Event System Channel Source Selection
ESDSA		sfr	0f9ah		; Event System Destination Subaddress
ESDSD		sfr	0f9bh		; Event System Destination Subdata
DSTCON		__z8bit	ESDSD,3		;  Event System Destination Connection
DSTCHSEL	__z8bfield ESDSD,0,3	;  Event System Destination Channel Selection

;----------------------------------------------------------------------------
; Flash Memory Control

FCTL		sfr	0ff8h		; Flash Control
FCMD		__z8bfield FCTL,0,8	;  Flash Command
FSTAT		sfr	0ff8h		; Flash Status
FPS		sfr	0ff9h		; Flash Page Select
INFO_EN		__z8bit	FPS,7		;  Information Area Enable
PAGE		__z8bfield FPS,0,7	;  Page Select
FPROT		sfr	0ff9h		; Flash Sector Protect
FPCONFIG	sfr	0ffah		; Flash Programming Configuration
PMODE		__z8bit	FPCONFIG,0	;  Programming Mode

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
		__defgpio "J",0fbch

;----------------------------------------------------------------------------
; LED Controller

LEDEN		sfr	0f82h		; LED Drive Enable
LEDLVLH		sfr	0f83h		; LED Drive Level High
LEDLVLL		sfr	0f84h		; LED Drive Level Low

;----------------------------------------------------------------------------
; Timer

__defmytimer	macro	NUM,Base,Base2,Base3
		__deftimer NUM,Base,1,0
T{NUM}MODE3	equ	T{NUM}MODEHI
T{NUM}PWM1H	sfr	Base2+0		; Timer PWM1 High Byte
T{NUM}PWM1L	sfr	Base2+1		; Timer PWM1 Low Byte
T{NUM}PWM1	sfr	T{NUM}PWM1H
T{NUM}CTL2	sfr	Base2+2		; Timer Control 2
T{NUM}PWM0UE	__z8bit	T{NUM}CTL2,5	;  PWM0 Update Enable
T{NUM}TPOLHI	__z8bit	T{NUM}CTL2,4	;  Timer Input/Output Polarity High Bit
T{NUM}OUTCTL	__z8bit	T{NUM}CTL2,2	;  Timer Output Control
T{NUM}TCLKS	__z8bfield T{NUM}CTL2,0,2 ;  Timer Clock Source
T{NUM}STAT	sfr	Base2+3		; Timer Status
T{NUM}NEF	__z8bit	T{NUM}STAT,7	;  Noise Event Flag
T{NUM}PWM1EO	__z8bit	T{NUM}STAT,5	;  PWM 1 Event Overrun
T{NUM}PWM0EO	__z8bit	T{NUM}STAT,4	;  PWM 0 Event Overrun
T{NUM}RTOEF	__z8bit	T{NUM}STAT,3	;  Reload Time-Out Event Flag
T{NUM}PWM1EF	__z8bit	T{NUM}STAT,1	;  PWM 1 Event Flag
T{NUM}PWM0EF	__z8bit	T{NUM}STAT,0	;  PWM 0 Event Flag
T{NUM}NFC	sfr	Base3+0		; Timer Noise Filter Control
T{NUM}NFCTL	__z8bfield T{NUM}NFC,4,4 ;  Noise Filter Control
T{NUM}NFCON	__z8bit	T{NUM}NFC,7	;  Noise Filter Connection
		endm

		__defmytimer "0",0f00h,0f20h,0f2ch
		__defmytimer "1",0f08h,0f24h,0f2dh
		__defmytimer "2",0f10h,0f28h,0f2eh

;----------------------------------------------------------------------------
; RTC

RTC_SEC		sfr	0f30h		; Real-Time Clock Seconds
TEN_SEC		__z8bfield RTC_SEC,4,3	;  Current Seconds Tens
SEC		__z8bfield RTC_SEC,0,4	;  Current Seconds Ones
RTC_MIN		sfr	0f31h		; Real-Time Clock Minutes
TEN_MIN		__z8bfield RTC_MIN,4,3	;  Current Minutes Tens
MIN		__z8bfield RTC_MIN,0,4	;  Current Minutes Ones
RTC_HRS		sfr	0f32h		; Real-Time Clock Hours
TEN_HRS		__z8bfield RTC_HRS,4,2	;  Current Hours Tens
HRS		__z8bfield RTC_HRS,0,4	;  Current Hours Ones
RTC_DOM		sfr	0f33h		; Real-Time Clock Day-of-the-Month
TENS_DOM	__z8bfield RTC_DOM,4,2	;  Current Day Of The Month Tens
DOM		__z8bfield RTC_DOM,0,4	;  Current Day Of The Month Ones
RTC_DOW		sfr	0f34h		; Real-Time Clock Day-of-the-Week
DOW		__z8bfield RTC_DOW,0,3	;  Current Day Of The Week
RTC_MON		sfr	0f35h		; Real-Time Clock Month
TENS_MON	__z8bfield RTC_MON,4,1	;  Current Month Tens
MON		__z8bfield RTC_MON,0,3	;  Current Month Ones
RTC_YR		sfr	0f36h		; Real-Time Clock Year
TENS_YR		__z8bfield RTC_YR,4,4	;  Current Year Tens
YR		__z8bfield RTC_YR,0,4	;  Current Year Ones
RTC_ASEC	sfr	0f37h		; Real-Time Clock Alarm Seconds
ATEN_SEC	__z8bfield RTC_ASEC,4,3	;  Alarm Seconds Tens
ASEC		__z8bfield RTC_ASEC,0,4	;  Alarm Seconds Ones
RTC_AMIN	sfr	0f38h		; Real-Time Clock Alarm Minutes
ATEN_MIN	__z8bfield RTC_AMIN,4,3	;  Alarm Minutes Tens
AMIN		__z8bfield RTC_AMIN,0,4	;  Alarm Minutes Ones
RTC_AHRS	sfr	0f39h		; Real-Time Clock Alarm Hours
ATEN_HRS	__z8bfield RTC_AHRS,4,2	;  Alarm Hours Tens
AHRS		__z8bfield RTC_AHRS,0,4	;  Alarm Hours Ones
RTC_ADOM	sfr	0f3ah		; Real-Time Clock Alarm Day-of-the-Month
ATEN_DOM	__z8bfield RTC_ADOM,4,2	;  Alarm Day Of The Month Tens
ADOM		__z8bfield RTC_ADOM,0,4	;  Alarm Day Of The Month Ones
RTC_ADOW	sfr	0f3bh		; Real-Time Clock Alarm Day-of-the-Week
ADOW		__z8bfield RTC_ADOW,0,3	;  Alarm Day Of The Week
RTC_ACTRL	sfr	0f3ch		; Real-Time Clock Alarm Control
ADOM_EN		__z8bit	RTC_ACTRL,4	;  Alarm Day Of The Month Enable
ADOW_EN		__z8bit	RTC_ACTRL,3	;  Alarm Day Of The Week Enable
AHRS_EN		__z8bit	RTC_ACTRL,2	;  Alarm Hours Enable
AMIN_EN		__z8bit	RTC_ACTRL,1	;  Alarm Minutes Enable
ASEC_EN		__z8bit	RTC_ACTRL,0	;  Alarm Seconds Enable
RTC_TIM		sfr	0f3eh		; Real-Time Clock Timing
RTC_PRESCALE	__z8bfield RTC_TIM,4,3	;  RTC Prescale
FREQ_SEL	__z8bfield RTC_TIM,2,2	;  RTC Frequency Select
CLK_SEL		__z8bfield RTC_TIM,0,2	;  RTC Source Select
RTC_CTRL	sfr	0f3fh		; Real-Time Clock Control
SYNC		__z8bit	RTC_CTRL,7	;  RTC Synchronize
ALARM		__z8bit	RTC_CTRL,6	;  RTC Alarm
BCD_EN		__z8bit	RTC_CTRL,4	;  Binary Coded Decimal Enable
RTC_MODE	__z8bit	RTC_CTRL,3	;  RTC Mode
DAY_SAV		__z8bit	RTC_CTRL,1	;  Daylight Savings Time
RTC_LOCK	__z8bit	RTC_CTRL,0	;  RTC Count Lock

;----------------------------------------------------------------------------
; Multi-Channel Timer

		__defmct

;----------------------------------------------------------------------------
; DMA

__defdma	macro	NUM,Base
DMA{NUM}SA	sfr	Base+0		; DMA Subaddress/Status
IRQS{NUM}	__z8bit	DMA{NUM}SA,7	;  Interrupt Request Status
LLACT{NUM}	__z8bit	DMA{NUM}SA,5	;  Linked List Active
ACT{NUM}	__z8bit	DMA{NUM}SA,4	;  DMA Active
DMA{NUM}SD	sfr	Base+1		; DMA Subdata
DMA{NUM}SRCH	equ	0		; DMA Source Address High
TXLIST{NUM}	__z8bit	DMA{NUM}SD,7	;  Transfer In List
DMA{NUM}SRCL	equ	1		; DMA Source Address Low
DMA{NUM}DSTH	equ	2		; DMA Destination Address High
DMA{NUM}DSTL	equ	3		; DMA Destination Address Low
DMA{NUM}CNTH	equ	4		; DMA Count High
WMCNT{NUM}	__z8bfield DMA{NUM}SD,4,4 ;  Watermark Count
DMA{NUM}CNTL	equ	5		; DMA Count Low
DMA{NUM}CTL0	equ	6		; DMA Control 0
LLCTL{NUM}	__z8bfield DMA{NUM}SD,6,2 ;  Linked List Control
EOCIRQE{NUM}	__z8bit	DMA{NUM}SD,5	;  End-of-Count Interrupt Request Enable
HALT{NUM}	__z8bit	DMA{NUM}SD,4	;  Halt Upon Descriptor Completion
DSTCTL{NUM}	__z8bfield DMA{NUM}SD,2,2 ;  Destination Address Control
SRCCTL{NUM}	__z8bfield DMA{NUM}SD,0,2 ;  Source Address Control
DMA{NUM}CTL1	equ	7		; DMA Control 1
ENABLE{NUM}	__z8bit	DMA{NUM}SD,7	;  DMA Channel Enable
REQSEL{NUM}	__z8bfield DMA{NUM}SD,0,5 ;  DMA Requestor Selection
DMA{NUM}LAH	equ	8		; DMA Linked List Descriptor Address High
DMA{NUM}LAL	equ	9		; DMA Linked List Descriptor Address Low
		endm
		
		__defdma "0",0fa8h
		__defdma "1",0faah
		__defdma "2",0fach
		__defdma "3",0faeh
		
DMACTL		sfr	0fb0h		; DMA Control
GDISABLE	__z8bit	DMACTL,7	;  DMA Global Disable
PRIORITY	__z8bit	DMACTL,6	;  DMA Priority Select
BURST		__z8bfield DMACTL,4,2	;  Burst Transfers
AUTOINC		__z8bit	DMACTL,2	;  Autoincrement Enable
CHAIN32		__z8bit	DMACTL,1	;  Chain DMA3 and DMA2
CHAIN10		__z8bit	DMACTL,0	;  Chain DMA1 and DMA0

;----------------------------------------------------------------------------
; LCD

LCDSA		sfr	0fb1h		; LCD Subaddress
LCDSD		sfr	0fb2h		; LCD Subdata
LCDCLK		sfr	0fb3h		; LCD Clock
CLKSEL		__z8bit	LCDCLK,7	;  LCD Clock Selection
LCD_PRESCALE	__z8bfield LCDCLK,4,3	;  LCD Clock Prescale Divider
FDIV		__z8bfield LCDCLK,0,4	;  Frame Divider
LCDCTL0		sfr	0fb4h		; LCD Control 0
BDIV		__z8bfield LCDCTL0,3,5	;  Blinking Divider Ratio
BMODE		__z8bfield LCDCTL0,1,2	;  Blinking Mode
LCDCTL1		sfr	0fb5h		; LCD Control 1
HBDDUR		__z8bfield LCDCTL1,5,3	;  Higher Bias Drive Duration
CPEN		__z8bit	LCDCTL1,4	;  Charge Pump Enable
BIASGSEL	__z8bit	LCDCTL1,3	;  Bias Generator Selection
CONTRAST	__z8bfield LCDCTL1,0,3	;  Contrast Control
LCDCTL2		sfr	0fb6h		; LCD Control 2
MSTAT		__z8bit	LCDCTL2,7	;  LCD Memory Status
IRQS		__z8bit	LCDCTL2,6	;  Interrupt Request Select
LCDMODE		__z8bfield LCDCTL2,2,4	;  LCD Operating Mode
DMMODE		__z8bfield LCDCTL2,0,2	;  Display Memory Mode
LCDCTL3		sfr	0fb7h		; LCD Control 3
CPTSEL		__z8bit	LCDCTL3,6	;  Charge Pump Type Select
VLCDDIR		__z8bit	LCDCTL3,5	;  VLCD Direction
WGENEN		__z8bit	LCDCTL3,4	;  Waveform Generator Enable

;----------------------------------------------------------------------------
; AES Unit

AESDATA		sfr	0fb8h		; AES Data
AESIV		sfr	0fb8h		; AES Initialization Vector
AESKEY		sfr	0fb9h		; AES Key
AESCTL		sfr	0fbah		; AES Control
AUTODIS		__z8bit	AESCTL,1	;  Auto-Start Mode Disable
IRQ		__z8bit	AESCTL,5	;  Interrupt Control
AESMODE		__z8bfield AESCTL,3,2	;  Confidentiality Mode Select
IVEN		__z8bit	AESCTL,2	;  Initialization Vector Enable
DECRYPT		__z8bit	AESCTL,1	;  Decryption/Encryption Select
AESEN		__z8bit	AESCTL,0	;  AES Enable
AESSTAT		sfr	0fbbh		; AES Status
START_BUSY	__z8bit	AESSTAT,7	;  AES Start/Busy Status
ERROR		__z8bit	AESSTAT,3	;  ERROR Status
IVLD		__z8bit	AESSTAT,2	;  Initialization Vector Load Status
KEYLD		__z8bit	AESSTAT,1	;  Key Load Status
DATALD		__z8bit	AESSTAT,0	;  Data Load Status

;----------------------------------------------------------------------------
; LDD UART

__defldduart	macro	NUM,Base
U{NUM}TXD	sfr	Base+0		; LIN UART Transmit Data
U{NUM}RXD	sfr	Base+0		; LIN UART Receive Data
U{NUM}STAT0	sfr	Base+1		; LIN UART Status 0
U{NUM}RDA	__z8bit	U{NUM}STAT0,7	;  Receive Data Available (LIN+UART+DALI+DMX)
U{NUM}PE	__z8bit	U{NUM}STAT0,6	;  Parity Error (UART)
U{NUM}PLE	__z8bit	U{NUM}STAT0,6	;  Physical Layer Error (LIN)
U{NUM}BPE	__z8bit	U{NUM}STAT0,6	;  Biphase Error (DALI)
U{NUM}OE	__z8bit	U{NUM}STAT0,5	;  Overrun Error (LIN+UART+DALI+DMX)
U{NUM}FE	__z8bit	U{NUM}STAT0,4	;  Framing Error (LIN+UART+DALI)
U{NUM}BRKD	__z8bit	U{NUM}STAT0,3	;  Break Detect (LIN+UART+DMX)
U{NUM}CLSN	__z8bit	U{NUM}STAT0,3	;  Collision Detect Error (DALI)
U{NUM}TDRE	__z8bit	U{NUM}STAT0,2	;  Transmitter Data Register Empty (LIN+UART+DALI+DMX)
U{NUM}TXE	__z8bit	U{NUM}STAT0,1	;  Transmitter Empty (LIN+UART+DALI+DMX)
U{NUM}CTS	__z8bit	U{NUM}STAT0,0	;  Clear to Send Signal (UART+DALI+DMX)
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
U{NUM}NFEN	__z8bit	U{NUM}CTL1,7	;  Noise Filter Enable (MSEL=001)
U{NUM}NFCTL	__z8bfield U{NUM}CTL1,4,3 ;  Noise Filter Control (MSEL=001)
U{NUM}LMST	__z8bit	U{NUM}CTL1,7	;  LIN MASTER Mode (MSEL=010)
U{NUM}LSLV	__z8bit	U{NUM}CTL1,6	;  LIN SLAVE Mode (MSEL=010)
U{NUM}ABEN	__z8bit	U{NUM}CTL1,5	;  Autobaud Enable (MSEL=010)
U{NUM}ABIEN	__z8bit	U{NUM}CTL1,4	;  Autobaud Interrupt Enable (MSEL=010)
U{NUM}LinState	__z8bfield U{NUM}CTL1,2,2 ;  LIN State Machine (MSEL=010)
U{NUM}TxBreakLength __z8bfield U{NUM}CTL1,0,2 ;  TxBreakLength (MSEL=010)
U{NUM}MULTTXE	__z8bit	U{NUM}CTL1,7	;  Multiple-Byte Transmit Enable (MSEL=100)
U{NUM}MULTRXE	__z8bit	U{NUM}CTL1,6	;  Multiple-Byte Receive Enable (MSEL=100)
U{NUM}BPEN	__z8bit	U{NUM}CTL1,5	;  Biphase Encoding Enable (MSEL=100)
U{NUM}BPENC	__z8bit	U{NUM}CTL1,4	;  Biphase Encoding (MSEL=100)
U{NUM}STRTPOL	__z8bit	U{NUM}CTL1,3	;  Start Bit Polarity (MSEL=100)
U{NUM}BITORD	__z8bit	U{NUM}CTL1,2	;  Bit Order (MSEL=100)
U{NUM}CLSNE	__z8bit	U{NUM}CTL1,1	;  Collision Detection Enable (MSEL=100)
U{NUM}PARTRXE	__z8bit	U{NUM}CTL1,0	;  Partial Byte Reception Enable (MSEL=100)
U{NUM}DMXMST	__z8bit	U{NUM}CTL1,7	;  DMX Master Mode (MSEL=101)
U{NUM}DMXSLV	__z8bit	U{NUM}CTL1,6	;  DMX Slave Mode (MSEL=101)
U{NUM}DMXSIRQ	__z8bfield U{NUM}CTL1,4,2 ;  DMX Slave Interrupt Control (MSEL=101)
U{NUM}AUTOBRK	__z8bit	U{NUM}CTL1,2	;  Automatic Break (MSEL=101)
U{NUM}WFBRK	__z8bit	U{NUM}CTL1,1	;  Wait for Break (MSEL=101)
U{NUM}COMP_ADDR8 __z8bit	U{NUM}CTL1,0	;  Comparison Address bit 8 (MSEL=101)
U{NUM}MDSTAT	sfr	Base+4		; LIN UART Mode Select and Status
U{NUM}MSEL	__z8bfield U{NUM}MDSTAT,5,3 ;  Mode Select
U{NUM}MODESTAT __z8bfield U{NUM}MDSTAT,0,5 ;  Mode Status
U{NUM}ADDR	sfr	Base+5		; UART Address Compare
U{NUM}BRH	sfr	Base+6		; UART Baud Rate High Byte
U{NUM}BRL	sfr	Base+7		; UART Baud Rate Low Byte
U{NUM}BR	sfr	U{NUM}BRH
		endm

		__defldduart "0",0f40h
		__defldduart "1",0f48h

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
BIRQS		__z8bit	I2CCTL,4	;  Baud Rate Generator Interrupt Request
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
DMAIF		__z8bit	I2CMODE,7	;  DMA Interface Mode
I2COPMODE	__z8bfield I2CMODE,5,2	;  Selects the I2C Controller Operational Mode
IRM		__z8bit	I2CMODE,4	;  Interactive Receive Mode
GCE		__z8bit	I2CMODE,3	;  General Call Address Enable
SLA		__z8bfield I2CMODE,1,2	;  Slave Address Bits 9 and 8
DIAG		__z8bit	I2CMODE,0	;  Diagnostic Mode
I2CSLVAD	sfr	0f57h		; I2C Slave Address Register

;----------------------------------------------------------------------------
; USB

USBSA		sfr	0f59h		; USB Subaddress
ADDRSEL		__z8bit	USBSA,7		;  Addressing Select
USBSD		sfr	0f5ah		; USB Subdata
USBCTL		sfr	0f5bh		; USB Control
AI		__z8bit	USBCTL,6	;  Addressing Select
EPSEL		__z8bfield USBCTL,3,3	;  Endpoint Select
USBDMA0CTL	sfr	0f5ch		; USB DMA 0 Control
EPSEL0		__z8bfield USBDMA0CTL,3,3 ; End Point Buffer Select  
STARTDMA0	__z8bit	USBDMA0CTL,0	;  Start DMA
USBDMA1CTL	sfr	0f5dh		; USB DMA 1 Control
EPSEL1		__z8bfield USBDMA0CTL,3,3 ; End Point Buffer Select  
STARTDMA1	__z8bit	USBDMA0CTL,0	;  Start DMA
USBDMADATA	sfr	0f5eh		; USB DMA Data
USBIRQCTL	sfr	0f5fh		; USB Interrupt Control
RIRQE		__z8bit	USBIRQCTL,1	;  Resume Interrupt Request Enable
WAKEUP		__z8bit	USBIRQCTL,0	;  Wake-Up (Device-Initiated Resume)

USBO1ADDR	equ	01h		; USB OUT Endpoint 1 Start Address Subregister
USBO2ADDR	equ	02h		; USB OUT Endpoint 2 Start Address Subregister
USBO3ADDR	equ	03h		; USB OUT Endpoint 3 Start Address Subregister
USBISTADDR	equ	08h		; USB IN Endpoints Start Address Subregister
INSTADDR	__z8bfield USBSD,0,7	;  IN Start Address
USBI1ADDR	equ	09h		; USB IN Endpoint 1 Start Address Subregister
USBI2ADDR	equ	0ah		; USB IN Endpoint 2 Start Address Subregister
USBI3ADDR	equ	0bh		; USB IN Endpoint 3 Start Address Subregister
USBCLKGATE	equ	10h		; USB Clock Gate Subregister
USBIID		equ	28h		; USB Interrupt Identification Subregister
IID		__z8bfield USBSD,2,5	;  Interrupt Identification (Source)
USBINIRQ	equ	29h		; USB IN Interrupt Request Subregister
IN3IRQ		__z8bit	USBSD,3		;  IN Endpoint 3 Interrupt Request
IN2IRQ		__z8bit	USBSD,2		;  IN Endpoint 2 Interrupt Request
IN1IRQ		__z8bit	USBSD,1		;  IN Endpoint 1 Interrupt Request
IN0IRQ		__z8bit	USBSD,0		;  IN Endpoint 0 Interrupt Request
USBOUTIRQ	equ	2ah		; USB OUT Interrupt Request Subregister
OUT3IRQ		__z8bit	USBSD,3		;  OUT Endpoint 3 Interrupt Request
OUT2IRQ		__z8bit	USBSD,2		;  OUT Endpoint 2 Interrupt Request
OUT1IRQ		__z8bit	USBSD,1		;  OUT Endpoint 1 Interrupt Request
OUT0IRQ		__z8bit	USBSD,0		;  OUT Endpoint 0 Interrupt Request
USBIRQ		equ	2bh		; USB Protocol Interrupt Request Subregister
URESIRQ		__z8bit	USBSD,4		;  USB Reset Bus State Interrupt Request
SUSPIRQ		__z8bit	USBSD,3		;  USB Suspend Interrupt Request
SUTOKIRQ	__z8bit	USBSD,2		;  USB Setup Token Interrupt Request
SOFIRQ		__z8bit	USBSD,1		;  USB Start-of-Frame Interrupt Request
SUDAVIRQ	__z8bit	USBSD,0		;  USB Setup Stage Data Valid Interrupt Request
USBINIEN	equ	2ch		; USB IN Interrupt Enable Subregister
IN3IEN		__z8bit	USBSD,3		;  IN Endpoint 3 Interrupt Enable
IN2IEN		__z8bit	USBSD,2		;  IN Endpoint 2 Interrupt Enable
IN1IEN		__z8bit	USBSD,1		;  IN Endpoint 1 Interrupt Enable
IN0IEN		__z8bit	USBSD,0		;  IN Endpoint 0 Interrupt Enable
USBOUTIEN	equ	2dh		; USB OUT Interrupt Enable Subregister
OUT3IEN		__z8bit	USBSD,3		;  OUT Endpoint 3 Interrupt Enable
OUT2IEN		__z8bit	USBSD,2		;  OUT Endpoint 2 Interrupt Enable
OUT1IEN		__z8bit	USBSD,1		;  OUT Endpoint 1 Interrupt Enable
OUT0IEN		__z8bit	USBSD,0		;  OUT Endpoint 0 Interrupt Enable
USBIEN		equ	2eh		; USB Protocol Interrupt Enable Subregister
URESIEN		__z8bit	USBSD,4		;  USB Reset Bus State Interrupt Enable
SUSPIEN		__z8bit	USBSD,3		;  USB Suspend Interrupt Enable
SUTOKIEN	__z8bit	USBSD,2		;  USB Setup Token Interrupt Enable
SOFIEN		__z8bit	USBSD,1		;  USB Start-of-Frame Interrupt Enable
SUDAVIEN	__z8bit	USBSD,0		;  USB Setup Stage Data Valid Interrupt Enable
USBEP0CS	equ	34h		; USB Endpoint 0 Control and Status Subregister
CHGSET		__z8bit	USBSD,5		;  Setup Buffer Contents Changed
DSTALL		__z8bit	USBSD,4		;  EP0 Data Stall
OUTBUSY0	__z8bit	USBSD,3		;  EP0 OUT Busy Status
INBUSY0		__z8bit	USBSD,2		;  EP0 IN Busy Status
HSNAK		__z8bit	USBSD,1		;  EP0 Handshake NA
STALL0		__z8bit	USBSD,0		;  EP0 Stall
USBI0BC		equ	35h		; USB IN 0 Byte Count Subregister
USBI1BC		equ	37h		; USB IN 1 Byte Count Subregister
USBI2BC		equ	39h		; USB IN 2 Byte Count Subregister
USBI3BC		equ	3bh		; USB IN 3 Byte Count Subregister
USBI1CS		equ	36h		; USB IN 1 Control and Status Subregister
USBI2CS		equ	38h		; USB IN 2 Control and Status Subregister
USBI3CS		equ	3ah		; USB IN 3 Control and Status Subregister
INBUSYn		__z8bit	USBSD,1		;  IN Busy Status
STALLn		__z8bit	USBSD,0		;  IN Stall
USBO0BC		equ	45h		; USB OUT 0 Byte Count Subregister
USBO1BC		equ	47h		; USB OUT 1 Byte Count Subregister
USBO2BC		equ	49h		; USB OUT 2 Byte Count Subregister
USBO3BC		equ	4bh		; USB OUT 3 Byte Count Subregister
USBO1CS		equ	46h		; USB OUT 1 Control and Status Subregister
USBO2CS		equ	48h		; USB OUT 2 Control and Status Subregister
USBO3CS		equ	4ah		; USB OUT 3 Control and Status Subregister
OUTBUSYn	__z8bit	USBSD,1		;  OUT Busy Status
OUTSTALL	__z8bit	USBSD,0		;  Out Stall
USBCS		equ	56h		; USB Control and Status Subregister
DEVRSUME	__z8bit	USBSD,7		;  Wake-Up Source
SOFWDOG		__z8bit	USBSD,5		;  Start of Frame (SOF) Watchdog
DISCON		__z8bit	USBSD,3		;  Disconnect the Internal Pull-Up Resistor
FORCEJ		__z8bit	USBSD,1		;  Force the Data J Bus State
SIGRSUME	__z8bit	USBSD,0		;  Signal Remote Device Resume
USBTOGCTL	equ	57h		; USB Toggle Control Subregister
DATA		__z8bit	USBSD,7		;  Data Toggle Value
TDATA1		__z8bit	USBSD,6		;  Set Data Toggle to Data1
TDATA0		__z8bit	USBSD,5		;  Clear Data Toggle to Data0
INOUT		__z8bit	USBSD,4		;  IN/OUT Endpoint Select
EP		__z8bfield USBSD,0,2	;  Endpoint Select
USBFCL		equ	58h		; USB Frame Count Low Subregister
USBFCH		equ	59h		; USB Frame Count High Subregister
USBFNADDR	equ	5bh		; USB Function Address Subregister
USBPAIR		equ	5dh		; USB Endpoint Pairing Subregister
PROUT23		__z8bit	USBSD,3		;  OUT 2 and OUT 3 Pairing
PRIN23		__z8bit	USBSD,0		;  IN 2 and IN 3 Pairing
USBINVAL	equ	5eh		; USB IN Endpoint Valid Subregister
IN3VAL		__z8bit	USBSD,3		;  IN 3 Endpoint Valid
IN2VAL		__z8bit	USBSD,2		;  IN 2 Endpoint Valid
IN1VAL		__z8bit	USBSD,1		;  IN 1 Endpoint Valid
IN0VAL		__z8bit	USBSD,0		;  IN 0 Endpoint Valid
USBOUTVAL	equ	5fh		; USB OUT Endpoint Valid Subregister
OUT3VAL		__z8bit	USBSD,3		;  OUT 3 Endpoint Valid
OUT2VAL		__z8bit	USBSD,2		;  OUT 2 Endpoint Valid
OUT1VAL		__z8bit	USBSD,1		;  OUT 1 Endpoint Valid
OUT0VAL		__z8bit	USBSD,0		;  OUT 0 Endpoint Valid
USBISPADDR	equ	62h		; USB IN Endpoints Stop Address Subregister
USBSU0		equ	68h		; USB Setup Buffer Byte 0 Subregister
USBSU1		equ	69h		; USB Setup Buffer Byte 1 Subregister
USBSU2		equ	6ah		; USB Setup Buffer Byte 2 Subregister
USBSU3		equ	6bh		; USB Setup Buffer Byte 3 Subregister
USBSU4		equ	6ch		; USB Setup Buffer Byte 4 Subregister
USBSU5		equ	6dh		; USB Setup Buffer Byte 5 Subregister
USBSU6		equ	6eh		; USB Setup Buffer Byte 6 Subregister
USBSU7		equ	6fh		; USB Setup Buffer Byte 7 Subregister

;----------------------------------------------------------------------------
; ESPI

__defespi	macro	NUM,Base
ESPI{NUM}DATA	sfr	0f60h		; ESPI Data
ESPI{NUM}TDCR	sfr	0f61h		; ESPI Transmit Data Command
CRDR{NUM}	__z8bit	ESPI{NUM}TDCR,7	;  Clear Receive Data Register
TEOF{NUM}	__z8bit	ESPI{NUM}TDCR,1	;  Transmit End of Frame
SSV{NUM}	__z8bit	ESPI{NUM}TDCR,0	;  Slave Select Value
ESPI{NUM}CTL	sfr	0f62h		; ESPI Control
DIRQS{NUM}	__z8bit	ESPI{NUM}CTL,7	;  Data Interrupt Request Select
ESPIEN1{NUM}	__z8bit	ESPI{NUM}CTL,6	;  ESPI Enable and Direction Control
BRGCTL{NUM}	__z8bit	ESPI{NUM}CTL,5	;  Baud Rate Generator Control
PHASE{NUM}	__z8bit	ESPI{NUM}CTL,4	;  Phase Select
CLKPOL{NUM}	__z8bit	ESPI{NUM}CTL,3	;  Clock Polarity
WOR{NUM}	__z8bit	ESPI{NUM}CTL,2	;  Wire-OR (Open-Drain) Mode Enabled
MMEN{NUM}	__z8bit	ESPI{NUM}CTL,1	;  ESPI MASTER Mode Enable
ESPIEN0{NUM}	__z8bit	ESPI{NUM}CTL,0	;  ESPI Enable and Direction Control
ESPI{NUM}MODE	sfr	0f63h		; ESPI Mode
SSMD{NUM}	__z8bfield ESPI{NUM}MODE,5,3	;  Slave Select Mode
NUMBITS{NUM}	__z8bfield ESPI{NUM}MODE,2,3	;  Number of Data Bits Per Character to Transfer
SSIO{NUM}	__z8bit	ESPI{NUM}MODE,1	;  Slave Select I/O
SSPO{NUM}	__z8bit	ESPI{NUM}MODE,0	;  Slave Select Polarity
ESPI{NUM}STAT	sfr	0f64h		; ESPI Status
TDRE{NUM}	__z8bit	ESPI{NUM}STAT,7	;  Transmit Data Register Empty
TUND{NUM}	__z8bit	ESPI{NUM}STAT,6	;  Transmit Underrun
COL{NUM}	__z8bit	ESPI{NUM}STAT,5	;  Collision
ABT{NUM}	__z8bit	ESPI{NUM}STAT,4	;  SLAVE Mode Transaction Abort
ROVR{NUM}	__z8bit	ESPI{NUM}STAT,3	;  Receive Overrun
RDRNE{NUM}	__z8bit	ESPI{NUM}STAT,2	;  Receive Data Register Not Empty
TFST{NUM}	__z8bit	ESPI{NUM}STAT,1	;  Transfer Status
SLAS{NUM}	__z8bit	ESPI{NUM}STAT,0	;  Slave Select
ESPI{NUM}STATE	sfr	0f65h		; ESPI State
SCKI{NUM}	__z8bit	ESPI{NUM}STATE,7	;  Serial Clock Input
SDI{NUM}	__z8bit	ESPI{NUM}STATE,6	;  Serial Data Input
ESPI{NUM}BRH	sfr	0f66h		; ESPI Baud Rate High Byte
ESPI{NUM}BRL	sfr	0f67h		; ESPI Baud Rate Low Byte
ESP{NUM}IBR	sfr	ESPI{NUM}BRH
		endm
		
		__defespi "0",0f60h
		__defespi "1",0f68h

;----------------------------------------------------------------------------
; Analog Comparators

CMPCTL		sfr	0f8fh		; Comparator Control Register
VBIASEN		__z8bit	CMPCTL,7	;  VBIAS Enable
WINEN		__z8bit	CMPCTL,2	;  Window Mode Enable
CSTATUS		__z8bfield CMPCTL,0,2	;  Comparator Status
CMP0CTL0	sfr	0f90h		; Comparator 0 Control 0
CPOWER0		__z8bfield CMP0CTL0,6,2	;  Comparator Power/Speed Select
HYST0		__z8bfield CMP0CTL0,4,2	;  Hysteresis Level Select
INNSEL0		__z8bfield CMP0CTL0,2,2	;  Negative Input Signal Select
INPSEL0		__z8bfield CMP0CTL0,0,2	;  Positive Input Signal Select
CMP0CTL1	sfr	0f91h		; Comparator 0 Control 1
POLSEL0		__z8bit	CMP0CTL1,7	;  Polarity Select
PREFEN0		__z8bit	CMP0CTL1,6	;  Programmable Reference Enable
PREFSRC0	__z8bit	CMP0CTL1,5	;  Programmable Reference Source Selection
PREFLVL0	__z8bfield CMP0CTL1,0,5	;  Programmable Reference Level Selection
CMP1CTL0	sfr	0f92h		; Comparator 1 Control 0
CPOWER1		__z8bfield CMP1CTL0,6,2	;  Comparator Power/Speed Select
HYST1		__z8bfield CMP1CTL0,4,2	;  Hysteresis Level Select
INNSEL1		__z8bfield CMP1CTL0,2,2	;  Negative Input Signal Select
INPSEL1		__z8bfield CMP1CTL0,0,2	;  Positive Input Signal Select
CMP1CTL1	sfr	0f93h		; Comparator 1 Control 1
POLSEL1		__z8bit	CMP1CTL1,7	;  Polarity Select
PREFEN1		__z8bit	CMP1CTL1,6	;  Programmable Reference Enable
PREFSRC1	__z8bit	CMP1CTL1,5	;  Programmable Reference Source Selection
PREFLVL1	__z8bfield CMP1CTL1,0,5	;  Programmable Reference Level Selection

;----------------------------------------------------------------------------
; OpAmps

AMPACTL0	sfr	0f94h		; Op Amp_A Control 0
OUTCTLA		__z8bit	AMPACTL0,7	;  Output Control
INPSELA		__z8bfield AMPACTL0,0,2	;  Positive Input Signal Select
AMPACTL1	sfr	0f95h		; Op Amp_A Control 1
GAINA		__z8bfield AMPACTL1,4,4	;  Internal Voltage Gain Setting
OPOWERA		__z8bit	AMPACTL1,3	;  Op Amp Power/Speed Select
INNSELA		__z8bfield AMPACTL1,0,2	;  Negative Input Signal Select
AMPBCTL0	sfr	0f96h		; Op Amp_B Control 0
OUTCTLB		__z8bit	AMPBCTL0,7	;  Output Control
INPSELB		__z8bfield AMPBCTL0,0,2	;  Positive Input Signal Select
AMPBCTL1	sfr	0f97h		; Op Amp_B Control 1
IRESSELB	__z8bfield AMPBCTL1,4,2	;  Current Source Resistor Select
OPOWERB		__z8bit	AMPBCTL1,3	;  Op Amp Power/Speed Select
INNSELB		__z8bit	AMPBCTL1,1	;  Negative Input Signal Select
MODEB		__z8bit	AMPBCTL1,0	;  Mode

;----------------------------------------------------------------------------
; Analog/Digital Converter

ADCCTL0		sfr	0f70h		; ADC Control 0
START		__z8bfield ADCCTL0,6,2	;  ADC Start/Busy
ADC_DMACTL	__z8bit	ADCCTL0,5	;  DMA Request Control
ADC_IRQ		__z8bit	ADCCTL0,4	;  Interrupt Control
CONTCONV	__z8bit	ADCCTL0,3	;  Continuous Conversion Enable
AVE		__z8bit	ADCCTL0,2	;  Averaging Enable
AVESAMP		__z8bfield ADCCTL0,0,2	;  Averaging Samples
ADCCTL1		sfr	0f71h		; ADC Control 1
ADC_POWER	__z8bfield ADCCTL1,6,2	;  Power Control
SCAN		__z8bit	ADCCTL1,5	;  Channel Scanning Enable
INMODE		__z8bfield ADCCTL1,3,2	;  Input Mode
ADC_DFORMAT	__z8bit	ADCCTL1,2	;  Data Format
RESOLUT		__z8bit	ADCCTL1,1	;  ADC Conversion Resolution
ADCCTL2		sfr	0f72h		; ADC Control 2
ADC_REFSEL	__z8bfield ADCCTL2,6,2	;  ADC Positive Voltage Reference Select
REFLVL		__z8bfield ADCCTL2,4,2	;  VBIAS Level Select
ADC_PRESCALE	__z8bfield ADCCTL2,0,4	;  ADC Clock Prescale Divider
ADCINSH		sfr	0f73h		; ADC Input Select High
ANAINH		__z8bfield ADCINSH,0,7	;  Analog Input Selection High
ADCINSL		sfr	0f74h		; ADC Input Select Low
ANAINL		__z8bfield ADCINSL,0,8	;  Analog Input Selection Low
ADCOFF		sfr	0f75h		; ADC Offset Calibration
ADCD_H		sfr	0f76h		; ADC Data High Byte
ADCD_L		sfr	0f77h		; ADC Data Low Bits
OVF		__z8bit	ADCD_L,0	;  Overflow Status
ADCD		sfr	ADCD_H
ADCST		sfr	0f78h		; ADC Sample Time
ST		__z8bfield ADCST,4,4	;  Sample Time
SST		__z8bfield ADCST,0,4	;  Sample Settling Time
ADCUWINH	sfr	0f79h		; ADC Upper Window Threshold High
ADCUWINL	sfr	0f7ah		; ADC Upper Window Threshold Low
ADCUWIN		sfr	ADCUWINH
ADCLWINH	sfr	0f7bh		; ADC Lower Window Threshold High
ADCLWINL	sfr	0f7ch		; ADC Lower Window Threshold Low
ADCLWIN		sfr	ADCLWINH

;----------------------------------------------------------------------------
; Digital/Analog Converter

DACCTL		sfr	0f7dh		; DAC Control
DAC_POWER	__z8bfield DACCTL,6,2	;  Power Control
DAC_REFSEL	__z8bfield DACCTL,3,3	;  DAC Positive Voltage Reference Select
DAC_DFORMAT	__z8bit	DACCTL,2	;  Data Format
DACTRIG		__z8bit	DACCTL,1	;  DAC Triggering
JUSTIFY		__z8bit	DACCTL,0	;  Data Register Justification
DACD_H		sfr	0f7eh		; DAC Data High
DACD_L		sfr	0f7fh		; DAC Data Low
DACD		sfr	DACD_H

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

		endif			; __regf6482inc
