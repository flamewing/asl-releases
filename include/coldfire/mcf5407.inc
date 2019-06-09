		ifndef	__mcf5407inc		; avoid multiple inclusion
__mcf5407inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5407.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5407                  *
;*                                                                          *
;****************************************************************************

		ifndef		MBAR
MBAR		equ		$fc000000
		warning		"MBAR not set - assume default value $fc000000"
		endif

;----------------------------------------------------------------------------
; System Integration Module

RSR		equ		MBAR+$000	; Reset status register (8b)
HRST		cfbit		RSR,7		;  Hardware or system reset
SWTR		cfbit		RSR,5		;  Software watchdog timer reset
SYPCR		equ		MBAR+$001	; System protection control register (8b)
SWE		cfbit		SYPCR,7		;  Software watchdog timer enable
SWRI		cfbit		SYPCR,6		;  Software watchdog reset/interrupt select
SWP		cfbit		SYPCR,5		;  Software watchdog prescaler
SWT		cffield		SYPCR,3,2	;  Software watchdog timing delay
SWTA		cfbit		SYPCR,2		;  Software watchdog transfer acknowledge enable
SWTAVAL		cfbit		SYPCR,1		;  Software watchdog transfer acknowledge valid
SWIVR		equ		MBAR+$002	; Software watchdog interrupt vector register (8b)
SWIV		cffield		SWIVR,0,8	;  Vector
SWSR		equ		MBAR+$003	; Software watchdog service register (8b)
PAR		equ		MBAR+$004	; Pin assignment register (16b)
PAR15		cfbit		PAR,15		;  PP15/A31
PAR14		cfbit		PAR,14		;  PP14/A30
PAR13		cfbit		PAR,13		;  PP13/A29
PAR12		cfbit		PAR,12		;  PP12/A28
PAR11		cfbit		PAR,11		;  PP11/A27
PAR10		cfbit		PAR,10		;  PP10/A26
PAR9		cfbit		PAR,9		;  PP9/A25
PAR8		cfbit		PAR,8		;  PP8/A24
PAR7		cfbit		PAR,7		;  PP7/-TIP
PAR6		cfbit		PAR,6		;  PP6/-DREQ0
PAR5		cfbit		PAR,5		;  PP5/-DREQ1
PAR4		cfbit		PAR,4		;  PP4/TM2
PAR3		cfbit		PAR,3		;  PP3/TM1/-DACK1
PAR2		cfbit		PAR,2		;  PP2/TM0/-DACK0
PAR1		cfbit		PAR,1		;  PP1/TT1
PAR0		cfbit		PAR,0		;  PP0/TT0
IRQPAR		equ		MBAR+$006	; Interrupt port assignment register (16b)
IRQPAR2		cfbit		IRQPAR,7	;  Configures the IRQ pin assignments and priorities
IRQPAR1		cfbit		IRQPAR,6
IRQPAR0		cfbit		IRQPAR,5
ENBDACK1	cfbit		IRQPAR,1	;  Enable DACK1.
ENBDACK0	cfbit		IRQPAR,0	;  Enable DACK0.
PLLCR		equ		MBAR+$008	; PLL control (8b)
ENBSTOP		cfbit		PLLCR,7		;  Enable CPU STOP instruction.
PLLIPL		cffield		PLLCR,4,3	;  PLL interrupt priority level to wake up from CPU STOP.
DISBCLKO	cfbit		PLLCR,3		;  BCLKO disable.
MPARK		equ		MBAR+$00c	; Default bus master park register (8b)
PARK		cffield		PARK,6,2	;  Park.
IARBCTRL	cfbit		PARK,5		;  Internal bus arbitration control.
EARBCTRL	cfbit		PARK,4		;  External bus arbitration control.Enable internal register data bus to be driven on external bus.
SHOWDATA	cfbit		PARK,3		;  Enable internal register data bus to be driven on external bus.

;----------------------------------------------------------------------------
; I2C

MBAR_I2C	equ		MBAR+$280
		include		"52xxi2c.inc"

;----------------------------------------------------------------------------
; Interrupt Controller (like 5307)

MBAR_IC		equ		MBAR+$40

__defipmr	macro		{INTLABEL},Base
__LABEL__	equ		Base
DMA3		cfbit		__LABEL__,17	;  DMA3
DMA2		cfbit		__LABEL__,16	;  DMA2
DMA1		cfbit		__LABEL__,15	;  DMA1
DMA0		cfbit		__LABEL__,14	;  DMA0
UART1		cfbit		__LABEL__,13	;  UART1
UART0		cfbit		__LABEL__,12	;  UART0
I2C		cfbit		__LABEL__,11	;  I2C
TIMER2		cfbit		__LABEL__,10	;  Timer2
TIMER1		cfbit		__LABEL__,9	;  Timer1
SWT		cfbit		__LABEL__,8	;  Software Watchdog Timer
EINT7		cfbit		__LABEL__,7	;  External Interrupt 7
EINT6		cfbit		__LABEL__,6	;  External Interrupt 6
EINT5		cfbit		__LABEL__,5	;  External Interrupt 5
EINT4		cfbit		__LABEL__,4	;  External Interrupt 4
EINT3		cfbit		__LABEL__,3	;  External Interrupt 3
EINT2		cfbit		__LABEL__,2	;  External Interrupt 2
EINT1		cfbit		__LABEL__,1	;  External Interrupt 1
		endm
