		ifndef	__regtn1634inc
__regtn1634inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN1634.INC                                           *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny1634                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     255		; End Address EEPROM
RAMSTART	equ	0x100,data	; Start Address SRAM
RAMEND		equ     0x4ff,data	; End Address SRAM
FLASHEND	label   16383		; End Address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x36		; MCU General Control Register
SE		avrbit	MCUCR,4		; Sleep Enable
SM0		avrbit	MCUCR,5		; Sleep Mode Select
SM1		avrbit	MCUCR,6

MCUSR		port	0x35		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL0		sfr	0x63		; Oscillator Calibration
OSSCAL1		sfr	0x66

OSCTCAL0A	sfr	0x64		; Oscillator Temperature Compensation
OSCTCAL0B	sfr	0x65

CLKPR		port	0x33		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3

CLKSR		port	0x32		; Clock Status Register
CKSEL0		avrbit	CLKSR,0		; Clock Select
CKSEL1		avrbit	CLKSR,1
CKSEL2		avrbit	CLKSR,2
CKSEL3		avrbit	CLKSR,3
SUT		avrbit	CLKSR,4		; Start-Up Time
CKOUT_IO	avrbit	CLKSR,5		; Clock Output
CSTR		avrbit	CLKSR,6		; Clock Select Trigger
OSCRDY		avrbit	CLKSR,7		; Oscillator Ready

PRR		port	0x34		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRUSART0	avrbit	PRR,1		; Power Reduction USART0
PRUSART1	avrbit	PRR,2		; Power Reduction USART1
PRUSI		avrbit	PRR,3		; Power Reduction USI
PRTIM0		avrbit	PRR,1		; Power Reduction Timer/Counter 0
PRTIM1		avrbit	PRR,2		; Power Reduction Timer/Counter 1
PRTWI		avrbit	PRR,7		; Power Reduction Two Wire Interface

CCP		port	0x2f		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x1e		; EEPROM Address Register Low
EEARH		port	0x1f		; EEPROM Address Register High
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
RSIG		avrbit	SPMCSR,5	; Read Device Signature Imprint Table
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x2e		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x0f		; Port A @ 0x0f (IO) ff.
PINB		port	0x0b		; Port B @ 0x0b (IO) ff.
PINC		port	0x07		; Port C @ 0x07 (IO) ff. (bit 0..5)

GPIOR0		port	0x14		; General Purpose I/O Register 0
GPIOR1		port	0x15		; General Purpose I/O Register 1
GPIOR2		port	0x16		; General Purpose I/O Register 2

PORTCR		port	0x13		; Port Control Register
BBMA		avrbit	PORTCR,0	; Break-Before-Make Mode Enable Port A
BBMB		avrbit	PORTCR,1	; Break-Before-Make Mode Enable Port B
BBMC		avrbit	PORTCR,2	; Break-Before-Make Mode Enable Port C

PUEA		port	0x12		; Pull-Up Enable Port A
PUEA0		avrbit	PUEA,0
PUEA1		avrbit	PUEA,1
PUEA2		avrbit	PUEA,2
PUEA3		avrbit	PUEA,3
PUEA4		avrbit	PUEA,4
PUEA5		avrbit	PUEA,5
PUEA6		avrbit	PUEA,6
PUEA7		avrbit	PUEA,7

PUEB		port	0x0e		; Pull-Up Enable Port B
PUEB0		avrbit	PUEB,0
PUEB1		avrbit	PUEB,1
PUEB2		avrbit	PUEB,2
PUEB3		avrbit	PUEB,3
PUEB4		avrbit	PUEB,4
PUEB5		avrbit	PUEB,5
PUEB6		avrbit	PUEB,6
PUEB7		avrbit	PUEB,7

PUEC		port	0x0a		; Pull-Up Enable Port C
PUEC0		avrbit	PUEC,0
PUEC1		avrbit	PUEC,1
PUEC2		avrbit	PUEC,2
PUEC3		avrbit	PUEC,3
PUEC4		avrbit	PUEC,4
PUEC5		avrbit	PUEC,5

