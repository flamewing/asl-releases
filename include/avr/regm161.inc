		ifndef	__regm161inc
__regm161inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM161.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega161                      *
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
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
SRW10		avrbit	MCUCR,6		; Wait State Select
SRE		avrbit	MCUCR,7		; Enable External SRAM

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out ResetOccured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured

EMCUCR		port	0x36		; Extended MCU Control Register
SRW11		avrbit	EMCUCR,1	; Wait State Select
SRW00		avrbit	EMCUCR,2
SRW01		avrbit	EMCUCR,3
SRL0		avrbit	EMCUCR,4	; Wait State Sector Limit
SRL1		avrbit	EMCUCR,5
SRL2		avrbit	EMCUCR,6
SM0		avrbit	EMCUCR,7	; Sleep Mode Select

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"

SPMCR		port	0x37		; Store Program Memory Control Register
LBSET		avrbit	SPMCR,3		; Lock Bit Set
PGWRT		avrbit	SPMCR,2		; Page Write
PGERS		avrbit	SPMCR,1		; Page Erase
SPMEN		avrbit	SPMCR,0		; Store Program Memory Enable

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
		enum	 INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum TIMER2_COMP_vect 	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Transfer Complete
		nextenum UART0_RX_vect		; UART0 Rx Complete
		nextenum UART1_RX_vect		; UART1 Rx Complete
		nextenum UART0_UDRE_vect	; UART0 Data Register Empty
		nextenum UART1_UDRE_vect	; UART1 Data Register Empty
		nextenum UART0_TX_vect		; UART0 Tx Complete
		nextenum UART1_TX_vect		; UART1 Tx Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

ISC2		avrbit	EMCUCR,0	; External Interrupt 2 Sense Control

GIMSK		port	0x3b		; General Interrupt Mask Register
INT2		avrbit	GIMSK,5		; Enable External Interrupt 2
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags:
INTF2		avrbit	GIFR,5		; External Interrupt 2 Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1	        avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

SFIOR		port	0x30		; Special Function I/O Register
PSR10		avrbit	SFIOR,0		; Prescaler Reset T0/1
PSR2		avrbit	SFIOR,1		; Prescaler Reset T2

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
CTC0		avrbit	TCCR0,3		; Timer/Counter 0 Clear on Compare Match
COM00		avrbit	TCCR0,4		; Timer/Counter 0 Compare Mode
COM01		avrbit	TCCR0,5
PWM0		avrbit	TCCR0,6		; Timer/Counter 0 PWM Enable
FOC0		avrbit	TCCR0,7		; Timer/Counter 0 Force Output Compare Match
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0		port	0x31		; Timer/Counter 0 Output Compare Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Timer/Counter 1 PWM Mode
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
CTC1		avrbit	TCCR1B,3	; Timer/Counter 1 Clear on Compare Match
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture Value MSB

TCCR2		port	0x27		; Timer/Counter 2 Control Register
CS20		avrbit	TCCR2,0		; Timer/Counter 2 Prescaler Setting
CS21		avrbit	TCCR2,1
CS22		avrbit	TCCR2,2
CTC2		avrbit	TCCR2,3		; Timer/Counter 2 Clear on Compare Match
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
PWM2		avrbit	TCCR2,6		; Timer/Counter 2 PWM Enable
FOC2		avrbit	TCCR2,7		; Timer/Counter 2 Force Output Compare
TCNT2		port	0x23		; Timer/Counter 2 Value
OCR2		port	0x22		; Timer/Counter 2 Output Compare Value

TIMSK		port	0x39		; Timer Interrupt Mask Register
OCIE0		avrbit	TIMSK,0		; Timer/Counter 0 Output Compare Interrupt Enable
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,2		; Timer/Counter 2 Output Compare Interrupt Enable
TICIE1		avrbit	TIMSK,3		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,4		; Timer/Counter 2 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable B
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

ASSR		port	0x26		; Asynchronous Status Register
TCR2UB		avrbit	ASSR,0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		avrbit	ASSR,1		; Output Compare Register 2
TCN2UB		avrbit	ASSR,2		; Timer/Counter 2 Update Busy
AS2		avrbit	ASSR,3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; UART

UDR0		port	0x0c		; UART0 I/O Data Register

UCSR0A		port	0x0b		; UART0 Control/Status Register A
OR0		avrbit	UCSR0A,3	; UART0 Overrun
FE0		avrbit	UCSR0A,4	; UART0 Framing Error
UDRE0		avrbit	UCSR0A,5	; UART0 Data Register Empty
TXC0		avrbit	UCSR0A,6	; UART0 Transmit Complete
RXC0		avrbit	UCSR0A,7	; UART0 Receive Complete

UCSR0B		port	0x0a		; UART0 Control/Status Register B
TXB80		avrbit	UCSR0B,0	; UART0 Transmit Bit 8
RXB80		avrbit	UCSR0B,1	; UART0 Receive Bit 8
CHR90		avrbit	UCSR0B,2	; UART0 9 Bit Characters
TXEN0		avrbit	UCSR0B,3	; UART0 Enable Transmitter
RXEN0		avrbit	UCSR0B,4	; UART0 Enable Receiver
UDRIE0		avrbit	UCSR0B,5	; UART0 Enable Data Register Empty Interrupt
TXCIE0		avrbit	UCSR0B,6	; UART0 Enable Transmit Complete Interrupt
RXCIE0		avrbit	UCSR0B,7	; UART0 Enable Receive Complete Interrupt

UBRR0		port	0x09		; UART0 Baud Rate Register LSB

UDR1		port	0x03		; UART1 I/O Data Register

UCSR1A		port	0x02		; UART1 Control/Status Register
OR1		avrbit	UCSR1A,3	; UART1 Overrun
FE1		avrbit	UCSR1A,4	; UART1 Framing Error
UDRE1		avrbit	UCSR1A,5	; UART1 Data Register Empty
TXC1		avrbit	UCSR1A,6	; UART1 Transmit Complete
RXC1		avrbit	UCSR1A,7	; UART1 Receive Complete

UCSR1B		port	0x01		; UART1 Control/Status Register
TXB81		avrbit	UCSR1B,0	; UART1 Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; UART1 Receive Bit 8
CHR91		avrbit	UCSR1B,2	; UART1 9 Bit Characters
TXEN1		avrbit	UCSR1B,3	; UART1 Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; UART1 Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; UART1 Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; UART1 Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; UART1 Enable Receive Complete Interrupt

UBRR1		port	0x00		; UART1 Baud Rate Register LSB

UBRRHI		port	0x20		; UART0/1 Baud Rate Register MSB

;----------------------------------------------------------------------------
; SPI

		include "avr/spim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm2.inc"

		restore			; re-enable listing

		endif			; __regm161inc
