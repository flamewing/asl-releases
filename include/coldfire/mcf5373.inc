 		ifndef	__mcf5373inc		; avoid multiple inclusion
__mcf5373inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5373.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5373                  *
;*                                                                          *
;****************************************************************************

MBAR		equ		$fc000000

;----------------------------------------------------------------------------
; Clock Module (same as 5329)

MBAR_CLK	equ		MBAR+$c0000

PODR		equ		MBAR_CLK+0	; PLL Output Divider Register (8b)
BUSDIV		cffield		PODR,0,4	;  Divider for generating the internal bus frequency
CPUDIV		cffield		PODR,4,4	;  Divider for generating the core frequency
PCR		equ		MBAR_CLK+4	; PLL Control Register (8b)
DITHEN		cfbit		PCR,7		;  Dithering enable bit
DITHDEV		cffield		PCR,0,3		;  Dither Deviation
PMDR		equ		MBAR_CLK+8	; PLL Modulation Divider Register (8b)
MODDIV		cffield		PMDR,0,8	;  Dither Modulation Divider
PFDR		equ		MBAR_CLK+12	; PLL Feedback Divider Register (8b)
MFD		cffield		PFDR,0,8	;  Feedback Bits

;----------------------------------------------------------------------------
; Power Management

WCR		equ		MBAR+$40013	; Wakeup Control Register (8b)
ENBWCR		cfbit		WCR,7		;  Enable low-power mode entry
PRILVL		cffield		WCR,0,3		;  Exit low-power mode interrupt priority level
PPMSR0		equ		MBAR+$4002c	; Peripheral Power Management Set Register 0 (8b)
SAMCD		cfbit		PPMSR0,6	;  Set all module clock disables
SMCD		cffield		PPMSR0,0,6	;  Set module clock disable
PPMCR0		equ		MBAR+$4002d	; Peripheral Power Management Clear Register 0 (8b)
CAMCD		cfbit		PPMCR0,6	;  Clear all module clock disables
CMCD		cffield		PPMCR0,0,6	;  Clear module clock disable
PPMSR1		equ		MBAR+$4002e	; Peripheral Power Management Set Register 1 (8b)
SAMCD		cfbit		PPMSR1,6	;  Set all module clock disables
SMCD		cffield		PPMSR1,0,6	;  Set module clock disable
PPMCR1		equ		MBAR+$4002f	; Peripheral Power Management Clear Register 1 (8b)
CAMCD		cfbit		PPMCR1,6	;  Clear all module clock disables
CMCD		cffield		PPMCR1,0,6	;  Clear module clock disable
PPMHR0		equ		MBAR+$40030	; Peripheral Power Management High Register 0 (32b)
CD48		cfbit		PPMHR0,16	;  PLL
CD47		cfbit		PPMHR0,15	;  SSI
CD46		cfbit		PPMHR0,14	;  SDRAM Controller
CD45		cfbit		PPMHR0,13	;  USB Host
CD44		cfbit		PPMHR0,12	;  USB On-the-Go
CD42		cfbit		PPMHR0,10	;  Real Time Clock
CD41		cfbit		PPMHR0,9	;  GPIO Module
CD40		cfbit		PPMHR0,8	;  CCM, Reset Controller, Power Management
CD38		cfbit		PPMHR0,6	;  On-chip Watchdog Timer
CD37		cfbit		PPMHR0,5	;  Edge Port
CD36		cfbit		PPMHR0,4	;  PWM
CD35		cfbit		PPMHR0,3	;  PIT 3
CD34		cfbit		PPMHR0,2	;  PIT 2
CD33		cfbit		PPMHR0,1	;  PIT 1
CD32		cfbit		PPMHR0,0	;  PIT 0
PPMLR0		equ		MBAR+$40034	; Peripheral Power Management Low Register 0 (32b)
CD31		cfbit		PPMLR0,31	;  DMA Timer 3
CD30		cfbit		PPMLR0,30	;  DMA Timer 2
CD29		cfbit		PPMLR0,29	;  DMA Timer 1
CD28		cfbit		PPMLR0,28	;  DMA Timer 0
CD26		cfbit		PPMLR0,26	;  UART2
CD25		cfbit		PPMLR0,25	;  UART1
CD24		cfbit		PPMLR0,24	;  UART0
CD23		cfbit		PPMLR0,23	;  QSPI
CD22		cfbit		PPMLR0,22	;  I2C
CD21		cfbit		PPMLR0,21	;  IACK
CD19		cfbit		PPMLR0,19	;  Interrupt Controller 1
CD18		cfbit		PPMLR0,18	;  Interrupt Controller 0
CD17		cfbit		PPMLR0,17	;  eDMA Controller
CD12		cfbit		PPMLR0,12	;  FEC
CD8		cfbit		PPMLR0,8	;  FlexCAN
CD2		cfbit		PPMLR0,2	;  FlexBus
PPMHR1		equ		MBAR+$40038	; Peripheral Power Management High Register 1 (32b)
CD34		cfbit		PPMHR1,2	;  Random Number Generator
CD33		cfbit		PPMHR1,1	;  SKHA
CD32		cfbit		PPMHR1,0	;  MDHA
LPCR		equ		MBAR+$a0007	; Low-Power Control Register (8b)
LPMD		cffield		LPCR,6,2	;  Low-power mode select
FWKUP		cfbit		LPCR,5		;  Fast wake-up
STPMD		cffield		LPCR,3,2	;  FB_CLK stop mode bits
MISCCR		equ		MBAR+$a0010	; Miscellaneous Control Register (16b)
PLLLOCK		cfbit		MISCCR,13	;  PLL lock status
LIMP		cfbit		MISCCR,12	;  Limp mode enable
SSIPUE		cfbit		MISCCR,7	;  SSI RXD/TXD pull enable.
SSIPUS		cfbit		MISCCR,6	;  SSI RXD/TXD pull select.
TIMDMA		cfbit		MISCCR,5	;  Timer DMA mux selection.
SSISRC		cfbit		MISCCR,4	;  SSI clock source.
USBDIV		cfbit		MISCCR,1	;  USB clock divisor.
USBSRC		cfbit		MISCCR,0	;  USB clock source.
CDR		equ		MBAR+$a0012	; Clock Divider Register (16b)
LPDIV		cffield		CDR,8,4		;  Low power clock divider.
SSIDIV		cffield		CDR,0,6		;  SSI baud clock divider.