PCMSK0		port	0x27		; Pin Change Interrupt Mask 0
PCMSK1		port	0x28		; Pin Change Interrupt Mask 1
PCINT8		avrbit	PCMSK1,0	; Enable Pin Change Interrupt 8
PCINT9		avrbit	PCMSK1,1	; Enable Pin Change Interrupt 9
PCINT10		avrbit	PCMSK1,2	; Enable Pin Change Interrupt 10
PCINT11		avrbit	PCMSK1,3	; Enable Pin Change Interrupt 11
PCMSK2		port	0x29		; Pin Change Interrupt Mask 2
PCINT12		avrbit	PCMSK2,0	; Enable Pin Change Interrupt 12
PCINT13		avrbit	PCMSK2,1	; Enable Pin Change Interrupt 13
PCINT14		avrbit	PCMSK2,2	; Enable Pin Change Interrupt 14
PCINT15		avrbit	PCMSK2,3	; Enable Pin Change Interrupt 15
PCINT16		avrbit	PCMSK2,4	; Enable Pin Change Interrupt 16
PCINT17		avrbit	PCMSK2,5	; Enable Pin Change Interrupt 17

QTCSR		port	0x38		; QTouch Control/Status Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 INT0_vect=2		; External Interrupt Request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum PCINT2_vect		; Pin Change Interrupt 2
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ANA_COMP_vect		; Analog Comparator 0
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum USART0_START_vect	; USART0 Rx Start
		nextenum USART0_RX_vect		; USART0 Rx Complete
		nextenum USART0_UDRE_vect	; USART0 Data Register Empty
		nextenum USART0_TX_vect		; USART0 Tx Complete
		nextenum USART1_START_vect	; USART1 Rx Start
		nextenum USART1_RX_vect		; USART1 Rx Complete
		nextenum USART1_UDRE_vect	; USART1 Data Register Empty
		nextenum USART1_TX_vect		; USART1 Tx Complete
		nextenum USI_START_vect		; USI START
		nextenum USI_OVF_vect		; USI Overflow
		nextenum TWI_SLAVE_vect		; 2-wire Serial Interface
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum QTRIP_vect		; QTRIP QTouch

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3c		; General Interrupt Mask Register
PCIE0		avrbit	GIMSK,3		; Enable Pin Change Interrupt 0
PCIE1		avrbit	GIMSK,4		; Enable Pin Change Interrupt 1
PCIE2		avrbit	GIMSK,5		; Enable Pin Change Interrupt 2
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0

GIFR		port	0x3b		; General Interrupt Flag Register
PCIF0		avrbit	GIFR,3		; Pin Change Interrupt 0 Occured
PCIF1		avrbit	GIFR,4		; Pin Change Interrupt 1 Occured
PCIF2		avrbit	GIFR,5		; Pin Change Interrupt 2 Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x1b		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 1 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 1 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x1a		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x19		; Timer/Counter 0 Value
OCR0A		port	0x18		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x17		; Timer/Counter 0 Output Compare Value B

TCCR1A		sfr	0x72		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,5
TCCR1B		sfr	0x71		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		sfr	0x70		; Timer/Counter 1 Control Register C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		sfr	0x6e		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x6f		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x6c		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x6d		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x6a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x6b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x68		; Timer/Counter 1 Input Capture LSB
ICR1H		sfr	0x69		; Timer/Counter 1 Input Capture MSB

TIMSK0		port	0x3a		; Timer/Counter Interrupt Mask Register
OCIE0A		avrbit	TIMSK0,0	; Timer/Counter 0 Output Compare Interrupt Enable A
TOIE0		avrbit	TIMSK0,1	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE1		avrbit	TIMSK0,3	; Timer/Counter 1 Input Capture Interrupt Enable
OCIE1B		avrbit	TIMSK0,5	; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK0,6	; Timer/Counter 1 Output Compare Interrupt Enable A
TOIE1		avrbit	TIMSK0,7	; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x39		; Timer/Counter Interrupt Flag Register

