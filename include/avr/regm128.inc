		ifndef	__regm128inc
__regm128inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM128.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega128                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	4095
RAMSTART	equ	0x100,data	; Memory Config A
RAMEND		equ	0x10ff,data
FLASHEND	label	0x1ffff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
IVCE		avrbit	MCUCR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select
SM2		avrbit	MCUCR,2		; Sleep Mode Select
SM0		avrbit	MCUCR,3
SM1		avrbit	MCUCR,4
SE		avrbit	MCUCR,5		; Sleep Enable
SRW10		avrbit	MCUCR,6		; Wait State Select
SRE		avrbit	MCUCR,7		; Enable External SRAM

MCUCSR		port	0x34		; MCU Control and Status Register
WDRF		avrbit	MCUCSR,3	; Watchdog Reset Occured
BORF		avrbit	MCUCSR,2	; Brown-Out Reset Occured
EXTRF		avrbit	MCUCSR,1	; External Reset Occured
PORF		avrbit	MCUCSR,0	; Power-On Reset Occured

OSCCAL		sfr	0x6f		; Oscillator Calibration

XDIV		port	0x3c		; XTAL Divide Control Register

XMCRA		sfr	0x6d		; External Memory Control Register A
SRL2		avrbit	XMCRA,6		; Wait State Sector Limit
SRL1		avrbit	XMCRA,5
SRL0		avrbit	XMCRA,4
SRW01		avrbit	XMCRA,3		; Wait State Select Bits for Lower Sector
SRW00		avrbit	XMCRA,2
SRW11		avrbit	XMCRA,1

XMCRB		sfr	0x6c		; External Memory Control Register B
XMBK		avrbit	XMCRB,7		; External Memory Bus Keeper Enable
XMM2		avrbit	XMCRB,2		; External Memory High Mask
XMM1		avrbit	XMCRB,1
XMM0		avrbit	XMCRB,0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem.inc"
		include	"spmcsr68.inc"

;----------------------------------------------------------------------------
; JTAG

JTRF		avrbit	MCUCSR,4	; JTAG Reset Occured
JTD		avrbit	MCUCSR,7	; JTAG Disable

OCDR		port	0x22		; On-Chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PINC		port	0x13		; Port C @ 0x13 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINE            port	0x01		; Port E @ 0x01 (IO) ff.
PINF            port	0x00		; Port F @ 0x00 (IO) ff.,
DDRF		sfr	0x61		; Registers non-continuous
PORTF		sfr	0x62	
PING		sfr	0x63		; Port G @ 0x63 (DATA) ff.

SFIOR		port	0x20		; Special Function I/O Register
PUD		avrbit	PUD,2		; Pullup Disable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum INT3_vect		; External Interrupt Request 3
		nextenum INT4_vect		; External Interrupt Request 4
		nextenum INT5_vect		; External Interrupt Request 5
		nextenum INT6_vect		; External Interrupt Request 6
		nextenum INT7_vect		; External Interrupt Request 7
		nextenum TIMER2_COMP_vect	; Timer/Counter 2 Compare Match
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMP_vect	; Timer/Counter 0 Compare Match
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum ANALOG_COMP_vect	; Analog Comparator
		nextenum TIMER1_COMPC_vect	; Timer/Counter 1 Compare Match C
		nextenum TIMER3_CAPT_vect	; Timer/Counter 3 Capture Event
		nextenum TIMER3_COMPA_vect	; Timer/Counter 3 Compare Match A
		nextenum TIMER3_COMPB_vect	; Timer/Counter 3 Compare Match B
		nextenum TIMER3_COMPC_vect	; Timer/Counter 3 Compare Match C
		nextenum TIMER3_OVF_vect	; Timer/Counter 3 Overflow
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum TWI_vect		; Two-Wire Serial Interface
		nextenum SPM_READY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x6a		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1
ISC10		avrbit	EICRA,2		; External Interrupt 1 Sense Control
ISC11		avrbit	EICRA,3
ISC20		avrbit	EICRA,4		; External Interrupt 2 Sense Control
ISC21		avrbit	EICRA,5
ISC30		avrbit	EICRA,6		; External Interrupt 3 Sense Control
ISC31		avrbit	EICRA,7

EICRB		port	0x3a		; External Interrupt Control Register B
ISC40		avrbit	EICRB,0		; External Interrupt 4 Sense Control
ISC41		avrbit	EICRB,1
ISC50		avrbit	EICRB,2		; External Interrupt 5 Sense Control
ISC51		avrbit	EICRB,3
ISC60		avrbit	EICRB,4		; External Interrupt 6 Sense Control
ISC61		avrbit	EICRB,5
ISC70		avrbit	EICRB,6		; External Interrupt 7 Sense Control
ISC71		avrbit	EICRB,7

