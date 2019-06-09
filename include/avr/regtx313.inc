		ifndef	__tx313inc
__tx313inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTX313.INC                                            *
;*                                                                          *
;*   Contains common bit & Register definitions for AT2313(A)/4313          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		avrbit	MCUCR,4		; Sleep Mode Select
SE		avrbit	MCUCR,5		; Sleep Enable
SM1		avrbit	MCUCR,6

MCUSR		port	0x34		; MCU Status Register
WDRF		avrbit	MCUSR,3		; Watchdog Reset Flag
BORF		avrbit	MCUSR,2		; Brown-out Reset Flag
EXTRF		avrbit	MCUSR,1		; External Reset Flag
PORF		avrbit	MCUSR,0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescaler
CLKPS0		avrbit	CLKPR,0		; Prescaler Select
CLKPS1		avrbit	CLKPR,1
CLKPS2		avrbit	CLKPR,2
CLKPS3		avrbit	CLKPR,3
CLKPCE		avrbit	CLKPR,7		; Clock Prescaler Change Enable

		if MOMCPUNAME<>"ATTINY2313"
PRR		 port	0x06		; Power Reduction Register
PRUSART		 avrbit	PRR,0		; Power Reduction USART
PRUSI		 avrbit	PRR,1		; Power Reduction USI
PRTIM0		 avrbit	PRR,2		; Power Reduction Timer/Counter 0
PRTIM1		 avrbit	PRR,3		; Power Reduction Timer/Counter 1

BODCR		 port	0x07		; Brown-Out Detector Control Register
BODSE		 avrbit	BODCR,0		; BOD Sleep
BODS		 avrbit	BODCR,1		; BOD Sleep Enable
		endif

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		avrbit	EECR,5		; EEPROM Program Mode
EEPM0		avrbit	EECR,4
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Write Enable
EEPE		avrbit	EECR,1		; EEPROM Write Enable
EERE		avrbit	EECR,0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
CTPB		avrbit	SPMCSR,4	; Clear Temporary Page Buffer
RFLB		avrbit	SPMCSR,3	; Read Fuse and Lock Bits
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SELFPRGEN	avrbit	SPMCSR,0	; Self Programming Enable

;----------------------------------------------------------------------------
; GPIO

PUD		avrbit	MCUCR,7		; Pull-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
__PORTA_BITS	equ	0x07		; (bits 0..2)
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D q 0x10 (IO) ff.
__PORTD_BITS	equ	0x7f		; (bits 0..6)

GPIOR0		port	0x13		; General Purpose I/O Register 0
GPIOR1		port	0x14		; General Purpose I/O Register 1
GPIOR2		port	0x15		; General Purpose I/O Register 2

DIDR		port	0x01		; Digital Input Disable Register
AIN0D		avrbit	DIDR,0		; AIN0 Digital Input Disable
AIN1D		avrbit	DIDR,1		; AIN1 Digital Input Disable

                if MOMCPUNAME="ATTINY2313"
PCMSK		 port	0x20		; Pin Change Interrupt Mask
PCINT0		 avrbit	PCMSK,0		; Enable Pin Change Interrupt 0
PCINT1		 avrbit	PCMSK,1		; Enable Pin Change Interrupt 1
PCINT2		 avrbit	PCMSK,2		; Enable Pin Change Interrupt 2
PCINT3		 avrbit	PCMSK,3		; Enable Pin Change Interrupt 3
PCINT4		 avrbit	PCMSK,4		; Enable Pin Change Interrupt 4
PCINT5		 avrbit	PCMSK,5		; Enable Pin Change Interrupt 5
PCINT6		 avrbit	PCMSK,6		; Enable Pin Change Interrupt 6
PCINT7		 avrbit	PCMSK,7		; Enable Pin Change Interrupt 7
		elseif
PCMSK0		 port	0x20		; Pin Change Interrupt Mask 0
		endif

		if MOMCPUNAME<>"ATTINY2313"
