		ifndef	__regm162inc
__regm162inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM162.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega162                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data	; SRAM Settings for non-161 Compatible Mode
RAMEND		equ	0x4ff,data
FLASHEND	label   0x3fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
SRW10		avrbit	MCUCR,6		; Wait State Select
SRE		avrbit	MCUCR,7		; Enable External SRAM

MCUCSR		port	0x34		; MCU Status Register
SM2		avrbit	MCUCSR,5
WDRF		avrbit	MCUCSR,3	; Watchdog Reset Occured
BORF		avrbit	MCUCSR,2	; Brown-Out Occured
EXTRF		avrbit	MCUCSR,1	; External Reset Occured
PORF		avrbit	MCUCSR,0	; Power-On Reset Occured

EMCUCR		port	0x36		; Extended MCU Control Register
SRW11		avrbit	EMCUCR,1	; Wait State Select
SRW00		avrbit	EMCUCR,2
SRW01		avrbit	EMCUCR,3
SRL0		avrbit	EMCUCR,4	; Wait State Sector Limit
SRL1		avrbit	EMCUCR,5
SRL2		avrbit	EMCUCR,6
SM0		avrbit	EMCUCR,7	; Sleep Mode Select

OSCCAL		port	0x04		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescale Register
CLKPS0		avrbit	CLKPR,0		; Clock Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem.inc"
		include	"avr/spm.inc"

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCSR
JTRF		avrbit	MCUCSR,4	; JTAG Reset Occured
JTD		avrbit	MCUCSR,7	; Disable JTAG

OCDR		port	0x04		; On-Chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE		port	0x05		; Port E @ 0x05 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1

SFIOR		port	0x30		; Special Function I/O Register
PUD		avrbit	SFIOR,2		; Pull-Up Disable
XMM0		avrbit	SFIOR,3		; External Memory High Mask
XMM1		avrbit	SFIOR,4
XMM2		avrbit	SFIOR,5
XMBK		avrbit	SFIOR,6		; External Memory Bus Keeper Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum PCINT1_vect		; Pin Change Interrupt Request 1
		nextenum TIMER3_CAPT_vect	; Timer/Counter 3 Capture Event
		nextenum TIMER3_COMPA_vect	; Timer/Counter 3 Compare Match A
		nextenum TIMER3_COMPB_vect	; Timer/Counter 3 Compare Match B
		nextenum TIMER3_OVF_vect	; Timer/Counter 3 Overflow
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Transfer Complete
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum SPM_READY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

ISC2		avrbit	EMCUCR,0	; External Interrupt 2 Sense Control

GICR		port	0x3b		; General Interrupt Control Register
ICE		avrbit	GICR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	GICR,1		; Interrupt Vector Select
PCIE0		avrbit	GICR,3		; Pin Change Interrupt Enable 0
PCIE1		avrbit	GICR,4		; Pin Change Interrupt Enable 1
INT2		avrbit	GICR,5		; Enable External Interrupt 2
INT0		avrbit	GICR,6		; Enable External Interrupt 0
INT1		avrbit	GICR,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
PCIF0		avrbit	GIFR,3		; Pin Change Interrupt Flag 0
PCIF1		avrbit	GIFR,4		; Pin Change Interrupt Flag 1
INTF2		avrbit	GIFR,5		; External Interrupt 2 Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1	        avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

PSR310		avrbit	SFIOR,0		; Prescaler Reset T0/1/3
PSR2		avrbit	SFIOR,1		; ditto T2
TSM		avrbit	SFIOR,7		; Timer/Counter Synchronization Mode

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
WGM01		avrbit	TCCR0,3		; Timer/Counter 0 Waveform Generation Mode
COM00		avrbit	TCCR0,4		; Timer/Counter 0 Compare Mode
COM01		avrbit	TCCR0,5
WGM00		avrbit	TCCR0,6
FOC0		avrbit	TCCR0,7		; Timer/Counter 0 Force Output Compare Match
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0		port	0x31		; Timer/Counter 0 Output Compare Value

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
WGM12		avrbit	TCCR1B,3
WGM13		avrbit	TCCR1B,4
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
WGM21		avrbit	TCCR2,3		; Timer/Counter 2 Waveform Generation Mode
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
WGM20		avrbit	TCCR2,6
FOC2		avrbit	TCCR2,7		; Timer/Counter 2 Force Output Compare
TCNT2		port	0x23		; Timer/Counter 2 Value
OCR2		port	0x22		; Timer/Counter 2 Output Compare Value

