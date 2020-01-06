		ifndef	__regm406inc
__regm406inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM406.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega406                      *
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
IVCE		avrbit	MCUCR,0		; Interrupt Vector Change Enable
IVSEL		avrbit	MCUCR,1		; Interrupt Vector Select

MCUSR		port	0x34		; MCU Status Register
PORF		avrbit	MCUSR,0		; Power-On Reset Occured
EXTRF		avrbit	MCUSR,1		; External Reset Occured
BORF		avrbit	MCUSR,2		; Brown-Out Reset Occured
WDRF		avrbit	MCUSR,3		; Watchdog Reset Occured

SMCR		port	0x33		; Sleep Mode Control Register
SE		avrbit	SMCR,0		; Sleep Mode Enable
SM0		avrbit	SMCR,1		; Sleep Mode Select
SM1		avrbit	SMCR,2
SM2		avrbit	SMCR,3

PRR0		sfr	0x64		; Power Reduction Register 0
PRVADC		avrbit	PRR0,0		; Power Reduction Voltage ADC
PRTIM0		avrbit	PRR0,1		; Power Reduction Timer/Counter 0
PRTIM1		avrbit	PRR0,2		; Power Reduction Timer/Counter 1
PRTWI		avrbit	PRR0,3		; Power Reduction Two Wire Interface

FOSCCAL		sfr	0x66		; Fast Oscillator Calibration

CCSR		sfr	0xc0		; Clock Control and Status Register
XOE		avrbit	CCSR,1		; 32 kHz Crystal Oscillator Enable
ACS		avrbit	CCSR,0		; Asynchronous Clock Select

WUTCSR		sfr	0x62		; Wake-up Timer Control and Status Register
WUTIF		avrbit	WUTCSR,7	; Wake-up Timer Interrupt Flag
WUTIE		avrbit	WUTCSR,6	; Wake-up Timer Interrupt Enable
WUTCF		avrbit	WUTCSR,5	; Wake-up Timer Calibration Flag
WUTR		avrbit	WUTCSR,4	; Wake-up Timer Reset
WUTE		avrbit	WUTCSR,3	; Wake-up Timer Enable
WUTP2		avrbit	WUTCSR,2	; Wake-up Timer Prescaler 2, 1, and 0
WUTP1		avrbit	WUTCSR,1
WUTP0		avrbit	WUTCSR,0

BGCCR		sfr	0xd0		; Bandgap Calibration C Register
BGEN		avrbit	BGCCR,7		; reserved
BGCC5		avrbit	BGCCR,5		; BG Calibration of PTAT Current
BGCC4		avrbit	BGCCR,4
BGCC3		avrbit	BGCCR,3
BGCC2		avrbit	BGCCR,2
BGCC1		avrbit	BGCCR,1
BGCC0		avrbit	BGCCR,0

BGCRR		sfr	0xd1		; Bandgap Calibration R Register
BGCR7		avrbit	BGCRR,7		; BG Calibration of Resistor Ladder
BGCR6		avrbit	BGCRR,6
BGCR5		avrbit	BGCRR,5
BGCR4		avrbit	BGCRR,4
BGCR3		avrbit	BGCRR,3
BGCR2		avrbit	BGCRR,2
BGCR1		avrbit	BGCRR,1
BGCR0		avrbit	BGCRR,0

;----------------------------------------------------------------------------
; EEPROM/Program Memory Access

		include	"eem2.inc"
		include	"spmcsr37.inc"

EEPM0		avrbit	EECR,4		; EEPROM Programming Mode
EEPM1		avrbit	EECR,5

SIGRD		avrbit	SPMCSR,5	; Signature Row Read

;----------------------------------------------------------------------------
; JTAG

JTD		avrbit	MCUCR,7		; JTAG Disable

JTRF		avrbit	MCUSR,4		; JTAG Reset Occured

OCDR		port	0x31		; On-Chip Debug Register

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

