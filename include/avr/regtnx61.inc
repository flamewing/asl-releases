		ifndef	__regtnx61inc
__regtnx61inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX61.INC                                            *
;*                                                                          *
;*Contains Common Bit & Register Definitions for ATtiny261(A)/461(A)/861(A) *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,3		; Sleep Mode Select
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
		if (MOMCPUNAME="ATTINY261A")||(MOMCPUNAME="ATTINY461A")||(MOMCPUNAME="ATTINY861A")
BODSE		 avrbit	MCUCR,2		; BOD Sleep Enable
BODS		 avrbit	MCUCR,7		; BOD Sleep
		endif

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-Out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x28		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

PRR		port	0x36		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRUSI		avrbit	PRR,1		; Power Reduction USI
PRTIM0		avrbit	PRR,2		; Power Reduction Timer/Counter 0
PRTIM1		avrbit	PRR,3		; Power Reduction Timer/Counter 1

PLLCSR		port	0x29		; PLL Control/Status Register
PLOCK		avrbit	PLLCSR,0	; PLL Lock Detector
PLLE		avrbit	PLLCSR,1	; PLL Enable
PCKE		avrbit	PLLCSR,2	; PCK Enable
LSM		avrbit	PLLCSR,7	; Low Speed Mode

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register Low
EEARH		port	0x1f		; EEPROM Address Register High
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
SIGRD		avrbit	SPMCSR,5	; Signature Row Read
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x20		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		avrbit	MCUCR,6		; Pull-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

GPIOR0		port	0x0a		; General Purpose I/O Register 0
GPIOR1		port	0x0b		; General Purpose I/O Register 1
GPIOR2		port	0x0c		; General Purpose I/O Register 2

DIDR0		port	0x01		; Digital Input Disable Register 0
ADC0D		avrbit	DIDR0,0		; ADC0 Digital Input Disable
ADC1D		avrbit	DIDR0,1		; ADC1 Digital Input Disable
ADC2D		avrbit	DIDR0,2		; ADC2 Digital Input Disable
AREFD		avrbit	DIDR0,3		; AREF Digital Input Disable
ADC3D		avrbit	DIDR0,4		; ADC3 Digital Input Disable
ADC4D		avrbit	DIDR0,5		; ADC4 Digital Input Disable
ADC5D		avrbit	DIDR0,6		; ADC5 Digital Input Disable
ADC6D		avrbit	DIDR0,7		; ADC6 Digital Input Disable

DIDR1		port	0x02		; Digital Input Disable Register 1
ADC7D		avrbit	DIDR1,4		; ADC7 Digital Input Disable
ADC8D		avrbit	DIDR1,5		; ADC8 Digital Input Disable
ADC9D		avrbit	DIDR1,6		; ADC9 Digital Input Disable
ADC10D		avrbit	DIDR1,7		; ADC10 Digital Input Disable

PCMSK0		port	0x23		; Pin Change Interrupt Mask 0
PCMSK1		port	0x22		; Pin Change Interrupt Mask 1

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum PCINT_vect		; Pin Change Interrupt
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_CAPT_vect	; Timer/Counter 0 Capture Event
		nextenum TIMER1_COMPD_vect	; Timer/Counter 1 Compare Match D
		nextenum FAULT_PROTECTION_vect	; Timer/Counter 1 Fault Protection

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
PCIE1		avrbit	GIMSK,5		; Pin Change Interrupt Enable 1
PCIE0		avrbit	GIMSK,4		; Pin Change Interrupt Enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF1		avrbit	GIFR,7		; External Interrupt 1 Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
PCIF		avrbit	GIFR,5		; Pin Change Interrupt Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x15		; Timer/Counter 0 Control Register A
CTC0		avrbit	TCCR0A,0	; Timer/Counter 0 Clear on Compare Match
ACIC0		avrbit	TCCR0A,3	; Analog Comparator Input Capture Enable
ICES0		avrbit	TCCR0A,4	; Timer/Counter 0 Edge Select
ICNC0		avrbit	TCCR0A,5	; Timer/Counter 0 Noise Canceler
ICEN0		avrbit	TCCR0A,6	; Timer/Counter 0 Input Capture Mode Enable
TCW0		avrbit	TCCR0A,7	; Timer/Counter 0 Timer/Counter0 Width
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
PSR0 		avrbit	TCCR0B,3	; Timer/Counter 0 Prescaler Reset
TSM		avrbit	TCCR0B,4	; Timer/Counter Synchonization
TCNT0L		port	0x32		; Timer/Counter 0 Value LSB
TCNT0H		port	0x14		; Timer/Counter 0 Value MSB
OCR0A		port	0x13		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x12		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x30		; Timer/Counter 1 Control Register A
PWM1B		avrbit	TCCR1A,0	; Timer/Counter 1 PWM Mode Select
PWM1A		avrbit	TCCR1A,1
FOC1B		avrbit	TCCR1A,2	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1A,3	; Timer/Counter 1 Force Output Compare A
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2f		; Timer/Counter 1 Control Register B
CS10            avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11            avrbit	TCCR1B,1
CS12            avrbit	TCCR1B,2
CS13		avrbit	TCCR1B,3
DTPS10		avrbit	TCCR1B,4	; Timer/Counter 1 Dead Time Prescaler
DTPS11		avrbit	TCCR1B,5
PSR1		avrbit	TCCR1B,6	; Timer/Counter 1 Prescaler Reset
PWM1X		avrbit	TCCR1B,7
TCCR1C		port	0x27		; Timer/Counter 1 Control Register C
PWM1D		avrbit	TCCR1C,0
FOC1D		avrbit	TCCR1C,1	; Timer/Counter 1 Force Output Compare D
COM1D0		avrbit	TCCR1C,2	; Timer/Counter 1 Output Compare Mode D
COM1D1		avrbit	TCCR1C,3
COM1B0S		avrbit	TCCR1C,4	; Timer/Counter 1 Output Compare Mode B Shadow
COM1B1S		avrbit	TCCR1C,5
COM1A0S		avrbit	TCCR1C,6	; Timer/Counter 1 Output Compare Mode A Shadow
COM1A1S		avrbit	TCCR1C,7
TCCR1D		port	0x26		; Timer/Counter 1 Control Register D
WGM10		avrbit	TCCR1D,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1D,1
FPF1		avrbit	TCCR1D,2	; Fault Protection Interrupt Flag
FPAC1		avrbit	TCCR1D,3	; Fault Protection Analog Comparator Enable
FPES1		avrbit	TCCR1D,4	; Fault Protection Edge Select
FPNC1		avrbit	TCCR1D,5	; Fault Protection Noise Canceler
FPEN1		avrbit	TCCR1D,6	; Fault Protection Mode Enable
FPIE1		avrbit	TCCR1D,7	; Fault Protection Interrupt Enable
TCNT1		port	0x2e		; Timer/Counter 1 Value
TC1H		port	0x25		; Timer/Counter 1 High Byte
OCR1A		port	0x2d		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2c		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2b		; Timer/Counter 1 Output Compare Value C
OCR1D		port	0x2a		; Timer/Counter 1 Output Compare Value D
DT1		port	0x24		; Timer/Counter 1 Dead Times
DT1L0		avrbit	DT1,0		; Timer/Counter 1 Dead Time Low
DT1L1		avrbit	DT1,1
DT1L2		avrbit	DT1,2
DT1L3		avrbit	DT1,3
DT1H0		avrbit	DT1,4		; Timer/Counter 1 Dead Time High
DT1H1		avrbit	DT1,5
DT1H2		avrbit	DT1,6
DT1H3		avrbit	DT1,7

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TICIE0		avrbit	TIMSK,0		; Timer/Counter 0 Input Capture Interrupt Enable
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE0B		avrbit	TIMSK,3		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		avrbit	TIMSK,4		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1D		avrbit	TIMSK,7		; Timer/Counter 1 Output Compare Interrupt Enable D

