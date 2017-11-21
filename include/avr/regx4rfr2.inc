		ifndef	__regx4rfr2inc
__regx4rfr2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGX4RFR2.INC                                           *
;*                                                                          *
;*  Contains bit & register definitions for ATmega644RFR2/1284RFR2/2564RFR2 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Control

MCUCR		port	0x35		; MCU Control Register
IVCE		equ	0		; interrupt vector change enable
IVSEL		equ	1		; interrupt vector select

MCUSR		port	0x34		; MCU Status Register
PORF		equ	0		; power-on reset occured
EXTRF		equ	1		; external reset occured
BORF		equ	2		; brown out reset occured
WDRF		equ	3		; watchdog reset occured

SMCR		port	0x33		; sleep mode control register
SE		equ	0		; sleep mode enable
SM0		equ	1		; sleep mode select
SM1		equ	2
SM2		equ	3

PRR0		sfr	0x64		; Power Reduction Register 0
PRADC		equ	0		; Power Reduction ADC
PRUSART0	equ	1		; Power Reduction USART0
PRSPI		equ	2		; Power Reduction Serial Peripheral Interface
PRTIM1		equ	3		; Power Reduction Timer/Counter 1
PRPGA		equ	4		; Power Reduction 
PRTIM0		equ	5		; Power Reduction Timer/Counter 0
PRTIM2		equ	6		; Power Reduction Timer/Counter 2
PRTWI		equ	7		; Power Reduction Two Wire Interface

PRR1		sfr	0x65		; Power Reduction Register 1
PRUSART1	equ	0		; Power Reduction USART1
PRTIM3		equ	3		; Power Reduction Timer/Counter 3
PRTIM4		equ	4		; Power Reduction Timer/Counter 4
PRTIM5		equ	5		; Power Reduction Timer/Counter 5
PRTRX24		equ	6		; Power Reduction Transceiver

PRR2		sfr	0x63		; Power Reduction Register 2
PRRAM0		equ	0		; Power Reduction SRAM 0
PRRAM1		equ	1		; Power Reduction SRAM 1
PRRAM2		equ	2		; Power Reduction SRAM 2
PRRAM3		equ	3		; Power Reduction SRAM 3

OSCCAL		sfr	0x66		; oscillator calibration

CLKPR		sfr	0x61		; clock prescale register
CLKPS0		equ	0		; clock prescaler select
CLKPS1		equ	1
CLKPS2		equ	2
CLKPS3		equ	3
CLKPCE		equ	7		; Clock Prescaler Change Enable

LLCR		sfr	0x12f		; Low Leakage Voltage Regulator Control Register
LLDONE		equ	5		; Calibration Done
LLCOMP		equ	4		; Comparator Output
LLCAL		equ	3		; Calibration Active
LLTCO		equ	2		; Temperature Coefficient of Current Source
LLSHORT		equ	1		; Short Lower Calibration Circuit
LLENCAL		equ	0		; Enable Automatic Calibration

LLDRH		sfr	0x131		; Low Leakage Voltage Regulator Data Register (High-Byte)
LLDRH4		equ	4		; High-Byte Data Register Bits
LLDRH3		equ	3
LLDRH2		equ	2
LLDRH1		equ	1
LLDRH0		equ	0

LLDRL		sfr	0x130		; Low Leakage Voltage Regulator Data Register (Low-Byte)
LLDRL3		equ	3		; Low-Byte Data Register Bits
LLDRL2		equ	2
LLDRL1		equ	1
LLDRL0		equ	0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem2.inc"
		include	"avr/spmcsr37.inc"

		; additional bits in EECR
EEPM0		equ	4		; EEPROM Programming Mode
EEPM1		equ	5

		; additional bits in SPMCSR
SIGRD		equ	5		; signature row read

