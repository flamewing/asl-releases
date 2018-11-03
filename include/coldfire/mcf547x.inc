		ifndef	__mcf547xinc		; avoid multiple inclusion
__mcf547xinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF547X.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF547x                  *
;*                                                                          *
;****************************************************************************

		ifndef		MBAR
MBAR		equ		$fc000000
		warning		"MBAR not set - assume default value $fc000000"
		endif

;----------------------------------------------------------------------------
; System Integration Unit

SBCR		equ		MBAR+$10	; System Breakpoint Control Register (32b)
PIN2CPU		cfbit		SBCR,31		;  Pin control of the ColdFire V4e breakpoint.
PIN2DMA		cfbit		SBCR,30		;  Pin control of the multichannel DMA breakpoint.
CPU2DMA		cfbit		SBCR,29		;  ColdFire V4e control of the multichannel DMA breakpoint.
DMA2CPU		cfbit		SBCR,28		;  DMA control of the ColdFire V4e breakpoint.
PIN2DSPI	cfbit		SBCR,27		;  Pin control of the DSPI breakpoint.
SECSACR		equ		MBAR+$38	; Sequential Access Control Register (32b)
SEQEN		cfbit		SECSACR,0	;  SEC Sequential access enable.
RSR		equ		MBAR+$44	; Reset Status Register (32b)
RSTJTG		cfbit		RSR,3		;  JTAG reset asserted.
RSTWD		cfbit		RSR,1		;  General purpose watchdog timer reset asserted.
RST		cfbit		RSR,0		;  External reset (PLL Lock qualification) asserted.
JTAGID		equ		MBAR+$50	; JTAG Device Identification Number

;----------------------------------------------------------------------------
; PLL

SPCR		equ		MBAR+$300	; System PLL Control Register (32b)
PLLK		cfbit		SPCR,31		;  System PLL Lock Status
COREN		cfbit		SPCR,14		;  Core & Communications Sub-System Clock Enable
CRYENB		cfbit		SPCR,13		;  Crypto Clock Enable B
CRYENA		cfbit		SPCR,12		;  Crypto Clock Enable A
CAN1EN		cfbit		SPCR,11		;  CAN1 Clock Enable
PSCEN		cfbit		SPCR,9		;  PSC Clock Enable
USBEN		cfbit		SPCR,7		;  USB Clock Enable
FEC1EN		cfbit		SPCR,6		;  FEC1 Clock Enable
FEC0EN		cfbit		SPCR,5		;  FEC0 Clock Enable
DMAEN		cfbit		SPCR,4		;  Multi-channel DMA Clock Enable
CAN0EN		cfbit		SPCR,3		;  CAN0 Clock Enable
FBEN		cfbit		SPCR,2		;  FlexBus Clock Enable
PCIEN		cfbit		SPCR,1		;  PCI Bus Clock Enable
MEMEN		cfbit		SPCR,0		;  Memory Clock Enable

;----------------------------------------------------------------------------
; XL Bus Arbiter

MBAR_XLB	equ		MBAR+$200

XARB_CFG	equ		MBAR_XLB+$40	; Arbiter Configuration Register (32b)
PLDIS		cfbit		XARB_CFG,31	;  Pipeline Disable.
SP		cffield		XARB_CFG,8,3	;  Select Parked Master.
PM		cffield		XARB_CFG,5,2	;  Parking Mode.
BA		cfbit		XARB_CFG,3	;  Bus Activity Time-out Enable.
DT		cfbit		XARB_CFG,2	;  Data Tenure Time-out Enable.
AT		cfbit		XARB_CFG,1	;  Address Tenure Time-out Enable.
XARB_VER	equ		MBAR_XLB+$44	; Arbiter Version Register (32b)
VER		cffield		XARB_VER,0,32	;  Hardware Version ID.
XARB_SR		equ		MBAR_XLB+$48	; Arbiter Status Register (32b)
SEA		cfbit		XARB_SR,8	;  Slave Error Acknowledge.
MM		cfbit		XARB_SR,7	;  Multiple Masters at priority 0.
TTA		cfbit		XARB_SR,6	;  TT Address Only.
TTR		cfbit		XARB_SR,5	;  TT Reserved.
ECW		cfbit		XARB_SR,4	;  External Control Word Read/Write.
TTM		cfbit		XARB_SR,3	;  TBST/TSIZ mismatch.
BA		cfbit		XARB_SR,2	;  Bus Activity Tenure Time-out.
DT		cfbit		XARB_SR,1	;  Data Tenure Time-out.
AT		cfbit		XARB_SR,0	;  Address Tenure Time-out.
XARB_IMR	equ		MBAR_XLB+$4c	; Arbiter Interrupt Mask Register (32b)
SEAE		cfbit		XARB_IMR,8	;  Slave Error Acknowledge interrupt enable.
MME		cfbit		XARB_IMR,7	;  Multiple Masters at priority 0 interrupt enable.
TTAE		cfbit		XARB_IMR,6	;  TT Address Only interrupt enable.
TTRE		cfbit		XARB_IMR,5	;  TT Reserved interrupt enable.
ECWE		cfbit		XARB_IMR,4	;  External Control Word Read/Write interrupt enable.
TTME		cfbit		XARB_IMR,3	;  TBST/TSIZ mismatch interrupt enable.
BAE		cfbit		XARB_IMR,2	;  Bus Activity Tenure Time-out interrupt enable.
DTE		cfbit		XARB_IMR,1	;  Data Tenure Time-out interrupt enable.
ATEq		cfbit		XARB_IMR,0	;  Address Tenure Time-out interrupt enable.
XARB_ADRCAP	equ		MBAR_XLB+$50	; Arbiter Address Capture (32b)
ADRCAP		cffield		XARB_ADRCAP,0,32;  Address that is captured when a bus error occurs.
XARB_SIGCAP	equ		MBAR_XLB+$54	; Arbiter Signal Capture (32b)
TSIZ		cffield		XARB_SIGCAP,7,3	;  Transfer Size.
TBST		cfbit		XARB_SIGCAP,5	;  Burst/Non-Burst
TT		cffield		XARB_SIGCAP,0,5	;  Transfer Type
XARB_ADRTO	equ		MBAR_XLB+$58	; Arbiter Address Timeout (32b)
ADRTO		cffield		XARB_ADRTO,0,28	;  Upper 28-bits of the Address time-out counter value.
XARB_DATTO	equ		MBAR_XLB+$5c	; Arbiter Data Timeout (32b)
DATTO		cffield		XARB_DATTO,0,28	;  Upper 28-bits fo the Data time-out counter value.
XARB_BUSTO	equ		MBAR_XLB+$60	; Arbiter Bus Timeout (32b)
BUSTO		cffield		XARB_BUSTO,0,32	;  Bus activity time-out counter value in XLB clocks.
XARB_PRIEN	equ		MBAR_XLB+$64	; Arbiter Master Priority Enable (32b)
M3		cfbit		XARB_PRIEN,3	;  Master 3 Priority Register Enable
M2		cfbit		XARB_PRIEN,2	;  Master 2 Priority Register Enable
M0		cfbit		XARB_PRIEN,0	;  Master 0 Priority Register Enable
XARB_PRI	equ		MBAR_XLB+$68	; Arbiter Master Priority (32b)
M3P		cffield		XARB_PRI,12,3	;  Master 3 Priority
M2P		cffield		XARB_PRI,8,3	;  Master 2 Priority
M0P		cffield		XARB_PRI,0,3	;  Master 0 Priority

;----------------------------------------------------------------------------
; General Purpose Timers

__defgpt	macro		N,Base
GMS{N}		equ		Base+0		; GPT Enable and Mode Select Register n (32b)
OCPW		cffield		GMS{N},24,8	;  Output capture pulse width.
OCT		cffield		GMS{N},20,2	;  Output capture type.
ICT		cffield		GMS{N},16,2	;  Input capture type.
WDEN		cfbit		GMS{N},15	;  Watchdog enable.
CE		cfbit		GMS{N},12	;  Counter enable.
SC		cfbit		GMS{N},10	;  Stop/continuous mode.
OD		cfbit		GMS{N},9	;  Open drain.
IEN		cfbit		GMS{N},8	;  Interrupt enable.
GPIO		cffield		GMS{N},4,2	;  GPIO mode type.
TMS		cffield		GMS{N},0,3	;  Timer mode select (and module enable).
GCIR{N}		equ		Base+4		; GPT Counter Input Register n (32b)
PRE		cffield		GCIR{N},16,16	;  Prescaler.
CNT		cffield		GCIR{N},0,16	;  Count value.
GPWM{N}		equ		Base+8		; GPT PWM Configuration Register n (32b)
WIDTH		cffield		GPWM{N},16,16	;  PWM width.
PWMOP		cfbit		GPWM{N},8	;  PWM output polarity.
LOAD		cfbit		GPWM{N},0	;  Bit forces immediate period update.
GSR{N}		equ		Base+12		; GPT Status Register n (32b)
CAPTURE		cffield		GSR{N},16,16	;  Read of internal counter, latch at reference event.
OVF		cffield		GSR{N},12,3	;  Overflow counter.
PIN		cfbit		GSR{N},8	;  GPIO input value.
TEXP		cfbit		GSR{N},3	;  Timer expired in internal timer mode.
PWMP		cfbit		GSR{N},2	;  PWM end of period occurred.
COMP		cfbit		GSR{N},1	;  OC reference event occurred.
CAPT		cfbit		GSR{N},0	;  IC reference event occurred.
		endm
		__defgpt	"0",MBAR+$800
		__defgpt	"1",MBAR+$810
		__defgpt	"2",MBAR+$820
		__defgpt	"3",MBAR+$830

;----------------------------------------------------------------------------
; Slice Timers

__defslt	macro		N,Base
STCNT{N}	equ		Base+0		; SLT Terminal Count Register n (32b)
TC		cffield		STCNT{N},0,32	;  Terminal count.
SCR{N}		equ		Base+4		; SLT Control Register n (32b)
RUN		cfbit		SCR{N},26	;  Run or wait mode
IEN		cfbit		SCR{N},25	;  Interrupt enable.
TEN		cfbit		SCR{N},24	;  Timer enable
SCNT{N}		equ		Base+8		; SLT Count Value Register n (32b)
CNT		cffield		SCNT{N},0,32	;  Timer count.
SSR{N}		equ		Base+12		; SLT Status Register n (32b)
BE		cfbit		SSR{N},25	;  Bus Error Status.
ST		cfbit		SSR{N},24	;  SLT timeout.
		endm
		__defslt	"0",MBAR+$900
		__defslt	"1",MBAR+$910