TIFR		port	0x38		; Timer Interrupt Status Register 0

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

WDCE		avrbit	WDTCR,4		; Change Enable
WDP3		avrbit	WDTCR,5
WDIE		avrbit	WDTCR,6		; Enable Watchdog Interrupt
WDIF		avrbit	WDTCR,7		; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x08		; Analog Comparator Control/Status Register A
ACIS0		avrbit	ACSRA,0		; Analog Comparator Interrupt-Mode
ACIS1		avrbit	ACSRA,1
ACME		avrbit	ACSRA,2		; Analog Comparator Multiplexer Enable		
ACIE		avrbit	ACSRA,3		; Analog Comparator Interrupt Enable
ACI		avrbit	ACSRA,4		; Analog Comparator Interrupt Flag
ACO		avrbit	ACSRA,5		; Analog Comparator Output
ACBG		avrbit	ACSRA,6		
ACD		avrbit	ACSRA,7		; Analog Comparator Disable

ACSRB		port	0x09		; Analog Comparator Control/Status Register B
ACM0		avrbit	ACSRB,0		; Analog Comparator Multiplexer
ACM1		avrbit	ACSRB,1
ACM2		avrbit	ACSRB,2
HLEV		avrbit	ACSRB,6		; Hysteresis Level
HSEL		avrbit	ACSRB,7		; Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX4		avrbit	ADMUX,4		; Multiplexer
MUX3		avrbit	ADMUX,3
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; ADC Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		port	0x03		; Control/Status Register B
BIN		avrbit	ADCSRB,7	; Bipolar Input Mode
GSEL		avrbit	ADCSRB,6	; Gain Select
REFS2		avrbit	ADCSRB,4
MUX5		avrbit	ADCSRB,3
ADTS2		avrbit	ADCSRB,2	; Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS0		avrbit	ADCSRB,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		avrbit	USISR,0		; Counter Value
USICNT1		avrbit	USISR,1
USICNT2		avrbit	USISR,2
USICNT3		avrbit	USISR,3
USIDC		avrbit	USISR,4		; Data Output Collision
USIPF		avrbit	USISR,5		; Stop Condition Flag
USIOIF		avrbit	USISR,6		; Counter Overflow Interrupt Flag
USISIF		avrbit	USISR,7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		avrbit	USICR,0		; Toggle Clock Port Pin
USICLK		avrbit	USICR,1		; Clock Strobe
USICS0		avrbit	USICR,2		; Clock Source Select
USICS1		avrbit	USICR,3
USIWM0		avrbit	USICR,4		; Wire Mode
USIWM1		avrbit	USICR,5		
USIOIE		avrbit	USICR,6		; Counter Overflow Interrupt Enable
USISIE		avrbit	USICR,7		; Start Condition Interrupt Enable

USIPP		port	0x11		; USI Pin Position
USIPOS		avrbit	USIPP,0		; USI Pin Position

USIBR		port	0x10		; USI Buffer Register

		restore			; re-enable listing

		endif			; __regtnx61inc