DRTRAM0		sfr	0x135		; Data Retention Configuration Register #0
DRTRAM1		sfr	0x134		; Data Retention Configuration Register #1
DRTRAM2		sfr	0x133		; Data Retention Configuration Register #2
DRTRAM3		sfr	0x132		; Data Retention Configuration Register #3
DRTSWOK		equ	5		; DRT Switch OK
ENDRT		equ	4		; Enable SRAM Data Retention
DRTMP1		equ	3		; Positive Data Retention Voltage Setting
DRTMP0		equ	2
DRTMN1		equ	1		; Negative Data Retention Voltage Setting
DRTMN0		equ	0

NEMCR		sfr	0x75		; Flash Extended-Mode Control-Register
ENEAM		equ	6		; Enable Extended Address Mode for Extra Rows
AEAM1		equ	5		; Address for Extended Address Mode of Extra Rows
AEAM0		equ	4

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCR
JTD		equ	7		; JTAG disable

		; bits in MCUSR
JTRF		equ	4		; JTAG reset occured

OCDR		port	0x31		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
PUD		equ	4		; pull-up disable

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PINC		port	0x06		; Port C @ 0x03 (IO) ff.
PIND		port    0x09		; Port D @ 0x09 (IO) ff.
PINE		port    0x0c		; Port E @ 0x0c (IO) ff.
PINF		port	0x0f		; Port F @ 0x0f (IO) ff.
PING		port	0x12		; Port G @ 0x12 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1
PCMSK2		sfr	0x6d		; Pin Change Mask Register 2 (not useful)
PCICR		sfr	0x68		; Pin Change Interrupt Control Register
PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

GPIOR0		port	0x1e		; general purpose registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

DPDS0		sfr	0x136		; Port Driver Strength Register 0
PFDRV1		equ	7		; Driver Strength Port F
PFDRV0		equ	6
PEDRV1		equ	5		; Driver Strength Port E
PEDRV0		equ	4
PDDRV1		equ	3		; Driver Strength Port D
PDDRV0		equ	2
PBDRV1		equ	1		; Driver Strength Port B
PBDRV0		equ	0