;----------------------------------------------------------------------------
; Interrupt Controller

MBAR_INTC	equ		MBAR+$700
IPRH		equ		MBAR_INTC+$00	; Interrupt Pending Register High (32b)
IPRL		equ		MBAR_INTC+$04	; Interrupt Pending Register Low (32b)
IMRH		equ		MBAR_INTC+$08	; Interrupt Mask Register High (32b)
IMRL		equ		MBAR_INTC+$0c	; Interrupt Mask Register Low (32b)
INTFRCH		equ		MBAR_INTC+$10	; Interrupt Force Register High (32b)
INTFRCL		equ		MBAR_INTC+$14	; Interrupt Force Register Low (32b)
IRLR		equ		MBAR_INTC+$18	; Interrupt Request Level Register (8b)
IRQ		cffield		IRLR,1,7	;  Interrupt requests.
IACKLPR		equ		MBAR_INTC+$19	; Interrupt Acknowledge Level and Priority Register (8b)
LEVEL		cffield		IACKLPR,4,3	;  Interrupt level.
PRI		cffield		IACKLPR,0,4	;  Interrupt Priority.
__N		set		1
		rept		64
		__dec02str	__NS,__N
ICR{"\{__NS}"}	equ		MBAR_INTC+$040+__N; Interrupt Control Register N (8b)
IL		cffield		ICR{"\{__NS}"},3,3 ;  Interrupt level.
IP		cffield		ICR{"\{__NS}"},0,3 ;  Interrupt priority.
__N		set		__N+1
		endm
SWIACK		equ		MBAR_INTC+$e0	; Software Interrupt Acknowledge (8b)
__N		set		1
		rept		6
		__decstr	__NS,__N
L{"\{__NS}"}IACK	equ	MBAR_INTC+$e0+(4*__N) ; Interrupt Acknowledge Register N (8b)
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; Edge Port Module

MBAR_EPORT	equ		MBAR+$f00
		include		"coldfire/52xxeport.inc"	; TODO: edge port 0 not defined on 547x
		
;----------------------------------------------------------------------------
; GPIO

MBAR_GPIO	equ		MBAR+$a00

; Port Output Data Registers

PODR_FBCTL	equ		MBAR_GPIO+$00	; FlexBus Control Output Data Register (8b)
PODR_FBCS	equ		MBAR_GPIO+$01	; FlexBus ChipSelect Output Data Register (8b)
PODR_DMA	equ		MBAR_GPIO+$02	; DMA Output Data Register (8b)
PODR_FEC0H	equ		MBAR_GPIO+$04	; FEC0 High Output Data Register (8b)
PODR_FEC0L	equ		MBAR_GPIO+$05	; FEC0 Low Output Data Register (8b)
PODR_FEC1H	equ		MBAR_GPIO+$06	; FEC1 High Output Data Register (8b)
PODR_FEC1L	equ		MBAR_GPIO+$07	; FEC1 Low Output Data Register (8b)
PODR_FECI2C	equ		MBAR_GPIO+$08	; FEC/I2C Output Data Register (8b)
PODR_PCIBG	equ		MBAR_GPIO+$09	; PCI Grant Output Data Register (8b)
PODR_PCIBR	equ		MBAR_GPIO+$0a	; PCI Request Output Data Register (8b)
PODR_PSC3PSC2	equ		MBAR_GPIO+$0c	; Progr. Serial Controller 3/2 Output Data Register (8b)
PODR_PSC1PSC0	equ		MBAR_GPIO+$0d	; Progr. Serial Controller 1/0 Output Data Register (8b)
PODR_DSPI	equ		MBAR_GPIO+$0e	; DMA Serial Peripheral Interface Output Data Register (8b)

; Port Data Direction Registers

PDDR_FBCTL	equ		MBAR_GPIO+$10	; FlexBus Control Data Direction Register (8b)
PDDR_FBCS	equ		MBAR_GPIO+$11	; FlexBus ChipSelect Data Direction Register (8b)
PDDR_DMA	equ		MBAR_GPIO+$12	; DMA Output Data Direction Register (8b)
PDDR_FEC0H	equ		MBAR_GPIO+$14	; FEC0 High Data Direction Register (8b)
PDDR_FEC0L	equ		MBAR_GPIO+$15	; FEC0 Low Data Direction Register (8b)
PDDR_FEC1H	equ		MBAR_GPIO+$16	; FEC1 High Data Direction Register (8b)
PDDR_FEC1L	equ		MBAR_GPIO+$17	; FEC1 Low Data Direction Register (8b)
PDDR_FECI2C	equ		MBAR_GPIO+$18	; FEC/I2C Data Direction Register (8b)
PDDR_PCIBG	equ		MBAR_GPIO+$19	; PCI Grant Data Direction Register (8b)
PDDR_PCIBR	equ		MBAR_GPIO+$1a	; PCI Request Data Direction Register (8b)
PDDR_PSC3PSC2	equ		MBAR_GPIO+$1c	; Progr. Serial Controller 3/2 Data Direction Register (8b)
PDDR_PSC1PSC0	equ		MBAR_GPIO+$1d	; Progr. Serial Controller 1/0 Data Direction Register (8b)
PDDR_DSPI	equ		MBAR_GPIO+$1e	; DMA Serial Peripheral Interface Data Direction Register (8b)

; Port Pin Data/Set Data Registers

PPDSDR_FBCTL	equ		MBAR_GPIO+$20	; FlexBus Control Data/Set Data Register (8b)
PPDSDR_FBCS	equ		MBAR_GPIO+$21	; FlexBus ChipSelect Data/Set Data Register (8b)
PPDSDR_DMA	equ		MBAR_GPIO+$22	; DMA Output Data/Set Data Register (8b)
PPDSDR_FEC0H	equ		MBAR_GPIO+$24	; FEC0 High Data/Set Data Register (8b)
PPDSDR_FEC0L	equ		MBAR_GPIO+$25	; FEC0 Low Data/Set Data Register (8b)
PPDSDR_FEC1H	equ		MBAR_GPIO+$26	; FEC1 High Data/Set Data Register (8b)
PPDSDR_FEC1L	equ		MBAR_GPIO+$27	; FEC1 Low Data/Set Data Register (8b)
PPDSDR_FECI2C	equ		MBAR_GPIO+$28	; FEC/I2C Data/Set Data Register (8b)
PPDSDR_PCIBG	equ		MBAR_GPIO+$29	; PCI Grant Data/Set Data Register (8b)
PPDSDR_PCIBR	equ		MBAR_GPIO+$2a	; PCI Request Data/Set Data Register (8b)
PPDSDR_PSC3PSC2	equ		MBAR_GPIO+$2c	; Progr. Serial Controller 3/2 Data/Set Data Register (8b)
PPDSDR_PSC1PSC0	equ		MBAR_GPIO+$2d	; Progr. Serial Controller 1/0 Data/Set Data Register (8b)
PPDSDR_DSPI	equ		MBAR_GPIO+$2e	; DMA Serial Peripheral Interface Data/Set Data Register (8b)

; Port Clear Output Data Registers

PCLRR_FBCTL	equ		MBAR_GPIO+$30	; FlexBus Control Clear Output Data Register (8b)
PCLRR_FBCS	equ		MBAR_GPIO+$31	; FlexBus ChipSelect Clear Output Data Register (8b)
PCLRR_DMA	equ		MBAR_GPIO+$32	; DMA Output Clear Output Data Register (8b)
PCLRR_FEC0H	equ		MBAR_GPIO+$34	; FEC0 High Clear Output Data Register (8b)
PCLRR_FEC0L	equ		MBAR_GPIO+$35	; FEC0 Low Clear Output Data Register (8b)
PCLRR_FEC1H	equ		MBAR_GPIO+$36	; FEC1 High Clear Output Data Register (8b)
PCLRR_FEC1L	equ		MBAR_GPIO+$37	; FEC1 Low Clear Output Data Register (8b)
PCLRR_FECI2C	equ		MBAR_GPIO+$38	; FEC/I2C Clear Output Data Register (8b)
PCLRR_PCIBG	equ		MBAR_GPIO+$39	; PCI Grant Clear Output Data Register (8b)
PCLRR_PCIBR	equ		MBAR_GPIO+$3a	; PCI Request Clear Output Data Register (8b)
PCLRR_PSC3PSC2	equ		MBAR_GPIO+$3c	; Progr. Serial Controller 3/2 Clear Output Data Register (8b)
PCLRR_PSC1PSC0	equ		MBAR_GPIO+$3d	; Progr. Serial Controller 1/0 Clear Output Data Register (8b)
PCLRR_DSPI	equ		MBAR_GPIO+$3e	; DMA Serial Peripheral Clear Output Data Register (8b)

; Pin Assignment Registers