;----------------------------------------------------------------------------
; Chip Configuration Module

MBAR_CCM	equ		MBAR+$a0000

CCR		equ		MBAR_CCM+$4	; Chip Configuration Register (16b)
CSC		cffield		CCR,8,2		;  Chip select configuration field
LIMP		cfbit		CCR,6		;  Limp mode
LOAD		cfbit		CCR,5		;  Pad driver load
BOOTPS		cffield		CCR,3,2		;  Boot port size
OSCMODE		cfbit		CCR,2		;  Oscillator clock mode
PLLMODE		cfbit		CCR,1		;  PLL clock mode
RCON		equ		MBAR_CCM+$8	; Reset Configuration Register (16b)
CSC		cfbit		RCON,8,2	;  Chip select configuration field
LIMP		cfbit		RCON,6		;  Limp mode
LOAD		cfbit		RCON,5		;  Pad driver load
BOOTPS		cffield		RCON,3,2	;  Boot port size
OSCMODE		cfbit		RCON,2		;  Oscillator clock mode
PLLMODE		cfbit		RCON,1		;  PLL clock mode
CIR		equ		MBAR_CCM+$a	; Chip Identification Register (16b)
PIN		cffield		CIR,6,10	;  Part identification number
PRN		cffield		CIR,0,6		;  Part revision number
UHCSR		equ		MBAR_CCM+$14	; USB Host Controller Status Register (16b)
PORTIND		cffield		UHCSR,14,2	;  USB port indicator.
WKUP		cfbit		UHCSR,2		;  USB host controller wakeup event.
UHMIE		cfbit		UHCSR,1		;  USB host miscellaneous interrupt enable.
XPDE		cfbit		UHCSR,0		;  On-chip transceiver pull-down enable
UOCSR		equ		MBAR_CCM+$16	; USB On-the-Go Controller Status Register (16b)
PORTIND		cffield		UOCSR,14,2	;  USB port indicator.
DPPD		cfbit		UOCSR,13	;  D+ 15 kOhm pull-down.
DMPD		cfbit		UOCSR,12	;  D- 15 kOhm pull-down.
DRV_VBUS	cfbit		UOCSR,11	;  Drive VBUS.
CRG_VBUS	cfbit		UOCSR,10	;  Charge VBUS.
DCR_VBUS	cfbit		UOCSR,9		;  Discharge VBUS.
DPPU		cfbit		UOCSR,8		;  D+ pull-up control.
AVLD		cfbit		UOCSR,7		;  A-peripheral is valid.
BVLD		cfbit		UOCSR,6		;  B-peripheral is valid.
VVLD		cfbit		UOCSR,5		;  VBUS valid.
SEND		cfbit		UOCSR,4		;  Session end.
PWRFLT		cfbit		UOCSR,3		;  VBUS power fault.
WKUP		cfbit		UOCSR,2		;  USB OTG controller wakeup event.
UOMIE		cfbit		UOCSR,1		;  USB OTG miscellaneous interrupt enable.
XPDE		cfbit		UOCSR,0		;  On-chip transceiver pull-down enable

;----------------------------------------------------------------------------
; Reset Controller Module (like 5329)

