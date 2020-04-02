		ifndef	__regmxu6inc
__regmxu4inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMXU4.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega32U6                     *
;*                                           AT90USB646/647                 *
;*                                           AT90USB1286/1287               *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
IVCE		avrbit	MCUCR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Enable
SM2		avrbit	SMCR,3		; Sleep Mode Select
SM1		avrbit	SMCR,2
SM0		avrbit	SMCR,1

MCUSR		port	0x34		; MCU Control and Status Register
JTRF		avrbit	MCUSR,4		; JTAG Reset Flag
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured

OSCCAL		sfr	0x66		; Oscillator Calibration

PRR0		sfr	0x64		; Power Reduction Register 0
PRADC		avrbit	PRR0,0		; Power Reduction A/D Converter
PRSPI		avrbit	PRR0,2		; Power Reduction SPI
PRTIM1		avrbit	PRR0,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR0,5		; Power Reduction Timer/Counter 0
PRTIM2		avrbit	PRR0,6		; Power Reduction Timer/Counter 2
PRTWI		avrbit	PRR0,7		; Power Reduction Two-Wire Interface
PRR1		sfr	0x65		; Power Reduction Register 1
PRUSART1	avrbit	PRR1,0		; Power Reduction USART1
PRTIM3		avrbit	PRR1,3		; Power Reduction Timer/Counter 3
PRUSB		avrbit	PRR1,7		; Power Reduction USB

CLKPR		sfr	0x61		; Clock Prescale Register
CLKPS0		avrbit	CLKPR,0		; Clock Prescaler Select Bits
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

PLLCSR		port	0x29		; PLL Control and Status Register
PLOCK		avrbit	PLLCSR,0	; PLL Lock Detector
PLLE		avrbit	PLLCSR,1	; PLL Enable
PLLP0		avrbit	PLLCSR,2	; PLL Prescaler
PLLP1		avrbit	PLLCSR,3
PLLP2		avrbit	PLLCSR,4

;----------------------------------------------------------------------------
; JTAG etc.

JTD		avrbit	MCUCR,7		; JTAG disable

CRDR		port	0x31		; Monitor Data Register
MONDR		port	0x31

XMCRA		sfr	0x74		; External Memory Access Register A
SRW00		avrbit	XMCRA,0		; Wait-state Select Bits for Lower Sector
SRW01		avrbit	XMCRA,1
SRW10		avrbit	XMCRA,2		; Wait-state Select Bits for Upper Sector
SRW11		avrbit	XMCRA,3
SRL0		avrbit	XMCRA,4		; Wait-state Sector Limit
SRL1		avrbit	XMCRA,5
SRL2		avrbit	XMCRA,6
SRE		avrbit	XMCRA,7		; External SRAM/XMEM Enable
XMCRB		sfr	0x75		; External Memory Access Register B
XMM0		avrbit	XMCRB,0		; External Memory High Mask
XMM1		avrbit	XMCRB,1
XMM2		avrbit	XMCRB,2 
XMBK		avrbit	XMCRB,7		; External Memory Bus-keeper Enable

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem2.inc"

EEPM0		avrbit	EECR,4		; EEPROM Programming Mode
EEPM1		avrbit	EECR,5

		include	"spmcsr37.inc"
		
SIGRD		avrbit	SPMCSR,5	; Signature Read

;----------------------------------------------------------------------------
; GPIO

__PORTPREFIX	equ	"PORT"

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.
PINE		port	0x0c		; Port E @ 0x0c (IO) ff.
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PUD		avrbit	MCUCR,4		; Pullup Disable

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0

PCICR		sfr	0x68		; Pin Change Control Register

