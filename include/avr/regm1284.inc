		ifndef	__regm1284inc
__regm1284inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM1284.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATmega1284                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x100,data
RAMEND		equ	0x40ff,data
FLASHEND	label	0x1ffff

;----------------------------------------------------------------------------
; Chip Control

MCUCR		port	0x35		; MCU Control Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select

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

PRR0		sfr	0x64		; Power Reduction Register 0
PRADC		equ	0		; Power Reduction ADC
PRUSART0	equ	1		; Power Reduction USART0
PRSPI0		equ	2		; Power Reduction Serial Peripheral Interface 0
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRUSART1	equ	4		; Power Reduction USART1
PRTIM0		equ	5		; Power Reduction Timer/Counter 0
PRTIM2		equ	6		; Power Reduction Timer/Counter 2
PRTWI		equ	7		; Power Reduction TWI

PRR1		sfr	0x65		; Power Reduction Register 1
PRTIM3		equ	0		; Power Reduction Timer/Counter 3

OSCCAL		sfr	0x66		; oscillator calibration

CLKPR		sfr	0x61		; clock prescale register
CLKPS0		equ	0		; clock prescaler select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

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
; JTAG

		; bits in MCUCR
JTD		equ	7		; JTAG disable

		; bits in MCUSR
JTRF		equ	4		; JTAG reset occured

OCDR		port	0x31		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	4		; pull up disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x03 (IO) ff.
PIND		port    0x09		; Port D @ 0x09 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1
PCMSK2		sfr	0x6d		; Pin Change Mask Register 2
PCMSK3		sfr	0x73		; Pin Change Mask Register 3
PCICR		sfr	0x68		; Pin Change Interrupt Control Register
PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

GPIOR0		port	0x1e		; general purpose registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum PCINT2_vect		; pin change interrupt request 2
		nextenum PCINT3_vect		; pin change interrupt request 3
		nextenum WDT_vect		; watchdog time-out interrupt
		nextenum TIMER2_COMPA_vect	; timer/counter 2 compare match A
		nextenum TIMER2_COMPB_vect	; timer/counter 2 compare match B
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMPA_vect	; timer/counter 0 compare match A
		nextenum TIMER0_COMPB_vect	; timer/counter 0 compare match B
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART0_RX_vect		; USART0 Rx complete
		nextenum USART0_UDRE_vect	; USART0 data register empty
		nextenum USART0_TX_vect		; USART0 Tx complete
		nextenum ANALOG_COMP_vect	; analog comparator
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum TWI_vect		; two-wire serial interface
		nextenum SPM_READY_vect		; store program memory ready
		nextenum USART1_RX_vect		; USART1 Rx complete
		nextenum USART1_UDRE_vect	; USART1 data register empty
		nextenum USART1_TX_vect		; USART1 Tx complete
		nextenum TIMER3_CAPT_vect	; timer/counter 3 capture event
		nextenum TIMER3_COMPA_vect	; timer/counter 3 compare match A
		nextenum TIMER3_COMPB_vect	; timer/counter 3 compare match B
		nextenum TIMER3_OVF_vect	; timer/counter 3 overflow

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3
ISC20		equ	4		; external interrupt 2 sense control
ISC21		equ	5

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1
INT2		equ	2		; enable external interrupt 2

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; external Interrupt 0 occured
INTF1	        equ	1		; external Interrupt 1 occured
INTF2		equ	2		; external Interrupt 2 occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		equ	0		; Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; timer/counter 0 control register A
WGM00		equ	0		; timer/counter 0 waveform generation mode
WGM01		equ	1
COM0B0		equ	4		; timer/counter 0 compare mode B
COM0B1		equ	5
COM0A0		equ	6		; timer/counter 0 compare mode A
COM0A1		equ	7
TCCR0B		port	0x25		; timer/counter 0 control register B
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; timer/counter 0 force output compare match B
FOC0A		equ	7		; timer/counter 0 force output compare match A
TCNT0		port	0x26		; timer/counter 0 value
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
WGM12		equ	3
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
COM2A0		equ	6		; timer/counter 2 compare mode A
COM2A1		equ	7
TCCR2B		sfr	0xb1		; timer/counter 2 control register B
CS20		equ	0		; timer/counter 2 prescaler setting
CS21		equ	1
CS22		equ	2
WGM22		equ	3
FOC2B		equ	6		; timer/counter 2 force output compare B
FOC2A		equ	7		; timer/counter 2 force output compare A
TCNT2		sfr	0xb2		; timer/counter 2 value
OCR2A		sfr	0xb3		; timer/counter 2 output compare value A
OCR2B		sfr	0xb4		; timer/counter 2 output compare value B

