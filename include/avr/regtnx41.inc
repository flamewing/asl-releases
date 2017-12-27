		ifndef	__regtnx41inc
__regtnx41inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTNX41.INC                                            *
;*                                                                          *
;*   Contains common bit & register definitions for ATtiny441/841           *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	3		; Sleep Mode Select
SM1		equ	4
SE		equ	5		; Sleep Enable

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL0		sfr	0x74		; Oscillator Calibration
OSSCAL1		sfr	0x77

OSCTCAL0A	sfr	0x75		; Oscillator Temperature Compensation
OSCTCAL0B	sfr	0x76

CLKPR		sfr	0x73		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3

CLKCR		sfr	0x72		; Clock Control Register
CKSEL0		equ	0		; Clock Select
CKSEL1		equ	1
CKSEL2		equ	2
CKSEL3		equ	3
SUT		equ	4		; Start-Up Time
CKOUTC		equ	5		; Clock Output (Copy)
CSTR		equ	6		; Clock Select Trigger
OSCRDY		equ	7		; Oscillator Ready

PRR		sfr	0x70		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRTIM0		equ	1		; Power Reduction Timer/Counter 0
PRTIM1		equ	2		; Power Reduction Timer/Counter 1
PRTIM2		equ	3		; Power Reduction Timer/Counter 2
PRSPI		equ	4		; Power Reduction SPI
PRUSART0	equ	5		; Power Reduction USART0
PRUSART1	equ	6		; Power Reduction USART1
PRTWI		equ	7		; Power Reduction Two Wire Interface

CCP		sfr	0x71		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register Low
EEARH		port	0x1f		; EEPROM Address Register High
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RSIG		equ	5		; Read Device Signature Imprint Table
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

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.

GPIOR0		port	0x13		; General Purpose I/O Register 0
GPIOR1		port	0x14		; General Purpose I/O Register 1
GPIOR2		port	0x15		; General Purpose I/O Register 2

PORTCR		sfr	0x64		; Port Control Register
BBMA		equ	0		; Break-Before-Make Mode Enable Port A
BBMB		equ	1		; Break-Before-Make Mode Enable Port B

PUEA		sfr	0x63		; Pull-up Enable Port A
PUEA0		equ	0
PUEA1		equ	1
PUEA2		equ	2
PUEA3		equ	3
PUEA4		equ	4
PUEA5		equ	5
PUEA6		equ	6
PUEA7		equ	7

PUEB		sfr	0x62		; Pull-up Enable Port B
PUEB0		equ	0
PUEB1		equ	1
PUEB2		equ	2
PUEB3		equ	3

PCMSK0		port	0x12		; Pin Change Interrupt Mask 0
PCMSK1		port	0x20		; Pin Change Interrupt Mask 1
PCINT8		equ	0		; Enable Pin Change interrupt 8
PCINT9		equ	1		; Enable Pin Change interrupt 9
PCINT10		equ	2		; Enable Pin Change interrupt 10
PCINT11		equ	3		; Enable Pin Change interrupt 11

PHDE		sfr	0x6a		; Port High Drive Enable Register
PHDEA0		equ	0		; Pin PA5 High Drive Enable
PHDEA1		equ	1		; Pin PA7 High Drive Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ANA_COMP0_vect		; Analog Comparator 0
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANA_COMP1_vect		; Analog Comparator 1
		nextenum TIMER2_CAPT_vect	; Timer/Counter 2 Capture Event
		nextenum TIMER2_COMPA_vect	; Timer/Counter 2 Compare Match A
		nextenum TIMER2_COMPB_vect	; Timer/Counter 2 Compare Match B
		nextenum TIMER2_OVF_vect	; Timer/Counter 2 Overflow
		nextenum SPI_vect		; SPI Serial Transfer Complete
		nextenum USART0_START_vect	; USART0 Rx Start
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum USART1_START_vect	; USART1 Rx Start
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum TWI_SLAVE_vect		; 2-wire Serial Interface

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
PCIE0		equ	4		; Enable Pin Change Interrupt 0
PCIE1		equ	5		; Enable Pin Change Interrupt 1
INT0		equ	6		; Enable External Interrupt 0