DPDS1		sfr	0x137		; Port Driver Strength Register 1
PGDRV1		equ	1		; Driver Strength Port G
PGDRV0		equ	0

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum     INT0_vect=2		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum INT3_vect		; external interrupt request 3
		nextenum INT4_vect		; external interrupt request 4
		nextenum INT5_vect		; external interrupt request 5
		nextenum INT6_vect		; external interrupt request 6
		nextenum INT7_vect		; external interrupt request 7
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum PCINT2_vect		; pin change interrupt request 2 (not useful)
		nextenum WDT_vect		; watchdog time-out interrupt
		nextenum TIMER2_COMPA_vect	; timer/counter 2 compare match A
		nextenum TIMER2_COMPB_vect	; timer/counter 2 compare match B
		nextenum TIMER2_OVF_vect	; timer/counter 2 overflow
		nextenum TIMER1_CAPT_vect	; timer/counter 1 capture event
		nextenum TIMER1_COMPA_vect	; timer/counter 1 compare match A
		nextenum TIMER1_COMPB_vect	; timer/counter 1 compare match B
		nextenum TIMER1_COMPC_vect	; timer/counter 1 compare match C
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMPA_vect	; timer/counter 0 compare match A
		nextenum TIMER0_COMPB_vect	; timer/counter 0 compare match B
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum SPI_STC_vect		; SPI serial transfer complete
		nextenum USART0_RX_vect		; USART0 Rx complete
		nextenum USART0_UDRE_vect	; USART0 data register empty
		nextenum USART0_TX_vect		; USART0 Tx complete
		nextenum ANALOG_COMP_vect	; analog comparator
		nextenum ADC_vect		; ADC conversion complete
		nextenum EE_READY_vect		; EEPROM ready
		nextenum TIMER3_CAPT_vect	; timer/counter 3 capture event
		nextenum TIMER3_COMPA_vect	; timer/counter 3 compare match A
		nextenum TIMER3_COMPB_vect	; timer/counter 3 compare match B
		nextenum TIMER3_COMPC_vect	; timer/counter 3 compare match C
		nextenum TIMER3_OVF_vect	; timer/counter 3 overflow
		nextenum USART1_RX_vect		; USART1 Rx complete
		nextenum USART1_UDRE_vect	; USART1 data register empty
		nextenum USART1_TX_vect		; USART1 Tx complete
		nextenum TWI_vect		; two-wire serial interface
		nextenum SPM_READY_vect		; store program memory ready
		nextenum TIMER4_CAPT_vect	; timer/counter 4 capture event (not useful)
		nextenum TIMER4_COMPA_vect	; timer/counter 4 compare match A
		nextenum TIMER4_COMPB_vect	; timer/counter 4 compare match B
		nextenum TIMER4_COMPC_vect	; timer/counter 4 compare match C
		nextenum TIMER4_OVF_vect	; timer/counter 4 overflow
		nextenum TIMER5_CAPT_vect	; timer/counter 5 capture event (not useful)
		nextenum TIMER5_COMPA_vect	; timer/counter 5 compare match A
		nextenum TIMER5_COMPB_vect	; timer/counter 5 compare match B
		nextenum TIMER5_COMPC_vect	; timer/counter 5 compare match C
		nextenum TIMER5_OVF_vect	; timer/counter 5 overflow
		nextenum USART2_RX_vect		; USART2 Rx complete (not useful)
		nextenum USART2_UDRE_vect	; USART2 data register empty (not useful)
		nextenum USART2_TX_vect		; USART2 Tx complete (not useful)
		nextenum USART3_RX_vect		; USART3 Rx complete (not useful)
		nextenum USART3_UDRE_vect	; USART3 data register empty (not useful)
		nextenum USART3_TX_vect		; USART3 Tx complete (not useful)
		nextenum TRX24_PLL_LOCK_vect	; Transceiver PLL Lock
		nextenum TRX24_PLL_UNLOCK_vect	; Transceiver PLL Unlock
		nextenum TRX24_RX_START_vect	; Transceiver Receive Start
		nextenum TRX24_RX_END_vect	; Transceiver Receive End
		nextenum TRX24_CCA_ED_DONE_vect	; Transceiver CCAED Meassurement finished
		nextenum TRX24_XAH_AMI_vect	; Transceiver Frame Address Match
		nextenum TRX24_TX_END_vect	; Transceiver Transmit End
		nextenum TRX24_AWAKE_vect	; Transceiver Wakeup finished
		nextenum SCNT_CMP1_vect		; Symbol Counter Compare Match 1
		nextenum SCNT_CMP2_vect		; Symbol Counter Compare Match 2
		nextenum SCNT_CMP3_vect		; Symbol Counter Compare Match 3
		nextenum SCNT_OVFL_vect		; Symbol Counter Overflow
		nextenum SCNT_BACKOFF_vect	; Symbol Counter Backoff Slot Counter
		nextenum AES_READY_vect		; AES Encryption Ready
		nextenum BAT_LOW_vect		; Battery Monitor Alert
		nextenum TRX24_TX_START_vect	; Transceiver Transmit Start
		nextenum TRX24_AMI0_vect	; Transceiver Address Match Filter 0
		nextenum TRX24_AMI1_vect	; Transceiver Address Match Filter 1
		nextenum TRX24_AMI2_vect	; Transceiver Address Match Filter 2
		nextenum TRX24_AMI3_vect	; Transceiver Address Match Filter 3

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1
ISC10		equ	2		; external interrupt 1 sense control
ISC11		equ	3
ISC20		equ	4		; external interrupt 2 sense control
ISC21		equ	5
ISC30		equ	6		; external interrupt 3 sense control
ISC31		equ	7

