		ifndef	__regtn15inc
__regtn15inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN15.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny15                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	63		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM (i.e. no SRAM)
FLASHEND	label   1023		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU Control Register
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Mode Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset
BORF		equ	2		; Brown Out Reset
EXTRF		equ	1		; External Reset Occured
PORF		equ	0		; Power-Fail Reset Occured


OSCCAL	 	port	0x31		; Oscillator Calibration

;----------------------------------------------------------------------------
; EEPROM Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EERE		equ	0		; EEPROM Read Enable
EEWE		equ	1		; EEPROM Write Enable
EEMWE		equ	2		; EEPROM Multiple Write Enable
EERIE		equ	3		; EEPROM Ready Interrupt Enable

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; Pull-Up Disable

PINB		port	0x16		; Port B @ 0x16 (bits 0..5)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum IO_PINS_vect		; Low-level Input on Port B
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_OVF_vect	; timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; Interrupt Sense Control 0
ISC01		equ	1

GIMSK		port	0x3b		; Global Interrupt Mask Register
PCIE		equ	5		; Enable Pin Change Interrupt
INT0		equ	6		; Enable External Interrupt 0

GIFR		port	0x3a		; Interrupt Flag Register
PCIF		equ	5		; Pin Change Interrupt Occured
INTF0		equ	6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00            equ     0               ; Timer/Counter 0 Clock Select
CS01            equ     1
CS02            equ     2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1		port	0x30		; Timer/Counter 1 Control Register
CS10		equ	0		; Timer/Counter 1 Clock Select
CS11		equ	1
CS12		equ	2
CS13		equ	3
COM1A0		equ	4		; Timer/Counter 1 Output Compare Mode
COM1A1		equ	5
PWM1		equ	6		; Timer/Counter 1 PWM Mode
CTC1		equ	7		; Timer/Counter 1 Clear on Compare Match
TCNT1		port	0x2f		; Timer/Counter 1 Value
OCR1A		port	0x2e		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2d		; Timer/Counter 1 Output Compare Value B

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		equ	2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		equ	6		; Timer/Counter 1 Output Compare Match A Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

SFIOR		port	0x2c		; Special Function I/O Register
PSR0		equ	0		; Timer/Counter 0 Prescaler Reset
PSR1		equ	1		; Timer/Counter 1 Prescaler Reset
FOC1A		equ	2		; Timer/Counter 1 Force Output Compare Match A

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

		; bits in WDTCR
WDTOE		equ	4		; Enable Time-Out Interrupt

;----------------------------------------------------------------------------
; Analog Comparator

		include	"avr/ac90.inc"

		; bits in ACSR
ACBG		equ	6		; Enable Band Gap

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; ADC Multiplexer Selection Register
MUX0		equ	0		; Analog Channel and Gain Selection
MUX1		equ	1
MUX2		equ	2
ADLAR		equ	5		; Left Adjust Result
REFS0		equ	6		; Reference Selection
REFS1		equ	7

ADCSR		port	0x06		; ADC Control and Status Register
ADPS0		equ	0		; ADC Prescaler Select Bits
ADPS1		equ	1
ADPS2		equ	2
ADIE		equ	3		; ADC Interrupt Enable
ADIF		equ	4		; ADC Interrupt Flag
ADFR		equ	5		; ADC Free Running Select
ADSC		equ	6		; ADC Start Conversion
ADEN		equ	7		; ADC Enable

ADCL		port	0x04		; ADC Value LSB
ADCH		port	0x05		; ADC Value MSB

		restore

		endif			; __regtn15inc
