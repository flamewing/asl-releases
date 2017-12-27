		ifndef	__regtn40inc
__regtn40inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN40.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny40                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x40,data	; start address SRAM
RAMEND		equ     0x13f,data	; end address SRAM
FLASHEND	label   4095		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

RSTFLR		port	0x3b		; Reset Flag Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-on Reset Flag

OSCCAL		port	0x39		; Oscillator Calibration

CLKPSR		port	0x36		; Clock Prescaler Register
CLKPS0		equ	0		; Clock Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3

CLKMSR		port	0x37		; Clock Main Settings Register
CLKMS0		equ	0		; Clock Main Select Bits
CLKMS1		equ	1

MCUCR		port	0x3a		; MCU Control Register
SE		equ	0		; Sleep Mode Enable
SM0		equ	1		; Sleep Mode Select
SM1		equ	2
SM2		equ	3
BODS		equ	4		; BOD Sleep

PRR		port	0x35		; Power Reduction Register
PRADC		equ	0		; Power Reduction A/D Converter
PRTIM0		equ	1		; Power Reduction Timer/Counter 0
PRTIM1		equ	2		; Power Reduction Timer/Counter 1
PRSPI		equ	3		; Power Reduction SPI
PRTWI		equ	4		; Power Reduction TWI

CCP		port	0x3c		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

NVMCSR		port	0x32		; Non-Volatile Memory Control and Status Register
NVMBSY		equ	7		; Non-Volatile Memory Busy

NVMCMD		port	0x33		; Non-Volatile Memory Command Register
NVMCMD0		equ	0		; Non-Volatile Memory Command
NVMCMD1		equ	1
NVMCMD2		equ	2
NVMCMD3		equ	3
NVMCMD4		equ	4
NVMCMD5		equ	5

RAMAR		port	0x20		; RAM Address Register
RAMDR		port	0x1f		; RAM Data Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PUEA		port	0x03		; Pull-Up Enable Port A
PUEA0		equ	0
PUEA1		equ	1
PUEA2		equ	2
PUEA3		equ	3
PUEA4		equ	4
PUEA5		equ	5
PUEA6		equ	6
PUEA7		equ	7
PINB		port	0x04		; Port B @ 0x03 (IO) ff. (bits 0..3)
PUEB		port	0x07		; Pull-Up Enable Port B
PUEB0		equ	0
PUEB1		equ	1
PUEB2		equ	2
PUEB3		equ	3
PINC		port	0x1b		; Port C @ 0x1b (IO) ff. (bits 0..5)
PUEC		port	0x1e		; Pull-Up Enable Port C
PUEC0           equ     0
PUEC1		equ	1
PUEC2		equ	2
PUEC3		equ	3
PUEC4		equ	4
PUEC5		equ	5

PCMSK0		port	0x09		; Pin-Change Mask Register 0
PCINT0		equ	0		; Enable Pin-Change Interrupt 0
PCINT1		equ	1		; Enable Pin-Change Interrupt 1
PCINT2		equ	2		; Enable Pin-Change Interrupt 2
PCINT3		equ	3		; Enable Pin-Change Interrupt 3
PCINT4		equ	4		; Enable Pin-Change Interrupt 4
PCINT5		equ	5		; Enable Pin-Change Interrupt 5
PCINT6		equ	6		; Enable Pin-Change Interrupt 6
PCINT7		equ	7		; Enable Pin-Change Interrupt 7

PCMSK1		port	0x0a		; Pin-Change Mask Register 1
PCINT8		equ	0		; Enable Pin-Change Interrupt 8
PCINT9		equ	1		; Enable Pin-Change Interrupt 9
PCINT10		equ	2		; Enable Pin-Change Interrupt 10
PCINT11		equ	3		; Enable Pin-Change Interrupt 11

PCMSK2		port	0x1a		; Pin-Change Mask Register 2
PCINT12		equ	0		; Enable Pin-Change Interrupt 12
PCINT13		equ	1		; Enable Pin-Change Interrupt 13
PCINT14		equ	2		; Enable Pin-Change Interrupt 14
PCINT15		equ	3		; Enable Pin-Change Interrupt 15
PCINT16		equ	4		; Enable Pin-Change Interrupt 16
PCINT17		equ	5		; Enable Pin-Change Interrupt 17

