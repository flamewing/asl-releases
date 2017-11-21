		ifndef	__reg4434inc
__reg4434inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG4434.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90S4434                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	255
RAMSTART	equ	0x60,data
RAMEND		equ	0x15f,data
FLASHEND	label	0xfff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		equ	4		; choose idle/power down mode
SE		equ	5		; enable sleep mode

MCUSR		port	0x34		; MCU General Status Register
EXTRF		equ	1		; External reset ocuured
PORF		equ	0		; Power-on reset occured


;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

		; bits in EECR
EEMWE		equ	2		; EEPROM Master Write Enable
EERIE		equ	3		; EEPROM interrupt enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER2_COMP		; timer/counter 2 compare match
		nextenum TIMER2_OVF		; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum UART_RX_vect		; UART Rx complete
		nextenum UART_UDRE_vect		; UART data register empty
		nextenum UART_TX_vect		; UART Tx complete
		nextenum ADC_vect		; A/D converter
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ     6		;  enable external Interrupt 0
INT1		equ	7		;  enable external Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		equ	6		;  external Interrupt 0 occured
INTF1		equ	7		;  external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; clock select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; timer/counter 0 value

TCCR1A		port	0x2f		; timer/counter 1 control register A
PWM10		equ	0		; mode of pulse width modulator
PWM11		equ	1
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; prescaler setting
CS11		equ	1
CS12		equ	2
CTC1		equ	3		; clear after equality?
ICES1		equ	6		; capture slope selection
ICNC1		equ	7		; capture noise filter
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB
OCR1AL		port	0x2a		; timer/counter 1 output compare value A LSB
OCR1AH		port	0x2b		; timer/counter 1 output compare value A MSB
OCR1BL		port	0x28		; timer/counter 1 output compare value B LSB
OCR1BH		port	0x29		; timer/counter 1 output compare value B MSB
ICR1L		port	0x27		; timer/counter 1 input capture value LSB
ICR1H		port	0x26		; timer/counter 1 input capture value MSB

TCCR2		port	0x25		; timer/counter 2 control register
CS20		equ	0		; timer/counter 2 clock select
CS21		equ	1
CS22		equ	2
CTC2		equ	3		; timer/counter 2 Clear Timer/Counter on Compare Match
COM20		equ	4		; timer/counter 2 Compare Output Mode
COM21		equ	5
PWM2		equ	6		; timer/counter 2 Pulse Width Modulator Enable
TCNT2		port	0x24		; timer/counter 2 value
OCR2		port	0x23		; timer/counter 2 output compare value

TIMSK		port	0x39		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	3		; timer/counter 1 output compare interrupt B enable
OCIE1A		equ	4		; timer/counter 1 output compare interrupt A enable
TICIE1		equ	5		; timer/counter 1 input capture interrupt enable
TOIE2		equ	6		; timer/counter 2 overflow interrupt enable
OCIE2		equ	7		; timer/counter 2 output compare interrupt enable

TIFR		port	0x38		; timer interrupt status register

ASSR		port	0x22		; Asynchronous Status Register
AS2		equ	3		; Asynchronous Timer/Counter2
TCN2UB		equ	2		; Timer/Counter2 Update Busy
OCR2UB		equ	1		; Output Compare Register2 Update Busy
TCR2UB		equ	0		; Timer/Counter Control Register2 Update Busy

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; UART

		include	"avr/uart90.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spi90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		equ	2		; Analog Channel Select Bits
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; control/status register
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADFR		equ	5		; free running select
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __reg4434inc
