		ifndef	__reg2313inc
__reg2313inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG2313.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S2313                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	127
RAMSTART	equ	0x60,data
RAMEND		equ	0xdf,data
FLASHEND	label	0x7ff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Powerdown Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

;----------------------------------------------------------------------------
; EEPROM

		include	"ee90.inc"

EEMWE		avrbit	 EECR,2		; EEPROM Master Write Enable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMP_vect	; Timer/Counter 1 Compare Match
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum UART_RX_vect		; UART Rx Complete
		nextenum UART_UDRE_vect		; UART Data Register Empty
		nextenum UART_TX_vect		; UART Tx Complete
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1		avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Mode of Pulse Width Modulator
PWM11		avrbit	TCCR1A,1
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Clear After Equality?
ICES1		avrbit	TCCR1B,6	; Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture Value MSB

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TICIE1		avrbit	TIMSK,3		; Timer/Counter 1 Input Capture Interrupt Enable
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; UART

		include "uart90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "ac90.inc"

		restore

		endif			; __reg2313inc
