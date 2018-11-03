		ifndef	__mcf5208inc		; avoid multiple inclusion
__mcf5208inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF5208.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF5208                  *
;*                                                                          *
;****************************************************************************

MBAR		equ		$fc000000

;----------------------------------------------------------------------------
; Clock Module

MBAR_CLK	equ		MBAR+$9000

PODR		equ		MBAR_CLK+0	; PLL Output Divider Register (8b)
BUSDIV		cffield		PODR,0,4	;  Divider for generating the internal bus frequency
CPUDIV		cffield		PODR,4,4	;  Divider for generating the core frequency
PCR		equ		MBAR_CLK+2	; PLL Control Register (8b)
DITHEN		cfbit		PCR,7		;  Dithering enable bit
DITHDEV		cffield		PCR,0,3		;  Dither Deviation
PMDR		equ		MBAR_CLK+4	; PLL Modulation Divider Register (8b)
MODDIV		cffield		PMDR,0,8	;  Dither Modulation Divider
PFDR		equ		MBAR_CLK+6	; PLL Feedback Divider Register (8b)
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
PPMHR0		equ		MBAR+$40030	; Peripheral Power Management High Register 0 (32b)
CD42		cfbit		PPMHR0,10	;  PIT 0
CD41		cfbit		PPMHR0,9	;  PIT 1
CD40		cfbit		PPMHR0,8	;  Edge Port
CD36		cfbit		PPMHR0,4	;  On-chip Watchdog Timer
CD35		cfbit		PPMHR0,3	;  PLL
CD34		cfbit		PPMHR0,2	;  CCM, Reset Controller, Power Management
CD33		cfbit		PPMHR0,1	;  GPIO Module
CD32		cfbit		PPMHR0,0	;  SDRAM Controller
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
CD18		cfbit		PPMLR0,18	;  Interrupt Controller
CD17		cfbit		PPMLR0,17	;  eDMA Controller
CD12		cfbit		PPMLR0,12	;  FEC
CD2		cfbit		PPMLR0,2	;  FlexBus
LPCR		equ		MBAR+$a0007	; Low-Power Control Register (8b)
LPMD		cffield		LPCR,6,2	;  Low-power mode select
FWKUP		cfbit		LPCR,5		;  Fast wake-up
STPMD		cffield		LPCR,3,2	;  FB_CLK stop mode bits
MISCCR		equ		MBAR+$a0010	; Miscellaneous Control Register (16b)
PLLLOCK		cfbit		MISCCR,13	;  PLL lock status
LIMP		cfbit		MISCCR,12	;  Limp mode enable
LPDIV		cffield		MISCCR,0,4	;  Low power divider

;----------------------------------------------------------------------------
; Chip Configuration Module

MBAR_CCM	equ		MBAR+$a0000
CCR		equ		MBAR_CCM+$0	; Chip Configuration Register (16b)
CSC		cfbit		CCR,9		;  Chip select configuration field
OSCFREQ		cfbit		CCR,7		;  Oscillator frequency
LIMP		cfbit		CCR,6		;  Limp mode
LOAD		cfbit		CCR,5		;  Pad driver load
BOOTPS		cffield		CCR,3,2		;  Boot port size
OSCMODE		cfbit		CCR,2		;  Oscillator clock mode
PLLMODE		cfbit		CCR,1		;  PLL clock mode
RCON		equ		MBAR_CCM+$4	; Reset Configuration Register (16b)
CSC		cfbit		RCON,9		;  Chip select configuration field
OSCFREQ		cfbit		RCON,7		;  Oscillator frequency
LIMP		cfbit		RCON,6		;  Limp mode
LOAD		cfbit		RCON,5		;  Pad driver load
BOOTPS		cffield		RCON,3,2	;  Boot port size
OSCMODE		cfbit		RCON,2		;  Oscillator clock mode
PLLMODE		cfbit		RCON,1		;  PLL clock mode
CIR		equ		MBAR_CCM+$a	; Chip Identification Register (16b)
PIN		cffield		CIR,8,8		;  Part identification number
PRN		cffield		CIR,0,8		;  Part revision number

;----------------------------------------------------------------------------
; Reset Controller Module

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
MPR		equ		MBAR_SCM+$00 	; Master Privilege Register (32b)
MPROT0		__defprot	MBAR_SCM,28	;  ColdFire Core:
MPROT1		__defprot	MBAR_SCM,24	;  eDMA Controller:
MPROT2		__defprot	MBAR_SCM,20	;  FEC:

