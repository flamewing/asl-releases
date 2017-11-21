		ifndef	__regm161inc
__regm161inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM161.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega161                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x60,data
RAMEND		equ	0x45f,data
FLASHEND	label	0x3fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM1		equ	4
SE		equ	5		; sleep enable
SRW10		equ	6		; wait state select
SRE		equ	7		; enable external SRAM

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; watchdog reset occured
BORF		equ	2		; brown-out occured
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

EMCUCR		port	0x36		; Extended MCU Control Register
SRW11		equ	1		; wait state select
SRW00		equ	2
SRW01		equ	3
SRL0		equ	4		; wait state sector limit
SRL1		equ	5
SRL2		equ	6
SM0		equ	7		; sleep mode select

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"

SPMCR		port	0x37		; Store Program Memory Control Register
LBSET		equ	3		; lock bit set
PGWRT		equ	2		; page write
PGERS		equ	1		; page erase
SPMEN		equ	0		; store program memory enable

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE		port	0x05		; Port E @ 0x05 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum TIMER2_COMP_vect 	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI transfer complete
		nextenum UART0_RX_vect		; UART0 Rx complete
		nextenum UART1_RX_vect		; UART1 Rx complete
		nextenum UART0_UDRE_vect	; UART0 data register empty
		nextenum UART1_UDRE_vect	; UART1 data register empty
		nextenum UART0_TX_vect		; UART0 Tx complete
		nextenum UART1_TX_vect		; UART1 Tx complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

		; bits in EMCUCR
ISC2		equ	0		; external interrupt 2 sense control

GIMSK		port	0x3b		; General Interrupt Mask Register
INT2		equ	5		; enable external interrupt 2
INT0		equ	6		; enable external interrupt 0
INT1		equ	7		; enable external interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags:
INTF2		equ	5		; external Interrupt 2 occured
INTF0		equ	6		; external Interrupt 0 occured
INTF1	        equ	7		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

SFIOR		port	0x30		; special function I/O Register
PSR10		equ	0		; prescaler reset T0/1
PSR2		equ	1		; ditto T2

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
CTC0		equ	3		; timer/counter 0 clear on compare match
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
PWM0		equ	6		; timer/counter 0 PWM enable
FOC0		equ	7		; timer/counter 0 force output compare match
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x31		; timer/counter 0 output compare value

TCCR1A		port	0x2f		; timer/counter 1 control register A
PWM10		equ	0		; timer/counter 1 PWM mode
PWM11		equ	1
FOC1B		equ	2		; timer/counter 1 force output compare B
FOC1A		equ	3		; timer/counter 1 force output compare A
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
CTC1		equ	3		; timer/counter 1 clear on compare match
ICES1		equ	6		; timer/counter 1 capture slope selection
ICNC1		equ	7		; timer/counter 1 capture noise filter
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB
OCR1AL		port	0x2a		; timer/counter 1 output compare value A LSB
OCR1AH		port	0x2b		; timer/counter 1 output compare value A MSB
OCR1BL		port	0x28		; timer/counter 1 output compare value B LSB
OCR1BH		port	0x29		; timer/counter 1 output compare value B MSB
ICR1L		port	0x24		; timer/counter 1 input capture value LSB
ICR1H		port	0x25		; timer/counter 1 input capture value MSB

TCCR2		port	0x27		; timer/counter 2 control register
CS20		equ	0		; timer/counter 2 prescaler setting
CS21		equ	1
CS22		equ	2
CTC2		equ	3		; timer/counter 2 clear on compare match
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
PWM2		equ	6		; timer/counter 2 PWM enable
FOC2		equ	7		; timer/counter 2 force output compare
TCNT2		port	0x23		; timer/counter 2 value
OCR2		port	0x22		; timer/counter 2 output compare value

TIMSK		port	0x39		; timer interrupt mask register
OCIE0		equ	0		; timer/counter 0 output compare interrupt enable
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
OCIE2		equ	2		; timer/counter 2 output compare interrupt enable
TICIE1		equ	3		; timer/counter 1 input capture interrupt enable
TOIE2		equ	4		; timer/counter 2 overflow interrupt enable
OCIE1B		equ	5		; timer/counter 1 output compare interrupt enable A
OCIE1A		equ	6		; timer/counter 1 output compare interrupt enable B
TOIE1		equ	7		; timer/counter 1 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

ASSR		port	0x26		; Asynchronous Status Register
TCR2UB		equ	0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		equ	1		; Output Compare Register 2
TCN2UB		equ	2		; Timer/Counter 2 Update Busy
AS2		equ	3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		equ	4		; turn-off enable

;----------------------------------------------------------------------------
; UART

UDR0		port	0x0c		; UART0 I/O Data Register

UCSR0A		port	0x0b		; UART0 Control/Status Register A
OR0		equ	3		; UART0 Overrun
FE0		equ	4		; UART0 Framing Error
UDRE0		equ	5		; UART0 Data Register Empty
TXC0		equ	6		; UART0 Transmit Complete
RXC0		equ	7		; UART0 Receive Complete

UCSR0B		port	0x0a		; UART0 Control/Status Register B
TXB80		equ     0		; UART0 transmit bit 8
RXB80		equ     1		; UART0 receive bit 8
CHR90		equ     2		; UART0 9 bit characters
TXEN0		equ     3		; UART0 enable transmitter
RXEN0		equ     4		; UART0 enable receiver
UDRIE0		equ     5		; UART0 enable data register empty interrupt
TXCIE0		equ     6		; UART0 enable transmit complete interrupt
RXCIE0		equ     7		; UART0 enable receive complete interrupt

UBRR0		port	0x09		; UART0 baud rate register

UDR1		port	0x03		; UART1 I/O Data Register

UCSR1A		port	0x02		; UART1 Control/Status Register
OR1		equ	3		; UART1 Overrun
FE1		equ	4		; UART1 Framing Error
UDRE1		equ	5		; UART1 Data Register Empty
TXC1		equ	6		; UART1 Transmit Complete
RXC1		equ	7		; UART1 Receive Complete

UCSR1B		port	0x01		; UART1 Control/Status Register
TXB81		equ     0		; UART1 transmit bit 8
RXB81		equ     1		; UART1 receive bit 8
CHR91		equ     2		; UART1 9 bit characters
TXEN1		equ     3		; UART1 enable transmitter
RXEN1		equ     4		; UART1 enable receiver
UDRIE1		equ     5		; UART1 enable data register empty interrupt
TXCIE1		equ     6		; UART1 enable transmit complete interrupt
RXCIE1		equ     7		; UART1 enable receive complete interrupt

UBRR1		port	0x00		; UART1 baud rate register

UBRRHI		port	0x20		; UART0/1 upper MSB baud rate register

;----------------------------------------------------------------------------
; SPI

		include "avr/spim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm2.inc"

		restore			; re-enable listing

		endif			; __regm161inc
