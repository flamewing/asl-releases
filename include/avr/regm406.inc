		ifndef	__regm406inc
__regm406inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM406.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATmega406                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END           equ	511
RAMSTART	equ	0x100,data
RAMEND		equ	0x8ff,data
FLASHEND	label	0x9fff

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
PRVADC		equ	0		; Power Reduction Voltage ADC
PRTIM0		equ	1		; Power Reduction Timer/Counter 0
PRTIM1		equ	2		; Power Reduction Timer/Counter 1
PRTWI		equ	3		; Power Reduction Two Wire Interface

FOSCCAL		sfr	0x66		; fast oscillator calibration

CCSR		sfr	0xc0		; Clock Control and Status Register
XOE		equ	1		; 32 kHz Crystal Oscillator Enable
ACS		equ	0		; Asynchronous Clock Select

WUTCSR		sfr	0x62		; Wake-up Timer Control and Status Register
WUTIF		equ	7		; Wake-up Timer Interrupt Flag
WUTIE		equ	6		; Wake-up Timer Interrupt Enable
WUTCF		equ	5		; Wake-up Timer Calibration Flag
WUTR		equ	4		; Wake-up Timer Reset
WUTE		equ	3		; Wake-up Timer Enable
WUTP2		equ	2		; Wake-up Timer Prescaler 2, 1, and 0
WUTP1		equ	1
WUTP0		equ	0

BGCCR		sfr	0xd0		; Bandgap Calibration C Register
BGEN		equ	7		; reserved
BGCC5		equ	5		; BG Calibration of PTAT Current
BGCC4		equ	4
BGCC3		equ	3
BGCC2		equ	2
BGCC1		equ	1
BGCC0		equ	0

BGCRR		sfr	0xd1		; Bandgap Calibration R Register
BGCR7		equ	7		; BG Calibration of Resistor ladder
BGCR6		equ	6
BGCR5		equ	5
BGCR4		equ	4
BGCR3		equ	3
BGCR2		equ	2
BGCR1		equ	1
BGCR0		equ	0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"avr/eem2.inc"
		include	"avr/spmcsr37.inc"

		; additional bits in EECR
EEPM0		equ	4		; EEPROM Programming Mode
EEPM1		equ	5

		; additional bits in SPMCSR
SIGRD		equ	5		; signature row read

;----------------------------------------------------------------------------
; JTAG

		; bits in MCUCR
JTD		equ	7		; JTAG disable

		; bits in MCUSR
JTRF		equ	4		; JTAG reset occured

OCDR		port	0x31		; On-chip Debug Register

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x00		; Port A @ 0x00 (IO) ff.
PINB		port	0x03		; Port B @ 0x03 (IO) ff.
PORTC		port	0x08		; Port C @ 0x08 (Inp only)
PIND		port	0x09		; Port D @ 0x09 (IO) ff.

PCMSK0		sfr	0x6b		; Pin Change Mask Register 0
PCMSK1		sfr	0x6c		; Pin Change Mask Register 1
PCICR		sfr	0x68		; Pin Change Interrupt Control Register
PCIFR		port	0x1b		; Pin Change Interrupt Flag Register

GPIOR0		port	0x1e		; general purpose registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 BPINT_vect=2		; battery protection interrupt
		nextenum INT0_vect		; external interrupt request 0
		nextenum INT1_vect		; external interrupt request 1
		nextenum INT2_vect		; external interrupt request 2
		nextenum INT3_vect		; external interrupt request 3
		nextenum PCINT0_vect		; pin change interrupt request 0
		nextenum PCINT1_vect		; pin change interrupt request 1
		nextenum WDT_vect		; watchdog time-out interrupt
		nextenum WAKE_UP_vect		; wake-up timer overflow
		nextenum TIMER1_COMP_vect	; timer/counter 1 compare match
		nextenum TIMER1_OVF_vect	; timer/counter 1 overflow
		nextenum TIMER0_COMPA_vect	; timer/counter 0 compare match A
		nextenum TIMER0_COMPB_vect	; timer/counter 0 compare match B
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum TWI_BUS_CD_vect	; Two-wire Bus Connect/Disconnect
		nextenum TWI_vect		; 2-wire transfer complete
		nextenum VADC_vect		; Voltage ADC Conversion Complete
		nextenum CCADC_CONV_vect	; CC-ADC Instantaneous Current Conversion Complete
		nextenum CCADC_REG_CUR_vect	; CC-ADC Regular Current
		nextenum CCADC_ACC_vect		; CC-ADC Accumulate Current Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum SPM_READY_vect		; Store Program Memory Ready

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

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		equ	0		; enable external interrupt 0
INT1		equ	1		; enable external interrupt 1
INT2		equ	2		; enable external interrupt 2
INT3		equ	3		; enable external interrupt 3

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		equ	0		; external Interrupt 0 occured
INTF1		equ	1		; external Interrupt 1 occured
INTF2		equ	2		; external Interrupt 2 occured
INTF3		equ	3		; external Interrupt 3 occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		equ	0		; Prescaler Reset
TSM		equ	7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; timer/counter 0 control register A
WGM00		equ	0		; timer/counter 0 waveform generation mode
WGM01		equ	1
COM0B0		equ	4		; timer/counter 0 compare B mode
COM0B1		equ	5
COM0A0		equ	6		; timer/counter 0 compare A mode
COM0A1		equ	7
TCCR0B		port	0x25		; timer/counter 0 control register B
CS00		equ	0		; timer/counter 0 clock select
CS01		equ	1
CS02		equ	2
WGM02		equ	3		; timer/counter 0 waveform generation mode
FOC0B		equ	6		; timer/counter 0 force output compare B
FOC0A		equ	7		; timer/counter 0 force output compare A
TCNT0		port	0x26		; timer/counter 0
OCR0A		port	0x27		; timer/counter 0 output compare value A
OCR0B		port	0x28		; timer/counter 0 output compare value B

