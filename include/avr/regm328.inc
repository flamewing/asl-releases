		ifndef	__regm328inc
__regm328inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM328.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega328                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	1023
RAMSTART	equ	0x100,data
RAMEND		equ	0x8ff,data
FLASHEND	label	0x7fff

;----------------------------------------------------------------------------
; Chip Control

MCUCR		port	0x35		; MCU Control Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select
BODSE		equ	5		; BOD Sleep Enable
BODS		equ	6		; BOD Sleep

MCUSR		port	0x34		; MCU Status Register
PORF		equ	0		; power-on reset occured
EXTRF		equ	1		; external reset occured
BORF		equ	2		; brown out reset occured
WDRF		equ	3		; watchdog reset occured

SMCR		port	0x33		; sleep mode control register
SE		equ	0		; sleep mode enable
SM0		equ	1		; sleep mode select
SM1		equ	2
SM2		equ	3

PRR		sfr	0x64		; Power Reduction Register
PRADC		equ	0		; Power Reduction ADC
PRUSART0	equ	1		; Power Reduction USART0
PRSPI0		equ	2		; Power Reduction Serial Peripheral Interface 0
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRTIM0		equ	5		; Power Reduction Timer/Counter 0
PRTIM2		equ	6		; Power Reduction Timer/Counter 2
PRTWI0		equ	7		; Power Reduction Two Wire Interface 0

OSCCAL		sfr	0x66		; oscillator calibration

CLKPR		sfr	0x61		; clock prescale register
CLKPS0		equ	0		; clock prescaler select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem2.inc"
		include	"avr/spmcsr37.inc"

		; additional bits in EECR
EEPM0		equ	4		; EEPROM Programming Mode
EEPM1		equ	5

		; additional bits in SPMCSR
SIGRD		equ	5		; signature row read

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	4		; pull up disable

PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1 (8..14)
PCMSK2		sfr	0x6d		; Pin Change Mask Register 2
PCICR		sfr	0x68		; Pin Change Interrupt Control Register
PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

GPIOR0		port	0x1e		; general purpose registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum PCINT2_vect		; pin change interrupt request 2
		nextenum WDT_vect		; watchdog time-out interrupt
		nextenum TIMER2_COMPA_vect	; timer/counter 2 compare match A
		nextenum TIMER2_COMPB_vect	; timer/counter 2 compare match B
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMPA_vect	; timer/counter 0 compare match A
		nextenum TIMER0_COMPB_vect	; timer/counter 0 compare match B
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART_RX_vect		; USART Rx complete
		nextenum USART_UDRE_vect	; USART data register empty
		nextenum USART_TX_vect		; USART Tx complete
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum ANALOG_COMP_vect	; analog comparator
		nextenum TWI_vect		; two-wire serial interface
		nextenum SPM_READY_vect		; store program memory ready

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; external Interrupt 0 occured
INTF1		equ	1		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		equ	0		; Prescaler Reset
PSRASY		equ	1		; Prescaler Reset Timer/Counter2
TSM		equ	7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; timer/counter 0 control register A
WGM00		equ	0		; timer/counter 0 waveform generation mode
WGM01		equ	1
COM0B0		equ	4		; timer/counter 0 compare B mode
COM0B1		equ	5
COM0A0		equ	6		; timer/counter 0 compare A mode
COM0A1		equ	7
TCCR0B		port	0x25		; timer/counter 0 control register B
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM02		equ	3		; timer/counter 0 waveform generation mode
FOC0B		equ	6		; timer/counter 0 force output compare B
FOC0A		equ	7		; timer/counter 0 force output compare A
TCNT0		port	0x26		; timer/counter 0
OCR0A		port	0x27		; timer/counter 0 output compare value A
OCR0B		port	0x28		; timer/counter 0 output compare value B