PCIFR		port	0x1b		; Pin Change Flag Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum INT3_vect		; External Interrupt Request 3
		nextenum INT4_vect		; External Interrupt Request 4
		nextenum INT5_vect		; External Interrupt Request 5
		nextenum INT6_vect		; External Interrupt Request 6
		nextenum INT7_vect		; External Interrupt Request 7
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum USB_GEN_vect		; USB General Interrupt Request
		nextenum USB_COM_vect		; USB Endpoint Interrupt Request
		nextenum WDT_vect		; Watchdog Time-out Interrupt
		nextenum TIMER2_COMPA_vect	; Timer/Counter 2 Compare Match A
		nextenum TIMER2_COMPB_vect	; Timer/Counter 2 Compare Match B
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_COMPC_vect	; Timer/Counter 1 Compare Match C
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum ANALOG_COMP_vect	; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum TIMER3_CAPT_vect	; Timer/Counter 3 Capture
		nextenum TIMER3_COMPA_vect	; Timer/Counter 3 Compare Match A
		nextenum TIMER3_COMPB_vect	; Timer/Counter 3 Compare Match B
		nextenum TIMER3_COMPC_vect	; Timer/Counter 3 Compare Match C
		nextenum TIMER3_OVF_vect	; Timer/Counter 3 Overflow
		nextenum TWI_vect		; Two-Wire Serial Interface
		nextenum SPM_READY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1
ISC10		avrbit	EICRA,2		; External Interrupt 1 Sense Control
ISC11		avrbit	EICRA,3
ISC20		avrbit	EICRA,4		; External Interrupt 2 Sense Control
ISC21		avrbit	EICRA,5
ISC30		avrbit	EICRA,6		; External Interrupt 3 Sense Control
ISC31		avrbit	EICRA,7
EICRB		sfr	0x6a		; External Interrupt Control Register A
ISC60		avrbit	EICRB,4		; External Interrupt 6 Sense Control
ISC61		avrbit	EICRB,5

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; External Interrupt Request 0 Enable
INT1		avrbit	EIMSK,1		; External Interrupt Request 1 Enable
INT2		avrbit	EIMSK,2		; External Interrupt Request 2 Enable
INT3		avrbit	EIMSK,3		; External Interrupt Request 3 Enable
INT4		avrbit	EIMSK,4		; External Interrupt Request 4 Enable
INT5		avrbit	EIMSK,5		; External Interrupt Request 5 Enable
INT6		avrbit	EIMSK,6		; External Interrupt Request 6 Enable
INT7		avrbit	EIMSK,7		; External Interrupt Request 7 Enable

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1	        avrbit	EIFR,1		; External Interrupt 1 Occured
INTF2		avrbit	EIFR,2		; External Interrupt 2 Occured
INTF3	        avrbit	EIFR,3		; External Interrupt 3 Occured
INTF4	        avrbit	EIFR,4		; External Interrupt 4 Occured
INTF5	        avrbit	EIFR,5		; External Interrupt 5 Occured
INTF6	        avrbit	EIFR,6		; External Interrupt 6 Occured
INTF7		avrbit	EIFR,7		; External Interrupt 7 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Compare Match Output B Mode
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Compare Match Output A Mode
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x25		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x26		; Timer/Counter 0 Value
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Register A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Register B

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1C0		avrbit	TCCR1A,2	; Timer/Counter 1 Compare Mode C
COM1C1		avrbit	TCCR1A,3
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3	; Timer/Counter 1 Waveform Generation Mode
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Capture Noise Filter
TCCR1C		sfr	0x82		; Timer/Counter 1 Control Register C
FOC1C		avrbit	TCCR1C,5	; Timer/Counter 1 Force Output Compare C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x8a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x8b		; Timer/Counter 1 Output Compare Value B MSB
OCR1CL		sfr	0x8c		; Timer/Counter 1 Output Compare Value C LSB
OCR1CH		sfr	0x8d		; Timer/Counter 1 Output Compare Value C MSB
ICR1L		sfr	0x86		; Timer/Counter 1 Input Capture Value LSB
ICR1H		sfr	0x87		; Timer/Counter 1 Input Capture Value MSB

