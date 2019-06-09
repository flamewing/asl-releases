		ifndef	__regmxx01inc
__regmxx01inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMXX01.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega 640/1280/1281/2560/2561 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Control

MCUCR		port	0x35		; MCU Control Register
IVCE		avrbit	MCUCR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select

MCUSR		port	0x34		; MCU Status Register
PORF		avrbit	MCUSR,0		; Power-On Reset Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
BORF		avrbit	MCUSR,2		; brown Out Reset Occured
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Mode Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2
SM2		avrbit	SMCR,3

PRR0		sfr	0x64		; Power Reduction Register 0
PRADC		avrbit	PRR0,0		; Power Reduction ADC
PRUSART0	avrbit	PRR0,1		; Power Reduction USART0
PRSPI		avrbit	PRR0,2		; Power Reduction Serial Peripheral Interface
PRTIM1		avrbit	PRR0,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR0,5		; Power Reduction Timer/Counter 0
PRTIM2		avrbit	PRR0,6		; Power Reduction Timer/Counter 2
PRTWI		avrbit	PRR0,7		; Power ReductionTwo Wire Interface

PRR1		sfr	0x65		; Power Reduction Register 1
PRUSART1	avrbit	PRR1,0		; Power Reduction USART1
PRUSART2	avrbit	PRR1,1		; Power Reduction USART2
PRUSART3	avrbit	PRR1,2		; Power Reduction USART3
PRTIM3		avrbit	PRR1,3		; Power Reduction Timer/Counter 3
PRTIM4		avrbit	PRR1,4		; Power Reduction Timer/Counter 4
PRTIM5		avrbit	PRR1,5		; Power Reduction Timer/Counter 5

OSCCAL		sfr	0x66		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescale Register
CLKPS0		avrbit	CLKPR,0		; Clock Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

XMCRA		sfr	0x74		; External Memory Control Register A
SRE		avrbit	XMCRA,7		; Enable External SRAM
SRL2		avrbit	XMCRA,6		; Wait State Sector Limit
SRL1		avrbit	XMCRA,5
SRL0		avrbit	XMCRA,4
SRW11		avrbit	XMCRA,3		; Wait State Select Bits for Lower Sector
SRW10		avrbit	XMCRA,2
SRW01		avrbit	XMCRA,1
SRW00		avrbit	XMCRA,0

XMCRB		sfr	0x75		; External Memory Control Register B
XMBK		avrbit	XMCRB,7		; External Memory Bus Keeper Enable
XMM2		avrbit	XMCRB,2		; External Memory High Mask
XMM1		avrbit	XMCRB,1
XMM0		avrbit	XMCRB,0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem2.inc"
		include	"spmcsr37.inc"

EEPM0		avrbit	EECR,4		; EEPROM Programming Mode
EEPM1		avrbit	EECR,5

SIGRD		avrbit	SPMCSR,5	; Signature Row Read

;----------------------------------------------------------------------------
; JTAG

JTD		avrbit	MCUCR,7		; JTAG Disable

JTRF		avrbit	MCUSR,4		; JTAG Reset Occured

OCDR		port	0x31		; On-Chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,4		; Pull-Up Disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x03 (IO) ff.
PIND		port	0x09		; Port D @ 0x09 (IO) ff.
PINE		port	0x0c		; Port E @ 0x0c (IO) ff.
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.
PING		port	0x12		; Port G @ 0x12 (IO) ff.
		ifdef	ATmegaxx0
PINH		 sfr	0x100		; Port H @ 0x100 (DATA) ff.
PINJ		 sfr	0x103		; Port J @ 0x103 (DATA) ff.
PINK		 sfr	0x106		; Port K @ 0x106 (DATA) ff.
PINL		 sfr	0x109		; Port L @ 0x109 (DATA) ff.
		endif

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1
		ifdef	ATmegaxx0
PCMSK2		 sfr	 0x6d		; Pin Change Mask Register 2
		endif
PCICR		sfr	0x68		; Pin Change Interrupt Control Register
PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