TCCR1A		sfr	0x80		; timer/counter 1 control register A
WGM10		equ	0		; timer/counter 1 waveform generation mode
WGM11		equ	1
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		sfr	0x81		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
WGM12		equ	3		; timer/counter 1 waveform generation mode
WGM13		equ	4
ICES1		equ	6		; timer/counter 1 capture slope selection
ICNC1		equ	7		; timer/counter 1 capture noise filter
TCCR1C		sfr	0x82		; timer/counter 1 control register C
FOC1B		equ	6		; timer/counter 1 force output compare B
FOC1A		equ	7		; timer/counter 1 force output compare A
TCNT1L		sfr	0x84		; timer/counter 1 value LSB
TCNT1H		sfr	0x85		; timer/counter 1 value MSB
OCR1AL		sfr	0x88		; timer/counter 1 output compare value A LSB
OCR1AH		sfr	0x89		; timer/counter 1 output compare value A MSB
OCR1BL		sfr	0x8a		; timer/counter 1 output compare value B LSB
OCR1BH		sfr	0x8b		; timer/counter 1 output compare value B MSB
ICR1L		sfr	0x86		; timer/counter 1 input capture value LSB
ICR1H		sfr	0x87		; timer/counter 1 input capture value MSB

TCCR2A		sfr	0xb0		; timer/counter 2 control register A
WGM20		equ	0		; timer/counter 2 waveform generation mode
WGM21		equ	1
COM2B0		equ	4		; timer/counter 2 compare mode B
COM2B1		equ	5
COM2A0		equ	6		; timer/counter 2 compare mode B
COM2A1		equ	7
TCCR2B		sfr	0xb1		; timer/counter 2 control register B
CS20		equ	0
CS21		equ	1
CS22		equ	2		; timer/counter 2  prescaler setting
WGM22		equ	3
FOC2B		equ	6		; timer/counter 2 force output compare B
FOC2A		equ	7		; timer/counter 2 force output compare A
TCNT2		sfr	0xb2		; timer/counter 2 value
OCR2A		sfr	0xb3		; timer/counter 2 output compare value A
OCR2B		sfr	0xb4		; timer/counter 2 output compare value B

TIMSK0		sfr	0x6e		; timer/counter 0 interrupt mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	1		; timer/counter 0 output compare interrupt enable A
OCIE0B		equ	2		; timer/counter 0 output compare interrupt enable B
TIMSK1		sfr	0x6f		; timer/counter 1 interrupt mask register
TOIE1		equ	0		; timer/counter 1 overflow interrupt enable
OCIE1A		equ	1		; timer/counter 1 output compare interrupt enable A
OCIE1B		equ	2		; timer/counter 1 output compare interrupt enable B
ICIE1		equ	5		; timer/counter 1 input capture event
TIMSK2		sfr	0x70		; timer/counter 2 interrupt mask register
TOIE2		equ	0		; timer/counter 2 overflow interrupt enable
OCIE2A		equ	1		; timer/counter 2 output compare interrupt enable A
OCIE2B		equ	2		; timer/counter 2 output compare interrupt enable B

TIFR0		port	0x15		; timer/counter 0 interrupt status register 
TIFR1		port	0x16		; timer/counter 1 interrupt status register
TIFR2		port	0x17		; timer/counter 2 interrupt status register

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2BUB		equ	0		; Timer/Counter Control Register 2 B Update Busy
TCR2AUB		equ	1		; Timer/Counter Control Register 2 A Update Busy
OCR2BUB		equ	2		; Output Compare Register 2 B Update Busy
OCR2AUB		equ	3		; Output Compare Register 2 A Update Busy
TCN2UB		equ	4		; Timer/Counter 2 Update Busy
AS2		equ	5		; Asynchronous Timer/Counter 2
EXCLK		equ	6		; Enable External Clock Input

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"

;----------------------------------------------------------------------------
; USART

__USART0_SPI__	equ	1
		include	"avr/usartc0.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim02c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

		restore			; re-enable listing

		endif			; __regm328inc
