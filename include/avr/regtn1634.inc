		ifndef	__regtn1634inc
__regtn1634inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN1634.INC                                           *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny1634                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     255		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x4ff,data	; end address SRAM
FLASHEND	label   16383		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x36		; MCU General Control Register
SE		equ	4		; Sleep Enable
SM0		equ	5		; Sleep Mode Select
SM1		equ	6

MCUSR		port	0x35		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL0		sfr	0x63		; Oscillator Calibration
OSSCAL1		sfr	0x66

OSCTCAL0A	sfr	0x64		; Oscillator Temperature Compensation
OSCTCAL0B	sfr	0x65

CLKPR		port	0x33		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3

CLKSR		port	0x32		; Clock Status Register
CKSEL0		equ	0		; Clock Select
CKSEL1		equ	1
CKSEL2		equ	2
CKSEL3		equ	3
SUT		equ	4		; Start-Up Time
CKOUT_IO	equ	5		; Clock Output
CSTR		equ	6		; Clock Select Trigger
OSCRDY		equ	7		; Oscillator Ready

PRR		port	0x34		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSART0	equ	1		; Power Reduction USART0
PRUSART1	equ	2		; Power Reduction USART1
PRUSI		equ	3		; Power Reduction USI
PRTIM0		equ	1		; Power Reduction Timer/Counter 0
PRTIM1		equ	2		; Power Reduction Timer/Counter 1
PRTWI		equ	7		; Power Reduction Two Wire Interface

CCP		port	0x2f		; Configuration Change Protection Register

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
BBMA		equ	0		; Break-Before-Make Mode Enable Port A
BBMB		equ	1		; Break-Before-Make Mode Enable Port B
BBMC		equ	2		; Break-Before-Make Mode Enable Port C

PUEA		port	0x12		; Pull-up Enable Port A
PUEA0		equ	0
PUEA1		equ	1
PUEA2		equ	2
PUEA3		equ	3
PUEA4		equ	4
PUEA5		equ	5
PUEA6		equ	6
PUEA7		equ	7

PUEB		port	0x0e		; Pull-up Enable Port B
PUEB0		equ	0
PUEB1		equ	1
PUEB2		equ	2
PUEB3		equ	3
PUEB4		equ	4
PUEB5		equ	5
PUEB6		equ	6
PUEB7		equ	7

PUEC		port	0x0a		; Pull-up Enable Port C
PUEC0		equ	0
PUEC1		equ	1
PUEC2		equ	2
PUEC3		equ	3
PUEC4		equ	4
PUEC5		equ	5

PCMSK0		port	0x27		; Pin Change Interrupt Mask 0
PCMSK1		port	0x28		; Pin Change Interrupt Mask 1
PCINT8		equ	0		; Enable Pin Change Interrupt 8
PCINT9		equ	1		; Enable Pin Change Interrupt 9
PCINT10		equ	2		; Enable Pin Change Interrupt 10
PCINT11		equ	3		; Enable Pin Change Interrupt 11
PCMSK2		port	0x29		; Pin Change Interrupt Mask 2
PCINT12		equ	0		; Enable Pin Change Interrupt 12
PCINT13		equ	1		; Enable Pin Change Interrupt 13
PCINT14		equ	2		; Enable Pin Change Interrupt 14
PCINT15		equ	3		; Enable Pin Change Interrupt 15
PCINT16		equ	4		; Enable Pin Change Interrupt 16
PCINT17		equ	5		; Enable Pin Change Interrupt 17

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

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1

GIMSK		port	0x3c		; General Interrupt Mask Register
PCIE0		equ	3		; Enable Pin Change Interrupt 0
PCIE1		equ	4		; Enable Pin Change Interrupt 1
PCIE2		equ	5		; Enable Pin Change Interrupt 2
INT0		equ	6		; Enable External Interrupt 0

GIFR		port	0x3b		; General Interrupt Flag Register
PCIF0		equ	3		; Pin Change Interrupt 0 Occured
PCIF1		equ	4		; Pin Change Interrupt 1 Occured
PCIF2		equ	5		; Pin Change Interrupt 2 Occured
INTF0		equ	6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x1b		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x1a		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare Match B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x19		; Timer/Counter 0 Value
OCR0A		port	0x18		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x17		; Timer/Counter 0 Output Compare Value B

TCCR1A		sfr	0x72		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	4		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	5
TCCR1B		sfr	0x71		; Timer/Counter 1 Control Register B
CS10		equ     0               ; Timer/Counter 1 Clock Select
CS11		equ     1
CS12		equ     2
WGM12		equ	3
WGM13		equ	4
ICES1		equ	6		; Timer/Counter 1 Input Capture Edge Selecr
ICNC1		equ	7		; Timer/Counter 1 Input Capture Noise Canceling
TCCR1C		sfr	0x70		; Timer/Counter 1 Control Register C
FOC1B		equ	6		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	7		; Timer/Counter 1 Force Output Compare A
TCNT1L		sfr	0x6e		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x6f		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x6c		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x6d		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		sfr	0x6a		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		sfr	0x6b		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		sfr	0x68		; Timer/Counter 1 Input Capture LSB
ICR1H		sfr	0x69		; Timer/Counter 1 Input Capture MSB