GPIOR0		port	0x1e		; General Purpose I/O Registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

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
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum PCINT1_vect		; Pin Change Interrupt Request 1
		ifdef	ATmegaxx0
		 nextenum PCINT2_vect		; Pin Change Interrupt Request 2
		elseif
		 nextenum NOT_USED1_vect	; unused on x1 versions
		endif
		nextenum WDT_vect		; Watchdog Time-Out Interrupt
		nextenum TIMER2_COMPA_vect	; Timer/Counter 2 Compare Match A
		nextenum TIMER2_COMPB_vect	; Timer/Counter 2 Compare Match B
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_COMPC_vect	; Timer/Counter 1 Compare Match C
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum ANALOG_COMP_vect	; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum TIMER3_CAPT_vect	; Timer/Counter 3 Capture Event
		nextenum TIMER3_COMPA_vect	; Timer/Counter 3 Compare Match A
		nextenum TIMER3_COMPB_vect	; Timer/Counter 3 Compare Match B
		nextenum TIMER3_COMPC_vect	; Timer/Counter 3 Compare Match C
		nextenum TIMER3_OVF_vect	; Timer/Counter 3 Overflow
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum TWI_vect		; Two-Wire Serial interface
		nextenum SPM_READY_vect		; Store Program Memory Ready
		ifdef	ATmegaxx0
		 nextenum TIMER4_CAPT_vect	; Timer/Counter 4 Capture Event
		elseif
		 nextenum NOT_USED2_vect	; unused on x1 versions
		endif
		nextenum TIMER4_COMPA_vect	; Timer/Counter 4 Compare Match A
		nextenum TIMER4_COMPB_vect	; Timer/Counter 4 Compare Match B
		nextenum TIMER4_COMPC_vect	; Timer/Counter 4 Compare Match C
		nextenum TIMER4_OVF_vect	; Timer/Counter 4 Overflow
		ifdef	ATmegaxx0
		 nextenum TIMER5_CAPT_vect	; Timer/Counter 5 Capture Event
		elseif
		 nextenum NOT_USED3_vect	; unused on x1 versions
		endif
		nextenum TIMER5_COMPA_vect	; Timer/Counter 5 Compare Match A
		nextenum TIMER5_COMPB_vect	; Timer/Counter 5 Compare Match B
		nextenum TIMER5_COMPC_vect	; Timer/Counter 5 Compare Match C
		nextenum TIMER5_OVF_vect	; Timer/Counter 5 Overflow
		ifdef   ATmegaxx0
		 nextenum USART2_RX_vect	; USART2 Rx Complete
		 nextenum USART2_UDRE_vect	; USART2 Data Register Empty
		 nextenum USART2_TX_vect	; USART2 Tx Complete
		 nextenum USART3_RX_vect	; USART3 Rx Complete
		 nextenum USART3_UDRE_vect	; USART3 Data Register Empty
		 nextenum USART3_TX_vect	; USART3 Tx Complete
		endif

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

