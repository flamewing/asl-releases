		ifndef	__regxx33inc
__regxx33inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGXX33.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90S2233/4433                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		equ	4		; choose idle/power down mode
SE		equ	5		; enable sleep mode

MCUSR		port	0x34		; MCU General Status Register
WDRF		equ	3		; Watchdog reset occured
BORF		equ	2		; Brown-Out reset occured
EXTRF		equ	1		; External reset ocuured
PORF		equ	0		; Power-on reset occured


;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

		; bits in EECR
EEMWE		equ	 2		; EEPROM Master Write Enable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMP_vect	; timer/counter 1 compare match
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum UART_RX_vect		; UART Rx complete
		nextenum UART_UDRE_vect		; UART data register empty
		nextenum UART_TX_vect		; UART Tx complete
		nextenum ADC_vect		; A/D converter
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ     6		;  enable external Interrupt 0
INT1		equ	7		;  enable external Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		equ	6		;  external Interrupt 0 occured
INTF1		equ	7		;  external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; clock select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; timer/counter 0 value

TCCR1A		port	0x2f		; timer/counter 1 control register A
PWM10		equ	0		; mode of pulse width modulator
PWM11		equ	1
COM10		equ	6		; timer/counter 1 compare mode
COM11		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; prescaler setting
CS11		equ	1
CS12		equ	2
CTC1		equ	3		; clear after equality?
ICES1		equ	6		; capture slope selection
ICNC1		equ	7		; capture noise filter
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB
OCR1L		port	0x2a		; timer/counter 1 output compare value LSB
OCR1H		port	0x2b		; timer/counter 1 output compare value MSB
ICR1L		port	0x27		; timer/counter 1 input capture value LSB
ICR1H		port	0x26		; timer/counter 1 input capture value MSB

TIMSK		port	0x39		; timer mask register
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
TICIE1		equ	3		; timer/counter 1 input capture interrupt enable
OCIE1		equ	6		; timer/counter 1 output compare interrupt enable
TOIE1		equ	7		; timer/counter 1 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; UART

UDR		port	0x0c		; UART I/O Data Register

UCSRA		port	0x0b		; UART Control and Status Register A
MPCM		equ	0		; Multi-processor Communication Mode
OR		equ	3		; Overrun
FE		equ	4		; Framing Error
UDRE		equ	5		; Data Register Empty
TXC		equ	6		; Transmit Complete
RXC		equ	7		; Receive Complete

UCSRB		port	0x0a		; UART Control and Status Register B
TXB8		equ     0		; transmit bit 8
RXB8		equ     1		; receive bit 8
CHR9		equ     2		; 9 bit characters
TXEN		equ     3		; enable transmitter
RXEN		equ     4		; enable receiver
UDRIE		equ     5		; enable data register empty interrupt
TXCIE		equ     6		; enable transmit complete interrupt
RXCIE		equ     7		; enable receive complete interrupt

UBRR		port	0x09		; baud rate register
UBRRHI		port	0x03		; baud rate register hi

;----------------------------------------------------------------------------
; SPI

		include	"avr/spi90.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		; bits in ACSR
AINBG		equ	6		; AINBG: Analog Comparator Bandgap Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
ADCBG		equ	6		; ADC Bandgap Select
MUX2		equ	2		; Analog Channel Select Bits
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; control/status register
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

		restore

		endif			; __regxx33inc
