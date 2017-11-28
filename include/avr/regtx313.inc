		ifndef	__tx313inc
__tx313inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTX313.INC                                            *
;*                                                                          *
;*   Contains common bit & register definitions for AT2313(A)/4313          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM0		equ	4		; Sleep Mode Select
SE		equ	5		; Sleep Enable
SM1		equ	6

MCUSR		port	0x34		; MCU Status Register
WDRF		equ	3		; Watchdog Reset Flag
BORF		equ	2		; Brown-out Reset Flag
EXTRF		equ	1		; External Reset Flag
PORF		equ	0		; Power-On Reset Flag

OSCCAL		port	0x31		; Oscillator Calibration

CLKPR		port	0x26		; Clock Prescaler
CLKPS0		equ	0		; Prescaler Select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

		if MOMCPUNAME<>"ATTINY2313"
PRR		 port	0x06		; Power Reduction Register
PRUSART		 equ	0		; Power Reduction USART
PRUSI		 equ	1		; Power Reduction USI
PRTIM0		 equ	2		; Power Reduction Timer/Counter 0
PRTIM1		 equ	3		; Power Reduction Timer/Counter 1

BODCR		 port	0x07		; Brown-Out Detector Control Register
BODSE		 equ	0		; BOD Sleep
BODS		 equ	1		; BOD Sleep Enable
		endif

;----------------------------------------------------------------------------
; EEPROM/Flash Access

EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EEPM1		equ	5		; EEPROM Program Mode
EEPM0		equ	4
EERIE		equ	3		; EEPROM Ready Interrupt Enable
EEMPE		equ	2		; EEPROM Master Write Enable
EEPE		equ	1		; EEPROM Write Enable
EERE		equ	0		; EEPROM Read Enable

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
CTPB		equ	4		; Clear Temporary Page Buffer
RFLB		equ	3		; Read Fuse and Lock Bits
PGWRT		equ	2		; Page Write
PGERS		equ	1		; Page Erase
SELFPRGEN	equ	0		; Self Programming Enable

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	7		; Pull-Up Disable

PINA		port	0x19		; Port A @ 0x19 (IO) ff. (bits 0..2)
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D q 0x10 (IO) ff. (bits 0..6)

GPIOR0		port	0x13		; General Purpose I/O Register 0
GPIOR1		port	0x14		; General Purpose I/O Register 1
GPIOR2		port	0x15		; General Purpose I/O Register 2

DIDR		port	0x01		; Digital Input Disable Register
AIN0D		equ	0		; AIN0 Digital Input Disable
AIN1D		equ	1		; AIN1 Digital Input Disable

                if MOMCPUNAME="ATTINY2313"
PCMSK		 port	0x20		; Pin Change Interrupt Mask
		elseif
PCMSK0		 port	0x20		; Pin Change Interrupt Mask 0
		endif
PCINT0		equ	0		; Enable Pin Change Interrupt 0 
PCINT1		equ	1		; Enable Pin Change Interrupt 1
PCINT2		equ	2		; Enable Pin Change Interrupt 2
PCINT3		equ	3		; Enable Pin Change Interrupt 3
PCINT4		equ	4		; Enable Pin Change Interrupt 4
PCINT5		equ	5		; Enable Pin Change Interrupt 5
PCINT6		equ	6		; Enable Pin Change Interrupt 6
PCINT7		equ	7		; Enable Pin Change Interrupt 7

		if MOMCPUNAME<>"ATTINY2313"
PCMSK1		 port	0x04		; Pin Change Interrupt Mask 1
PCINT8		 equ	0		; Enable Pin Change Interrupt 8
PCINT9		 equ	1		; Enable Pin Change Interrupt 9
PCINT10		 equ	2		; Enable Pin Change Interrupt 10
PCMSK2		 port	0x05		; Pin Change Interrupt Mask 2
PCINT11		 equ	0		; Enable Pin Change Interrupt 11
PCINT12		 equ	1		; Enable Pin Change Interrupt 12
PCINT13		 equ	2		; Enable Pin Change Interrupt 13
PCINT14		 equ	3		; Enable Pin Change Interrupt 14
PCINT15		 equ	4		; Enable Pin Change Interrupt 15
PCINT16		 equ	5		; Enable Pin Change Interrupt 16
PCINT17		 equ	6		; Enable Pin Change Interrupt 17
		endif

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; external interrupt request 0
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
		nextenum EEPROM_READY_vect	; EEPROM ready
		nextenum WDT_OVERFLOW_vect	; Watchdog Timer Overflow
		if MOMCPUNAME<>"ATTINY2313"
		 nextenum PCINT1_vect		; Pin Change Interrupt 1
		 nextenum PCINT2_vect		; Pin Change Interrupt 2
		endif

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; External Interrupt 0 Sense Control
ISC01		equ	1
ISC10		equ	2		; External Interrupt 1 Sense Control
ISC11		equ	3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		; Enable External Interrupt 0
INT1		equ	7		; Enable External Interrupt 1
		if MOMCPUNAME="ATTINY2313"
PCIE		 equ	5		; Pin Change Interrupt Enable
		elseif
PCIE0		 equ	5		; Pin Change Interrupt Enable 0
PCIE2		 equ	4		; Pin Change Interrupt Enable 2
PCIE1		 equ	3		; Pin Change Interrupt Enable 1
		endif

