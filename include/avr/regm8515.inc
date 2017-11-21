		ifndef	__regm8515inc
__regm8515inc	equ	1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM8515.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATmega8515                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	511
RAMSTART	equ	0x60,data
RAMEND		equ	0x25f,data
FLASHEND	label	0x1fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM1		equ	4
SE		equ	5		; sleep enable
SRW10		equ	6		; wait state select
SRE		equ	7		; enable external SRAM

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		equ	3		; watchdog reset occured
BORF		equ	2		; brown-out occured
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

EMCUCR		port	0x36		; Extended MCU Control Register
SRW11		equ	1		; wait state select
SRW00		equ	2
SRW01		equ	3
SRL0		equ	4		; wait state sector limit
SRL1		equ	5
SRL2		equ	6
SM0		equ	7		; sleep mode select

OSCCAL		port	0x04		; oscillator calibration

;----------------------------------------------------------------------------
; EEPROM/ProgramMemory Access

		include	"avr/eem.inc"
		include	"avr/spm.inc"

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE		port	0x05		; Port E @ 0x05 (IO) ff., bits 0..2

SFIOR		port	0x30		; special function I/O Register
PUD		equ	2		; pullup disable
XMM0		equ	3		; External Memory High Mask
XMM1		equ	4
XMM2		equ	5
XMBK		equ	6		; External Memory Bus Keeper Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART_RX_vect		; UART Rx complete
		nextenum USART_UDRE_vect	; UART data register empty
		nextenum USART_TX_vect		; UART Tx complete
		nextenum ANA_COMP_vect		; analog comparator
		nextenum INT2_vect		; external interrupt request 2
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum EE_RDY_vect		; two-wire serial interface
		nextenum SPM_RDY_vect		; store program memory ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

		; bits in EMCUCR
ISC2		equ	0		; external interrupt 2 sense control

GICR		port	0x3b		; General Interrupt Mask Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select
INT2		equ     5		; enable external Interrupt 2
INT0		equ     6		; enable external Interrupt 0
INT1		equ	7		; enable external Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF2		equ	5		; external Interrupt 2 occured
INTF0		equ	6		; external Interrupt 0 occured
INTF1		equ	7		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

		; bits in SFIOR
PSR10		equ	0		; prescaler reset T0/T1

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM01		equ	3		; timer/counter 0 waveform generation mode	
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
WGM00		equ	6
FOC0		equ	7		; timer/counter 0 force output compare
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x31		; timer/counter 0 output compare value

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
ICR1L		port	0x24		; timer/counter 1 input capture value LSB
ICR1H		port	0x25		; timer/counter 1 input capture value MSB

TIMSK		port	0x39		; timer interrupt mask register
OCIE0		equ	0		; timer/counter 0 output compare interrupt enable
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
TICIE1		equ	3		; timer/counter 1 input capture interrupt enable
OCIE1B		equ	5		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	6		; timer/counter 1 output compare interrupt enable A
TOIE1		equ	7		; timer/counter 1 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDCE		equ	4		; change enable

;----------------------------------------------------------------------------
; USART

		include	"avr/usartm.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include	"avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm8515inc