EICRB		sfr	0x6a		; External Interrupt Control Register B
ISC40		equ	0		; external interrupt 4 sense control
ISC41		equ	1
ISC50		equ	2		; external interrupt 5 sense control
ISC51		equ	3
ISC60		equ	4		; external interrupt 6 sense control
ISC61		equ	5
ISC70		equ	6		; external interrupt 7 sense control
ISC71		equ	7

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1
INT2		equ	2		; enable external interrupt 2
INT3		equ	3		; enable external interrupt 3
INT4		equ	4		; enable external interrupt 4
INT5		equ	5		; enable external interrupt 5
INT6		equ	6		; enable external interrupt 6
INT7		equ	7		; enable external interrupt 7

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; external Interrupt 0 occured
INTF1		equ	1		; external Interrupt 1 occured
INTF2		equ	2		; external Interrupt 2 occured
INTF3		equ	3		; external Interrupt 3 occured
INTF4		equ	4		; external Interrupt 4 occured
INTF5		equ	5		; external Interrupt 5 occured
INTF6		equ	6		; external Interrupt 6 occured
INTF7		equ	7		; external Interrupt 7 occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		equ	0		; Prescaler Reset
PSRASY		equ	1		; Prescaler Reset Timer/Counter2
TSM		equ	7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; timer/counter 0 control register A
WGM00		equ	0		; timer/counter 0 waveform generation mode
WGM01		equ	1
COM0B0		equ	4		; timer/counter 0 compare mode B
COM0B1		equ	5
COM0A0		equ	6		; timer/counter 0 compare mode A
COM0A1		equ	7
TCCR0B		port	0x25		; timer/counter 0 control register B
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM02		equ	3
FOC0B		equ	6		; timer/counter 0 force output compare match B
FOC0A		equ	7		; timer/counter 0 force output compare match A
TCNT0		port	0x26		; timer/counter 0 value
OCR0A		port	0x27		; timer/counter 0 output compare value A
OCR0B		port	0x28		; timer/counter 0 output compare value B

TCCR1A		sfr	0x80		; timer/counter 1 control register A
WGM10		equ	0		; timer/counter 1 waveform generation mode
WGM11		equ	1
COM1C0		equ	2		; timer/counter 1 compare mode C
COM1C1		equ	3
COM1B0		equ	4		; timer/counter 1 compare mode B
COM1B1		equ	5
COM1A0		equ	6		; timer/counter 1 compare mode A
COM1A1		equ	7
TCCR1B		sfr	0x81		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
WGM12		equ	3
WGM13		equ	4
ICES1		equ	6		; timer/counter 1 capture slope selection
ICNC1		equ	7		; timer/counter 1 capture noise filter
TCCR1C		sfr	0x82		; timer/counter 1 control register C
FOC1C		equ	5		; timer/counter 1 force output compare C
FOC1B		equ	6		; timer/counter 1 force output compare B
FOC1A		equ	7		; timer/counter 1 force output compare A
TCNT1L		sfr	0x84		; timer/counter 1 value LSB
TCNT1H		sfr	0x85		; timer/counter 1 value MSB
OCR1AL		sfr	0x88		; timer/counter 1 output compare value A LSB
OCR1AH		sfr	0x89		; timer/counter 1 output compare value A MSB
OCR1BL		sfr	0x8a		; timer/counter 1 output compare value B LSB
OCR1BH		sfr	0x8b		; timer/counter 1 output compare value B MSB
OCR1CL		sfr	0x8c		; timer/counter 1 output compare value C LSB
OCR1CH		sfr	0x8d		; timer/counter 1 output compare value C MSB
ICR1L		sfr	0x86		; timer/counter 1 input capture value LSB
ICR1H		sfr	0x87		; timer/counter 1 input capture value MSB

