		ifndef	__regtn112inc
__regtn112inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN112.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny11/12                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU Control Register
SM		avrbit	MCUCR,4		; Sleep Mode Select
SE		avrbit	MCUCR,5		; Sleep Mode Enable

MCUSR		port	0x34		; MCU Status Register
EXTRF		avrbit	MCUSR,1		; External Reset Occured
PORF		avrbit	MCUSR,0		; Power-Fail Reset Occured


		if	MOMCPUNAME="ATTINY12"
OSCCAL		 port	0x31		; Oscillator Calibration
		endif

;----------------------------------------------------------------------------
; EEPROM Access

		ifdef	E2END
EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EERE		avrbit	EECR,0		; EEPROM Read Enable
EEWE		avrbit	EECR,1		; EEPROM Write Enable
EEMWE		avrbit	EECR,2		; EEPROM Multiple Write Enable
EERIE		avrbit	EECR,3		; EEPROM Ready Interrupt Enable
		endif

;----------------------------------------------------------------------------
; GPIO

		if	MOMCPUNAME="ATTINY12"
PUD		 avrbit	MCUCR,6		; Pull-Up Disable
		endif

PINB		port	0x16		; Port B @ 0x16
__PORTB_BITS	equ	0x3f		; (bits 0..5)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum IO_PINS_vect		; Low-level Input on Port B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		ifdef	 E2END
		 nextenum EE_RDY_vect		; EEPROM Ready
		endif
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		avrbit	MCUCR,0		; Interrupt Sense Control 0
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; Global Interrupt Mask Register
PCIE		avrbit	GIMSK,5		; Enable Pin Change Interrupt
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0

GIFR		port	0x3a		; Interrupt Flag Register
PCIF		avrbit	GIFR,5		; Pin Change Interrupt Occured
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00            avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01            avrbit	TCCR0,1
CS02            avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TIMSK		port	0x39		; Timer Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"

WDTOE		avrbit	WDTCR,4		; Enable Time-Out Interrupt

;----------------------------------------------------------------------------
; Analog Comparator

		include	"ac90.inc"

		if	MOMCPUNAME="ATTINY12"
AINBG		 avrbit	ACSR,6		; Enable Band Gap
		endif

		restore

		endif			; __regtn112inc
