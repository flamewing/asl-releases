		ifndef	__regt13inc
__regt13inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT13.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny13                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; End Address EEPROM
RAMSTART	equ	0x60,data	; Start Address SRAM
RAMEND		equ     0x9f,data	; End Address SRAM
FLASHEND	label   1023		; End Address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,3		; Sleep Mode Select
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-Out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescale Register
CLKPS0		avrbit	CLKPR,0		; Clock Prescaler Select Bits
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x2e		; debugWire Data Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Programming Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Program Enable
EEPE		avrbit	EECR,1		; EEPROM Program Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control and Status Register
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SELFPRGEN	avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,6		; Pull-Up Disable

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
__PORTB_BITS	equ	0x3f		; (6 bits)

PCMSK		port	0x15		; Pin Change Interrupt Mask
PCINT0		avrbit	PCMSK,0		; Enable Pin Change Interrupt 0
PCINT1		avrbit	PCMSK,1		; Enable Pin Change Interrupt 1
PCINT2		avrbit	PCMSK,2		; Enable Pin Change Interrupt 2
PCINT3		avrbit	PCMSK,3		; Enable Pin Change Interrupt 3
PCINT4		avrbit	PCMSK,4		; Enable Pin Change Interrupt 4
PCINT5		avrbit	PCMSK,5		; Enable Pin Change Interrupt 5

DIDR0		port	0x14		; Digital Input Disable Register 0
AIN0D		avrbit	DIDR0,0		; Analog Comparator Digital Input Disable
AIN1D		avrbit	DIDR0,1
ADC1D		avrbit	DIDR0,2		; ADC Digital Input Disable
ADC3D		avrbit	DIDR0,3
ADC2D		avrbit	DIDR0,4
ADC0D		avrbit	DIDR0,5

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum TIM0_OVF_vect		; Timer/Counter 0 Overflow
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum TIM0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIM0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum WDT_vect		; watchdog Timer
		nextenum ADC_vect		; ADC conversion Complete

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
PCIE		avrbit	GIMSK,5		; pin change Interrupt Enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
PCIF		avrbit	GIFR,5		; Pin Change Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2f		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x29		; Timer/Counter 0 Output Compare Value B

TIMSK0		port	0x39		; Timer Interrupt Mask Register 0
TOIE0		avrbit	TIMSK0,1	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK0,3	; Timer/Counter 0 Output Compare Interrupt Enable B

TIFR0		port	0x38		; Timer Interrupt Flag Register 0

GTCCR		port	0x28		; Global Timer/Counter Control Register
PSR10		avrbit	GTCCR,0		; Prescaler Reset Timer 0/1
TSM		avrbit	GTCCR,7		; Timer Synchronization

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

WDCE		avrbit	WDTCR,4		; Watchdog Change Enable
WDP3		avrbit	WDTCR,5		; 
WDTIE		avrbit	WDTCR,6		; Watchdog Timer Interrupt Enable
WDTIF		avrbit	WDTCR,7		; Watchdog Timer Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS0		avrbit	ADMUX,6		; Reference Selection Bits
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; Free Running Select
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		port	0x03		; Control/Status Register B
ADTS0		avrbit	ADCSRB,0	; ADC Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS2		avrbit	ADCSRB,2
ACME		avrbit	ADCSRB,6	; Analog Comparator multiplexer Enable

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __regt13inc