TCCR2A		sfr	0xb0		; timer/counter 2 control register A
WGM20		equ	0		; timer/counter 2 waveform generation mode
WGM21		equ	1
COM2B0		equ	4		; timer/counter 2 compare mode B
COM2B1		equ	5
COM2A0		equ	6		; timer/counter 2 compare mode A
COM2A1		equ	7
TCCR2B		sfr	0xb1		; timer/counter 2 control register B
CS20		equ	0		; timer/counter 2 prescaler setting
CS21		equ	1
CS22		equ	2
WGM22		equ	3
FOC2B		equ	6		; timer/counter 2 force output compare B
FOC2A		equ	7		; timer/counter 2 force output compare A
TCNT2		sfr	0xb2		; timer/counter 2 value
OCR2A		sfr	0xb3		; timer/counter 2 output compare value A
OCR2B		sfr	0xb4		; timer/counter 2 output compare value B

TCCR3A		sfr	0x90		; timer/counter 3 control register A
WGM30		equ	0		; timer/counter 3 waveform generation mode
WGM31		equ	1
COM3C0		equ	2		; timer/counter 3 compare mode C
COM3C1		equ	3
COM3B0		equ	4		; timer/counter 3 compare mode B
COM3B1		equ	5
COM3A0		equ	6		; timer/counter 3 compare mode A
COM3A1		equ	7
TCCR3B		sfr	0x91		; timer/counter 3 control register B
CS30		equ	0		; timer/counter 3 prescaler setting
CS31		equ	1
CS32		equ	2
WGM32		equ	3
WGM33		equ	4
ICES3		equ	6		; timer/counter 3 capture slope selection
ICNC3		equ	7		; timer/counter 3 capture noise filter
TCCR3C		sfr	0x92		; timer/counter 3 control register C
FOC3C		equ	5		; timer/counter 3 force output compare C
FOC3B		equ	6		; timer/counter 3 force output compare B
FOC3A		equ	7		; timer/counter 3 force output compare A
TCNT3L		sfr	0x94		; timer/counter 3 value LSB
TCNT3H		sfr	0x95		; timer/counter 3 value MSB
OCR3AL		sfr	0x98		; timer/counter 3 output compare value A LSB
OCR3AH		sfr	0x99		; timer/counter 3 output compare value A MSB
OCR3BL		sfr	0x9a		; timer/counter 3 output compare value B LSB
OCR3BH		sfr	0x9b		; timer/counter 3 output compare value B MSB
OCR3CL		sfr	0x9c		; timer/counter 3 output compare value C LSB
OCR3CH		sfr	0x9d		; timer/counter 3 output compare value C MSB
ICR3L		sfr	0x96		; timer/counter 3 input capture value LSB
ICR3H		sfr	0x97		; timer/counter 3 input capture value MSB

TCCR4A		sfr	0xa0		; timer/counter 4 control register A
WGM40		equ	0		; timer/counter 4 waveform generation mode
WGM41		equ	1
COM4C0		equ	2		; timer/counter 4 compare mode C
COM4C1		equ	3
COM4B0		equ	4		; timer/counter 4 compare mode B
COM4B1		equ	5
COM4A0		equ	6		; timer/counter 4 compare mode A
COM4A1		equ	7
TCCR4B		sfr	0xa1		; timer/counter 4 control register B
CS40		equ	0		; timer/counter 4 prescaler setting
CS41		equ	1
CS42		equ	2
WGM42		equ	3
WGM43		equ	4
ICES4		equ	6		; timer/counter 4 capture slope selection
ICNC4		equ	7		; timer/counter 4 capture noise filter
TCCR4C		sfr	0xa2		; timer/counter 4 control register C
FOC4C		equ	5		; timer/counter 4 force output compare C
FOC4B		equ	6		; timer/counter 4 force output compare B
FOC4A		equ	7		; timer/counter 4 force output compare A
TCNT4L		sfr	0xa4		; timer/counter 4 value LSB
TCNT4H		sfr	0xa5		; timer/counter 4 value MSB
OCR4AL		sfr	0xa8		; timer/counter 4 output compare value A LSB
OCR4AH		sfr	0xa9		; timer/counter 4 output compare value A MSB
OCR4BL		sfr	0xaa		; timer/counter 4 output compare value B LSB
OCR4BH		sfr	0xab		; timer/counter 4 output compare value B MSB
OCR4CL		sfr	0xac		; timer/counter 4 output compare value C LSB
OCR4CH		sfr	0xad		; timer/counter 4 output compare value C MSB
ICR4L		sfr	0xa6		; timer/counter 4 input capture value LSB
ICR4H		sfr	0xa7		; timer/counter 4 input capture value MSB