PAR_FBCTL	equ		MBAR_GPIO+$00	; FlexBus Control Pin Assignment Register (16b)
PAR_BWE3	cfbit		PAR_FBCTL,14	;  -BE3/-BWE3 Usage
PAR_BWE2	cfbit		PAR_FBCTL,12	;  -BE2/-BWE2 Usage
PAR_BWE1	cfbit		PAR_FBCTL,10	;  -BE1/-BWE1 Usage
PAR_BWE0	cfbit		PAR_FBCTL,8	;  -BE0/-BWE0 Usage
PAR_OE		cfbit		PAR_FBCTL,6	;  -OE Usage
PAR_RWB		cffield		PAR_FBCTL,4,2	;  -R/-W Usage
PAR_TA		cfbit		PAR_FBCTL,2	;  -TA Usage
PAR_ALE		cffield		PAR_FBCTL,0,2	;  ALE Usage
PAR_FBCS	equ		MBAR_GPIO+$02	; FlexBus ChipSelect Pin Assignment Register (8b)
PAR_CS5		cfbit		PAR_FBCS,5	;  -FBCS5 Usage
PAR_CS4		cfbit		PAR_FBCS,4	;  -FBCS4 Usage
PAR_CS3		cfbit		PAR_FBCS,3	;  -FBCS3 Usage
PAR_CS2		cfbit		PAR_FBCS,2	;  -FBCS2 Usage
PAR_CS1		cfbit		PAR_FBCS,1	;  -FBCS1 Usage
PAR_DMA		equ		MBAR_GPIO+$03	; DMA Output Pin Assignment Register (8b)
PAR_DACK1	cffield		PAR_DMA,6,2	;  -DACK1 Usage
PAR_DACK0	cffield		PAR_DMA,4,2	;  -DACK0 Usage
PAR_DREQ1	cffield		PAR_DMA,2,2	;  -DREQ1 Usage
PAR_DREQ0	cffield		PAR_DMA,0,2	;  -DREQ0 Usage
PAR_FECI2CIRQ	equ		MBAR_GPIO+$04	; FEC/I2C Pin Assignment Register (16b)
PAR_E07		cfbit		PAR_FECI2CIRQ,15;  FEC0 7-wire mode pin assignment.
PAR_E0MII	cfbit		PAR_FECI2CIRQ,14;  FEC1 MII mode-only pin assignment.
PAR_E0MDIO	cfbit		PAR_FECI2CIRQ,13;  FEC0 MDIO pin assignment.
PAR_E0MDC	cfbit		PAR_FECI2CIRQ,12;  FEC0 MDC pin assignment.
PAR_E17		cfbit		PAR_FECI2CIRQ,11;  FEC1 7-wire mode pin assignment.
PAR_E1MII	cfbit		PAR_FECI2CIRQ,10;  FEC1 MII mode-only pin assignment.
PAR_E1MDIO	cffield		PAR_FECI2CIRQ,8,2;  FEC1 MDIO pin assignment.
PAR_E1MDC	cffield		PAR_FECI2CIRQ,6,2;  FEC1 MDC pin assignment.
PAR_SDA		cfbit		PAR_FECI2CIRQ,3	;  SDA Pin Assignment.
PAR_SCL		cfbit		PAR_FECI2CIRQ,2	;  SCL Pin Assignment.
PAR_IRQ6	cfbit		PAR_FECI2CIRQ,1	;  -IRQ6 Pin Assignment.
PAR_IRQ5	cfbit		PAR_FECI2CIRQ,0	;  -IRQ5 Pin Assignment.
PAR_PCIBG	equ		MBAR_GPIO+$08	; PCI Grant Pin Assignment Register (16b)
PAR_PCIBG4	cffield		PAR_PCIBG,8,2	;  -PCIBG4 pin assignment.
PAR_PCIBG3	cffield		PAR_PCIBG,6,2	;  -PCIBG3 pin assignment.
PAR_PCIBG2	cffield		PAR_PCIBG,4,2	;  -PCIBG2 pin assignment.
PAR_PCIBG1	cffield		PAR_PCIBG,2,2	;  -PCIBG1 pin assignment.
PAR_PCIBG0	cffield		PAR_PCIBG,0,2	;  -PCIBG0 pin assignment.
PAR_PCIBR	equ		MBAR_GPIO+$0a	; PCI Request Pin Assignment Register (16b)
PAR_PCIBR4	cffield		PAR_PCIBR,8,2	;  -PCIBR4 Pin Assignment.
PAR_PCIBR3	cffield		PAR_PCIBR,6,2	;  -PCIBR3 Pin Assignment.
PAR_PCIBR2	cffield		PAR_PCIBR,4,2	;  -PCIBR2 Pin Assignment.
PAR_PCIBR1	cffield		PAR_PCIBR,2,2	;  -PCIBR1 Pin Assignment.
PAR_PCIBR0	cffield		PAR_PCIBR,0,2	;  -PCIBR0 Pin Assignment.
PAR_PSC3	equ		MBAR_GPIO+$0c	; Progr. Serial Controller 3 Pin Assignment Register (8b)
PAR_CTS3	cffield		PAR_PSC3,6,2	;  -PSC3CTS pin assignment.
PAR_RTS3	cffield		PAR_PSC3,4,2	;  -PSC3RTS pin assignment.
PAR_RXD3	cfbit		PAR_PSC3,3	;  PSC3RXD pin assignment.
PAR_TXD3	cfbit		PAR_PSC3,2	;  PSC3TXD pin assignment.
PAR_PSC2	equ		MBAR_GPIO+$0d	; Progr. Serial Controller 2 Pin Assignment Register (8b)
PAR_CTS2	cffield		PAR_PSC2,6,2	;  -PSC2CTS pin assignment.
PAR_RTS2	cffield		PAR_PSC2,4,2	;  -PSC2RTS pin assignment.
PAR_RXD2	cfbit		PAR_PSC2,3	;  PSC2RXD pin assignment.
PAR_TXD2	cfbit		PAR_PSC2,2	;  PSC2TXD pin assignment.
PAR_PSC1	equ		MBAR_GPIO+$0e	; Progr. Serial Controller 1 Pin Assignment Register (8b)
PAR_CTS1	cffield		PAR_PSC1,6,2	;  -PSC1CTS pin assignment.
PAR_RTS1	cffield		PAR_PSC1,4,2	;  -PSC1RTS pin assignment.
PAR_RXD1	cfbit		PAR_PSC1,3	;  PSC1RXD pin assignment.
PAR_TXD1	cfbit		PAR_PSC1,2	;  PSC1TXD pin assignment.
PAR_PSC0	equ		MBAR_GPIO+$0f	; Progr. Serial Controller 0 Pin Assignment Register (8b)
PAR_CTS0	cffield		PAR_PSC0,6,2	;  -PSC0CTS pin assignment.
PAR_RTS0	cffield		PAR_PSC0,4,2	;  -PSC0RTS pin assignment.
PAR_RXD0	cfbit		PAR_PSC0,3	;  PSC0RXD pin assignment.
PAR_TXD0	cfbit		PAR_PSC0,2	;  PSC0TXD pin assignment.
PAR_DSPI	equ		MBAR_GPIO+$10	; DMA Serial Peripheral Pin Assignment Register (16b)
PAR_CS5		cfbit		PAR_DSPI,12	;  DSPICS5/-PCSS pin assignment.
PAR_CS3		cffield		PAR_DSPI,10,2	;  DSPICS3 pin assignment.
PAR_CS2		cffield		PAR_DSPI,8,2	;  DSPICS2 pin assignment.
PAR_CS0		cffield		PAR_DSPI,6,2	;  DSPICS0/-SS pin assignment.
PAR_SCK		cffield		PAR_DSPI,4,2	;  DSPISCK pin assignment.
PAR_SIN		cffield		PAR_DSPI,2,2	;  DSPISIN pin assignment.
PAR_SOUT	cffield		PAR_DSPI,0,2	;  DSPISOUT pin assignment.
PAR_TIMER	equ		MBAR_GPIO+$12	; Timer Pin Assignment Register (8b)
PAR_TIN3	cffield		PAR_TIMER,4,2	;  TIN3 pin assignment.
PAR_TOUT3	cfbit		PAR_TIMER,3	;  TOUT3 pin assignment.
PAR_TIN2	cffield		PAR_TIMER,1,2	;  TIN2 pin assignment.
PAR_TOUT2	cfbit		PAR_TIMER,0	;  TOUT2 pin assignment.

;----------------------------------------------------------------------------
; FlexBus

MBAR_FBUS	equ		MBAR+$500
		include		"coldfire/52xxfbus.inc"

;----------------------------------------------------------------------------
; SDRAM Controller

SDRAMDS		equ		MBAR+$04	; SDRAM Drive Strength Register (32b)
SB_E		cffield		SDRAMDS,8,2	;  Controls the drive strength of SDCKE.
SB_C		cffield		SDRAMDS,6,2	;  Controls the drive strength of SDRAM clocks.
SB_A		cffield		SDRAMDS,4,2	;  Controls the drive strength of SDCS[3:0], RAS, CAS, SDWE, SDADDR[12:0], and SDBA[1:0].
SB_S		cffield		SDRAMDS,2,2	;  Controls the drive strength of SDRDQS.
SB_D		cffield		SDRAMDS,0,2	;  Controls the drive strength of SDDATA[31:0], SDDM[3:0], and SDQS[3:0].
__N		set		0
		rept		4
		__decstr	__NS,__N
CS{__NS}CFG	equ		MBAR+$20+(__N*4); SDRAM Chip Select Configuration Registers n (32b)
CSBA		cffield		CS{__NS}CFG,20,12 ;  Chip select base address.
CSSZ		cffield		CS{__NS}CFG,0,5	;  Chip select size.
__N		set		__N+1
		endm

MBAR_SDRAM	equ		MBAR+$100	; like 52xx SDRAM with different names

SDMR		equ		MBAR_SDRAM+0	; Mode/Extended Mode Register (32b)
BNKAD		cffield		SDMR,30,2	;  Bank Address
AD		cffield		SDMR,18,12	;  Address
CMD		cfbit		SDMR,16		;  Command
SDCR		equ		MBAR_SDRAM+4	; Control Register (32b)
MODE_EN		cfbit		SDCR,31		;  SDRAM Mode Register Programming Enable
CKE		cfbit		SDCR,30		;  Clock Enable
DDR		cfbit		SDCR,29		;  DDR Mode Select
REF		cfbit		SDCR,28		;  Refresh Enable
MUX		cffield		SDCR,24,2	;  Use of Internal Address Bits A[27:24]
AP		cfbit		SDCR,23		;  Auto precharge control bit.
DRIVE		cfbit		SDCR,22		;  Drive Rule Selection
RCNT		cffield		SDCR,16,6	;  Refresh Rate
DQS_OE		cffield		SDCR,8,4	;  DQS Output Enable
BUFF		cfbit		SDCR,4		;  Buffering mode.
IREF		cfbit		SDCR,2		;  Initiate Refresh Command
IPALL		cfbit		SDCR,1		;  Initiate Precharge All Command
SDCFG1		equ		MBAR_SDRAM+8	; Configuration Register 1 (32b)
SRD2RW		cffield		SDCFG1,28,4	;  Single Read to Read/Write/Precharge Delay
SWT2RD		cffield		SDCFG1,24,3	;  Single Write to Read/Write/Precharge Delay
RDLAT		cffield		SDCFG1,20,4	;  Read CAS Latency
ACT2RW		cffield		SDCFG1,16,3	;  Active to Read/Write Delay
PRE2ACT		cffield		SDCFG1,12,3	;  Precharge to Active Delay
REF2ACT		cffield		SDCFG1,8,4	;  Refresh to Active Delay
WTLAT		cffield		SDCFG1,4,3	;  Write Latency
SDCFG2		equ		MBAR_SDRAM+12	; Configuration Register 2 (32b)
BRD2PRE		cffield		SDCFG2,28,4	;  Burst Read to Read/Precharge Delay
BWT2RW		cffield		SDCFG2,24,4	;  Burst Write to Read/Write/Precharge Delay
BRD2WT		cffield		SDCFG2,20,4	;  Burst Read to Write Delay
BL		cffield		SDCFG2,16,4	;  Burst Length
;----------------------------------------------------------------------------
; PCI Controller