TCCR2A		sfr	0xb0		; Timer/Counter 2 Control Register A
WGM20		avrbit	TCCR2A,0	; Timer/Counter 2 Waveform Generation Mode
WGM21		avrbit	TCCR2A,1
COM2B0		avrbit	TCCR2A,4	; Timer/Counter 2 Compare Mode B
COM2B1		avrbit	TCCR2A,5
COM2A0		avrbit	TCCR2A,6	; Timer/Counter 2 Compare Mode A
COM2A1		avrbit	TCCR2A,7
TCCR2B		sfr	0xb1		; Timer/Counter 2 Control Register B
CS20		avrbit	TCCR2B,0	; Timer/Counter 2 Prescaler Setting
CS21		avrbit	TCCR2B,1
CS22		avrbit	TCCR2B,2
WGM22		avrbit	TCCR2B,3
FOC2B		avrbit	TCCR2B,6	; Timer/Counter 2 Force Output Compare B
FOC2A		avrbit	TCCR2B,7	; Timer/Counter 2 Force Output Compare A
TCNT2		sfr	0xb2		; Timer/Counter 2 Value
OCR2A		sfr	0xb3		; Timer/Counter 2 Output Compare Value A
OCR2B		sfr	0xb4		; Timer/Counter 2 Output Compare Value B

TCCR3A		sfr	0x90		; Timer/Counter 3 Control Register A
WGM30		avrbit	TCCR3A,0	; Timer/Counter 3 Waveform Generation Mode
WGM31		avrbit	TCCR3A,1
COM3C0		avrbit	TCCR3A,2	; Timer/Counter 3 Compare Mode C
COM3C1		avrbit	TCCR3A,3
COM3B0		avrbit	TCCR3A,4	; Timer/Counter 3 Compare Mode B
COM3B1		avrbit	TCCR3A,5
COM3A0		avrbit	TCCR3A,6	; Timer/Counter 3 Compare Mode A
COM3A1		avrbit	TCCR3A,7
TCCR3B		sfr	0x91		; Timer/Counter 3 Control Register B
CS30		avrbit	TCCR3B,0	; Timer/Counter 3 Prescaler Setting
CS31		avrbit	TCCR3B,1
CS32		avrbit	TCCR3B,2
WGM32		avrbit	TCCR3B,3	; Timer/Counter 3 Waveform Generation Mode
WGM33		avrbit	TCCR3B,4
ICES3		avrbit	TCCR3B,6	; Timer/Counter 3 Capture Slope Selection
ICNC3		avrbit	TCCR3B,7	; Timer/Counter 3 Capture Noise Filter
TCCR3C		sfr	0x92		; Timer/Counter 3 Control Register C
FOC3C		avrbit	TCCR3C,5	; Timer/Counter 3 Force Output Compare C
FOC3B		avrbit	TCCR3B,6	; Timer/Counter 3 Force Output Compare B
FOC3A		avrbit	TCCR3C,7	; Timer/Counter 3 Force Output Compare A
TCNT3L		sfr	0x94		; Timer/Counter 3 Value LSB
TCNT3H		sfr	0x95		; Timer/Counter 3 Value MSB
OCR3AL		sfr	0x98		; Timer/Counter 3 Output Compare Value A LSB
OCR3AH		sfr	0x99		; Timer/Counter 3 Output Compare Value A MSB
OCR3BL		sfr	0x9a		; Timer/Counter 3 Output Compare Value B LSB
OCR3BH		sfr	0x9b		; Timer/Counter 3 Output Compare Value B MSB
OCR3CL		sfr	0x9c		; Timer/Counter 3 Output Compare Value C LSB
OCR3CH		sfr	0x9d		; Timer/Counter 3 Output Compare Value C MSB
ICR3L		sfr	0x96		; Timer/Counter 3 Input Capture Value LSB
ICR3H		sfr	0x97		; Timer/Counter 3 Input Capture Value MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK0,2	; Timer/Counter 1 Output Compare Interrupt Enable B

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1C		avrbit	TIMSK1,3	; Timer/Counter 1 Output Compare Interrupt Enable C
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Interrupt Enable

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

TIMSK2		sfr	0x70		; Timer/Counter 2 Interrupt Mask Register
TOIE2		avrbit	TIMSK2,0	; Timer/Counter 2 Overflow Interrupt Enable
OCIE2A		avrbit	TIMSK2,1	; Timer/Counter 2 Output Compare Interrupt Enable A
OCIE2B		avrbit	TIMSK2,2	; Timer/Counter 2 Output Compare Interrupt Enable B