TCCR5A		sfr	0x120		; timer/counter 5 control register A
WGM50		equ	0		; timer/counter 5 waveform generation mode
WGM51		equ	1
COM5C0		equ	2		; timer/counter 5 compare mode C
COM5C1		equ	3
COM5B0		equ	4		; timer/counter 5 compare mode B
COM5B1		equ	5
COM5A0		equ	6		; timer/counter 5 compare mode A
COM5A1		equ	7
TCCR5B		sfr	0x121		; timer/counter 5 control register B
CS50		equ	0		; timer/counter 5 prescaler setting
CS51		equ	1
CS52		equ	2
WGM52		equ	3
WGM53		equ	4
ICES5		equ	6		; timer/counter 5 capture slope selection
ICNC5		equ	7		; timer/counter 5 capture noise filter
TCCR5C		sfr	0x122		; timer/counter 5 control register C
FOC5C		equ	5		; timer/counter 5 force output compare C
FOC5B		equ	6		; timer/counter 5 force output compare B
FOC5A		equ	7		; timer/counter 5 force output compare A
TCNT5L		sfr	0x124		; timer/counter 5 value LSB
TCNT5H		sfr	0x125		; timer/counter 5 value MSB
OCR5AL		sfr	0x128		; timer/counter 5 output compare value A LSB
OCR5AH		sfr	0x129		; timer/counter 5 output compare value A MSB
OCR5BL		sfr	0x12a		; timer/counter 5 output compare value B LSB
OCR5BH		sfr	0x12b		; timer/counter 5 output compare value B MSB
OCR5CL		sfr	0x12c		; timer/counter 5 output compare value C LSB
OCR5CH		sfr	0x12d		; timer/counter 5 output compare value C MSB
ICR5L		sfr	0x126		; timer/counter 5 input capture value LSB
ICR5H		sfr	0x127		; timer/counter 5 input capture value MSB

TIMSK0		sfr	0x6e		; timer/counter 0 interrupt mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	1		; timer/counter 0 output compare interrupt enable A
OCIE0B		equ	2		; timer/counter 0 output compare interrupt enable B
TIMSK1		sfr	0x6f		; timer/counter 1 interrupt mask register
TOIE1		equ	0		; timer/counter 1 overflow interrupt enable
OCIE1A		equ	1		; timer/counter 1 output compare interrupt enable A
OCIE1B		equ	2		; timer/counter 1 output compare interrupt enable B
ICIE1		equ	5		; timer/counter 1 input capture enable
TIMSK2		sfr	0x70		; timer/counter 2 interrupt mask register
TOIE2		equ	0		; timer/counter 2 overflow interrupt enable
OCIE2A		equ	1		; timer/counter 2 output compare interrupt enable A
OCIE2B		equ	2		; timer/counter 2 output compare interrupt enable B
TIMSK3		sfr	0x71		; timer/counter 3 interrupt mask register
TOIE3		equ	0		; timer/counter 3 overflow interrupt enable
OCIE3A		equ	1		; timer/counter 3 output compare interrupt enable A
OCIE3B		equ	2		; timer/counter 3 output compare interrupt enable B
ICIE3		equ	5		; timer/counter 3 input capture enable
TIMSK4		sfr	0x72		; timer/counter 4 interrupt mask register
TOIE4		equ	0		; timer/counter 4 overflow interrupt enable
OCIE4A		equ	1		; timer/counter 4 output compare interrupt enable A
OCIE4B		equ	2		; timer/counter 4 output compare interrupt enable B
ICIE4		equ	5		; timer/counter 4 input capture enable
TIMSK5		sfr	0x73		; timer/counter 5 interrupt mask register
TOIE5		equ	0		; timer/counter 5 overflow interrupt enable
OCIE5A		equ	1		; timer/counter 5 output compare interrupt enable A
OCIE5B		equ	2		; timer/counter 5 output compare interrupt enable B
ICIE5		equ	5		; timer/counter 5 input capture enable

