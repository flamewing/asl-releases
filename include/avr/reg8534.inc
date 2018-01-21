		ifndef	__reg8534inc
__reg8534inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG8534.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90C8534                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	511
RAMSTART	equ	0x60,data
RAMEND		equ	0x15f,data
FLASHEND	label	0x1fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Power-Down Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

EEMWE		avrbit	EECR,2		; EEPROM Master Write Enable
EERIE		avrbit	EECR,3		; EEPROM Interrupt Enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
__PORTA_BITS	equ	0x7f		; (bits 0..6)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ADC_vect		; A/D Converter
		nextenum EE_RDY_vect		; EEPROM Ready

;----------------------------------------------------------------------------
; External Interrupts

ISC0		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC1		avrbit	MCUCR,2		; External Interrupt 1 Sense Control

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1		avrbit	GIFR,7		; External Interrupt 1 Occured

GIPR		port	0x10		; General Interrupt Pin Register
IPIN0		avrbit	GIPR,2		; External Interrupt Pin 0
IPIN1		avrbit	GIPR,3		; External Interrupt Pin 1

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1		port	0x2e		; Timer/Counter 1 Control Register
CS10		avrbit	TCCR1,0		; Prescaler Setting
CS11		avrbit	TCCR1,1
CS12		avrbit	TCCR1,2
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		avrbit	ADMUX,2		; Analog Channel Select Bits
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x06		; Control/Status Register
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADRF		avrbit	ADCSR,5		; Free Running Select
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __reg8534inc
