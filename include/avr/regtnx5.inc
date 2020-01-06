		ifndef	__regtnx5inc
__regtnx5inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX5.INC                                             *
;*                                                                          *
;*   Contains Common Bit & Register Definitions for ATtiny25/45/85          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		avrbit	MCUCR,2		; BOD Sleep
SM0		avrbit	MCUCR,3		; Sleep Mode Select
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
BODS		avrbit	MCUCR,7		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

PLLCSR		port	0x27		; PLL Control/Status Register
PLOCK		avrbit	PLLCSR,0	; PLL Lock Detector
PLLE		avrbit	PLLCSR,1	; PLL Enable
PCKE		avrbit	PLLCSR,2	; PCK Enable
LSM		avrbit	PLLCSR,7	; Low Speed Mode

PRR		port	0x20		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRUSI		avrbit	PRR,1		; Power Reduction USI
PRTIM0		avrbit	PRR,2		; Power Reduction Timer/Counter 0
PRTIM1		avrbit	PRR,3		; Power Reduction Timer/Counter 1

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register Low
EEARH		port	0x1f		; EEPROM Address Register High
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		avrbit	EEPM1,5		; EEPROM Program Mode
EEPM0		avrbit	EEPM1,4
EERIE		avrbit	EEPM1,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EEPM1,2		; EEPROM Master Write Enable
EEPE		avrbit	EEPM1,1		; EEPROM Write Enable
EERE		avrbit	EEPM1,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RSIG		avrbit	SPMCSR,5	; Read Device Signature Imprint Table
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x22		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,6		; Pull-Up Disable

PINB		port	0x16		; Port B @ 0x16 (IO) ff. (bit 0..5)

GPIOR0		port	0x11		; General Purpose I/O Register 0
GPIOR1		port	0x12		; General Purpose I/O Register 1
GPIOR2		port	0x13		; General Purpose I/O Register 2

DIDR		port	0x14		; Digital Input Disable Register
AIN0D		avrbit	DIDR,0		; AIN0 Digital Input Disable
AIN1D		avrbit	DIDR,1		; AIN1 Digital Input Disable
ADC1D		avrbit	DIDR,2		; ADC1 Digital Input Disable
ADC3D		avrbit	DIDR,3		; ADC3 Digital Input Disable
ADC2D		avrbit	DIDR,4		; ADC2 Digital Input Disable
ADC0D		avrbit	DIDR,5		; ADC0 Digital Input Disable

PCMSK		port	0x15		; Pin Change Interrupt Mask
PCINT0		avrbit	PCMSK,0		; Enable Pin Change Interrupt 0 
PCINT1		avrbit	PCMSK,1		; Enable Pin Change Interrupt 1
PCINT2		avrbit	PCMSK,2		; Enable Pin Change Interrupt 2
PCINT3		avrbit	PCMSK,3		; Enable Pin Change Interrupt 3
PCINT4		avrbit	PCMSK,4		; Enable Pin Change Interrupt 4
PCINT5		avrbit	PCMSK,5		; Enable Pin Change Interrupt 5

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum EE_RDY_vect		; EEPROM Ready
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

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
PCIE		avrbit	GIMSK,5		; Pin Change Interrupt Enable

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
PCIF		avrbit	GIFR,5		; Pin Change Interrupt Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2a		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x29		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

TCCR1		port	0x30		; Timer/Counter 1 Control Register
CS10            avrbit	TCCR1,0		; Timer/Counter 1 Clock Select
CS11            avrbit	TCCR1,1
CS12            avrbit	TCCR1,2
CS13		avrbit	TCCR1,3
COM1A0		avrbit	TCCR1,4		; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1,5
PWM1A		avrbit	TCCR1,6		; Timer/Counter 1 PWM Mode A
CTC1		avrbit	TCCR1,7		; Timer/Counter 1 Clear on Clear on Compare Match
TCNT1		port	0x2f		; Timer/Counter 1 Value
OCR1A		port	0x2e		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2b		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2d		; Timer/Counter 1 Output Compare Value C
DTPS1		port	0x23		; Timer/Counter Dead Time Prescaler Register
DTPS10		avrbit	DTPS1,0		; Dead Time Prescaler
DTPS11		avrbit	DTPS1,1
DT1A		port	0x25		; Timer/Counter1 Dead Time A
DT1AL0		avrbit	DT1A,0		; Dead Time Value for -OC1A Output
DT1AL1		avrbit	DT1A,1
DT1AL2		avrbit	DT1A,2
DT1AL3		avrbit	DT1A,3
DT1AH0		avrbit	DT1A,4		; Dead Time Value for OC1A Output
DT1AH1		avrbit	DT1A,5
DT1AH2		avrbit	DT1A,6
DT1AH3		avrbit	DT1A,7
DT1B		port	0x24		; Timer/Counter1 Dead Time B
DT1BL0		avrbit	DT1B,0		; Dead Time Value for -OC1B Output
DT1BL1		avrbit	DT1B,1
DT1BL2		avrbit	DT1B,2
DT1BL3		avrbit	DT1B,3
DT1BH0		avrbit	DT1B,4		; Dead Time Value for OC1B Output
DT1BH1		avrbit	DT1B,5
DT1BH2		avrbit	DT1B,6
DT1BH3		avrbit	DT1B,7

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE0B		avrbit	TIMSK,3		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		avrbit	TIMSK,4		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable A

TIFR		port	0x38		; Timer Interrupt Status Register

GTCCR		port	0x2c		; General Timer/Counter Control 1 Register
PSR0		avrbit	GTCCR,0		; Prescaler Reset Timer/Counter 0
PSR1		avrbit	GTCCR,1		; Prescaler Reset Timer/Counter 1
FOC1A		avrbit	GTCCR,2		; Force Output Compare Match 1 A
FOC1B		avrbit	GTCCR,3		; Force Output Compare Match 1 B
COM1B0		avrbit	GTCCR,4		; Timer/Counter 1 Output Compare IMode B
COM1B1		avrbit	GTCCR,5 
PWM1B		avrbit	GTCCR,6		; Pulse Width Modulator B Enable
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"

WDCE		avrbit	WDTCR,4		; Change Enable
WDP3		avrbit	WDTCR,5
WDIE		avrbit	WDTCR,6		; Enable Watchdog Interrupt
WDIF		avrbit	WDTCR,7		; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
REFS2		avrbit	ADMUX,4
MUX3		avrbit	ADMUX,3		; Multiplexer
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
ACME		avrbit	ADCSRB,6	; Analog Comparator Multiplexer Enable
IPR		avrbit	ADCSRB,5	; Input Polarity Reversal
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

USIBR		port	0x10		; USI Buffer Register

		restore			; re-enable listing

		endif			; __regtnx5inc
