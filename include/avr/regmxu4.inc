		ifndef	__regmxu4inc
__regmxu4inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMXU4.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega[16|32]U4                *
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
JTRF		avrbit	MCUSR,4		; JTAG Reset Flag
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-On Reset Occured

OSCCAL		sfr	0x66		; Oscillator Calibration

RCCTRL		sfr	0x67		; Oscillator Control Register
RCFREQ		avrbit	RCCTRL,0	; RC Oscillator Frequency Select

PRR0		sfr	0x64		; Power Reduction Register 0
PRADC		avrbit	PRR0,0		; Power Reduction A/D Converter
PRSPI		avrbit	PRR0,2		; Power Reduction SPI
PRTIM1		avrbit	PRR0,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR0,5		; Power Reduction Timer/Counter 0
PRTWI		avrbit	PRR0,7		; Power Reduction Two-Wire Interface
PRR1		sfr	0x65		; Power Reduction Register 1
PRUSART1	avrbit	PRR1,0		; Power Reduction USART1
PRTIM3		avrbit	PRR1,3		; Power Reduction Timer/Counter 3
PRTIM4		avrbit	PRR1,4		; Power Reduction Timer/Counter 4
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
PINDIV		avrbit	PLLCSR,4	; PLL Input Prescaler (1:1, 1:2)

PLLFRQ		port	0x32		; PLL Frequency Control Register
PDIV0		avrbit	PLLFRQ,0	; PLL Lock Frequency
PDIV1		avrbit	PLLFRQ,1
PDIV2		avrbit	PLLFRQ,2
PDIV3		avrbit	PLLFRQ,3
PLLTM0		avrbit	PLLFRQ,4	; PLL Postcaler for High Speed Timer
PLLTM1		avrbit	PLLFRQ,5
PLLUSB		avrbit	PLLFRQ,6	; PLL Postcaler for USB Peripheral
PINMUX		avrbit	PLLFRQ,7	; PLL Input Multiplexer

CLKSEL0		sfr	0xc5		; Clock Selection Register 0
CLKS		avrbit	CLKSEL0,0	; Clock Selector
EXTE		avrbit	CLKSEL0,2	; Enable External Oscillator / Low Power Oscillator
RCE		avrbit	CLKSEL0,3	; Enable RC Oscillator
EXSUT0		avrbit	CLKSEL0,4	; SUT for External Oscillator / Low Power Oscillator
EXSUT1		avrbit	CLKSEL0,5
RCSUT0		avrbit	CLKSEL0,6	; SUT for RC Oscillator
RCSUT1		avrbit	CLKSEL0,7

CLKSEL1		sfr	0xc6		; Clock Selection Register 1
EXCKSEL0	avrbit	CLKSEL1,0	; CKSEL for External Oscillator / Low Power Oscillator
EXCKSEL1	avrbit	CLKSEL1,1
EXCKSEL2	avrbit	CLKSEL1,2
EXCKSEL3	avrbit	CLKSEL1,3
RCCKSEL0	avrbit	CLKSEL1,4	; CKSEL for RC Oscillator
RCCKSEL1	avrbit	CLKSEL1,5
RCCKSEL2	avrbit	CLKSEL1,6
RCCKSEL3	avrbit	CLKSEL1,7

CLKSTA		sfr	0xc7		; Clock Status Register
EXTON		avrbit	CLKSTA,0	; External Oscillator / Low Power Oscillator On
RCON		avrbit	CLKSTA,1	; RC Oscillator On

;----------------------------------------------------------------------------
; JTAG etc.

JTD		avrbit	MCUCR,7		; JTAG disable

CRDR		port	0x31		; Monitor Data Register
MONDR		port	0x31

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem2.inc"

EEPM0		avrbit	EECR,4		; EEPROM Programming Mode
EEPM1		avrbit	EECR,5

		include	"avr/spmcsr37.inc"
		
SIGRD		avrbit	SPMCSR,5	; Signature Read

