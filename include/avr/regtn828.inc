		ifndef	__regtn828inc
__regtn828inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN828.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny828                      *
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
SE		avrbit	SMCR,0		; Sleep Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL0		sfr	0x66		; Oscillator Calibration
OSSCAL1		sfr	0x67

OSCTCAL0A	sfr	0xf0		; Oscillator Temperature Compensation
OSCTCAL0B	sfr	0xf1

CLKPR		sfr	0x73		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3

CLKCR		sfr	0x61		; Clock Prescale Register
CKPS0		avrbit	CLKCR,0		; Clock Prescaler Select
CKPS1		avrbit	CLKCR,1
CKPS2		avrbit	CLKCR,2
CKPS3		avrbit	CLKCR,3

PRR		sfr	0x64		; Power Reduction Register
PRADC		avrbit	PRR,0		; Power Reduction AD Converter
PRUSART0	avrbit	PRR,1		; Power Reduction USART0
PRSPI		avrbit	PRR,2		; Power Reduction SPI
PRTIM1		avrbit	PRR,3		; Power Reduction Timer/Counter 1
PRTIM0		avrbit	PRR,5		; Power Reduction Timer/Counter 0
PRTWI		avrbit	PRR,7		; Power Reduction Two Wire Interface

CCP		port	0x36		; Configuration Change Protection Register

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEARL		port	0x21		; EEPROM Address Register Low
EEDR		port	0x20		; EEPROM Data Register
EECR		port	0x1f		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
SPMIE		avrbit	SPMCSR,7	; SPM Interrupt Enable
RWWSB		avrbit	SPMCSR,6	; Read-While-Write Section Busy
RSIG		avrbit	SPMCSR,5	; Read Device Signature Imprint Table
RWWSRE		avrbit	SPMCSR,4	; Read-While-Write Section Read Enable
RWFLB		avrbit	SPMCSR,3	; Read/Write Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; JTAG etc.

DWDR		port	0x31		; debugWire Data Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x04		; Port B @ 0x04 (IO) ff.
PINC		port	0x08		; Port A @ 0x08 (IO) ff.
PIND		port	0x0c		; Port B @ 0x0c (IO) ff.
__PORTD_BITS	equ	0x0f		; (bits 0..3)

GPIOR0		port	0x1e		; General Purpose I/O Register 0
GPIOR1		port	0x2a		; General Purpose I/O Register 1
GPIOR2		port	0x2b		; General Purpose I/O Register 2

PUEA		port	0x03		; Pull-up Enable Port A
PUEA0		avrbit	PUEA,0
PUEA1		avrbit	PUEA,1
PUEA2		avrbit	PUEA,2
PUEA3		avrbit	PUEA,3
PUEA4		avrbit	PUEA,4
PUEA5		avrbit	PUEA,5
PUEA6		avrbit	PUEA,6
PUEA7		avrbit	PUEA,7

PUEB		port	0x07		; Pull-up Enable Port B
PUEB0		avrbit	PUEB,0
PUEB1		avrbit	PUEB,1
PUEB2		avrbit	PUEB,2
PUEB3		avrbit	PUEB,3
PUEB4		avrbit	PUEB,4
PUEB5		avrbit	PUEB,5
PUEB6		avrbit	PUEB,6
PUEB7		avrbit	PUEB,7

PUEC		port	0x0b		; Pull-up Enable Port C
PUEC0		avrbit	PUEC,0
PUEC1		avrbit	PUEC,1
PUEC2		avrbit	PUEC,2
PUEC3		avrbit	PUEC,3
PUEC4		avrbit	PUEC,4
PUEC5		avrbit	PUEC,5
PUEC6		avrbit	PUEC,6
PUEC7		avrbit	PUEC,7

PUED		port	0x0f		; Pull-up Enable Port D
PUED0		avrbit	PUED,0
PUED1		avrbit	PUED,1
PUED2		avrbit	PUED,2
PUED3		avrbit	PUED,3

PCMSK0		sfr	0x6b		; Pin Change Interrupt Mask 0
PCMSK1		sfr	0x6c		; Pin Change Interrupt Mask 1
PCMSK2		sfr	0x6d		; Pin Change Interrupt Mask 2
PCMSK3		sfr	0x73		; Pin Change Interrupt Mask 3
PCINT24		avrbit	PCMSK3,0	; Enable Pin Change Interrupt 24
PCINT25		avrbit	PCMSK3,1	; Enable Pin Change Interrupt 25
PCINT26		avrbit	PCMSK3,2	; Enable Pin Change Interrupt 26
PCINT27		avrbit	PCMSK3,3	; Enable Pin Change Interrupt 27

PCICR		sfr	0x68		; Pin Change Interrupt Control Register

PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

PHDE		port	0x14		; Port High Drive Enable Register
PHDEC		avrbit	PHDE,2		; Port C High Drive Enable

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

IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select

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

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 1 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 1 Output Compare Mode A
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
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,5
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
OCIE0B		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable B
OCIE0A		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable A

TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1B		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK1,2	; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Interrupt Enable

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Flag Register

TIFR1		port	0x16		; Timer/Counter 1 Interrupt Flag Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR		avrbit	GTCCR,0		; Prescaler Reset
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

TOCPMSA0	sfr	0xe8		; Timer/Counter Output Compare Pin Mux Selection Register 0
TOCC0S0		avrbit	TOCPMSA0,0	; Timer/Counter Output Compare Channel Select 0
TOCC0S1		avrbit	TOCPMSA0,1
TOCC1S0		avrbit	TOCPMSA0,2	; Timer/Counter Output Compare Channel Select 1
TOCC1S1		avrbit	TOCPMSA0,3
TOCC2S0		avrbit	TOCPMSA0,4	; Timer/Counter Output Compare Channel Select 2
TOCC2S1		avrbit	TOCPMSA0,5
TOCC3S0		avrbit	TOCPMSA0,6	; Timer/Counter Output Compare Channel Select 3
TOCC3S1		avrbit	TOCPMSA0,7 

TOCPMSA1	sfr	0xe9		; Timer/Counter Output Compare Pin Mux Selection Register 1
TOCC4S0		avrbit	TOCPMSA1,0	; Timer/Counter Output Compare Channel Select 4
TOCC4S1		avrbit	TOCPMSA1,1
TOCC5S0		avrbit	TOCPMSA1,2	; Timer/Counter Output Compare Channel Select 5
TOCC5S1		avrbit	TOCPMSA1,3
TOCC6S0		avrbit	TOCPMSA1,4	; Timer/Counter Output Compare Channel Select 6
TOCC6S1		avrbit	TOCPMSA1,5
TOCC7S0		avrbit	TOCPMSA1,6	; Timer/Counter Output Compare Channel Select 7
TOCC7S1		avrbit	TOCPMSA1,7 

TOCPMCOE	sfr	0xe2		; Timer/Counter Output Compare Pin Mux Channel Output Enable
TOCC0OE		avrbit	TOCPMCOE,0	; Timer/Counter Output Compare Channel 0 Output Enable
TOCC01E		avrbit	TOCPMCOE,1	; Timer/Counter Output Compare Channel 1 Output Enable
TOCC02E		avrbit	TOCPMCOE,2	; Timer/Counter Output Compare Channel 2 Output Enable
TOCC03E		avrbit	TOCPMCOE,3	; Timer/Counter Output Compare Channel 3 Output Enable
TOCC04E		avrbit	TOCPMCOE,4	; Timer/Counter Output Compare Channel 4 Output Enable
TOCC05E		avrbit	TOCPMCOE,5	; Timer/Counter Output Compare Channel 5 Output Enable
TOCC06E		avrbit	TOCPMCOE,6	; Timer/Counter Output Compare Channel 6 Output Enable
TOCC07E		avrbit	TOCPMCOE,7	; Timer/Counter Output Compare Channel 7 Output Enable

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		sfr	0x60		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable watchdog
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Interrupt Enable
WDIF		avrbit	WDTCSR,7	; Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port	0x30		; Analog Comparator Control/Status Register A
ACIS00		avrbit	ACSRA,0		; Analog Comparator Interrupt-Mode
ACIS01		avrbit	ACSRA,1
ACIC	 	avrbit	ACSRA,2		; Analog Comparator Use As Capture Signal For Timer 1?
ACIE		avrbit	ACSRA,3		; Analog Comparator Interrupt Enable
ACI		avrbit	ACSRA,4		; Analog Comparator Interrupt Flag
ACO		avrbit	ACSRA,5		; Analog Comparator Output
ACPMUX2		avrbit	ACSRA,6		; Analog Comparator Positive Input Multiplexer
ACD		avrbit	ACSRA,7		; Analog Comparator Disable
ACSRB		port	0x2f		; Analog Comparator Control/Status Register B
ACPMUX0		avrbit	ACSRB,0		; Analog Comparator Positive Input Multiplexer
ACPMUX1		avrbit	ACSRB,1
ACNMUX0		avrbit	ACSRB,2		; Analog Comparator Negative Input Multiplexer
ACNMUX1		avrbit	ACSRB,3
ACLP		avrbit	ACSRB,5		; ???
HLEV		avrbit	ACSRB,6		; Analog Comparator Hysteresis Level
HSEL		avrbit	ACSRB,7		; Analog Comparator Hysteresis Select