PCMSK1		 port	0x04		; Pin Change Interrupt Mask 1
PCINT8		 avrbit	PCMSK1,0	; Enable Pin Change Interrupt 8
PCINT9		 avrbit	PCMSK1,1	; Enable Pin Change Interrupt 9
PCINT10		 avrbit	PCMSK1,2	; Enable Pin Change Interrupt 10
PCMSK2		 port	0x05		; Pin Change Interrupt Mask 2
PCINT11		 avrbit	PCMSK2,0	; Enable Pin Change Interrupt 11
PCINT12		 avrbit	PCMSK2,1	; Enable Pin Change Interrupt 12
PCINT13		 avrbit	PCMSK2,2	; Enable Pin Change Interrupt 13
PCINT14		 avrbit	PCMSK2,3	; Enable Pin Change Interrupt 14
PCINT15		 avrbit	PCMSK2,4	; Enable Pin Change Interrupt 15
PCINT16		 avrbit	PCMSK2,5	; Enable Pin Change Interrupt 16
PCINT17		 avrbit	PCMSK2,6	; Enable Pin Change Interrupt 17
		endif

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect
		nextenum TIMER1_CAPT_vect
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum USART_RX_vect		; USART Rx Complete
		nextenum USART_UDRE_vect	; USART Data Register Empty
		nextenum USART_TX_vect		; USART Tx Complete
		nextenum ANA_COMP_vect		; Analog Comparator
		if MOMCPUNAME="ATTINY2313"
		 nextenum PCINT_vect		; Pin Change Interrupt
		elseif
		 nextenum PCINT0_vect		; Pin Change Interrupt 0
		endif
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum USI_START_vect		; USI Start
		nextenum USI_OVERFLOW_vect	; USI Overflow
		nextenum EEPROM_READY_vect	; EEPROM Ready
		nextenum WDT_OVERFLOW_vect	; Watchdog Timer Overflow
		if MOMCPUNAME<>"ATTINY2313"
		 nextenum PCINT1_vect		; Pin Change Interrupt 1
		 nextenum PCINT2_vect		; Pin Change Interrupt 2
		endif

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1
		if MOMCPUNAME="ATTINY2313"
PCIE		 avrbit	GIMSK,5		; Pin Change Interrupt Enable
		elseif
PCIE0		 avrbit	GIMSK,5		; Pin Change Interrupt Enable 0
PCIE2		 avrbit	GIMSK,4		; Pin Change Interrupt Enable 2
PCIE1		 avrbit	GIMSK,3		; Pin Change Interrupt Enable 1
		endif

EIFR		port	0x3a		; External Interrupt Flag Register
INTF0		avrbit	EIFR,6		; External Interrupt 0 Occured
INTF1		avrbit	EIFR,7		; External Interrupt 1 Occured
		if MOMCPUNAME="ATTINY2313"
PCIF		 avrbit	EIFR,5		; Pin Change Interrupt Occured
		elseif
PCIF0		 avrbit	EIFR,5		; Pin Change Interrupt 0 Occured
PCIF2		 avrbit	EIFR,4		; Pin Change Interrupt 2 Occured
PCIF1		 avrbit	EIFR,3		; Pin Change Interrupt 1 Occured
		endif

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x30		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Output Compare Mode B
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Output Compare Mode A
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x3c		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		avrbit	TCCR1A,0	; Timer/Counter 1 Waveform Generation Mode
WGM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Output Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Output Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10            avrbit	TCCR1B,0	; Timer/Counter 1 Clock Select
CS11            avrbit	TCCR1B,1
CS12            avrbit	TCCR1B,2
WGM12		avrbit	TCCR1B,3
WGM13		avrbit	TCCR1B,4
ICES1		avrbit	TCCR1B,6	; Timer/Counter 1 Input Capture Edge Select
ICNC1		avrbit	TCCR1B,7	; Timer/Counter 1 Input Capture Noise Canceler
TCCR1C		port	0x22		; Timer/Counter 1 Control Register C
FOC1B		avrbit	TCCR1C,6	; Timer/Counter 1 Force Output Compare B
FOC1A		avrbit	TCCR1C,7	; Timer/Counter 1 Force Output Compare A
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture Register LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture Register MSB

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
OCIE0A		avrbit	TIMSK,0		; Timer/Counter 0 Output Compare Interrupt Enable A
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		avrbit	TIMSK,2		; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE1		avrbit	TIMSK,3		; Timer/Counter 0 Input Capute Interrupt Enable
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable A
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Status Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR10		avrbit	GTCCR,0		; Timer/Counter 0/1 Prescaler Reset

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x21		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable Watchdog
WDCE		avrbit	WDTCSR,4	; Change Enable
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Enable Watchdog Interrupt
WDIF		avrbit	WDTCSR,7	; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

		include "acm.inc"

