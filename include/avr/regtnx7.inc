		ifndef	__regtnx7inc
__regtnx7inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX7.INC                                             *
;*                                                                          *
;*   Contains common bit & register definitions for ATtiny87/167            *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		equ	5		; BOD Sleep
BODS		equ	6		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

SMCR		port	0x33		; Sleep Mode Control Register
SE		equ	0		; Sleep Enable
SM0		equ	1		; Sleep Mode Select
SM1		equ	2

OSCCAL		sfr	0x66		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

CLKCSR		sfr	0x62		; Clock Control/Status Register
CLKC0		equ	0		; Clock Control Bits
CLKC1		equ	1
CLKC2		equ	2
CLKC3		equ	3
CLKRDY		equ	4		; Clock Ready Flag
CLKCCE		equ	7		; Clock Control Change Enable

CLKSELR		sfr	0x63		; Clock Selection Register
CSEL0		equ	0		; Clock Source Select
CSEL1		equ	1
CSEL2		equ	2
CSEL3		equ	3
CSUT0		equ	4		; Clock Start-up Time
CSUT1		equ	5
COUT		equ	6		; Clock Out

PRR		sfr	0x64		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSI		equ	1		; Power Reduction USI
PRTIM1		equ	2		; Power Reduction Timer/Counter 1
PRTIM0		equ	3		; Power Reduction Timer/Counter 0
PRSPI		equ	4		; Power Reduction  SPI
PRLIN		equ	5		; Power Reduction Power Reduction LIN / UART controller

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x21		; EEPROM Address Register Low
EEARH		port	0x22		; EEPROM Address Register High
EEDR		port	0x20		; EEPROM Data Register
EECR		port	0x1f		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RWWSB		equ	6		; Read-While-Write Section Busy
SIGRD		equ	5		; Read Signature Bytes
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SPMEN		equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	4		; Pull-Up Disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PORTCR		port	0x12		; Port Control Register
PUDA		equ	0		; Pull-up Disable Port A
PUDB		equ	1		; Pull-up Disable Port B
BBMA		equ	4		; Break-Before-Make Mode Enable A
BBMB		equ	5		; Break-Before-Make Mode Enable B

PCMSK0		sfr	0x6b		; Pin Change Interrupt Mask 0
PCMSK1		sfr	0x6c		; Pin Change Interrupt Mask 1

PCICR		sfr	0x68		; Pin Change Interrupt Control Register

PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum LIN_TC_vect		; LIN/UART Transfer Complete
		nextenum LIN_ERR_vect		; LIN/UART Error
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1
ISC10		equ	2		; External Interrupt 1 Sense Control
ISC11		equ	3

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; Enable External Interrupt 0
INT1		equ	1		; Enable External Interrupt 1

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; External Interrupt 0 Occured
INTF1		equ	1		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x25		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x26		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
FOC0A		equ	7		; Timer/Counter 0 Forc Output Compare Match
TCNT0		port	0x27		; Timer/Counter 0 Value
OCR0A		port	0x28		; Timer/Counter 0 Output Compare Value A
ASSR		sfr	0xb6		; Asynchronous Status Register
TCR0BUB		equ	0		; Timer/Counter0 Control Register B Update Busy
TCR0AUB		equ	1		; Timer/Counter0 Control Register A Update Busy
OCR0AUB		equ	3		; Output Compare 0 Register A Update Busy
TCN0UB		equ	4		; Timer/Counter0 Update Busy
AS0		equ	5		; Asynchronous Timer/Counter 0
EXCLK		equ	6		; Enable External Clock Input

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	7
TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		equ     0               ; Timer/Counter 1 Clock Select
CS11		equ     1
CS12		equ     2
WGM12		equ	3
WGM13		equ	4
ICES1		equ	6		; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		equ	7		; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		sfr	0x82		; Timer/Counter 1 Control Register C
FOC1B		equ	6		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	7		; Timer/Counter 1 Force Output Compare A
TCCR1D		sfr	0x83		; Timer/Counter 1 Control Register D
OC1AU		equ	0		; Output Compare Pin Enable for Channel A
OC1AV		equ	1
OC1AW		equ	2
OC1AX		equ	3
OC1BU		equ	4		; Output Compare Pin Enable for Channel B
OC1BV		equ	5
OC1BW		equ	6
OC1BX		equ	7
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x8a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x8b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x86		; Timer/Counter 1 Input Capture LSB
ICR1H		sfr	0x87		; Timer/Counter 1 Input Capture MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		equ	0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		equ	0		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		equ	1		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	2		; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		equ	5		; Timer/Counter 1 Input Capture Interrupt Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR1		equ	0		; Timer/Counter 1 Prescaler Reset
PSR0		equ	1		; Timer/Counter 0 Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"

;----------------------------------------------------------------------------
; Analog Comparator