TCCR3A		sfr	0x8b		; Timer/Counter 3 Control Register A
WGM30		avrbit	TCCR3A,0	; Timer/Counter 3 Waveform Generation Mode
WGM31		avrbit	TCCR3A,1
FOC3B		avrbit	TCCR3A,2	; Timer/Counter 3 Force Output Compare B
FOC3A		avrbit	TCCR3A,3	; Timer/Counter 3 Force Output Compare A
COM3B0		avrbit	TCCR3A,4	; Timer/Counter 3 Compare Mode B
COM3B1		avrbit	TCCR3A,5
COM3A0		avrbit	TCCR3A,6	; Timer/Counter 3 Compare Mode A
COM3A1		avrbit	TCCR3A,7
TCCR3B		sfr	0x8a		; Timer/Counter 3 Control Register B
CS30		avrbit	TCCR3B,0	; Timer/Counter 3 Prescaler Setting
CS31		avrbit	TCCR3B,1
CS32		avrbit	TCCR3B,2
WGM32		avrbit	TCCR3B,3
WGM33		avrbit	TCCR3B,4
ICES3		avrbit	TCCR3B,6	; Timer/Counter 3 Capture Slope Selection
ICNC3		avrbit	TCCR3B,7	; Timer/Counter 3 Capture Noise Filter
TCNT3L		sfr	0x88		; Timer/Counter 3 Value LSB
TCNT3H		sfr	0x89		; Timer/Counter 3 Value MSB
OCR3AL		sfr	0x86		; Timer/Counter 3 Output Compare Value A LSB
OCR3AH		sfr	0x87		; Timer/Counter 3 Output Compare Value A MSB
OCR3BL		sfr	0x84		; Timer/Counter 3 Output Compare Value B LSB
OCR3BH		sfr	0x85		; Timer/Counter 3 Output Compare Value B MSB
ICR3L		sfr	0x80		; Timer/Counter 3 Input Capture Value LSB
ICR3H		sfr	0x81		; Timer/Counter 3 Input Capture Value MSB

TIMSK		port	0x39		; Timer Interrupt Mask Register
OCIE0		avrbit	TIMSK,0		; Timer/Counter 0 Output Compare Interrupt Enable
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TOIE2		avrbit	TIMSK,2		; Timer/Counter 2 Overflow Interrupt Enable
TICIE1		avrbit	TIMSK,3		; Timer/Counter 1 Input Capture Interrupt Enable
OCIE2		avrbit	TIMSK,4		; Timer/Counter 2 Output Compare Interrupt Enable
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable B
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

ETIMSK		sfr	0x7d		; Extended Timer Interrupt Mask Register
TOIE3		avrbit	ETIMSK,2	; Timer/Counter 3 Overflow Interrupt Enable
OCIE3B		avrbit	ETIMSK,3	; Timer/Counter 3 Output Compare Interrupt Enable B
OCIE3A		avrbit	ETIMSK,4	; Timer/Counter 3 Output Compare Interrupt Enable A
TICIE3		avrbit	ETIMSK,5	; Timer/Counter 3 Input Capture Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

ETIFR		sfr	0x7c		; Extended Interrupt Status Register

ASSR		port	0x26		; Asynchronous Status Register
TCR2UB		avrbit	ASSR,0		; Timer/Counter Control Register 2 Update Busy
OCR2UB		avrbit	ASSR,1		; Output Compare Register 2
TCN2UB		avrbit	ASSR,2		; Timer/Counter 2 Update Busy
AS2		avrbit	ASSR,3		; Asynchronous Timer/Counter 2

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"
WDCE		avrbit	WDTCR,4		; Change Enable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x0c		; UART0 I/O Data Register

UCSR0A		port	0x0b		; UART0 Control & Status Register A
MPCM0		avrbit	UCSR0A,0	; UART0 Multi Processor Communication Mode
U2X0		avrbit	UCSR0A,1	; UART0 Double Transmission Speed
UPE0		avrbit	UCSR0A,2	; UART0 Parity Error
DOR0		avrbit	UCSR0A,3	; UART0 Overrun
FE0		avrbit	UCSR0A,4	; UART0 Framing Error
UDRE0		avrbit	UCSR0A,5	; UART0 Data Register Empty
TXC0		avrbit	UCSR0A,6	; UART0 Transmit Complete
RXC0		avrbit	UCSR0A,7	; UART0 Receive Complete