MBAR_RCM	equ		MBAR+$a0000
RCR		equ		MBAR_RCM+0	; Reset Control Register (8b)
SOFTRST		cfbit		RCR,7		;  Allows software to request a reset
FRCRSTOUT	cfbit		RCR,6		;  Allows software to assert or negate the external /RSTOUT pin
RSR		equ		MBAR_RCM+1	; Reset Status Register (8b)
SOFT		cfbit		RSR,5		;  Software reset flag
WDRCHIP		cfbit		RSR,4		;  On-chip watchdog timer reset flag
POR		cfbit		RSR,3		;  Power-on reset flag
EXT		cfbit		RSR,2		;  External reset flag
WDRCORE		cfbit		RSR,1		;  Core watchdog timer reset flag
LOL		cfbit		RSR,0		;  Loss-of-lock reset flag

;----------------------------------------------------------------------------
; System Control Module

MBAR_SCM	equ		MBAR+$0000

__defprot	macro		{INTLABEL},Reg,Startbit
__LABEL__	cffield		Reg,Startbit,4
__LABEL__.MTR	cfbit		Reg,Startbit+0	;   Master trusted for read
__LABEL__.MTW	cfbit		Reg,Startbit+1	;   Master trusted for writes
__LABEL__.MPL	cfbit		Reg,Startbit+2	;   Master privilege level
		endm
MPR1		equ		$ec000000 	; Master Privilege Register 1 (32b)
MPROT0		__defprot	MPR1,28		;  MDHA
MPROT1		__defprot	MPR1,24		;  SKHA
MPROT2		__defprot	MPR1,20		;  RNG
MPROT5		__defprot	MPR1,8
MPROT6		__defprot	MPR1,4
MPR0		equ		MBAR_SCM+0	; Master Privilege Register 0 (32b)
MPROT0		__defprot	MPR0,28		;  ColdFire Core
MPROT1		__defprot	MPR0,24		;  eDMA Controller
MPROT2		__defprot	MPR0,20		;  FEC
MPROT5		__defprot	MPR0,8		;  USB Host
MPROT6		__defprot	MPR0,4		;  USB On-the-Go

__defpacr	macro		{INTLABEL},Reg,Startbit
__LABEL__	cffield		Reg,Startbit,4
__LABEL__.TP	cfbit		Reg,Startbit+0	;   Trusted Protect
__LABEL__.WP	cfbit		Reg,Startbit+1	;   Write protect
__LABEL__.SP	cfbit		Reg,Startbit+2	;   Supervisor protect.
		endm