TCCR1B		sfr	0x81		; timer/counter 1 control register B
CS10		equ	0		; timer/counter 1 prescaler setting
CS11		equ	1
CS12		equ	2
CTC1		equ	3		; timer/counter 1 clear on match
TCNT1L		sfr	0x84		; timer/counter 1 value LSB
TCNT1H		sfr	0x85		; timer/counter 1 value MSB
OCR1AL		sfr	0x88		; timer/counter 1 output compare value A LSB
OCR1AH		sfr	0x89		; timer/counter 1 output compare value A MSB

TIMSK0		sfr	0x6e		; timer/counter 0 interrupt mask register
TOIE0		equ	0		; timer/counter 0 overflow interrupt enable
OCIE0A		equ	1		; timer/counter 0 output compare interrupt enable A
OCIE0B		equ	2		; timer/counter 0 output compare interrupt enable B
TIMSK1		sfr	0x6f		; timer/counter 1 interrupt mask register
TOIE1		equ	0		; timer/counter 1 overflow interrupt enable
OCIE1A		equ	1		; timer/counter 1 output compare interrupt enable A
ICIE1		equ	5		; timer/counter 1 input capture event

TIFR0		port	0x15		; timer/counter 0 interrupt status register 
TIFR1		port	0x16		; timer/counter 1 interrupt status register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdme.inc"

;----------------------------------------------------------------------------
; TWI

		include	"avr/twimb8.inc"

TWBCSR		sfr	0xbe		; TWI Bus Control and Status Register
TWBCIF		equ	7		; TWI Bus Connect/Disconnect Interrupt Flag
TWBCIE		equ	6		; TWI Bus Connect/Disconnect Interrupt Enable
TWBDT1		equ	2		; TWI Bus Disconnect Time-out Period
TWBDT0		equ	1
TWBCIP		equ	0		; TWI Bus Connect/Disconnect Interrupt Polarity

;----------------------------------------------------------------------------
; Coulomb Counter

CADCSRA		sfr	0xe4		; CC-ADC Control and Status Register A
CADSE		equ	0		; CC-ADC Current Sampling Enable
CADIS0		equ	1		; CC-ADC Current Sampling Interval
CADIS1		equ	2
CADAS0		equ	3		; CC-ADC Accumulate Current Select
CADAS1		equ	4
CADUB		equ	5		; CADUB: CC-ADC Update Busy
CADEN		equ	7		; CC-ADC Enable

CADCSRB		sfr	0xe5		; CC-ADC Control and Status Register B
CADACIE		equ	6		; CC-ADC Accumulate Current Interrupt Enable
CADRCIE		equ	5		; CC-ADC Regular Current Interrupt Enable
CADICIE		equ	4		; CC-ADC Instantaneous Current Interrupt Enable
CADACIF		equ	2		; CC-ADC Accumulate Current Interrupt Flag
CADRCIF		equ	1		; CC-ADC Regular Current Interrupt Flag
CADICIF		equ	0		; CC-ADC Instantaneous Current Interrupt Flag

CADICL		sfr	0xe8		; CC-ADC Instantaneous Current
CADICH		sfr	0xe9

CADAC0		sfr	0xe0		; CC-ADC Accumulate Current
CADAC1		sfr	0xe1
CADAC2		sfr	0xe2
CADAC3		sfr	0xe3

CADRCC		sfr	0xe6		; CC-ADC Regular Charge Current