GTCCR		sfr	0x67		; General Timer/Counter Control Register
PSR10		avrbit	GTCCR,0		; Prescaler Reset
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x30		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable Watchdog
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Interrupt Enable
WDIF		avrbit	WDTCSR,7	; Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x06		; Analog Comparator Control/Status Register A
ACIS0		avrbit	ACSRA,0		; Analog Comparator Interrupt-Mode
ACIS1		avrbit	ACSRA,1
ACIC	 	avrbit	ACSRA,2		; Analog Comparator Use As Capture Signal For Timer 1?
ACIE		avrbit	ACSRA,3		; Analog Comparator Interrupt Enable
ACI		avrbit	ACSRA,4		; Analog Comparator Interrupt Flag
ACO		avrbit	ACSRA,5		; Analog Comparator Output
ACBG		avrbit	ACSRA,6		; Analog Comparator Bang Gap Select
ACD		avrbit	ACSRA,7		; Analog Comparator Disable
ACSRB		port	0x05		; Analog Comparator Control/Status Register B
ACIRS0		avrbit	ACSRB,0		; reserved for QTouch
ACIRS1		avrbit	ACSRB,1
ACME		avrbit	ACSRB,2		; Analog Comparator Multiplexer Enable
ACCE		avrbit	ACSRB,3		; Reserved for QTouch
ACLP		avrbit	ACSRB,5		; Reserved for QTouch
HLEV		avrbit	ACSRB,6		; Hysteresis Level
HSEL		avrbit	ACSRB,7		; Hysteresis Select

DIDR0		sfr	0x60		; Digital Input Disable Register 0
AIN0D		avrbit	DIDR0,1		; AIN0 Digital Input Disable
AIN1D		avrbit	DIDR0,2		; AIN1 Digital Input Disable

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x04		; ADC Multiplexer Selection Register
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
REFEN		avrbit	ADMUX,5		; Reserved for QTouch
ADC0EN		avrbit	ADMUX,4		; Reserved for QTouch
MUX3		avrbit	ADMUX,3		; Analog Channel and Gain Selection Bits
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSRA		port	0x03		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		port	0x02		; Control/Status Register B
VDEN		avrbit	ADCSRB,7	; Reserved for QTouch
VDPD		avrbit	ADCSRB,6	; Reserved for QTouch
ADLAR		avrbit	ADCSRB,3	; Left Adjust Result
ADTS2		avrbit	ADCSRB,2	; Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS0		avrbit	ADCSRB,0

ADCH		port	0x01		; Data Register
ADCL		port	0x00

AREFD		avrbit	DIDR0,0		; 
ADC0D		avrbit	DIDR0,3		; ADC0 Digital Input Buffer Disable
ADC1D		avrbit	DIDR0,4		; ADC1 Digital Input Buffer Disable
ADC2D		avrbit	DIDR0,5		; ADC2 Digital Input Buffer Disable
ADC3D		avrbit	DIDR0,6		; ADC3/AIN10 Digital Input Buffer Disable
ADC4D		avrbit	DIDR0,7		; ADC4/AIN11 Digital Input Buffer Disable

DIDR1		sfr	0x61		; Digital Input Disable Register 1
ADC5D		avrbit	DIDR1,0		; ADC5 Digital Input Buffer Disable
ADC6D		avrbit	DIDR1,1		; ADC6 Digital Input Buffer Disable
ADC7D		avrbit	DIDR1,2		; ADC7 Digital Input Buffer Disable
ADC8D		avrbit	DIDR1,3		; ADC8 Digital Input Buffer Disable

