		ifndef	__regt1024inc
__regt1024inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGT1024.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny102/104                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

VLMCSR		port	0x34		; VCC Level Monitoring Control and Status Register
VLM0		avrbit	VLMCSR,0	; Trigger Level of Voltage Level Monitor
VLM1		avrbit	VLMCSR,1
VLM2		avrbit	VLMCSR,2
VLMIE		avrbit	VLMCSR,6	; VLM Interrupt Enable
VLMF		avrbit	VLMCSR,7	; VLM Flag

RSTFLR		port	0x3b		; Reset Flag Register
WDRF		avrbit	RSTFLR,3	; Watchdog Reset Flag
EXTRF		avrbit	RSTFLR,1	; External Reset Flag
PORF		avrbit	RSTFLR,0	; Power-on Reset Flag

OSCCAL		port	0x39		; Oscillator Calibration

CLKPSR		port	0x36		; Clock Prescaler Register
CLKPS0		avrbit	CLKPSR,0	; Clock Prescaler Select
CLKPS1		avrbit	CLKPSR,1
CLKPS2		avrbit	CLKPSR,2

CLKMSR		port	0x37		; Clock Main Settings Register
CLKMS0		avrbit	CLKMSR,0	; Clock Main Select Bits
CLKMS1		avrbit	CLKMSR,1

SMCR		port	0x3a		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Mode Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2
SM2		avrbit	SMCR,3

PRR		port	0x35		; Power Reduction Register
PRTIM0		avrbit	PRR,0		; Power Reduction Timer/Counter 0
PRADC		avrbit	PRR,1		; Power Reduction A/D Converter
PRUSART0	avrbit	PRR,2		; Power Reduction USART

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

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
		if	MOMCPUNAME="ATTINY102"
__PORTA_BITS	 equ	0x07		; (bits 0..2 on ATtiny102)
		endif
PUEA		port	0x03		; Pull-Up Enable Port A
PUEA0		avrbit	PUEA,0
PUEA1		avrbit	PUEA,1
PUEA2		avrbit	PUEA,2
		if	MOMCPUNAME="ATTINY104"
PUEA3		 avrbit	PUEA,3
PUEA4		 avrbit	PUEA,4
PUEA5		 avrbit	PUEA,5
PUEA6		 avrbit	PUEA,6
PUEA7		 avrbit	PUEA,7
		endif
PINB		port	0x04		; Port B @ 0x04 (IO) ff.
PUEB		port	0x07		; Pull-Up Enable Port B
		if	MOMCPUNAME="ATTINY102"
__PORTB_BITS	 equ	0x0e		; (bits 1..3 on ATtiny102)
		endif
		if	MOMCPUNAME="ATTINY104"
__PORTB_BITS	 equ	0x0f		; (bits 0..3 on ATtiny104)
PUEB0		 avrbit	PUEB,0
		endif
PUEB1		avrbit	PUEB,1
PUEB2		avrbit	PUEB,2
PUEB3		avrbit	PUEB,3

PCMSK0		port	0x0f		; Pin-Change Mask Register 0
PCINT0		avrbit	PCMSK0,0	; Enable Pin-Change Interrupt 0
PCINT1		avrbit	PCMSK0,1	; Enable Pin-Change Interrupt 1
PCINT2		avrbit	PCMSK0,2	; Enable Pin-Change Interrupt 2
PCINT3		avrbit	PCMSK0,3	; Enable Pin-Change Interrupt 3
PCINT4		avrbit	PCMSK0,4	; Enable Pin-Change Interrupt 4
PCINT5		avrbit	PCMSK0,5	; Enable Pin-Change Interrupt 5
PCINT6		avrbit	PCMSK0,6	; Enable Pin-Change Interrupt 6
PCINT7		avrbit	PCMSK0,7	; Enable Pin-Change Interrupt 7
PCMSK1		port	0x10		; Pin-Change Mask Register 1
PCINT8		avrbit	PCMSK1,0	; Enable Pin-Change Interrupt 8
PCINT9		avrbit	PCMSK1,1	; Enable Pin-Change Interrupt 9
PCINT10		avrbit	PCMSK1,2	; Enable Pin-Change Interrupt 10
PCINT11		avrbit	PCMSK1,3	; Enable Pin-Change Interrupt 11

