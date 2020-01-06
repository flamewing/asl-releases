		ifndef	__mcf5307inc		; avoid multiple inclusion
__mcf5307inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5307.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5307                  *
;*                                                                          *
;****************************************************************************

		ifndef		MBAR
MBAR		equ		$fc000000
		warning		"MBAR not set - assume default value $fc000000"
		endif

;----------------------------------------------------------------------------
; System Integration Module

RSR		equ		MBAR+$0		; Reset Status Register (8b)
HRST		cfbit		RSR,7		;  Hardware or System Reset
SWTR		cfbit		RSR,5		;  Software Watchdog Timer Reset
SYPCR		equ		MBAR+$1		; System Protection Control Register (8b)
SWE		cfbit		SYPCR,7		;  Software Watchdog Timer enable
SWRI		cfbit		SYPCR,6		;  Software Watchdog Reset/Interrupt Select
SWP		cfbit		SYPCR,5		;  Software Watchdog Prescaler
SWT		cffield		SYPCR,3,2	;  Software Watchdog Timing Delay
SWTA		cfbit		SYPCR,2		;  Software Watchdog Transfer Acknowledge Enable
SWTAVAL		cfbit		SYPCR,1		;  Software Watchdog Transfer Acknowledge Valid
SWIVR		equ		MBAR+$2		; Software Watchdog Interrupt Vector Register (8b)
SWIV		cffield		SWIVR,0,7
SWSR		equ		MBAR+$3		; Software Watchdog Service Register (8b)
PAR		equ		MBAR+$4		; Pin Assignment Register (32b)
IRQPAR		equ		MBAR+$6		; Interrupt Port Assignment Register (8b)
IRQPAR2		cfbit		IRQPAR,7
IRQPAR1		cfbit		IRQPAR,6
IRQPAR0		cfbit		IRQPAR,5
PLLCR		equ		MBAR+$8		; PLL Control (8b)
ENBSTOP		cfbit		PLLCR,7		;  Enable CPU STOP Instruction
PLLIPL		cffield		PLLCR,4,3	;  PLL Interrupt Priority Level to Wake Up from CPU STOP
MPARK		equ		MBAR+$c		; Default Bus Master Park Register (8b)
PARK		cffield		MPARK,6,2	;  Park
IARBCTRL	cfbit		MPARK,5		;  Internal Bus Arbitration Control
EARBCTRL	cfbit		MPARK,4		;  External Bus Arbitration Control
SHOWDATA	cfbit		MPARK,3		;  Enable Internal Register Data Bus to be Driven on External Bus
BCR24BIT	cfbit		MPARK,0		;  BCR and Address Mapping for DMA

;----------------------------------------------------------------------------
; I2C

MBAR_I2C	equ		MBAR+$280
		include		"52xxi2c.inc"

;----------------------------------------------------------------------------
; Interrupt Controller

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
TIMER1		cfbit		__LABEL__,10	;  Timer1
TIMER0		cfbit		__LABEL__,9	;  Timer0
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
; DRAM Controller

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
PM		cffield		DACR{N},2,2	;  Page Mode
DMR{N}		equ		Addr+4		; Mask Register (32b)
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
; DMA Controller

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

;----------------------------------------------------------------------------
; Parallel Port

PADDR		equ		MBAR+$244	; Port A Data Direction Register (16b)
PADAT		equ		MBAR+$248	; Port A Data Register (16b)

                restore				; re-enable listing

                endif				; __mcf5307inc