DIDR2		sfr	0x62		; Digital Input Disable Register 2
ADC9D		avrbit	DIDR2,0		; ADC9 Digital Input Buffer Disable
ADC10D		avrbit	DIDR2,1		; ADC10 Digital Input Buffer Disable
ADC11D		avrbit	DIDR2,2		; ADC11 Digital Input Buffer Disable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x20		; USART0 I/O Data Register
UCSR0A		port	0x26		; USART0 Control & Status Register A
MPCM0		avrbit	UCSR0A,0	; USART0 Multi Processor Communication Mode
U2X0		avrbit	UCSR0A,1	; USART0 Double Transmission Speed
UPE0		avrbit	UCSR0A,2	; USART0 Parity Error
DOR0		avrbit	UCSR0A,3	; USART0 Overrun
FE0		avrbit	UCSR0A,4	; USART0 Framing Error
UDRE0		avrbit	UCSR0A,5	; USART0 Data Register Empty
TXC0		avrbit	UCSR0A,6	; USART0 Transmit Complete
RXC0		avrbit	UCSR0A,7	; USART0 Receive Complete
UCSR0B		port	0x25		; USART0 Control & Status Register B
TXB80		avrbit	UCSR0B,0	; USART0 Transmit Bit 8
RXB80		avrbit	UCSR0B,1	; USART0 Receive Bit 8
UCSZ02		avrbit	UCSR0B,2	; USART0 Character Size
TXEN0		avrbit	UCSR0B,3	; USART0 Enable Transmitter
RXEN0		avrbit	UCSR0B,4	; USART0 Enable Receiver
UDRIE0		avrbit	UCSR0B,5	; USART0 Enable Data Register Empty Interrupt
TXCIE0		avrbit	UCSR0B,6	; USART0 Enable Transmit Complete Interrupt
RXCIE0		avrbit	UCSR0B,7	; USART0 Enable Receive Complete Interrupt
UCSR0C		port	0x24		; USART0 Control & Status Register C
UCPOL0		avrbit	UCSR0C,0	; USART0 Clock polarity
UCSZ00		avrbit	UCSR0C,1	; USART0 character size
UCSZ01		avrbit	UCSR0C,2
USBS0		avrbit	UCSR0C,3	; USART0 Stop Bit Select
UPM00		avrbit	UCSR0C,4	; USART0 Parity Mode : Odd/Even
UPM01		avrbit	UCSR0C,5	; USART0 Parity Mode : Enable/Disable
UMSEL00		avrbit	UCSR0C,6	; USART0 Mode Select
UMSEL01		avrbit	UCSR0C,7
UCSR0D		port	0x23		; USART0 Control & Status Register D
SFDE0		avrbit	UCSR0D,5	; USART0 Start Frame Detection Enable
RXS0		avrbit	UCSR0D,6	; USART0 RX Start
RXSIE0		avrbit	UCSR0D,7	; USART0 RX Start Interrupt Enable
UBRR0L		port	0x21		; USART0 Baud Rate Register Low
UBRR0H		port	0x22		; USART0 Baud Rate Register High

UDR1		sfr	0x73		; USART1 I/O Data Register
UCSR1A		sfr	0x79		; USART1 Control & Status Register A
MPCM1		avrbit	UCSR1A,0	; USART1 Multi Processor Communication Mode
U2X1		avrbit	UCSR1A,1	; USART1 Double Transmission Speed
UPE1		avrbit	UCSR1A,2	; USART1 Parity Error
DOR1		avrbit	UCSR1A,3	; USART1 Overrun
FE1		avrbit	UCSR1A,4	; USART1 Framing Error
UDRE1		avrbit	UCSR1A,5	; USART1 Data Register Empty
TXC1		avrbit	UCSR1A,6	; USART1 Transmit Complete
RXC1		avrbit	UCSR1A,7	; USART1 Receive Complete
UCSR1B		sfr	0x78		; USART1 Control & Status Register B
TXB81		avrbit	UCSR1B,0	; USART1 Transmit Bit 8
RXB81		avrbit	UCSR1B,1	; USART1 Receive Bit 8
UCSZ12		avrbit	UCSR1B,2	; USART1 Character Size
TXEN1		avrbit	UCSR1B,3	; USART1 Enable Transmitter
RXEN1		avrbit	UCSR1B,4	; USART1 Enable Receiver
UDRIE1		avrbit	UCSR1B,5	; USART1 Enable Data Register Empty Interrupt
TXCIE1		avrbit	UCSR1B,6	; USART1 Enable Transmit Complete Interrupt
RXCIE1		avrbit	UCSR1B,7	; USART1 Enable Receive Complete Interrupt
UCSR1C		sfr	0x77		; USART1 Control & Status Register C
UCPOL1		avrbit	UCSR1C,0	; USART1 Clock polarity
UCSZ10		avrbit	UCSR1C,1	; USART1 character size
UCSZ11		avrbit	UCSR1C,2
USBS1		avrbit	UCSR1C,3	; USART1 Stop Bit Select
UPM10		avrbit	UCSR1C,4	; USART1 Parity Mode : Odd/Even
UPM11		avrbit	UCSR1C,5	; USART1 Parity Mode : Enable/Disable
UMSEL10		avrbit	UCSR1C,6	; USART1 Mode Select
UMSEL11		avrbit	UCSR1C,7
UCSR1D		sfr	0x76		; USART1 Control & Status Register D
SFDE1		avrbit	UCSR1D,5	; USART1 Start Frame Detection Enable
RXS1		avrbit	UCSR1D,6	; USART1 RX Start
RXSIE1		avrbit	UCSR1D,7	; USART1 RX Start Interrupt Enable
UBRR1L		sfr	0x74		; USART1 Baud Rate Register Low
UBRR1H		sfr	0x75		; USART1 Baud Rate Register High

