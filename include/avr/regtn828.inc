		ifndef	__regtn828inc
__regtn828inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN828.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny828                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ	255		; end address EEPROM
RAMSTART	equ	0x100,data	; start address SRAM
RAMEND		equ     0x2ff,data	; end address SRAM
FLASHEND	label   8191		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register

SMCR		port	0x33		; Sleep Mode Control Register
SE		equ	0		; Sleep Enable
SM0		equ	1		; Sleep Mode Select
SM1		equ	2

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL0		sfr	0x66		; Oscillator Calibration
OSSCAL1		sfr	0x67

OSCTCAL0A	sfr	0xf0		; Oscillator Temperature Compensation
OSCTCAL0B	sfr	0xf1

CLKPR		sfr	0x73		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3

CLKCR		sfr	0x61		; Clock Prescale Register
CKPS0		equ	0		; Clock Prescaler Select
CKPS1		equ	1
CKPS2		equ	2
CKPS3		equ	3

PRR		sfr	0x64		; Power Reduction Register
PRADC		equ	0		; Power Reduction AD Converter
PRUSART0	equ	1		; Power Reduction USART0
PRSPI		equ	2		; Power Reduction SPI
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRTIM0		equ	5		; Power Reduction Timer/Counter 0
PRTWI		equ	7		; Power Reduction Two Wire Interface

CCP		port	0x36		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x21		; EEPROM Address Register Low
EEDR		port	0x20		; EEPROM Data Register
EECR		port	0x1f		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
SPMIE		equ	7		; SPM Interrupt Enable
RWWSB		equ	6		; Read-While-Write Section Busy
RSIG		equ	5		; Read Device Signature Imprint Table
RWWSRE		equ	4		; Read-While-Write Section Read Enable
RWFLB		equ	3		; Read/Write Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SPMEN		equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x04		; Port B @ 0x04 (IO) ff.
PINC		port	0x08		; Port A @ 0x08 (IO) ff.
PIND		port	0x0c		; Port B @ 0x0c (IO) ff. (bits 0..3)

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PUEA		port	0x03		; Pull-up Enable Port A
PUEA0		equ	0
PUEA1		equ	1
PUEA2		equ	2
PUEA3		equ	3
PUEA4		equ	4
PUEA5		equ	5
PUEA6		equ	6
PUEA7		equ	7

PUEB		port	0x07		; Pull-up Enable Port B
PUEB0		equ	0
PUEB1		equ	1
PUEB2		equ	2
PUEB3		equ	3
PUEB4		equ	4
PUEB5		equ	5
PUEB6		equ	6
PUEB7		equ	7

PUEC		port	0x0b		; Pull-up Enable Port C
PUEC0		equ	0
PUEC1		equ	1
PUEC2		equ	2
PUEC3		equ	3
PUEC4		equ	4
PUEC5		equ	5
PUEC6		equ	6
PUEC7		equ	7

PUED		port	0x0f		; Pull-up Enable Port D
PUED0		equ	0
PUED1		equ	1
PUED2		equ	2
PUED3		equ	3

PCMSK0		sfr	0x6b		; Pin Change Interrupt Mask 0
PCMSK1		sfr	0x6c		; Pin Change Interrupt Mask 1
PCMSK2		sfr	0x6d		; Pin Change Interrupt Mask 2
PCMSK3		sfr	0x73		; Pin Change Interrupt Mask 3
PCINT24		equ	0		; Enable Pin Change interrupt 24
PCINT25		equ	1		; Enable Pin Change interrupt 25
PCINT26		equ	2		; Enable Pin Change interrupt 26
PCINT27		equ	3		; Enable Pin Change interrupt 27

PCICR		sfr	0x68		; Pin Change Interrupt Control Register

PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

PHDE		port	0x14		; Port High Drive Enable Register
PHDEC		equ	2		; Port C High Drive Enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum PCINT2_vect		; Pin Change Interrupt 2
		nextenum PCINT3_vect		; Pin Change Interrupt 3
		nextenum WDT_vect		; Watchdog Time-Out
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum USART_START_vect	; USART Start
		nextenum USART_RX_vect		; USART Rx Complete
		nextenum USART_UDRE_vect	; USART Data Register Empty
		nextenum USART_TX_vect		; USART Tx Complete
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum EE_RDY_vect		; EEPROM Ready
		nextenum ANALOG_COMP_vect	; Analog Comparator
		nextenum TWI_SLAVE_vect		; 2-wire Serial Interface
		nextenum SPM_Ready_vect		; Store Program Memory Read
		nextenum QTRIP_vect		; Touch Sensing

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
IVSEL		equ	1		; Interrupt Vector Select

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

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x25		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare Match B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare Match A
TCNT0		port	0x26		; Timer/Counter 0 Value
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

