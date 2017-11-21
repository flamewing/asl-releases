		ifndef	__regm169inc
__regm169inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM169.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega169                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x4ff,data
FLASHEND	label	0x3fff

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
PRSPI		equ	2		; Power Reduction Serial Peripheral Interface
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRLCD		equ	4		; Power Reduction LCD Controller

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
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.
PINE		port	0x0c		; Port E @ 0x0c (IO) ff.
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.
PING		port	0x12		; Port G @ 0x12 (IO) ff.
PINH		sfr	0xd8		; Port H @ 0xd8 (DATA) ff.
PINJ		sfr	0xdb		; Port J @ 0xdb (DATA) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1

GPIOR0		port	0x1e		; general purpose registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum TIMER2_COMP_vect	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART0_RX_vect		; USART0 Rx complete
		nextenum USART0_UDRE_vect	; USART0 data register empty
		nextenum USART0_TX_vect		; USART0 Tx complete
		nextenum USI_START_vect		; USI start condition
		nextenum USI_OVERFLOW_vect	; USI overflow
		nextenum ANALOG_COMP_vect	; analog comparator
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum SPM_READY_vect		; store program memory ready
		nextenum LCD_vect		; LCD start of frame
		nextenum PCINT2_vect		; pin change interrupt request 2
		nextenum PCINT3_vect		; pin change interrupt request 3

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
PCIE0		equ	4		; pin change interrupt enable 0
PCIE1		equ	5		; pin change interrupt enable 1
PCIE2		equ	6		; pin change interrupt enable 2
PCIE3		equ	7		; pin change interrupt enable 3

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; external Interrupt 0 occured
PCIF0		equ	4		; pin change interrupt 0 occured
PCIF1		equ	5		; pin change interrupt 1 occured
PCIF2		equ	6		; pin change interrupt 2 occured
PCIF3		equ	7		; pin change interrupt 3 occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR10		equ	0		; Prescaler Reset Timer 0/1
PSR2		equ	1		; Prescaler Reset Timer 2
TSM		equ	7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; timer/counter 0 control register A
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM01		equ	3
COM0A0		equ	4		; timer/counter 0 compare mode A
COM0A1		equ	5
WGM00		equ	6		; timer/counter 0 waveform generation mode
FOC0A		equ	7		; timer/counter 0 force output compare match A
TCNT0		port	0x26		; timer/counter 0 value
OCR0A		port	0x27		; timer/counter 0 output compare value A

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
CS20		equ	0		; timer/counter 2 prescaler setting
CS21		equ	1
CS22		equ	2
WGM21		equ	3
COM2A0		equ	4		; timer/counter 2 compare mode A
COM2A1		equ	5
WGM20		equ	6		; timer/counter 2 waveform generation mode
FOC2A		equ	7		; timer/counter 2 force output compare A
TCNT2		sfr	0xb2		; timer/counter 2 value
OCR2A		sfr	0xb3		; timer/counter 2 output compare value A

TIMSK0		sfr	0x6e		; timer/counter 0 interrupt mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	1		; timer/counter 0 output compare interrupt enable A
TIMSK1		sfr	0x6f		; timer/counter 1 interrupt mask register
TOIE1		equ	0		; timer/counter 1 overflow interrupt enable
OCIE1A		equ	1		; timer/counter 1 output compare interrupt enable A
OCIE1B		equ	2		; timer/counter 1 output compare interrupt enable B
ICIE1		equ	5		; timer/counter 1 input capture enable
TIMSK2		sfr	0x70		; timer/counter 2 interrupt mask register
TOIE2		equ	0		; timer/counter 2 overflow interrupt enable
OCIE2A		equ	1		; timer/counter 2 output compare interrupt enable A

TIFR0		port	0x15		; timer/counter 0 interrupt status register
TIFR1		port	0x16		; timer/counter 1 interrupt status register
TIFR2		port	0x17		; timer/counter 2 interrupt status register

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2UB		equ	0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		equ	1		; Output Compare Register 2
TCN2UB		equ	2		; Timer/Counter 2 Update Busy
AS2		equ	3		; Asynchronous Timer/Counter 2
EXCLK		equ	4		; Enable External Clock Input

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm60.inc"

;----------------------------------------------------------------------------
; USART

		include	"avr/usartc0.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim2c.inc"

;----------------------------------------------------------------------------
; USI

		include	"avr/usimb8.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

		; bits in ADMUX
MUX4		equ	4

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

;----------------------------------------------------------------------------
; LCD Controller

		include "avr/lcd.inc"

		restore			; re-enable listing

		endif			; __regm169inc