TIFR2		port	0x17		; Timer/Counter 2 Interrupt Flag Register

TIMSK3		sfr	0x71		; Timer/Counter 3 Interrupt Mask Register
TOIE3		avrbit	TIMSK3,0	; Timer/Counter 3 Overflow Interrupt Enable
OCIE3A		avrbit	TIMSK3,1	; Timer/Counter 3 Output Compare Interrupt Enable A
OCIE3B		avrbit	TIMSK3,2	; Timer/Counter 3 Output Compare Interrupt Enable B
OCIE3C		avrbit	TIMSK3,3	; Timer/Counter 3 Output Compare Interrupt Enable C
ICIE3		avrbit	TIMSK3,5	; Timer/Counter 3 Input Capture Interrupt Enable

TIFR3		port	0x18		; Timer/Counter 3 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset for Synchronous Timer/Counters
PSRASY		avrbit	GTCCR,1
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2BUB		avrbit	ASSR,0		; Timer/Counter Control Register2 Update Busy
TCR2AUB		avrbit	ASSR,1		; Timer/Counter Control Register2 Update Busy
OCR2BUB		avrbit	ASSR,2		; Output Compare Register2 Update Busy
OCR2AUB		avrbit	ASSR,3		; Output Compare Register2 Update Busy
TCN2UB		avrbit	ASSR,4		; Timer/Counter2 Update Busy
AS2		avrbit	ASSR,5		; Asynchronous Timer/Counter 2
EXCLK		avrbit	ASSR,6		; Enable External Clock Input

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdme.inc"
		
;----------------------------------------------------------------------------
; USART

UDR1		sfr	0xce		; I/O Data Register

UCSR1A		sfr	0xc8		; Control & Status Register A
MPCM1		avrbit	UCSR1A,0	; Multi Processor Communication Mode
U2X1		avrbit	UCSR1A,1	; Double Transmission Speed
PE1		avrbit	UCSR1A,2	; Parity Error
DOR1		avrbit	UCSR1A,3	; Overrun
FE1		avrbit	UCSR1A,4	; Framing Error
UDRE1		avrbit	UCSR1A,5	; Data Register Empty
TXC1		avrbit	UCSR1A,6	; Transmit Complete
RXC1		avrbit	UCSR1A,7	; Receive Complete

UCSR1B		sfr	0xc9		; Control & Status Register B
TXB81		avrbit	UCSR1B,0	; Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; Receive Bit 8
UCSZ21		avrbit	UCSR1B,2	; Character Size
TXEN1		avrbit	UCSR1B,3	; Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; Enable Receive Complete Interrupt

UCSR1C		sfr	0xca		; Control & Status Register C
UCPOL1		avrbit	UCSR1C,0	; Clock Polarity
UCSZ10		avrbit	UCSR1C,1	; Character Size
UCSZ11		avrbit	UCSR1C,2
USBS1		avrbit	UCSR1C,3	; Stop Bit Select
UPM10		avrbit	UCSR1C,4	; Parity Mode : Odd/Even
UPM11		avrbit	UCSR1C,5	; Parity Mode : Enable/Disable
UMSEL10		avrbit	UCSR1C,6	; USART Mode Select
UMSEL11		avrbit	UCSR1C,7

UBRR1H		sfr	0xcc		; Baud Rate Register High
UBRR1L		sfr	0xcd		; Baud Rate Register Low

;----------------------------------------------------------------------------
; SPI

		include "spim2c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"twimb8.inc"
		
;----------------------------------------------------------------------------
; Analog Comparator

		include "acm30.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"adcm78.inc"

MUX4		avrbit	ADMUX,4

ADHSM		avrbit	ADCSRB,7	; A/D Converter High Speed Mode

;----------------------------------------------------------------------------
; USB

		include	"usbm.inc"

OTGPADE		avrbit	USBCON,4	; VBUS Pad Enable
HOST		avrbit	USBCON,2	; Host Mode (conditional?)
IDTE		avrbit	USBCON,1	; ID Transition Interrupt Enable Bit
VBUSTE		avrbit	USBCON,0	; VBUS Transition Interrupt Enable Bit

