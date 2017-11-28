		ifndef	__regt26inc
__regt26inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT26.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny26                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0xdf,data	; end address SRAM
FLASHEND	label   2047		; end address Flash

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

PLLCSR		port	0x29		; PLL Control/Status Register
PLOCK		equ	0		; PLL Lock Detector
PLLE		equ	1		; PLL Enable
PCKE		equ	2		; PCK Enable

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMWE		equ	2		; EEPROM Master Write Enable
EEWE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; pull up disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum IO_PINS_vect		; Pin Change Interrupt
		nextenum TIMER1_CMPA_vect	; Timer/Counter 1 Compare Match 1A
		nextenum TIMER1_CMPB_vect	; Timer/Counter 1 Compare Match 1B
		nextenum TIMER1_OVF1_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF0_vect	; Timer/Counter 0 Overflow
		nextenum USI_STRT_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; enable external Interrupt 0
PCIE1		equ	5		; pin change interrupt enable 1
PCIE0		equ	4		; pin change interrupt enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 occured
PCIF		equ	5		; pin change interrupt 0 occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
PSR0		equ	3		; Timer/Counter 0 Prescaler Reset
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x30		; Timer/Counter 1 Control Register A
PWM1B		equ	0		; Timer/Tounter 1 PWM Control
PWM1A		equ	1
FOC1B		equ	2		; Timer/Counter 1 Force Output Compare Match B
FOC1A		equ	3		; Timer/Counter 1 Force Output Compare Match A
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	7
TCCR1B		port	0x2f		; Timer/Counter 1 Control Register B
CS10            equ     0               ; Timer/Counter 1 Clock Select
CS11            equ     1
CS12            equ     2
CS13		equ	3
PSR1		equ	6		; Timer/Counter 1 Prescaler Reset
CTC1		equ	7		; Timer/Counter 1 Clean on Match
TCNT1		port	0x2e		; Timer/Counter 1 Value
OCR1A		port	0x2d		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2c		; Timer/Counter 1 Output Compare Value B
OCR1C		port	0x2b		; Timer/Counter 1 Output Compare Value C


TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	5		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	6		; timer/counter 1 output compare interrupt enable A

TIFR		port	0x38		; Timer Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

		; additional bits in WDTCR
WDCE		equ	4		; Watchdog Change Enable

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS1		equ	7		; reference selection bits
REFS0		equ	6
ADLAR		equ	5		; left adjust right
MUX3		equ	3		; Multiplexer
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADFR		equ	5		; free running select
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

		restore

		endif			; __regt26inc