;----------------------------------------------------------------------------
; A/D Converter

ADMUX0		sfr	0x7c		; ADC Multiplexer Selection Register A
MUX4		avrbit	ADMUX0,4	; Analog Channel and Gain Selection Bits
MUX3		avrbit	ADMUX0,3
MUX2		avrbit	ADMUX0,2
MUX1		avrbit	ADMUX0,1
MUX0		avrbit	ADMUX0,0

ADMUX1		sfr	0x7d		; ADC Multiplexer Selection Register B
REFS		avrbit	ADMUX1,5	; Reference Selection
MUX5		avrbit	ADMUX1,0

ADCSRA		sfr	0x7a		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		sfr	0x7b		; Control/Status Register B
ADLAR		avrbit	ADCSRB,3	; Left Adjust Result
ADTS2		avrbit	ADCSRB,2	; Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS0		avrbit	ADCSRB,0

ADCH		sfr	0x79		; Data Register
ADCL		sfr	0x78

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
ADC0D		avrbit	DIDR0,0		; ADC0 Digital Input Buffer Disable
ADC1D		avrbit	DIDR0,1		; ADC1 Digital Input Buffer Disable
ADC2D		avrbit	DIDR0,2		; ADC2 Digital Input Buffer Disable
ADC3D		avrbit	DIDR0,3		; ADC3/AIN10 Digital Input Buffer Disable
ADC4D		avrbit	DIDR0,4		; ADC4/AIN11 Digital Input Buffer Disable
ADC5D		avrbit	DIDR0,5		; ADC5 Digital Input Buffer Disable
ADC6D		avrbit	DIDR0,6		; ADC6 Digital Input Buffer Disable
ADC7D		avrbit	DIDR0,7		; ADC7 Digital Input Buffer Disable

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
ADC8D		avrbit	DIDR1,0		; ADC8 Digital Input Buffer Disable
ADC9D		avrbit	DIDR1,1		; ADC9 Digital Input Buffer Disable
ADC10D		avrbit	DIDR1,2		; ADC10 Digital Input Buffer Disable
ADC11D		avrbit	DIDR1,3		; ADC11 Digital Input Buffer Disable
ADC12D		avrbit	DIDR1,4		; ADC12 Digital Input Buffer Disable
ADC13D		avrbit	DIDR1,5		; ADC13 Digital Input Buffer Disable
ADC14D		avrbit	DIDR1,6		; ADC14 Digital Input Buffer Disable
ADC15D		avrbit	DIDR1,7		; ADC15 Digital Input Buffer Disable

DIDR2		sfr	0xde		; Digital Input Disable Register 2
ADC16D		avrbit	DIDR2,0		; ADC16 Digital Input Buffer Disable
ADC17D		avrbit	DIDR2,1		; ADC17 Digital Input Buffer Disable
ADC18D		avrbit	DIDR2,2		; ADC18 Digital Input Buffer Disable
ADC19D		avrbit	DIDR2,3		; ADC19 Digital Input Buffer Disable
ADC20D		avrbit	DIDR2,4		; ADC20 Digital Input Buffer Disable
ADC21D		avrbit	DIDR2,5		; ADC21 Digital Input Buffer Disable
ADC22D		avrbit	DIDR2,6		; ADC22 Digital Input Buffer Disable
ADC23D		avrbit	DIDR2,7		; ADC23 Digital Input Buffer Disable

DIDR3		sfr	0xdf		; Digital Input Disable Register 3
ADC24D		avrbit	DIDR3,0		; ADC24 Digital Input Buffer Disable
ADC25D		avrbit	DIDR3,1		; ADC25 Digital Input Buffer Disable
ADC26D		avrbit	DIDR3,2		; ADC26 Digital Input Buffer Disable
ADC27D		avrbit	DIDR3,3		; ADC27 Digital Input Buffer Disable

;----------------------------------------------------------------------------
; USART

