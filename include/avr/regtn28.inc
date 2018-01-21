		ifndef	__regtn28inc
__regtn28inc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGTN28.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATtiny28                       *
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
SE		avrbit	MCUCS,5		; Sleep Mode Enable
SM		avrbit	MCUCS,4		; Sleep Mode Select
WDRF		avrbit	MCUCS,3		; Watchdog Reset Occured
EXTRF		avrbit	MCUCS,1		; External Reset Occured
PORF		avrbit	MCUCS,0		; Power-Fail Reset Occured

OSCCAL		port	0x00		; Oscillator Calibration

;----------------------------------------------------------------------------
; GPIO

PLUPB		avrbit	MCUCS,7		; Enable Pull-Up on Port B

PINA		port	0x19		; Port A Pin Status
PACR		port	0x1a		; Port A Control Register
DDA0		avrbit	PACR,0		; Bit 0 Direction
DDA1		avrbit	PACR,1		; Bit 1 Direction
PA2HC		avrbit	PACR,2		; Bit 2 Enable High Current Driver
DDA3		avrbit	PACR,3		; Bit 3 Direction
PINB		port	0x16		; Port B @ 0x16 (Input Only)
PINB_inponly	equ	1
PIND		port	0x10		; Port D @ 0x10 (IO) ff.

ICR		port	0x06		; Interrupt Control Register
LLIE		avrbit	ICR,5		; Port B Low Level Interrupt Enable

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

ISC00		avrbit	ICR,0		; Interrupt Sense Control 0
ISC01		avrbit	ICR,1
ISC10		avrbit	ICR,2		; Interrupt Sense Control 1
ISC11		avrbit	ICR,3
INT0		avrbit	ICR,6		; Enable External Interrupt 0
INT1		avrbit	ICR,7		; Enable External Interrupt 1

IFR		port	0x05		; Interrupt Flag Register
INTF0		avrbit	IFR,6		; External Interrupt 0 Occured
INTF1		avrbit	IFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x04		; Timer/Counter 0 Control Register
CS00            avrbit	TCCR0,0		; Timer/Counter 0 Clock Select
CS01            avrbit	TCCR0,1
CS02            avrbit	TCCR0,2
OOM0		avrbit	TCCR0,3		; Overflow Output Mode
OOM1		avrbit	TCCR0,4
FOV0		avrbit	TCCR0,7		; Force Overflow
TCNT0		port	0x03		; Timer/Counter 0 Value

MODCR		port	0x02		; Modulation Control Register
MCONF0		avrbit	MODCR,0		; Modulation Configuration
MCONF1		avrbit	MODCR,1
MCONF2		avrbit	MODCR,2
ONTIM0		avrbit	MODCR,3		; Modulation On-time
ONTIM1		avrbit	MODCR,4
ONTIM2		avrbit	MODCR,5
ONTIM3		avrbit	MODCR,6
ONTIM4		avrbit	MODCR,7

TOIE0		avrbit	ICR,3		; Timer/Counter 0 Overflow Interrupt Enable

;----------------------------------------------------------------------------
; Watchdog Timer

WDTCR		port	0x01		; Watchdog Control Register
WDP0		avrbit	WDTCR,0		; Prescaler
WDP1		avrbit	WDTCR,1
WDP2		avrbit	WDTCR,2
WDE		avrbit	WDTCR,3		; Enable watchdog
WDTOE		avrbit	WDTCR,4		; Enable Time-Out Interrupt

;----------------------------------------------------------------------------
; Analog Comparator

ACSR		port    0x08		; Analog Comparator Control and Status Register
ACIS0		avrbit	ACSR,0		; Interrupt-Mode
ACIS1		avrbit	ACSR,1
ACIE		avrbit	ACSR,3		; Interrupt Enable
ACI		avrbit	ACSR,4		; Interrupt Flag
ACO		avrbit	ACSR,5		; Analog Comparator Output
ACD		avrbit	ACSR,7		; Disable

		restore

		endif			; __regtn28inc