MBAR_PCI	equ		MBAR+$b00
MBAR_CBUSPCI	equ		MBAR+$8400

; PCI Type 0 Configuration Registers

PCIIDR		equ		MBAR_PCI+$00	; PCI Device ID/Vendor ID (32b)
DEVICE_ID	cffield		PCIIDR,16,16	;  PCI Device Id assigned to the MCF547x.
VENDOR_ID	cffield		PCIIDR,0,16	;  PCI Vendor Id assigned to the MCF547x.
PCISCR		equ		MBAR_PCI+$04	; PCI Status/Command (32b)
PE		cfbit		PCISCR,31	;  Parity error detected.
SE		cfbit		PCISCR,30	;  System error signalled.
MA		cfbit		PCISCR,29	;  Master abort received.
TR		cfbit		PCISCR,28	;  Target abort received.
TS		cfbit		PCISCR,27	;  Target abort signalled.
DT		cffield		PCISCR,25,2	;  DEVSEL timing.
DP		cfbit		PCISCR,24	;  Master data parity error.
FC		cfbit		PCISCR,23	;  Fast back-to-back capable.
R		cfbit		PCISCR,22	;  Reserved.
66M		cfbit		PCISCR,21	;  66 MHz capable.
C		cfbit		PCISCR,20	;  Capabilities list.
F		cfbit		PCISCR,9	;  Fast back-to-back transfer enable.
S		cfbit		PCISCR,8	;  SERR enable.
ST		cfbit		PCISCR,7	;  Address and data stepping.
PER		cfbit		PCISCR,6	;  Parity error response.
V		cfbit		PCISCR,5	;  VGA palette snoop enable.
MW		cfbit		PCISCR,4	;  Memory write and invalidate enable.
SP		cfbit		PCISCR,3	;  Special cycle monitor or ignore.
B		cfbit		PCISCR,2	;  Bus master enable.
M		cfbit		PCISCR,1	;  Memory access control.
IO		cfbit		PCISCR,0	;  I/O access control.
PCICCRIR	equ		MBAR_PCI+$08	; PCI Class Code/Revision ID (32b)
CLASS_CODE	cffield		PCICCRIR,8,24	;  PCI Class Code assigned to processor.
REVISION_ID	cffield		PCICCRIR,0,8	;  PCI Revision ID for this version of the processor.
PCICR1		equ		MBAR_PCI+$0c	; PCI Configuration 1 Register (32b)
BIST		cffield		PCICR1,24,8	;  BIST
HEADER_TYPE	cffield		PCICR1,16,8	;  Header Type
LAT_TIMER	cffield		PCICR1,8,8	;  Lat Timer
CLSIZ		cffield		PCICR1,0,8	;  Cache Line Size
PCIBAR0		equ		MBAR_PCI+$10	; PCI Base Address Register 0 (32b)
BAR0		cffield		PCIBAR0,18,12	;  Base address register 0 (b12..31)
PREF		cfbit		PCIBAR0,3	;  Prefetchable access.
RANGE		cffield		PCIBAR0,1,2	;  Fixed to 00.
IOM		cfbit		PCIBAR0,0	;  IO or memory space.
PCIBAR1		equ		MBAR_PCI+$14	; PCI Base Address Register 1 (32b)
BAR1		cffield		PCIBAR1,30,2	;  Base address register 1 (b30..31)
PREF		cfbit		PCIBAR1,3	;  Prefetchable access.
RANGE		cffield		PCIBAR1,1,2	;  Fixed to 00.
IOM		cfbit		PCIBAR1,0	;  IO or memory space.
PCICCPR		equ		MBAR_PCI+$28	; PCI Cardbus CIS Pointer (32b)
PCISID		equ		MBAR_PCI+$2c	; Subsystem ID/Subsystem Vendor ID (32b)
PCIERBAR	equ		MBAR_PCI+$30	; PCI Expansion ROM (32b)
PCICPR		equ		MBAR_PCI+$34	; PCI Capabilities Pointer (32b)
PCICR2		equ		MBAR_PCI+$3c	; PCI Configuration Register 2 (32b)
MAX_LAT		cffield		PCICR2,24,8	;  Maximum latency.
MIN_GNT		cffield		PCICR2,16,8	;  Minimum grant.
INTERRUPT_PIN	cffield		PCICR2,8,8	;  Fixed to 0x00.
INTERRUPT_LINE	cffield		PCICR2,0,8	;  Fixed to 0x00.

; General Control/Status Registers

PCIGSCR		equ		MBAR_PCI+$60	; Global Status/Control Register (32b)
PE		cfbit		PCIGSCR,29	;  PERR detected.
SE		cfbit		PCIGSCR,28	;  SERR detected.
XLB2CLKIN	cffield		PCIGSCR,24,3	;  stores the XL bus clock to external PCI clock (CLKIN)divide ratio.
PEE		cfbit		PCIGSCR,13	;  Parity error interrupt enable.
SEE		cfbit		PCIGSCR,12	;  System error interrupt enable.
PR		cfbit		PCIGSCR,0	;  PCI reset.
PCITBATR0	equ		MBAR_PCI+$64	; Target Base Address Translation Register 0 (32b)
BAT0		cffield		PCITBATR0,18,14	;  Base address (b18...31)
EN		cfbit		PCITBATR0,0	;  Enables a transaction in BAR0 space.
PCITBATR1	equ		MBAR_PCI+$68	; Target Base Address Translation Register 1 (32b)
BAT1		cffield		PCITBATR1,30,2	;  Base address (b30..31)
EN		cfbit		PCITBATR1,0	;  Enables a transaction in BAR1 space.
PCITCR		equ		MBAR_PCI+$6c	; Target Control Register (32b)
LD		cfbit		PCITCR,24	;  Latency rule disable.
P		cfbit		PCITCR,16	;  Prefetch reads.
PCIIW0BTAR	equ		MBAR_PCI+$70	; Initiator Window 0 Base/Translation Address Register (32b)
W0BAR		cffield		PCIIW0BTAR,24,8	;  Window 0 Base Address
W0AM		cffield		PCIIW0BTAR,16,8	;  Window 0 Address Mask
W0TA		cffield		PCIIW0BTAR,8,8	;  Window 0 Translation Address
PCIIW1BTAR	equ		MBAR_PCI+$74	; Initiator Window 1 Base/Translation Address Register (32b)
W1BAR		cffield		PCIIW1BTAR,24,8	;  Window 1 Base Address
W1AM		cffield		PCIIW1BTAR,16,8	;  Window 1 Address Mask
W1TA		cffield		PCIIW1BTAR,8,8	;  Window 1 Translation Address
PCIIW2BTAR	equ		MBAR_PCI+$78	; Initiator Window 2 Base/Translation Address Register (32b)
W2BAR		cffield		PCIIW2BTAR,24,8	;  Window 2 Base Address
W2AM		cffield		PCIIW2BTAR,16,8	;  Window 2 Address Mask
W2TA		cffield		PCIIW2BTAR,8,8	;  Window 2 Translation Address
PCIIWCR		equ		MBAR_PCI+$80	; Initiator Window Configuration Register (32b)
W0CTRL		cffield		PCIIWCR,24,4	;  Window 0 Control
W1CTRL		cffield		PCIIWCR,16,4	;  Window 1 Control
W2CTRL		cffield		PCIIWCR,8,4	;  Window 2 Control
PCIICR		equ		MBAR_PCI+$84	; Initiator Control Register (32b)
REE		cfbit		PCIICR,26	;  Retry error enable.
IAE		cfbit		PCIICR,25	;  Initiator abort enable.
TAE		cfbit		PCIICR,24	;  Target abort enable.
MAX_RETRIES	cffield		PCIICR,0,8	;  maximum number of automatic PCI retries or master latency time-outs (write)
PCIISR		equ		MBAR_PCI+$88	; Initiator Status Register (32b)
RE		cfbit		PCIISR,26	;  Retry error.
IA		cfbit		PCIISR,25	;  Initiator abort.
TA		cfbit		PCIISR,24	;  Target abort.
PCICAR		equ		MBAR_PCI+$f8	; Configuration Address Register (32b)
E		cfbit		PCICAR,31	;  Enable.
BUS_NUMBER	cffield		PCICAR,16,8	;  Target bus of the configuration access.
DEV_NUMBER	cffield		PCICAR,11,5	;  Select a specific device on the target bus.
FNC_NUMBER	cffield		PCICAR,8,3	;  Select a specific function in the requested device.
DWORD		cffield		PCICAR,2,6	;  Select the Dword address offset in the configuration space of the target device.

; CommBus FIFO Transmit Interface Registers

