		ifndef	__regm163inc
__regm163inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM163.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega163                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ     511
RAMSTART	equ	0x60,data
RAMEND		equ     0x45f,data
FLASHEND	label   0x3fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	4		; sleep mode select
SM1		equ	5
SE		equ	6		; sleep enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; watchdog reset occured
BORF		equ	2		; brown-out occured
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

OSCCAL		port	0x31		; oscillator calibration

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"

SPMCR		port	0x37		; Store Program Memory Control Register
ASB		equ	6		; application section busy
ASRE		equ	4		; application section rd enable
BLBSET		equ	3		; boot lock bit set
PGWRT		equ	2		; page write
PGERS		equ	1		; page erase
SPMEN		equ	0		; store program memory enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

SFIOR		port	0x30		; special function I/O Register
PUD		equ	2		; pull-up disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER2_COMP_vect	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI transfer complete
		nextenum UART_RX_vect		; UART Rx complete
		nextenum UART_UDRE_vect		; UART data register empty
		nextenum UART_TX_vect		; UART Tx complete
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator
		nextenum TWI_vect		; 2-wire serial interface

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; enable external interrupt 0
INT1		equ	7		; enable external interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		equ	6		; external Interrupt 0 occured
INTF1	        equ	7		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

		; bits in SFIOR
PSR10		equ	0		; prescaler reset T0/1
PSR2		equ	1		; ditto T2

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; timer/counter 0 value

TCCR1A		port	0x2f		; timer/counter 1 control register A
PWM10		equ	0		; timer/counter 1 PWM config
PWM11		equ	1
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
CTC1		equ	3		; timer/counter 1 clear on match
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
CS20		equ	0		; timer/counter 2 prescaler setting
CS21		equ	1
CS22		equ	2
CTC2		equ	3		; timer/counter 2 clear on match
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
PWM2		equ	6		; timer/counter 2 PWM config
FOC2		equ	7		; timer/counter 2 force output compare
TCNT2		port	0x24		; timer/counter 2 value
OCR2		port	0x23		; timer/counter 2 output compare value

TIMSK		port	0x39		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	3		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	4		; timer/counter 1 output compare interrupt enable A
TICIE1		equ	5		; timer/counter 1 input capture interrupt enable
TOIE2		equ	6		; timer/counter 2 overflow interrupt enable
OCIE2		equ	7		; timer/counter 2 output compare interrupt enable

TIFR		port	0x38		; timer interrupt status register

ASSR		port	0x22		; Asynchronous Status Register
TCR2UB		equ	0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		equ	1		; Output Compare Register 2
TCN2UB		equ	2		; Timer/Counter 2 Update Busy
AS2		equ	3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; UART

UDR		port	0x0c		; I/O Data Register

UCSRA		port	0x0b		; Control & Status Register A
MPCM		equ	0		; multi processor communication mode
U2X		equ	1		; double transmission speed
OR		equ	3		; Overrun
FE		equ	4		; Framing Error
UDRE		equ	5		; Data Register Empty
TXC		equ	6		; Transmit Complete
RXC		equ	7		; Receive Complete

UCSRB		port	0x0a		; Control & Status Register B
TXB8		equ     0		; transmit bit 8
RXB8		equ     1		; receive bit 8
CHR9		equ     2		; character size
TXEN		equ     3		; enable transmitter
RXEN		equ     4		; enable receiver
UDRIE		equ     5		; enable data register empty interrupt
TXCIE		equ     6		; enable transmit complete interrupt
RXCIE		equ     7		; enable receive complete interrupt

UCSRC		port	0x20		; Control & Status Register C
UCPOL		equ	0		; clock polarity
UCSZ0		equ	1		; character size
UCSZ1		equ	2
USBS		equ	3		; stop bit select
UPM0		equ	4		; parity mode : odd/even
UPM1		equ	5		; parity mode : enable/disable
UMSEL		equ	6		; USART mode select
URSEL		equ	7		; register select (1 for UCSRC)

UBRR		port	0x09		; baud rate register low
UBRRHI		port	0x20		; baud rate register high

;---------------------------------------------------------------------------- 
; SPI

		include	"avr/spim.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twim.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm8.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm163inc
