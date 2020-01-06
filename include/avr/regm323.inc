		ifndef	__regm323inc
__regm323inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM323.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega323                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	1023
RAMSTART	equ	0x60,data
RAMEND		equ	0x85f,data
FLASHEND	label	0x7fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,4
SM1		avrbit	MCUCR,5
SM2		avrbit	MCUCR,6		; Sleep Mode Select
SE		avrbit	MCUCR,7		; Sleep Enable

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		avrbit	MCUCSR,3	; Watchdog Reset Occured
BORF		avrbit	MCUCSR,2	; Brown-Out Reset Occured
EXTRF		avrbit	MCUCSR,1	; External Reset Occured
PORF		avrbit	MCUCSR,0	; Power-On Reset Occured

OSCCAL		port	0x04		; Oscillator Calibration

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem.inc"

SPMCR		port	0x37		; Store Program Memory Control Register
ASB		avrbit	SPMCR,6		; Application Section Busy
ASRE		avrbit	SPMCR,4		; Application Section Rd Enable
BLBSET		avrbit	SPMCR,3		; Boot Lock Bit Set
PGWRT		avrbit	SPMCR,2		; Page Write
PGERS		avrbit	SPMCR,1		; Page Erase
SPMEN		avrbit	SPMCR,0		; Store Program Eemory Enable

;----------------------------------------------------------------------------
; JTAG

JTRF		avrbit	MCUCSR,4	; JTAG Reset Occured
JTD		avrbit	MCUCSR,7	; JTAG Disable

OCDR		port	0x31		; On-Chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

SFIOR		port	0x30		; Special Function I/O Register
PUD		avrbit	SFIOR,2		; Pull-Up Disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART_RX_vectc		; USART Rx Complete
		nextenum USART_UDRE_vect	; USART Data Register Empty
		nextenum USART_TX_vectc		; USART Tx Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum TWI_vect		; 2-Wire Serial Interface
		nextenum SPM_RDY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

		; bits in MCUCSR
ISC2		avrbit	MCUCSR,6	; External Interrupt 2 Sense Control

GICR		port	0x3b		; General Interrupt Control Register
IVCE		avrbit	GICR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	GICR,1		; Interrupt Vector Select
INT2		avrbit	GICR,5		; Enable External Interrupt 2
INT0		avrbit	GICR,6		; Enable External Interrupt 0
INT1		avrbit	GICR,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF2		avrbit	GIFR,5		; External Interrupt 2 Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1	        avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

PSR10		avrbit	SFIOR,0		; Prescaler Reset T0/1
PSR2		avrbit	SFIOR,1		; ditto T2

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
CTC0		avrbit	TCCR0,3		; Timer/Counter 0 Clear on Match
COM00		avrbit	TCCR0,4		; Timer/Counter 0 Compare Mode
COM01		avrbit	TCCR0,5
PWM0		avrbit	TCCR0,6		; Timer/Counter 0 PWM Mode
FOC0		avrbit	TCCR0,7		; Timer/Counter 0 Force Output Compare
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0		port	0x3c		; Timer/Counter 0 Output Compare Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Timer/Counter 1 PWM Config
PWM11		avrbit	TCCR1A,1
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
CTC1		avrbit	TCCR1B,3	; Timer/Counter 1 Clear on Match
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
CTC2		avrbit	TCCR2,3		; Timer/Counter 2 Clear on Match
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
PWM2		avrbit	TCCR2,6		; Timer/Counter 2 PWM Config
FOC2		avrbit	TCCR2,7		; Timer/Counter 2 Force Output Compare
TCNT2		port	0x24		; Timer/Counter 2 Value
OCR2		port	0x23		; Timer/Counter 2 Output Compare Value

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0		avrbit	TIMSK,1		; Timer/Counter 0 Output Compare Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,3		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt Enable A
TICIE1		avrbit	TIMSK,5		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,6		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,7		; Timer/Counter 2 Output Compare Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

ASSR		port	0x22		; Asynchronous Status Register
TCR2UB		avrbit	TIFR,0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		avrbit	TIFR,1		; Output Compare Register 2
TCN2UB		avrbit	TIFR,2		; Timer/Counter 2 Update Busy
AS2		avrbit	TIFR,3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; USART

		include	"usartm.inc"

;----------------------------------------------------------------------------
; SPI

		include	"spim.inc"

;----------------------------------------------------------------------------
; TWI

		include	"twim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include "adcm8.inc"

		restore			; re-enable listing

		endif			; __regm323inc