LSM		avrbit	UDCON,2		; USB Device Low Speed Mode Selection

EPRST5		avrbit	UERST,5		; Endpoint 5 FIFO Reset
EPRST6		avrbit	UERST,6		; Endpoint 6 FIFO Reset

EPINT5		avrbit	UEINT,5		; Endpoint Interrupts Bits
EPINT6		avrbit	UEINT,6		; 

UHWCON		sfr	0xd7		; USB Hardware Control
UVREGE		avrbit	UHWCON,0	; USB Pad Regulator Enable
UVCONE		avrbit	UHWCON,4	; UVCON Pin Enable
UIDE		avrbit	UHWCON,6	; UID Pin Enable
UIMOD		avrbit	UHWCON,7	; USB Mode

USBSTA		sfr	0xd9		; 
VBUS		avrbit	USBSTA,0	; VBus Flag
ID		avrbit	USBSTA,1	; ID status
SPEED		avrbit	USBSTA,3	; Speed Status Flag

USBINT		sfr	0xda		; 
VBUSTI		avrbit	USBINT,0	; IVBUS Transition Interrupt Flag
IDTI		avrbit	USBINT,1	; D Transition Interrupt Flag

UEBCHX		sfr	0xf3		; Byte Count High

		if	__USBHOSTMODE	; only for variants with host/OTG mode

UHCON		sfr	0x9e		; USB Host Control
SOFEN		avrbit	UHCON,0		; Start Of Frame Generation Enable
RESET		avrbit	UHCON,1		; Send USB Reset
RESUME		avrbit	UHCON,2		; Send USB Resume

UHINT		sfr	0x9f		; USB Host Interrupt Status
DCONNI		avrbit	UHINT,0		; Device Connection Interrupt
DDISCI		avrbit	UHINT,1		; Device Disconnection Interrupt
RSTI		avrbit	UHINT,2		; USB Reset Sent Interrupt
RSMEDI		avrbit	UHINT,3		; Downstream Resume Sent Interrupt
RXRSMI		avrbit	UHINT,4		; Upstream Resume Received Interrupt
HSOFI		avrbit	UHINT,5		; Host Start Of Frame Interrupt
HWUPI		avrbit	UHINT,6		; Host Wake-Up Interrupt

UHIEN		sfr	0xa0		; USB Host Interrupt Enable
DCONNE		avrbit	UHIEN,0		; Device Connection Interrupt Enable
DDISCE		avrbit	UHIEN,1		; Device Disconnection Interrupt Enable
RSTE		avrbit	UHIEN,2		; USB Reset Sent Interrupt Enable
RSMEDE		avrbit	UHIEN,3		; Downstream Resume Sent Interrupt Enable
RXRSME		avrbit	UHIEN,4		; Upstream Resume Received Interrupt Enable
HSOFE		avrbit	UHIEN,5		; Host Start Of frame Interrupt Enable
HWUPE		avrbit	UHIEN,6		; Host Wake-Up Interrupt Enable

UHADDR		sfr	0xa1		; USB Host Address

UHFNUML		sfr	0xa2		; USB Host Frame Number LSB

UHFNUMH		sfr	0xa3		; USB Host Frame Number MSB

UHFLEN		sfr	0xa4		; USB Host Frame Length

UPINRQX		sfr	0xa5		; 
INRQ0		avrbit	UPINRQX,0	; IN Request Number Before Freeze
INRQ1		avrbit	UPINRQX,1
INRQ2		avrbit	UPINRQX,2
INRQ3		avrbit	UPINRQX,3
INRQ4		avrbit	UPINRQX,4
INRQ5		avrbit	UPINRQX,5
INRQ6		avrbit	UPINRQX,6
INRQ7		avrbit	UPINRQX,7

UPINTX		sfr	0xa6		; USB Pipe Interrupt Status
RXINI		avrbit	UPINTX,0	; IN Data received
RXSTALLI	avrbit	UPINTX,1	; CRCERR - STALL Received / Isochronous CRC Error
TXOUTI		avrbit	UPINTX,2	; OUT Bank ready
TXSTPI		avrbit	UPINTX,3	; SETUP Bank ready
PERRI		avrbit	UPINTX,4	; PIPE Error
PRWAL		avrbit	UPINTX,5	; Read/Write Allowed (also as RWAL from UEINTX definition)
PNAKEDI		avrbit	UPINTX,6	; NAK Handshake received (also as NAKEDI from UEINTX definition)
PFIFOCON	avrbit	UPINTX,7	; FIFO Control