PCITPSR		equ		MBAR_CBUSPCI+$00; Tx Packet Size Register (32b)
PACKET_SIZE	cffield		PCITPSR,16,16	;  Packet Size
PCITSAR		equ		MBAR_CBUSPCI+$04; Tx Start Address Register (32b)
START_ADD	cffield		PCITSAR,0,32	;  PCI Packet Start Address
PCITTCR		equ		MBAR_CBUSPCI+$08; Tx Transaction Control Register (32b)
PCI_CMD		cffield		PCITTCR,24,4	;  PCI command to present during the address phase of each PCI transaction.
MAX_RETRIES	cffield		PCITTCR,16,8	;  Maximum number of retries to permit "per packet".
MAX_BEATS	cffield		PCITTCR,8,3	;  Number of PCI data beats to attempt on each PCI transaction.
W		cfbit		PCITTCR,4	;  Word transfer.
DI		cfbit		PCITTCR,0	;  Disable address incrementing.
PCITER		equ		MBAR_CBUSPCI+$0c; Tx Enables Register (32b)
RC		cfbit		PCITER,31	;  Reset controller. User
RF		cfbit		PCITER,30	;  Reset FIFO.
CM		cfbit		PCITER,28	;  Continuous mode.
BE		cfbit		PCITER,27	;  Bus error enable.
ME		cfbit		PCITER,24	;  Master enable.
FEE		cfbit		PCITER,21	;  FIFO error enable.
SE		cfbit		PCITER,20	;  System error enable.
RE		cfbit		PCITER,19	;  Retry abort enable.
TAE		cfbit		PCITER,18	;  Target abort enable.
IAE		cfbit		PCITER,17	;  Initiator abort enable.
NE		cfbit		PCITER,16	;  Normal termination enable.
PCITNAR		equ		MBAR_CBUSPCI+$10; Tx Next Address Register (32b)
NEX_TADDRESS	cffield		PCITNAR,0,32	;  Contains the next (unwritten) PCI address.
PCITLWR		equ		MBAR_CBUSPCI+$14; Tx Last Word Register (32b)
LAST_wORD	cffield		PCITLWR,0,32	;  indicates the last 32-bit data fetched from the FIFO
PCITDCR		equ		MBAR_CBUSPCI+$18; Tx Done Counts Register (32b)
BYTES_DONE	cffield		PCITDCR,16,16	;  Indicates the number of bytes transmitted since the start of a packet.
PACKETS_DONE	cffield		PCITDCR,0,16	;  Indicates the number of previous packets transmitted.
PCITSR		equ		MBAR_CBUSPCI+$1c; Tx Status Register (32b)
NT		cfbit		PCITSR,24	;  Normal termination.
BE3		cfbit		PCITSR,23	;  Bus error type 3.
BE2		cfbit		PCITSR,22	;  Bus error type 2.
BE1		cfbit		PCITSR,21	;  Bus error type 1.
FE		cfbit		PCITSR,20	;  FIFO error.
SE		cfbit		PCITSR,19	;  System error.
RE		cfbit		PCITSR,18	;  Retry error.
TA		cfbit		PCITSR,17	;  Target abort.
IA		cfbit		PCITSR,16	;  Initiator abort.
PCITFDR		equ		MBAR_CBUSPCI+$40; Tx FIFO Data Register (32b)
FIFO_DATA_WORD	cffield		PCITFDR,0,32	;  This is the data port to the FIFO.
PCITFSR		equ		MBAR_CBUSPCI+$44; Tx FIFO Status Register (32b)
IP		cfbit		PCITFSR,31	;  Illegal Pointer.
TXW		cfbit		PCITFSR,30	;  Transmit Wait Condition.
FAE		cfbit		PCITFSR,23	;  Frame accept error.
RXW		cfbit		PCITFSR,22	;  Receive wait condition.
UF		cfbit		PCITFSR,21	;  Underflow.
OF		cfbit		PCITFSR,20	;  Overflow.
FR		cfbit		PCITFSR,19	;  Frame ready.
FULL		cfbit		PCITFSR,18	;  The FIFO is Full.
ALARM		cfbit		PCITFSR,17	;  The FIFO is at or above the Alarm "watermark".
EMPTY		cfbit		PCITFSR,16	;  The FIFO is empty.
PCITFCR		equ		MBAR_CBUSPCI+$48; Tx FIFO Control Register (32b)
WFR		cfbit		PCITFCR,29	;  Write frame.
GR		cffield		PCITFCR,24,3	;  Granularity.
IP_MASK		cfbit		PCITFCR,23	;  Illegal pointer mask.
FAE_MASK	cfbit		PCITFCR,22	;  Frame accept error mask.
RXW_MASK	cfbit		PCITFCR,21	;  Receive wait condition mask.
UF_MASK		cfbit		PCITFCR,20	;  Underflow mask.
OF_MASK		cfbit		PCITFCR,19	;  Overflow mask.
TXW_MASK	cfbit		PCITFCR,18	;  Transmit wait condition mask.
PCITFAR		equ		MBAR_CBUSPCI+$4c; Tx FIFO Alarm Register (32b)
ALARM		cffield		PCITFAR,0,12	;  Low level "watermark".
PCITFRPR	equ		MBAR_CBUSPCI+$50; Tx FIFO Read Pointer Register (32b)
READPTR		cffield		PCITFRPR,0,7	;  Read address presented to the FIFO RAM.
PCITFWPR	equ		MBAR_CBUSPCI+$54; Tx FIFO Write Pointer Register (32b)
WRITEPTR	cffield		PCITFWPR,0,7	;  Write address presented to the FIFO RAM.

; CommBus FIFO Receive Interface Registers

PCIRPSR		equ		MBAR_CBUSPCI+$80; Rx Packet Size Register (32b)
PACKET_SIZE	cffield		PCIRPSR,16,16	;  Packet Size
PCIRSAR		equ		MBAR_CBUSPCI+$84; Rx Start Address Register (32b)
START_ADD	cffield		PCIRSAR,0,32	;  Packet Starting Address
PCIRTCR		equ		MBAR_CBUSPCI+$88; Rx Transaction Control Register (32b)
PCI_CMD		cffield		PCIRTCR,24,4	;  Desired PCI command.
MAX_RETRIES	cffield		PCIRTCR,16,8	;  Maximum number of retries to permit "per packet".
FB		cfbit		PCIRTCR,12	;  Full burst.
MAX_BEATS	cffield		PCIRTCR,8,3	;  Desired number of PCI data beats to attempt on each PCI transaction.
W		cfbit		PCIRTCR,4	;  Disable the two high byte enables of the PCI bus
DI		cfbit		PCIRTCR,0	;  Disable PCI address incrementing between transactions.
PCIRER		equ		MBAR_CBUSPCI+$8c; Rx Enables Register (32b)
RC		cfbit		PCIRER,31	;  Reset controller.
RF		cfbit		PCIRER,30	;  Reset FIFO.
FE		cfbit		PCIRER,29	;  Flush enable.
CM		cfbit		PCIRER,28	;  Continuous mode.
BE		cfbit		PCIRER,27	;  Bus error enable.
ME		cfbit		PCIRER,24	;  Master enable.
FEE		cfbit		PCIRER,21	;  FIFO error enable.
SE		cfbit		PCIRER,20	;  System error enable.
RE		cfbit		PCIRER,19	;  Retry abort enable.
TAE		cfbit		PCIRER,18	;  Target abort enable.
IAE		cfbit		PCIRER,17	;  Initiator abort enable.
NE		cfbit		PCIRER,16	;  Normal termination enable.
PCIRNAR		equ		MBAR_CBUSPCI+$90; Rx Next Address Register (32b)
NEXT_ADDRESS	cffield		PCIRNAR,0,32	;  Next Address
PCIRDCR		equ		MBAR_CBUSPCI+$98; Rx Done Counts Register (32b)
BYTES_DONE	cffield		PCIRDCR,16,16	;  Bytes Done
PACKETS_DONE	cffield		PCIRDCR,0,16	;  Packets Done
PCIRSR		equ		MBAR_CBUSPCI+$9c; Rx Status Register (32b)
NT		cfbit		PCIRSR,24	;  Normal Termination.
BE3		cfbit		PCIRSR,23	;  Bus Error type 3.
BE2		cfbit		PCIRSR,22	;  Bus Error type 2.
BE1		cfbit		PCIRSR,21	;  Bus Error type 1.
FE		cfbit		PCIRSR,20	;  FIFO Error.
SE		cfbit		PCIRSR,19	;  System error.
RE		cfbit		PCIRSR,18	;  Retry Error.
TA		cfbit		PCIRSR,17	;  Target Abort.
IA		cfbit		PCIRSR,16	;  Initiator abort.
PCIRFDR		equ		MBAR_CBUSPCI+$c0; Rx FIFO Data Register (32b)
FIFO_DATA_WORD	cffield		PCIRFDR,0,32	;  FIFO data port.
PCIRFSR		equ		MBAR_CBUSPCI+$c4; Rx FIFO Status Register (32b)
IP		cfbit		PCIRFSR,31	;  Illegal Pointer.
TXW		cfbit		PCIRFSR,30	;  Transmit Wait Condition.
FAE		cfbit		PCIRFSR,23	;  Frame accept error.
RXW		cfbit		PCIRFSR,22	;  Receive Wait Condition.
UF		cfbit		PCIRFSR,21	;  UnderFlow.
OF		cfbit		PCIRFSR,20	;  OverFlow.
FR		cfbit		PCIRFSR,19	;  Frame Ready.
FULL		cfbit		PCIRFSR,18	;  The FIFO is Full.
ALARM		cfbit		PCIRFSR,17	;  The FIFO is at or above the Alarm "watermark".
EMPTY		cfbit		PCIRFSR,16	;  The FIFO is empty.
PCIRFCR		equ		MBAR_CBUSPCI+$c8; Rx FIFO Control Register (32b)
WFR		cfbit		PCIRFCR,29	;  Write frame.
GR		cffield		PCIRFCR,24,3	;  Granularity.
IP_MASK		cfbit		PCIRFCR,23	;  Illegal Pointer Mask.
FAE_MASK	cfbit		PCIRFCR,22	;  Frame accept error mask.
RXW_MASK	cfbit		PCIRFCR,21	;  Receive wait condition mask.
UF_MASK		cfbit		PCIRFCR,20	;  Underflow mask.
OF_MASK		cfbit		PCIRFCR,19	;  Overflow mask.
TXW_MASK	cfbit		PCIRFCR,18	;  Transmit wait condition mask.
PCIRFAR		equ		MBAR_CBUSPCI+$cc; Rx FIFO Alarm Register (32b)
ALARM		cffield		PCIRFAR,0,7	;  Low level watermark,
PCIRFRPR	equ		MBAR_CBUSPCI+$d0; Rx FIFO Read Pointer Register (32b)
READPTR		cffield		PCIRFRPR,0,7	;  Read address being presented to the FIFO RAM.
PCIRFWPR	equ		MBAR_CBUSPCI+$d4; Rx FIFO Write Pointer Register (32b)
WRITEPTR	cffield		PCIRFWPR,0,7	;  Write address being presented to the FIFO RAM.

;----------------------------------------------------------------------------
; PCI Arbiter

MBAR_PARB	equ		MBAR+$c00

PACR		equ		MBAR_PARB+$00	; PCI Arbiter Control Register
DS		cfbit		PACR,31		;  Disable bit for the internal PCI arbiter.
EXTMINTEN	cffield		PACR,17,5	;  External master broken interrupt enables.
INTMINTEN	cfbit		PACR,16		;  Internal master broken interrupt enable.
EXTMPRI		cffield		PACR,1,5	;  External master priority levels.
INTMPRI		cfbit		PACR,0		;  Internal master priority level.
PASR		equ		MBAR_PARB+$04	; PCI Arbiter Status Register
EXTMBK		cffield		PASR,17,5	;  External master broken.
ITLMBK		cfbit		PASR,16		;  Internal master broken.