PACRA		equ		MBAR_SCM+$20 	; Peripheral Access Control Register A (32b)
PACR0		__defpacr	PACRA,28	;  SCM (MPR & PACRs)
PACR1		__defpacr	PACRA,24	;  Cross-Bar Switch
PACR2		__defpacr	PACRA,20	;  FlexBus
PACRB		equ		MBAR_SCM+$24 	;  Peripheral Access Control Register B (32b)
PACR8		__defpacr	PACRB,28	;  FlexCAN
PACR12		__defpacr	PACRB,12	;  FEC
PACRC		equ		MBAR_SCM+$28 	; Peripheral Access Control Register C (32b)
PACR16		__defpacr	PACRC,28	;  SCM (CWT & Core Fault Registers)
PACR17		__defpacr	PACRC,24	;  eDMA Controller
PACR18		__defpacr	PACRC,20	;  Interrupt Controller 0
PACR19		__defpacr	PACRC,16	;  Interrupt Controller 1
PACR21		__defpacr	PACRC,8		;  Interrupt Controller IACK
PACR22		__defpacr	PACRC,4		;  I2C
PACR23		__defpacr	PACRC,0		;  QSPI
PACRD		equ		MBAR_SCM+$2C 	; Peripheral Access Control Register D (32b)
PACR24		__defpacr	PACRD,28	;  UART0
PACR25		__defpacr	PACRD,24	;  UART1
PACR26		__defpacr	PACRD,20	;  UART2
PACR28		__defpacr	PACRD,12	;  DMA Timer 0
PACR29		__defpacr	PACRD,8		;  DMA Timer 1
PACR30		__defpacr	PACRD,4		;  DMA Timer 2
PACR31		__defpacr	PACRD,0		;  DMA Timer 3
PACRE		equ		MBAR_SCM+$40 	; Peripheral Access Control Register E (32b)
PACR32		__defpacr	PACRE,28	;  PIT 0
PACR33		__defpacr	PACRE,24	;  PIT 1
PACR34		__defpacr	PACRE,20	;  PIT 2
PACR35		__defpacr	PACRE,16	;  PIT 3
PACR36		__defpacr	PACRE,12	;  PWM
PACR37		__defpacr	PACRE,8		;  Edge Port
PACR38		__defpacr	PACRE,4		;  On-chip Watchdog Timer
PACRF		equ		MBAR_SCM+$44 	; Peripheral Access Control Register F (32b)
PACR40		__defpacr	PACRF,28	;  CCM, Reset Controller, Power Management
PACR41		__defpacr	PACRF,24	;  GPIO Module
PACR42		__defpacr	PACRF,20	;  Real Time Clock
PACR44		__defpacr	PACRF,12	;  USB On-the-Go
PACR45		__defpacr	PACRF,8		;  USB Host
PCAR46		__defpacr	PACRF,4		;  SDRAM Controller
PACR47		__defpacr	PACRF,0		;  SSI
PACRG		equ		MBAR_SCM*48	; Peripheral Access Control Register G (32b)
PACR48		__defpacr	PACRG,28	;  PLL
PACRH		equ		$ec000040	; Peripheral Access Control Register H (32b)
PACR56		__defpacr	PACRH,28	;  MDHA
PACR57		__defpacr	PACRH,24	;  SKHA
PACR58		__defpacr	PACRH,20	;  RNG
BMT1		equ		$ec000054	; Bus Monitor Timeout 1 (32b)
BMT		cffield		BMT1,0,3	;  Bus Monitor Timeout Period
BME		cfbit		BMT1,3		;  Bus Monitor Timeout Enable
BMT0		equ		MBAR_SCM+$54 	; Bus Monitor Timeout 0 (32b)
BMT		cffield		BMT0,0,3	;  Bus Monitor Timeout Period
BME		cfbit		BMT0,3		;  Bus Monitor Timeout Enable
CWCR		equ		MBAR_SCM+$40016	; Core Watchdog Control Register (16b)
RO		cfbit		CWCR,15		;  Read-Only Control
CWRWH		cfbit		CWCR,8		;  Core Watchdog run while halted
CWE		cfbit		CWCR,7		;  Core Watchdog Timer Enable
CWRI		cffield		CWCR,5,2	;  Core Watchdog Reset/Interrupt
CWT		cffield		CWCR,0,5	;  Core Watchdog Time-Out Period
CWSR		equ		MBAR_SCM+$4001B	; Core Watchdog Service Register (8b)
SCMISR		equ		MBAR_SCM+$4001F	; SCM Interrupt Status Register (8b)
CFEI		cfbit		SCMISR,1	;  Core Fault Error Interrupt Flag
CWIC		cfbit		SCMISR,0	;  Core Watchdog Interrupt Flag
BCR		equ		MBAR_SCM+$40024	; Burst Configuration Register (32b)
GBR		cfbit		BCR,9		;  Global burst enable for reads.
GBW		cfbit		BCR,8		;  Global burst enable for writes.
SBE		cffield		BCR,0,8		;  Slave burst enable.
CFADR		equ		MBAR_SCM+$40070	; Core Fault Address Register (32b)
CFIER		equ		MBAR_SCM+$40075	; Core Fault Interrupt Enable Register (8b)
ECFEI		cfbit		CFIER,0		;  Enable Core Fault Error Interrupt
CFLOC		equ		MBAR_SCM+$40076	; Core Fault Location Register (8b)
LOC		cfbit		CFLOC,7		;  Location of the last captured fault
CFATR		equ		MBAR_SCM+$40077	; Core Fault Attributes Register (8b)
WRITE		cfbit		CFATR,7		;  Direction of the last faulted core access
SIZE		cffield		CFATR,4,2	;  Size of the last faulted core access
CACHE		cfbit		CFATR,3		;  Indicates if last faulted core access was cacheable
MODE		cfbit		CFATR,1		;  Indicates the mode the device was in during the last faulted core access
TYPE		cfbit		CFATR,0		;  Defines the type of last faulted core access
CFDTR		equ		MBAR_SCM+$4007C	; Core Fault Data Register (32b)

;----------------------------------------------------------------------------
; Crossbar Switch

MBAR_XBS	equ		MBAR+$4000
__defxbs	macro		n,Base
XBS_PRS{n}	equ		Base+$00	; Priority Register (32b)
M7		cffield		XBS_PRS{n},28,3	;  Master 7 (Factory Test) Priority
M6		cffield		XBS_PRS{n},24,3	;  Master 6 (USB OTG) Priority
M5		cffield		XBS_PRS{n},20,3	;  Master 5 (USB Host) Priority
M4		cffield		XBS_PRS{n},16,3 ;  Master 4 (LCD Controller) Priority
M2		cffield		XBS_PRS{n},8,3	;  Master 2 (FEC) Priority
M1		cffield		XBS_PRS{n},4,3	;  Master 1 (eDMA) Priority
M0		cffield		XBS_PRS{n},0,3	;  Master 0 (ColdFire core) Priority
XBS_CRS{n}	equ		Base+$10	; Control Register (32b)
RO		cfbit		XBS_CRS{n},31	;  Read Only
ARB		cfbit		XBS_CRS{n},8	;  Arbitration Mode
PCTL		cffield		XBS_CRS{n},4,2	;  Parking Control
PARK		cffield		XBS_CRS{n},0,3	;  Park
		endm
		__defxbs	"1",MBAR_XBS+$100
		__defxbs	"4",MBAR_XBS+$400
		__defxbs	"6",MBAR_XBS+$600
		__defxbs	"7",MBAR_XBS+$700

