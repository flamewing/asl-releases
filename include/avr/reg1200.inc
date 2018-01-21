		ifndef	__reg1200inc
__reg1200inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG1200.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S1200                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; End Address EEPROM
RAMSTART	equ	0x60,data	; Start Address SRAM
RAMEND		equ     95,data		; End Address SRAM (i.e. no SRAM!)
FLASHEND	label   1023		; End Address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Powerdown Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
__PORTD_BITS	equ	0x7f		; (bits 0..6)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	INT0_vect=1		; External Interrupt Request 0
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		restore

		endif			; __reg1200inc
