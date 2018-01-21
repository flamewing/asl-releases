		ifndef	__regm649inc
__regm649inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM649.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega649                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x100,data
RAMEND		equ	0x10ff,data
FLASHEND	label	0xffff

;----------------------------------------------------------------------------
; Chip Control

MCUCR		port	0x35		; MCU Control Register
IVCE		avrbit	MCUCR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select

MCUSR		port	0x34		; MCU Status Register
PORF		avrbit	MCUSR,0		; Power-On Reset Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
BORF		avrbit	MCUSR,2		; Brown Out Reset Occured
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Mode Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2
SM2		avrbit	SMCR,3

PRR		sfr	0x64		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction ADC
PRUSART0	avrbit	PRR,1		; Power Reduction USART0
PRSPI		avrbit	PRR,2		; Power Reduction Serial Peripheral Interface
PRTIM1		avrbit	PRR,3		; Power Reduction Timer/Counter 1
PRLCD		avrbit	PRR,4		; Power Reduction LCD Controller

OSCCAL		sfr	0x66		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescale Register
CLKPS0		avrbit	CLKPR,0		; Clock Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem2.inc"
		include	"avr/spmcsr37.inc"

;----------------------------------------------------------------------------
; JTAG

JTD		avrbit	MCUCR,7		; JTAG Disable

JTRF		avrbit	MCUSR,4		; JTAG Reset Occured

OCDR		port	0x31		; On-Chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,4		; Pull-Up Disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x03 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.
PINE		port	0x0c		; Port E @ 0x0c (IO) ff.
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.
PING		port	0x12		; Port G @ 0x12 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1

GPIOR0		port	0x1e		; General Purpose I/O Registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum PCINT1_vect		; Pin Change Interrupt Request 1
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum USI_START_vect		; USI Start Condition
		nextenum USI_OVERFLOW_vect	; USI Overflow
		nextenum ANALOG_COMP_vect	; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum SPM_READY_vect		; Store Program Memory Ready
		nextenum LCD_vect		; LCD Start Of Frame

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
PCIE0		avrbit	EIMSK,4		; Pin Change Interrupt Enable 0
PCIE1		avrbit	EIMSK,5		; Pin Change Interrupt Enable 1
PCIE2		avrbit	EIMSK,6		; Pin Change Interrupt Enable 2
PCIE3		avrbit	EIMSK,7		; Pin Change Interrupt Enable 3

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
PCIF0		avrbit	EIFR,4		; Pin Change Interrupt 0 Occured
PCIF1		avrbit	EIFR,5		; Pin Change Interrupt 1 Occured
PCIF2		avrbit	EIFR,6		; Pin Change Interrupt 2 Occured
PCIF3		avrbit	EIFR,7		; Pin Change Interrupt 3 Occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR10		avrbit	GTCCR,0		; Prescaler Reset Timer 0/1
PSR2		avrbit	GTCCR,1		; Prescaler Reset Timer 2
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
CS00		avrbit	TCCR0A,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0A,1
CS02		avrbit	TCCR0A,2
WGM01		avrbit	TCCR0A,3
COM0A0		avrbit	TCCR0A,4	; Timer/Counter 0 Compare A Mode
COM0A1		avrbit	TCCR0A,5
WGM00		avrbit	TCCR0A,6	; Timer/Counter 0 Waveform Generation Mode
FOC0A		avrbit	TCCR0A,7	; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x26		; Timer/Counter 0 Value
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Value A

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3	; Timer/Counter 1 Waveform Generation Mode
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Capture Noise Filter
TCCR1C		sfr	0x82		; Timer/Counter 1 Control Register C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x8a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x8b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x86		; Timer/Counter 1 Input Capture Value LSB
ICR1H		sfr	0x87		; Timer/Counter 1 Input Capture Value MSB

TCCR2A		sfr	0xb0		; Timer/Counter 2 Control Register A
CS20		avrbit	TCCR2A,0
CS21		avrbit	TCCR2A,1
CS22		avrbit	TCCR2A,2	; Timer/Counter 2 Prescaler Setting
WGM21		avrbit	TCCR2A,3
COM2A0		avrbit	TCCR2A,4	; Timer/Counter 2 Compare Mode B
COM2A1		avrbit	TCCR2A,5
WGM20		avrbit	TCCR2A,6	; Timer/Counter 2 Waveform Generation Mode
FOC2A		avrbit	TCCR2A,7	; Timer/Counter 2 force Output Compare A
TCNT2		sfr	0xb2		; Timer/Counter 2 Value
OCR2A		sfr	0xb3		; Timer/Counter 2 Output Compare Value A

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable A
TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable B
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Event
TIMSK2		sfr	0x70		; Timer/Counter 2 Interrupt Mask Register
TOIE2		avrbit	TIMSK2,0	; Timer/Counter 2 Overflow Interrupt Enable
OCIE2A		avrbit	TIMSK2,1	; Timer/Counter 2 Output Compare Interrupt Enable A

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Status Register 
TIFR1		port	0x16		; Timer/Counter 1 Interrupt Status Register
TIFR2		port	0x17		; Timer/Counter 2 Interrupt Status Register

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2UB		avrbit	ASSR,0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		avrbit	ASSR,1		; Output Compare Register 2
TCN2UB		avrbit	ASSR,2		; Timer/Counter 2 Update Busy
AS2		avrbit	ASSR,3		; Asynchronous Timer/Counter 2
EXCLK		avrbit	ASSR,4		; Enable External Clock Input

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

MUX4		avrbit	ADMUX,4

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

;----------------------------------------------------------------------------
; LCD Controller

		include "avr/lcd.inc"

		restore			; re-enable listing

		endif			; __regm649inc