;----------------------------------------------------------------------------
; Security Engine

		if		__has_sec
MBAR_SEC	 equ		MBAR+$20000
		 include	"coldfire/54xxsec.inc"
		endif

;----------------------------------------------------------------------------
; Multi-Channel DMA

MBAR_MDMA	equ		MBAR+$8000

TASKBAR		equ		MBAR_MDMA+$000	; Task Base Address Register (32b)
CP		equ		MBAR_MDMA+$004	; Current Pointer (32b)
EP		equ		MBAR_MDMA+$008	; End Pointer (32b)
VP		equ		MBAR_MDMA+$00c	; Variable Pointer (32b)
PTD		equ		MBAR_MDMA+$012	; PTD Control Register (16b)
PCTL15		cfbit		PTD,15		;  Task priority control.
PCTL14		cfbit		PTD,14		;  Bus error control
PCTL13		cfbit		PTD,13		;  Task arbitration control
PCTL1		cfbit		PTD,1		;  Registered request control
PCTL0		cfbit		PTD,0		;  CommBus Prefetch
DIPR		equ		MBAR_MDMA+$014	; DMA Interrupt Pending Register (32b)
DIMR		equ		MBAR_MDMA+$018	; DMA Interrupt Mask Register (32b)
__N		set		0
		rept		16
		__decstr	__NS,__N
TCR{__NS}	equ		MBAR_MDMA+$1c+(__N*2)	; Task Control Register n (16b)
EN		cfbit		TCR{__NS},15	;  Task enable.
V		cfbit		TCR{__NS},14	;  Initiator number is valid.
ALWINIT		cfbit		TCR{__NS},13	;  Decode of the always initiator.
INITNUM		cffield		TCR{__NS},8,5	;  Initiator number from task descriptor.
ASTRT		cfbit		TCR{__NS},7	;  Auto start.
HIPRITSKEN	cfbit		TCR{__NS},6	;  High-priority task enable.
HLDINITNUM	cfbit		TCR{__NS},5	;  Hold initiator number.
ASTSKNUM	cffield		TCR{__NS},0,4	;  Auto-start task number.
__N		set		__N+1
		endm
__N		set		0
		rept		32
		__decstr	__NS,__N
PRIOR{__NS}	equ		MBAR_MDMA+$3c+__N	; Priority Register n (8b)
HLD		cfbit		PRIOR{__NS},7	;  Keep current priority of initiator.
PRI		cffield		PRIOR{__NS},0,3	;  Priority level.
__N		set		__N+1
		endm
IMCR		equ		MBAR_MDMA+$05c	; InitiatorMuxControl (32b)
TSKSZ0		equ		MBAR_MDMA+$060	; Task Size Register 0 (32b)
__N		set		0
		rept		8
		__decstr	__NS,__N
TASK{__NS}.SRCSZ cffield	TSKSZ0,30-(__N*4) ; Source Size
TASK{__NS}.DSTSZ cffield	TSKSZ0,28-(__N*4) ; Dest Size
__N		set		__N+1
		endm
TSKSZ1		equ		MBAR_MDMA+$064	; Task Size Register 1 (32b)
		rept		8
		__decstr	__NS,__N
TASK{__NS}.SRCSZ cffield	TSKSZ1,62-(__N*4) ; Source Size
TASK{__NS}.DSTSZ cffield	TSKSZ1,60-(__N*4) ; Dest Size
__N		set		__N+1
		endm
DBGCOMP1	equ		MBAR_MDMA+$070	; Debug Comparator 1 (32b)
DBGCOMP2	equ		MBAR_MDMA+$074	; Debug Comparator 2 (32b)
DBGCTL		equ		MBAR_MDMA+$078	; Debug Control (32b)
BLOCKTASKS	cffield		DBGCTL,16,16	;  Specify for each of tasks 15-0, whether to block that task with detection of a breakpoint
AA		cfbit		DBGCTL,15	;  AutoArm
B		cfbit		DBGCTL,14	;  Breakpoint
COMP1TYPE	cffield		DBGCTL,11,3	;  Comparator 1 type
COMP2TYPE	cffield		DBGCTL,8,3	;  Comparator 2 type
ANDOR		cfbit		DBGCTL,7	;  AND/OR
E		cfbit		DBGCTL,2	;  Enable external breakpoint.
I		cfbit		DBGCTL,1	;  Enable internal breakpoint
DBGSTAT		equ		MBAR_MDMA+$07c	; Debug Status (32b)
I		cfbit		DBGSTAT,18	;  Interrupt.
E		cfbit		DBGSTAT,17	;  External Breakpoint.
T		cfbit		DBGSTAT,16	;  Triggered.
BLOCKED		cffield		DBGSTAT,0,16	;  Task Blocked.
PTDDBG		equ		MBAR_MDMA+$080	; PTD Debug Registers (32b)

MBAR_EREQ	equ		MBAR+$d00

__N		set		0
		rept		2
		__decstr	__NS,__N
EREQBAR{__NS}	equ		MBAR_EREQ+(__N*16)+0	; Base Address Register n (32b)
EREQMASK{__NS}	equ		MBAR_EREQ+(__N*16)+4	; Base Address Mask Register n (32b)
EREQCTRL{__NS}	equ		MBAR_EREQ+(__N*16)+8	; Control Reg n (32b)
MD		cffield		EREQCTRL{__NS},6,2	;  Mode.
BSEL		cffield		EREQCTRL{__NS},4,2	;  Bus Select.
DACKWID		cffield		EREQCTRL{__NS},2,2	;  External DMA Acknowledge Width.
SYNC		cfbit		EREQCTRL{__NS},1	;  Sync.
EN		cfbit		EREQCTRL{__NS},0	;  Enable.
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; CommBUS FIFO Interface

__deffifo	macro		PR,Base
{PR}FIFODR	equ		Base+00		; FIFO Data Register (32b)
DATA		cffield		{PR}FIFODR	;  FIFO Data
{PR}FIFOSR	equ		Base+04		; FIFO Status Register (16b)
IP		cfbit		{PR}FIFOSR,15	;  Illegal Pointer
TXW		cfbit		{PR}FIFOSR,14	;  Transmit Wait Condition
TYPE		cffield		{PR}FIFOSR,12,3	;  Frame Boundary Type Indicator
FRM		cffield		{PR}FIFOSR,8,4	;  Frame Indicator
FAE		cfbit		{PR}FIFOSR,7	;  Frame Accept Error
RXW		cfbit		{PR}FIFOSR,6	;  Receive Wait Condition
UF		cfbit		{PR}FIFOSR,5	;  FIFO Underflow
OF		cfbit		{PR}FIFOSR,4	;  FIFO Overflow
FRMRDY		cfbit		{PR}FIFOSR,3	;  Frame Ready
FU		cfbit		{PR}FIFOSR,2	;  Full
ALARM		cfbit		{PR}FIFOSR,1	;  FIFO Alarm
EMT		cfbit		{PR}FIFOSR,0	;  Empty
{PR}FIFOCR	equ		Base+08		; FIFO Control Register (32b)
SHADOW		cfbit		{PR}FIFOCR,31	;  Shadow Frame Mode Enable.
WCTL		cfbit		{PR}FIFOCR,30	;  Write Control.
WFR		cfbit		{PR}FIFOCR,29	;  Write Frame.
TIMER		cfbit		{PR}FIFOCR,28	;  Timer Mode Enable.
FRMEN		cfbit		{PR}FIFOCR,27	;  Frame Mode Enable.
GR		cffield		{PR}FIFOCR,24,3	;  Last Transfer Granularity.
IP_MASK		cfbit		{PR}FIFOCR,23	;  Illegal Pointer Mask.
FAE_MASK	cfbit		{PR}FIFOCR,22	;  Frame Accept Error Mask.
RXW_MASK	cfbit		{PR}FIFOCR,21	;  Receive Wait Condition Mask.
UF_MASK		cfbit		{PR}FIFOCR,20	;  Underflow Mask.
OF_MASK		cfbit		{PR}FIFOCR,19	;  Overflow Mask.
TXW_MASK	cfbit		{PR}FIFOCR,18	;  Masks the Status Register's TXW bit from generating an error.
COUNTER		cffield		{PR}FIFOCR,0,16	;  Timer Mode Counter
{PR}ALARMP	equ		Base+0e		; Alarm Pointer (16b)
ALARM		cffield		{PR}ALARMP,0,12
{PR}READP	equ		Base+12		; FIFO Read Pointer (16b)
READ		cffield		{PR}READP,0,12
{PR}WRITEP	equ		Base+16		; FIFO Write Pointer (16b)
WRITE		cffield		{PR}WRITEP,0,12
{PR}LRFP	equ		Base+1a		; Last Read Frame Pointer (16b)
{PR}LWFP	equ		Base+1e		; Last Write Frame Pointer (16b)
		endm

;----------------------------------------------------------------------------
; CommTimer Module

MBAR_CTM	equ		MBAR+$7f00

__N		set		0
		rept		4
		__decstr	__NS,__N
CTCR{__NS}	equ		MBAR_CTM+__N*4	; Comm Timer Control Register n - Fixed Timer Channel
I		cfbit		CTCR{__NS},31	;  Interrupt.
IM		cfbit		CTCR{__NS},24	;  Interrupt mask.
M		cfbit		CTCR{__NS},23	;  Mode.
PCT		cffield		CTCR{__NS},20,3	;  Percent active time select.
S		cffield		CTCR{__NS},16,4	;  Clock enable source select.
CRV		cffield		CTCR{__NS},0,16	;  Counter reference value.
__N		set		__N+1
		endm
		rept		4
		__decstr	__NS,__N
CTCR{__NS}	equ		MBAR_CTM+__N*4	; Comm Timer Control Register n - Variable Timer Channel
S		cfbit		CTCR{__NS},28	;  Clock enable source select.
M		cfbit		CTCR{__NS},27	;  Mode.
PCT		cffield		CTCR{__NS},24,3	;  Percent active time select.
CRV		cffield		CTCR{__NS},0,24	;  Counter reference value.
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; Programmable Serial Controller(s)

