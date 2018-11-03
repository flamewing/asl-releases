		ifndef	__mcf5204inc		; avoid multiple inclusion
__mcf5204inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5204.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5204                  *
;*                                                                          *
;****************************************************************************

		ifndef		MBAR
MBAR		equ		$fc000000
		warning		"MBAR not set - assume default value $fc000000"
		endif

;----------------------------------------------------------------------------
; System Integration Module

SIMR		equ		MBAR+$003	; SIM Configuration Register (8b)
FRZ0		cfbit		SIMR,6		;  Freeze Bus Monitor Enable
FRZ1		cfbit		SIMR,7		;  Freeze Software Watchdog Timer Enable

__deficr	macro		name,addr
name		equ		addr
AVEC		cfbit		name,0,7	;  Autovector Enable
IL		cffield		name,2,3	;  Interrupt Level
IP		cffield		name,0,2	;  Interrupt Priority
		endm

		__deficr	ICR_E0,MBAR+$014; Interrupt Control Register Ext0 (8b)
		__deficr	ICR_E1,MBAR+$015; Interrupt Control Register Ext1 (8b)
		__deficr	ICR_E2,MBAR+$016; Interrupt Control Register Ext2 (8b)
		__deficr	ICR_E3,MBAR+$017; Interrupt Control Register Ext3 (8b)
		__deficr	ICR_SW,MBAR+$01b; Interrupt Control Register SWT (8b)
		__deficr	ICR_T1,MBAR+$01c; Interrupt Control Register T1 (8b)
		__deficr	ICR_T2,MBAR+$01d; Interrupt Control Register T2 (8b)
		__deficr	ICR_U1,MBAR+$01f; Interrupt Control Register UART1 (8b)

__defirq	macro		name,addr
name		equ		addr
UART		cfbit		name,12
T2		cfbit		name,10
T1		cfbit		name,9
SWT		cfbit		name,8
IRQ3		cfbit		name,4
IRQ2		cfbit		name,3
IRQ1		cfbit		name,2
IRQ0		cfbit		name,1
		endm
		__defirq	IMR,MBAR+$034	; Interrupt Mask Register (32b)
		__defirq	IPR,MBAR+$038	; Interrupt Pending Register (32b)

RSR		equ		MBAR+$040	; Reset Status Register (8b)
HRST		cfbit		RSR,7		;  Hard Reset or System Reset
SWTR		cfbit		RAR,5		;  Software Watchdog Timer Reset.

SYPCR		equ		MBAR+$041	; System Protection Control Register (8b)
SWE		cfbit		SYPCR,7		;  Software Watchdog Enable
SWRI		cfbit		SYPCR,6		;  Software Watchdog Reset/Interrupt Select
SWP		cfbit		SYPCR,5		;  Software Watchdog Prescaler
SWT		cffield		SYPCR,3,2	;  Software Watchdog Timing
BME		cfbit		SYPCR,2		;  Bus Monitor External Enable
BMT		cffield		SYPCR,0,1	;  Bus Monitor Timing
SWIVR		equ		MBAR+$042	; Software Watchdog Interrupt Vector Register (8b)
SWSR		equ		MBAR+$043	; Software Watchdog Service Register (8b)

__defcs		macro		n,Base
CSAR{n}		equ		Base+0		; Chip-Select n Base Address Register (32b)
CSMR{n}		equ		Base+4		; Chip-Select n Address Mask Register (32b)
UD		cfbit		CSMR{n},1	;  Mask user data space in address range
UC		cfbit		CSMR{n},2	;  Mask user code space in address range
SD		cfbit		CSMR{n},3	;  Mask supervisor data space in address range
SC		cfbit		CSMR{n},4	;  Mask supervisor code space in address range
CI		cfbit		CSMR{n},5	;  Mask CPU/IACK space in address range
WP		cfbit		CSMR{n},8	;  Write Protect
CSCR{n}		equ		Base+8		; Chip-Select n Control Register (32b)
V		cfbit		CSCR{n},0	;  Valid Bit
BEM		cfbit		CSCR{n},5	;  Byte Mode Enable
PS		cfbit		CSCR{n},6	;  Port Size
AA		cfbit		CSCR{n},8	;  Auto Acknowledge Enable
BRST		cfbit		CSCR{n},9	;  Burst Enable
WS		cffield		CSCR{n},10,3	;  Wait States
		endm
		__defcs		"0",MBAR+$064
		__defcs		"1",MBAR+$070
		__defcs		"2",MBAR+$07c
		__defcs		"3",MBAR+$088
		__defcs		"4",MBAR+$094
		__defcs		"5",MBAR+$0a0

PAR		equ		MBAR+$0cb	; Pin Assignment Register (8b)
PADDR		equ		MBAR+$1c5	; Port A Data Direction Register (8b)
PADAT		equ		MBAR+$1c9	; Port A Data Register (8b)

;----------------------------------------------------------------------------
; UART
		include		"coldfire/52xxuart.inc"
		__defuart	"",MBAR+$140

;----------------------------------------------------------------------------
; Timers

		include		"coldfire/52xxtmr.inc"
		__deftimer	"1",MBAR+$100
		__deftimer	"2",MBAR+$120

                restore				; re-enable listing

                endif				; __mcf5204inc