ACSR		port    0x30            ; Config/Status Register
ACIS0		equ     0               ; Interrupt-Mode
ACIS1		equ     1
ACIC	 	equ	2		; use Comparator als capture signal for Timer 1?
ACIE		equ     3               ; Interrupt Enable
ACI		equ     4               ; Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACIRS		equ	6		; Analog Comparator Internal Reference Select
ACD		equ	7		; Disable

		; bits in ADCSRB
ACIR0		equ	4		; Analog Comparator Internal Voltage Reference Select
ACIR1		equ	5

		; bits in DIDR0
AIN1D		equ	7		; disable digital input on AIN0
AIN0D		equ	6		; disable digital input on AIN1

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

		; bits in ADMUX
MUX4		equ	4

		; bits in ADCSRB
BIN		equ	7		; Bipolar Input Mode

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
ADC8D		equ	4		; Digital Input Disable ADC8
ADC9D		equ	5		; Digital Input Disable ADC9
ADC10D		equ	6		; Digital Input Disable ADC10

AMISCR		sfr	0x77		; Analog Miscellaneous Control Register
ISRCEN		equ	0		; Current Source Enable
XREFEN		equ	1		; Internal Voltage Reference Output Enable
AREFEN		equ	2		; External Voltage Reference Input Enable

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim2c.inc"

;----------------------------------------------------------------------------
; USI

		include "avr/usimb8.inc"

USIBR		sfr	0xbb		; USI Buffer Register

USIPP		sfr	0xbc		; USI Pin Position
USIPOS		equ	0		; USI Pin Position

;----------------------------------------------------------------------------
; LIN/UART

LINCR		sfr	0xc8		; LIN Control Register
LCMD0		equ	0		; Command and mode
LCMD1		equ	1
LCMD2		equ	2
LENA		equ	3		; Enable
LCONF0		equ	4		; Configuration
LCONF1		equ	5
LIN13		equ	6		; LIN 1.3 mode
LSWRES		equ	7		; Software Reset

LINSIR		sfr	0xc9		; LIN Status and Interrupt Register
LRXOK		equ	0		; Receive Performed Interrupt
LTXOK		equ	1		; Transmit Performed Interrupt
LIDOK		equ	2		; Identifier Interrupt
LERR		equ	3		; Error Interrupt
LBUSY		equ	4		; Busy Signal
LIDST0		equ	5		; Identifier Status
LIDST1		equ	6
LIDST2		equ	7

LINENIR		sfr	0xca		; LIN Enable Interrupt Register
LENRXOK		equ	0		; Enable Receive Performed Interrupt
LENTXOK		equ	1		; Enable Transmit Performed Interrupt
LENIDOK		equ	2		; Enable Identifier Interrupt
LENERR		equ	3		; Enable Error Interrupt

LINERR		sfr	0xcb		; LIN Error Register
LBERR		equ	0		; Bit Error Flag
LCERR		equ	1		; Checksum Error Flag
LPERR		equ	2		; Parity Error Flag
LSERR		equ	3		; Synchronization Error Flag
LFERR		equ	4		; Framing Error Flag
LOVERR		equ	5		; Overrun Error Flag
LTOERR		equ	6		; Frame_Time_Out Error Flag
LABORT		equ	7		; Abort Flag

LINBTR		sfr	0xcc		; LIN Bit Timing Register
LBT0		equ	0		; LIN Bit Timing
LBT1		equ	1
LBT2		equ	2
LBT3		equ	3
LBT4		equ	4
LBT5		equ	5
LDISR		equ	7		; Disable Bit Timing Re synchronization

LINBRRL		sfr	0xcd		; LIN Baud Rate Register Low
LINBRRH		sfr	0xce		; LIN Baud Rate Register High

LINDLR		sfr	0xcf		; LIN Data Length Register
LRXDL0		equ	0		; LIN Receive Data Length
LRXDL1		equ	1
LRXDL2		equ	2
LRXDL3		equ	3
LTXDL0		equ	4		; LIN Transmit Data Length
LTXDL1		equ	5
LTXDL2		equ	6
LTXDL3		equ	7

LINIDR		sfr	0xd0		; LIN Identifier Register
LID0		equ	0		; LIN 2.1 Identifier
LID1		equ	1
LID2		equ	2
LID3		equ	3
LID4		equ	4
LID5		equ	5
LDL0		equ	4		; LIN 1.3 Data Length
LDL1		equ	5	
LP0		equ	6		; Parity
LP1		equ	7

LINSEL		sfr	0xd1		; LIN Data Buffer Selection Register
LINDX0		equ	0		; FIFO LIN Data Buffer Index
LINDX1		equ	1
LINDX2		equ	2
LAINC		equ	3		; Auto Increment of Data Buffer Index

LINDAT		sfr	0xd2		; LIN Data Register

		restore			; re-enable listing

		endif			; __regtnx7inc
