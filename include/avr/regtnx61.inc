		ifndef	__regtnx61inc
__regtnx61inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX4.INC                                             *
;*                                                                          *
;*Contains common bit & register definitions for ATtiny261(A)/461(A)/861(A) *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Enable
		if (MOMCPUNAME="ATTINY261A")||(MOMCPUNAME="ATTINY461A")||(MOMCPUNAME="ATTINY861A")
BODSE		 equ	2		; BOD Sleep Enable
BODS		 equ	7		; BOD Sleep
		endif

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x28		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

PRR		port	0x36		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSI		equ	1		; Power Reduction USI
PRTIM0		equ	2		; Power Reduction Timer/Counter 0
PRTIM1		equ	3		; Power Reduction Timer/Counter 1

PLLCSR		port	0x29		; PLL Control/Status Register
PLOCK		equ	0		; PLL Lock Detector
PLLE		equ	1		; PLL Enable
PCKE		equ	2		; PCK Enable
LSM		equ	7		; Low Speed Mode

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register Low
EEARH		port	0x1f		; EEPROM Address Register High
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
SIGRD		equ	5		; Signature Row Read
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SPMEN		equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x20		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; Pull-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

GPIOR0		port	0x0a		; General Purpose I/O Register 0
GPIOR1		port	0x0b		; General Purpose I/O Register 1
GPIOR2		port	0x0c		; General Purpose I/O Register 2

DIDR0		port	0x01		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Disable
ADC1D		equ	1		; ADC1 Digital Input Disable
ADC2D		equ	2		; ADC2 Digital Input Disable
AREFD		equ	3		; AREF Digital Input Disable
ADC3D		equ	3		; ADC3 Digital Input Disable
ADC4D		equ	4		; ADC4 Digital Input Disable
ADC5D		equ	5		; ADC5 Digital Input Disable
ADC6D		equ	6		; ADC6 Digital Input Disable

