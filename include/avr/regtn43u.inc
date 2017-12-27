		ifndef	__regtn43uinc
__regtn43uinc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN43U.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny43U                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x15f,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		equ	2		; BOD Sleep
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Enable
BODS		equ	7		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

PRR		port	0x00		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSI		equ	1		; Power Reduction USI
PRTIM0		equ	2		; Power Reduction Timer/Counter 0
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRE0		equ	5		; Prepared Read Enable
PRE1		equ	6
PRE2		equ	7

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SPMEN		equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x27		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	6		; Pull-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

GPIOR0		port	0x13		; General Purpose I/O Register 0
GPIOR1		port	0x14		; General Purpose I/O Register 1
GPIOR2		port	0x15		; General Purpose I/O Register 2

DIDR0		port	0x01		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Disable
ADC1D		equ	1		; ADC1 Digital Input Disable
ADC2D		equ	2		; ADC2 Digital Input Disable
ADC3D		equ	3		; ADC3 Digital Input Disable
AIN0D		equ	4		; Analog Comparator Digital Input 0 Disable
AIN1D		equ	5		; Analog Comparator Digital Input 1 Disable

PCMSK0		port	0x12		; Pin Change Interrupt Mask 0
PCMSK1		port	0x20		; Pin Change Interrupt Mask 1

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; Enable External Interrupt 0
PCIE1		equ	5		; Pin Change Interrupt Enable 1
PCIE0		equ	4		; Pin Change Interrupt Enable 0

GIFR		port	0x3a		; General Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 Occured
PCIF1		equ	5		; Pin Change Interrupt 1 Occured
PCIF0		equ	4		; Pin Change Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x30		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 0 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x3c		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10            equ     0               ; Timer/Counter 1 Clock Select
CS11            equ     1
CS12            equ     2
WGM12		equ	3
FOC1B		equ	6		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	7		; Timer/Counter 1 Force Output Compare A
TCNT1		port	0x2d		; Timer/Counter 1 Value
OCR1A		port	0x2c		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x2b		; Timer/Counter 1 Output Compare Value B

TIMSK0		port	0x39		; Timer/Counter Interrupt Mask Register 0
TOIE0		equ	0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable B

TIMSK1		port	0x0c		; Timer/Counter Interrupt Mask Register 1
TOIE1		equ	0		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		equ	1		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		equ	2		; Timer/Counter 1 Output Compare Interrupt Enable B

TIFR0		port	0x38		; Timer Interrupt Status Register 0
TIFR1		port	0x0b		; Timer Interrupt Status Register 1

GTCCR		port	0x23		; General Timer/Counter Control 1 Register
PSR10		equ	0		; Prescaler Reset Timer/Counter 0/1
TSM		equ	7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x21		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDCE		equ	4		; Change Enable
WDP3		equ	5
WDIE		equ	6		; Enable Watchdog Interrupt
WDIF		equ	7		; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x07		; Multiplexer Selection
REFS		equ	6		; Reference Selection
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		equ	7		; Enable ADC
ADSC		equ	6		; Start Conversion
ADATE		equ	5		; ADC Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x03		; Control/Status Register B
BS		equ	7		; Boost Status
ACME		equ	6		; Analog Comparator Multiplexer Enable
ADLAR		equ	4		; Left Adjust Right
ADTS2		equ	2		; Auto Trigger Source
ADTS1		equ	1
ADTS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

USIBR		port	0x10		; USI Buffer Register

		restore

		endif			; __regtn43uinc