EIMSK		port	0x39		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1
INT2		avrbit	EIMSK,2		; Enable External Interrupt 2
INT3		avrbit	EIMSK,3		; Enable External Interrupt 3
INT4		avrbit	EIMSK,4		; Enable External Interrupt 4
INT5		avrbit	EIMSK,5		; Enable External Interrupt 5
INT6		avrbit	EIMSK,6		; Enable External Interrupt 6
INT7		avrbit	EIMSK,7		; Enable External Interrupt 7

EIFR		port	0x38		; External Interrupt Flags Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1	        avrbit	EIFR,1		; External Interrupt 1 Occured
INTF2		avrbit	EIFR,2		; External Interrupt 2 Occured
INTF3		avrbit	EIFR,3		; External Interrupt 3 Occured
INTF4	        avrbit	EIFR,4		; External Interrupt 4 Occured
INTF5		avrbit	EIFR,5		; External Interrupt 5 Occured
INTF6		avrbit	EIFR,6		; External Interrupt 6 Occured
INTF7	        avrbit	EIFR,7		; External Interrupt 7 Occured

;----------------------------------------------------------------------------
; Timers

TSM		avrbit	SFIOR,7		; Timer Syncronization
PSR321		avrbit	SFIOR,0		; Prescaler Reset T1..3
PSR0		avrbit	SFIOR,1		; Prescaler Reset T0

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
WGM01		avrbit	TCCR0,3		; Timer/Counter 0 Clear on Compare Match
COM00		avrbit	TCCR0,4		; Timer/Counter 0 Compare Mode
COM01		avrbit	TCCR0,5
WGM00		avrbit	TCCR0,6		; Timer/Counter 0 Waveform Generation Mode
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0		port	0x31

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1C0		avrbit	TCCR1A,2	; Timer/Counter 1 Compare Mode C
COM1C1		avrbit	TCCR1A,3
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
TCCR1C		sfr	0x7a		; Timer/Counter 1 Control Register C
FOC1C		avrbit	TCCR1C,5	; Timer/Counter 1 Force Output Compare C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
OCR1CL		sfr	0x78		; Timer/Counter 1 Output Compare Value C LSB
OCR1CH		sfr	0x79		; Timer/Counter 1 Output Compare Value C MSB
ICR1L		port	0x26		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x27		; Timer/Counter 1 Input Capture Value MSB

TCCR2		port	0x25		; Timer/Counter 2 Control Register
CS20		avrbit	TCCR2,0		; Timer/Counter 2 Prescaler Setting
CS21		avrbit	TCCR2,1
CS22		avrbit	TCCR2,2
WGM21		avrbit	TCCR2,3
COM20		avrbit	TCCR2,4		; Timer/Counter 2 Compare Mode
COM21		avrbit	TCCR2,5
WGM20		avrbit	TCCR2,6		; Timer/Counter 2 Waveform Generation Mode
TCNT2		port	0x24		; Timer/Counter 2 Value
OCR2		port	0x23		; Timer/Counter 2 Output Compare Value

TCCR3A		sfr	0x8b		; Timer/Counter 3 Control Register A
WGM30		avrbit	TCCR3A,0	; Timer/Counter 3 Waveform Generation Mode
WGM31		avrbit	TCCR3A,1
COM3C0		avrbit	TCCR3A,2	; Timer/Counter 3 Compare Mode C
COM3C1		avrbit	TCCR3A,3
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
TCCR3C		sfr	0x8c		; Timer/Counter 3 Control Register C
FOC3C		avrbit	TCCR3C,5	; Timer/Counter 3 Force Output Compare C
FOC3B		avrbit	TCCR3C,6	; Timer/Counter 3 Force Output Compare B
FOC3A		avrbit	TCCR3C,7	; Timer/Counter 3 Force Output Compare A
TCNT3L		sfr	0x88		; Timer/Counter 3 Value LSB
TCNT3H		sfr	0x89		; Timer/Counter 3 Value MSB
OCR3AL		sfr	0x86		; Timer/Counter 3 Output Compare Value A LSB
OCR3AH		sfr	0x87		; Timer/Counter 3 Output Compare Value A MSB
OCR3BL		sfr	0x84		; Timer/Counter 3 Output Compare Value B LSB
OCR3BH		sfr	0x85		; Timer/Counter 3 Output Compare Value B MSB
OCR3CL		sfr	0x82		; Timer/Counter 3 Output Compare Value C LSB
OCR3CH		sfr	0x83		; Timer/Counter 3 Output Compare Value C MSB
ICR3L		sfr	0x80		; Timer/Counter 3 Input Capture Value LSB
ICR3H		sfr	0x81		; Timer/Counter 3 Input Capture Value MSB