PORTCR		port	0x08		; Port Control Register
BBMA		equ	0		; Break-Before-Make Mode Enable Port A
BBMB		equ	1		; Break-Before-Make Mode Enable Port B
BBMC		equ	2		; Break-Before-Make Mode Enable Port C

QTCSR		port	0x34		; QTouch Control and Status Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum PCINT2_vect		; Pin Change Interrupt 2
		nextenum WDT_vect		; Watchdog Time-out Interrupt
		nextenum TIM1_CAPT_vect		; Timer/Counter 1 Capture
		nextenum TIM1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIM1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIM1_OVF_vect		; Timer/Counter 1 Overflow
		nextenum TIM0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIM0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIM0_OVF_vect		; Timer/Counter 0 Overflow
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum TWI_SLAVE_vect		; Two-Wire Interface
		nextenum SPI_vect		; Serial Peripheral Interface
		nextenum QTRIP_vect		; Touch Sensing

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	6		; Interrupt Sense Control 0
ISC01		equ	7

GIMSK		port	0x0c		; General Interrupt Mask Register
INT0		equ	0		; Enable External Interrupt 0
PCIE0		equ	4		; Enable Pin-Change Interrupt 0
PCIE1		equ	5		; Enable Pin-Change Interrupt 1
PCIE2		equ	6		; Enable Pin-Change Interrupt 2

GIFR		port	0x0b		; General Interrupt Flag Register
INTF0		equ	0		; External Interrupt 0 occured
PCIF0		equ	4		; Pin-Change Interrupt 0 Occured
PCIF1		equ	5		; Pin-Change Interrupt 1 Occured
PCIF2		equ	6		; Pin-Change Interrupt 2 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x19		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 0 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x18		; Timer/Counter 0 Control Register B
CS00            equ     0               ; Timer/Counter 0 Clock Select
CS01            equ     1
CS02            equ     2
WGM02		equ	3
PSR		equ	4		; Prescaler Reset
TSM		equ	5		; Timer/Counter Synchronization Mode
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare Match B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x17		; Timer/Counter 0 Value
OCR0A		port	0x16		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x15		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x24		; Timer/Counter 1 Control Register A
CS10		equ	0		; Timer/Counter 1 Clock Select
CS11		equ	1
CS12		equ	2		
CTC1		equ	3		; Timer/Counter 1 Waveform Generation Mode
ICES1		equ	4		; Timer/Counter 1 Input Capture Edge Select
ICNC1		equ	5		; Timer/Counter 1 Input Capture Noise Canceler
ICEN1		equ	6		; Timer/Counter 1 Input Capture Mode Enable
TCW1		equ	7		; Timer/Counter 1 Width
TCNT1L		port	0x23		; Timer/Counter 1 Value LSB
TCNT1H		port	0x27		; Timer/Counter 1 Value MSB
OCR1A		port	0x22		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x21		; Timer/Counter 1 Output Compare Value B

TIMSK		port	0x26		; Timer/Counter Interrupt Mask Register
TOIE0		equ	0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable B
TOIE1		equ	3		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		equ	4		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		equ	5		; Timer/Counter 1 Output Compare Interrupt Enable B
ICE1		equ	7		; Timer/Counter 1 Input Capture Interrupt Enable

TIFR		port	0x25		; Timer/Counter Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x31		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDP3		equ	5
WDIE		equ	6		; Watchdog Interrupt Enable
WDIF		equ	7		; Watchdog Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port    0x14		; Analog Comparator Control and Status Register A
ACIS0		equ     0		; Interrupt-Mode
ACIS1		equ     1
ACIC		equ	2		; Use Comparator As Capture Signal For Timer 0?
ACIE		equ     3               ; Interrupt Enable
ACI		equ     4               ; Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACBG		equ	6		; Enable Bandgap
ACD		equ	7		; Disable

