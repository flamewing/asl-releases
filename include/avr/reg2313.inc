		ifndef	__reg2313inc
__reg2313inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG2313.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90S2313                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	127
RAMSTART	equ	0x60,data
RAMEND		equ	0xdf,data
FLASHEND	label	0x7ff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		equ	4		; choose idle/powerdown mode
SE		equ	5		; enable sleep mode

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

		; bits in EECR
EEMWE		equ	 2		; EEPROM Master Write Enable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMP_vect	; timer/counter 1 compare match
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum UART_RX_vect		; UART Rx complete
		nextenum UART_UDRE_vect		; UART data register empty
		nextenum UART_TX_vect		; UART Tx complete
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
COM1A0		equ	6		; timer/counter 1 compare mode
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
ICR1L		port	0x24		; timer/counter 1 input capture value LSB
ICR1H		port	0x25		; timer/counter 1 input capture value MSB

TIMSK		port	0x39		; timer mask register
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
TICIE1		equ	3		; timer/counter 1 input capture interrupt enable
OCIE1A		equ	6		; timer/counter 1 output compare interrupt enable
TOIE1		equ	7		; timer/counter 1 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; UART

		include "avr/uart90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		restore

		endif			; __reg2313inc