TIFR0		port	0x15		; timer/counter 0 interrupt status register
TIFR1		port	0x16		; timer/counter 1 interrupt status register
TIFR2		port	0x17		; timer/counter 2 interrupt status register
TIFR3		port	0x18		; timer/counter 3 interrupt status register
TIFR4		port	0x19		; timer/counter 4 interrupt status register
TIFR5		port	0x1a		; timer/counter 5 interrupt status register

ASSR		sfr	0xb6		; Asynchronous Status Register
TCR2BUB		equ	0		; Timer/Counter Control Register 2 B Update Busy
TCR2AUB		equ	1		; Timer/Counter Control Register 2 A Update Busy
OCR2BUB		equ	2		; Output Compare Register 2 B Update Busy
OCR2AUB		equ	3		; Output Compare Register 2 A Update Busy
TCN2UB		equ	4		; Timer/Counter 2 Update Busy
AS2		equ	5		; Asynchronous Timer/Counter 2
EXCLK		equ	6		; Enable External Clock Input
EXCLKAMR	equ	7		; Enable External Clock Input for AMR

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"

;----------------------------------------------------------------------------
; U(S)ART

__USART0_SPI__	equ	1
		include	"avr/usartc0.inc"
__USART1_SPI__	equ	1
		include	"avr/usartc8.inc"

;----------------------------------------------------------------------------
; SPI

		include	"avr/spim2c.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"

;----------------------------------------------------------------------------
; A/D Converter

		include	"avr/adcm78.inc"

		; bits in ADMUX
MUX4		equ	4
		; bits in ADCSRB
MUX5		equ	3
ACCH		equ	4		; Analog Channel Change
REFOK		equ	5		; Reference Voltage OK
AVDDOK		equ	7		; AVDD Supply Voltage OK

DIDR2		sfr	0x7d		; Digital Input Disable Register 2
ADC8D		equ	0		; disable digital input on ADC8
ADC9D		equ	1		; disable digital input on ADC9
ADC10D		equ	2		; disable digital input on ADC10
ADC11D		equ	3		; disable digital input on ADC11
ADC12D		equ	4		; disable digital input on ADC12
ADC13D		equ	5		; disable digital input on ADC13
ADC14D		equ	6		; disable digital input on ADC14
ADC15D		equ	7		; disable digital input on ADC15

ADCSRC		sfr	0x77		; Control/Status Register C
ADSUT0		equ	0		; ADC Start-up Time
ADSUT1		equ	1		; 
ADSUT2		equ	2		; 
ADSUT3		equ	3		; 
ADSUT4		equ	4		; 
ADSUT5		equ	5		; 
ADTHT0		equ	6		; ADC Track-and-Hold Time
ADTHT1		equ	7		; 

BGCR		sfr	0x67		; Reference Voltage Calibration Register
BGCAL0		equ	0		; Coarse Calibration Bits
BGCAL1		equ	1
BGCAL2		equ	2
BGCAL_FINE0	equ	3		; Fine Calibration Bits
BGCAL_FINE1	equ	4
BGCAL_FINE2	equ	5
BGCAL_FINE3	equ	6

;----------------------------------------------------------------------------
; Analog Comparator

		include	"avr/acm30.inc"

;----------------------------------------------------------------------------
; 2.4 GHz Transceiver

		include	"avr/tr24.inc"

;----------------------------------------------------------------------------
; MAC Symbol Counter

		include	"avr/macsc.inc"

		restore			; re-enable listing

		endif			; __regx4rfr2inc