TCCR3A		sfr	0x90		; timer/counter 3 control register A
WGM30		equ	0		; timer/counter 3 waveform generation mode
WGM31		equ	1
COM3B0		equ	4		; timer/counter 3 compare mode B
COM3B1		equ	5
COM3A0		equ	6		; timer/counter 3 compare mode A
COM3A1		equ	7
TCCR3B		sfr	0x91		; timer/counter 3 control register B
CS30		equ	0		; timer/counter 3 prescaler setting
CS31		equ	1
CS32		equ	2
WGM32		equ	3
WGM33		equ	4
ICES3		equ	6		; timer/counter 3 capture slope selection
ICNC3		equ	7		; timer/counter 3 capture noise filter
TCCR3C		sfr	0x92		; timer/counter 3 control register C
FOC3B		equ	6		; timer/counter 3 force output compare B
FOC3A		equ	7		; timer/counter 3 force output compare A
TCNT3L		sfr	0x94		; timer/counter 3 value LSB
TCNT3H		sfr	0x95		; timer/counter 3 value MSB
OCR3AL		sfr	0x98		; timer/counter 3 output compare value A LSB
OCR3AH		sfr	0x99		; timer/counter 3 output compare value A MSB
OCR3BL		sfr	0x9a		; timer/counter 3 output compare value B LSB
OCR3BH		sfr	0x9b		; timer/counter 3 output compare value B MSB
ICR3L		sfr	0x96		; timer/counter 3 input capture value LSB
ICR3H		sfr	0x97		; timer/counter 3 input capture value MSB

TIMSK0		sfr	0x6e		; timer/counter 0 interrupt mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	1		; timer/counter 0 output compare interrupt enable A
OCIE0B		equ	2		; timer/counter 0 output compare interrupt enable B
TIMSK1		sfr	0x6f		; timer/counter 1 interrupt mask register
TOIE1		equ	0		; timer/counter 1 overflow interrupt enable
OCIE1A		equ	1		; timer/counter 1 output compare interrupt enable A
OCIE1B		equ	2		; timer/counter 1 output compare interrupt enable B
ICIE1		equ	5		; timer/counter 1 input capture enable
TIMSK2		sfr	0x70		; timer/counter 2 interrupt mask register
TOIE2		equ	0		; timer/counter 2 overflow interrupt enable
OCIE2A		equ	1		; timer/counter 2 output compare interrupt enable A
OCIE2B		equ	2		; timer/counter 2 output compare interrupt enable B
TIMSK3		sfr	0x71		; timer/counter 3 interrupt mask register
TOIE3		equ	0		; timer/counter 3 overflow interrupt enable
OCIE3A		equ	1		; timer/counter 3 output compare interrupt enable A
OCIE3B		equ	2		; timer/counter 3 output compare interrupt enable B
ICIE3		equ	5		; timer/counter 3 input capture enable

TIFR0		port	0x15		; timer/counter 0 interrupt status register
TIFR1		port	0x16		; timer/counter 1 interrupt status register
TIFR2		port	0x17		; timer/counter 2 interrupt status register
TIFR3		port	0x18		; timer/counter 3 interrupt status register

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
__USART1_SPI__	equ	1
		include	"avr/usartc8.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim02c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

		; additional bits in ADMUX
MUX4		equ	4

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

		restore			; re-enable listing

		endif			; __regm1284inc