GIFR		port	0x3a		; General Interrupt Flag Register
PCIF0		equ	4		; Pin Change Interrupt 0 Occured
PCIF1		equ	5		; Pin Change Interrupt 1 Occured
INTF0		equ	6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x30		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare Match B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x3c		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	4		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	5
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		equ     0               ; Timer/Counter 1 Clock Select
CS11		equ     1
CS12		equ     2
WGM12		equ	3
WGM13		equ	4
ICES1		equ	6		; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		equ	7		; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		port	0x22		; Timer/Counter 1 Control Register C
FOC1B		equ	6		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	7		; Timer/Counter 1 Force Output Compare A
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture MSB

TCCR2A		sfr	0xca		; Timer/Counter 2 Control Register A
WGM20		equ	0		; Timer/Counter 2 Waveform Generation Mode
WGM21		equ	1
COM2B0		equ	4		; Timer/Counter 2 Output Compare Mode B
COM2B1		equ	5
COM2A0		equ	4		; Timer/Counter 2 Output Compare Mode A
COM2A1		equ	5
TCCR2B		sfr	0xc9		; Timer/Counter 2 Control Register B
CS20		equ     0               ; Timer/Counter 2 Clock Select
CS21		equ     1
CS22		equ     2
WGM22		equ	3
WGM23		equ	4
ICES2		equ	6		; Timer/Counter 2 Input Capture Edge Selecr
ICNC2		equ	7		; Timer/Counter 2 Input Capture Noise Canceling
TCCR2C		sfr	0xc8		; Timer/Counter 2 Control Register C
FOC2B		equ	6		; Timer/Counter 2 Force Output Compare B
FOC2A		equ	7		; Timer/Counter 2 Force Output Compare A
TCNT2L		sfr	0xc6		; Timer/Counter 2 Value LSB
TCNT2H		sfr	0xc7		; Timer/Counter 2 Value MSB
OCR2AL		sfr	0xc4		; Timer/Counter 2 Output Compare Value A LSB
OCR2AH		sfr	0xc5		; Timer/Counter 2 Output Compare Value A MSB
OCR2BL		sfr	0xc2		; Timer/Counter 2 Output Compare Value B LSB
OCR2BH		sfr	0xc3		; Timer/Counter 2 Output Compare Value B MSB
ICR2L		sfr	0xc0		; Timer/Counter 2 Input Capture LSB
ICR2H		sfr	0xc1		; Timer/Counter 2 Input Capture MSB


TIMSK0		port	0x39		; Timer/Counter 0 Interrupt Mask Register
TOIE0		equ	0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		port	0x0f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		equ	0		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		equ	1		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	2		; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		equ	5		; Timer/Counter 1 Input Capture Interrupt Enable

TIMSK2		port	0x11		; Timer/Counter 2 Interrupt Mask Register
TOIE2		equ	0		; Timer/Counter 2 Overflow Interrupt Enable
OCIE2B		equ	1		; Timer/Counter 2 Output Compare Interrupt Enable B
OCIE2A		equ	2		; Timer/Counter 2 Output Compare Interrupt Enable A
ICIE2		equ	5		; Timer/Counter 2 Input Capture Interrupt Enable

TIFR0		port	0x38		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x0e		; Timer/Counter 1 Interrupt Flag Register

TIFR2		port	0x10		; Timer/Counter 2 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR		equ	0		; Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

TOCPMSA0	sfr	0x67		; Timer/Counter Output Compare Pin Mux Selection Register 0
TOCC0S0		equ	0		; Timer/Counter Output Compare Channel Select 0
TOCC0S1		equ	1
TOCC1S0		equ	2		; Timer/Counter Output Compare Channel Select 1
TOCC1S1		equ	3
TOCC2S0		equ	4		; Timer/Counter Output Compare Channel Select 2
TOCC2S1		equ	5
TOCC3S0		equ	6		; Timer/Counter Output Compare Channel Select 3
TOCC3S1		equ	7 