TIMSK0		port	0x3a		; Timer/Counter Interrupt Mask Register
OCIE0A		equ	0		; Timer/Counter 0 Output Compare Interrupt Enable A
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE1		equ	3		; Timer/Counter 1 Input Capture Interrupt Enable
OCIE1B		equ	5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	6		; Timer/Counter 1 Output Compare Interrupt Enable A
TOIE1		equ	7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x39		; Timer/Counter Interrupt Flag Register

GTCCR		sfr	0x67		; General Timer/Counter Control Register
PSR10		equ	0		; Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x30		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDP3		equ	5
WDIE		equ	6		; interrupt enable
WDIF		equ	7		; interrupt flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x06		; Analog Comparator Control/Status Register A
ACIS0		equ     0               ; Analog Comparator Interrupt-Mode
ACIS1		equ     1
ACIC	 	equ	2		; Analog Comparator Use As Capture Signal For Timer 1?
ACIE		equ     3               ; Analog Comparator Interrupt Enable
ACI		equ     4               ; Analog Comparator Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACBG		equ	6		; Analog Comparator Bang Gap Select
ACD		equ	7		; Analog Comparator Disable
ACSRB		port	0x05		; Analog Comparator Control/Status Register B
ACIRS0		equ	0		; reserved for QTouch
ACIRS1		equ	1
ACME		equ	2		; Analog Comparator Multiplexer Enable
ACCE		equ	3		; reserved for QTouch
ACLP		equ	5		; reserved for QTouch
HLEV		equ	6		; Hysteresis Level
HSEL		equ	7		; Hysteresis Select

DIDR0		sfr	0x60		; Digital Input Disable Register 0
AIN0D		equ	1		; AIN0 Digital Input Disable
AIN1D		equ	2		; AIN1 Digital Input Disable

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x04		; ADC Multiplexer Selection Register
REFS1		equ	7		; Reference Selection Bits
REFS0		equ	6
REFEN		equ	5		; reserved for QTouch
ADC0EN		equ	4		; reserved for QTouch
MUX3		equ	3		; Analog Channel and Gain Selection Bits
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSRA		port	0x03		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x02		; Control/Status Register B
VDEN		equ	7		; reserved for QTouch
VDPD		equ	6		; reserved for QTouch
ADLAR		equ	3		; Left Adjust Result
ADTS2		equ	2		; Auto Trigger Source
ADTS1		equ	1
ADTS0		equ	0

ADCH		port	0x01		; Data Register
ADCL		port	0x00

		; bits in DIDR0
AREFD		equ	0		; 
ADC0D		equ	3		; ADC0 Digital Input Buffer Disable
ADC1D		equ	4		; ADC1 Digital Input Buffer Disable
ADC2D		equ	5		; ADC2 Digital Input Buffer Disable
ADC3D		equ	6		; ADC3/AIN10 Digital Input Buffer Disable
ADC4D		equ	7		; ADC4/AIN11 Digital Input Buffer Disable

DIDR1		sfr	0x61		; Digital Input Disable Register 1
ADC5D		equ	0		; ADC5 Digital Input Buffer Disable
ADC6D		equ	1		; ADC6 Digital Input Buffer Disable
ADC7D		equ	2		; ADC7 Digital Input Buffer Disable
ADC8D		equ	3		; ADC8 Digital Input Buffer Disable

DIDR2		sfr	0x62		; Digital Input Disable Register 2
ADC9D		equ	0		; ADC9 Digital Input Buffer Disable
ADC10D		equ	1		; ADC10 Digital Input Buffer Disable
ADC11D		equ	2		; ADC11 Digital Input Buffer Disable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x20		; USART0 I/O Data Register
UCSR0A		port	0x26		; USART0 Control & Status Register A
MPCM0		equ	0		; USART0 Multi Processor Communication Mode
U2X0		equ	1		; USART0 Double Transmission Speed
UPE0		equ	2		; USART0 Parity Error
DOR0		equ	3		; USART0 Overrun
FE0		equ	4		; USART0 Framing Error
UDRE0		equ	5		; USART0 Data Register Empty
TXC0		equ	6		; USART0 Transmit Complete
RXC0		equ	7		; USART0 Receive Complete
UCSR0B		port	0x25		; USART0 Control & Status Register B
TXB80		equ     0		; USART0 Transmit Bit 8
RXB80		equ     1		; USART0 Receive Bit 8
UCSZ02		equ     2		; USART0 Character Size
TXEN0		equ     3		; USART0 Enable Transmitter
RXEN0		equ     4		; USART0 Enable Receiver
UDRIE0		equ     5		; USART0 Enable Data Register Empty Interrupt
TXCIE0		equ     6		; USART0 Enable Transmit Complete Interrupt
RXCIE0		equ     7		; USART0 Enable Receive Complete Interrupt
UCSR0C		port	0x24		; USART0 Control & Status Register C
UCPOL0		equ	0		; USART0 clock polarity
UCSZ00		equ	1		; USART0 character size
UCSZ01		equ	2
USBS0		equ	3		; USART0 Stop Bit Select
UPM00		equ	4		; USART0 Parity Mode : Odd/Even
UPM01		equ	5		; USART0 Parity Mode : Enable/Disable
UMSEL00		equ	6		; USART0 Mode Select
UMSEL01		equ	7
UCSR0D		port	0x23		; USART0 Control & Status Register D
SFDE0		equ	5		; USART0 Start Frame Detection Enable
RXS0		equ	6		; USART0 RX Start
RXSIE0		equ	7		; USART0 RX Start Interrupt Enable
UBRR0L		port	0x21		; USART0 Baud Rate Register Low
UBRR0H		port	0x22		; USART0 Baud Rate Register High

