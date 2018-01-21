		ifndef	__regt26inc
__regt26inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT26.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny26                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; End Address EEPROM
RAMSTART	equ	0x60,data	; Start Address SRAM
RAMEND		equ     0xdf,data	; End Address SRAM
FLASHEND	label   2047		; End Address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,3		; Sleep Mode Select
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-on Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

PLLCSR		port	0x29		; PLL Control/Status Register
PLOCK		avrbit	PLLCSR,0	; PLL Lock Detector
PLLE		avrbit	PLLCSR,1	; PLL Enable
PCKE		avrbit	PLLCSR,2	; PCK Enable

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMWE		avrbit	EECR,2		; EEPROM Master Write Enable
EEWE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,6		; Pul-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum IO_PINS_vect		; Pin Change Interrupt
		nextenum TIMER1_CMPA_vect	; Timer/Counter 1 Compare Match 1A
		nextenum TIMER1_CMPB_vect	; Timer/Counter 1 Compare Match 1B
		nextenum TIMER1_OVF1_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF0_vect	; Timer/Counter 0 Overflow
		nextenum USI_STRT_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
PCIE1		avrbit	GIMSK,5		; Pin Change Interrupt Enable 1
PCIE0		avrbit	GIMSK,4		; Pin Change Interrupt Enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
PCIF		avrbit	GIFR,5		; Pin Change Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
PSR0		avrbit	TCCR0,3		; Timer/Counter 0 Prescaler Reset
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x30		; Timer/Counter 1 Control Register A
PWM1B		avrbit	TCCR1A,0	; Timer/Tounter 1 PWM Control
PWM1A		avrbit	TCCR1A,1
FOC1B		avrbit	TCCR1A,2	; Timer/Counter 1 Force Output Compare Match B
FOC1A		avrbit	TCCR1A,3	; Timer/Counter 1 Force Output Compare Match A
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2f		; Timer/Counter 1 Control Register B
CS10            avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11            avrbit	TCCR1B,1
CS12            avrbit	TCCR1B,2
CS13		avrbit	TCCR1B,3
PSR1		avrbit	TCCR1B,6	; Timer/Counter 1 Prescaler Reset
CTC1		avrbit	TCCR1B,7	; Timer/Counter 1 Clean on Match
TCNT1		port	0x2e		; Timer/Counter 1 Value
OCR1A		port	0x2d		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2c		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2b		; Timer/Counter 1 Output Compare Value C


TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable A

TIFR		port	0x38		; Timer Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

WDCE		avrbit	WDTCR,4		; Watchdog Change Enable

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX3		avrbit	ADMUX,3		; Multiplexer
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x06		; Control/Status Register A
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADFR		avrbit	ADCSR,5		; Free Running Select
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

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

		restore

		endif			; __regt26inc
