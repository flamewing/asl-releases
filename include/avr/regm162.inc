		ifndef	__regm162inc
__regm162inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM162.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega162                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data	; SRAM settings for non-161 compatible mode
RAMEND		equ	0x4ff,data
FLASHEND	label   0x3fff

		; TODO
ICE		equ	0
IVSEL		equ	1

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM1		equ	4
SE		equ	5		; sleep enable
SRW10		equ	6		; wait state select
SRE		equ	7		; enable external SRAM

MCUCSR		port	0x34		; MCU Status Register
SM2		equ	5
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

OSCCAL		port	0x04		; oscillator calibration

CLKPR		sfr	0x61		; clock prescale register
CLKPS0		equ	0		; clock prescaler select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"
		include	"avr/spm.inc"

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCSR
JTRF		equ	4		; JTAG reset occured
JTD		equ	7		; disable JTAG

OCDR		port	0x04		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE		port	0x05		; Port E @ 0x05 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1

SFIOR		port	0x30		; special function I/O Register
PUD		equ	2		; pull-up disable
XMM0		equ	3		; External Memory High Mask
XMM1		equ	4
XMM2		equ	5
XMBK		equ	6		; External Memory Bus Keeper Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum TIMER3_CAPT_vect	; timer/counter 3 capture event
		nextenum TIMER3_COMPA_vect	; timer/counter 3 compare match A
		nextenum TIMER3_COMPB_vect	; timer/counter 3 compare match B
		nextenum TIMER3_OVF_vect	; timer/counter 3 overflow
		nextenum TIMER2_COMP_vect	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI transfer complete
		nextenum USART0_RX_vect		; USART0 Rx complete
		nextenum USART1_RX_vect		; USART1 Rx complete
		nextenum USART0_UDRE_vect	; USART0 data register empty
		nextenum USART1_UDRE_vect	; USART1 data register empty
		nextenum USART0_TX_vect		; USART0 Tx complete
		nextenum USART1_TX_vect		; USART1 Tx complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; analog comparator
		nextenum SPM_READY_vect		; store program memory ready

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3

		; bits in EMCUCR
ISC2		equ	0		; external interrupt 2 sense control

GICR		port	0x3b		; General Interrupt Control Register
INT2		equ	5		; enable external interrupt 2
INT0		equ	6		; enable external interrupt 0
INT1		equ	7		; enable external interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF2		equ	5		; external Interrupt 2 occured
INTF0		equ	6		; external Interrupt 0 occured
INTF1	        equ	7		; external Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

		; bits in SFIOR
PSR310		equ	0		; prescaler reset T0/1/3
PSR2		equ	1		; ditto T2
TSM		equ	7		; timer/Counter Synchronization Mode

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM01		equ	3		; timer/counter 0 waveform generation mode
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
WGM00		equ	6
FOC0		equ	7		; timer/counter 0 force output compare match
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x31		; timer/counter 0 output compare value

TCCR1A		port	0x2f		; timer/counter 1 control register A
WGM10		equ	0		; timer/counter 1 waveform generation mode
WGM11		equ	1
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
WGM12		equ	3
WGM13		equ	4
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
WGM21		equ	3		; timer/counter 2 waveform generation mode
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
WGM20		equ	6
FOC2		equ	7		; timer/counter 2 force output compare
TCNT2		port	0x23		; timer/counter 2 value
OCR2		port	0x22		; timer/counter 2 output compare value

TCCR3A		sfr	0x8b		; timer/counter 3 control register A
WGM30		equ	0		; timer/counter 3 waveform generation mode
WGM31		equ	1
FOC3B		equ	2		; timer/counter 3 force output compare B
FOC3A		equ	3		; timer/counter 3 force output compare A
COM3B0		equ	4		; timer/counter 3 compare mode B
COM3B1		equ	5
COM3A0		equ	6		; timer/counter 3 compare mode A
COM3A1		equ	7
TCCR3B		sfr	0x8a		; timer/counter 3 control register B
CS30		equ	0		; timer/counter 3 prescaler setting
CS31		equ	1
CS32		equ	2
WGM32		equ	3
WGM33		equ	4
ICES3		equ	6		; timer/counter 3 capture slope selection
ICNC3		equ	7		; timer/counter 3 capture noise filter
TCNT3L		sfr	0x88		; timer/counter 3 value LSB
TCNT3H		sfr	0x89		; timer/counter 3 value MSB
OCR3AL		sfr	0x86		; timer/counter 3 output compare value A LSB
OCR3AH		sfr	0x87		; timer/counter 3 output compare value A MSB
OCR3BL		sfr	0x84		; timer/counter 3 output compare value B LSB
OCR3BH		sfr	0x85		; timer/counter 3 output compare value B MSB
ICR3L		sfr	0x80		; timer/counter 3 input capture value LSB
ICR3H		sfr	0x81		; timer/counter 3 input capture value MSB