;----------------------------------------------------------------------------
; TWI

TWSCRA		sfr	0x7f		; TWI Slave Control Register A
TWSME		avrbit	TWSCRA,0	; TWI Smart Mode Enable
TWPME		avrbit	TWSCRA,1	; TWI Promiscuous Mode Enable
TWSIE		avrbit	TWSCRA,2	; TWI Stop Interrupt Enable
TWEN		avrbit	TWSCRA,3	; TWI Enable
TWASIE		avrbit	TWSCRA,4	; TWI Address/Stop Interrupt Enable
TWDIE		avrbit	TWSCRA,5	; TWI Data Interrupt Enable
TWSHE		avrbit	TWSCRA,7	; TWI SDA Hold Time Enable

TWSCRB		sfr	0x7e		; TWI Slave Control Register B
TWCMD0		avrbit	TWSCRB,0	; TWI Command
TWCMD1		avrbit	TWSCRB,1
TWAA		avrbit	TWSCRB,2	; TWI Acknowledge Action

TWSSRA		sfr	0x7d		; TWI Slave Status Register A
TWAS		avrbit	TWSSRA,0	; TWI Address or Stop
TWDIR		avrbit	TWSSRA,1	; TWI Read/Write Direction
TWBE		avrbit	TWSSRA,2	; TWI Bus Error
TWC		avrbit	TWSSRA,3	; TWI Collision
TWRA		avrbit	TWSSRA,4	; TWI Receive Acknowledge
TWCH		avrbit	TWSSRA,5	; TWI Clock Hold
TWASIF		avrbit	TWSSRA,6	; TWI Address/Stop Interrupt Flag
TWDIF		avrbit	TWSSRA,7	; TWI Data Interrupt Flag

TWSA		sfr	0x7c		; TWI Slave Address Register

TWSAM		sfr	0x7b		; TWI Slave Address Mask Register

TWSD		sfr	0x7a		; TWI Slave Data Register

;----------------------------------------------------------------------------
; USI

USIBR		port	0x2d		; USI Buffer Register

USIDR		port	0x2c		; USI Data Register

USISR		port	0x2b		; USI Status Register
USICNT0		avrbit	USISR,0		; Counter Value
USICNT1		avrbit	USISR,1
USICNT2		avrbit	USISR,2
USICNT3		avrbit	USISR,3
USIDC		avrbit	USISR,4		; Data Output Collision
USIPF		avrbit	USISR,5		; Stop Condition Flag
USIOIF		avrbit	USISR,6		; Counter Overflow Interrupt Flag
USISIF		avrbit	USISR,7		; Start Condition Interrupt Flag

USICR		port	0x2a		; USI Control Register
USITC		avrbit	USICR,0		; Toggle Clock Port Pin
USICLK		avrbit	USICR,1		; Clock Strobe
USICS0		avrbit	USICR,2		; Clock Source Select
USICS1		avrbit	USICR,3
USIWM0		avrbit	USICR,4		; Wire Mode
USIWM1		avrbit	USICR,5		
USIOIE		avrbit	USICR,6		; Counter Overflow Interrupt Enable
USISIE		avrbit	USICR,7		; Start Condition Interrupt Enable

		restore			; re-enable listing

		endif			; __regtn1634inc
