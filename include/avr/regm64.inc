		ifndef	__regm64inc
__regm64inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM64.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATmega64                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	2047
RAMSTART	equ	0x100,data
RAMEND		equ	0x10ff,data
FLASHEND	label	0xffff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select
SM2		equ	2		; sleep mode select
SM0		equ	3
SM1		equ	4
SE		equ	5		; sleep enable
SRW10		equ	6		; wait state select
SRE		equ	7		; enable external SRAM

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		equ	3		; watchdog reset occured
BORF		equ	2		; brown-out occured
EXTRF		equ	1		; external reset occured
PORF		equ	0		; power-on reset occured

OSCCAL		sfr	0x6f		; oscillator calibration

XDIV		port	0x3c		; XTAL Divide Control Register

XMCRA		sfr	0x6d		; External Memory Control Register A
SRL2		equ	6		; Wait State Sector Limit
SRL1		equ	5
SRL0		equ	4
SRW01		equ	3		; Wait State Select Bits for Lower Sector
SRW00		equ	2
SRW11		equ	1

XMCRB		sfr	0x6c		; External Memory Control Register B
XMBK		equ	7		; External Memory Bus Keeper Enable
XMM2		equ	2		; External Memory High Mask
XMM1		equ	1
XMM0		equ	0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"
		include	"avr/spmcsr68.inc"

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCSR
JTRF		equ	4		; JTAG reset occured
JTD		equ	7		; JTAG disable

OCDR		port	0x22		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port    0x10		; Port D @ 0x10 (IO) ff.
PINE		port	0x01		; Port E @ 0x01 (IO) ff.
PINF            port	0x00		; Port F @ 0x00 (IO) ff.,
DDRF		sfr	0x61		; registers non-continuous
PORTF		sfr	0x62
PING		sfr	0x63		; Port G @ 0x63 (DATA) ff.

SFIOR		port	0x20		; special function I/O Register
PUD		equ	2		; pullup disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum INT3_vect		; external interrupt request 3
		nextenum INT4_vect		; external interrupt request 4
		nextenum INT5_vect		; external interrupt request 5
		nextenum INT6_vect		; external interrupt request 6
		nextenum INT7_vect		; external interrupt request 7
		nextenum TIMER2_COMP_vect	; timer/counter 2 compare match
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMP_vect	; timer/counter 0 compare match
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART0_RX_vect		; USART0 Rx complete
		nextenum USART0_UDRE_vect	; USART0 data register empty
		nextenum USART0_TX_vect		; USART0 Tx complete
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum ANALOG_COMP_vect	; analog comparator
		nextenum TIMER1_COMPC_vect	; timer/counter 1 compare match C
		nextenum TIMER3_CAPT_vect	; timer/counter 3 capture event
		nextenum TIMER3_COMPA_vect	; timer/counter 3 compare match A
		nextenum TIMER3_COMPB_vect	; timer/counter 3 compare match B
		nextenum TIMER3_COMPC_vect	; timer/counter 3 compare match C
		nextenum TIMER3_OVF_vect	; timer/counter 3 overflow
		nextenum USART1_RX_vect		; USART1 Rx complete
		nextenum USART1_UDRE_vect	; USART1 data register empty
		nextenum USART1_TX_vect		; USART1 Tx complete
		nextenum TWI_vect		; two-wire serial interface
		nextenum SPM_READY_vect		; store program memory ready

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x6a		; External Interrupt Control Register A
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3
ISC20		equ	4		; external interrupt 2 sense control
ISC21		equ	5
ISC30		equ	6		; external interrupt 3 sense control
ISC31		equ	7

EICRB		port	0x3a		; External Interrupt Control Register B
ISC40		equ	0		; external interrupt 4 sense control
ISC41		equ	1
ISC50		equ	2		; external interrupt 5 sense control
ISC51		equ	3
ISC60		equ	4		; external interrupt 6 sense control
ISC61		equ	5
ISC70		equ	6		; external interrupt 7 sense control
ISC71		equ	7

EIMSK		port	0x39		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1
INT2		equ	2		; enable external interrupt 2
INT3		equ	3		; enable external interrupt 3
INT4		equ	4		; enable external interrupt 4
INT5		equ	5		; enable external interrupt 5
INT6		equ	6		; enable external interrupt 6
INT7		equ	7		; enable external interrupt 7