UPNUM		sfr	0xa7		; USB Pipe Number
PNUM0		avrbit	UPNUM,0		; Pipe Number
PNUM1		avrbit	UPNUM,1
PNUM2		avrbit	UPNUM,2

UPRST		sfr	0xa8		; USB Pipe Reset
P0RST		avrbit	UPRST,0		; Pipe 0 Reset
P1RST		avrbit	UPRST,1		; Pipe 1 Reset
P2RST		avrbit	UPRST,2		; Pipe 2 Reset
P3RST		avrbit	UPRST,3		; Pipe 3 Reset
P4RST		avrbit	UPRST,4		; Pipe 4 Reset
P5RST		avrbit	UPRST,5		; Pipe 5 Reset
P6RST		avrbit	UPRST,6		; Pipe 6 Reset

UPCONX		sfr	0xa9		; USB Pipe Config
PEN		avrbit	UPCONX,0	; Pipe Enable
PRSTDT		avrbit	UPCONX,3	; Reset Data Toggle (also RSTDT from UECONX definition)
INMODE		avrbit	UPCONX,5	; IN Request mode
PFREEZE		avrbit	UPCONX,6	; Pipe Freeze

UPCFG0X		sfr	0xaa		; USB Pipe Config 0
PEPNUM0		avrbit	UPCFG0X,0	; Pipe Endpoint Number
PEPNUM1		avrbit	UPCFG0X,1
PEPNUM2		avrbit	UPCFG0X,2
PEPNUM3		avrbit	UPCFG0X,3
PTOKEN0		avrbit	UPCFG0X,4	; Pipe Token
PTOKEN1		avrbit	UPCFG0X,5
PTYPE0		avrbit	UPCFG0X,6	; Pipe Type
PTYPE1		avrbit	UPCFG0X,7

UPCFG1X		sfr	0xab		; USB Pipe Config 1
PALLOC		avrbit	UPCFG1X,1	; Configure Pipe Memory (also ALLOC from UECFG1X definition)
PBK0		avrbit	UPCFG1X,2	; Pipe Bank
PBK1		avrbit	UPCFG1X,3
PSIZE0		avrbit	UPCFG1X,4	; Pipe Size
PSIZE1		avrbit	UPCFG1X,5
PSIZE2		avrbit	UPCFG1X,6

UPSTAX		sfr	0xac		; USB Status
PNBUSYBK0	avrbit	UPSTAX,0	; Busy Bank Flag (also NBUSYBK0/1 from UESTA0X definition)
PNBUSYBK1	avrbit	UPSTAX,1
PDTSEQ0		avrbit	UPSTAX,2	; Toggle Sequencing Flag (also DTSEQ0/1 from UESTA0X definition)
PDTSEQ1		avrbit	UPSTAX,3	
PUNDERFI	avrbit	UPSTAX,5	; Underflow (also UNDERFI from UESTA0X definition)
POVERFI		avrbit	UPSTAX,6	; Overflow (also OVERFI from UESTA0X definition)
PCFGOK		avrbit	UPSTAX,7	; Configure Pipe Memory OK (also CFGOK from UESTA0X definition)

UPCFG2X		sfr	0xad		; USB Pipe Config 2
INTFRQ0		avrbit	UPCFG2X,0	; Interrupt Pipe Request Frequency
INTFRQ1		avrbit	UPCFG2X,1
INTFRQ2		avrbit	UPCFG2X,2
INTFRQ3		avrbit	UPCFG2X,3
INTFRQ4		avrbit	UPCFG2X,4
INTFRQ5		avrbit	UPCFG2X,5
INTFRQ6		avrbit	UPCFG2X,6
INTFRQ7		avrbit	UPCFG2X,7