ACSRB		port	0x13		; Analog Comparator Control and Status Register B
ACIRS0		equ	0		; reserved for QTouch
ACIRS1		equ	1
ACME		equ	2		; Analog Comparator Multiplexer Enable
ACCE		equ	3		; reserved for QTouch
ACLP		equ	5		; reserved for QTouch
HLEV		equ	6		; Hysteresis Level
HSEL		equ	7		; Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x10		; ADC Multiplexer Selection Register
MUX0		equ	0		; Analog Channel Selection
MUX1		equ	1
MUX2		equ	2
MUX3		equ	3
ADC0EN		equ	4		; reserved for QTouch
REFEN		equ	5		; Enable Reference
REFS		equ	6		; Reference Selection

ADCSRA		port	0x12		; ADC Control/Status Register A
ADEN		equ	7		; Enable ADC
ADSC		equ	6		; Start Conversion
ADATE		equ	5		; ADC Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x11		; ADC Control/Status Register A
ADTS0		equ	0		; ADC Auto Trigger Source
ADTS1		equ	1
ADTS2		equ	2
ADLAR		equ	3		; ADC Left Adjust Result
VDPD		equ	6		; reserved for QTouch
VDEN		equ	7		; reserved for QTouch

ADCL		port	0x0e		; ADC Conversion Result LSB
ADCH		port	0x0f		; ADC Conversion Result MSB

DIDR0		port	0x0d		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Disable
ADC1D		equ	1		; ADC1 Digital Input Disable
ADC2D		equ	2		; ADC2 Digital Input Disable
ADC3D		equ	3		; ADC3 Digital Input Disable
ADC4D		equ	0		; ADC4 Digital Input Disable
ADC5D		equ	1		; ADC5 Digital Input Disable
ADC6D		equ	2		; ADC6 Digital Input Disable
ADC7D		equ	3		; ADC7 Digital Input Disable

		; bits in PORTCR
ADC8D		equ	4		; ADC8 Digital Input Disable
ADC9D		equ	5		; ADC9 Digital Input Disable
ADC10D		equ	6		; ADC10 Digital Input Disable
ADC11D		equ	7		; ADC11 Digital Input Disable

;----------------------------------------------------------------------------
; SPI

SPCR		port	0x30		; SPI Control Register
SPR0		equ	0		; Clock Select
SPR1		equ	1
CPHA		equ	2		; Clock Phase
CPOL		equ	3		; Clock Polarity
MSTR		equ	4		; Master/Slave Selection
DORD		equ	5		; Bit Order
SPE		equ	6		; Enable SPI
SPIE		equ	7		; SPI Interrupt Enable

SPSR		port	0x2f		; SPI Status Register
SPI2X		equ	0		; Double Speed Mode
WCOL		equ	6		; Write Collision
SPIF		equ	7		; SPI Interrupt Occured?

SPDR		port	0x2e		; SPI data register

;----------------------------------------------------------------------------
; SPI

TWSCRA		port	0x2d		; TWI Slave Control Register A
TWSME		equ	0		; TWI Smart Mode Enable
TWPME		equ	1		; TWI Promiscuous Mode Enable
TWSIE		equ	2		; TWI Stop Interrupt Enable
TWEN		equ	3		; TWI Enable
TWASIE		equ	4		; TWI Address/Stop Interrupt Enable
TWDIE		equ	5		; TWI Data Interrupt Enable
TWSHE		equ	7		; TWI SDA Hold Time Enable

TWSCRB		port	0x2c		; TWI Slave Control Register B
TWCMD0		equ	0		; TWI Command
TWCMD1		equ	1
TWAA		equ	2		; TWI Acknowledge Action

TWSSRA		port	0x2b		; TWI Slave Status Register A
TWAS		equ	0		; TWI Address or Stop
TWDIR		equ	1		; TWI Read/Write Direction
TWBE		equ	2		; TWI Bus Error
TWC		equ	3		; TWI Collision
TWRA		equ	4		; TWI Receive Acknowledge
TWCH		equ	5		; TWI Clock Hold
TWASIF		equ	6		; TWI Address/Stop Interrupt Flag
TWDIF		equ	7		; TWI Data Interrupt Flag

TWSA		port	0x2a		; TWI Slave Address Register

TWSAM		port	0x29		; TWI Slave Address Mask Register
TWAE		equ	0		; TWI Address Enable

TWSD		port	0x28		; TWI Slave Data Register

		restore

		endif			; __regtn40inc
