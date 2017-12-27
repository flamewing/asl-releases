		ifndef	__regtn28inc
__regtn28inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN28.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for ATtiny28                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

RAMSTART	equ	0x60,data	; start address SRAM
RAMEND		equ     0x5f,data	; end address SRAM (i.e. NO SRAM)
FLASHEND	label   2047		; end address Flash

;----------------------------------------------------------------------------
; Chip Configuration

MCUCS		port	0x07		; MCU Control/Status Register
SE		equ	5		; Sleep Mode Enable
SM		equ	4		; Sleep Mode Select
WDRF		equ	3		; Watchdog Reset Occured
EXTRF		equ	1		; External Reset Occured
PORF		equ	0		; Power-Fail Reset Occured

OSCCAL		port	0x00		; Oscillator Calibration

;----------------------------------------------------------------------------
; GPIO

		; bits in MCUCS
PLUPB		equ	7		; Enable Pull-Up on Port B

PINA		port	0x19		; Port A Pin Status
PACR		port	0x1a		; Port A Control Register
DDA0		equ	0		; Bit 0 Direction
DDA1		equ	1		; Bit 1 Direction
PA2HC		equ	2		; Bit 2 Enable High Current Driver
DDA3		equ	3		; Bit 3 Direction
PINB		port	0x16		; Port B @ 0x16 (Input Only)
PINB_inponly	equ	1
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

		; bits in ICR
LLIE		equ	5		; Port B low level interrupt enable

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum LOWLEVEL_IO_PINS_vect	; Low-level Input on Port B
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum ANA_COMP_vect		; Analog Comparator

;----------------------------------------------------------------------------
; External Interrupts

ICR		port	0x06		; Interrupt Control Register
ISC00		equ	0		; Interrupt Sense Control 0
ISC01		equ	1
ISC10		equ	2		; Interrupt Sense Control 1
ISC11		equ	3
INT0		equ	6		; Enable External Interrupt 0
INT1		equ	7		; Enable External Interrupt 1

IFR		port	0x05		; Interrupt Flag Register
INTF0		equ	6		; External Interrupt 0 occured
INTF1		equ	7		; External Interrupt 1 occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x04		; Timer/Counter 0 Control Register
CS00            equ     0               ; Timer/Counter 0 Clock Select
CS01            equ     1
CS02            equ     2
OOM0		equ	3		; Overflow Output Mode
OOM1		equ	4
FOV0		equ	7		; Force Overflow
TCNT0		port	0x03		; Timer/Counter 0 Value

MODCR		port	0x02		; Modulation Control Register
MCONF0		equ	0		; Modulation Configuration
MCONF1		equ	1
MCONF2		equ	2
ONTIM0		equ	3		; Modulation On-time
ONTIM1		equ	4
ONTIM2		equ	5
ONTIM3		equ	6
ONTIM4		equ	7

		; bits in ICR
TOIE0		equ	3		; Timer/Counter 0 Overflow Interrupt Enable

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCR		port	0x01		; Watchdog Control Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDTOE		equ	4		; Enable Time-Out Interrupt

;----------------------------------------------------------------------------
; Analog Comparator

ACSR		port    0x08		; Analog Comparator Control and Status Register
ACIS0		equ     0		; Interrupt-Mode
ACIS1		equ     1
ACIE		equ     3               ; Interrupt Enable
ACI		equ     4               ; Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACD		equ	7		; Disable

		restore

		endif			; __regtn28inc