GPIOR0		port	0x1e		; General Purpose I/O Registers
GPIOR1		port	0x2a
GPIOR2		port	0x2b

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 2,code
		enum	 BPINT_vect=2		; battery protection Interrupt
		nextenum INT0_vect		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum INT2_vect		; External Interrupt Request 2
		nextenum INT3_vect		; External Interrupt Request 3
		nextenum PCINT0_vect		; Pin Change Interrupt Request 0
		nextenum PCINT1_vect		; Pin Change Interrupt Request 1
		nextenum WDT_vect		; Watchdog Time-Out Interrupt
		nextenum WAKE_UP_vect		; Wake-Up Timer Overflow
		nextenum TIMER1_COMP_vect	; Timer/Counter 1 Compare Match
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_COMPA_vect	; Timer/Counter 0 Compare Match A
		nextenum TIMER0_COMPB_vect	; Timer/Counter 0 Compare Match B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum TWI_BUS_CD_vect	; Two-Wire Bus Connect/Disconnect
		nextenum TWI_vect		; 2-Wire Transfer Complete
		nextenum VADC_vect		; Voltage ADC Conversion Complete
		nextenum CCADC_CONV_vect	; CC-ADC Instantaneous Current Conversion Complete
		nextenum CCADC_REG_CUR_vect	; CC-ADC Regular Current
		nextenum CCADC_ACC_vect		; CC-ADC Accumulate Current Conversion Complete
		nextenum EE_READY_vect		; EEPROM Ready
		nextenum SPM_READY_vect		; Store Program Memory Ready

;----------------------------------------------------------------------------
; External Interrupts

EICRA		sfr	0x69		; External Interrupt Control Register A
ISC00		avrbit	EICRA,0		; External Interrupt 0 Sense Control
ISC01		avrbit	EICRA,1
ISC10		avrbit	EICRA,2		; External Interrupt 1 Sense Control
ISC11		avrbit	EICRA,3
ISC20		avrbit	EICRA,4		; External Interrupt 2 Sense Control
ISC21		avrbit	EICRA,5
ISC30		avrbit	EICRA,6		; External Interrupt 3 Sense Control
ISC31		avrbit	EICRA,7

EIMSK		port	0x1d		; External Interrupt Mask Register
INT0		avrbit	EIMSK,0		; Enable External Interrupt 0
INT1		avrbit	EIMSK,1		; Enable External Interrupt 1
INT2		avrbit	EIMSK,2		; Enable External Interrupt 2
INT3		avrbit	EIMSK,3		; Enable External Interrupt 3

EIFR		port	0x1c		; External Interrupt Flag Register
INTF0		avrbit	EIFR,0		; External Interrupt 0 Occured
INTF1		avrbit	EIFR,1		; External Interrupt 1 Occured
INTF2		avrbit	EIFR,2		; External Interrupt 2 Occured
INTF3		avrbit	EIFR,3		; External Interrupt 3 Occured

;----------------------------------------------------------------------------
; Timers

GTCCR		port	0x23		; General Timer/Counter Control Register
PSRSYNC		avrbit	GTCCR,0		; Prescaler Reset
TSM		avrbit	GTCCR,7		; Timer/Counter Synchronization Mode

TCCR0A		port	0x24		; Timer/Counter 0 Control Register A
WGM00		avrbit	TCCR0A,0	; Timer/Counter 0 Waveform Generation Mode
WGM01		avrbit	TCCR0A,1
COM0B0		avrbit	TCCR0A,4	; Timer/Counter 0 Compare B Mode
COM0B1		avrbit	TCCR0A,5
COM0A0		avrbit	TCCR0A,6	; Timer/Counter 0 Compare A Mode
COM0A1		avrbit	TCCR0A,7
TCCR0B		port	0x25		; Timer/Counter 0 Control Register B
CS00		avrbit	TCCR0B,0	; Timer/Counter 0 Clock Select
CS01		avrbit	TCCR0B,1
CS02		avrbit	TCCR0B,2
WGM02		avrbit	TCCR0B,3	; Timer/Counter 0 Waveform Generation Mode
FOC0B		avrbit	TCCR0B,6	; Timer/Counter 0 Force Output Compare B
FOC0A		avrbit	TCCR0B,7	; Timer/Counter 0 Force Output Compare A
TCNT0		port	0x26		; Timer/Counter 0
OCR0A		port	0x27		; Timer/Counter 0 Output Compare Value A
OCR0B		port	0x28		; Timer/Counter 0 Output Compare Value B

TCCR1B		sfr	0x81		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter 1 Prescaler Setting
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Timer/Counter 1 Clear on Match
TCNT1L		sfr	0x84		; Timer/Counter 1 Value LSB
TCNT1H		sfr	0x85		; Timer/Counter 1 Value MSB
OCR1AL		sfr	0x88		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		sfr	0x89		; Timer/Counter 1 Output Compare Value A MSB