TOCPMSA1	sfr	0x68		; Timer/Counter Output Compare Pin Mux Selection Register 1
TOCC4S0		equ	0		; Timer/Counter Output Compare Channel Select 4
TOCC4S1		equ	1
TOCC5S0		equ	2		; Timer/Counter Output Compare Channel Select 5
TOCC5S1		equ	3
TOCC6S0		equ	4		; Timer/Counter Output Compare Channel Select 6
TOCC6S1		equ	5
TOCC7S0		equ	6		; Timer/Counter Output Compare Channel Select 7
TOCC7S1		equ	7 

TOCPMCOE	sfr	0x66		; Timer/Counter Output Compare Pin Mux Channel Output Enable
TOCC0OE		equ	0		; Timer/Counter Output Compare Channel 0 Output Enable
TOCC01E		equ	1		; Timer/Counter Output Compare Channel 1 Output Enable
TOCC02E		equ	2		; Timer/Counter Output Compare Channel 2 Output Enable
TOCC03E		equ	3		; Timer/Counter Output Compare Channel 3 Output Enable
TOCC04E		equ	4		; Timer/Counter Output Compare Channel 4 Output Enable
TOCC05E		equ	5		; Timer/Counter Output Compare Channel 5 Output Enable
TOCC06E		equ	6		; Timer/Counter Output Compare Channel 6 Output Enable
TOCC07E		equ	7		; Timer/Counter Output Compare Channel 7 Output Enable

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x21		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDP3		equ	5
WDIE		equ	6		; interrupt enable
WDIF		equ	7		; interrupt flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSR0A		port	0x0a
ACIS00		equ     0               ; Analog Comparator 0 Interrupt-Mode
ACIS01		equ     1
ACIC0	 	equ	2		; Analog Comparator 0 Use As Capture Signal For Timer 1?
ACIE0		equ     3               ; Analog Comparator 0 Interrupt Enable
ACI0		equ     4               ; Analog Comparator 0 Interrupt Flag
ACO0		equ     5               ; Analog Comparator 0 Output
ACPMUX02	equ	6		; Analog Comparator 0 Positive Input Multiplexer
ACD0		equ	7		; Analog Comparator 0 Disable
ACSR0B		port	0x0b
ACPMUX0		equ	0		; Analog Comparator 0 Positive Input Multiplexer
ACPMUX1		equ	1
ACNMUX0		equ	2		; Analog Comparator 0 Negative Input Multiplexer
ACNMUX1		equ	3
ACOE0		equ	4		; Analog Comparator 0 Output Enable 0
HLEV0		equ	6		; Analog Comparator 0 Hysteresis Level
HSEL0		equ	7		; Analog Comparator 0 Hysteresis Select

ACSR1A		port	0x0c
ACIS10		equ     0               ; Analog Comparator 1 Interrupt-Mode
ACIS11		equ     1
ACIC1	 	equ	2		; Analog Comparator 1 Use As Capture Signal For Timer 2?
ACIE1		equ     3               ; Analog Comparator 1 Interrupt Enable
ACI1		equ     4               ; Analog Comparator 1 Interrupt Flag
ACO1		equ     5               ; Analog Comparator 1 Output
ACBG1		equ	6		; Analog Comparator 1 Bandgap Select
ACD1		equ	7		; Analog Comparator 1 Disable
ACSR1B		port	0x0d
ACME1		equ	2		; Analog Comparator 1 Multiplexer Enable
ACOE1		equ	4		; Analog Comparator 1 Output Enable 1
HLEV1		equ	6		; Analog Comparator 1 Hysteresis Level
HSEL1		equ	7		; Analog Comparator 1 Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX0		port	0x09		; ADC Multiplexer Selection Register A
MUX5		equ	5		; Analog Channel and Gain Selection Bits
MUX4		equ	4
MUX3		equ	3
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADMUX1		port	0x08		; ADC Multiplexer Selection Register B
REFS2		equ	7		; Reference Selection Bits
REFS1		equ	6
REFS0		equ	5
GSEL1		equ	1		; Gain Selection
GSEL0		equ	0