__defpacr	macro		{INTLABEL},Reg,Startbit
__LABEL__	cffield		Reg,Startbit,4
__LABEL__.TP	cfbit		Reg,Startbit+0	;   Trusted Protect
__LABEL__.WP	cfbit		Reg,Startbit+1	;   Write protect
__LABEL__.SP	cfbit		Reg,Startbit+2	;   Supervisor protect.
		endm
PACRA		equ		MBAR_SCM+$20 	; Peripheral Access Control Register A (32b)
PACR0		__defpacr	PACRA,28	; SCM (MPR & PACRs)
PACR1		__defpacr	PACRA,24	; Cross-Bar Switch
PACR2		__defpacr	PACRA,20	; FlexBus
PACRB		equ		MBAR_SCM+$24 	; Peripheral Access Control Register B (32b)
PACR12		__defpacr	PACRB,12	; FEC
PACRC		equ		MBAR_SCM+$28 	; Peripheral Access Control Register C (32b)
PACR16		__defpacr	PACRC,28	; SCM (CWT & Core Fault Registers
PACR17		__defpacr	PACRC,24	; eDMA Controller
PACR18		__defpacr	PACRC,20	; Interrupt Controller 0
PACR21		__defpacr	PACRC,8		; Interrupt Controller IACK
PACR22		__defpacr	PACRC,4		; I2C
PACR23		__defpacr	PACRC,0		; QSPI
PACRD		equ		MBAR_SCM+$2C 	; Peripheral Access Control Register D (32b)
PACR24		__defpacr	PACRD,28	; UART0
PACR25		__defpacr	PACRD,24	; UART1
PACR26		__defpacr	PACRD,20	; UART2
PACR28		__defpacr	PACRD,12	; DMA Timer 0
PACR29		__defpacr	PACRD,8		; DMA Timer 1
PACR30		__defpacr	PACRD,4		; DMA Timer 2
PACR31		__defpacr	PACRD,0		; DMA Timer 3
PACRE		equ		MBAR_SCM+$40 	; Peripheral Access Control Register E (32b)
PACR32		__defpacr	PACRE,28	; PIT 0
PACR33		__defpacr	PACRE,24	; PIT 1
PACR34		__defpacr	PACRE,20	; Edge Port
PACR35		__defpacr	PACRE,16	; On-Chip Watchdog Timer
PACR36		__defpacr	PACRE,12	; PLL
PACRF		equ		MBAR_SCM+$44 	; Peripheral Access Control Register F (32b)
PACR40		__defpacr	PACRF,28	; CCM, Reset Controller, Power Management
PACR41		__defpacr	PACRF,24	; GPIO Module
PACR42		__defpacr	PACRF,20	; SDRAM Controller
BMT		equ		MBAR_SCM+$54 	; Bus Monitor Timeout (32b)
BMT		cffield		BMT,0,3		;  Bus Monitor Timeout Period
BME		cfbit		BMT,3		;  Bus Monitor Timeout Enable
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
		__defxbs	"7",MBAR_XBS+$700

;----------------------------------------------------------------------------
; GPIO Module

MBAR_GPIO	equ		MBAR+$a4000

; Port Output Data Registers

PODR_BUSCTL	equ		MBAR_GPIO+$000	; Bus Control Output Data Register (8b)
PODR_BE		equ		MBAR_GPIO+$001	; Byte Enable Output Data Register (8b)
PODR_CS		equ		MBAR_GPIO+$002	; Chip Select Output Data Register (8b)
PODR_FECI2C	equ		MBAR_GPIO+$003	; FEC/I2C Output Data Register (8b)
PODR_QSPI	equ		MBAR_GPIO+$004	; QSPI Output Data Register (8b)
PODR_TIMER	equ		MBAR_GPIO+$005	; Timer Output Data Register (8b)
PODR_UART	equ		MBAR_GPIO+$006	; UART Output Data Register (8b)
PODR_FECH	equ		MBAR_GPIO+$007	; FEC High Output Data Register (8b)
PODR_FECL	equ		MBAR_GPIO+$008	; FEC Low Output Data Register (8b)

; Port Data Direction Registers

PDDR_BUSCTL	equ		MBAR_GPIO+$00C	; Bus Control Data Direction Register (8b)
PDDR_BE		equ		MBAR_GPIO+$00D	; Byte Enable Data Direction Register (8b)
PDDR_CS		equ		MBAR_GPIO+$00E	; Chip Select Data Direction Register (8b)
PDDR_FECI2C	equ		MBAR_GPIO+$00F	; FEC/I2C Data Direction Register (8b)
PDDR_QSPI	equ		MBAR_GPIO+$010	; QSPI Data Direction Register (8b)
PDDR_TIMER	equ		MBAR_GPIO+$011	; Timer Data Direction Register (8b)
PDDR_UART	equ		MBAR_GPIO+$012	; UART Data Direction Register (8b)
PDDR_FECH	equ		MBAR_GPIO+$013	; FEC High Data Direction Register (8b)
PDDR_FECL	equ		MBAR_GPIO+$014	; FEC Low Data Direction Register (8b)

; Port Pin Data/Set Data Registers

PPDSDR_CS	equ		MBAR_GPIO+$01A	; Chip Select Pin Data/Set Data Register (8b)
PPDSDR_FECI2C	equ		MBAR_GPIO+$01B	; FEC/I2C Pin Data/Set Data Register (8b)
PPDSDR_QSPI	equ		MBAR_GPIO+$01C	; QSPI Pin Data/Set Data Register (8b)
PPDSDR_TIMER	equ		MBAR_GPIO+$01D	; Timer Pin Data/Set Data Register (8b)
PPDSDR_UART	equ		MBAR_GPIO+$01E	; UART Pin Data/Set Data Register (8b)
PPDSDR_FECH	equ		MBAR_GPIO+$01F	; FEC High Pin Data/Set Data Register (8b)
PPDSDR_FECL	equ		MBAR_GPIO+$020	; FEC Low Pin Data/Set Data Register (8b)

; Port Clear Output Data Registers

PCLRR_BUSCTL	equ		MBAR_GPIO+$024	; Bus Control Clear Output Data Register (8b)
PCLRR_BE	equ		MBAR_GPIO+$025	; Byte Enable Clear Output Data Register (8b)
PCLRR_CS	equ		MBAR_GPIO+$026	; Chip Select Clear Output Data Register (8b)
PCLRR_FECI2C	equ		MBAR_GPIO+$027	; FEC/I2C Clear Output Data Register (8b)
PCLRR_QSPI	equ		MBAR_GPIO+$028	; QSPI Clear Output Data Register (8b)
PCLRR_TIMER	equ		MBAR_GPIO+$029	; Timer Clear Output Data Register (8b)
PCLRR_UART	equ		MBAR_GPIO+$02A	; UART Clear Output Data Register (8b)
PCLRR_FECH	equ		MBAR_GPIO+$02B	; FEC High Clear Output Data Register (8b)
PCLRR_FECL	equ		MBAR_GPIO+$02C	; FEC Low Clear Output Data Register (8b)

; Pin Assignment Registers

PAR_BUSCTL	equ		MBAR_GPIO+$030	; External Bus Control Pin Assignment Register (8b)
PAR_OE		cfbit		PAR_BUSCTL,4	;  /OE Pin Assignment
PAR_TA		cfbit		PAR_BUSCTL,3	;  /TA Pin Assignment
PAR_RWB		cfbit		PAR_BUSCTL,2	;  R/-W Pin Assignment
PAR_TS		cffield		PAR_BUSCTL,0,2	;  /TS Pin Assignment
PAR_BE		equ		MBAR_GPIO+$031	; Byte Enable Pin Assignment Register (8b)
PAR_CS		equ		MBAR_GPIO+$032	; Chip Select Pin Assignment Register (8b)
PAR_CS3		cfbit		PAR_CS,3	;  /FB_CS3 Pin Assignment
PAR_CS2		cfbit		PAR_CS,2	;  /FB_CS2 Pin Assignment
PAR_CS1		cffield		PAR_CS,0,2	;  /FB_CS1 Pin Assignment
PAR_FECI2C	equ		MBAR_GPIO+$033	; FEC/I2C Pin Assignment (8b)
PAR_MDC		cffield		PAR_FECI2C,6,2	;  MDC Pin Assignment
PAR_MDIO	cffield		PAR_FECI2C,4,2	;  MDIO Pin Assignment
PAR_SCL		cffield		PAR_FECI2C,2,2	;  SCL Pin Assignment
PAR_SDA		cffield		PAR_FECI2C,0,2	;  SDA Pin Assignment
PAR_QSPI	equ		MBAR_GPIO+$034	; QSPI Pin Assignment (8b)
PAR_PCS2	cffield		PAR_QSPI,6,2	;  QSPI Pin Assignment
PAR_DIN		cffield		PAR_QSPI,4,2
PAR_DOUT	cffield		PAR_QSPI,2,2
PAR_SCK		cffield		PAR_QSPI,0,2
PAR_TIMER	equ		MBAR_GPIO+$035	; Timer Pin Assignment (8b)
PAR_T3IN	cffield		PAR_TIMER,6,2	;  DMA Timer 3 Pin Assignment
PAR_T2IN	cffield		PAR_TIMER,4,2	;  DMA Timer 2 Pin Assignment
PAR_T1IN	cffield		PAR_TIMER,2,2	;  DMA Timer 1 Pin Assignment
PAR_T0IN	cffield		PAR_TIMER,0,2	;  DMA Timer 0 Pin Assignment
PAR_UART	equ		MBAR_GPIO+$036	; UART Pin Assignment (16b)
PAR_U1CTS	cffield		PAR_UART,10,2	;  UART1 Control Pin Assignment
PAR_U1RTS	cffield		PAR_UART,8,2	;  
PAR_U1TXD	cfbit		PAR_UART,7	;  U1TXD Pin Assignment
PAR_U1RXD	cfbit		PAR_UART,6	;  U1RXD Pin Assignment
PAR_U0CTS	cffield		PAR_UART,4,2	;  UART0 Control Pin Assignment
PAR_U0RTS	cffield		PAR_UART,2,2	;  
PAR_U0TXD	cfbit		PAR_UART,1	;  U0TXD Pin Assignment
PAR_U0RXD	cfbit		PAR_UART,0	;  U0RXD Pin Assignment
PAR_FEC		equ		MBAR_GPIO+$038	; FEC Pin Assignment (8b)
PAR_FEC_7W	cffield		PAR_FEC,2,2	;  FEC 7-wire Pin Assignment
PAR_FEC_MII	cffield		PAR_FEC,0,2	;  FEC MII Pin Assignment
PAR_IRQ		equ		MBAR_GPIO+$039	; IRQ Pin Assignment (8b)
PAR_IRQ4	cfbit		PAR_IRQ,0	;  /IRQ4 Pin Assignment

; Mode Select Control Registers

MSCR_FLEXBUS	equ		MBAR_GPIO+$03A	; FlexBus Mode Select Control Register (8b)
MSCR_FBCLK	cffield		MSCR_FLEXBUS,6,2;  FB_CLK Mode Select Control
MSCR_DUPPER	cffield		MSCR_FLEXBUS,4,2;  FB_D[31:16] Mode Select Control
MSCR_DLOWER	cffield		MSCR_FLEXBUS,2,2;  FB_D[15:0] Mode Select Control
MSCR_ADDRCTL	cffield		MSCR_FLEXBUS,0,2;  FB_A[23:0], BE/BWE[3:0], OE, R/W, FB_CS[5:0], TA, and TS Mode Select Control
MSCR_SDRAM	equ		MBAR_GPIO+$03B	; SDRAM Mode Select Control Register (8b)
MSCR_SDCLKB	cffield		MSCR_SDRAM,4,2	;  SD_CLK Mode Select Control
MSCR_SDCLK	cffield		MSCR_SDRAM,2,2	;  SD_CLK Mode Select Control
MSCR_SDRAM	cffield		MSCR_SDRAM,0,2	;  SD_A10, SD_CAS, SD_CKE, SD_CS0, SD_DQS[3:2], SD_RAS, SD_SDRDQS, SD_WE Mode Select Control

; Drive Strength Control Registers

DSCR_I2C	equ		MBAR_GPIO+$03C	; I2C Drive Strength Control Register (8b)
I2C_DSE		cffield		DSCR_I2C,0,2	;  I2C Drive Strength Control
DSCR_MISC	equ		MBAR_GPIO+$03D	; Miscellaneous Drive Strength Control Register (8b)
DEBUG_DSE	cffield		DSCR_MISC,4,2	;  Debug Drive Strength Control
RSTOUT_DSE	cffield		DSCR_MISC,2,2	;  /RSTOUT Drive Strength Control
TIMER_DSE	cffield		DSCR_MISC,0,2	;  Timer Drive Strength Control
DSCR_FEC	equ		MBAR_GPIO+$03E	; FEC Drive Strength Control Register (8b)
FEC_DSE		cffield		DSCR_FEC,0,2	;  FEC Drive Strength Control
DSCR_UART	equ		MBAR_GPIO+$03F	; UART/IRQ Drive Strength Control Register (8b)
UART1_DSE	cffield		DSCR_UART,4,2	;  UART1 Drive Strength Control
UART0_DSE	cffield		DSCR_UART,2,2	;  UART0 Drive Strength Control
IRQ_DSE		cffield		DSCR_UART,0,2	;  IRQ drive strength Control
DSCR_QSPI	equ		MBAR_GPIO+$040	; QSPI Drive Strength Control Register (8b)
QSPI_DSE	cffield		DSCR_QSPI,0,2	;  QSPI Drive Strength Control

;----------------------------------------------------------------------------
; Interrupt Controller Module

MBAR_INTC	equ		MBAR+$48000

IPRH		equ		MBAR_INTC+$000	; Interrupt Pending Register High (32b)
IPRL		equ		MBAR_INTC+$004	; Interrupt Pending Register Low (32b)
IMRH		equ		MBAR_INTC+$008	; Interrupt Mask Register High (32b)
IMRL		equ		MBAR_INTC+$00C	; Interrupt Mask Register Low (32b)
INTFRCH		equ		MBAR_INTC+$010	; Interrupt Force Register High	(32b)
INTFRCL		equ		MBAR_INTC+$014	; Interrupt Force Register Low (32b)
ICONFIG		equ		MBAR_INTC+$01A	; Interrupt Configuration Register (16b)
ELVLPRI		cffield		ICONFIG,9,7	;  Enable core's priority elevation on priority levels
EMASK		cfbit		ICONFIG,5	;  If set, the interrupt controller automatically loads the level of an interrupt request into the CLMASK (current level mask) when the acknowledge is performed.
SIMR		equ		MBAR_INTC+$01C	; Set Interrupt Mask (8b)
SALL		cfbit		SIMR,6		;  Set all bits in the IMR register
CIMR		equ		MBAR_INTC+$01D	; Clear Interrupt Mask (8b)
CALL		cfbit		CIMR,6		;  Clear all bits in the IMR register
CLMASK		equ		MBAR_INTC+$01E	; Current Level Mask (8b)
SLMASK		equ		MBAR_INTC+$01F	; Saved Level Mask (8b)
__N		set		0
		rept		64
		__decstr	__NS,__N	; note we need name with decimal number!
ICR{"\{__NS}"}	set		MBAR_INTC+$040+__N ; Interrupt Control Register N (8b)
LEVEL		cffield		ICR{"\{__NS}"},0,3	; Interrupt Level
__N		set		__N+1
		endm
SWIACK		equ		MBAR_INTC+$0E0	; Software Interrupt Acknowledge (8b)
__N		set		1
		rept		7
L{"\{__N}"}IACK	set		MBAR_INTC+$0e0+(4*__N) ; Interrupt Acknowledge Register N (8b)
__N		set		__n+1
		endm

;----------------------------------------------------------------------------
; Edge Port Module

MBAR_EPORT	equ		MBAR+$88000
		include		"coldfire/52xxeport.inc"

;----------------------------------------------------------------------------
; Enhanced Direct Memory Access

MBAR_EDMA	equ		MBAR+$44000
		include		"coldfire/52xxedma.inc"
                
;----------------------------------------------------------------------------
; FlexBus

MBAR_FBUS	equ		MBAR+$8000
		include		"coldfire/52xxfbus.inc"

;----------------------------------------------------------------------------
; SDRAM Controller

MBAR_SDRAM	equ		MBAR+$a8000
		include		"coldfire/52xxdram.inc"

;----------------------------------------------------------------------------
; Fast Ethernet Controller

MBAR_FEC	equ		MBAR+$30000
		include		"coldfire/52xxfec.inc"

;----------------------------------------------------------------------------
; Watchdog Timer

MBAR_WDT	equ		MBAR+$8c000
		include		"coldfire/52xxwdt.inc"

;----------------------------------------------------------------------------
; Programmable Interrupt Timer

		include		"coldfire/52xxpit.inc"
		__defpit	"1",MBAR+$80000
		__defpit	"2",MBAR+$84000

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

                restore				; re-enable listing

                endif				; __mcf5208inc