EICRB		sfr	0x6a		; External Interrupt Control Register B
ISC40		avrbit	EICRB,0		; External Interrupt 4 Sense Control
ISC41		avrbit	EICRB,1
ISC50		avrbit	EICRB,2		; External Interrupt 5 Sense Control
ISC51		avrbit	EICRB,3
ISC60		avrbit	EICRB,4		; External Interrupt 6 Sense Control
ISC61		avrbit	EICRB,5
ISC70		avrbit	EICRB,6		; External Interrupt 7 Sense Control
ISC71		avrbit	EICRB,7

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1
INT2		avrbit	EIMSK,2		; Enable External Interrupt 2
INT3		avrbit	EIMSK,3		; Enable External Interrupt 3
INT4		avrbit	EIMSK,4		; Enable External Interrupt 4
INT5		avrbit	EIMSK,5		; Enable External Interrupt 5
INT6		avrbit	EIMSK,6		; Enable External Interrupt 6
INT7		avrbit	EIMSK,7		; Enable External Interrupt 7

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1		avrbit	EIFR,1		; External Interrupt 1 Occured
INTF2		avrbit	EIFR,2		; External Interrupt 2 Occured
INTF3		avrbit	EIFR,3		; External Interrupt 3 Occured
INTF4		avrbit	EIFR,4		; External Interrupt 4 Occured
INTF5		avrbit	EIFR,5		; External Interrupt 5 Occured
INTF6		avrbit	EIFR,6		; External Interrupt 6 Occured
INTF7		avrbit	EIFR,7		; External Interrupt 7 Occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset
PSRASY		avrbit	GTCCR,1		; Prescaler Reset Timer/Counter2
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x25		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x26		; Timer/Counter 0 Value
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

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
WGM12		avrbit	TCCR1B,3
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
WGM32		avrbit	TCCR3B,3
WGM33		avrbit	TCCR3B,4
ICES3		avrbit	TCCR3B,6	; Timer/Counter 3 Capture Slope Selection
ICNC3		avrbit	TCCR3B,7	; Timer/Counter 3 Capture Noise Filter
TCCR3C		sfr	0x92		; Timer/Counter 3 Control Register C
FOC3C		avrbit	TCCR3C,5	; Timer/Counter 3 Force Output Compare C
FOC3B		avrbit	TCCR3C,6	; Timer/Counter 3 Force Output Compare B
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

TCCR4A		sfr	0xa0		; Timer/Counter 4 Control Register A
WGM40		avrbit	TCCR4A,0	; Timer/Counter 4 Waveform Generation Mode
WGM41		avrbit	TCCR4A,1
COM4C0		avrbit	TCCR4A,2	; Timer/Counter 4 Compare Mode C
COM4C1		avrbit	TCCR4A,3
COM4B0		avrbit	TCCR4A,4	; Timer/Counter 4 Compare Mode B
COM4B1		avrbit	TCCR4A,5
COM4A0		avrbit	TCCR4A,6	; Timer/Counter 4 Compare Mode A
COM4A1		avrbit	TCCR4A,7
TCCR4B		sfr	0xa1		; Timer/Counter 4 Control Register B
CS40		avrbit	TCCR4B,0	; Timer/Counter 4 Prescaler Setting
CS41		avrbit	TCCR4B,1
CS42		avrbit	TCCR4B,2
WGM42		avrbit	TCCR4B,3
WGM43		avrbit	TCCR4B,4
ICES4		avrbit	TCCR4B,6	; Timer/Counter 4 Capture Slope Selection
ICNC4		avrbit	TCCR4B,7	; Timer/Counter 4 Capture Noise Filter
TCCR4C		sfr	0xa2		; Timer/Counter 4 Control Register C
FOC4C		avrbit	TCCR4C,5	; Timer/Counter 4 Force Output Compare C
FOC4B		avrbit	TCCR4C,6	; Timer/Counter 4 Force Output Compare B
FOC4A		avrbit	TCCR4C,7	; Timer/Counter 4 Force Output Compare A
TCNT4L		sfr	0xa4		; Timer/Counter 4 Value LSB
TCNT4H		sfr	0xa5		; Timer/Counter 4 Value MSB
OCR4AL		sfr	0xa8		; Timer/Counter 4 Output Compare Value A LSB
OCR4AH		sfr	0xa9		; Timer/Counter 4 Output Compare Value A MSB
OCR4BL		sfr	0xaa		; Timer/Counter 4 Output Compare Value B LSB
OCR4BH		sfr	0xab		; Timer/Counter 4 Output Compare Value B MSB
OCR4CL		sfr	0xac		; Timer/Counter 4 Output Compare Value C LSB
OCR4CH		sfr	0xad		; Timer/Counter 4 Output Compare Value C MSB
ICR4L		sfr	0xa6		; Timer/Counter 4 Input Capture Value LSB
ICR4H		sfr	0xa7		; Timer/Counter 4 Input Capture Value MSB