IPR		__defipmr	MBAR_IC+0	; Interrupt Pending Register (32b)
IMR		__defipmr	MBAR_IC+4	; Interrupt Mask Register (32b)
AVR		equ		MBAR_IC+11	; Autovector Register (8b)
AVEC		cffield		AVR,1,7		;  Autovector Control
BLK		cfbit		AVR,0		;  Block Address Strobe
__deficr	macro		Num,Base
ICR{Num}	equ		Base
AVEC		cfbit		ICR{Num},7	;  Autovector Enable
IL		cffield		ICR{Num},2,3	;  Interrupt Level
IP		cffield		ICR{Num},0,2	;  Interrupt Priority
		endm
		__deficr	"0",MBAR_IC+12	; Software Watchdog Timer
		__deficr	"1",MBAR_IC+13	; Timer0
		__deficr	"2",MBAR_IC+14	; Timer1
		__deficr	"3",MBAR_IC+15	; I2C
		__deficr	"4",MBAR_IC+16	; UART0
		__deficr	"5",MBAR_IC+17	; UART1
		__deficr	"6",MBAR_IC+18	; DMA0
		__deficr	"7",MBAR_IC+19	; DMA1
		__deficr	"8",MBAR_IC+20	; DMA2
		__deficr	"9",MBAR_IC+21	; DMA3

;----------------------------------------------------------------------------
; Chip Select Module

MBAR_CS		equ		MBAR+$80

__N		set		0
		rept	8
		__decstr	__NS,__N
CSAR{__NS}	equ		MBAR_CS+(__N*12)+0   ; Chip-Select Address Register (16b)
BA		cffield		CSAR{__NS},0,16      ;  Base Address (upper 16 bits)
CSMR{__NS}	equ		MBAR_CS+(__N*12)+4   ; Chip-Select Mask Register (32b)
BAM		cffield		CSMR{__NS},16,16     ;  Base Address Mask
WP		cfbit		CSMR{__NS},8         ;  Write Protect
AM		cfbit		CSMR{__NS},6         ;  Alternate master
CI		cfbit		CSMR{__NS},5         ;  CPU space and interrupt acknowledge cycle mask
SC		cfbit		CSMR{__NS},4         ;  Supervisor Code Address Space Mask
SD		cfbit		CSMR{__NS},3         ;  Supervisor Data Address Space Mask
UC		cfbit		CSMR{__NS},2         ;  User Code Address Space Mask
UD		cfbit		CSMR{__NS},1         ;  User Data Address Space Mask
V		cfbit		CSMR{__NS},0         ;  Valid
CSCR{__NS}	equ		MBAR_CS+(__N*12)+8   ; Chip-Select Control Register (16b)
WS		cffield		CSCR{__NS},10,4	     ;  Wait States
AA		cfbit		CSCR{__NS},8         ;  Auto-Acknowledge Enable
PS		cffield		CSCR{__NS},6,2       ;  Port Size
BEM		cfbit		CSCR{__NS},5         ;  Bus Master Emable
BSTR		cfbit		CSCR{__NS},4         ;  Burst-Read Enable
BSTW		cfbit		CSCR{__NS},3         ;  Burst-Write Enable
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; DRAM Controller (like 5307)

MBAR_DRAM	equ		MBAR+$100