PORTCR		port	0x16		; Port Control Register
BBMA		avrbit	PORTCR,0	; Break-Before-Make Mode Enable Port A
BBMB		avrbit	PORTCR,1	; Break-Before-Make Mode Enable Port B

PCICR		port	0x12		; Pin-Change Interrupt Control Register

PCIFR		port	0x11		; Pin-Change Interrupt Flag Register

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
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
ISC00		avrbit	EICRA,0		; Interrupt Sense Control 0
ISC01		avrbit	EICRA,1

EIMSK		port	0x13		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0

EIFR		port	0x14		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x2e		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Tounter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x2d		; Timer/Counter 0 Control Register B
CS00            avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01            avrbit	TCCR0B,1
CS02            avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
WGM03		avrbit	TCCR0B,4
ICES0		avrbit	TCCR0B,6	; Timer/Counter 0 Input Capture Edge Select
ICNC0		avrbit	TCCR0B,7	; Timer/Counter 0 Input Capture Noise Canceling
TCCR0C		port	0x2c		; Timer/Counter 0 Control Register C
FOC0B		avrbit	TCCR0C,6	; Timer/Counter 0 Force Output Compare Match B
FOC0A		avrbit	TCCR0C,7	; Timer/Counter 0 Force Output Compare Match A
TCNT0L		port	0x28		; Timer/Counter 0 Value LSB
TCNT0H		port	0x29		; Timer/Counter 0 Value MSB
OCR0AL		port	0x26		; Timer/Counter 0 Output Compare Value A LSB
OCR0AH		port	0x27		; Timer/Counter 0 Output Compare Value A MSB
OCR0BL		port	0x24		; Timer/Counter 0 Output Compare Value B LSB
OCR0BH		port	0x25		; Timer/Counter 0 Output Compare Value B MSB
ICR0L		port	0x22		; Timer/Counter 0 Input Capture Register LSB
ICR0H		port	0x23		; Timer/Counter 0 Input Capture Register MSB

TIMSK0		port	0x2b		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE0		avrbit	TIMSK0,5	; Timer/Counter 0 Input Capture Interrupt Enable

TIFR0		port	0x2a		; Timer/Counter 0 Interrupt Status Register

GTCCR		port	0x2f		; General Timer/Counter Control Register
PSR		avrbit	GTCCR,0		; Prescaler 0 Reset Timer/Counter 0
REMAP		avrbit	GTCCR,1		; Timer Pin Mapping
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x31		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable Watchdog
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Watchdog Interrupt Enable
WDIF		avrbit	WDTCSR,7	; Watchdog Interrupt Flag

;----------------------------------------------------------------------------
; Analog Comparator

ACSRA		port    0x1f		; Analog Comparator Control and Status Register A
ACIS0		avrbit	ACSRA,0		; Interrupt-Mode
ACIS1		avrbit	ACSRA,1
ACIC		avrbit	ACSRA,2		; Use Comparator As Capture Signal For Timer 0?
ACIE		avrbit	ACSRA,3		; Interrupt Enable
ACI		avrbit	ACSRA,4		; Interrupt Flag
ACO		avrbit	ACSRA,5		; Analog Comparator Output
ACBG		avrbit	ACSRA,6		; Analog Comparator Band Gap Select
ACD		avrbit	ACSRA,7		; Disable

ACSRB		port	0x1e		; Analog Comparator Control and Status Register B
ACOE		avrbit	ACSRB,1		; Analog Comparator Output Enable
ACPMUX		avrbit	ACSRB,0		; Analog Comparator Positive Input Multiplexer

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x1b		; ADC Multiplexer Selection Register
MUX0		avrbit	ADMUX,0		; Analog Channel Selection
MUX1		avrbit	ADMUX,1
MUX2		avrbit	ADMUX,2
REFS0		avrbit	ADMUX,6		; Reference Selection
REFS1		avrbit	ADMUX,7