DIDR1		port	0x02		; Digital Input Disable Register 1
ADC7D		equ	4		; ADC7 Digital Input Disable
ADC8D		equ	5		; ADC8 Digital Input Disable
ADC9D		equ	6		; ADC9 Digital Input Disable
ADC10D		equ	7		; ADC10 Digital Input Disable

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
		nextenum EE_RDY_vect		; EEPROM ready
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

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT1		equ	7		; Enable External Interrupt 1
INT0		equ	6		; Enable External Interrupt 0
PCIE1		equ	5		; Pin Change Interrupt Enable 1
PCIE0		equ	4		; Pin Change Interrupt Enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF1		equ	7		; External Interrupt 1 Occured
INTF0		equ	6		; External Interrupt 0 Occured
PCIF		equ	5		; Pin Change Interrupt Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x15		; Timer/Counter 0 Control Register A
CTC0		equ	0		; Timer/Counter 0 Clear on Compare Match
ACIC0		equ	3		; Analog Comparator Input Capture Enable
ICES0		equ	4		; Timer/Counter 0 Edge Select
ICNC0		equ	5		; Timer/Counter 0 Noise Canceler
ICEN0		equ	6		; Timer/Counter 0 Input Capture Mode Enable
TCW0		equ	7		; Timer/Counter 0 Timer/Counter0 Width
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
PSR0 		equ	3		; Timer/Counter 0 Prescaler Reset
TSM		equ	4		; Timer/Counter Synchonization
TCNT0L		port	0x32		; Timer/Counter 0 Value LSB
TCNT0H		port	0x14		; Timer/Counter 0 Value MSB
OCR0A		port	0x13		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x12		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x30		; Timer/Counter 1 Control Register A
PWM1B		equ	0		; Timer/Counter 1 PWM Mode Select
PWM1A		equ	1
FOC1B		equ	2		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	3		; Timer/Counter 1 Force Output Compare A
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	7
TCCR1B		port	0x2f		; Timer/Counter 1 Control Register B
CS10            equ     0               ; Timer/Counter 1 Clock Select
CS11            equ     1
CS12            equ     2
CS13		equ	3
DTPS10		equ	4		; Timer/Counter 1 Dead Time Prescaler
DTPS11		equ	5
PSR1		equ	6		; Timer/Counter 1 Prescaler Reset
PWM1X		equ	7
TCCR1C		port	0x27		; Timer/Counter 1 Control Register C
PWM1D		equ	0
FOC1D		equ	1		; Timer/Counter 1 Force Output Compare D
COM1D0		equ	2		; Timer/Counter 1 Output Compare Mode D
COM1D1		equ	3
COM1B0S		equ	4		; Timer/Counter 1 Output Compare Mode B Shadow
COM1B1S		equ	5
COM1A0S		equ	6		; Timer/Counter 1 Output Compare Mode A Shadow
COM1A1S		equ	7
TCCR1D		port	0x26		; Timer/Counter 1 Control Register D
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
FPF1		equ	2		; Fault Protection Interrupt Flag
FPAC1		equ	3		; Fault Protection Analog Comparator Enable
FPES1		equ	4		; Fault Protection Edge Select
FPNC1		equ	5		; Fault Protection Noise Canceler
FPEN1		equ	6		; Fault Protection Mode Enable
FPIE1		equ	7		; Fault Protection Interrupt Enable
TCNT1		port	0x2e		; Timer/Counter 1 Value
TC1H		port	0x25		; Timer/Counter 1 High Byte
OCR1A		port	0x2d		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2c		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2b		; Timer/Counter 1 Output Compare Value C
OCR1D		port	0x2a		; Timer/Counter 1 Output Compare Value D
DT1		port	0x24		; Timer/Counter 1 Dead Times
DT1L0		equ	0		; Timer/Counter 1 Dead Time Low
DT1L1		equ	1
DT1L2		equ	2
DT1L3		equ	3
DT1H0		equ	4		; Timer/Counter 1 Dead Time High
DT1H1		equ	5
DT1H2		equ	6
DT1H3		equ	7

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TICIE0		equ	0		; Timer/Counter 0 Input Capture Interrupt Enable
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		equ	2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE0B		equ	3		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		equ	4		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE1B		equ	5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	6		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1D		equ	7		; Timer/Counter 1 Output Compare Interrupt Enable D

TIFR		port	0x38		; Timer Interrupt Status Register 0

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
		; bits in WDTCR
WDCE		equ	4		; Change Enable
WDP3		equ	5
WDIE		equ	6		; Enable Watchdog Interrupt
WDIF		equ	7		; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x08		; Analog Comparator Control/Status Register A
ACIS0		equ	0		; Analog Comparator Interrupt-Mode
ACIS1		equ	1
ACME		equ	2		; Analog Comparator Multiplexer Enable		
ACIE		equ	3		; Analog Comparator Interrupt Enable
ACI		equ	4		; Analog Comparator Interrupt Flag
ACO		equ	5		; Analog Comparator Output
ACBG		equ	6		
ACD		equ	7		; Analog Comparator Disable

ACSRB		port	0x09		; Analog Comparator Control/Status Register B
ACM0		equ	0		; Analog Comparator Multiplexer
ACM1		equ	1
ACM2		equ	2
HLEV		equ	6		; Hysteresis Level
HSEL		equ	7		; Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		equ	7		; Reference Selection Bits
REFS0		equ	6
ADLAR		equ	5		; Left Adjust Right
MUX4		equ	4		; Multiplexer
MUX3		equ	3
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		equ	7		; Enable ADC
ADSC		equ	6		; Start Conversion
ADATE		equ	5		; ADC Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x03		; Control/Status Register B
BIN		equ	7		; Bipolar Input Mode
GSEL		equ	6		; Gain Select
REFS2		equ	4
MUX5		equ	3
ADTS2		equ	2		; Auto Trigger Source
ADTS1		equ	1
ADTS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

USIPP		port	0x11		; USI Pin Position
USIPOS		equ	0		; USI Pin Position

USIBR		port	0x10		; USI Buffer Register

		restore			; re-enable listing

		endif			; __regtnx61inc