__defpsc	macro		N,Base
PSCMR1{N}	equ		Base+$00	; PSC Mode register 1 (8b)
RXRTS		cfbit		PSCMR1{N},7	;  Receiver request-to-send (UART and SIR modes only).
RXIRQ		cfbit		PSCMR1{N},6	;  Receiver interrupt select.
ERR		cfbit		PSCMR1{N},5	;  Error mode (UART mode only).
PM		cffield		PSCMR1{N},3,2	;  Parity mode (UART mode only).
PT		cfbit		PSCMR1{N},2	;  Parity type (UART mode only).
BC		cffield		PSCMR1{N},0,2	;  Bits per character (UART mode only).
PSCMR2{N}	equ		Base+$00	; PSC Mode register 2 (8b)
CM		cffield		PSCMR2{N},6,2	;  Channel mode.
TXRTS		cfbit		PSCMR2{N},5	;  Transmitter ready-to-send (UART and SIR modes).
TXCTS		cfbit		PSCMR2{N},4	;  Transmitter clear-to-send (UART and SIR modes).
SB		cffield		PSCMR2{N},0,4	;  Stop-bit length control (UART mode only).
PSCSR{N}	equ		Base+$04	; PSC Status Register (16b)
RB_NEOF		cfbit		PSCSR{N},15	;  For UART and SIR modes, this field signifies a received break.
FE_PHYERR	cfbit		PSCSR{N},14	;  For UART and SIR modes, this field signifies a framing error.
PE_CRCERR	cfbit		PSCSR{N},13	;  For UART and SIR modes, this field signifies a parity error.
OE		cfbit		PSCSR{N},12	;  This field signifies an overrun error occurred.
TXEMP_URERR	cfbit		PSCSR{N},11	;  For UART and SIR modes, this field signifies a transmitter empty.
TXRDY		cfbit		PSCSR{N},10	;  This field signifies a Transmitter ready.
FU		cfbit		PSCSR{N},9	;  This field signifies that the RxFIFO is full.
RXRDY		cfbit		PSCSR{N},8	;  This field signifies a Receiver ready.
CDE_DEOF	cfbit		PSCSR{N},7	;  In MIR and FIR mode, this bit signifies Detect End of Frame or the RxFIFO contains EOF.
ERR		cfbit		PSCSR{N},6	;  Error bit.
PSCCSR{N}	equ		Base+$04	; PSC Clock Select Register (8b)
RCSEL		cffield		PSCCSR{N},4,4	;  In UART or SIR mode, this is the receiver clock select.
TCSEL		cffield		PSCCSR{N},0,4	;  In UART or SIR mode, this is the transmitter clock select.
PSCCR{N}	equ		Base+$08	; PSC Command Register (8b)
MISC		cffield		PSCCR{N},4,3	;  Misc Commands
TXC		cffield		PSCCR{N},2,2	;  Receiver Command
RXC		cffield		PSCCR{N},0,2	;  Transmitter Command
PSCRB{N}	equ		Base+$0c	; PSC Receive Buffer (32b)
RB		cffield		PSCRB{N},0,32	;  Received data.
PSCTB{N}	equ		Base+$0c	; PSC Transmit Buffer (32b)
TB		cffield		PSCTB{N},0,32	;  Transmitted data.
PSCIPCR{N}	equ		Base+$10	; PSC Input Port Change Register (8b)
SYNC		cfbit		PSCIPCR{N},7	;  For modem modes, this bit signifies Sync is detected or not.
D_CTS		cfbit		PSCIPCR{N},4	;  Delta CTS
CTS		cfbit		PSCIPCR{N},0	;  Current state of PSCnCTS port.
PSCACR{N}	equ		Base+$10	; PSC Auxiliary Control Register (8b)
IEC0		cfbit		PSCACR{N},0	;  Interrupt enable control for D_CTS.
PSCISR{N}	equ		Base+$14	; PSC Interrupt Status Register (16b)
IPC		cffield		PSCISR{N},15	;  Input port change.
DB		cffield		PSCISR{N},10	;  In UART / SIR, this is a Delta break.
RXRDY_FU	cffield		PSCISR{N},9	;  RxFIFO over threshold/Receive data is ready.
TXRDY		cffield		PSCISR{N},8	;  Transmitter ready
DEOF		cffield		PSCISR{N},7	;  For SIR and MIR modes, this bit signifies detect end of frame or the RxFIFO contains EOF.
ERR		cffield		PSCISR{N},6	;  OR of all errors status including FIFO errors.
PSCIMR{N}	equ		Base+$14	; PSC Interrupt Mask Register (16b)
IPC		cffield		PSCIMR{N},15	;  Input port change.
DB		cffield		PSCIMR{N},10	;  In UART / SIR, this is a Delta break.
RXRDY_FU	cffield		PSCIMR{N},9	;  RxFIFO over threshold/Receive data is ready.
TXRDY		cffield		PSCIMR{N},8	;  Transmitter ready
DEOF		cffield		PSCIMR{N},7	;  For SIR and MIR modes, this bit signifies detect end of frame or the RxFIFO contains EOF.
ERR		cffield		PSCIMR{N},6	;  OR of all errors status including FIFO errors.
PSCCTUR{N}	equ		Base+$18	; PSC Counter Timer Upper Register (8b)
PSCCTLR{N}	equ		Base+$18	; PSC Counter Timer Lower Register (8b)
PSCIP{N}	equ		Base+$34	; PSC Input Port (8b)
LPWR_B		cfbit		PSCIP{N},7	;  In AC97 mode, this bit signifies the low power mode.
TGL		cfbit		PSCIP{N},6	;  In AC97 and modem modes, this bit signifies test usage.
CTS		cfbit		PSCIP{N},0	;  Current state of the PSCnCTS input
PSCOPSET{N}	equ		Base+$38	; PSC Output Port Set (8b)
RTS		cfbit		PSCOPSET{N},0	;  Assert /PSCnRTS output
PSCOPRESET{N}	equ		Base+$3c	; PSC Output Port Reset (8b)
RTS		cfbit		PSCOPRESET{N},0	;  Negate /PSCnRTS output
PSCSICR{N}	equ		Base+$40	; PSC PSC / IrDA Control Register (8b)
ACRB		cfbit		PSCSICR{N},7	;  In AC97 mode, this bit signifies Cold Reset to the transceiver in PSC.
AWR		cfbit		PSCSICR{N},6	;  In AC97 mode, this bit signifies Warm Reset (to the transceiver in PSC and AC97 CODEC).
DTS1		cfbit		PSCSICR{N},5	;  In modem modes, this bit signifies delay of time slot 1.
SHDIR		cfbit		PSCSICR{N},4	;  In modem modes, this bit signifies Shift Direction.
SIM		cffield		PSCSICR{N},0,3	;  PSC/IrDA operation mode.
PSCIRCR1{N}	equ		Base+$44	; PSC IrDA Control Register 1 (8b)
FD		cfbit		PSCIRCR1{N},2	;  In MIR, FIR, SIR, and modem modes, this bit signifies full duplex enable.
SIPEN		cfbit		PSCIRCR1{N},1	;  In MIR, FIR, and modem mode, this bit signifies sends SIP enable after every frame.
SPUL		cfbit		PSCIRCR1{N},0	;  In SIR mode, this bit signifies SIR pulse width.
PSCIRCR2{N}	equ		Base+$48	; PSC IrDA Control Register 2 (8b)
SIPREQ		cfbit		PSCIRCR2{N},2	;  In MIR and FIR mode, this bit signifies request to send SIP.
ABORT		cfbit		PSCIRCR2{N},1	;  In MIR and FIR mode, this bit signifies abort output.
NXTEOF		cfbit		PSCIRCR2{N},0	;  In MIR and FIR mode, this bit signifies next is the last byte.
PSCIRSDR{N}	equ		Base+$4c	; PSC IrDA SIR Divide Register (8b)
IRSTIM		cffield		PSCIRSDR{N},0,8	;  In SIR mode, this field signifies the timer counter value for 1.6 us pulse.
PSCIRMDR{N}	equ		Base+$50	; PSC IrDA MIR Divide Register (8b)
FREQ		cfbit		PSCIRMDR{N},7	;  In MIR mode, this bit signifies 0.576 Mbps mode.
M_FDIV		cffield		PSCIRMDR{N},0,7	;  In MIR mode, this bit signifies clock divide ratio.
PSCIRFDR{N}	equ		Base+$54	; PSC IrDA FIR Divide Register (8b)
F_FDIV		cffield		PSCIRFDR{N},0,4	;  In FIR mode, this field signifies clock divide ratio.
PSCRFCNT{N}	equ		Base+$58	; PSC RxFIFO Counter Register (16b)
CNT		cffield		PSCRFCNT{N},0,9	;  Number of bytes in the FIFO
PSCTFCNT{N}	equ		Base+$5c	; PSC TxFIFO Counter Register (16b)
CNT		cffield		PSCTFCNT{N},0,9	;  Number of bytes in the FIFO
PSCRFDR{N}	equ		Base+$60	; PSC RxFIFO Data Register (32b)
PSCRFSR{N}	equ		Base+$64	; PSC RxFIFO Status Register (16b)
PSCRFCR{N}	equ		Base+$68	; PSC RxFIFO Control Register (16b)
PSCRFAR{N}	equ		Base+$6e	; PSC RxFIFO Alarm Register (16b)
PSCRFRP{N}	equ		Base+$72	; PSC RxFIFO Read Pointer (16b)
PSCRFWP{N}	equ		Base+$76	; PSC RxFIFO Write Pointer (16b)
PSCRLRFP{N}	equ		Base+$7a	; PSC RxFIFO Last Read Frame Pointer (16b)
PSCRLWFP{N}	equ		Base+$7e	; PSC RxFIFO Last Write Frame Pointer (16b)
PSCTFDR{N}	equ		Base+$80	; PSC TxFIFO Data Register (32b)
PSCTFSR{N}	equ		Base+$84	; PSC TxFIFO Status Register (16b)
PSCTFCR{N}	equ		Base+$88	; PSC TxFIFO Control Register (32b)
PSCTFAR{N}	equ		Base+$8e	; PSC TxFIFO Alarm Register (16b)
PSCTFRP{N}	equ		Base+$92	; PSC TxFIFO Read Pointer (16b)
PSCTFWP{N}	equ		Base+$96	; PSC TxFIFO Write Pointer (16b)
PSCTLRFP{N}	equ		Base+$9a	; PSC TxFIFO Last Read Frame Pointer (16b)
PSCTLWFP{N}	equ		Base+$9e	; PSC TxFIFO Last Write Frame Pointer (16b)
		irp		Reg,PSCRFSR{N},PSCTFSR{N}