RAMPZ		port	0x3b		; RAM Pointer
RAMPZ0		avrbit	RAMPZ,0
RAMPZ1		avrbit	RAMPZ,1

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x06 (IO) ff.
__PORTC_BITS	equ	0xc0		; (bits 6..7)
PIND		port	0x09		; Port D @ 0x09 (IO) ff.
PINE		port	0x0c		; Port E @ 0x0c (IO) ff.
__PORTE_BITS	equ	0x44		; (bits 2,6)
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.
__PORTF_BITS	equ	0xf3		; (bits 0..1,4..7)

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
		nextenum Reserved1_vect		; Reserved
		nextenum Reserved2_vect		; Reserved
		nextenum INT6_vect		; External Interrupt Request 6
		nextenum Reserved3_vect		; Reserved
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum USB_GEN_vect		; USB General Interrupt Request
		nextenum USB_COM_vect		; USB Endpoint Interrupt Request
		nextenum WDT_vect		; Watchdog Time-out Interrupt
		nextenum Reserved4_vect		; Reserved
		nextenum Reserved5_vect		; Reserved
		nextenum Reserved6_vect		; Reserved
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
		nextenum TIMER4_COMPA_vect	; Timer/Counter 4 Compare Match A
		nextenum TIMER4_COMPB_vect	; Timer/Counter 4 Compare Match B
		nextenum TIMER4_COMPD_vect	; Timer/Counter 4 Compare Match D
		nextenum TIMER4_OVF_vect	; Timer/Counter 4 Overflow
		nextenum TIMER4_FPF_vect	; Timer/Counter 4 Fault Protection

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
INT6		avrbit	EIMSK,6		; External Interrupt Request 6 Enable

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1	        avrbit	EIFR,1		; External Interrupt 1 Occured
INTF2		avrbit	EIFR,2		; External Interrupt 2 Occured
INTF3	        avrbit	EIFR,3		; External Interrupt 3 Occured
INTF6		avrbit	EIFR,6		; External Interrupt 6 Occured

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

TCCR4A		sfr	0xc0		; Timer/Counter 4 Control Register A
PWM4B		avrbit	TCCR4A,0	; Timer/Counter 4 Pulse Width Modulator B Enable
PWM4A		avrbit	TCCR4A,1	; Timer/Counter 4 Pulse Width Modulator A Enable
FOC4B		avrbit	TCCR4A,2	; Timer/Counter 4 Force Output Compare B
FOC4A		avrbit	TCCR4A,3	; Timer/Counter 4 Force Output Compare A
COM4B0		avrbit	TCCR4A,4	; Timer/Counter 4 Compare Mode B
COM4B1		avrbit	TCCR4A,5
COM4A0		avrbit	TCCR4A,6	; Timer/Counter 4 Compare Mode A
COM4A1		avrbit	TCCR4A,7
TCCR4B		sfr	0xc1		; Timer/Counter 4 Control Register B
CS40		avrbit	TCCR4B,0	; Timer/Counter 4 Clock Select
CS41		avrbit	TCCR4B,1
CS42		avrbit	TCCR4B,2
CS43		avrbit	TCCR4B,3
DTPS40		avrbit	TCCR4B,4	; Timer/Counter 4 Dead Time Prescaler
DTPS41		avrbit	TCCR4B,5
PSR4		avrbit	TCCR4B,6	; Timer/Counter 4 Prescaler Reset
PWM4X		avrbit	TCCR4B,7	; Timer/Counter 4 PWM Inversion Mode
TCCR4C		sfr	0xc2		; Timer/Counter 4 Control Register C
PWM4D		avrbit	TCCR4C,0	; Timer/Counter 4 Pulse Width Modulator D Enable
FOC4D		avrbit	TCCR4C,1	; Timer/Counter 4 Force Output Compare D
COM4D0		avrbit	TCCR4C,2	; Timer/Counter 4 Compare Mode D
COM4D1		avrbit	TCCR4C,3
COMAB0S		avrbit	TCCR4C,4	; Timer/Counter 4 Comparator B Output Mode
COM4B1S		avrbit	TCCR4C,5
COM4A0S		avrbit	TCCR4C,6	; Timer/Counter 4 Comparator A Output Mode
COM4A1S		avrbit	TCCR4C,7
TCCR4D		sfr	0xc3		; Timer/Counter 4 Control Register D
WGM40		avrbit	TCCR4D,0	; Timer/Counter 4 Waveform Generation Mode
WGM41		avrbit	TCCR4D,1
FPF4		avrbit	TCCR4D,2	; Timer/Counter 4 Fault Protection Interrupt Flag
FPAC4		avrbit	TCCR4D,3	; Timer/Counter 4 Fault Protection Analog Comparator Enable
FPES4		avrbit	TCCR4D,4	; Timer/Counter 4 Fault Protection Edge Select
FPNC4		avrbit	TCCR4D,5	; Timer/Counter 4 Fault Protection Noise Canceler
FPEN4		avrbit	TCCR4D,6	; Timer/Counter 4 Fault Protection Mode Enable
FPIE4		avrbit	TCCR4D,7	; Timer/Counter 4 Fault Protection Interrupt Enable
TCCR4E		sfr	0xc4		; Timer/Counter 4 Control Register E
OC4OE0		avrbit	TCCR4E,0	; Timer/Counter 4 Output Compare Override Enable
OC4OE1		avrbit	TCCR4E,1
OC4OE2		avrbit	TCCR4E,2
OC4OE3		avrbit	TCCR4E,3
OC4OE4		avrbit	TCCR4E,4
OC4OE5		avrbit	TCCR4E,5
ENHC4		avrbit	TCCR4E,6	; Timer/Counter 4 Enhanced Compare/PWM Mode
TLOCK4		avrbit	TCCR4E,7	; Timer/Counter 4 Register Update Lock
TCNT4		sfr	0xbe		; Timer/Counter 4 Value 
TC4H		sfr	0xbf		; Timer/Counter 4 Value MSB
OCR4A		sfr	0xcf		; Timer/Counter 4 Output Compare Value A
OCR4B		sfr	0xd0		; Timer/Counter 4 Output Compare Value B
OCR4C		sfr	0xd1		; Timer/Counter 4 Output Compare Value C
OCR4D		sfr	0xd2		; Timer/Counter 4 Output Compare Value D
DT4		sfr	0xd4		; Timer/Counter 4 Dead Time Value
DT4L0		avrbit	DT4,0		; Timer/Counter 4 Dead Time Value for /OC4x Output
DT4L1		avrbit	DT4,1
DT4L2		avrbit	DT4,2
DT4L3		avrbit	DT4,3
DT4H0		avrbit	DT4,4		; Timer/Counter 4 Dead Time Value for OC4x Output
DT4H1		avrbit	DT4,5
DT4H2		avrbit	DT4,6
DT4H3		avrbit	DT4,7

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