ADCSRA		port	0x05		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x04		; Control/Status Register B
ADLAR		equ	3		; Left Adjust Result
ADTS2		equ	2		; Auto Trigger Source
ADTS1		equ	1
ADTS0		equ	0

ADCH		port	0x06		; Data Register
ADCL		port	0x07

DIDR0		sfr	0x60		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Buffer Disable
ADC1D		equ	1		; ADC1 Digital Input Buffer Disable
ADC2D		equ	2		; ADC2 Digital Input Buffer Disable
ADC3D		equ	3		; ADC3/AIN10 Digital Input Buffer Disable
ADC4D		equ	4		; ADC4/AIN11 Digital Input Buffer Disable
ADC5D		equ	5		; ADC5 Digital Input Buffer Disable
ADC6D		equ	6		; ADC6 Digital Input Buffer Disable
ADC7D		equ	7		; ADC7 Digital Input Buffer Disable

DIDR1		sfr	0x61		; Digital Input Disable Register 1
ADC11D		equ	0		; ADC11 Digital Input Buffer Disable
ADC10D		equ	1		; ADC10 Digital Input Buffer Disable
ADC8D		equ	2		; ADC8 Digital Input Buffer Disable
ADC9D		equ	3		; ADC9 Digital Input Buffer Disable

;----------------------------------------------------------------------------
; USART

UDR0		sfr	0x80		; USART0 I/O Data Register
UCSR0A		sfr	0x86		; USART0 Control & Status Register A
MPCM0		equ	0		; USART0 Multi Processor Communication Mode
U2X0		equ	1		; USART0 Double Transmission Speed
UPE0		equ	2		; USART0 Parity Error
DOR0		equ	3		; USART0 Overrun
FE0		equ	4		; USART0 Framing Error
UDRE0		equ	5		; USART0 Data Register Empty
TXC0		equ	6		; USART0 Transmit Complete
RXC0		equ	7		; USART0 Receive Complete
UCSR0B		sfr	0x85		; USART0 Control & Status Register B
TXB80		equ     0		; USART0 Transmit Bit 8
RXB80		equ     1		; USART0 Receive Bit 8
UCSZ02		equ     2		; USART0 Character Size
TXEN0		equ     3		; USART0 Enable Transmitter
RXEN0		equ     4		; USART0 Enable Receiver
UDRIE0		equ     5		; USART0 Enable Data Register Empty Interrupt
TXCIE0		equ     6		; USART0 Enable Transmit Complete Interrupt
RXCIE0		equ     7		; USART0 Enable Receive Complete Interrupt
UCSR0C		sfr	0x84		; USART0 Control & Status Register C
UCPOL0		equ	0		; USART0 clock polarity
UCSZ00		equ	1		; USART0 character size
UCSZ01		equ	2
USBS0		equ	3		; USART0 Stop Bit Select
UPM00		equ	4		; USART0 Parity Mode : Odd/Even
UPM01		equ	5		; USART0 Parity Mode : Enable/Disable
UMSEL00		equ	6		; USART0 Mode Select
UMSEL01		equ	7
UCSR0D		sfr	0x83		; USART0 Control & Status Register D
SFDE0		equ	5		; USART0 Start Frame Detection Enable
RXS0		equ	6		; USART0 RX Start
RXSIE0		equ	7		; USART0 RX Start Interrupt Enable
UBRR0L		sfr	0x81		; USART0 Baud Rate Register Low
UBRR0H		sfr	0x82		; USART0 Baud Rate Register High