TCCR1A		sfr	0x80		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	4		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	5
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
OCIE0B		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		equ	0		; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		equ	1		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	2		; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		equ	5		; Timer/Counter 1 Input Capture Interrupt Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR		equ	0		; Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

TOCPMSA0	sfr	0xe8		; Timer/Counter Output Compare Pin Mux Selection Register 0
TOCC0S0		equ	0		; Timer/Counter Output Compare Channel Select 0
TOCC0S1		equ	1
TOCC1S0		equ	2		; Timer/Counter Output Compare Channel Select 1
TOCC1S1		equ	3
TOCC2S0		equ	4		; Timer/Counter Output Compare Channel Select 2
TOCC2S1		equ	5
TOCC3S0		equ	6		; Timer/Counter Output Compare Channel Select 3
TOCC3S1		equ	7 

TOCPMSA1	sfr	0xe9		; Timer/Counter Output Compare Pin Mux Selection Register 1
TOCC4S0		equ	0		; Timer/Counter Output Compare Channel Select 4
TOCC4S1		equ	1
TOCC5S0		equ	2		; Timer/Counter Output Compare Channel Select 5
TOCC5S1		equ	3
TOCC6S0		equ	4		; Timer/Counter Output Compare Channel Select 6
TOCC6S1		equ	5
TOCC7S0		equ	6		; Timer/Counter Output Compare Channel Select 7
TOCC7S1		equ	7 

TOCPMCOE	sfr	0xe2		; Timer/Counter Output Compare Pin Mux Channel Output Enable
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

WDTCSR		sfr	0x60		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDP3		equ	5
WDIE		equ	6		; interrupt enable
WDIF		equ	7		; interrupt flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x30		; Analog Comparator Control/Status Register A
ACIS00		equ     0               ; Analog Comparator Interrupt-Mode
ACIS01		equ     1
ACIC	 	equ	2		; Analog Comparator Use As Capture Signal For Timer 1?
ACIE		equ     3               ; Analog Comparator Interrupt Enable
ACI		equ     4               ; Analog Comparator Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACPMUX2		equ	6		; Analog Comparator Positive Input Multiplexer
ACD		equ	7		; Analog Comparator Disable
ACSRB		port	0x2f		; Analog Comparator Control/Status Register B
ACPMUX0		equ	0		; Analog Comparator Positive Input Multiplexer
ACPMUX1		equ	1
ACNMUX0		equ	2		; Analog Comparator Negative Input Multiplexer
ACNMUX1		equ	3
ACLP		equ	5		; ???
HLEV		equ	6		; Analog Comparator Hysteresis Level
HSEL		equ	7		; Analog Comparator Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX0		sfr	0x7c		; ADC Multiplexer Selection Register A
MUX4		equ	4		; Analog Channel and Gain Selection Bits
MUX3		equ	3
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADMUX1		sfr	0x7d		; ADC Multiplexer Selection Register B
REFS		equ	5		; Reference Selection
MUX5		equ	0

ADCSRA		sfr	0x7a		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		sfr	0x7b		; Control/Status Register B
ADLAR		equ	3		; Left Adjust Result
ADTS2		equ	2		; Auto Trigger Source
ADTS1		equ	1
ADTS0		equ	0

ADCH		sfr	0x79		; Data Register
ADCL		sfr	0x78

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Buffer Disable
ADC1D		equ	1		; ADC1 Digital Input Buffer Disable
ADC2D		equ	2		; ADC2 Digital Input Buffer Disable
ADC3D		equ	3		; ADC3/AIN10 Digital Input Buffer Disable
ADC4D		equ	4		; ADC4/AIN11 Digital Input Buffer Disable
ADC5D		equ	5		; ADC5 Digital Input Buffer Disable
ADC6D		equ	6		; ADC6 Digital Input Buffer Disable
ADC7D		equ	7		; ADC7 Digital Input Buffer Disable

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
ADC8D		equ	0		; ADC8 Digital Input Buffer Disable
ADC9D		equ	1		; ADC9 Digital Input Buffer Disable
ADC10D		equ	2		; ADC10 Digital Input Buffer Disable
ADC11D		equ	3		; ADC11 Digital Input Buffer Disable
ADC12D		equ	4		; ADC12 Digital Input Buffer Disable
ADC13D		equ	5		; ADC13 Digital Input Buffer Disable
ADC14D		equ	6		; ADC14 Digital Input Buffer Disable
ADC15D		equ	7		; ADC15 Digital Input Buffer Disable

DIDR2		sfr	0xde		; Digital Input Disable Register 2
ADC16D		equ	0		; ADC16 Digital Input Buffer Disable
ADC17D		equ	1		; ADC17 Digital Input Buffer Disable
ADC18D		equ	2		; ADC18 Digital Input Buffer Disable
ADC19D		equ	3		; ADC19 Digital Input Buffer Disable
ADC20D		equ	4		; ADC20 Digital Input Buffer Disable
ADC21D		equ	5		; ADC21 Digital Input Buffer Disable
ADC22D		equ	6		; ADC22 Digital Input Buffer Disable
ADC23D		equ	7		; ADC23 Digital Input Buffer Disable