ADCSRA		port	0x1d		; ADC Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; ADC Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		port	0x1c		; ADC Control/Status Register A
ADTS0		avrbit	ADCSRB,0	; ADC Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS2		avrbit	ADCSRB,2

ADCL		port	0x19		; ADC Conversion Result LSB
ADCH		port	0x1a		; ADC Conversion Result MSB

DIDR0		port	0x17		; Digital Input Disable Register 0
ADC0D		avrbit	DIDR0,0		; ADC0 Digital Input Disable
ADC1D		avrbit	DIDR0,1		; ADC1 Digital Input Disable
ADC2D		avrbit	DIDR0,2		; ADC2 Digital Input Disable
ADC3D		avrbit	DIDR0,3		; ADC3 Digital Input Disable
ADC4D		avrbit	DIDR0,4		; ADC4 Digital Input Disable
ADC5D		avrbit	DIDR0,5		; ADC5 Digital Input Disable
ADC6D		avrbit	DIDR0,6		; ADC6 Digital Input Disable
ADC7D		avrbit	DIDR0,7		; ADC7 Digital Input Disable

;----------------------------------------------------------------------------
; USART

UDR0		port	0x08		; USART0 I/O Data Register

UCSR0A		port	0x0e		; USART0 Control and Status Register A
RXC0		avrbit	UCSR0A,7	; USART0 Receive Complete
TXC0		avrbit	UCSR0A,6	; USART0 Transmit Complete
UDRE0		avrbit	UCSR0A,5	; USART0 Data Register Empty
FE0		avrbit	UCSR0A,4	; USART0 Frame Error
DOR0		avrbit	UCSR0A,3	; USART0 Data OverRun
UPE0		avrbit	UCSR0A,2	; USART0 Parity Error
U2X0		avrbit	UCSR0A,1	; USART0 Double Transmission Speed
MPCM0		avrbit	UCSR0A,0	; USART0 Multi Processor Communication Mode

UCSR0B		port	0x0d		; USART0 Control and Status Register B
RXCIE0		avrbit	UCSR0B,7	; USART0 RX Complete Interrupt Enable
TXCIE0		avrbit	UCSR0B,6	; USART0 TX Complete Interrupt Enable
UDRIE0		avrbit	UCSR0B,5	; USART0 Data Register Empty Interrupt Enable
RXEN0		avrbit	UCSR0B,4	; USART0 Receiver Enable
TXEN0		avrbit	UCSR0B,3	; USART0 Transmitter Enable
UCSZ02		avrbit	UCSR0B,2	; USART0 Character Size
RXB80		avrbit	UCSR0B,1	; USART0 Receive Data Bit 8
TXB80		avrbit	UCSR0B,0	; USART0 Transmit Data Bit 8

UCSR0C		port	0x0c		; USART0 Control and Status Register C
UMSEL01		avrbit	UCSR0C,7	; USART0 Mode Select
UMSEL00		avrbit	UCSR0C,6
UPM01		avrbit	UCSR0C,5	; USART0 Parity Mode
UPM00		avrbit	UCSR0C,4
USBS0		avrbit	UCSR0C,3	; USART0 Stop Bit Select
UCSZ01		avrbit	UCSR0C,2	; USART0 Character Size
UDORD0		avrbit	UCSR0C,2	; USART0 Data Order
UCSZ00		avrbit	UCSR0C,1
UCPHA0		avrbit	UCSR0C,1	; USART0 Character Size
UCPOL0		avrbit	UCSR0C,0	; USERT0 Clock Phase

UCSR0D		port	0x0b		; USART0 Control and Status Register D
RXSIE		avrbit	UCSR0D,7	; USART0 RX Start Interrupt Enable
RXS		avrbit	UCSR0D,6	; USART0 RX Start
SFDE		avrbit	UCSR0D,5	; Start0 Frame Detection Enable

UBRR0L		port	0x09		; USART0 Baud Rate Register LSB
UBRR0H		port	0x0a		; USART0 Baud Rate Register MSB

		restore

		endif			; __regt1024inc