;----------------------------------------------------------------------------
; General Purpose I/O

MBAR_GPIO	equ		MBAR+$a4000

; Port Output Data Registers

PODR_SSI	equ		MBAR_GPIO+$002  ; SSI Output Data Register (8b)
PODR_BUSCTL	equ		MBAR_GPIO+$003	; Bus Control Output Data Register (8b)
PODR_BE		equ		MBAR_GPIO+$004	; Byte Enable Output Data Register (8b)
PODR_CS		equ		MBAR_GPIO+$005	; Chip Select Output Data Register (8b)
PODR_PWM	equ		MBAR_GPIO+$006  ; PWM Output Data Register (8b)
PODR_FECI2C	equ		MBAR_GPIO+$007	; FEC/I2C Output Data Register (8b)
PODR_UART	equ		MBAR_GPIO+$009	; UART Output Data Register (8b)
PODR_QSPI	equ		MBAR_GPIO+$00a	; QSPI Output Data Register (8b)
PODR_TIMER	equ		MBAR_GPIO+$00b	; Timer Output Data Register (8b)
PODR_FECH	equ		MBAR_GPIO+$00e	; FEC High Output Data Register (8b)
PODR_FECL	equ		MBAR_GPIO+$00f	; FEC Low Output Data Register (8b)

; Port Data Direction Registers

PDDR_SSI	equ		MBAR_GPIO+$016	; SSI Data Direction Register (8b)
PDDR_BUSCTL	equ		MBAR_GPIO+$017	; Bus Control Data Direction Register (8b)
PDDR_BE		equ		MBAR_GPIO+$018	; Byte Enable Data Direction Register (8b)
PDDR_CS		equ		MBAR_GPIO+$019	; Chip Select Data Direction Register (8b)
PDDR_PWM	equ		MBAR_GPIO+$01a	; PWM Data Direction Register (8b)
PDDR_FECI2C	equ		MBAR_GPIO+$01b	; FEC/I2C Data Direction Register (8b)
PDDR_UART	equ		MBAR_GPIO+$01d	; UART Data Direction Register (8b)
PDDR_QSPI	equ		MBAR_GPIO+$01e	; QSPI Data Direction Register (8b)
PDDR_TIMER	equ		MBAR_GPIO+$01f	; Timer Data Direction Register (8b)
PDDR_FECH	equ		MBAR_GPIO+$022	; FEC High Data Direction Register (8b)
PDDR_FECL	equ		MBAR_GPIO+$023	; FEC Low Data Direction Register (8b)

; Port Pin Data/Set Data Registers

PPDSDR_SSI	equ		MBAR_GPIO+$02a  ; SSI Pin Data/Set Data Register (8b)
PPDSDR_BUSCTL	equ		MBAR_GPIO+$02b  ; Bus Control Pin Data/Set Data Register (8b)
PPDSDR_BE	equ		MBAR_GPIO+$02c  ; Byte Enable Pin Data/Set Data Register (8b)
PPDSDR_CS	equ		MBAR_GPIO+$02d	; Chip Select Pin Data/Set Data Register (8b)
PPDSDR_PWM	equ		MBAR_GPIO+$02e  ; PWM Pin Data/Set Data Register (8b)
PPDSDR_FECI2C	equ		MBAR_GPIO+$02f	; FEC/I2C Pin Data/Set Data Register (8b)
PPDSDR_UART	equ		MBAR_GPIO+$031	; UART Pin Data/Set Data Register (8b)
PPDSDR_QSPI	equ		MBAR_GPIO+$032	; QSPI Pin Data/Set Data Register (8b)
PPDSDR_TIMER	equ		MBAR_GPIO+$033	; Timer Pin Data/Set Data Register (8b)
PPDSDR_FECH	equ		MBAR_GPIO+$036	; FEC High Pin Data/Set Data Register (8b)
PPDSDR_FECL	equ		MBAR_GPIO+$037	; FEC Low Pin Data/Set Data Register (8b)

; Port Clear Output Data Registers