IP		 cfbit		Reg,15		;  Illegal pointer.
TXW		 cfbit		Reg,14		;  Transmit wait condition.
TAG		 cffield	Reg,12,2	;  Holds the last read tag information.
FRM		 cffield	Reg,8,4		;  Frame indicator.
FAE		 cfbit		Reg,7		;  Frame accept error.
RXW		 cfbit		Reg,6		;  Receive wait condition.
UF		 cfbit		Reg,5		;  FIFO underflow.
OF		 cfbit		Reg,4		;  FIFO Overflow.
FRMRDY		 cfbit		Reg,3		;  Frame ready.
FU		 cfbit		Reg,2		;  FIFO full alarm.
ALARM		 cfbit		Reg,1		;  Alarm.
EMT		 cfbit		Reg,0		;  FIFO empty.
		endm
		irp		Reg,PSCRFCR{N},PSCTFCR{N}
WFR		 cfbit		Reg,29		;  Write frame.
TIMER		 cfbit		Reg,28		;  Timer mode enable.
FRMEN		 cfbit		Reg,27		;  Frame mode enable
GR		 cffield	Reg,24,3	;  Granularity
IP_MSK		 cfbit		Reg,23		;  Illegal pointer mask.
FAE_MSK		 cfbit		Reg,22		;  Frame accept error mask.
RXW_MSK		 cfbit		Reg,21		;  Receive wait condition mask.
UF_MSK		 cfbit		Reg,20		;  FIFO underflow mask.
OF_MSK		 cfbit		Reg,19		;  FIFO overflow mask.
TXW_MSK		 cfbit		Reg,18		;  Transmit wait condition mask.
CNTR		 cffield	Reg,0,16	;  Timer mode counter.
		endm
		irp		Reg,PSCRFAR{N},PSCTFAR{N}
ALARM		 cffield	Reg,0,9		;  Alarm pointer.
		endm
		irp		Reg,PSCRFRP{N},PSCTFRP{N}
READ		 cffield	Reg,0,9		;  Read pointer.		
		endm
		irp		Reg,PSCRFWP{N},PSCTFWP{N}
WRITE		 cffield	Reg,0,9		;  Write pointer.
		endm
		irp		Reg,PSCRLRFP{N},PSCTLRFP{N}
LRFP		 cffield	Reg,0,9		;  Last read frame pointer.
		endm
		irp		Reg,PSCRLWFP{N},PSCTLWFP{N}
LWFP		 cffield	Reg,0,9		;  Last write frame pointer.
		endm
		endm

		__defpsc		"0",MBAR+$8600
		__defpsc		"1",MBAR+$8700
		__defpsc		"2",MBAR+$8800
		__defpsc		"3",MBAR+$8900

;----------------------------------------------------------------------------
; DMA Serial Peripheral Interface (similar to 52277, different names)

MBAR_DSPI	equ		MBAR+$8a00

DMCR		equ		MBAR_DSPI+$000	; DSPI module configuration register (32b)
MSTR		cfbit		DMCR,31		;  Master/slave mode select.
CSCK		cfbit		DMCR,30		;  Continuous DSPISCK enable.
DCONF		cffield		DMCR,28,2	;  DSPI configuration.
FRZ		cfbit		DMCR,27		;  Freeze.
MTFE		cfbit		DMCR,26		;  Modified timing format enable.
PCSSE		cfbit		DMCR,25		;  Peripheral chip select strobe enable.
ROOE		cfbit		DMCR,24		;  Receive FIFO overflow overwrite enable.
CSIS5		cfbit		DMCR,21		;  Chip select n inactive state.
CSIS3		cfbit		DMCR,19
CSIS2		cfbit		DMCR,18
CSIS0		cfbit		DMCR,16
DTXF		cfbit		DMCR,13		;  Disable transmit FIFO.
DRXF		cfbit		DMCR,12		;  Disable receive FIFO.
CTXF		cfbit		DMCR,11		;  Clear TX FIFO.
CRXF		cfbit		DMCR,10		;  Clear RX FIFO.
SMPL_PT		cffield		DMCR,8,2	;  Sample point.
HALT		cfbit		DMCR,0		;  Halt.
DTCR		equ		MBAR_DSPI+$008	; DSPI transfer count register (32b)
SPI_TCNT	cffield		DTCR,16,16	;  SPI transfer counter.
__N		set		0
		rept		8
		__decstr	__NS,__N
DCTAR{"\{__NS}"}	equ	MBAR_DSPI+$00C+(__N*4)	; DSPI clock and transfer attributes registers (8*32b)
TRSZ		cffield		DCTAR{"\{__NS}"},27,4	;  Transfer size.
CPOL		cfbit		DCTAR{"\{__NS}"},26	;  Clock polarity.
CPHA		cfbit		DCTAR{"\{__NS}"},25	;  Clock phase.
LSBFE		cfbit		DCTAR{"\{__NS}"},24	;  LSB first enable.
PCSSCK		cffield		DCTAR{"\{__NS}"},22,2	;  CS to SCK delay prescaler.
PASC		cffield		DCTAR{"\{__NS}"},20,2	;  After SCK delay prescaler.
PDT		cffield		DCTAR{"\{__NS}"},18,2	;  Delay after transfer prescaler.
PBR		cffield		DCTAR{"\{__NS}"},16,2	;  Baud rate prescaler.
CSSCK		cffield		DCTAR{"\{__NS}"},12,4	;  PCS to SCK delay scaler.
ASC		cffield		DCTAR{"\{__NS}"},8,4	;  After SCK delay scaler.
DT		cffield		DCTAR{"\{__NS}"},4,4	;  Delay after transfer scaler.
BR		cffield		DCTAR{"\{__NS}"},0,4	;  Baud rate scaler.
__N		set		__N+1
		endm
DSR		equ		MBAR_DSPI+$02C	; DSPI status register (32b)
TCF		cfbit		DSR,31		;  Transfer complete flag.
TXRXS		cfbit		DSR,30		;  TX and RX status.
EOQF		cfbit		DSR,28		;  End of queue flag.
TFUF		cfbit		DSR,27		;  Transmit FIFO underflow flag.
TFFF		cfbit		DSR,25		;  Transmit FIFO fill flag.
RFOF		cfbit		DSR,19		;  Receive FIFO overflow flag.
RFDF		cfbit		DSR,17		;  Receive FIFO drain flag.
TXCTR		cffield		DSR,12,4	;  TX FIFO counter.
TXNXTPTR	cffield		DSR,8,4		;  Transmit next pointer.
RXCTR		cffield		DSR,4,4		;  RX FIFO counter.
POPNXTPTR	cffield		DSR,0,4		;  Pop next pointer.
DIRSR		equ		MBAR_DSPI+$030	; DSPI DMA/interrupt request select and enable register (32b)
TCFE		cfbit		DIRSR,31	;  Transmission complete interrupt enable.
EOQFE		cfbit		DIRSR,28	;  End of queue flag interrupt enable.
TFUFE		cfbit		DIRSR,27	;  Transmit FIFO underflow interrupt enable.
TFFFE		cfbit		DIRSR,25	;  Transmit FIFO fill interrupt enable.
TFFFS		cfbit		DIRSR,24	;  Transmit FIFO fill DMA or interrupt request select.
RFOFRE		cfbit		DIRSR,19	;  Receive FIFO overflow interrupt enable.
RFDFE		cfbit		DIRSR,17	;  Receive FIFO drain interrupt enable.
RFDFS		cfbit		DIRSR,16	;  Receive FIFO drain DMA or interrupt request select.
DTFR		equ		MBAR_DSPI+$034	; DSPI TX FIFO register (32b)
CONT		cfbit		DTFR,31		;  Continuous peripheral chip select enable.
CTAS		cffield		DTFR,28,3	;  Clock and transfer attributes select.
EOQ		cfbit		DTFR,27		;  End of queue.
CTCNT		cfbit		DTFR,26		;  Clear SPI_TCNT.
CS5		cfbit		DTFR,21		;  Cip select n.
CS3		cfbit		DTFR,19
CS2		cfbit		DTFR,18
CS0		cfbit		DTFR,16
TXDATA		cffield		DTFR,0,16	; Transmit data.
DRFR		equ		MBAR_DSPI+$038	; DSPI RX FIFO register (32b)
RXDATA		cffield		DRFR,0,16	;  Received data.
__DSPI_TXRX	macro
DTFDR{"\{__NS}"}	equ	MBAR_DSPI+$03C+(__N*4) ; DSPI Tx FIFO Debug Registers (32b)
TXCMD		cffield		DTFDR{"\{__NS}"},16,16 ;  Transmit command.
TXDATA		cffield		DTFDR{"\{__NS}"},0,16  ;  Transmit data.
DRFDR{"\{__NS}"}	equ	MBAR_DSPI+$07C+(__N*4) ; DSPI Rx FIFO Debug Registers (32b)
RXDATA		cffield		DRFDR{"\{__NS}"},0,16  ;  Receive data.
		endm
		__enumregs	__DSPI_TXRX,0,15

;----------------------------------------------------------------------------
; I2C

MBAR_I2C	equ		MBAR+$8f00
		include 	"coldfire/52xxi2c.inc"
		
I2ICR		equ		MBAR_I2C+$20	; I2C Interrupt Control Register (8b)
BNBE		cfbit		I2ICR,3		;  Permits I2C module to generate an interrupt when the bus is NOT busy.
TE		cfbit		I2ICR,2		;  Routes the interrupt for the I2C module to the TX requestor at the multichannel DMA.
RE		cfbit		I2ICR,1		;  Routes the interrupt for the I2C module to the RX requestor at the multichannel DMA.
IE		cfbit		I2ICR,0		;  Routes the interrupt for the I2C module to the CPU.

;----------------------------------------------------------------------------
; USB 2.0

                if              __has_usb
MBAR_USB2	 equ		MBAR+$b000
		 include	"coldfire/54xxusb2.inc"
		endif

;----------------------------------------------------------------------------
; FEC

		include		"coldfire/54xxfec.inc"
		__def54fec	"FEC0.",MBAR+$9000
		if		__has_fec1
		 __def54fec	"FEC1.",MBAR+$9800
		endif

;----------------------------------------------------------------------------

                restore				; re-enable listing

                endif				; __mcf547xinc
