		ifndef	__reg2323inc
__reg2323inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG2323.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90S2323/2343                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2END		equ     127		; End Address EEPROM
RAMSTART	equ	0x60,data	; Start Address SRAM
RAMEND		equ     0xdf,data	; End Address SRAM
FLASHEND	label   2047		; End Address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Powerdown Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

MCUSR		port	0x34		; MCU Satus Register
PORF		avrbit	MCUSR,0		; Power-On Reset Flag
EXTRF		avrbit	MCUSR,1		; Externel Reset Flag

;----------------------------------------------------------------------------
; EEPROM

		include	"ee90.inc"

EEMWE		avrbit	EECR,2		; Multiple Write Emable

;----------------------------------------------------------------------------
; GPIO

PINB		port	0x16		; Port B @ 0x16 (IO) ff.
__PORTB_BITS	equ	0x1f		; (bits 0..4)

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0

GIFR		port	0x3a		; General Interrput Flag Register
INTF0		avrbit	GIFR,6

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

		include	"wdm21.inc"

WDTOE		avrbit	WDTCR,3		; Watchdog Turn-Off Enable

		restore

		endif			; __reg2323inc