UDR		sfr	0xc6		; USART I/O Data Register
UCSR0A		sfr	0xc0		; USART Control & Status Register A
MPCM		avrbit	UCSR0A,0	; USART Multi Processor Communication Mode
U2X		avrbit	UCSR0A,1	; USART Double Transmission Speed
UPE		avrbit	UCSR0A,2	; USART Parity Error
DOR		avrbit	UCSR0A,3	; USART Overrun
FE		avrbit	UCSR0A,4	; USART Framing Error
UDRE		avrbit	UCSR0A,5	; USART Data Register Empty
TXC		avrbit	UCSR0A,6	; USART Transmit Complete
RXC		avrbit	UCSR0A,7	; USART Receive Complete
UCSR0B		sfr	0xc1		; USART Control & Status Register B
TXB8		avrbit	UCSR0B,0	; USART Transmit Bit 8
RXB8		avrbit	UCSR0B,1	; USART Receive Bit 8
UCSZ2		avrbit	UCSR0B,2	; USART Character Size
TXEN		avrbit	UCSR0B,3	; USART Enable Transmitter
RXEN		avrbit	UCSR0B,4	; USART Enable Receiver
UDRIE		avrbit	UCSR0B,5	; USART Enable Data Register Empty Interrupt
TXCIE		avrbit	UCSR0B,6	; USART Enable Transmit Complete Interrupt
RXCIE		avrbit	UCSR0B,7	; USART Enable Receive Complete Interrupt
UCSR0C		sfr	0xc2		; USART Control & Status Register C
UCPOL		avrbit	UCSR0C,0	; USART Clock polarity
UCSZ0		avrbit	UCSR0C,1	; USART character size
UCSZ1		avrbit	UCSR0C,2
USBS		avrbit	UCSR0C,3	; USART Stop Bit Select
UPM0		avrbit	UCSR0C,4	; USART Parity Mode : Odd/Even
UPM1		avrbit	UCSR0C,5	; USART Parity Mode : Enable/Disable
UMSEL0		avrbit	UCSR0C,6	; USART Mode Select
UMSEL1		avrbit	UCSR0C,7
UCSR0D		sfr	0xc3		; USART Control & Status Register D
SFDE		avrbit	UCSR0D,5	; USART Start Frame Detection Enable
RXS		avrbit	UCSR0D,6	; USART RX Start
RXSIE		avrbit	UCSR0D,7	; USART RX Start Interrupt Enable
UBRRL		sfr	0xc4		; USART Baud Rate Register Low
UBRRH		sfr	0xc5		; USART Baud Rate Register High

;----------------------------------------------------------------------------
; TWI

TWSCRA		sfr	0xb8		; TWI Slave Control Register A
TWSME		avrbit	TWSCRA,0	; TWI Smart Mode Enable
TWPME		avrbit	TWSCRA,1	; TWI Promiscuous Mode Enable
TWSIE		avrbit	TWSCRA,2	; TWI Stop Interrupt Enable
TWEN		avrbit	TWSCRA,3	; TWI Enable
TWASIE		avrbit	TWSCRA,4	; TWI Address/Stop Interrupt Enable
TWDIE		avrbit	TWSCRA,5	; TWI Data Interrupt Enable
TWSHE		avrbit	TWSCRA,7	; TWI SDA Hold Time Enable

TWSCRB		sfr	0xb9		; TWI Slave Control Register B
TWCMD0		avrbit	TWSCRB,0	; TWI Command
TWCMD1		avrbit	TWSCRB,1
TWAA		avrbit	TWSCRB,2	; TWI Acknowledge Action

TWSSRA		sfr	0xba		; TWI Slave Status Register A
TWAS		avrbit	TWSSRA,0	; TWI Address or Stop
TWDIR		avrbit	TWSSRA,1	; TWI Read/Write Direction
TWBE		avrbit	TWSSRA,2	; TWI Bus Error
TWC		avrbit	TWSSRA,3	; TWI Collision
TWRA		avrbit	TWSSRA,4	; TWI Receive Acknowledge
TWCH		avrbit	TWSSRA,5	; TWI Clock Hold
TWASIF		avrbit	TWSSRA,6	; TWI Address/Stop Interrupt Flag
TWDIF		avrbit	TWSSRA,7	; TWI Data Interrupt Flag

TWSA		sfr	0xbc		; TWI Slave Address Register

TWSAM		sfr	0xbb		; TWI Slave Address Mask Register
TWAE		avrbit	TWSAM,0		; TWI Address Enable

TWSD		sfr	0xbd		; TWI Slave Data Register

;----------------------------------------------------------------------------
; SPI

SPCR		port	0x2c		; SPI Control Register
SPR0		avrbit	SPCR,0		; Clock Select
SPR1		avrbit	SPCR,1
CPHA		avrbit	SPCR,2		; Clock Phase
CPOL		avrbit	SPCR,3		; Clock Polarity
MSTR		avrbit	SPCR,4		; Master/Slave Selection
DORD		avrbit	SPCR,5		; Bit Order
SPE		avrbit	SPCR,6		; Enable SPI
SPIE		avrbit	SPCR,7		; SPI Interrupt Enable

SPSR		port	0x2d		; SPI Status Register
SPI2X		avrbit	SPSR,0		; Double Speed Mode
WCOL		avrbit	SPSR,6		; Write Collision
SPIF		avrbit	SPSR,7		; SPI Interrupt Occured?

SPDR		port	0x2e		; SPI Data Register

		restore

		endif			; __regtn828inc