EIFR		port	0x38		; External Interrupt Flags Register
INTF0		equ	0		; external Interrupt 0 occured
INTF1	        equ	1		; external Interrupt 1 occured
INTF2		equ	2		; external Interrupt 2 occured
INTF3		equ	3		; external Interrupt 3 occured
INTF4	        equ	4		; external Interrupt 4 occured
INTF5		equ	5		; external Interrupt 5 occured
INTF6		equ	6		; external Interrupt 6 occured
INTF7	        equ	7		; external Interrupt 7 occured

;----------------------------------------------------------------------------
; Timers

		; bits in SFIOR
TSM		equ	7		; timer syncronization
PSR321		equ	0		; prescaler reset T1..3
PSR0		equ	1		; ditto T0

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM01		equ	3
COM00		equ	4		; timer/counter 0 compare mode
COM01		equ	5
WGM00		equ	6		; timer/counter 0 waveform generation mode
FOC0		equ	7		; timer/counter 0 force output compare match
TCNT0		port	0x32		; timer/counter 0 value
OCR0		port	0x31		; timer/counter 0 output compare value

TCCR1A		port	0x2f		; timer/counter 1 control register A
WGM10		equ	0		; timer/counter 1 waveform generation mode
WGM11		equ	1
COM1C0		equ	2		; timer/counter 1 compare mode C
COM1C1		equ	3
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
WGM12		equ	3		; timer/counter 1 waveform generation mode
WGM13		equ	4
ICES1		equ	6		; timer/counter 1 capture slope selection
ICNC1		equ	7		; timer/counter 1 capture noise filter
TCCR1C		sfr	0x7a		; timer/counter 1 control register C
FOC1C		equ	5		; timer/counter 1 force output compare match C
FOC1B		equ	6		; timer/counter 1 force output compare match B
FOC1A		equ	7		; timer/counter 1 force output compare match A
TCNT1L		port	0x2c		; timer/counter 1 value LSB
TCNT1H		port	0x2d		; timer/counter 1 value MSB
OCR1AL		port	0x2a		; timer/counter 1 output compare value A LSB
OCR1AH		port	0x2b		; timer/counter 1 output compare value A MSB
OCR1BL		port	0x28		; timer/counter 1 output compare value B LSB
OCR1BH		port	0x29		; timer/counter 1 output compare value B MSB
OCR1CL		sfr	0x78		; timer/counter 1 output compare value C LSB
OCR1CH		sfr	0x79		; timer/counter 1 output compare value C MSB
ICR1L		port	0x26		; timer/counter 1 input capture value LSB
ICR1H		port	0x27		; timer/counter 1 input capture value MSB

TCCR2		port	0x25		; timer/counter 2 control register
CS20		equ	0		; timer/counter 2  prescaler setting
CS21		equ	1
CS22		equ	2
WGM21		equ	3
COM20		equ	4		; timer/counter 2 compare mode
COM21		equ	5
WGM20		equ	6		; timer/counter 2 waveform generation mode
FOC2		equ	7		; timer/counter 2 force output compare
TCNT2		port	0x24		; timer/counter 2 value
OCR2		port	0x23		; timer/counter 2 output compare value

TCCR3A		sfr	0x8b		; timer/counter 3 control register A
WGM30		equ	0		; timer/counter 3 waveform generation mode
WGM31		equ	1
COM3C0		equ	2		; timer/counter 3 compare mode C
COM3C1		equ	3
COM3B0		equ	4		; timer/counter 3 compare mode B
COM3B1		equ	5
COM3A0		equ	6		; timer/counter 3 compare mode A
COM3A1		equ	7
TCCR3B		sfr	0x8a		; timer/counter 3 control register B
CS30		equ	0		; timer/counter 3 prescaler setting
CS31		equ	1
CS32		equ	2
WGM32		equ	3		; timer/counter 3 waveform generation mode
WGM33		equ	4
ICES3		equ	6		; timer/counter 3 capture slope selection
ICNC3		equ	7		; timer/counter 3 capture noise filter
TCCR3C		sfr	0x8c		; timer/counter 3 control register C
FOC3C		equ	5		; timer/counter 3 force output compare match C
FOC3B		equ	6		; timer/counter 3 force output compare match B
FOC3A		equ	7		; timer/counter 3 force output compare match A
TCNT3L		sfr	0x88		; timer/counter 3 value LSB
TCNT3H		sfr	0x89		; timer/counter 3 value MSB
OCR3AL		sfr	0x86		; timer/counter 3 output compare value A LSB
OCR3AH		sfr	0x87		; timer/counter 3 output compare value A MSB
OCR3BL		sfr	0x84		; timer/counter 3 output compare value B LSB
OCR3BH		sfr	0x85		; timer/counter 3 output compare value B MSB
OCR3CL		sfr	0x82		; timer/counter 3 output compare value C LSB
OCR3CH		sfr	0x83		; timer/counter 3 output compare value C MSB
ICR3L		sfr	0x80		; timer/counter 3 input capture value LSB
ICR3H		sfr	0x81		; timer/counter 3 input capture value MSB