TCCR5A		sfr	0x120		; Timer/Counter 5 Control Register A
WGM50		avrbit	TCCR5A,0	; Timer/Counter 5 Waveform Generation Mode
WGM51		avrbit	TCCR5A,1
COM5C0		avrbit	TCCR5A,2	; Timer/Counter 5 Compare Mode C
COM5C1		avrbit	TCCR5A,3
COM5B0		avrbit	TCCR5A,4	; Timer/Counter 5 Compare Mode B
COM5B1		avrbit	TCCR5A,5
COM5A0		avrbit	TCCR5A,6	; Timer/Counter 5 Compare Mode A
COM5A1		avrbit	TCCR5A,7
TCCR5B		sfr	0x121		; Timer/Counter 5 Control Register B
CS50		avrbit	TCCR5B,0	; Timer/Counter 5 Prescaler Setting
CS51		avrbit	TCCR5B,1
CS52		avrbit	TCCR5B,2
WGM52		avrbit	TCCR5B,3
WGM53		avrbit	TCCR5B,4
ICES5		avrbit	TCCR5B,6	; Timer/Counter 5 Capture Slope Selection
ICNC5		avrbit	TCCR5B,7	; Timer/Counter 5 Capture Noise Filter
TCCR5C		sfr	0x122		; Timer/Counter 5 Control Register C
FOC5C		avrbit	TCCR5C,5	; Timer/Counter 5 Force Output Compare C
FOC5B		avrbit	TCCR5C,6	; Timer/Counter 5 Force Output Compare B
FOC5A		avrbit	TCCR5C,7	; Timer/Counter 5 Force Output Compare A
TCNT5L		sfr	0x124		; Timer/Counter 5 Value LSB
TCNT5H		sfr	0x125		; Timer/Counter 5 Value MSB
OCR5AL		sfr	0x128		; Timer/Counter 5 Output Compare Value A LSB
OCR5AH		sfr	0x129		; Timer/Counter 5 Output Compare Value A MSB
OCR5BL		sfr	0x12a		; Timer/Counter 5 Output Compare Value B LSB
OCR5BH		sfr	0x12b		; Timer/Counter 5 Output Compare Value B MSB
OCR5CL		sfr	0x12c		; Timer/Counter 5 Output Compare Value C LSB
OCR5CH		sfr	0x12d		; Timer/Counter 5 Output Compare Value C MSB
ICR5L		sfr	0x126		; Timer/Counter 5 Input Capture Value LSB
ICR5H		sfr	0x127		; Timer/Counter 5 Input Capture Value MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable B
TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable B
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Enable
TIMSK2		sfr	0x70		; Timer/Counter 2 Interrupt Mask Register
TOIE2		avrbit	TIMSK2,0	; Timer/Counter 2 Overflow Interrupt Enable
OCIE2A		avrbit	TIMSK2,1	; Timer/Counter 2 Output Compare Interrupt Enable A
OCIE2B		avrbit	TIMSK2,2	; Timer/Counter 2 Output Compare Interrupt Enable B
TIMSK3		sfr	0x71		; Timer/Counter 3 Interrupt Mask Register
TOIE3		avrbit	TIMSK3,0	; Timer/Counter 3 Overflow Interrupt Enable
OCIE3A		avrbit	TIMSK3,1	; Timer/Counter 3 Output Compare Interrupt Enable A
OCIE3B		avrbit	TIMSK3,2	; Timer/Counter 3 Output Compare Interrupt Enable B
ICIE3		avrbit	TIMSK3,5	; Timer/Counter 3 Input Capture Enable
TIMSK4		sfr	0x72		; Timer/Counter 4 Interrupt Mask Register
TOIE4		avrbit	TIMSK4,0	; Timer/Counter 4 Overflow Interrupt Enable
OCIE4A		avrbit	TIMSK4,1	; Timer/Counter 4 Output Compare Interrupt Enable A
OCIE4B		avrbit	TIMSK4,2	; Timer/Counter 4 Output Compare Interrupt Enable B
ICIE4		avrbit	TIMSK4,5	; Timer/Counter 4 Input Capture Enable
TIMSK5		sfr	0x73		; Timer/Counter 5 Interrupt Mask Register
TOIE5		avrbit	TIMSK5,0	; Timer/Counter 5 Overflow Interrupt Enable
OCIE5A		avrbit	TIMSK5,1	; Timer/Counter 5 Output Compare Interrupt Enable A
OCIE5B		avrbit	TIMSK5,2	; Timer/Counter 5 Output Compare Interrupt Enable B
ICIE5		avrbit	TIMSK5,5	; Timer/Counter 5 Input Capture Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Status Register
TIFR1		port	0x16		; Timer/Counter 1 Interrupt Status Register
TIFR2		port	0x17		; Timer/Counter 2 Interrupt Status Register
TIFR3		port	0x18		; Timer/Counter 3 Interrupt Status Register
TIFR4		port	0x19		; Timer/Counter 4 Interrupt Status Register
TIFR5		port	0x1a		; Timer/Counter 5 Interrupt Status Register

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2BUB		avrbit	ASSR,0		; Timer/Counter Control Register 2 B Update Busy
TCR2AUB		avrbit	ASSR,1		; Timer/Counter Control Register 2 A Update Busy
OCR2BUB		avrbit	ASSR,2		; Output Compare Register 2 B Update Busy
OCR2AUB		avrbit	ASSR,3		; Output Compare Register 2 A Update Busy
TCN2UB		avrbit	ASSR,4		; Timer/Counter 2 Update Busy
AS2		avrbit	ASSR,5		; Asynchronous Timer/Counter 2
EXCLK		avrbit	ASSR,6		; Enable External Clock Input

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdme.inc"