DIDR3		sfr	0xdf		; Digital Input Disable Register 3
ADC24D		equ	0		; ADC24 Digital Input Buffer Disable
ADC25D		equ	1		; ADC25 Digital Input Buffer Disable
ADC26D		equ	2		; ADC26 Digital Input Buffer Disable
ADC27D		equ	3		; ADC27 Digital Input Buffer Disable

;----------------------------------------------------------------------------
; USART

UDR		sfr	0xc6		; USART I/O Data Register
UCSR0A		sfr	0xc0		; USART Control & Status Register A
MPCM		equ	0		; USART Multi Processor Communication Mode
U2X		equ	1		; USART Double Transmission Speed
UPE		equ	2		; USART Parity Error
DOR		equ	3		; USART Overrun
FE		equ	4		; USART Framing Error
UDRE		equ	5		; USART Data Register Empty
TXC		equ	6		; USART Transmit Complete
RXC		equ	7		; USART Receive Complete
UCSR0B		sfr	0xc1		; USART Control & Status Register B
TXB8		equ     0		; USART Transmit Bit 8
RXB8		equ     1		; USART Receive Bit 8
UCSZ2		equ     2		; USART Character Size
TXEN		equ     3		; USART Enable Transmitter
RXEN		equ     4		; USART Enable Receiver
UDRIE		equ     5		; USART Enable Data Register Empty Interrupt
TXCIE		equ     6		; USART Enable Transmit Complete Interrupt
RXCIE		equ     7		; USART Enable Receive Complete Interrupt
UCSR0C		sfr	0xc2		; USART Control & Status Register C
UCPOL		equ	0		; USART clock polarity
UCSZ0		equ	1		; USART character size
UCSZ1		equ	2
USBS		equ	3		; USART Stop Bit Select
UPM0		equ	4		; USART Parity Mode : Odd/Even
UPM1		equ	5		; USART Parity Mode : Enable/Disable
UMSEL0		equ	6		; USART Mode Select
UMSEL1		equ	7
UCSR0D		sfr	0xc3		; USART Control & Status Register D
SFDE		equ	5		; USART Start Frame Detection Enable
RXS		equ	6		; USART RX Start
RXSIE		equ	7		; USART RX Start Interrupt Enable
UBRRL		sfr	0xc4		; USART Baud Rate Register Low
UBRRH		sfr	0xc5		; USART Baud Rate Register High

;----------------------------------------------------------------------------
; TWI

TWSCRA		sfr	0xb8		; TWI Slave Control Register A
TWSME		equ	0		; TWI Smart Mode Enable
TWPME		equ	1		; TWI Promiscuous Mode Enable
TWSIE		equ	2		; TWI Stop Interrupt Enable
TWEN		equ	3		; TWI Enable
TWASIE		equ	4		; TWI Address/Stop Interrupt Enable
TWDIE		equ	5		; TWI Data Interrupt Enable
TWSHE		equ	7		; TWI SDA Hold Time Enable

TWSCRB		sfr	0xb9		; TWI Slave Control Register B
TWCMD0		equ	0		; TWI Command
TWCMD1		equ	1
TWAA		equ	2		; TWI Acknowledge Action

TWSSRA		sfr	0xba		; TWI Slave Status Register A
TWAS		equ	0		; TWI Address or Stop
TWDIR		equ	1		; TWI Read/Write Direction
TWBE		equ	2		; TWI Bus Error
TWC		equ	3		; TWI Collision
TWRA		equ	4		; TWI Receive Acknowledge
TWCH		equ	5		; TWI Clock Hold
TWASIF		equ	6		; TWI Address/Stop Interrupt Flag
TWDIF		equ	7		; TWI Data Interrupt Flag

TWSA		sfr	0xbc		; TWI Slave Address Register

TWSAM		sfr	0xbb		; TWI Slave Address Mask Register
TWAE		equ	0		; TWI Address Enable

TWSD		sfr	0xbd		; TWI Slave Data Register

;----------------------------------------------------------------------------
; SPI

SPCR		port	0x2c		; SPI Control Register
SPR0		equ	0		; Clock Select
SPR1		equ	1
CPHA		equ	2		; Clock Phase
CPOL		equ	3		; Clock Polarity
MSTR		equ	4		; Master/Slave Selection
DORD		equ	5		; Bit Order
SPE		equ	6		; Enable SPI
SPIE		equ	7		; SPI Interrupt Enable

SPSR		port	0x2d		; SPI Status Register
SPI2X		equ	0		; Double Speed Mode
WCOL		equ	6		; Write Collision
SPIF		equ	7		; SPI Interrupt Occured?

SPDR		port	0x2e		; SPI data register

		restore

		endif			; __regtn828inc
