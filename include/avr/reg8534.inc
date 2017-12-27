		ifndef	__reg8534inc
__reg8534inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG8534.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90C8534                      *
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
SM		equ	4		; choose idle/power down mode
SE		equ	5		; enable sleep mode

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

		; bits in EECR
EEMWE		equ	2		; EEPROM Master Write Enable
EERIE		equ	3		; EEPROM interrupt enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff. (bits 0..6)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum ADC_vect		; A/D converter
		nextenum EE_RDY_vect		; EEPROM ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC0		equ	0		; external interrupt 0 sense control
ISC1		equ	2		; external interrupt 1 sense control

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ     6		;  enable external Interrupt 0
INT1		equ	7		;  enable external Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		equ	6		;  external Interrupt 0 occured
INTF1		equ	7		;  external Interrupt 1 occured

GIPR		port	0x10		; General Interrupt Pin Register
IPIN0		equ	2		; External Interrupt Pin 0
IPIN1		equ	3		; External Interrupt Pin 1

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		equ	0		; Clock Select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; timer/counter 0 value

TCCR1		port	0x2e		; timer/counter 1 control register
CS10		equ	0		; prescaler setting
CS11		equ	1
CS12		equ	2
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB

TIMSK		port	0x39		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		equ	2		; Analog Channel Select Bits
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; control/status register
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADRF		equ	5		; free running select
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __reg8534inc