UCSR0B		port	0x0a		; UART0 Control & Status Register B
TXB80		avrbit	UCSR0B,0	; UART0 Transmit Bit 8
RXB80		avrbit	UCSR0B,1	; UART0 Receive Bit 8
UCSZ02		avrbit	UCSR0B,2	; UART0 Character Size
TXEN0		avrbit	UCSR0B,3	; UART0 Enable Transmitter
RXEN0		avrbit	UCSR0B,4	; UART0 Enable Receiver
UDRIE0		avrbit	UCSR0B,5	; UART0 Enable Data Register Empty Interrupt
TXCIE0		avrbit	UCSR0B,6	; UART0 Enable Transmit Complete Interrupt
RXCIE0		avrbit	UCSR0B,7	; UART0 Enable Receive Complete Interrupt

UCSR0C		port	0x20		; UART0 Control & Status Register C
UCPOL0		avrbit	UCSR0C,0	; UART0 Clock Polarity
UCSZ00		avrbit	UCSR0C,1	; UART0 Character Size
UCSZ01		avrbit	UCSR0C,2
USBS0		avrbit	UCSR0C,3	; UART0 Stop Bit Select
UPM00		avrbit	UCSR0C,4	; UART0 Parity Mode : Odd/Even
UPM01		avrbit	UCSR0C,5	; UART0 Parity Mode : Enable/Disable
UMSEL0		avrbit	UCSR0C,6	; UART0 USART Mode Select
URSEL0		avrbit	UCSR0C,7	; UART0 Register Select (1 for UCSR0C)

UBRR0H		port	0x20		; UART0 Baud Rate Register MSB (overlayed with UCSR0C)
UBRR0L		port	0x09		; UART0 Baud Rate Register LSB

UDR1		port	0x03		; UART1 I/O Data Register

UCSR1A		port	0x02		; UART1 Control & Status Register A
MPCM1		avrbit	UCSR1A,0	; UART1 Multi Processor Communication Mode
U2X1		avrbit	UCSR1A,1	; UART1 Double Transmission Speed
UPE1		avrbit	UCSR1A,2	; UART1 Parity Error
DOR1		avrbit	UCSR1A,3	; UART1 Overrun
FE1		avrbit	UCSR1A,4	; UART1 Framing Error
UDRE1		avrbit	UCSR1A,5	; UART1 Data Register Empty
TXC1		avrbit	UCSR1A,6	; UART1 Transmit Complete
RXC1		avrbit	UCSR1A,7	; UART1 Receive Complete

UCSR1B		port	0x01		; UART1 Control & Status Register B
TXB81		avrbit	UCSR1B,0	; UART1 Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; UART1 Receive Bit 8
UCSZ12		avrbit	UCSR1B,2	; UART1 Character Size
TXEN1		avrbit	UCSR1B,3	; UART1 Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; UART1 Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; UART1 Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; UART1 Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; UART1 Enable Receive Complete Interrupt

UCSR1C		port	0x3c		; UART1 Control & Status Register C
UCPOL1		avrbit	UCSR1C,0	; UART1 Clock Polarity
UCSZ10		avrbit	UCSR1C,1	; UART1 Character Size
UCSZ11		avrbit	UCSR1C,2
USBS1		avrbit	UCSR1C,3	; UART1 Stop Bit Select
UPM10		avrbit	UCSR1C,4	; UART1 Parity Mode : Odd/Even
UPM11		avrbit	UCSR1C,5	; UART1 Parity Mode : Enable/Disable
UMSEL1		avrbit	UCSR1C,6	; UART1 USART Mode Select
URSEL1		avrbit	UCSR1C,7	; UART1 Register Select (1 for UCSR1C)

UBRR1H		port	0x3c		; UART1 Baud Rate Register MSB (overlayed with UCSR1C)
UBRR1L		port	0x00		; UART1 Baud Rate Register LSB

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

		restore			; re-enable listing

		endif			; __regm162inc