TIMSK		port	0x39		; timer interrupt mask register
OCIE0		equ	0		; timer/counter 0 output compare interrupt enable
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable
TOIE2		equ	2		; timer/counter 2 overflow interrupt enable
TICIE1		equ	3		; timer/counter 1 input capture interrupt enable
OCIE2		equ	4		; timer/counter 2 output compare interrupt enable
OCIE1B		equ	5		; timer/counter 1 output compare interrupt enable A
OCIE1A		equ	6		; timer/counter 1 output compare interrupt enable B
TOIE1		equ	7		; timer/counter 1 overflow interrupt enable

ETIMSK		sfr	0x7d		; extended timer interrupt mask register
TOIE3		equ	2		; timer/counter 3 overflow interrupt enable
OCIE3B		equ	3		; timer/counter 3 output compare interrupt enable B
OCIE3A		equ	4		; timer/counter 3 output compare interrupt enable A
TICIE3		equ	5		; timer/counter 3 input capture interrupt enable

TIFR		port	0x38		; timer interrupt status register

ETIFR		sfr	0x7c		; extended interrupt status register

ASSR		port	0x26		; Asynchronous Status Register
TCR2UB		equ	0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		equ	1		; Output Compare Register 2
TCN2UB		equ	2		; Timer/Counter 2 Update Busy
AS2		equ	3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDCE		equ	4		; change enable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x0c		; UART0 I/O Data Register

UCSR0A		port	0x0b		; UART0 Control & Status Register A
MPCM0		equ	0		; UART0 multi processor communication mode
U2X0		equ	1		; UART0 double transmission speed
UPE0		equ	2		; UART0 parity error
DOR0		equ	3		; UART0 Overrun
FE0		equ	4		; UART0 Framing Error
UDRE0		equ	5		; UART0 Data Register Empty
TXC0		equ	6		; UART0 Transmit Complete
RXC0		equ	7		; UART0 Receive Complete

UCSR0B		port	0x0a		; UART0 Control & Status Register B
TXB80		equ     0		; UART0 transmit bit 8
RXB80		equ     1		; UART0 receive bit 8
UCSZ02		equ     2		; UART0 character size
TXEN0		equ     3		; UART0 enable transmitter
RXEN0		equ     4		; UART0 enable receiver
UDRIE0		equ     5		; UART0 enable data register empty interrupt
TXCIE0		equ     6		; UART0 enable transmit complete interrupt
RXCIE0		equ     7		; UART0 enable receive complete interrupt

UCSR0C		port	0x20		; UART0 Control & Status Register C
UCPOL0		equ	0		; UART0 clock polarity
UCSZ00		equ	1		; UART0 character size
UCSZ01		equ	2
USBS0		equ	3		; UART0 stop bit select
UPM00		equ	4		; UART0 parity mode : odd/even
UPM01		equ	5		; UART0 parity mode : enable/disable
UMSEL0		equ	6		; UART0 USART mode select
URSEL0		equ	7		; UART0 register select (1 for UCSR0C)

UBRR0H		port	0x20		; UART0 baud rate register high (overlayed with UCSR0C)
UBRR0L		port	0x09		; UART0 baud rate register low

UDR1		port	0x03		; UART1 I/O Data Register

UCSR1A		port	0x02		; UART1 Control & Status Register A
MPCM1		equ	0		; UART1 multi processor communication mode
U2X1		equ	1		; UART1 double transmission speed
PE1		equ	2		; UART1 parity error
DOR1		equ	3		; UART1 Overrun
FE1		equ	4		; UART1 Framing Error
UDRE1		equ	5		; UART1 Data Register Empty
TXC1		equ	6		; UART1 Transmit Complete
RXC1		equ	7		; UART1 Receive Complete

UCSR1B		port	0x01		; UART1 Control & Status Register B
TXB81		equ     0		; UART1 transmit bit 8
RXB81		equ     1		; UART1 receive bit 8
UCSZ12		equ     2		; UART1 character size
TXEN1		equ     3		; UART1 enable transmitter
RXEN1		equ     4		; UART1 enable receiver
UDRIE1		equ     5		; UART1 enable data register empty interrupt
TXCIE1		equ     6		; UART1 enable transmit complete interrupt
RXCIE1		equ     7		; UART1 enable receive complete interrupt

UCSR1C		port	0x3c		; UART1 Control & Status Register C
UCPOL1		equ	0		; UART1 clock polarity
UCSZ10		equ	1		; UART1 character size
UCSZ11		equ	2
USBS1		equ	3		; UART1 stop bit select
UPM10		equ	4		; UART1 parity mode : odd/even
UPM11		equ	5		; UART1 parity mode : enable/disable
UMSEL1		equ	6		; UART1 USART mode select
URSEL1		equ	7		; UART1 register select (1 for UCSR1C)

UBRR1H		port	0x3c		; UART1 baud rate register high (overlayed with UCSR1C)
UBRR1L		port	0x00		; UART1 baud rate register low

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm162inc
