		ifndef	__regm8inc
__regm8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM8.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega8                        *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x60,data
RAMEND		equ	0x45f,data
FLASHEND	label	0x1fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SE		avrbit	MCUCR,7		; Sleep Enable
SM2		avrbit	MCUCR,6		; Sleep Mode Select
SM1		avrbit	MCUCR,5
SM0		avrbit	MCUCR,4

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		avrbit	MCUCSR,3	; Watchdog Reset Occured
BORF		avrbit	MCUCSR,2	; Brown-Out Occured
EXTRF		avrbit	MCUCSR,1	; External Reset Occured
PORF		avrbit	MCUCSR,0	; Power-On Reset Occured

OSCCAL		port	0x31		; Oscillator Calibration

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem.inc"
		include	"spm.inc"

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

SFIOR		port	0x30		; Special Function I/O Register
PUD		avrbit	SFIOR,2		; Pullup Disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum UART_RX_vect		; UART Rx Complete
		nextenum UART_UDRE_vect		; UART Data Register Empty
		nextenum UART_TX_vect		; UART Tx Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum TWI_vect		; Two-Wire Serial Interface
		nextenum SPM_RDY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

GICR		port	0x3b		; General Interrupt Control Register
IVCE		avrbit	GICR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	GICR,1		; Interrupt Vector Select
INT0		avrbit	GICR,6		; Enable External Interrupt 0
INT1		avrbit	GICR,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags:
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1	        avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

PSR10		avrbit	SFIOR,0		; Prescaler Reset T0/T1
PSR2		avrbit	SFIOR,1		; ditto T2

ASSR		port	0x22		; Asynchronous Status Register
AS2		avrbit	ASSR,3		; Asynchronous Timer/Counter2
TCN2UB		avrbit	ASSR,2		; Timer/Counter2 Update Busy
OCR2UB		avrbit	ASSR,1		; Output Compare Register2 Update Busy
TCR2UB		avrbit	ASSR,0		; Timer/Counter Control Register2 Update Busy

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
FOC1B		avrbit	TCCR1A,2	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1A,3	; Timer/Counter 1 Force Output Compare A
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3	; Timer/Counter 1 Waveform Generation Mode
WGM13		avrbit	TCCR1B,4
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
CS20		avrbit	TCCR2,0		; Timer/Counter 2  Prescaler Setting
CS21		avrbit	TCCR2,1
CS22		avrbit	TCCR2,2
WGM21		avrbit	TCCR2,3
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
WGM20		avrbit	TCCR2,6		; Timer/Counter 2 Waveform Generation Mode
FOC2		avrbit	TCCR2,7		; Timer/Counter 2 Force Output Compare
TCNT2		port	0x24		; Timer/Counter 2 Value
OCR2		port	0x23		; Timer/Counter 2 Output Compare Value

TIMSK		port	0x39		; Timer Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,3		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt Enable A
TICIE1		avrbit	TIMSK,5		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,6		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,7		; Timer/Counter 2 Output Compare Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
WDCE		avrbit	WDTCR,4		; Change Enable

;----------------------------------------------------------------------------
; USART

		include "usartm.inc"

;----------------------------------------------------------------------------
; SPI

		include "spim.inc"

;----------------------------------------------------------------------------
; TWI

		include "twim.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"adcm8.inc"
ADHSM		avrbit	SFIOR,4		; ADC High Speed Mode

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm.inc"

		restore			; re-enable listing

		endif			; __regm8inc