UPIENX		sfr	0xae		; USB Pipe Interrupt Enable
RXINE		avrbit	UPIENX,0	; IN Data received Interrupt Enable
RXSTALLE	avrbit	UPIENX,1	; STALL Received Interrupt Enable
TXOUTE		avrbit	UPIENX,2	; OUT Bank ready Interrupt Enable
TXSTPE		avrbit	UPIENX,3	; SETUP Bank ready Interrupt Enable
PERRE		avrbit	UPIENX,4	; PIPE Error Interrupt Enable
NAKEDE		avrbit	UPIENX,6	; NAK Handshake Received Interrupt Enable
PFLERRE		avrbit	UPIENX,7	; Flow Error Interrupt enable (also FLERRE from UEIENX definition)

UPDATX		sfr	0xaf		; Pipe Data Bits

UPERRX		sfr	0xf5		; USB Pipe Error Bits
DATATGL		avrbit	UPERRX,0	; Bad Data Toggle
DATAPID		avrbit	UPERRX,1	; Data PID Error
PID		avrbit	UPERRX,2	; PID Error
TIMEOUT		avrbit	UPERRX,3	; Time-out Error
CRC16		avrbit	UPERRX,4	; CRC16 Error
COUNTER0	avrbit	UPERRX,5	; Error counter
COUNTER1	avrbit	UPERRX,6

UPBCLX		sfr	0xf6		; USB Pipe Byte Count LSB
UPBCHX		sfr	0xf7		; USB Pipe Byte Count MSB

UPINT		sfr	0xf8		; USB Pipe Interrupts
PINT0		avrbit	UPINT,0		; Pipe Interrupts Bits
PINT1		avrbit	UPINT,1
PINT2		avrbit	UPINT,2
PINT3		avrbit	UPINT,3
PINT4		avrbit	UPINT,4
PINT5		avrbit	UPINT,5
PINT6		avrbit	UPINT,6

OTGTCON		sfr	0xf9		; USB On-The-Go Configuration
VALUE0		avrbit	OTGTCON,0	; Value Bit
VALUE1		avrbit	OTGTCON,1
PAGE0		avrbit	OTGTCON,5	; Timer page access Bit
PAGE1		avrbit	OTGTCON,6

OTGCON		sfr	0xdd		; USB On-The-Go Control
VBUSRQC		avrbit	OTGCON,0	; VBUS Request Clear Bit
VBUSREQ		avrbit	OTGCON,1	; VBUS Request Bit
VBUSHWC		avrbit	OTGCON,2	; VBus Hardware Control Bit
SRPSEL		avrbit	OTGCON,3	; SRP Selection Bit
SRPREQ		avrbit	OTGCON,4	; SRP Request Bit
HNPREQ		avrbit	OTGCON,5	; HNP Request Bit

OTGIEN		sfr	0xde		; USB On-The-Go Interrupt Enable
SRPE		avrbit	OTGIEN,0	; SRP Interrupt Enable Bit
VBERRE		avrbit	OTGIEN,1	; VBus Error Interrupt Enable Bit
BCERRE		avrbit	OTGIEN,2	; B-Connection Error Interrupt Enable Bit
ROLEEXE		avrbit	OTGIEN,3	; Role Exchange Interrupt Enable Bit
HNPERRE		avrbit	OTGIEN,4	; HNP Error Interrupt Enable Bit
STOE		avrbit	OTGIEN,5	; Suspend Time-out Error Interrupt Enable Bit

OTGINT		sfr	0xdf		; USB On-The-Go Interrupt Status
SRPI		avrbit	OTGINT,0	; Suspend Time-out Error Interrupt Flag
VBERRI		avrbit	OTGINT,1	; HNP Error Interrupt Flag
BCERRI		avrbit	OTGINT,2	; Role Exchange Interrupt Flag
ROLEEXI		avrbit	OTGINT,3	; B-Connection Error Interrupt Flag
HNPERRI		avrbit	OTGINT,4	; V-Bus Error Interrupt Flag
STOI		avrbit	OTGINT,5	; SRP Interrupt Flag

		endif			; __USBHOSTMODE

		restore			; re-enable Listing

		endif			; __regmxu6inc