;----------------------------------------------------------------------------
; USART

UDR		port	0x0c		; USART I/O Data Register

UCSRA		port	0x0b		; USART Control & Status Register A
MPCM		avrbit	UCSRA,0		; Multi Processor Communication Mode
U2X		avrbit	UCSRA,1		; Double Transmission Speed
UPE		avrbit	UCSRA,2		; Parity Error
DOR		avrbit	UCSRA,3		; Overrun
FE		avrbit	UCSRA,4		; Framing Error
UDRE		avrbit	UCSRA,5		; Data Register Empty
TXC		avrbit	UCSRA,6		; Transmit Complete
RXC		avrbit	UCSRA,7		; Receive Complete

UCSRB		port	0x0a		; USART Control & Status Register B
TXB8		avrbit	UCSRB,0		; Transmit Bit 8
RXB8		avrbit	UCSRB,1		; Receive Bit 8
UCSZ2		avrbit	UCSRB,2		; Character Size
TXEN		avrbit	UCSRB,3		; Enable Transmitter
RXEN		avrbit	UCSRB,4		; Enable Receiver
UDRIE		avrbit	UCSRB,5		; Enable Data Register Empty Interrupt
TXCIE		avrbit	UCSRB,6		; Enable Transmit Complete Interrupt
RXCIE		avrbit	UCSRB,7		; Enable Receive Complete Interrupt

UCSRC		port	0x03		; USART Control & Status Register C
UCPOL		avrbit	UCSRC,0		; Clock Polarity
UCSZ0		avrbit	UCSRC,1		; Character Size
UCSZ1		avrbit	UCSRC,2
USBS		avrbit	UCSRC,3		; Stop Bit Select
UPM0		avrbit	UCSRC,4		; Parity Mode : Odd/Even
UPM1		avrbit	UCSRC,5		; Parity Mode : Enable/Disable
		if MOMCPUNAME="ATTINY2313"
UMSEL		 avrbit	UCSRC,6		; USART Mode Select
		elseif
UMSEL0		 avrbit	UCSRC,6		; USART Mode Select
UMSEL1		 avrbit	UCSRC,7
		endif

UBRRH		port	0x02		; Baud Rate Register High
UBRRL		port	0x09		; Baud Rate Register Low

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		avrbit	USISR,0		; Counter Value
USICNT1		avrbit	USISR,1
USICNT2		avrbit	USISR,2
USICNT3		avrbit	USISR,3
USIDC		avrbit	USISR,4		; Data Output Collision
USIPF		avrbit	USISR,5		; Stop Condition Flag
USIOIF		avrbit	USISR,6		; Counter Overflow Interrupt Flag
USISIF		avrbit	USISR,7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		avrbit	USICR,0		; Toggle Clock Port Pin
USICLK		avrbit	USICR,1		; Clock Strobe
USICS0		avrbit	USICR,2		; Clock Source Select
USICS1		avrbit	USICR,3
USIWM0		avrbit	USICR,4		; Wire Mode
USIWM1		avrbit	USICR,5		
USIOIE		avrbit	USICR,6		; Counter Overflow Interrupt Enable
USISIE		avrbit	USICR,7		; Start Condition Interrupt Enable

		if MOMCPUNAME<>"ATTINY2313"
USIBR		 port	0x00		; USI Buffer Register
		endif

		restore			; re-enable listing

		endif			; __tx313inc
