		ifndef	__regtn40inc
__regtn40inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN40.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny40                       *
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
WDRF		avrbit	RSTFLR,3	; Watchdog Reset Flag
BORF		avrbit	RSTFLR,2	; Brown-out Reset Flag
EXTRF		avrbit	RSTFLR,1	; External Reset Flag
PORF		avrbit	RSTFLR,0	; Power-on Reset Flag

OSCCAL		port	0x39		; Oscillator Calibration

CLKPSR		port	0x36		; Clock Prescaler Register
CLKPS0		avrbit	CLKPSR,0	; Clock Prescaler Select
CLKPS1		avrbit	CLKPSR,1
CLKPS2		avrbit	CLKPSR,2
CLKPS3		avrbit	CLKPSR,3

CLKMSR		port	0x37		; Clock Main Settings Register
CLKMS0		avrbit	CLKMSR,0	; Clock Main Select Bits
CLKMS1		avrbit	CLKMSR,1

MCUCR		port	0x3a		; MCU Control Register
SE		avrbit	MCUCR,0		; Sleep Mode Enable
SM0		avrbit	MCUCR,1		; Sleep Mode Select
SM1		avrbit	MCUCR,2
SM2		avrbit	MCUCR,3
BODS		avrbit	MCUCR,4		; BOD Sleep

PRR		port	0x35		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction A/D Converter
PRTIM0		avrbit	PRR,1		; Power Reduction Timer/Counter 0
PRTIM1		avrbit	PRR,2		; Power Reduction Timer/Counter 1
PRSPI		avrbit	PRR,3		; Power Reduction SPI
PRTWI		avrbit	PRR,4		; Power Reduction TWI

CCP		port	0x3c		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

NVMCSR		port	0x32		; Non-Volatile Memory Control and Status Register
NVMBSY		avrbit	NVMCSR,7	; Non-Volatile Memory Busy

NVMCMD		port	0x33		; Non-Volatile Memory Command Register
NVMCMD0		avrbit	NVMCMD,0	; Non-Volatile Memory Command
NVMCMD1		avrbit	NVMCMD,1
NVMCMD2		avrbit	NVMCMD,2
NVMCMD3		avrbit	NVMCMD,3
NVMCMD4		avrbit	NVMCMD,4
NVMCMD5		avrbit	NVMCMD,5

RAMAR		port	0x20		; RAM Address Register
RAMDR		port	0x1f		; RAM Data Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PUEA		port	0x03		; Pull-Up Enable Port A
PUEA0		avrbit	PUEA,0
PUEA1		avrbit	PUEA,1
PUEA2		avrbit	PUEA,2
PUEA3		avrbit	PUEA,3
PUEA4		avrbit	PUEA,4
PUEA5		avrbit	PUEA,5
PUEA6		avrbit	PUEA,6
PUEA7		avrbit	PUEA,7
PINB		port	0x04		; Port B @ 0x03 (IO) ff.
__PORTB_BITS	equ	0x0f		; (bits 0..3)
PUEB		port	0x07		; Pull-Up Enable Port B
PUEB0		avrbit	PUEB,0
PUEB1		avrbit	PUEB,1
PUEB2		avrbit	PUEB,2
PUEB3		avrbit	PUEB,3
PINC		port	0x1b		; Port C @ 0x1b (IO) ff.
__PORTC_BITS	equ	0x3f		; (bits 0..5)
PUEC		port	0x1e		; Pull-Up Enable Port C
PUEC0           avrbit	PUEC,0
PUEC1		avrbit	PUEC,1
PUEC2		avrbit	PUEC,2
PUEC3		avrbit	PUEC,3
PUEC4		avrbit	PUEC,4
PUEC5		avrbit	PUEC,5

PCMSK0		port	0x09		; Pin-Change Mask Register 0
PCINT0		avrbit	PCMSK0,0	; Enable Pin-Change Interrupt 0
PCINT1		avrbit	PCMSK0,1	; Enable Pin-Change Interrupt 1
PCINT2		avrbit	PCMSK0,2	; Enable Pin-Change Interrupt 2
PCINT3		avrbit	PCMSK0,3	; Enable Pin-Change Interrupt 3
PCINT4		avrbit	PCMSK0,4	; Enable Pin-Change Interrupt 4
PCINT5		avrbit	PCMSK0,5	; Enable Pin-Change Interrupt 5
PCINT6		avrbit	PCMSK0,6	; Enable Pin-Change Interrupt 6
PCINT7		avrbit	PCMSK0,7	; Enable Pin-Change Interrupt 7

PCMSK1		port	0x0a		; Pin-Change Mask Register 1
PCINT8		avrbit	PCMSK1,0	; Enable Pin-Change Interrupt 8
PCINT9		avrbit	PCMSK1,1	; Enable Pin-Change Interrupt 9
PCINT10		avrbit	PCMSK1,2	; Enable Pin-Change Interrupt 10
PCINT11		avrbit	PCMSK1,3	; Enable Pin-Change Interrupt 11

