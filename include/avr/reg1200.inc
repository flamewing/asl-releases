		ifndef	__reg1200inc
__reg1200inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG1200.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90S1200                      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     63		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     95,data		; end address SRAM (i.e. no SRAM!)
FLASHEND	label   1023		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		equ	4		; choose Idle/Powerdown mode
SE		equ	5		; enable sleep mode

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	INT0_vect=1		; external interrupt request 0
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow
		nextenum ANA_COMP_vect		; analog comparator

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		;  enable external Interrupt 0

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; timer/counter 0 control register
CS00		equ	0		; clock select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; timer/counter 0 value

TIMSK		port	0x39		; timer interrupt mask register
TOIE0		equ	1		; timer/counter 0 overflow interrupt enable

TIFR		port	0x38		; timer interrupt status register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

;----------------------------------------------------------------------------
; Analog Comparator

		include "avr/ac90.inc"

		restore

		endif			; __reg1200inc