PCLRR_SSI	equ		MBAR_GPIO+$03e  ; SSI Output Data Register (8b)
PCLRR_BUSCTL	equ		MBAR_GPIO+$03f	; Bus Control Clear Output Data Register (8b)
PCLRR_BE	equ		MBAR_GPIO+$040	; Byte Enable Clear Output Data Register (8b)
PCLRR_CS	equ		MBAR_GPIO+$041	; Chip Select Clear Output Data Register (8b)
PCLRR_PWM	equ		MBAR_GPIO+$042  ; PWM Output Data Register (8b)
PCLRR_FECI2C	equ		MBAR_GPIO+$043	; FEC/I2C Clear Output Data Register (8b)
PCLRR_UART	equ		MBAR_GPIO+$045	; UART Clear Output Data Register (8b)
PCLRR_QSPI	equ		MBAR_GPIO+$046	; QSPI Clear Output Data Register (8b)
PCLRR_TIMER	equ		MBAR_GPIO+$047	; Timer Clear Output Data Register (8b)
PCLRR_FECH	equ		MBAR_GPIO+$04a	; FEC High Clear Output Data Register (8b)
PCLRR_FECL	equ		MBAR_GPIO+$04b	; FEC Low Clear Output Data Register (8b)

; Pin Assignment Registers

PAR_PWM		equ		MBAR_GPIO+$051	; PWM Pin Assignment (8b)
PAR_PWM7	cfbit		PAR_PWM,5	;  PWM7 pin assignment.
PAR_PWM5	cfbit		PAR_PWM,4	;  PWM5 pin assignment.
PAR_PWM3	cffield		PAR_PWM,2,2	;  PWM3 pin assignment.
PAR_PWM1	cffield		PAR_PWM,0,2	;  PWM1 pin assignment.
PAR_BUSCTL	equ		MBAR_GPIO+$052	; External Bus Control Pin Assignment Register (8b)
PAR_OE		cfbit		PAR_BUSCTL,7	;  /OE Pin Assignment
PAR_TA		cfbit		PAR_BUSCTL,6	;  /TA Pin Assignment
PAR_RWB		cfbit		PAR_BUSCTL,5	;  R/-W Pin Assignment
PAR_TS		cffield		PAR_BUSCTL,3,2	;  /TS Pin Assignment
PAR_FECI2C	equ		MBAR_GPIO+$053	; FEC/I2C Pin Assignment (8b)
PAR_MDC		cffield		PAR_FECI2C,6,2	;  MDC Pin Assignment
PAR_MDIO	cffield		PAR_FECI2C,4,2	;  MDIO Pin Assignment
PAR_SCL		cffield		PAR_FECI2C,2,2	;  SCL Pin Assignment
PAR_SDA		cffield		PAR_FECI2C,0,2	;  SDA Pin Assignment
PAR_BE		equ		MBAR_GPIO+$054	; Byte Enable Pin Assignment Register (8b)
PAR_BE		cffield		PAR_BE,0,4	;  Byte Enable Pin Assignment
PAR_CS		equ		MBAR_GPIO+$055	; Chip Select Pin Assignment Register (8b)
PAR_CS5		cfbit		PAR_CS,5	;  /FB_CS5 Pin Assignment
PAR_CS4		cfbit		PAR_CS,4	;  /FB_CS4 Pin Assignment
PAR_CS3		cfbit		PAR_CS,3	;  /FB_CS3 Pin Assignment
PAR_CS2		cfbit		PAR_CS,2	;  /FB_CS2 Pin Assignment
PAR_CS1		cfbit		PAR_CS,1	;  /FB_CS1 Pin Assignment
PAR_SSI		equ		MBAR_GPIO+$056	; SSI Pin Assignment (16b)
PAR_BCLK	cffield		PAR_SSI,14,2	;  BCLK Pin Assignment
PAR_FS		cffield		PAR_SSI,12,2	;  FS Pin Assignment
PAR_RXD		cffield		PAR_SSI,10,2	;  RxD Pin Assignment
PAR_TXD		cffield		PAR_SSI,8,2	;  TxD Pin Assignment
PAR_MCLK	cfbit		PAR_SSI,7	;  MCLK Pin Assignment
PAR_UART	equ		MBAR_GPIO+$058	; UART Pin Assignment (16b)
PAR_U1CTS	cffield		PAR_UART,10,2	;  UART1 CTS Pin Assignment
PAR_U1RTS	cffield		PAR_UART,8,2	;  UART1 RTS Pin Assignment
PAR_U1RXD	cffield		PAR_UART,6,2	;  UART1 RXD Pin Assignment
PAR_U1TXD	cffield		PAR_UART,4,2	;  UART1 TXD Pin Assignment
PAR_U0CTS	cfbit		PAR_UART,3	;  UART0 CTS Pin Assignment
PAR_U0RTS	cfbit		PAR_UART,2	;  UART0 RTS Pin Assignment
PAR_U0RXD	cfbit		PAR_UART,1	;  UART0 RXD Pin Assignment
PAR_U0TXD	cfbit		PAR_UART,0	;  UART0 TXD Pin Assignment
PAR_QSPI	equ		MBAR_GPIO+$05a	; QSPI Pin Assignment (16b)
PAR_PCS2	cffield		PAR_QSPI,14,2	;  QSPI Pin Assignment
PAR_PCS1	cffield		PAR_QSPI,12,2
PAR_PCS0	cffield		PAR_QSPI,10,2
PAR_DIN		cffield		PAR_QSPI,8,2
PAR_DOUT	cffield		PAR_QSPI,6,2
PAR_SCK		cffield		PAR_QSPI,4,2
PAR_TIMER	equ		MBAR_GPIO+$05c	; Timer Pin Assignment (8b)
PAR_T3IN	cffield		PAR_TIMER,6,2	; DMA Timer 3 Pin Assignment
PAR_T2IN	cffield		PAR_TIMER,4,2	; DMA Timer 2 Pin Assignment
PAR_T1IN	cffield		PAR_TIMER,2,2	; DMA Timer 1 Pin Assignment
PAR_T0IN	cffield		PAR_TIMER,0,2	; DMA Timer 0 Pin Assignment
PAR_FEC		equ		MBAR_GPIO+$05d	; FEC Pin Assignment (8b)
PAR_FEC_7W	cfbit		PAR_FEC,3	;  FEC 7-wire Pin Assignment
PAR_FEC_MII	cfbit		PAR_FEC,1	;  FEC MII Pin Assignment
PAR_IRQ		equ		MBAR_GPIO+$060	; IRQ Pin Assignment (16b)
PAR_IRQ6	cffield		PAR_IRQ,12,2	;  /IRQ6 Pin Assignment
PAR_IRQ5	cffield		PAR_IRQ,10,2	;  /IRQ5 Pin Assignment
PAR_IRQ4	cffield		PAR_IRQ,8,2	;  /IRQ4 Pin Assignment
PAR_IRQ2	cffield		PAR_IRQ,6,2	;  /IRQ2 Pin Assignment
PAR_IRQ1	cffield		PAR_IRQ,4,2	;  /IRQ1 Pin Assignment

