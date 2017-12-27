		ifndef	__reg2323inc
__reg2323inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG2323.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for AT90S2323/2343                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; end address EEPROM
RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0xdf,data	; end address SRAM
FLASHEND	label   2047		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		equ	4		; choose Idle/Powerdown mode
SE		equ	5		; enable sleep mode

MCUSR		port	0x34		; MCU Satus Register
PORF		equ	0		; Power-On Reset Flag
EXTRF		equ	1		; Externel Reset Flag

;----------------------------------------------------------------------------
; EEPROM

		include	"avr/ee90.inc"

		; bits in EECR
EEMWE		equ	2		; Multiple Write Emable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff. (bits 0..4)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	INT0_vect=1		; external interrupt request 0
		nextenum TIMER0_OVF_vect	; timer/counter 0 overflow

;----------------------------------------------------------------------------
; External Interrupts

		; bits in MCUCR
ISC00		equ	0		; external interrupt 0 sense control
ISC01		equ	1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		equ	6		;  enable external Interrupt 0

GIFR		port	0x3a		; General Interrput Flag Register
INTF0		equ	6

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		equ	0		; Clock Select
CS01		equ	1
CS02		equ	2
TCNT0		port	0x32		; Timer/Counter 0 Value

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		equ	1		; Timer/Counter 0 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Status Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"avr/wdm21.inc"

		; bits in WDTCR
WDTOE		equ	3		; Watchdog Turn-Off Enable

		restore

		endif			; __reg2323inc
