		ifndef	__regtnx7inc
__regtnx7inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX7.INC                                             *
;*                                                                          *
;*   Contains common bit & Register definitions for ATtiny87/167            *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
BODSE		avrbit	MCUCR,5		; BOD Sleep
BODS		avrbit	MCUCR,6		; BOD Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2

OSCCAL		sfr	0x66		; Oscillator Calibration

CLKPR		sfr	0x61		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

CLKCSR		sfr	0x62		; Clock Control/Status Register
CLKC0		avrbit	CLKCSR,0	; Clock Control Bits
CLKC1		avrbit	CLKCSR,1
CLKC2		avrbit	CLKCSR,2
CLKC3		avrbit	CLKCSR,3
CLKRDY		avrbit	CLKCSR,4	; Clock Ready Flag
CLKCCE		avrbit	CLKCSR,7	; Clock Control Change Enable

CLKSELR		sfr	0x63		; Clock Selection Register
CSEL0		avrbit	CLKSELR,0	; Clock Source Select
CSEL1		avrbit	CLKSELR,1
CSEL2		avrbit	CLKSELR,2
CSEL3		avrbit	CLKSELR,3
CSUT0		avrbit	CLKSELR,4	; Clock Start-up Time
CSUT1		avrbit	CLKSELR,5
COUT		avrbit	CLKSELR,6	; Clock Out

PRR		sfr	0x64		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRUSI		avrbit	PRR,1		; Power Reduction USI
PRTIM1		avrbit	PRR,2		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR,3		; Power Reduction Timer/Counter 0
PRSPI		avrbit	PRR,4		; Power Reduction  SPI
PRLIN		avrbit	PRR,5		; Power Reduction Power Reduction LIN / UART Controller

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x21		; EEPROM Address Register Low
EEARH		port	0x22		; EEPROM Address Register High
EEDR		port	0x20		; EEPROM Data Register
EECR		port	0x1f		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RWWSB		avrbit	SPMCSR,6	; Read-While-Write Section Busy
SIGRD		avrbit	SPMCSR,5	; Read Signature Bytes
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,4		; Pull-Up Disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PORTCR		port	0x12		; Port Control Register
PUDA		avrbit	PORTCR,0	; Pull-up Disable Port A
PUDB		avrbit	PORTCR,1	; Pull-up Disable Port B
BBMA		avrbit	PORTCR,4	; Break-Before-Make Mode Enable A
BBMB		avrbit	PORTCR,5	; Break-Before-Make Mode Enable B

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
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVF_vect		; USI Overflow

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1
ISC10		avrbit	EICRA,2		; External Interrupt 1 Sense Control
ISC11		avrbit	EICRA,3

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1		avrbit	EIFR,1		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x25		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x26		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Forc Output Compare Match
TCNT0		port	0x27		; Timer/Counter 0 Value
OCR0A		port	0x28		; Timer/Counter 0 Output Compare Value A
ASSR		sfr	0xb6		; Asynchronous Status Register
TCR0BUB		avrbit	ASSR,0		; Timer/Counter0 Control Register B Update Busy
TCR0AUB		avrbit	ASSR,1		; Timer/Counter0 Control Register A Update Busy
OCR0AUB		avrbit	ASSR,3		; Output Compare 0 Register A Update Busy
TCN0UB		avrbit	ASSR,4		; Timer/Counter0 Update Busy
AS0		avrbit	ASSR,5		; Asynchronous Timer/Counter 0
EXCLK		avrbit	ASSR,6		; Enable External Clock Input

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		sfr	0x82		; Timer/Counter 1 Control Register C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCCR1D		sfr	0x83		; Timer/Counter 1 Control Register D
OC1AU		avrbit	TCCR1D,0	; Output Compare Pin Enable for Channel A
OC1AV		avrbit	TCCR1D,1
OC1AW		avrbit	TCCR1D,2
OC1AX		avrbit	TCCR1D,3
OC1BU		avrbit	TCCR1D,4	; Output Compare Pin Enable for Channel B
OC1BV		avrbit	TCCR1D,5
OC1BW		avrbit	TCCR1D,6
OC1BX		avrbit	TCCR1D,7
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x8a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x8b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x86		; Timer/Counter 1 Input Capture LSB
ICR1H		sfr	0x87		; Timer/Counter 1 Input Capture MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Interrupt Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR1		avrbit	GTCCR,0		; Timer/Counter 1 Prescaler Reset
PSR0		avrbit	GTCCR,1		; Timer/Counter 0 Prescaler Reset
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdme.inc"

;----------------------------------------------------------------------------
; Analog Comparator

ACSR		port    0x30            ; Config/Status Register
ACIS0		avrbit	ACSR,0		; Interrupt-Mode
ACIS1		avrbit	ACSR,1
ACIC	 	avrbit	ACSR,2		; use Comparator as Capture Signal for Timer 1?
ACIE		avrbit	ACSR,3		; Interrupt Enable
ACI		avrbit	ACSR,4		; Interrupt Flag
ACO		avrbit	ACSR,5		; Analog Comparator Output
ACIRS		avrbit	ACSR,6		; Analog Comparator Internal Reference Select
ACD		avrbit	ACSR,7		; Disable

ACIR0		avrbit	ADCSRB,4	; Analog Comparator Internal Voltage Reference Select
ACIR1		avrbit	ADCSRB,5

AIN1D		avrbit	DIDR0,7		; Disable Digital Input on AIN0
AIN0D		avrbit	DIDR0,6		; Disable Digital Input on AIN1