; Mode Select Control Registers

MSCR_FLEXBUS	equ		MBAR_GPIO+$064	; FlexBus Mode Select Control Register (8b)
MSCR_DUPPER	cffield		MSCR_FLEXBUS,4,2;  FB_D[31:16] Mode Select Control
MSCR_DLOWER	cffield		MSCR_FLEXBUS,2,2;  FB_D[15:0] Mode Select Control
MSCR_ADDRCTL	cffield		MSCR_FLEXBUS,0,2;  FB_A[23:0], BE/BWE[3:0], OE, R/W, FB_CS[5:0], TA, and TS Mode Select Control
MSCR_SDRAM	equ		MBAR_GPIO+$065	; SDRAM Mode Select Control Register (8b)
MSCR_SDCLKB	cffield		MSCR_SDRAM,4,2	;  SD_CLK Mode Select Control
MSCR_SDCLK	cffield		MSCR_SDRAM,2,2	;  SD_CLK Mode Select Control
MSCR_SDRAM	cffield		MSCR_SDRAM,0,2	;  SD_A10, SD_CAS, SD_CKE, SD_CS0, SD_DQS[3:2], SD_RAS, SD_SDRDQS, SD_WE Mode Select Control

; Drive Strength Control Registers

DSCR_I2C	equ		MBAR_GPIO+$068	; I2C Drive Strength Control Register (8b)
I2C_DSE		cffield		DSCR_I2C,0,2	;  I2C Drive Strength Control
DSCR_PWM	equ		MBAR_GPIO+$069	; PWM Drive Strength Control Register (8b)
PWM_DSE		cffield		DSCR_PWM,0,2	;  PWM Drive Strength Control
DSCR_FEC	equ		MBAR_GPIO+$06a	; FEC Drive Strength Control Register (8b)
FEC_DSE		cffield		DSCR_FEC,0,2	;  FEC Drive Strength Control
DSCR_UART	equ		MBAR_GPIO+$06b	; UART/IRQ Drive Strength Control Register (8b)
UART1_DSE	cffield		DSCR_UART,2,2	;  UART1 Drive Strength Control
UART0_DSE	cffield		DSCR_UART,0,2	;  UART0 Drive Strength Control
DSCR_QSPI	equ		MBAR_GPIO+$06c	; QSPI Drive Strength Control Register (8b)
QSPI_DSE	cffield		DSCR_QSPI,0,2	;  QSPI Drive Strength Control
DSCR_TIMER	equ		MBAR_GPIO+$06d	; Timer Drive Strength Control Register (8b)
TIMER_DSE	cffield		DSCR_TIMER,0,2	;  Timer Drive Strength Control
DSCR_SSI	equ		MBAR_GPIO+$06e	; SSI Drive Strength Control Register (8b)
SSI_DSE		cffield		DSCR_SSI,0,2	;  SSI Drive Strength Control
DSCR_DEBUG	equ		MBAR_GPIO+$070	; Debug Drive Strength Control Register (8b)
DEBUG_DSE	cffield		DSCR_DEBUG,0,2	;  Debug Drive Strength Control
DSCR_CLKRST	equ		MBAR_GPIO+$071	; Clock/Reset Drive Strength Control Register (8b)
RSTOUT_DSE	cffield		DSCR_CLKRST,2,2	;  /RSTOUT drive strength control.
MSCR_FBCLK	cffield		DSCR_CLKRST,0,2	;  FB_CLK mode select control.
DSCR_IRQ	equ		MBAR_GPIO+$072	; IRQ Drive Strength Control Register (8b)
IRW_DSE		cffield		DSCR_IRQ,0,2	;  IRQ Drive Strength Control

