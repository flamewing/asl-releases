		ifndef	__regxx33inc
__regxx33inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGXX33.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S2233/4433                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Power Down Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

MCUSR		port	0x34		; MCU General Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out Reset Occured
EXTRF		avrbit	MCUSR,1		; External Reset Ocuured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured


;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

EEMWE		avrbit	EECR,2		; EEPROM Master Write Enable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
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
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum UART_RX_vect		; UART Rx Complete
		nextenum UART_UDRE_vect		; UART Data Register Empty
		nextenum UART_TX_vect		; UART Tx Complete
		nextenum ADC_vect		; A/D Converter
		nextenum EE_RDY_vect		; EEPROM Ready
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
COM10		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode
COM11		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Clear after Equality?
ICES1		avrbit	TCCR1B,6	; Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1L		port	0x2a		; Timer/Counter 1 Output Compare Value LSB
OCR1H		port	0x2b		; Timer/Counter 1 Output Compare Value MSB
ICR1L		port	0x27		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x26		; Timer/Counter 1 Input Capture Value MSB

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TICIE1		avrbit	TIMSK,3		; Timer/Counter 1 Input Capture Interrupt Enable
OCIE1		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; UART

UDR		port	0x0c		; UART I/O Data Register

UCSRA		port	0x0b		; UART Control and Status Register A
MPCM		avrbit	UCSRA,0		; Multi-Processor Communication Mode
OR		avrbit	UCSRA,3		; Overrun
FE		avrbit	UCSRA,4		; Framing Error
UDRE		avrbit	UCSRA,5		; Data Register Empty
TXC		avrbit	UCSRA,6		; Transmit Complete
RXC		avrbit	UCSRA,7		; Receive Complete

UCSRB		port	0x0a		; UART Control and Status Register B
TXB8		avrbit	UCSRB,0		; Transmit Bit 8
RXB8		avrbit	UCSRB,1		; Receive Bit 8
CHR9		avrbit	UCSRB,2		; 9 Bit Characters
TXEN		avrbit	UCSRB,3		; Enable Transmitter
RXEN		avrbit	UCSRB,4		; Enable Receiver
UDRIE		avrbit	UCSRB,5		; Enable Data Register Empty Interrupt
TXCIE		avrbit	UCSRB,6		; Enable Transmit Complete Interrupt
RXCIE		avrbit	UCSRB,7		; Enable Receive Complete Interrupt

UBRR		port	0x09		; Baud Rate Register LSB
UBRRHI		port	0x03		; Baud Rate Register MSB

;----------------------------------------------------------------------------
; SPI

		include	"avr/spi90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

AINBG		avrbit	ACSR,6		; AINBG: Analog Comparator Bandgap Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
ADCBG		avrbit	ADMUX,6		; ADC Bandgap Select
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

		endif			; __regxx33inc