;----------------------------------------------------------------------------
; USART

__USART0_SPI__	equ	1
		include	"usartc0.inc"
__USART1_SPI__	equ	1
		include	"usartc8.inc"

                ifdef   ATmegaxx0

UDR2		sfr	0xd6		; USART2 I/O Data Register

UCSR2A		sfr	0xd0		; USART2 Control & Status Register A
MPCM2		avrbit	UCSR2A,0	; USART2 Multi Processor Communication Mode
U2X2		avrbit	UCSR2A,1	; USART2 Double Transmission Speed
UPE2		avrbit	UCSR2A,2	; USART2 Parity Error
DOR2		avrbit	UCSR2A,3	; USART2 Overrun
FE2		avrbit	UCSR2A,4	; USART2 Framing Error
UDRE2		avrbit	UCSR2A,5	; USART2 Data Register Empty
TXC2		avrbit	UCSR2A,6	; USART2 Transmit Complete
RXC2		avrbit	UCSR2A,7	; USART2 Receive Complete

UCSR2B		sfr	0xd1		; USART2 Control & Status Register B
TXB82		avrbit	UCSR2B,0	; USART2 Transmit Bit 8
RXB82		avrbit	UCSR2B,1	; USART2 Receive Bit 8
UCSZ22		avrbit	UCSR2B,2	; USART2 Character Size
TXEN2		avrbit	UCSR2B,3	; USART2 Enable Transmitter
RXEN2		avrbit	UCSR2B,4	; USART2 Enable Receiver
UDRIE2		avrbit	UCSR2B,5	; USART2 Enable Data Register Empty Interrupt
TXCIE2		avrbit	UCSR2B,6	; USART2 Enable Transmit Complete Interrupt
RXCIE2		avrbit	UCSR2B,7	; USART2 Enable Receive Complete Interrupt

UCSR2C		sfr	0xd2		; USART2 Control & Status Register C
UCPOL2		avrbit	UCSR2C,0	; USART2 Clock Polarity
UCSZ20		avrbit	UCSR2C,1	; USART2 Character Size
UCSZ21		avrbit	UCSR2C,2
USBS2		avrbit	UCSR2C,3	; USART2 Stop Bit Select
UPM20		avrbit	UCSR2C,4	; USART2 Parity Mode : Odd/Even
UPM21		avrbit	UCSR2C,5	; USART2 Parity Mode : Enable/Disable
UMSEL20		avrbit	UCSR2C,6	; USART2 USART Mode Select
UMSEL21		avrbit	UCSR2C,7

