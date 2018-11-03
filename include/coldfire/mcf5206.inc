		ifndef	__mcf5206inc		; avoid multiple inclusion
__mcf5206inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5206.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5206                  *
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

		__deficr	ICR1,MBAR+$014; Interrupt Control Register External IRQ1 (8b)
		__deficr	ICR2,MBAR+$015; Interrupt Control Register External IRQ2 (8b)
		__deficr	ICR3,MBAR+$016; Interrupt Control Register External IRQ3 (8b)
		__deficr	ICR4,MBAR+$017; Interrupt Control Register External IRQ4 (8b)
		__deficr	ICR5,MBAR+$018; Interrupt Control Register External IRQ5 (8b)
		__deficr	ICR6,MBAR+$019; Interrupt Control Register External IRQ6 (8b)
		__deficr	ICR7,MBAR+$01a; Interrupt Control Register External IRQ7 (8b)
		__deficr	ICR8,MBAR+$01b; Interrupt Control Register SWI (8b)
		__deficr	ICR9,MBAR+$01c; Interrupt Control Register Timer 1 (8b)
		__deficr	ICR10,MBAR+$01d; Interrupt Control Register Timer 2 (8b)
		__deficr	ICR11,MBAR+$01e; Interrupt Control Register MBUS (8b)
		__deficr	ICR12,MBAR+$01f; Interrupt Control Register UART1 (8b)
		__deficr	ICR13,MBAR+$020; Interrupt Control Register UART2 (8b)

__defirq	macro		name,addr
name		equ		addr
UART2		cfbit		name,13
UART1		cfbit		name,12
MBUS		cfbit		name,11
T2		cfbit		name,10
T1		cfbit		name,9
SWT		cfbit		name,8
IRQ7		cfbit		name,7
IRQ6		cfbit		name,6
IRQ5		cfbit		name,5
IRQ4		cfbit		name,4
IRQ3		cfbit		name,3
IRQ2		cfbit		name,2
IRQ1		cfbit		name,1
		endm
		__defirq	IMR,MBAR+$036	; Interrupt Mask Register (16b)
		__defirq	IPR,MBAR+$03a	; Interrupt Pending Register (16b)

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

PAR		equ		MBAR+$0cb	; Pin Assignment Register (8b)
PADDR		equ		MBAR+$1c5	; Port A Data Direction Register (8b)
PADAT		equ		MBAR+$1c9	; Port A Data Register (8b)

;----------------------------------------------------------------------------
; Chip Select Module

__defcs		macro		n,Base
CSAR{n}		equ		Base+0		; Chip-Select n Base Address Register (32b)
CSMR{n}		equ		Base+4		; Chip-Select n Address Mask Register (32b)
UD		cfbit		CSMR{n},1	;  Mask user data space in address range
UC		cfbit		CSMR{n},2	;  Mask user code space in address range
SD		cfbit		CSMR{n},3	;  Mask supervisor data space in address range
SC		cfbit		CSMR{n},4	;  Mask supervisor code space in address range
CI		cfbit		CSMR{n},5	;  Mask CPU/IACK space in address range (only CSMR1)
CSCR{n}		equ		Base+10		; Chip-Select n Control Register (16b)
RD		cfbit		CSCR{n},0	;  Read Enable
WR		cfbit		CSCR{n},1	;  Write Enable
RDAH		cfbit		CSCR{n},2	;  Read Address Hold Enable
WRAH		cfbit		CSCR{n},3	;  Write Address Hold Enable
ASET		cfbit		CSCR{n},4	;  Address Setup Enable
EMAA		cfbit		CSCR{n},5	;  Alternate Master Automatic Acknowledge Enable
PS		cffield		CSCR{n},6,2	;  Port Size
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
		
DMCR		equ		MBAR+$0c6	; Default Memory Control Register (16b)
WS		cffield		DMCR,10,4	;  Wait States
BRST		cfbit		DMCR,9		;  Burst Enable
AA		cfbit		DMCR,8		;  Auto Acknowledge Enable
PS		cffield		DMCR,6,2	;  Port Size
EMAA		cfbit		DMCR,5		;  Alternate Master Automatic Acknowledge Enable
WRAH		cfbit		DMCR,3		;  Write Address Hold Enable
RDAH		cfbit		DMCR,2		;  Read Address Hold Enable


