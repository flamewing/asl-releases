		ifndef	__regm103inc
__regm103inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM103.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega103                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x60,data
RAMEND		equ	0x105f,data
FLASHEND	label	0x1ffff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	3
SM1		equ	4
SE		equ	5		; sleep enable
SRW		equ	6		; wait state select
SRE		equ	7		; enable external SRAM

MCUSR		port	0x34		; MCU Status Register
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

XDIV		port	0x3c		; XTAL Divide Control Register
XDIVEN		equ	7		; XTAL Divide Enable
XDIV6		equ	6		; XTAL Divide Select
XDIV5		equ	5
XDIV4		equ	4
XDIV3		equ	3
XDIV2		equ	2
XDIV1		equ	1
XDIV0		equ	0

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/eem.inc"

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE            port	0x01		; Port E @ 0x01 (IO) ff.
PINF            port	0x00		; Port F @ 0x01 (IO) ff.,
PINF_inponly	equ	  1		; input only

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum INT3_vect		; external interrupt request 3
		nextenum INT4_vect		; external interrupt request 4
		nextenum INT5_vect		; external interrupt request 5
		nextenum INT6_vect		; external interrupt request 6
		nextenum INT7_vect		; external interrupt request 7
		nextenum TIMER2_COMP_vect	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum UART_RX_vect		; UART Rx complete
		nextenum UART_UDRE_vect		; UART data register empty
		nextenum UART_TX_vect		; UART Tx complete
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum ANALOG_COMP_vect	; analog comparator

;----------------------------------------------------------------------------
; External Interrupts

EICR		port	0x3a		; External Interrupt Control Register
ISC40		equ	0		; external interrupt 4 sense control
ISC41		equ	1
ISC50		equ	2		; external interrupt 5 sense control
ISC51		equ	3
ISC60		equ	4		; external interrupt 6 sense control
ISC61		equ	5
ISC70		equ	6		; external interrupt 7 sense control
ISC71		equ	7

EIMSK		port	0x39		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1
INT2		equ	2		; enable external interrupt 2
INT3		equ	3		; enable external interrupt 3
INT4		equ	4		; enable external interrupt 4
INT5		equ	5		; enable external interrupt 5
INT6		equ	6		; enable external interrupt 6
INT7		equ	7		; enable external interrupt 7

EIFR		port	0x38		; External Interrupt Flags Register
INTF4	        equ	4		; external Interrupt 4 occured
INTF5		equ	5		; external Interrupt 5 occured
INTF6		equ	6		; external Interrupt 6 occured
INTF7	        equ	7		; external Interrupt 7 occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
CTC0		equ	3		; timer/counter 0 clear on compare match
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
PWM0		equ	6		; timer/counter 0 PWM mode
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x31

TCCR1A		port	0x2f		; timer/counter 1 control register A
PWM10		equ	0		; timer/counter 1 PWM mode
PWM11		equ	1
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
CTC1		equ	3		; timer/counter 1 clear on compare match
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
CTC2		equ	3		; timer/counter 2 clear on compare match
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
PWM2		equ	6		; timer/counter 2 PWM mode
TCNT2		port	0x24		; timer/counter 2 value
OCR2		port	0x23		; timer/counter 2 output compare value

TIMSK		port	0x37		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0		equ	1		; timer/counter 0 output compare interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	3		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	4		; timer/counter 1 output compare interrupt enable A
TICIE1		equ	5		; timer/counter 1 input capture interrupt enable
TOIE2		equ	6		; timer/counter 2 overflow interrupt enable
OCIE2		equ	7		; timer/counter 2 output compare interrupt enable

TIFR		port	0x36		; timer interrupt status register

ASSR		port	0x30		; Asynchronous Status Register
TCR0UB		equ	0		; Timer/Counter Control Register 0 Update Busy
OCR0UB		equ	1		; Output Compare Register 0
TCN0UB		equ	2		; Timer/Counter 0 Update Busy
AS0		equ	3		; Asynchronous Timer/Counter 0

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
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; control/status register
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		restore			; re-enable listing

		endif			; __regm103inc
