		ifndef	__regtn101inc
__regtn101inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN101.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny10/11                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU Control Register
SM		equ	4		; Sleep Mode Select
SE		equ	5		; Sleep Mode Enable

MCUSR		port	0x34		; MCU Status Register
EXTRF		equ	1		; External Reset Occured
PORF		equ	0		; Power-Fail Reset Occured


		if	MOMCPUNAME="ATTINY12"
OSCCAL		 port	0x31		; Oscillator Calibration
		endif

;----------------------------------------------------------------------------
; EEPROM Access

		ifdef	E2END
EEAR		port	0x1e		; EEPROM Address Register
EEDR		port	0x1d		; EEPROM Data Register
EECR		port	0x1c		; EEPROM Control Register
EERE		equ	0		; EEPROM Read Enable
EEWE		equ	1		; EEPROM Write Enable
EEMWE		equ	2		; EEPROM Multiple Write Enable
EERIE		equ	3		; EEPROM Ready Interrupt Enable
		endif

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCR
		if	MOMCPUNAME="ATTINY12"
PUD		 equ	6		; Pull-Up Disable
		endif

PINB		port	0x16		; Port B @ 0x16 (bits 0..5)

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
ISC00		equ	0		; Interrupt Sense Control 0
ISC01		equ	1

GIMSK		port	0x3b		; Global Interrupt Mask Register
PCIE		equ	5		; Enable Pin Change Interrupt
INT0		equ	6		; Enable External Interrupt 0

GIFR		port	0x3a		; Interrupt Flag Register
PCIF		equ	5		; Pin Change Interrupt Occured
INTF0		equ	6		; External Interrupt 0 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00            equ     0               ; Timer/Counter 0 Clock Select
CS01            equ     1
CS02            equ     2
TCNT0		port	0x32		; Timer/Counter 0 Value

TIMSK		port	0x39		; Timer Mask Register
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

		; bits in WDTCR
WDTOE		equ	4		; Enable Time-Out Interrupt

;----------------------------------------------------------------------------
; Analog Comparator

		include	"avr/ac90.inc"

		; bits in ACSR
		if	MOMCPUNAME="ATTINY12"
AINBG		 equ	6		; Enable Band Gap
		endif

		restore

		endif			; __regtn101inc
