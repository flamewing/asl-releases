		ifndef	__regt13inc
__regt13inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT13.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny13                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x9f,data	; end address SRAM
FLASHEND	label   1023		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-on Reset Flag

OSCCAL		port	0x31		; oscillator calibration

CLKPR		port	0x26		; Clock Prescale Register
CLKPS0		equ	0		; Clock Prescaler Select Bits
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x2e		; debugWire Data Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Programming Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Program Enable
EEPE		equ	1		; EEPROM Program Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control and Status Register
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SELFPRGEN	equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; pull up disable

PINB		port	0x16		; Port B @ 0x16 (IO) ff. (6 bits)

PCMSK		port	0x15		; Pin Change Interrupt Mask
PCINT0		equ	0		; enable pin change interrupt 0
PCINT1		equ	1		; enable pin change interrupt 1
PCINT2		equ	2		; enable pin change interrupt 2
PCINT3		equ	3		; enable pin change interrupt 3
PCINT4		equ	4		; enable pin change interrupt 4
PCINT5		equ	5		; enable pin change interrupt 5

DIDR0		port	0x14		; Digital Input Disable Register 0
AIN0D		equ	0		; Analog Comparator Digital Input Disable
AIN1D		equ	1
ADC1D		equ	2		; ADC Digital Input Disable
ADC3D		equ	3
ADC2D		equ	4
ADC0D		equ	5

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum TIM0_OVF_vect		; timer/counter 0 overflow
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator
		nextenum TIM0_COMPA_vect	; timer/counter 0 Compare Match A
		nextenum TIM0_COMPB_vect	; timer/counter 0 Compare Match B
		nextenum WDT_vect		; watchdog timer
		nextenum ADC_vect		; ADC conversion complete

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; enable external Interrupt 0
PCIE		equ	5		; pin change interrupt enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 occured
PCIF		equ	5		; pin change interrupt 0 occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2f		; timer/counter 0 control register A
WGM00		equ	0		; waveform generation mode
WGM01		equ	1
COM0B0		equ	4		; compare mode B
COM0B1		equ	5
COM0A0		equ	6		; compare mode A
COM0A1		equ	7
TCCR0B		port	0x33		; timer/counter 0 control register B
CS00		equ	0		; clock select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; force output compare match B
FOC0A		equ	7		; force output compare match A
TCNT0		port	0x32		; timer/counter 0 value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x29		; Timer/Counter 0 Output Compare Value B

TIMSK0		port	0x39		; timer interrupt mask register 0
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	2		; timer/counter 0 output compare interrupt enable A
OCIE0B		equ	3		; timer/counter 0 output compare interrupt enable B

TIFR0		port	0x38		; timer interrupt status register 0

GTTCCR		port	0x28		; global timer/counter control register
PSR10		equ	0		; Prescaler Reset Timer 0/1
TSM		equ	7		; timer synchronization

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

		; additional bits in WDTCR
WDCE		equ	4		; Watchdog Change Enable
WDP3		equ	5		; 
WDTIE		equ	6		; Watchdog Timer Interrupt Enable
WDTIF		equ	7		; Watchdog Timer Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS0		equ	6		; reference selection bits
ADLAR		equ	5		; left adjust right
MUX1		equ	1
MUX0		equ	0

ADCSRA		port	0x06		; control/status register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; free running select
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x03		; control/status register B
ADTS0		equ	0		; ADC Auto Trigger Source
ADTS1		equ	1
ADTS2		equ	2
ACME		equ	6		; analog comparator multiplexer enable

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __regt13inc