;----------------------------------------------------------------------------
; A/D Converter

		include	"adcm78.inc"

MUX4		avrbit	ADMUX,4

BIN		avrbit	ADCSRB,7	; Bipolar Input Mode

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
ADC8D		avrbit	DIDR1,4		; Digital Input Disable ADC8
ADC9D		avrbit	DIDR1,5		; Digital Input Disable ADC9
ADC10D		avrbit	DIDR1,6		; Digital Input Disable ADC10

AMISCR		sfr	0x77		; Analog Miscellaneous Control Register
ISRCEN		avrbit	AMISCR,0	; Current Source Enable
XREFEN		avrbit	AMISCR,1	; Internal Voltage Reference Output Enable
AREFEN		avrbit	AMISCR,2	; External Voltage Reference Input Enable

;----------------------------------------------------------------------------
; SPI

		include	"spim2c.inc"

;----------------------------------------------------------------------------
; USI

		include "usimb8.inc"

USIBR		sfr	0xbb		; USI Buffer Register

USIPP		sfr	0xbc		; USI Pin Position
USIPOS		avrbit	USIPP,0		; USI Pin Position

;----------------------------------------------------------------------------
; LIN/UART

LINCR		sfr	0xc8		; LIN Control Register
LCMD0		avrbit	LINCR,0		; Command and Mode
LCMD1		avrbit	LINCR,1
LCMD2		avrbit	LINCR,2
LENA		avrbit	LINCR,3		; Enable
LCONF0		avrbit	LINCR,4		; Configuration
LCONF1		avrbit	LINCR,5
LIN13		avrbit	LINCR,6		; LIN 1.3 Mode
LSWRES		avrbit	LINCR,7		; Software Reset

LINSIR		sfr	0xc9		; LIN Status and Interrupt Register
LRXOK		avrbit	LINSIR,0	; Receive Performed Interrupt
LTXOK		avrbit	LINSIR,1	; Transmit Performed Interrupt
LIDOK		avrbit	LINSIR,2	; Identifier Interrupt
LERR		avrbit	LINSIR,3	; Error Interrupt
LBUSY		avrbit	LINSIR,4	; Busy Signal
LIDST0		avrbit	LINSIR,5	; Identifier Status
LIDST1		avrbit	LINSIR,6
LIDST2		avrbit	LINSIR,7

LINENIR		sfr	0xca		; LIN Enable Interrupt Register
LENRXOK		avrbit	LINENIR,0	; Enable Receive Performed Interrupt
LENTXOK		avrbit	LINENIR,1	; Enable Transmit Performed Interrupt
LENIDOK		avrbit	LINENIR,2	; Enable Identifier Interrupt
LENERR		avrbit	LINENIR,3	; Enable Error Interrupt

LINERR		sfr	0xcb		; LIN Error Register
LBERR		avrbit	LINERR,0		; Bit Error Flag
LCERR		avrbit	LINERR,1	; Checksum Error Flag
LPERR		avrbit	LINERR,2	; Parity Error Flag
LSERR		avrbit	LINERR,3	; Synchronization Error Flag
LFERR		avrbit	LINERR,4	; Framing Error Flag
LOVERR		avrbit	LINERR,5	; Overrun Error Flag
LTOERR		avrbit	LINERR,6	; Frame_Time_Out Error Flag
LABORT		avrbit	LINERR,7	; Abort Flag

LINBTR		sfr	0xcc		; LIN Bit Timing Register
LBT0		avrbit	LINBTR,0	; LIN Bit Timing
LBT1		avrbit	LINBTR,1
LBT2		avrbit	LINBTR,2
LBT3		avrbit	LINBTR,3
LBT4		avrbit	LINBTR,4
LBT5		avrbit	LINBTR,5
LDISR		avrbit	LINBTR,7	; Disable Bit Timing Re synchronization

LINBRRL		sfr	0xcd		; LIN Baud Rate Register Low
LINBRRH		sfr	0xce		; LIN Baud Rate Register High

LINDLR		sfr	0xcf		; LIN Data Length Register
LRXDL0		avrbit	LINDLR,0	; LIN Receive Data Length
LRXDL1		avrbit	LINDLR,1
LRXDL2		avrbit	LINDLR,2
LRXDL3		avrbit	LINDLR,3
LTXDL0		avrbit	LINDLR,4	; LIN Transmit Data Length
LTXDL1		avrbit	LINDLR,5
LTXDL2		avrbit	LINDLR,6
LTXDL3		avrbit	LINDLR,7

LINIDR		sfr	0xd0		; LIN Identifier Register
LID0		avrbit	LINIDR,0	; LIN 2.1 Identifier
LID1		avrbit	LINIDR,1
LID2		avrbit	LINIDR,2
LID3		avrbit	LINIDR,3
LID4		avrbit	LINIDR,4
LID5		avrbit	LINIDR,5
LDL0		avrbit	LINIDR,4	; LIN 1.3 Data Length
LDL1		avrbit	LINIDR,5	
LP0		avrbit	LINIDR,6	; Parity
LP1		avrbit	LINIDR,7

LINSEL		sfr	0xd1		; LIN Data Buffer Selection Register
LINDX0		avrbit	LINSEL,0	; FIFO LIN Data Buffer Index
LINDX1		avrbit	LINSEL,1
LINDX2		avrbit	LINSEL,2
LAINC		avrbit	LINSEL,3	; Auto Increment of Data Buffer Index

LINDAT		sfr	0xd2		; LIN Data Register

		restore			; re-enable listing

		endif			; __regtnx7inc
