		ifndef	__regt1024inc
__regt1024inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT1024.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny102/104                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

VLMCSR		port	0x34		; VCC Level Monitoring Control and Status register
VLM0		equ	0		; Trigger Level of Voltage Level Monitor
VLM1		equ	1
VLM2		equ	2
VLMIE		equ	6		; VLM Interrupt Enable
VLMF		equ	7		; VLM Flag

RSTFLR		port	0x3b		; Reset Flag Register
WDRF		equ	3		; Watchdog Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-on Reset Flag

OSCCAL		port	0x39		; Oscillator Calibration

CLKPSR		port	0x36		; Clock Prescaler Register
CLKPS0		equ	0		; Clock Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2

CLKMSR		port	0x37		; Clock Main Settings Register
CLKMS0		equ	0		; Clock Main Select Bits
CLKMS1		equ	1

SMCR		port	0x3a		; Sleep Mode Control Register
SE		equ	0		; Sleep Mode Enable
SM0		equ	1		; Sleep Mode Select
SM1		equ	2
SM2		equ	3

PRR		port	0x35		; Power Reduction Register
PRTIM0		equ	0		; Power Reduction Timer/Counter 0
PRADC		equ	1		; Power Reduction A/D Converter
PRUSART0	equ	2		; Power Reduction USART

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

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff. (bits 0..2 on ATmega102)
PUEA		port	0x03		; Pull-Up Enable Port A
PUEA0		equ	0
PUEA1		equ	1
PUEA2		equ	2
		if	MOMCPUNAME="ATTINY104"
PUEA3		 equ	3
PUEA4		 equ	4
PUEA5		 equ	5
PUEA6		 equ	6
PUEA7		 equ	7
		endif
PINB		port	0x04		; Port B @ 0x04 (IO) ff. (bits 1..3 on ATmega102, bits 0..3 on ATmega104)
PUEB		port	0x07		; Pull-Up Enable Port B
		if	MOMCPUNAME="ATTINY104"
PUEB0		 equ	0
PUEB1		 equ	1
PUEB2		 equ	2
		endif
PUEB3		equ	3

PCMSK0		port	0x0f		; Pin-Change Mask Register 0
PCINT0		equ	0		; Enable Pin-Change Interrupt 0
PCINT1		equ	1		; Enable Pin-Change Interrupt 1
PCINT2		equ	2		; Enable Pin-Change Interrupt 2
PCINT3		equ	3		; Enable Pin-Change Interrupt 3
PCINT4		equ	4		; Enable Pin-Change Interrupt 4
PCINT5		equ	5		; Enable Pin-Change Interrupt 5
PCINT6		equ	6		; Enable Pin-Change Interrupt 6
PCINT7		equ	7		; Enable Pin-Change Interrupt 7
PCMSK1		port	0x10		; Pin-Change Mask Register 1
PCINT8		equ	0		; Enable Pin-Change Interrupt 8
PCINT9		equ	1		; Enable Pin-Change Interrupt 9
PCINT10		equ	2		; Enable Pin-Change Interrupt 10
PCINT11		equ	3		; Enable Pin-Change Interrupt 11

PORTCR		port	0x16		; Port Control Register
BBMA		equ	0		; Break-Before-Make Mode Enable Port A
BBMB		equ	1		; Break-Before-Make Mode Enable Port B

PCICR		port	0x12		; Pin-Change Interrupt Control Register

PCIFR		port	0x11		; Pin-Change Interrupt Flag Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
		nextenum PCINT0_vect		; Pin Change Interrupt 0
		nextenum PCINT1_vect		; Pin Change Interrupt 1
		nextenum TIM0_CAPT_vect		; Timer/Counter 0 Capture
		nextenum TIM0_OVF_vect		; Timer/Counter 0 Overflow
		nextenum TIM0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIM0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum WDT_vect		; Watchdog Time-out Interrupt
		nextenum VLM_vect		; Vcc Voltage Level Monitor
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum USART0_RXS_vect	; USART0 Rx Start
		nextenum USART0_RXC_vect	; USART0 Rx Complete
		nextenum USART0_DRE_vect	; USART0 Data Register Empty
		nextenum USART0_TXC_vect	; USART0 Tx Complete

;----------------------------------------------------------------------------
; External Interrupts

EICRA		port	0x15		; External Interrupt Control Register A
ISC00		equ	0		; Interrupt Sense Control 0
ISC01		equ	1

EIMSK		port	0x13		; External Interrupt Mask Register
INT0		equ	0		; Enable External Interrupt 0

EIFR		port	0x14		; External Interrupt Flag Register
INTF0		equ	0		; External Interrupt 0 occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2e		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Tounter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 0 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x2d		; Timer/Counter 0 Control Register B
CS00            equ     0               ; Timer/Counter 0 Clock Select
CS01            equ     1
CS02            equ     2
WGM02		equ	3
WGM03		equ	4
ICES0		equ	6		; Timer/Counter 0 Input Capture Edge Select
ICNC0		equ	7		; Timer/Counter 0 Input Capture Noise Canceling
TCCR0C		port	0x2c		; Timer/Counter 0 Control Register C
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare Match B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare Match A
TCNT0L		port	0x28		; Timer/Counter 0 Value LSB
TCNT0H		port	0x29		; Timer/Counter 0 Value MSB
OCR0AL		port	0x26		; Timer/Counter 0 Output Compare Value A LSB
OCR0AH		port	0x27		; Timer/Counter 0 Output Compare Value A MSB
OCR0BL		port	0x24		; Timer/Counter 0 Output Compare Value B LSB
OCR0BH		port	0x25		; Timer/Counter 0 Output Compare Value B MSB
ICR0L		port	0x22		; Timer/Counter 0 Input Capture Register LSB
ICR0H		port	0x23		; Timer/Counter 0 Input Capture Register MSB