TIMSK		port	0x37		; timer mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0		equ	1		; timer/counter 0 output compare interrupt enable
TOIE1		equ	2		; timer/counter 1 overflow interrupt enable
OCIE1B		equ	3		; timer/counter 1 output compare interrupt enable B
OCIE1A		equ	4		; timer/counter 1 output compare interrupt enable A
TICIE1		equ	5		; timer/counter 1 input capture interrupt enable
TOIE2		equ	6		; timer/counter 2 overflow interrupt enable
OCIE2		equ	7		; timer/counter 2 output compare interrupt enable

ETIMSK		sfr	0x7d		; extended timer mask register
OCIE1C		equ	0		; timer/counter 1 output compare interrupt enable C
OCIE3C		equ	1		; timer/counter 3 output compare interrupt enable C
TOIE3		equ	2		; timer/counter 3 overflow interrupt enable
OCIE3B		equ	3		; timer/counter 3 output compare interrupt enable B
OCIE3A		equ	4		; timer/counter 3 output compare interrupt enable A
TICIE3		equ	5		; timer/counter 3 input capture interrupt enable

TIFR		port	0x36		; timer interrupt status register
ETIFR		sfr	0x7c		; extended timer interrupt status register

ASSR		port	0x30		; Asynchronous Status Register
TCR0UB		equ	0		; Timer/Counter Control Register 0 Update Busy
OCR0UB		equ	1		; Output Compare Register 0
TCN0UB		equ	2		; Timer/Counter 0 Update Busy
AS0		equ	3		; Asynchronous Timer/Counter 0

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDCE		equ	4		; change enable

;----------------------------------------------------------------------------
; U(S)ART

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

UCSR0C		sfr	0x95		; UART0 Control & Status Register C
UCPOL0		equ	0		; UART0 clock polarity
UCSZ00		equ	1		; UART0 character size
UCSZ01		equ	2
USBS0		equ	3		; UART0 stop bit select
UPM00		equ	4		; UART0 parity mode : odd/even
UPM01		equ	5		; UART0 parity mode : enable/disable
UMSEL0		equ	6		; UART0 USART mode select
URSEL0		equ	7		; UART0 register select (1 for UCSRC)

UBRR0H		sfr	0x90		; UART0 baud rate register high (overlayed with UCSRC)
UBRR0L		port	0x09		; UART0 baud rate register low

UDR1		sfr	0x9c		; UART1 I/O Data Register

UCSR1A		sfr	0x9b		; UART1 Control & Status Register A
MPCM1		equ	0		; UART1 multi processor communication mode
U2X1		equ	1		; UART1 double transmission speed
UPE1		equ	2		; UART1 parity error
DOR1		equ	3		; UART1 Overrun
FE1		equ	4		; UART1 Framing Error
UDRE1		equ	5		; UART1 Data Register Empty
TXC1		equ	6		; UART1 Transmit Complete
RXC1		equ	7		; UART1 Receive Complete

UCSR1B		sfr	0x9a		; UART1 Control & Status Register B
TXB81		equ     0		; UART1 transmit bit 8
RXB81		equ     1		; UART1 receive bit 8
UCSZ12		equ     2		; UART1 character size
TXEN1		equ     3		; UART1 enable transmitter
RXEN1		equ     4		; UART1 enable receiver
UDRIE1		equ     5		; UART1 enable data register empty interrupt
TXCIE1		equ     6		; UART1 enable transmit complete interrupt
RXCIE1		equ     7		; UART1 enable receive complete interrupt

UCSR1C		sfr	0x9d		; UART1 Control & Status Register C
UCPOL1		equ	0		; UART1 clock polarity
UCSZ10		equ	1		; UART1 character size
UCSZ11		equ	2
USBS1		equ	3		; UART1 stop bit select
UPM10		equ	4		; UART1 parity mode : odd/even
UPM11		equ	5		; UART1 parity mode : enable/disable
UMSEL1		equ	6		; UART1 USART mode select

UBRR1H		sfr	0x98		; UART1 baud rate register high (overlayed with UCSRC)
UBRR1L		sfr	0x99		; UART1 baud rate register low

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twim70.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm16.inc"

ADCSRB		sfr	0x8e		; ADC Control/Status Register B
ADTS0		equ	0		; auto trigger source
ADTS1		equ	1
ADTS2		equ	2

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm64inc