TIMSK0		sfr	0x6e		; Timer/Counter 0 Interrupt Mask Register
TOIE0		avrbit	TIMSK0,0	; Timer/Counter 0 Overflow Interrupt Enable
OCIE0A		avrbit	TIMSK0,1	; Timer/Counter 0 Output Compare Interrupt Enable A
OCIE0B		avrbit	TIMSK0,2	; Timer/Counter 0 Output Compare Interrupt Enable B
TIMSK1		sfr	0x6f		; Timer/Counter 1 Interrupt Mask Register
TOIE1		avrbit	TIMSK1,0	; Timer/Counter 1 Overflow Interrupt Enable
OCIE1A		avrbit	TIMSK1,1	; Timer/Counter 1 Output Compare Interrupt Enable A
ICIE1		avrbit	TIMSK1,5	; Timer/Counter 1 Input Capture Event

TIFR0		port	0x15		; Timer/Counter 0 Interrupt Status Register 
TIFR1		port	0x16		; Timer/Counter 1 Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdme.inc"

;----------------------------------------------------------------------------
; TWI

		include	"twimb8.inc"

TWBCSR		sfr	0xbe		; TWI Bus Control and Status Register
TWBCIF		avrbit	TWBCSR,7	; TWI Bus Connect/Disconnect Interrupt Flag
TWBCIE		avrbit	TWBCSR,6	; TWI Bus Connect/Disconnect Interrupt Enable
TWBDT1		avrbit	TWBCSR,2	; TWI Bus Disconnect Time-Out Period
TWBDT0		avrbit	TWBCSR,1
TWBCIP		avrbit	TWBCSR,0	; TWI Bus Connect/Disconnect Interrupt Polarity

;----------------------------------------------------------------------------
; Coulomb Counter

CADCSRA		sfr	0xe4		; CC-ADC Control and Status Register A
CADSE		avrbit	CADCSRA,0	; CC-ADC Current Sampling Enable
CADIS0		avrbit	CADCSRA,1	; CC-ADC Current Sampling Interval
CADIS1		avrbit	CADCSRA,2
CADAS0		avrbit	CADCSRA,3	; CC-ADC Accumulate Current Select
CADAS1		avrbit	CADCSRA,4
CADUB		avrbit	CADCSRA,5	; CADUB: CC-ADC Update Busy
CADEN		avrbit	CADCSRA,7	; CC-ADC Enable

CADCSRB		sfr	0xe5		; CC-ADC Control and Status Register B
CADACIE		avrbit	CADCSRB,6	; CC-ADC Accumulate Current Interrupt Enable
CADRCIE		avrbit	CADCSRB,5	; CC-ADC Regular Current Interrupt Enable
CADICIE		avrbit	CADCSRB,4	; CC-ADC Instantaneous Current Interrupt Enable
CADACIF		avrbit	CADCSRB,2	; CC-ADC Accumulate Current Interrupt Flag
CADRCIF		avrbit	CADCSRB,1	; CC-ADC Regular Current Interrupt Flag
CADICIF		avrbit	CADCSRB,0	; CC-ADC Instantaneous Current Interrupt Flag

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
VADMUX0		avrbit	VADMUX,0	; Channel Selection Bits
VADMUX1		avrbit	VADMUX,1
VADMUX2		avrbit	VADMUX,2
VADMUX3		avrbit	VADMUX,3

VADCSR		sfr	0x7a		; Control and Status Register
VADCCIE		avrbit	VADCSR,0	; V-ADC Conversion Complete Interrupt Enable
VADCCIF		avrbit	VADCSR,1	; V-ADC Conversion Complete Interrupt Flag
VADSC		avrbit	VADCSR,2	; Voltage ADC Start Conversion
VADEN		avrbit	VADCSR,3	; V-ADC Enable

VADCL		sfr	0x78		; V-ADC Data Register
VADCH		sfr	0x79

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
VADC0D		avrbit	DIDR0,0		; V-ADC0 Digital Input Disable
VADC1D		avrbit	DIDR0,1		; V-ADC1 Digital Input Disable
VADC2D		avrbit	DIDR0,2		; V-ADC2 Digital Input Disable
VADC3D		avrbit	DIDR0,3		; V-ADC3 Digital Input Disable

;----------------------------------------------------------------------------
; FET Control