CADRDC		sfr	0xe7		; CC-ADC Regular Discharge Current

;----------------------------------------------------------------------------
; Voltage A/D Converter

VADMUX		sfr	0x7c		; Multiplexer Selection Register
VADMUX0		equ	0		; Channel Selection Bits
VADMUX1		equ	1
VADMUX2		equ	2
VADMUX3		equ	3

VADCSR		sfr	0x7a		; Control and Status Register
VADCCIE		equ	0		; V-ADC Conversion Complete Interrupt Enable
VADCCIF		equ	1		; V-ADC Conversion Complete Interrupt Flag
VADSC		equ	2		; Voltage ADC Start Conversion
VADEN		equ	3		; V-ADC Enable

VADCL		sfr	0x78		; V-ADC Data Register
VADCH		sfr	0x79

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
VADC0D		equ	0		; V-ADC0 Digital Input Disable
VADC1D		equ	1		; V-ADC1 Digital Input Disable
VADC2D		equ	2		; V-ADC2 Digital Input Disable
VADC3D		equ	3		; V-ADC3 Digital Input Disable

;----------------------------------------------------------------------------
; FET Control

FCSR		sfr	0xf0		; FET Control and Status Register
PWMOC		equ	5		; Pulse Width Modulation of OC output
PWMOPC		equ	4		; Pulse Width Modulation of OPC output
CPS		equ	3		; Current Protection Status
DFE		equ	2		; Discharge FET Enable
CFE		equ	1		; Charge FET Enable
PFD		equ	0		; Precharge FET Disable

;----------------------------------------------------------------------------
; Cell Balancing

CBCR		sfr	0xf1		; Cell Balancing Control Register
CBE4		equ	3		; Cell Balancing Enable 4
CBE3		equ	2		; Cell Balancing Enable 3
CBE2		equ	1		; Cell Balancing Enable 2
CBE1		equ	0		; Cell Balancing Enable 1

;----------------------------------------------------------------------------
; Battery Protection

BPPLR		sfr	0xf8		; Battery Protection Parameter Lock Register
BPPLE		equ	1		; Battery Protection Parameter Lock Enable
BPPL		equ	0		; Battery Protection Parameter Lock

BPCR		sfr	0xf7		; Battery Protection Control Register
DUVD		equ	3		; Deep Under-voltage Protection Disable
SCD		equ	2		; Short Circuit Protection Disabled
DCD		equ	1		; Discharge Over-current Protection Disable
CCD		equ	0		; Charge Over-current Protection Disable

CBPTR		sfr	0xf6		; Current Battery Protection Timing Register
SCPT3		equ	7		; Short-circuit Protection Timing
SCPT2		equ	6
SCPT1		equ	5
SCPT0		equ	4
OCPT3		equ	3		; Over-current Protection Timing
OCPT2		equ	2
OCPT1		equ	1
OCPT0		equ	0

BPOCD		sfr	0xf5		; Battery Protection Over-current Detection Level Register
DCDL3		equ	7		; Discharge Over-current Detection Level
DCDL2		equ	6
DCDL1		equ	5
DCDL0		equ	4
CCDL3		equ	3		; Charge Over-current Detection Level
CCDL2		equ	2
CCDL1		equ	1
CCDL0		equ	0

BPSCD		sfr	0xf4		; Battery Protection Short-circuit Detection Level Register
SCDL3		equ	3		; Short-circuit Detection Level
SCDL2		equ	2
SCDL1		equ	1
SCDL0		equ	0

BPDUV		sfr	0xf3		; Battery Protection Deep Under Voltage Register
DUVT1		equ	5		; Deep Under-voltage Timing
DUVT0		equ	4
DUDL3		equ	3		; Deep Under-voltage Detection Level
DUDL2		equ	2
DUDL1		equ	1
DUDL0		equ	0

BPIR		sfr	0xf2		; Battery Protection Interrupt Register
DUVIF		equ	7		; Deep Under-voltage Early Warning Interrupt Flag
COCIF		equ	6		; Charge Over-current Protection Activated Interrupt Flag
DOCIF		equ	5		; Discharge Over-current Protection Activated Interrupt Flag
SCIF		equ	4		; Short-circuit Protection Activated Interrupt Flag
DUVIE		equ	3		; Deep Under-voltage Early Warning Interrupt Enable
COCIE		equ	2		; Charge Over-current Protection Activated Interrupt Enable
DOCIE		equ	1		; Discharge Over-current Protection Activated Interrupt Enable
SCIE		equ	0		; Short-circuit Protection Activated Interrupt Enable

		restore			; re-enable listing

		endif			; __regm406inc