TIMSK0		port	0x2b		; Timer/Counter 0 Interrupt Mask Register
TOIE0		equ	0		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		equ	1		; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE0		equ	5		; Timer/Counter 0 Input Capture Interrupt Enable

TIFR0		port	0x2a		; Timer/Counter 0 Interrupt Status Register

GTCCR		port	0x2f		; General Timer/Counter Control Register
PSR		equ	0		; Prescaler 0 Reset Timer/Counter 0
REMAP		equ	1		; Timer Pin Mapping
TSM		equ	7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x31		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable Watchdog
WDP3		equ	5
WDIE		equ	6		; Watchdog Interrupt Enable
WDIF		equ	7		; Watchdog Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port    0x1f		; Analog Comparator Control and Status Register A
ACIS0		equ     0		; Interrupt-Mode
ACIS1		equ     1
ACIC		equ	2		; Use Comparator As Capture Signal For Timer 0?
ACIE		equ     3               ; Interrupt Enable
ACI		equ     4               ; Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACBG		equ	6		; Analog Comparator Band Gap Select
ACD		equ	7		; Disable

ACSRB		port	0x1e		; Analog Comparator Control and Status Register B
ACOE		equ	1		; Analog Comparator Output Enable
ACPMUX		equ	0		; Analog Comparator Positive Input Multiplexer

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x1b		; ADC Multiplexer Selection Register
MUX0		equ	0		; Analog Channel Selection
MUX1		equ	1
MUX2		equ	2
REFS0		equ	6		; Reference Selection
REFS1		equ	7

ADCSRA		port	0x1d		; ADC Control/Status Register A
ADEN		equ	7		; Enable ADC
ADSC		equ	6		; Start Conversion
ADATE		equ	5		; ADC Auto Trigger Enable
ADIF		equ	4		; Interrupt Flag
ADIE		equ	3		; Interrupt Enable
ADPS2		equ	2		; Prescaler Select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		port	0x1c		; ADC Control/Status Register A
ADTS0		equ	0		; ADC Auto Trigger Source
ADTS1		equ	1
ADTS2		equ	2

ADCL		port	0x19		; ADC Conversion Result LSB
ADCH		port	0x1a		; ADC Conversion Result MSB

DIDR0		port	0x17		; Digital Input Disable Register 0
ADC0D		equ	0		; ADC0 Digital Input Disable
ADC1D		equ	1		; ADC1 Digital Input Disable
ADC2D		equ	2		; ADC2 Digital Input Disable
ADC3D		equ	3		; ADC3 Digital Input Disable
ADC4D		equ	4		; ADC4 Digital Input Disable
ADC5D		equ	5		; ADC5 Digital Input Disable
ADC6D		equ	6		; ADC6 Digital Input Disable
ADC7D		equ	7		; ADC7 Digital Input Disable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x08		; USART I/O Data Register 0

UCSR0A		port	0x0e		; USART Control and Status Register 0 A
RXC0		equ	7		; USART Receive Complete
TXC0		equ	6		; USART Transmit Complete
UDRE0		equ	5		; USART Data Register Empty
FE0		equ	4		; Frame Error
DOR0		equ	3		; Data OverRun
UPE0		equ	2		; USART Parity Error
U2X0		equ	1		; Double the USART Transmission Speed
MPCM0		equ	0		; Multi-processor Communication Mode

UCSR0B		port	0x0d		; USART Control and Status Register 0 B
RXCIE0		equ	7		; RX Complete Interrupt Enable 0
TXCIE0		equ	6		; TX Complete Interrupt Enable 0
UDRIE0		equ	5		; USART Data Register Empty Interrupt Enable 0
RXEN0		equ	4		; Receiver Enable 0
TXEN0		equ	3		; Transmitter Enable 0
UCSZ02		equ	2		; Character Size 0
RXB80		equ	1		; Receive Data Bit 8 0
TXB80		equ	0		; Transmit Data Bit 8 0

UCSR0C		port	0x0c		; USART Control and Status Register 0 C
UMSEL01		equ	7		; USART Mode Select 0
UMSEL00		equ	6
UPM01		equ	5		; USART Parity Mode 0
UPM00		equ	4
USBS0		equ	3		; USART Stop Bit Select 0
UCSZ01		equ	2		; USART Character Size
UDORD0		equ	2		; USART Data Order
UCSZ00		equ	1
UCPHA0		equ	1		; USART Character Size
UCPOL0		equ	0		; USERT Clock Phase

UCSR0D		port	0x0b		; USART Control and Status Register 0 D
RXSIE		equ	7		; USART RX Start Interrupt Enable
RXS		equ	6		; USART RX Start
SFDE		equ	5		; Start Frame Detection Enable

UBRR0L		port	0x09		; USART Baud Rate 0 Register LSB
UBRR0H		port	0x0a		; USART Baud Rate 0 Register MSB

		restore

		endif			; __regt1024inc