DCR		equ		MBAR_DRAM+0	; DRAM Control Register (16b)
SO		cfbit		DCR,15		;  Synchronous Operation
NAM		cfbit		DCR,13		;  No Address Multiplexing
RRA		cffield		DCR,11,2	;  Refresh /RAS Asserted
RRP		cffield		DCR,9,2		;  Refresh /RAS Precharge
RC		cffield		DCR,0,9		;  Refresh Count
__defsdcs	macro		N,Addr
DACR{N}		equ		Addr+0		; Address and Control Register (32b)
BA		cffield		DACR{N},18,14	;  Base Address
RE		cfbit		DACR{N},15	;  Refresh Enable
CAS		cffield		DACR{N},12,2	;  /CAS Timing
RP		cfbit		DACR{N},10,2	;  /RAS Precharge Timing
RNCN		cffield		DACR{N},9	;  /RAS-Negate-to-/CAS-Negate
RCD		cfbit		DACR{N},8	;  /RAS-to-/CAS Delay
EDO		cfbit		DACR{N},6	;  Extended Data Out
PS		cffield		DACR{N},4,2	;  Port Size
PM		cffield		DACR{N},2,2	;  Page Mode (32b)
DMR{N}		equ		Addr+4		; Mask Register
BAM		cffield		DMR{N},18,14	;  Base Address Mask
WP		cfbit		DMR{N},8	;  Write Protect
CI		cfbit		DMR{N},6	;  CPU Space/Interrupt Acknowledge
AM		cfbit		DMR{N},5	;  Alternate Master
SC		cfbit		DMR{N},4	;  Supervisor Code
SD		cfbit		DMR{N},3	;  Supervisor Data
UC		cfbit		DMR{N},2	;  User Code
UD		cfbit		DMR{N},1	;  User Data
V		cfbit		DMR{N},0	;  Valid
		endm
		__defsdcs	"0",MBAR_DRAM+8
		__defsdcs	"1",MBAR_DRAM+16

;----------------------------------------------------------------------------
; DMA Controller (like 5307)

MBAR_DMA	equ		MBAR+$300

__N		set		0
		rept		4
		__decstr	__NS,__N
SAR{__NS}	equ		MBAR_DMA+(__N*64)+0  ; Source Address Register (32b)
DAR{__NS}	equ		MBAR_DMA+(__N*64)+4  ; Destination Address Register (32b)
DCR{__NS}	equ		MBAR_DMA+(__N*64)+8  ; Control Register (32b)
INT		cfbit		DCR{__NS},31         ;  Interrupt on Completion of Transfer
EEXT		cfbit		DCR{__NS},30         ;  Enable External Request
CS		cfbit		DCR{__NS},29         ;  Cycle Steal
AA		cfbit		DCR{__NS},28         ;  Auto-Align
BWC		cffield		DCR{__NS},25,3       ;  Bandwidth Control
SAA		cfbit		DCR{__NS},24         ;  Single-Address Access
S_RW		cfbit		DCR{__NS},23         ;  Single-Address Access Read/Write Value
SINC		cfbit		DCR{__NS},22         ;  Source Increment
SSIZE		cffield		DCR{__NS},20,2       ;  Source Size
DINC		cfbit		DCR{__NS},19         ;  Destination Increment
DSIZE		cffield		DCR{__NS},17,2       ;  Destination Size
START		cfbit		DCR{__NS},16         ;  Start Transfer
AT		cfbit		DCR{__NS},15         ;  DMA Acknowledge Type
BCR{__NS}	equ		MBAR_DMA+(__N*64)+12 ; Byte Count Register (32b)
DSR{__NS}	equ		MBAR_DMA+(__N*64)+16 ; Status Register (8b)
CE		cfbit		DSR{__NS},6          ;  Configuration Error
BES		cfbit		DSR{__NS},5          ;  Bus Error on Source
BED		cfbit		DSR{__NS},4          ;  Bus Error on Destination
REQ		cfbit		DSR{__NS},2          ;  Request
BSY		cfbit		DSR{__NS},1          ;  Busy
DONE		cfbit		DSR{__NS},0          ;  Transactions Done
DIVR{__NS}	equ		MBAR_DMA+(__N*64)+20 ; Interrupt Vector Register (8b)
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; Timer

		include		"52xxtmr.inc"
		__deftimer	"0",MBAR+$140
		__deftimer	"1",MBAR+$180

;----------------------------------------------------------------------------
; UARTs

		include		"53xxuart.inc"
		__defuart	"0",MBAR+$140
		__defuart	"1",MBAR+$200

RXLVL		equ		UMR11+1		; Rx FIFO threshold register (8b) (UART1 only)
MODCTL		equ		UMR11+2		; Modem control register (8b) (UART1 only)
ACRB		cfbit		MODCTL,7	;  AC `97 cold reset
AWR		cfbit		MODCTL,6	;  AC `97 warm reset
DSL		cffield		MODCTL,4,2	;  Channel select for DMA channels 2 and 3.
DTS1		cfbit		MODCTL,3	;  Delay of time slot 1.
SHDIR		cfbit		MODCTL,2	;  Shift direction.
MODE		cffield		MODCTL,0,2	;  Mode select for UART1.
TXLXL		equ		UMR11+3		; Tx FIFO threshold register (8b) (UART1 only)
RSMP		equ		USR1+2		; Rx samples available (8b) (UART1 only)
TSPC		equ		USR1+3		; Tx space available (8b) (UART1 only)

;----------------------------------------------------------------------------
; Parallel Port

PADDR		equ		MBAR+$244	; Port A Data Direction Register (16b)
PADAT		equ		MBAR+$248	; Port A Data Register (16b)

;----------------------------------------------------------------------------

                restore				; re-enable listing

                endif				; __mcf5407inc