PCMSK2		port	0x1a		; Pin-Change Mask Register 2
PCINT12		avrbit	PCMSK2,0	; Enable Pin-Change Interrupt 12
PCINT13		avrbit	PCMSK2,1	; Enable Pin-Change Interrupt 13
PCINT14		avrbit	PCMSK2,2	; Enable Pin-Change Interrupt 14
PCINT15		avrbit	PCMSK2,3	; Enable Pin-Change Interrupt 15
PCINT16		avrbit	PCMSK2,4	; Enable Pin-Change Interrupt 16
PCINT17		avrbit	PCMSK2,5	; Enable Pin-Change Interrupt 17

PORTCR		port	0x08		; Port Control Register
BBMA		avrbit	PORTCR,0	; Break-Before-Make Mode Enable Port A
BBMB		avrbit	PORTCR,1	; Break-Before-Make Mode Enable Port B
BBMC		avrbit	PORTCR,2	; Break-Before-Make Mode Enable Port C

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

ISC00		avrbit	MCUCR,6		; Interrupt Sense Control 0
ISC01		avrbit	MCUCR,7

GIMSK		port	0x0c		; General Interrupt Mask Register
INT0		avrbit	GIMSK,0		; Enable External Interrupt 0
PCIE0		avrbit	GIMSK,4		; Enable Pin-Change Interrupt 0
PCIE1		avrbit	GIMSK,5		; Enable Pin-Change Interrupt 1
PCIE2		avrbit	GIMSK,6		; Enable Pin-Change Interrupt 2

GIFR		port	0x0b		; General Interrupt Flag Register
INTF0		avrbit	GIFR,0		; External Interrupt 0 Occured
PCIF0		avrbit	GIFR,4		; Pin-Change Interrupt 0 Occured
PCIF1		avrbit	GIFR,5		; Pin-Change Interrupt 1 Occured
PCIF2		avrbit	GIFR,6		; Pin-Change Interrupt 2 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x19		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x18		; Timer/Counter 0 Control Register B
CS00            avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01            avrbit	TCCR0B,1
CS02            avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
PSR		avrbit	TCCR0B,4	; Prescaler Reset
TSM		avrbit	TCCR0B,5	; Timer/Counter Synchronization Mode
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x17		; Timer/Counter 0 Value
OCR0A		port	0x16		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x15		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x24		; Timer/Counter 1 Control Register A
CS10		avrbit	TCCR1A,0	; Timer/Counter 1 Clock Select
CS11		avrbit	TCCR1A,1
CS12		avrbit	TCCR1A,2		
CTC1		avrbit	TCCR1A,3	; Timer/Counter 1 Waveform Generation Mode
ICES1		avrbit	TCCR1A,4	; Timer/Counter 1 Input Capture Edge Select
ICNC1		avrbit	TCCR1A,5	; Timer/Counter 1 Input Capture Noise Canceler
ICEN1		avrbit	TCCR1A,6	; Timer/Counter 1 Input Capture Mode Enable
TCW1		avrbit	TCCR1A,7	; Timer/Counter 1 Width
TCNT1L		port	0x23		; Timer/Counter 1 Value LSB
TCNT1H		port	0x27		; Timer/Counter 1 Value MSB
OCR1A		port	0x22		; Timer/Counter 1 Output Compare Value A
OCR1B		port	0x21		; Timer/Counter 1 Output Compare Value B

TIMSK		port	0x26		; Timer/Counter Interrupt Mask Register
TOIE0		avrbit	TIMSK,0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK,1		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK,2		; Timer/Counter 0 Output Compare Interrupt Enable B
TOIE1		avrbit	TIMSK,3		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		avrbit	TIMSK,4		; Timer/Counter 1 Output Compare Interrupt Enable A
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
ICE1		avrbit	TIMSK,7		; Timer/Counter 1 Input Capture Interrupt Enable

TIFR		port	0x25		; Timer/Counter Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x31		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable watchdog
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Watchdog Interrupt Enable
WDIF		avrbit	WDTCSR,7	; Watchdog Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port    0x14		; Analog Comparator Control and Status Register A
ACIS0		avrbit	ACSRA,0		; Interrupt-Mode
ACIS1		avrbit	ACSRA,1
ACIC		avrbit	ACSRA,2		; Use Comparator As Capture Signal For Timer 0?
ACIE		avrbit	ACSRA,3		; Interrupt Enable
ACI		avrbit	ACSRA,4		; Interrupt Flag
ACO		avrbit	ACSRA,5		; Analog Comparator Output
ACBG		avrbit	ACSRA,6		; Enable Bandgap
ACD		avrbit	ACSRA,7		; Disable

ACSRB		port	0x13		; Analog Comparator Control and Status Register B
ACIRS0		avrbit	ACSRB,0		; reserved for QTouch
ACIRS1		avrbit	ACSRB,1
ACME		avrbit	ACSRB,2		; Analog Comparator Multiplexer Enable
ACCE		avrbit	ACSRB,3		; reserved for QTouch
ACLP		avrbit	ACSRB,5		; reserved for QTouch
HLEV		avrbit	ACSRB,6		; Hysteresis Level
HSEL		avrbit	ACSRB,7		; Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x10		; ADC Multiplexer Selection Register
MUX0		avrbit	ADMUX,0		; Analog Channel Selection
MUX1		avrbit	ADMUX,1
MUX2		avrbit	ADMUX,2
MUX3		avrbit	ADMUX,3
ADC0EN		avrbit	ADMUX,4		; reserved for QTouch
REFEN		avrbit	ADMUX,5		; Enable Reference
REFS		avrbit	ADMUX,6		; Reference Selection