UDR1		sfr	0x73		; USART1 I/O Data Register
UCSR1A		sfr	0x79		; USART1 Control & Status Register A
MPCM1		equ	0		; USART1 Multi Processor Communication Mode
U2X1		equ	1		; USART1 Double Transmission Speed
UPE1		equ	2		; USART1 Parity Error
DOR1		equ	3		; USART1 Overrun
FE1		equ	4		; USART1 Framing Error
UDRE1		equ	5		; USART1 Data Register Empty
TXC1		equ	6		; USART1 Transmit Complete
RXC1		equ	7		; USART1 Receive Complete
UCSR1B		sfr	0x78		; USART1 Control & Status Register B
TXB81		equ     0		; USART1 Transmit Bit 8
RXB81		equ     1		; USART1 Receive Bit 8
UCSZ12		equ     2		; USART1 Character Size
TXEN1		equ     3		; USART1 Enable Transmitter
RXEN1		equ     4		; USART1 Enable Receiver
UDRIE1		equ     5		; USART1 Enable Data Register Empty Interrupt
TXCIE1		equ     6		; USART1 Enable Transmit Complete Interrupt
RXCIE1		equ     7		; USART1 Enable Receive Complete Interrupt
UCSR1C		sfr	0x77		; USART1 Control & Status Register C
UCPOL1		equ	0		; USART1 clock polarity
UCSZ10		equ	1		; USART1 character size
UCSZ11		equ	2
USBS1		equ	3		; USART1 Stop Bit Select
UPM10		equ	4		; USART1 Parity Mode : Odd/Even
UPM11		equ	5		; USART1 Parity Mode : Enable/Disable
UMSEL10		equ	6		; USART1 Mode Select
UMSEL11		equ	7
UCSR1D		sfr	0x76		; USART1 Control & Status Register D
SFDE1		equ	5		; USART1 Start Frame Detection Enable
RXS1		equ	6		; USART1 RX Start
RXSIE1		equ	7		; USART1 RX Start Interrupt Enable
UBRR1L		sfr	0x74		; USART1 Baud Rate Register Low
UBRR1H		sfr	0x75		; USART1 Baud Rate Register High

;----------------------------------------------------------------------------
; TWI

TWSCRA		sfr	0x7f		; TWI Slave Control Register A
TWSME		equ	0		; TWI Smart Mode Enable
TWPME		equ	1		; TWI Promiscuous Mode Enable
TWSIE		equ	2		; TWI Stop Interrupt Enable
TWEN		equ	3		; TWI Enable
TWASIE		equ	4		; TWI Address/Stop Interrupt Enable
TWDIE		equ	5		; TWI Data Interrupt Enable
TWSHE		equ	7		; TWI SDA Hold Time Enable

TWSCRB		sfr	0x7e		; TWI Slave Control Register B
TWCMD0		equ	0		; TWI Command
TWCMD1		equ	1
TWAA		equ	2		; TWI Acknowledge Action

TWSSRA		sfr	0x7d		; TWI Slave Status Register A
TWAS		equ	0		; TWI Address or Stop
TWDIR		equ	1		; TWI Read/Write Direction
TWBE		equ	2		; TWI Bus Error
TWC		equ	3		; TWI Collision
TWRA		equ	4		; TWI Receive Acknowledge
TWCH		equ	5		; TWI Clock Hold
TWASIF		equ	6		; TWI Address/Stop Interrupt Flag
TWDIF		equ	7		; TWI Data Interrupt Flag

TWSA		sfr	0x7c		; TWI Slave Address Register

TWSAM		sfr	0x7b		; TWI Slave Address Mask Register

TWSD		sfr	0x7a		; TWI Slave Data Register

;----------------------------------------------------------------------------
; USI

USIBR		port	0x2d		; USI Buffer Register

USIDR		port	0x2c		; USI Data Register

USISR		port	0x2b		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		port	0x2a		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

		restore			; re-enable listing

		endif			; __regtn1634inc