EIFR		port	0x3a		; External Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 Occured
INTF1		equ	7		; External Interrupt 1 Occured
		if MOMCPUNAME="ATTINY2313"
PCIF		 equ	5		; Pin Change Interrupt Occured
		elseif
PCIF0		 equ	5		; Pin Change Interrupt 0 Occured
PCIF2		 equ	4		; Pin Change Interrupt 2 Occured
PCIF1		 equ	3		; Pin Change Interrupt 1 Occured
		endif

;----------------------------------------------------------------------------
; Timers

TCCR0A		port	0x30		; Timer/Counter 0 Control Register A
WGM00		equ	0		; Timer/Counter 0 Waveform Generation Mode
WGM01		equ	1
COM0B0		equ	4		; Timer/Counter 0 Output Compare Mode B
COM0B1		equ	5
COM0A0		equ	6		; Timer/Counter 0 Output Compare Mode A
COM0A1		equ	7
TCCR0B		port	0x33		; Timer/Counter 0 Control Register B
CS00		equ	0		; Timer/Counter 0 Clock Select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; Timer/Counter 0 Force Output Compare B
FOC0A		equ	7		; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x32		; Timer/Counter 0 Value
OCR0A		port	0x36		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x3c		; Timer/Counter 0 Output Compare Value B

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
WGM10		equ	0		; Timer/Counter 1 Waveform Generation Mode
WGM11		equ	1
COM1B0		equ	4		; Timer/Counter 1 Output Compare Mode B
COM1B1		equ	5
COM1A0		equ	6		; Timer/Counter 1 Output Compare Mode A
COM1A1		equ	7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10            equ     0               ; Timer/Counter 1 Clock Select
CS11            equ     1
CS12            equ     2
WGM12		equ	3
WGM13		equ	4
ICES1		equ	6		; Timer/Counter 1 Input Capture Edge Select
ICNC1		equ	7		; Timer/Counter 1 Input Capture Noise Canceler
TCCR1C		port	0x22		; Timer/Counter 1 Control Register C
FOC1B		equ	6		; Timer/Counter 1 Force Output Compare B
FOC1A		equ	7		; Timer/Counter 1 Force Output Compare A
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture Register LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture Register MSB

TIMSK		port	0x39		; Timer/Counter Interrupt Mask Register
OCIE0A		equ	0		; Timer/Counter 0 Output Compare Interrupt Enable A
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable
OCIE0B		equ	2		; Timer/Counter 0 Output Compare Interrupt Enable B
ICIE1		equ	3		; Timer/Counter 0 Input Capute Interrupt Enable
OCIE1B		equ	5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		equ	6		; Timer/Counter 1 Output Compare Interrupt Enable A
TOIE1		equ	7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Status Register

GTCCR		port	0x23		; General Timer/Counter Control Register
PSR10		equ	0		; Timer/Counter 0/1 Prescaler Reset

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCSR		port	0x21		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable Watchdog
WDCE		equ	4		; Change Enable
WDP3		equ	5
WDIE		equ	6		; Enable Watchdog Interrupt
WDIF		equ	7		; Watchdog Interrupt Occured?

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/acm.inc"

;----------------------------------------------------------------------------
; USART

UDR		port	0x0c		; USART I/O Data Register

UCSRA		port	0x0b		; USART Control & Status Register A
MPCM		equ	0		; Multi Processor Communication Mode
U2X		equ	1		; Double Transmission Speed
UPE		equ	2		; Parity Error
DOR		equ	3		; Overrun
FE		equ	4		; Framing Error
UDRE		equ	5		; Data Register Empty
TXC		equ	6		; Transmit Complete
RXC		equ	7		; Receive Complete

UCSRB		port	0x0a		; USART Control & Status Register B
TXB8		equ     0		; Transmit Bit 8
RXB8		equ     1		; Receive Bit 8
UCSZ2		equ     2		; Character Size
TXEN		equ     3		; Enable Transmitter
RXEN		equ     4		; Enable Receiver
UDRIE		equ     5		; Enable Data Register Empty Interrupt
TXCIE		equ     6		; Enable Transmit Complete Interrupt
RXCIE		equ     7		; Enable Receive Complete Interrupt

UCSRC		port	0x03		; USART Control & Status Register C
UCPOL		equ	0		; Clock Polarity
UCSZ0		equ	1		; Character Size
UCSZ1		equ	2
USBS		equ	3		; Stop Bit Select
UPM0		equ	4		; Parity Mode : Odd/Even
UPM1		equ	5		; Parity Mode : Enable/Disable
		if MOMCPUNAME="ATTINY2313"
UMSEL		 equ	6		; USART Mode Select
		elseif
UMSEL0		 equ	6		; USART Mode Select
UMSEL1		 equ	7
		endif

UBRRH		port	0x02		; Baud Rate Register High
UBRRL		port	0x09		; Baud Rate Register Low

;----------------------------------------------------------------------------
; USI

USIDR		port	0x0f		; USI Data Register

USISR		port	0x0e		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		port	0x0d		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

		if MOMCPUNAME<>"ATTINY2313"
USIBR		 port	0x00		; USI Buffer Register
		endif

		restore			; re-enable listing

		endif			; __tx313inc