;----------------------------------------------------------------------------
; Parallel Port Module

PPDDR		equ		MBAR+$1c5	; Port A Data Direction Register (8b)
PPDAT		equ		MBAR+$1c9	; Port A Data Register

;----------------------------------------------------------------------------
; DRAM Controller

DCRR		equ		MBAR+$46	; Refresh (16b)
RC		cffield		DCRR,0,12	;  Refresh Count
DCTR		equ		MBAR+$4a	; Timing Register (16b)
DAEM		cfbit		DCTR,15		;  Drive Multiplexed Address During External Master DRAM transfers
EDO		cfbit		DCTR,14		;  Extended Data-Out Enable
RCD		cfbit		DCTR,12		;  /RAS-to-/CAS Delay Time
RSH		cffield		DCTR,9,2	;  /RAS Hold Time
RP		cffield		DCTR,5,2	;  /RAS Precharge Time
CAS		cfbit		DCTR,3		;  Column Address Strobe Time
CP		cfbit		DCTR,1		;  /CAS Precharge Time
CSR		cfbit		DCTR,0		;  /CAS Setup Time for /CAS Before /RAS Refresh

__defdrambank	macro		n,Base
DCAR{n}		equ		Base		; Address Register - Bank n (16b)
DCMR{n}		equ		Base+4		; Mask Register - Bank n (32b)
SC		cfbit		DCMR{n},4	;  Supervisor Code Transfer Mask
SD		cfbit		DCMR{n},3	;  Supervisor Data Transfer Mask
UC		cfbit		DCMR{n},2	;  User Code Transfer Mask
UD		cfbit		DCMR{n},1	;  User Data Transfer Mask
DCCR{n}		equ		Base+11		; Control Register - Bank n (8b)
PS		cffield		DCCR{n},6,2	;  Port Size
BPS		cffield		DCCR{n},4,2	;  Bank Page Size
PM		cffield		DCCR{n},2,2	;  Page Mode Select
WR		cfbit		DCCR{n},1	;  Write Enable
RD		cfbit		DCCR{n},0	;  Read Enable
		endm
		__defdrambank	"0",MBAR+$4c
		__defdrambank	"1",MBAR+$58

;----------------------------------------------------------------------------
; UART

		include		"coldfire/52xxuart.inc"
		__defuart       "1",MBAR+$140
		__defuart	"2",MBAR+$180 

;----------------------------------------------------------------------------
; M-Bus Module

MADR		equ		MBAR+$1E0	; Address Register
MFDR		equ		MBAR+$1E4	; Frequency Divider Register
MBC		cffield		MFDR,0,6	;  M-Bus Clock Rate
MBCR		equ		MBAR+$1E8	; Control Register
MEN		cfbit		MBCR,7		;  M-Bus Enable
MIEN		cfbit		MBCR,6		;  M-Bus Interrupt Enable
MSTA		cfbit		MBCR,5		;  Master/Slave Mode Select Bit
MTX		cfbit		MBCR,4		;  Transmit/Receive mode Select Bit
TXAK		cfbit		MBCR,3		;  Transmit Acknowledge Enable
RSTA		cfbit		MBCR,2		;  Repeat Start
MBSR		equ		MBAR+$1EC	; Status Register
MCF		cfbit		MBSR,7		;  Data Transferring Bit
MAAS		cfbit		MBSR,6		;  Addressed as a Slave Bit
MBB		cfbit		MBSR,5		;  Bus Busy Bit
MAL		cfbit		MBSR,4		;  Arbitration Lost
SRW		cfbit		MBSR,2		;  Slave Read/Write
MIF		cfbit		MBSR,1		;  M-Bus Interrupt
RXAK		cfbit		MBSR,0		;  Received Acknowledge
MBDR		equ		MBAR+$1F0	; Data I/O Register

;----------------------------------------------------------------------------
; Timers

		include		"coldfire/52xxtmr.inc"
		__deftimer	"1",MBAR+$100
		__deftimer	"2",MBAR+$120

                restore				; re-enable listing

                endif				; __mcf5206inc