UDR1		sfr	0x90		; USART1 I/O Data Register
UCSR1A		sfr	0x96		; USART1 Control & Status Register A
MPCM1		equ	0		; USART1 Multi Processor Communication Mode
U2X1		equ	1		; USART1 Double Transmission Speed
UPE1		equ	2		; USART1 Parity Error
DOR1		equ	3		; USART1 Overrun
FE1		equ	4		; USART1 Framing Error
UDRE1		equ	5		; USART1 Data Register Empty
TXC1		equ	6		; USART1 Transmit Complete
RXC1		equ	7		; USART1 Receive Complete
UCSR1B		sfr	0x95		; USART1 Control & Status Register B
TXB81		equ     0		; USART1 Transmit Bit 8
RXB81		equ     1		; USART1 Receive Bit 8
UCSZ12		equ     2		; USART1 Character Size
TXEN1		equ     3		; USART1 Enable Transmitter
RXEN1		equ     4		; USART1 Enable Receiver
UDRIE1		equ     5		; USART1 Enable Data Register Empty Interrupt
TXCIE1		equ     6		; USART1 Enable Transmit Complete Interrupt
RXCIE1		equ     7		; USART1 Enable Receive Complete Interrupt
UCSR1C		sfr	0x94		; USART1 Control & Status Register C
UCPOL1		equ	0		; USART1 clock polarity
UCSZ10		equ	1		; USART1 character size
UCSZ11		equ	2
USBS1		equ	3		; USART1 Stop Bit Select
UPM10		equ	4		; USART1 Parity Mode : Odd/Even
UPM11		equ	5		; USART1 Parity Mode : Enable/Disable
UMSEL10		equ	6		; USART1 Mode Select
UMSEL11		equ	7
UCSR1D		sfr	0x93		; USART1 Control & Status Register D
SFDE1		equ	5		; USART1 Start Frame Detection Enable
RXS1		equ	6		; USART1 RX Start
RXSIE1		equ	7		; USART1 RX Start Interrupt Enable
UBRR1L		sfr	0x91		; USART1 Baud Rate Register Low
UBRR1H		sfr	0x92		; USART1 Baud Rate Register High

REMAP		sfr	0x65		; Remap Port Pins
U0MAP		equ	1		; USART0 Pin Mapping

;----------------------------------------------------------------------------
; TWI

TWSCRA		sfr	0xa5		; TWI Slave Control Register A
TWSME		equ	0		; TWI Smart Mode Enable
TWPME		equ	1		; TWI Promiscuous Mode Enable
TWSIE		equ	2		; TWI Stop Interrupt Enable
TWEN		equ	3		; TWI Enable
TWASIE		equ	4		; TWI Address/Stop Interrupt Enable
TWDIE		equ	5		; TWI Data Interrupt Enable
TWSHE		equ	7		; TWI SDA Hold Time Enable

TWSCRB		sfr	0xa4		; TWI Slave Control Register B
TWCMD0		equ	0		; TWI Command
TWCMD1		equ	1
TWAA		equ	2		; TWI Acknowledge Action
TWHNM		equ	3		; TWI High Noise Mode

TWSSRA		sfr	0xa3		; TWI Slave Status Register A
TWAS		equ	0		; TWI Address or Stop
TWDIR		equ	1		; TWI Read/Write Direction
TWBE		equ	2		; TWI Bus Error
TWC		equ	3		; TWI Collision
TWRA		equ	4		; TWI Receive Acknowledge
TWCH		equ	5		; TWI Clock Hold
TWASIF		equ	6		; TWI Address/Stop Interrupt Flag
TWDIF		equ	7		; TWI Data Interrupt Flag

TWSA		sfr	0xa2		; TWI Slave Address Register

TWSAM		sfr	0xa1		; TWI Slave Address Mask Register

TWSD		sfr	0xa0		; TWI Slave Data Register

;----------------------------------------------------------------------------
; SPI

SPCR		sfr	0xb2		; SPI Control Register
SPR0		equ	0		; Clock Select
SPR1		equ	1
CPHA		equ	2		; Clock Phase
CPOL		equ	3		; Clock Polarity
MSTR		equ	4		; Master/Slave Selection
DORD		equ	5		; Bit Order
SPE		equ	6		; Enable SPI
SPIE		equ	7		; SPI Interrupt Enable

SPSR		sfr	0xb1		; SPI Status Register
SPI2X		equ	0		; Double Speed Mode
WCOL		equ	6		; Write Collision
SPIF		equ	7		; SPI Interrupt Occured?

SPDR		sfr	0xb0		; SPI data register

		; bits in REMAP
SPIMAP		equ	1		; SPI Pin Mapping

		restore			; re-enable listing

		endif			; __regtnx41inc
