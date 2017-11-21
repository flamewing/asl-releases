		ifndef	__regm16inc
__regm16inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM16.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATmega16                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x60,data
RAMEND		equ     0x45f,data
FLASHEND	label   0x3fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SE		equ	7		; sleep enable
SM2		equ	6		; sleep mode select
SM1		equ	5
SM0		equ	4

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		equ	3		; watchdog reset occured
BORF		equ	2		; brown-out occured
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

OSCCAL		port	0x31		; oscillator calibration

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"
		include	"avr/spm.inc"

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCSR
JTRF		equ	4		; JTAG reset occured
JTD		equ	7		; JTAG disable

OCDR		port	0x31		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

SFIOR		port	0x30		; special function I/O Register
PUD		equ	2		; pullup disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER2_COMP_vect 	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI transfer complete
		nextenum USART_RXC_vect		; UART Rx complete
		nextenum USART_UDRE_vect	; UART data register empty
		nextenum USART_TXC_vect		; UART Tx complete
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator
		nextenum TWI_vect		; two-wire serial interface
		nextenum INT2_vect		; external interrupt 2
		nextenum TIMER0_COMP_vect	; timer 0 compare match
		nextenum SPM_RDY_vect		; store program memory ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

		; bits in MCUCSR
ISC2		equ	6		; external interrupt 2 sense control

GICR		port	0x3b		; General Interrupt Control Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select
INT2		equ	5		; enable external interrupt 2
INT0		equ	6		; enable external interrupt 0
INT1		equ	7		; enable external interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags:
INTF2		equ	5		; external Interrupt 2 occured
INTF0		equ	6		; external Interrupt 0 occured
INTF1	        equ	7		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

		; bits in SFIOR
PSR10		equ	0		; prescaler reset T0/T1
PSR2		equ	1		; ditto T2

ASSR		port	0x22		; Asynchronous Status Register
AS2		equ	3		; Asynchronous Timer/Counter2
TCN2UB		equ	2		; Timer/Counter2 Update Busy
OCR2UB		equ	1		; Output Compare Register2 Update Busy
TCR2UB		equ	0		; Timer/Counter Control Register2 Update Busy

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM01		equ	3
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
WGM00		equ	6		; timer/counter 0 waveform generation mode
FOC0		equ	7		; timer/counter 0 force output compare match
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x3c		; timer/counter 0 output compare value

TCCR1A		port	0x2f		; timer/counter 1 control register A
WGM10		equ	0		; timer/counter 1 waveform generation mode
WGM11		equ	1
FOC1B		equ	2		; timer/counter 1 force output compare B
FOC1A		equ	3		; timer/counter 1 force output compare A
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
WGM12		equ	3		; timer/counter 1 waveform generation mode
WGM13		equ	4
ICES1		equ	6		; timer/counter 1 capture slope selection
ICNC1		equ	7		; timer/counter 1 capture noise filter
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB
OCR1AL		port	0x2a		; timer/counter 1 output compare value A LSB
OCR1AH		port	0x2b		; timer/counter 1 output compare value A MSB
OCR1BL		port	0x28		; timer/counter 1 output compare value B LSB
OCR1BH		port	0x29		; timer/counter 1 output compare value B MSB
ICR1L		port	0x26		; timer/counter 1 input capture value LSB
ICR1H		port	0x27		; timer/counter 1 input capture value MSB

TCCR2		port	0x25		; timer/counter 2 control register
CS20		equ	0		; timer/counter 2  prescaler setting
CS21		equ	1
CS22		equ	2
WGM21		equ	3
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
WGM20		equ	6		; timer/counter 2 waveform generation mode
FOC2		equ	7		; timer/counter 2 force output compare
TCNT2		port	0x24		; timer/counter 2 value
OCR2		port	0x23		; timer/counter 2 output compare value

TIMSK		port	0x39		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0		equ	1		; timer/counter 0 output compare interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	3		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	4		; timer/counter 1 output compare interrupt enable A
TICIE1		equ	5		; timer/counter 1 input capture interrupt enable
TOIE2		equ	6		; timer/counter 2 overflow interrupt enable
OCIE2		equ	7		; timer/counter 2 output compare interrupt enable

TIFR		port	0x38		; timer interrupt status register

ASSR		port	0x30		; Asynchronous Status Register
TCR2UB		equ	0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		equ	1		; Output Compare Register 2
TCN2UB		equ	2		; Timer/Counter 2 Update Busy
AS2		equ	3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; USART

		include	"avr/usartm.inc"

;----------------------------------------------------------------------------
; SPI

		include "avr/spim.inc"

;----------------------------------------------------------------------------
; TWI

		include "avr/twim.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm16.inc"

		; bits in SFIOR
ADTS0		equ	5		; auto trigger source
ADTS1		equ	6
ADTS2		equ	7

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm16inc
