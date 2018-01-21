		ifndef	__regtnx8inc
__regtnx8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX8.INC                                             *
;*                                                                          *
;*   Contains common bit & Register definitions for ATtiny48/88             *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		avrbit	MCUCR,5		; BOD Sleep
BODS		avrbit	MCUCR,6		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2

OSCCAL		sfr	0x66		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

PRR		sfr	0x64		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRSPI		avrbit	PRR,2		; Power Reduction SPI
PRTIM1		avrbit	PRR,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR,5		; Power Reduction Timer/Counter 0
PRTWI		avrbit	PRR,7		; Power Reduction Two Wire Interface

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x21		; EEPROM Address Register Low
EEDR		port	0x20		; EEPROM Data Register
EECR		port	0x1f		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RWWSB		avrbit	SPMCSR,6	; Read-While-Write Section Busy
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SELFPRGEN	avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,4		; Pull-Up Disable

PINA		port	0x0c		; Port A @ 0x0c (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PORTCR		port	0x12		; Port Control Register
PUDA		avrbit	PORTCR,0	; Pull-up Disable Port A
PUDB		avrbit	PORTCR,1	; Pull-up Disable Port B
PUDC		avrbit	PORTCR,2	; Pull-up Disable Port C
PUDD		avrbit	PORTCR,3	; Pull-up Disable Port D
BBMA		avrbit	PORTCR,4	; Break-Before-Make Mode Enable A
BBMB		avrbit	PORTCR,5	; Break-Before-Make Mode Enable B
BBMC		avrbit	PORTCR,6	; Break-Before-Make Mode Enable C
BBMD		avrbit	PORTCR,7	; Break-Before-Make Mode Enable D

PCMSK0		sfr	0x6b		; Pin Change Interrupt Mask 0
PCMSK1		sfr	0x6c		; Pin Change Interrupt Mask 1
PCMSK2		sfr	0x6d		; Pin Change Interrupt Mask 2
PCMSK3		sfr	0x6a		; Pin Change Interrupt Mask 3
PCINT24		avrbit	PCMSK3,0	; Enable Pin Change Interrupt 24
PCINT25		avrbit	PCMSK3,1	; Enable Pin Change Interrupt 25
PCINT26		avrbit	PCMSK3,2	; Enable Pin Change Interrupt 26
PCINT27		avrbit	PCMSK3,3	; Enable Pin Change Interrupt 27

PCICR		sfr	0x68		; Pin Change Interrupt Control Register

PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum PCINT2_vect		; Pin Change Interrupt 2
		nextenum PCINT3_vect		; Pin Change Interrupt 3
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum TWI_vect		; 2-wire Serial Interface

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1
ISC10		avrbit	EICRA,2		; External Interrupt 1 Sense Control
ISC11		avrbit	EICRA,3

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1		avrbit	EIFR,1		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x25		; Timer/Counter 0 Control Register A
CS00		avrbit	TCCR0A,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0A,1
CS02		avrbit	TCCR0A,2
CTC0		avrbit	TCCR0A,3	; Timer/Counter 0 Clear on Compare Match Mode
TCNT0		port	0x26		; Timer/Counter 0 Value
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		sfr	0x82		; Timer/Counter 1 Control Register C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x8a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x8b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x86		; Timer/Counter 1 Input Capture LSB
ICR1H		sfr	0x87		; Timer/Counter 1 Input Capture MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Interrupt Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"

TWHSR		sfr	0xbe		; TWI High Speed Register
TWHS		avrbit	TWHSR,0		; TWI High Speed Enable

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim2c.inc"

		restore			; re-enable listing

		endif			; __regtnx8inc
