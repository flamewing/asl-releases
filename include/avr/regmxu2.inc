		ifndef	__regmxu2inc
__regmxu2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMXU2.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega[8|16|32]U2              *
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
USBRF		avrbit	MCUSR,5		; USB Reset Flag
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured

OSCCAL		sfr	0x66		; Oscillator Calibration

PRR0		sfr	0x64		; Power Reduction Register 0
PRSPI		avrbit	PRR0,2		; Power Reduction SPI
PRTIM1		avrbit	PRR0,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR0,5		; Power Reduction Timer/Counter 0
PRR1		sfr	0x65		; Power Reduction Register 1
PRUSART1	avrbit	PRR1,0		; Power Reduction USART1
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
PINDIV		avrbit	PLLCSR,2	; PLL Input Prescaler (1:1, 1:2)
DIV3		avrbit	PLLCSR,3	; PLL Input Prescaler (1:3)
DIV5		avrbit	PLLCSR,4	; PLL Input Prescaler (1:5)

CLKSEL0		sfr	0xd0		; Clock Selection Register 0
CLKS		avrbit	CLKSEL0,0	; Clock Selector
EXTE		avrbit	CLKSEL0,2	; Enable External Oscillator / Low Power Oscillator
RCE		avrbit	CLKSEL0,3	; Enable RC Oscillator
EXSUT0		avrbit	CLKSEL0,4	; SUT for External Oscillator / Low Power Oscillator
EXSUT1		avrbit	CLKSEL0,5
RCSUT0		avrbit	CLKSEL0,6	; SUT for RC Oscillator
RCSUT1		avrbit	CLKSEL0,7

CLKSEL1		sfr	0xd1		; Clock Selection Register 1
EXCKSEL0	avrbit	CLKSEL1,0	; CKSEL for External Oscillator / Low Power Oscillator
EXCKSEL1	avrbit	CLKSEL1,1
EXCKSEL2	avrbit	CLKSEL1,2
EXCKSEL3	avrbit	CLKSEL1,3
RCCKSEL0	avrbit	CLKSEL1,4	; CKSEL for RC Oscillator
RCCKSEL1	avrbit	CLKSEL1,5
RCCKSEL2	avrbit	CLKSEL1,6
RCCKSEL3	avrbit	CLKSEL1,7

CLKSTA		sfr	0xd2		; Clock Status Register
EXTON		avrbit	CLKSTA,0	; External Oscillator / Low Power Oscillator On
RCON		avrbit	CLKSTA,1	; RC Oscillator On

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem2.inc"

EEPM0		avrbit	EECR,4		; EEPROM Programming Mode
EEPM1		avrbit	EECR,5

		include	"spmcsr37.inc"
		
SIGRD		avrbit	SPMCSR,5	; Signature Read

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
__PORTC_BITS	equ	0xf7		; (bits 0..2,4..7)
PIND		port	0x09		; Port D @ 0x09 (IO) ff.

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PUD		avrbit	MCUCR,4		; Pullup Disable

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0

PCMSK1		sfr	0x6c		; Pin Change Mask Register 1
PCINT8		avrbit	PCMSK1,0	; Pin Change Enable Mask 8
PCINT9		avrbit	PCMSK1,1	; Pin Change Enable Mask 9
PCINT10		avrbit	PCMSK1,2	; Pin Change Enable Mask 10
PCINT11		avrbit	PCMSK1,3	; Pin Change Enable Mask 11
PCINT12		avrbit	PCMSK1,4	; Pin Change Enable Mask 12

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
		nextenum PCINT1_vect		; Pin Change Interrupt Request 1
		nextenum USB_GEN_vect		; USB General Interrupt Request
		nextenum USB_COM_vect		; USB Endpoint Interrupt Request
		nextenum WDT_vect		; Watchdog Time-out Interrupt
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
		nextenum EE_READY_vect		; EEPROM Ready
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
ISC40		avrbit	EICRB,0		; External Interrupt 4 Sense Control
ISC41		avrbit	EICRB,1
ISC50		avrbit	EICRB,2		; External Interrupt 5 Sense Control
ISC51		avrbit	EICRB,3
ISC60		avrbit	EICRB,4		; External Interrupt 6 Sense Control
ISC61		avrbit	EICRB,5
ISC70		avrbit	EICRB,6		; External Interrupt 7 Sense Control
ISC71		avrbit	EICRB,7

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
INTF4		avrbit	EIFR,4		; External Interrupt 4 Occured
INTF5	        avrbit	EIFR,5		; External Interrupt 5 Occured
INTF6		avrbit	EIFR,6		; External Interrupt 6 Occured
INTF7	        avrbit	EIFR,7		; External Interrupt 7 Occured

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

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset for Synchronous Timer/Counters
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdme.inc"
		
WDTCKD		sfr	0x62		; Watchdog Timer Clock Divider Register
WCLKD0		avrbit	WDTCKD,0	; Watchdog Timer Clock Divider
WCLKD1		avrbit	WDTCKD,1
WDEWIE		avrbit	WDTCKD,2	; Watchdog Early Warning Interrupt Enable
WDEWIF		avrbit	WDTCKD,3	; Watchdog Early Warning Interrupt Flag
WCLKD2		avrbit	WDTCKD,4
WDEWIFCM	avrbit	WDTCKD,5	; Watchdog Early Warning Flag Clear Mode

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

UCSR1D		sfr	0xcb		; Control & Status Register D
RTSEN		avrbit	UCSR1D,0	; USART RTS Enable
CTSEN		avrbit	UCSR1D,1	; USART CTS Enable

UBRR1H		sfr	0xcc		; Baud Rate Register High
UBRR1L		sfr	0xcd		; Baud Rate Register Low

;----------------------------------------------------------------------------
; SPI

		include "spim2c.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm30.inc"

AIN6D		avrbit	DIDR1,6		; Disable Digital Input on AIN6
AIN5D		avrbit	DIDR1,5		; Disable Digital Input on AIN5
AIN4D		avrbit	DIDR1,4		; Disable Digital Input on AIN4
AIN3D		avrbit	DIDR1,3		; Disable Digital Input on AIN3
AIN2D		avrbit	DIDR1,2		; Disable Digital Input on AIN2

ACMUX		sfr	0x7d		; Analog Comparator Multiplexer Selection
CMUX0		avrbit	ACMUX,0		; Multiplexer Select
CMUX1		avrbit	ACMUX,1
CMUX2		avrbit	ACMUX,2

;----------------------------------------------------------------------------
; USB

		include	"usbm.inc"

UPOE		sfr	0xfb		; USB Software Output Enable Register
DMI		avrbit	UPOE,0		; D- Input Value
DPI		avrbit	UPOE,1		; D+ Input Value
DATAI		avrbit	UPOE,2
SCKI		avrbit	UPOE,3
UPDRV0		avrbit	UPOE,4		; USB Direct Drive Values
UPDRV1		avrbit	UPOE,5
UPWE0		avrbit	UPOE,6		; USB Buffers Direct Drive Enable Configuration
UPWE1		avrbit	UPOE,7

REGCR		sfr	0x63		; Regulator Control
REGDIS		avrbit	REGCR,0		; Regulator Disable

RSTCPU		avrbit	UDCON,2		; USB Reset CPU Bit

KILLBK		avrbit	UEINTX,2	; Kills the Last Loaded Bank

		restore			; Re-Enable Listing

		endif			; __regmxu2inc