;----------------------------------------------------------------------------
; Interrupt Controllers

MBAR_INTC0	equ		MBAR+$48000
MBAR_INTC1	equ		MBAR+$4c000
MBAR_INTC	equ		MBAR+$54000
		include		"coldfire/52xxintc.inc"

;----------------------------------------------------------------------------
; Edge Port Module

MBAR_EPORT	equ		MBAR+$94000
		include		"coldfire/52xxeport.inc"

;----------------------------------------------------------------------------
; Enhanced DMA Controller

MBAR_EDMA	equ		MBAR+$44000
		include		"coldfire/52xxedma.inc"

;----------------------------------------------------------------------------
; FlexBus

MBAR_FBUS	equ		MBAR+$8000
		include		"coldfire/52xxfbus.inc"

;----------------------------------------------------------------------------
; SDRAM Controller

MBAR_SDRAM	equ		MBAR+$b8000
		include		"coldfire/52xxdram.inc"

;----------------------------------------------------------------------------
; Fast Ethernet Controller

MBAR_FEC	equ		MBAR+$30000
		include		"coldfire/52xxfec.inc"

;----------------------------------------------------------------------------
; USB Host/On-the-Go

		include		"coldfire/52xxusb.inc"
MBAR_USB	equ		MBAR+$b4000
		__defusb	"USB.",MBAR_USB
MBAR_OTG	equ		MBAR+$b0000
		__defusb	"OTG.",MBAR_OTG

;----------------------------------------------------------------------------
; FlexCAN

MBAR_CAN	equ		MBAR+$20000

		include		"coldfire/52xxcan.inc"

;----------------------------------------------------------------------------
; Synchronous Serial Interface

MBAR_SSI	equ		MBAR+$bc000
		include		"coldfire/52xxssi.inc"

;----------------------------------------------------------------------------
; Real Time Clock

MBAR_RTC	equ		MBAR+$a8000
		include		"coldfire/52xxrtc.inc"
XTL		cffield		RTC_CR,5,2	; Crystal selection.

;----------------------------------------------------------------------------
; Pulse-Width Modulation Module

MBAR_PWM	equ		MBAR+$90000
		include		"coldfire/52xxpwm.inc"

;----------------------------------------------------------------------------
; Watchdog Timer

MBAR_WDT	equ		MBAR+$98000
		include		"coldfire/52xxwdt.inc"

;----------------------------------------------------------------------------
; Programmable Interrupt Timer

		include		"coldfire/52xxpit.inc"
		__defpit	"0",MBAR+$80000
		__defpit	"1",MBAR+$84000
		__defpit	"2",MBAR+$88000
		__defpit	"3",MBAR+$8c000

;----------------------------------------------------------------------------
; DMA Timer

		include		"coldfire/52xxdtim.inc"
		__defdtim	"0",MBAR+$70000
		__defdtim	"1",MBAR+$74000
		__defdtim	"2",MBAR+$78000
		__defdtim	"3",MBAR+$7c000

;----------------------------------------------------------------------------
; Queued Serial Peripheral Interface

MBAR_QSPI	equ		MBAR+$5c000
		include		"coldfire/52xxqspi.inc"

;----------------------------------------------------------------------------
; UARTs

		include		"coldfire/52xxuart.inc"
		__defuart	"0",MBAR+$60000
		__defuart	"1",MBAR+$64000
		__defuart	"2",MBAR+$68000

;----------------------------------------------------------------------------
; I2C

MBAR_I2C	equ		MBAR+$58000
		include		"coldfire/52xxi2c.inc"

;----------------------------------------------------------------------------
; Message Digest Hardware Accelerator

MBAR_MDHA	equ		$ec080000
		include		"coldfire/52xxmdha.inc"

;----------------------------------------------------------------------------
; Random Number Generator

MBAR_RNG	equ		$ec088000
		include		"coldfire/52xxrng.inc"

;----------------------------------------------------------------------------
; Symmetric Key Hardware Accelerator

MBAR_SKHA	equ		$ec084000
		include		"coldfire/52xxskha.inc"

;----------------------------------------------------------------------------

                restore				; re-enable listing

                endif				; __mcf5373inc