TIMSK3		sfr	0x71		; Timer/Counter 3 Interrupt Mask Register
TOIE3		avrbit	TIMSK3,0	; Timer/Counter 3 Overflow Interrupt Enable
OCIE3A		avrbit	TIMSK3,1	; Timer/Counter 3 Output Compare Interrupt Enable A
OCIE3B		avrbit	TIMSK3,2	; Timer/Counter 3 Output Compare Interrupt Enable B
OCIE3C		avrbit	TIMSK3,3	; Timer/Counter 3 Output Compare Interrupt Enable C
ICIE3		avrbit	TIMSK3,5	; Timer/Counter 3 Input Capture Interrupt Enable

TIFR3		port	0x18		; Timer/Counter 3 Interrupt Flag Register

TIMSK4		sfr	0x72		; Timer/Counter 4 Interrupt Mask Register
TOIE4		avrbit	TIMSK4,2	; Timer/Counter 4 Overflow Interrupt Enable
OCIE4B		avrbit	TIMSK4,5	; Timer/Counter 4 Output Compare Interrupt Enable B
OCIE4A		avrbit	TIMSK4,6	; Timer/Counter 4 Output Compare Interrupt Enable A
OCIE4D		avrbit	TIMSK4,7	; Timer/Counter 4 Output Compare Interrupt Enable D

TIFR4		port	0x19		; Timer/Counter 4 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset for Synchronous Timer/Counters
PSRASY		avrbit	GTCCR,1
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"
		
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

		include "avr/spim2c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"
		
;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm30.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

MUX4		avrbit	ADMUX,4

ADTS3		avrbit	ADCSRB,3
MUX5		avrbit	ADCSRB,4
ADHSM		avrbit	ADCSRB,7	; A/D Converter High Speed Mode

DIDR2		sfr	0x7d		; Digital Input Disable Register 2
ADC8D		avrbit	DIDR2,0		; ADC8 Input Disable
ADC9D		avrbit	DIDR2,1		; ADC9 Input Disable
ADC10D		avrbit	DIDR2,2		; ADC10 Input Disable
ADC11D		avrbit	DIDR2,3		; ADC11 Input Disable
ADC12D		avrbit	DIDR2,4		; ADC12 Input Disable
ADC13D		avrbit	DIDR2,5		; ADC13 Input Disable

;----------------------------------------------------------------------------
; USB

		include	"avr/usbm.inc"

OTGPADE		avrbit	USBCON,4	; VBUS Pad Enable
VBUSTE		avrbit	USBCON,0	; VBUS Transition Interrupt Enable Bit

LSM		avrbit	UDCON,2		; USB Device Low Speed Mode Selection
RSTCPU		avrbit	UDCON,3		; USB Reset CPU Bit

MSOFI		avrbit	UDINT,1

MSOFE		avrbit	UDIEN,1

UHWCON		sfr	0xd7		; USB Hardware Control
UVREGE		avrbit	UHWCON,0	; USB Pad Regulator Enable

USBSTA		sfr	0xd9		; 
VBUS		avrbit	USBSTA,0	; VBus Flag
ID		avrbit	USBSTA,1	; ID status

USBINT		sfr	0xda		; 
VBUSTI		avrbit	USBINT,0	; IVBUS Transition Interrupt Flag

UEBCHX		sfr	0xf3		; Byte Count High

		restore			; re-enable Listing

		endif			; __regmxu4inc
