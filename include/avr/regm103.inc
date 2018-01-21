		ifndef	__regm103inc
__regm103inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM103.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega103                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x60,data
RAMEND		equ	0x105f,data
FLASHEND	label	0x1ffff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,3
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
SRW		avrbit	MCUCR,6		; Wait State Select
SRE		avrbit	MCUCR,7		; Enable External SRAM

MCUSR		port	0x34		; MCU Status Register
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured

XDIV		port	0x3c		; XTAL Divide Control Register
XDIVEN		avrbit	XDIV,7		; XTAL Divide Enable
XDIV6		avrbit	XDIV,6		; XTAL Divide Select
XDIV5		avrbit	XDIV,5
XDIV4		avrbit	XDIV,4
XDIV3		avrbit	XDIV,3
XDIV2		avrbit	XDIV,2
XDIV1		avrbit	XDIV,1
XDIV0		avrbit	XDIV,0

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/eem.inc"

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE            port	0x01		; Port E @ 0x01 (IO) ff.
PINF            port	0x00		; Port F @ 0x01 (IO) ff.,
PINF_inponly	equ	  1		; Input Only

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum INT3_vect		; External Interrupt Request 3
		nextenum INT4_vect		; External Interrupt Request 4
		nextenum INT5_vect		; External Interrupt Request 5
		nextenum INT6_vect		; External Interrupt Request 6
		nextenum INT7_vect		; External Interrupt Request 7
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum UART_RX_vect		; UART Rx Complete
		nextenum UART_UDRE_vect		; UART Data Register Empty
		nextenum UART_TX_vect		; UART Tx Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum ANALOG_COMP_vect	; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

EICR		port	0x3a		; External Interrupt Control Register
ISC40		avrbit	EICR,0		; External Interrupt 4 Sense Control
ISC41		avrbit	EICR,1
ISC50		avrbit	EICR,2		; External Interrupt 5 Sense Control
ISC51		avrbit	EICR,3
ISC60		avrbit	EICR,4		; External Interrupt 6 Sense Control
ISC61		avrbit	EICR,5
ISC70		avrbit	EICR,6		; External Interrupt 7 Sense Control
ISC71		avrbit	EICR,7

EIMSK		port	0x39		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1
INT2		avrbit	EIMSK,2		; Enable External Interrupt 2
INT3		avrbit	EIMSK,3		; Enable External Interrupt 3
INT4		avrbit	EIMSK,4		; Enable External Interrupt 4
INT5		avrbit	EIMSK,5		; Enable External Interrupt 5
INT6		avrbit	EIMSK,6		; Enable External Interrupt 6
INT7		avrbit	EIMSK,7		; Enable External Interrupt 7

EIFR		port	0x38		; External Interrupt Flags Register
INTF4	        avrbit	EIFR,4		; External Interrupt 4 Occured
INTF5		avrbit	EIFR,5		; External Interrupt 5 Occured
INTF6		avrbit	EIFR,6		; External Interrupt 6 Occured
INTF7	        avrbit	EIFR,7		; External Interrupt 7 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
CTC0		avrbit	TCCR0,3		; Timer/Counter 0 Clear on Compare Match
COM00		avrbit	TCCR0,4		; Timer/Counter 0 Compare Mode
COM01		avrbit	TCCR0,5
PWM0		avrbit	TCCR0,6		; Timer/Counter 0 PWM Mode
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0		port	0x31

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Timer/Counter 1 PWM Mode
PWM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Timer/Counter 1 Clear on Compare Match
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x26		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x27		; Timer/Counter 1 Input Capture Value MSB

TCCR2		port	0x25		; Timer/Counter 2 Control Register
CS20		avrbit	TCCR2,0		; Timer/Counter 2 Prescaler Setting
CS21		avrbit	TCCR2,1
CS22		avrbit	TCCR2,2
CTC2		avrbit	TCCR2,3		; Timer/Counter 2 Clear on Compare Match
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
PWM2		avrbit	TCCR2,6		; Timer/Counter 2 PWM Mode
TCNT2		port	0x24		; Timer/Counter 2 Value
OCR2		port	0x23		; Timer/Counter 2 Output Compare Value

TIMSK		port	0x37		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0		avrbit	TIMSK,1		; Timer/Counter 0 Output Compare Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,3		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt Enable A
TICIE1		avrbit	TIMSK,5		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,6		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,7		; Timer/Counter 2 Output Compare Interrupt Enable

TIFR		port	0x36		; Timer Interrupt Flag Register

ASSR		port	0x30		; Asynchronous Status Register
TCR0UB		avrbit	ASSR,0		; Timer/Counter Control Register 0 Update Busy
OCR0UB		avrbit	ASSR,1		; Output Compare Register 0
TCN0UB		avrbit	ASSR,2		; Timer/Counter 0 Update Busy
AS0		avrbit	ASSR,3		; Asynchronous Timer/Counter 0

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; UART

		include	"avr/uart90.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spi90.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x06		; Control/Status Register
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		restore			; re-enable listing

		endif			; __regm103inc
