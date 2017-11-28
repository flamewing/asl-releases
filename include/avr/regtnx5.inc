		ifndef	__regtnx5inc
__regtnx5inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX5.INC                                             *
;*                                                                          *
;*   Contains common bit & register definitions for ATtiny25/45/85          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		equ	2		; BOD Sleep
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Enable
BODS		equ	7		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

PLLCSR		port	0x27		; PLL Control/Status Register
PLOCK		equ	0		; PLL Lock Detector
PLLE		equ	1		; PLL Enable
PCKE		equ	2		; PCK Enable
LSM		equ	7		; Low Speed Mode

PRR		port	0x20		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSI		equ	1		; Power Reduction USI
PRTIM0		equ	2		; Power Reduction Timer/Counter 0
PRTIM1		equ	3		; Power Reduction Timer/Counter 1

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
RSIG		equ	5		; Read Device Signature Imprint Table
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SPMEN		equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x22		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; Pull-Up Disable

PINB		port	0x16		; Port B @ 0x16 (IO) ff. (bit 0..5)

GPIOR0		port	0x11		; General Purpose I/O Register 0
GPIOR1		port	0x12		; General Purpose I/O Register 1
GPIOR2		port	0x13		; General Purpose I/O Register 2

DIDR		port	0x14		; Digital Input Disable Register
AIN0D		equ	0		; AIN0 Digital Input Disable
AIN1D		equ	1		; AIN1 Digital Input Disable
ADC1D		equ	2		; ADC1 Digital Input Disable
ADC3D		equ	3		; ADC3 Digital Input Disable
ADC2D		equ	4		; ADC2 Digital Input Disable
ADC0D		equ	5		; ADC0 Digital Input Disable

PCMSK		port	0x15		; Pin Change Interrupt Mask
PCINT0		equ	0		; Enable Pin Change Interrupt 0 
PCINT1		equ	1		; Enable Pin Change Interrupt 1
PCINT2		equ	2		; Enable Pin Change Interrupt 2
PCINT3		equ	3		; Enable Pin Change Interrupt 3
PCINT4		equ	4		; Enable Pin Change Interrupt 4
PCINT5		equ	5		; Enable Pin Change Interrupt 5

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; Enable External Interrupt 0
PCIE		equ	5		; Pin Change Interrupt Enable

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 Occured
PCIF		equ	5		; Pin Change Interrupt Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2a		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 0 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x29		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

TCCR1		port	0x30		; Timer/Counter 1 Control Register
CS10            equ     0               ; Timer/Counter 1 Clock Select
CS11            equ     1
CS12            equ     2
CS13		equ	3
COM1A0		equ	4		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	5
PWM1A		equ	6		; Timer/Counter 1 PWM Mode A
CTC1		equ	7		; Timer/Counter 1 Clear on Clear on Compare Match
TCNT1		port	0x2f		; Timer/Counter 1 Value
OCR1A		port	0x2e		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2b		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2d		; Timer/Counter 1 Output Compare Value C
DTPS1		port	0x23		; Timer/Counter Dead Time Prescaler Register
DTPS10		equ	0		; Dead Time Prescaler
DTPS11		equ	1
DT1A		port	0x25		; Timer/Counter1 Dead Time A
DT1AL0		equ	0		; Dead Time Value for -OC1A Output
DT1AL1		equ	1
DT1AL2		equ	2
DT1AL3		equ	3
DT1AH0		equ	4		; Dead Time Value for OC1A Output
DT1AH1		equ	5
DT1AH2		equ	6
DT1AH3		equ	7
DT1B		port	0x24		; Timer/Counter1 Dead Time B
DT1BL0		equ	0		; Dead Time Value for -OC1B Output
DT1BL1		equ	1
DT1BL2		equ	2
DT1BL3		equ	3
DT1BH0		equ	4		; Dead Time Value for OC1B Output
DT1BH1		equ	5
DT1BH2		equ	6
DT1BH3		equ	7

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		equ	2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE0B		equ	3		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		equ	4		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE1B		equ	5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	6		; Timer/Counter 1 Output Compare Interrupt Enable A

TIFR		port	0x38		; Timer Interrupt Status Register

GTCCR		port	0x2c		; General Timer/Counter Control 1 Register
PSR0		equ	0		; Prescaler Reset Timer/Counter 0
PSR1		equ	1		; Prescaler Reset Timer/Counter 1
FOC1A		equ	2		; Force Output Compare Match 1 A
FOC1B		equ	3		; Force Output Compare Match 1 B
COM1B0		equ	4		; Timer/Counter 1 Output Compare IMode B
COM1B1		equ	5 
PWM1B		equ	6		; Pulse Width Modulator B Enable
TSM		equ	7		; Timer/Counter Synchronization Mode

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

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		equ	7		; Reference Selection Bits
REFS0		equ	6
ADLAR		equ	5		; Left Adjust Right
REFS2		equ	4
MUX3		equ	3		; Multiplexer
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
ACME		equ	6		; Analog Comparator Multiplexer Enable
IPR		equ	5		; Input Polarity Reversal
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

USIBR		port	0x10		; USI Buffer Register

		restore			; re-enable listing

		endif			; __regtnx5inc