ADCSRA		port	0x12		; ADC Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; ADC Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		port	0x11		; ADC Control/Status Register A
ADTS0		avrbit	ADCSRB,0	; ADC Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS2		avrbit	ADCSRB,2
ADLAR		avrbit	ADCSRB,3	; ADC Left Adjust Result
VDPD		avrbit	ADCSRB,6	; reserved for QTouch
VDEN		avrbit	ADCSRB,7	; reserved for QTouch

ADCL		port	0x0e		; ADC Conversion Result LSB
ADCH		port	0x0f		; ADC Conversion Result MSB

DIDR0		port	0x0d		; Digital Input Disable Register 0
ADC0D		avrbit	DIDR0,0		; ADC0 Digital Input Disable
ADC1D		avrbit	DIDR0,1		; ADC1 Digital Input Disable
ADC2D		avrbit	DIDR0,2		; ADC2 Digital Input Disable
ADC3D		avrbit	DIDR0,3		; ADC3 Digital Input Disable
ADC4D		avrbit	DIDR0,4		; ADC4 Digital Input Disable
ADC5D		avrbit	DIDR0,5		; ADC5 Digital Input Disable
ADC6D		avrbit	DIDR0,6		; ADC6 Digital Input Disable
ADC7D		avrbit	DIDR0,7		; ADC7 Digital Input Disable

ADC8D		avrbit	PORTCR,4	; ADC8 Digital Input Disable
ADC9D		avrbit	PORTCR,5	; ADC9 Digital Input Disable
ADC10D		avrbit	PORTCR,6	; ADC10 Digital Input Disable
ADC11D		avrbit	PORTCR,7	; ADC11 Digital Input Disable

;----------------------------------------------------------------------------
; SPI

SPCR		port	0x30		; SPI Control Register
SPR0		avrbit	SPCR,0		; Clock Select
SPR1		avrbit	SPCR,1
CPHA		avrbit	SPCR,2		; Clock Phase
CPOL		avrbit	SPCR,3		; Clock Polarity
MSTR		avrbit	SPCR,4		; Master/Slave Selection
DORD		avrbit	SPCR,5		; Bit Order
SPE		avrbit	SPCR,6		; Enable SPI
SPIE		avrbit	SPCR,7		; SPI Interrupt Enable

SPSR		port	0x2f		; SPI Status Register
SPI2X		avrbit	SPSR,0		; Double Speed Mode
WCOL		avrbit	SPSR,6		; Write Collision
SPIF		avrbit	SPSR,7		; SPI Interrupt Occured?

SPDR		port	0x2e		; SPI Data Register

;----------------------------------------------------------------------------
; SPI

TWSCRA		port	0x2d		; TWI Slave Control Register A
TWSME		avrbit	TWSCRA,0	; TWI Smart Mode Enable
TWPME		avrbit	TWSCRA,1	; TWI Promiscuous Mode Enable
TWSIE		avrbit	TWSCRA,2	; TWI Stop Interrupt Enable
TWEN		avrbit	TWSCRA,3	; TWI Enable
TWASIE		avrbit	TWSCRA,4	; TWI Address/Stop Interrupt Enable
TWDIE		avrbit	TWSCRA,5	; TWI Data Interrupt Enable
TWSHE		avrbit	TWSCRA,7	; TWI SDA Hold Time Enable

TWSCRB		port	0x2c		; TWI Slave Control Register B
TWCMD0		avrbit	TWSCRB,0	; TWI Command
TWCMD1		avrbit	TWSCRB,1
TWAA		avrbit	TWSCRB,2	; TWI Acknowledge Action

TWSSRA		port	0x2b		; TWI Slave Status Register A
TWAS		avrbit	TWSSRA,0	; TWI Address or Stop
TWDIR		avrbit	TWSSRA,1	; TWI Read/Write Direction
TWBE		avrbit	TWSSRA,2	; TWI Bus Error
TWC		avrbit	TWSSRA,3	; TWI Collision
TWRA		avrbit	TWSSRA,4	; TWI Receive Acknowledge
TWCH		avrbit	TWSSRA,5	; TWI Clock Hold
TWASIF		avrbit	TWSSRA,6	; TWI Address/Stop Interrupt Flag
TWDIF		avrbit	TWSSRA,7	; TWI Data Interrupt Flag

TWSA		port	0x2a		; TWI Slave Address Register

TWSAM		port	0x29		; TWI Slave Address Mask Register
TWAE		avrbit	TWSAM,0		; TWI Address Enable

TWSD		port	0x28		; TWI Slave Data Register

		restore

		endif			; __regtn40inc
