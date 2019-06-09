		ifndef	__reg8535inc
__reg8535inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG8535.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S8535                      *
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
SM		avrbit	MCUCR,4		; Choose Idle/Power-Down Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

MCUSR		port	0x34		; MCU General Status Register
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-on Reset Occured


;----------------------------------------------------------------------------
; EEPROM

		include	"ee90.inc"

EEMWE		avrbit	EECR,2		; EEPROM Master Write Enable
EERIE		avrbit	EECR,3		; EEPROM Interrupt Enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER2_COMP		; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF		; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum UART_RX_vect		; UART Rx Complete
		nextenum UART_UDRE_vect		; UART Data Register Empty
		nextenum UART_TX_vect		; UART Tx Complete
		nextenum ADC_vect		; A/D Converter
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1		avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Mode of Pulse Width Modulator
PWM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Clear after Equality?
ICES1		avrbit	TCCR1B,6	; Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x27		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x26		; Timer/Counter 1 Input Capture Value MSB

TCCR2		port	0x25		; Timer/Counter 2 Control Register
CS20		avrbit	TCCR2,0		; Timer/Counter 2 Clock Select
CS21		avrbit	TCCR2,1
CS22		avrbit	TCCR2,2
CTC2		avrbit	TCCR2,3		; Timer/Counter 2 Clear Timer/Counter on Compare Match
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Output Mode
COM21		avrbit	TCCR2,5
PWM2		avrbit	TCCR2,6		; Timer/Counter 2 Pulse Width Modulator Enable
TCNT2		port	0x24		; Timer/Counter 2 Value
OCR2		port	0x23		; Timer/Counter 2 Output Compare Value

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,3		; Timer/Counter 1 Output Compare Interrupt B Enable
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt A Enable
TICIE1		avrbit	TIMSK,5		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,6		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,7		; Timer/Counter 2 Output Compare Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

ASSR		port	0x22		; Asynchronous Status Register
AS2		avrbit	ASSR,3		; Asynchronous Timer/Counter2
TCN2UB		avrbit	ASSR,2		; Timer/Counter2 Update Busy
OCR2UB		avrbit	ASSR,1		; Output Compare Register 2 Update Busy
TCR2UB		avrbit	ASSR,0		; Timer/Counter Control Register 2 Update Busy

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; UART

		include	"uart90.inc"

;----------------------------------------------------------------------------
; SPI

		include	"spi90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "ac90.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		avrbit	ADMUX,2		; Analog Channel Select Bits
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x06		; Control/Status Register
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADFR		avrbit	ADCSR,5		; Free Running Select
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		restore

		endif			; __reg8535inc