TIMSK		port	0x37		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0		avrbit	TIMSK,1		; Timer/Counter 0 Output Compare Interrupt Enable
TOIE1		avrbit	TIMSK,2		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK,3		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt Enable A
TICIE1		avrbit	TIMSK,5		; Timer/Counter 1 Input Capture Interrupt Enable
TOIE2		avrbit	TIMSK,6		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2		avrbit	TIMSK,7		; Timer/Counter 2 Output Compare Interrupt Enable
ETIMSK		sfr	0x7d		; Extended Timer Interrupt Mask Register
OCIE1C		avrbit	ETIMSK,0	; Timer/Counter 1 Output Compare Interrupt Enable C
OCIE3C		avrbit	ETIMSK,1	; Timer/Counter 3 Output Compare Interrupt Enable C
TOIE3		avrbit	ETIMSK,2	; Timer/Counter 3 Overflow Interrupt Enable
OCIE3B		avrbit	ETIMSK,3	; Timer/Counter 3 Output Compare Interrupt Enable B
OCIE3A		avrbit	ETIMSK,4	; Timer/Counter 3 Output Compare Interrupt Enable A
TICIE3		avrbit	ETIMSK,5	; Timer/Counter 3 Input Capture Interrupt Enable

TIFR		port	0x36		; Timer Interrupt Flag Register
ETIFR		sfr	0x7c		; Extended Timer Interrupt Flag Register

ASSR		port	0x30		; Asynchronous Status Register
TCR0UB		avrbit	ASSR,0		; Timer/Counter Control Register 0 Update Busy
OCR0UB		avrbit	ASSR,1		; Output Compare Register 0
TCN0UB		avrbit	ASSR,2		; Timer/Counter 0 Update Busy
AS0		avrbit	ASSR,3		; Asynchronous Timer/Counter 0

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
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

UCSR0C		sfr	0x95		; UART0 Control & Status Register C
UCPOL0		avrbit	UCSR0C,0	; UART0 Clock Polarity
UCSZ00		avrbit	UCSR0C,1	; UART0 Character Size
UCSZ01		avrbit	UCSR0C,2
USBS0		avrbit	UCSR0C,3	; UART0 Stop Bit Select
UPM00		avrbit	UCSR0C,4	; UART0 Parity Mode : Odd/Even
UPM01		avrbit	UCSR0C,5	; UART0 Parity Mode : Enable/Disable
UMSEL0		avrbit	UCSR0C,6	; UART0 USART Mode Select
URSEL0		avrbit	UCSR0C,7	; UART0 Register Select (1 for UCSRC)

UBRR0H		sfr	0x90		; UART0 Baud Rate Register MSB (overlayed with UCSRC)
UBRR0L		port	0x09		; UART0 Baud Rate Register LSB

UDR1		sfr	0x9c		; UART1 I/O Data Register

UCSR1A		sfr	0x9b		; UART1 Control & Status Register A
MPCM1		avrbit	UCSR1A,0	; UART1 Multi Processor Communication Mode
U2X1		avrbit	UCSR1A,1	; UART1 Double Transmission Speed
UPE1		avrbit	UCSR1A,2	; UART1 Parity Error
DOR1		avrbit	UCSR1A,3	; UART1 Overrun
FE1		avrbit	UCSR1A,4	; UART1 Framing Error
UDRE1		avrbit	UCSR1A,5	; UART1 Data Register Empty
TXC1		avrbit	UCSR1A,6	; UART1 Transmit Complete
RXC1		avrbit	UCSR1A,7	; UART1 Receive Complete

UCSR1B		sfr	0x9a		; UART1 Control & Status Register B
TXB81		avrbit	UCSR1B,0	; UART1 Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; UART1 Receive Bit 8
UCSZ12		avrbit	UCSR1B,2	; UART1 Character Size
TXEN1		avrbit	UCSR1B,3	; UART1 Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; UART1 Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; UART1 Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; UART1 Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; UART1 Enable Receive Complete Interrupt

UCSR1C		sfr	0x9d		; UART1 Control & Status Register C
UCPOL1		avrbit	UCSR1C,0	; UART1 Clock Polarity
UCSZ10		avrbit	UCSR1C,1	; UART1 Character Size
UCSZ11		avrbit	UCSR1C,2
USBS1		avrbit	UCSR1C,3	; UART1 Stop Bit Select
UPM10		avrbit	UCSR1C,4	; UART1 Parity Mode : Odd/Even
UPM11		avrbit	UCSR1C,5	; UART1 Parity Mode : Enable/Disable
UMSEL1		avrbit	UCSR1C,6	; UART1 USART Mode Select

UBRR1H		sfr	0x98		; UART1 Baud Rate Register MSB (overlayed with UCSRC)
UBRR1L		sfr	0x99		; UART1 Baud Rate Register LSB

;----------------------------------------------------------------------------
; SPI

		include	"spim.inc"

;----------------------------------------------------------------------------
; TWI

		include	"twim70.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"adcm16.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm.inc"

		restore			; re-enable listing

		endif			; __regm128inc