UBRR2H		sfr	0xd5		; USART2 Baud Rate Register MSB
UBRR2L		sfr	0xd4		; USART2 Baud Rate Register LSB

UDR3		sfr	0x136		; USART3 I/O Data Register

UCSR3A		sfr	0x130		; USART3 Control & Status Register A
MPCM3		avrbit	UCSR3A,0	; USART3 Multi Processor Communication Mode
U2X3		avrbit	UCSR3A,1	; USART3 Double Transmission Speed
UPE3		avrbit	UCSR3A,2	; USART3 Parity Error
DOR3		avrbit	UCSR3A,3	; USART3 Overrun
FE3		avrbit	UCSR3A,4	; USART3 Framing Error
UDRE3		avrbit	UCSR3A,5	; USART3 Data Register Empty
TXC3		avrbit	UCSR3A,6	; USART3 Transmit Complete
RXC3		avrbit	UCSR3A,7	; USART3 Receive Complete

UCSR3B		sfr	0x131		; USART3 Control & Status Register B
TXB83		avrbit	UCSR3B,0	; USART3 Transmit Bit 8
RXB83		avrbit	UCSR3B,1	; USART3 Receive Bit 8
UCSZ32		avrbit	UCSR3B,2	; USART3 Character Size
TXEN3		avrbit	UCSR3B,3	; USART3 Enable Transmitter
RXEN3		avrbit	UCSR3B,4	; USART3 Enable Receiver
UDRIE3		avrbit	UCSR3B,5	; USART3 Enable Data Register Empty Interrupt
TXCIE3		avrbit	UCSR3B,6	; USART3 Enable Transmit Complete Interrupt
RXCIE3		avrbit	UCSR3B,7	; USART3 Enable Receive Complete Interrupt

UCSR3C		sfr	0x132		; USART3 Control & Status Register C
UCPOL3		avrbit	UCSR3C,0	; USART3 Clock Polarity
UCSZ30		avrbit	UCSR3C,1	; USART3 Character Size
UCSZ31		avrbit	UCSR3C,2
USBS3		avrbit	UCSR3C,3	; USART3 Stop Bit Select
UPM30		avrbit	UCSR3C,4	; USART3 Parity Mode : Odd/Even
UPM31		avrbit	UCSR3C,5	; USART3 Parity Mode : Enable/Disable
UMSEL30		avrbit	UCSR3C,6	; USART3 USART Mode Select
UMSEL31		avrbit	UCSR3C,7

UBRR3H		sfr	0x135		; USART3 Baud Rate Register MSB
UBRR3L		sfr	0x134		; USART3 Baud Rate Register LSB

		endif

;----------------------------------------------------------------------------
; SPI

		include	"spim2c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"twimb8.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"adcm78.inc"

MUX4		avrbit	ADMUX,4
MUX5		avrbit	ADCSRB,3

DIDR2		sfr	0x7d		; Digital Input Disable Register 2
ADC8D		avrbit	DIDR2,0		; Disable Digital Input on ADC8
ADC9D		avrbit	DIDR2,1		; Disable Digital Input on ADC9
ADC10D		avrbit	DIDR2,2		; Disable Digital Input on ADC10
ADC11D		avrbit	DIDR2,3		; Disable Digital Input on ADC11
ADC12D		avrbit	DIDR2,4		; Disable Digital Input on ADC12
ADC13D		avrbit	DIDR2,5		; Disable Digital Input on ADC13
ADC14D		avrbit	DIDR2,6		; Disable Digital Input on ADC14
ADC15D		avrbit	DIDR2,7		; Disable Digital Input on ADC15

;----------------------------------------------------------------------------
; Analog Comparator

		include	"acm30.inc"

		restore			; re-enable listing

		endif			; __regmxx01inc