FCSR		sfr	0xf0		; FET Control and Status Register
PWMOC		avrbit	FCSR,5		; Pulse Width Modulation of OC Output
PWMOPC		avrbit	FCSR,4		; Pulse Width Modulation of OPC Output
CPS		avrbit	FCSR,3		; Current Protection Status
DFE		avrbit	FCSR,2		; Discharge FET Enable
CFE		avrbit	FCSR,1		; Charge FET Enable
PFD		avrbit	FCSR,0		; Precharge FET Disable

;----------------------------------------------------------------------------
; Cell Balancing

CBCR		sfr	0xf1		; Cell Balancing Control Register
CBE4		avrbit	CBCR,3		; Cell Balancing Enable 4
CBE3		avrbit	CBCR,2		; Cell Balancing Enable 3
CBE2		avrbit	CBCR,1		; Cell Balancing Enable 2
CBE1		avrbit	CBCR,0		; Cell Balancing Enable 1

;----------------------------------------------------------------------------
; Battery Protection

BPPLR		sfr	0xf8		; Battery Protection Parameter Lock Register
BPPLE		avrbit	BPPLR,1		; Battery Protection Parameter Lock Enable
BPPL		avrbit	BPPLR,0		; Battery Protection Parameter Lock

BPCR		sfr	0xf7		; Battery Protection Control Register
DUVD		avrbit	BPCR,3		; Deep Under-Voltage Protection Disable
SCD		avrbit	BPCR,2		; Short Circuit Protection Disabled
DCD		avrbit	BPCR,1		; Discharge Over-Current Protection Disable
CCD		avrbit	BPCR,0		; Charge Over-Current Protection Disable

CBPTR		sfr	0xf6		; Current Battery Protection Timing Register
SCPT3		avrbit	CBPTR,7		; Short-Circuit Protection Timing
SCPT2		avrbit	CBPTR,6
SCPT1		avrbit	CBPTR,5
SCPT0		avrbit	CBPTR,4
OCPT3		avrbit	CBPTR,3		; Over-Current Protection Timing
OCPT2		avrbit	CBPTR,2
OCPT1		avrbit	CBPTR,1
OCPT0		avrbit	CBPTR,0

BPOCD		sfr	0xf5		; Battery Protection Over-Current Detection Level Register
DCDL3		avrbit	BPOCD,7		; Discharge Over-Current Detection Level
DCDL2		avrbit	BPOCD,6
DCDL1		avrbit	BPOCD,5
DCDL0		avrbit	BPOCD,4
CCDL3		avrbit	BPOCD,3		; Charge Over-Current Detection Level
CCDL2		avrbit	BPOCD,2
CCDL1		avrbit	BPOCD,1
CCDL0		avrbit	BPOCD,0

BPSCD		sfr	0xf4		; Battery Protection Short-Circuit Detection Level Register
SCDL3		avrbit	BPSCD,3		; Short-Circuit Detection Level
SCDL2		avrbit	BPSCD,2
SCDL1		avrbit	BPSCD,1
SCDL0		avrbit	BPSCD,0

BPDUV		sfr	0xf3		; Battery Protection Deep Under Voltage Register
DUVT1		avrbit	BPDUV,5		; Deep Under-Voltage Timing
DUVT0		avrbit	BPDUV,4
DUDL3		avrbit	BPDUV,3		; Deep Under-Voltage Detection Level
DUDL2		avrbit	BPDUV,2
DUDL1		avrbit	BPDUV,1
DUDL0		avrbit	BPDUV,0

BPIR		sfr	0xf2		; Battery Protection Interrupt Register
DUVIF		avrbit	BPIR,7		; Deep Under-Voltage Early Warning Interrupt Flag
COCIF		avrbit	BPIR,6		; Charge Over-Current Protection Activated Interrupt Flag
DOCIF		avrbit	BPIR,5		; Discharge Over-Current Protection Activated Interrupt Flag
SCIF		avrbit	BPIR,4		; Short-Circuit Protection Activated Interrupt Flag
DUVIE		avrbit	BPIR,3		; Deep Under-Voltage Early Warning Interrupt Enable
COCIE		avrbit	BPIR,2		; Charge Over-Current Protection Activated Interrupt Enable
DOCIE		avrbit	BPIR,1		; Discharge Over-Current Protection Activated Interrupt Enable
SCIE		avrbit	BPIR,0		; Short-Circuit Protection Activated Interrupt Enable

		restore			; re-enable listing

		endif			; __regm406inc
