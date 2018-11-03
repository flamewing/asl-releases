		ifndef	__mcf52277inc		; avoid multiple inclusion
__mcf52277inc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF52277.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52274/52277           *
;*                                                                          *
;****************************************************************************

MBAR            equ             $fc000000

;----------------------------------------------------------------------------
; Clock Module

MBAR_CLK        equ             MBAR+$9000

PCR		equ		MBAR_CLK+0	; PLL Control Register (32b)
PFDR		cffield		PCR,24,8	;  Feedback Divider for Setting the VCO Frequency
OUTDIV5		cffield		PCR,16,4	;  Output Divider for Generating the USB Clock Frequency
OUTDIV3		cffield		PCR,8,4		;  Output Divider for Generating the SDRAM Clock Frequency
OUTDIV2		cffield		PCR,4,4		;  Output Divider for Generating the Internal Bus Clock Frequency
OUTDIV1		cffield		PCR,0,4		;  Output Divider for Generating the CPU clock Frequency
PSR		equ		MBAR_CLK+4	; PLL Status Register (32b)
LOLRE		cfbit		PSR,3		;  PLL Loss of Lock Reset Enable
LOLIRQ		cfbit		PSR,2		;  PLL Loss-of-Lock Interrupt Enable
LOCK		cfbit		PSR,1		;  PLL Lock Status
LOCKS		cfbit		PSR,0		;  PLL Lost Lock

;----------------------------------------------------------------------------
; Power Management

WCR		equ		MBAR+$40013	; Wakeup Control Register (8b)
ENBWCR		cfbit		WCR,7		;  Enable low-Power Mode Entry
LPMD		cffield		WCR,4,2		;  Low-Power Mode Select
PRILVL		cffield		WCR,0,3		;  Exit Low-Power Mode Interrupt Priority Level
PPMSR		equ		MBAR+$4002c	; Peripheral Power Management Set Register (8b)
SAMCD		cfbit		PPMSR,6		;  Set all module clock disables
SMCD		cffield		PPMSR,0,6	;  Set module clock disable
PPMCR		equ		MBAR+$4002d	; Peripheral Power Management Clear Register (8b)
CAMCD		cfbit		PPMCR,6		;  Clear all module clock disables
CMCD		cffield		PPMCR,0,6	;  Clear module clock disable
PPMHR		equ		MBAR+$40030	; Peripheral Power Management High Register (32b)
CD48		cfbit		PPMHR,16	;  PLL
CD47		cfbit		PPMHR,15	;  SSI
CD46		cfbit		PPMHR,14	;  SDRAM Controller
CD44		cfbit		PPMHR,12	;  USB On-the GO
CD43		cfbit		PPMHR,11	;  LCD Controller
CD42		cfbit		PPMHR,10	;  Touchscreen Controller
CD41		cfbit		PPMHR,9		;  GPIO Module
CD40		cfbit		PPMHR,8		;  CCM, Reset Controller, Power Management
CD37		cfbit		PPMHR,5		;  Edge Port
CD36		cfbit		PPMHR,4		;  PWM
CD33		cfbit		PPMHR,1		;  PIT 1
CD32		cfbit		PPMHR,0		;  PIT 0
PPMLR		equ		MBAR+$40034	; Peripheral Power Management Low Register (32b)
CD31		cfbit		PPMLR,31	;  DMA Timer 3
CD30		cfbit		PPMLR,30	;  DMA Timer 2
CD29		cfbit		PPMLR,29	;  DMA Timer 1
CD28		cfbit		PPMLR,28	;  DMA Timer 0
CD26		cfbit		PPMLR,26	;  UART2
CD25		cfbit		PPMLR,25	;  UART1
CD24		cfbit		PPMLR,24	;  UART0
CD23		cfbit		PPMLR,23	;  DSPI
CD22		cfbit		PPMLR,22	;  I2C
CD21		cfbit		PPMLR,21	;  IACK
CD19		cfbit		PPMLR,19	;  Interrupt Controller 1
CD18		cfbit		PPMLR,18	;  Interrupt Controller 0
CD17		cfbit		PPMLR,17	;  eDMA Controller
CD15		cfbit		PPMLR,15	;  Real-Time Clock
CD8		cfbit		PPMLR,12	;  FlexCAN
CD2		cfbit		PPMLR,2		;  FlexBus
CDG		cfbit		PPMLR,1		;  Global Space
LPCR		equ		MBAR+$a0007	; Low-Power Control Register (8b)
FWKUP		cfbit		LPCR,5		;  Fast Wake-Up
STPMD		cffield		LPCR,3,2	;  FB_CLK stop mode bits
MISCCR		equ		MBAR+$a0010	; Miscellaneous Control Register (16b)
RTCSRC		cfbit		MISCCR,14	;  RTC Clock Source
USBPUE		cfbit		MISCCR,13	;  USB transceiver pull-up enable.
LIMP		cfbit		MISCCR,12	;  Limp mode enable.
BME		cfbit		MISCCR,11	;  Bus monitor external enable bit.
BMT		cffield		MISCCR,8,3	;  Bus monitor timing field.
SSIPUE		cfbit		MISCCR,7	;  SSI RXD/TXD pull enable.
SSIPUS		cfbit		MISCCR,6	;  SSI RXD/TXD pull select.
TIMDMA		cfbit		MISCCR,5	;  Timer DMA mux selection.
SSISRC		cfbit		MISCCR,4	;  SSI clock source.
LCDCHEN		cfbit		MISCCR,2	;  LCD internal clock enable.
USBOC		cfbit		MISCCR,1	;  USB VBUS over-current sense polarity.
USBSRC		cfbit		MISCCR,0	;  USB clock source.
CDR		equ		MBAR+$a0012	; Clock Divider Register (16b)
LPDIV		cffield		CDR,8,4		;  Low power clock divider.
SSIDIV		cffield		CDR,0,8		;  SSI oversampling clock divider.

;----------------------------------------------------------------------------
; Chip Configuration

MBAR_CC		equ		MBAR+$a0000

CCR		equ		MBAR_CC+$4	; Chip Configuration Register (16b)
DRAMSEL		cfbit		CCR,8		;  FlexBus/SDRAM data pin configuration.
CSC		cffield		CCR,6,2		;  Chip select configuration.
LIMP		cfbit		CCR,5		;  Limp mode bit.
LOAD		cfbit		CCR,4		;  Pad driver load bit.
BOOTPS		cffield		CCR,2,2		;  Boot port size field.
OSCMODE		cfbit		CCR,1		;  Oscillator clock mode bit.
RCON		equ		MBAR_CC+$8	; Reset Configuration Register (16b)
CSC		cffield		RCON,6,2	;  Chip select configuration.
LIMP		cfbit		RCON,5		;  Limp mode bit.
LOAD		cfbit		RCON,4		;  Pad driver load bit.
BOOTPS		cffield		RCON,2,2	;  Boot port size field.
OSCMODE		cfbit		RCON,1		;  Oscillator clock mode bit.
CIR		equ		MBAR_CC+$a	; Chip Identification Register (16b)
PIN		cffield		CIR,4,12	;  Part identification number.
PRN		cffield		CIR,0,4		;  Part revision number.
UOCSR		equ		MBAR_CC+$14	; USB On-the-Go Controller Status Register (16b)
DPPD		cfbit		UOCSR,13	;  D+ 15 kOhm pull-down.
DMPD		cfbit		UOCSR,12	;  D- 15 kOhm pull-down.
CRG_VBUS	cfbit		UOCSR,10	;  Charge VBUS.
DCR_VBUS	cfbit		UOCSR,9		;  Discharge VBUS.
DPPU		cfbit		UOCSR,8		;  D+ pull-up.
AVLD		cfbit		UOCSR,7		;  A-peripheral is valid.
BVLD		cfbit		UOCSR,6		;  B-peripheral is valid.
VVLD		cfbit		UOCSR,5		;  VBUS valid.
SEND		cfbit		UOCSR,4		;  Session end.
WKUP		cfbit		UOCSR,2		;  USB OTG controller wake-up event.
UOMIE		cfbit		UOCSR,1		;  USB OTG miscellaneous interrupt enable.
XPDE		cfbit		UOCSR,0		;  On-chip transceiver pull-down enable.

;----------------------------------------------------------------------------
; Serial Boot Facility

SBFSR		equ		MBAR_CC+$18	; Serial Boot Facility Status Register (16b)
BLL		cffield		SBFSR,0,16	;  Boot load length.
SBFCR		equ		MBAR_CC+$1a	; Serial Boot Facility Control Register (16b)
FR		cfbit		SBFCR,4		;  Fast read.
BLDIV		cffield		SBFCR,0,4	;  Boot loader clock divider.

;----------------------------------------------------------------------------
; Reset Controller

RCR		equ		MBAR_CC+0	; Reset Control Register (8b)
SOFTRST		cfbit		RCR,7		;  Allows software to request a reset.
FRCRSTOUT	cfbit		RCR,6		;  Allows software to assert or negate the external /RSTOUT pin.
RSR		equ		MBAR_CC+1	; Reset Status Register (8b)
SOFT		cfbit		RSR,5		;  Software reset flag.
POR		cfbit		RSR,3		;  Power-on reset flag.
EXT		cfbit		RSR,2		;  External reset flag.
WDRCORE		cfbit		RSR,1		;  Core watchdog timer reset flag.
LOL		cfbit		RSR,0		;  Loss-of-lock reset flag.

;----------------------------------------------------------------------------
; System Control Module

MBAR_SCM	equ		MBAR+$0

__defmprot	macro		{INTLABEL},Reg,Start
__LABEL__.MTR	cfbit		Reg,Start+2	;  Trusted For Read
__LABEL__.MTW	cfbit		Reg,Start+1	;  Trusted For Writes
__LABEL__.MPL	cfbit		Reg,Start+0	;  Privilege Level
		endm
MPR		equ		MBAR+$00	; Master Privilege Register (32b)
MPROT0		__defmprot	MPR,28		;  ColdFire Core
MPROT1		__defmprot	MPR,24		;  eDMA Controller
MPROT4		__defmprot	MPR,12		;  LCD Controller
MPROT6		__defmprot	MPR,4		;  USB On-the-Go
MPROT7		__defmprot	MPR,0		;  Serial Boot
__defpacr	macro		{INTLABEL},Reg,Start
__LABEL__.SP	cfbit		Reg,Start+2	;  Supervisor protect.
__LABEL__.WP	cfbit		Reg,Start+1	;  Write protect.
__LABEL__.TP	cfbit		Reg,Start+0	;  Trusted protect.
		endm
PACRA		equ		MBAR+$20	; Peripheral Access Control Register A (32b)
PACR0		__defpacr	PACRA,28	;  SCM (MPR & PACRs)
PACR1		__defpacr	PACRA,24	;  Crossbar switch
PACR2		__defpacr	PACRA,20	;  FlexBus
PACRB		equ		MBAR+$24	; Peripheral Access Control Register B (32b)
PACR8		__defpacr	PACRB,28	;  FlexCAN
PACR15		__defpacr	PACRB,0		;  Real-Time Clock
PACRC		equ		MBAR+$28	; Peripheral Access Control Register C (32b)
PACR16		__defpacr	PACRC,28	;  SCM (CWT & Core Fault Registers)
PACR17		__defpacr	PACRC,24	;  eDMA Controller
PACR18		__defpacr	PACRC,20	;  Interrupt Controller 0
PACR19		__defpacr	PACRC,16	;  Interrupt Controller 1
PACR21		__defpacr	PACRC,8		;  Interrupt Controller IACK
PACR22		__defpacr	PACRC,4		;  I2C
PACR23		__defpacr	PACRC,0		;  DSPi
PACRD		equ		MBAR+$2C	; Peripheral Access Control Register D (32b)
PACR24		__defpacr	PACRD,28	;  UART0
PACR25		__defpacr	PACRD,24	;  UART1
PACR26		__defpacr	PACRD,20	;  UART2
PACR28		__defpacr	PACRD,12	;  DMA Timer 0
PACR29		__defpacr	PACRD,8		;  DMA Timer 1
PACR30		__defpacr	PACRD,4		;  DMA Timer 2
PACR31		__defpacr	PACRD,0		;  DMA Timer 3
PACRE		equ		MBAR+$40	; Peripheral Access Control Register E (32b)
PACR32		__defpacr	PACRE,28	;  PIT 0
PACR33		__defpacr	PACRE,24	;  PIT 1
PACR36		__defpacr	PACRE,12	;  PWM
PACR37		__defpacr	PACRE,8		;  Edge Port
PACRF		equ		MBAR+$44	; Peripheral Access Control Register F (32b)
PACR40		__defpacr	PACRF,28	;  CCM, Reset Controller, Power Management
PACR41		__defpacr	PACRF,24	;  GPIO Module
PACR42		__defpacr	PACRF,20	;  Touchscreen Controller
PACR43		__defpacr	PACRF,16	;  LCD Controller
PACR44		__defpacr	PACRF,12	;  USB On-the-Go
PACR46		__defpacr	PACRF,4		;  SDRAM Controller
PACR47		__defpacr	PACRF,0		;  SSI
PACRG		equ		MBAR+$48	; Peripheral Access Control Register G (32b)
PACR48		__defpacr	PACRG,28	;  PLL
PACRI		equ		MBAR+$50	; Peripheral Access Control Register I (32b)
PACR64		__defpacr	PACRI,28	;  Global Space 1
PACR65		__defpacr	PACRI,24	;  Global Space 2
CWCR		equ		MBAR_SCM+$40016	; Core Watchdog Control Register (16b)
RO		cfbit		CWCR,15		;  Read-only control bit.
CWCIN16		cfbit		CWCR,9		;  Force CWT carry in 16.
CWRWH		cfbit		CWCR,8		;  Core watchdog run while halted.
CWE		cfbit		CWCR,7		;  Core watchdog timer enable.
CWRI		cffield		CWCR,5,2	;  Core watchdog reset/interrupt.
CWT		cffield		CWCR,0,5	;  Core watchdog time-out period.
CWSR		equ		MBAR_SCM+$4001B	; Core Watchdog Service Register (8b)
SCMISR		equ		MBAR_SCM+$4001F	; SCM Interrupt Status Register (8b)
CFEI		cfbit		SCMISR,1	;  Core fault error interrupt flag.
CWIC		cfbit		SCMISR,0	;  Core watchdog interrupt flag.	
BCR		equ		MBAR_SCM+$40024	; Burst Configuration Register (32b)
GBR		cfbit		BCR,10		;  Global burst enable for reads.
GBW		cfbit		BCR,9		;  Global burst enable for writes.
SBE		cffield		BCR,0,8		;  Slave burst enable
CFADR		equ		MBAR_SCM+$40070	; Core Fault Address Register (32b)
CFIER		equ		MBAR_SCM+$40075	; Core Fault Interrupt Enable Register (8b)
ECFEI		cfbit		CFIER,0		;  Enable core fault error interrupt.
CFLOC		equ		MBAR_SCM+$40076	; Core Fault Location Register (8b)
LOC		cfbit		CFLOC,7		;  The location of the last captured fault.
CFATR		equ		MBAR_SCM+$40077	; Core Fault Attributes Register (8b)
WRITE		cfbit		CFATR,7		;  Indicates the direction of the last faulted core access.
SIZE		cffield		CFATR,4,3	;  Indicates the size of the last faulted core access.
MODE		cfbit		CFATR,1		;  Indicates the mode the device was in during the last faulted core access.
TYPE		cfbit		CFATR,0		;  Defines the type of last faulted core access.
CFDTR		equ		MBAR_SCM+$4007C	; Core Fault Data Register (32b)

;----------------------------------------------------------------------------
; Crossbar Switch

MBAR_XBS	equ		MBAR+$4000
__defxbs	macro		n,Base
XBS_PRS{n}	equ		Base+$00	; Priority Register (32b)
M7		cffield		XBS_PRS{n},28,3	;  Master 7 (Serial Boot) Priority
M6		cffield		XBS_PRS{n},24,3 ;  Master 6 (USB OTG) Priority
M4		cffield		XBS_PRS{n},16,3 ;  Master 4 (LCD Controller) Priority
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

PODR_BE		equ		MBAR_GPIO+$000	; Byte Enable Output Data Register (8b)
PODR_CS		equ		MBAR_GPIO+$001	; Chip Select Output Data Register (8b)
PODR_FBCTL	equ		MBAR_GPIO+$002	; FlexBus Output Data Register (8b)
PODR_I2C	equ		MBAR_GPIO+$003	; I2C Output Data Register (8b)
PODR_UART	equ		MBAR_GPIO+$005	; UART Output Data Register (8b)
PODR_DSPI	equ		MBAR_GPIO+$006	; DSPI Output Data Register (8b)
PODR_TIMER	equ		MBAR_GPIO+$007	; Timer Output Data Register (8b)
PODR_LCDCTL	equ		MBAR_GPIO+$008	; LCD Controller Output Data Register (8b)
PODR_LCDDATAH	equ		MBAR_GPIO+$009	; LCD Controller Data High Output Data Register (8b)
PODR_LCDDATAM	equ		MBAR_GPIO+$00a	; LCD Controller Data Mid Output Data Register (8b)
PODR_LCDDATAL	equ		MBAR_GPIO+$00b	; LCD Controller Data Low Output Data Register (8b)

; Port Data Direction Registers

PDDR_BE		equ		MBAR_GPIO+$00c	; Byte Enable Data Direction Register (8b)
PDDR_CS		equ		MBAR_GPIO+$00d	; Chip Select Data Direction Register (8b)
PDDR_FBCTL	equ		MBAR_GPIO+$00e	; FlexBus Data Direction Register (8b)
PDDR_I2C	equ		MBAR_GPIO+$00f	; I2C Data Direction Register (8b)
PDDR_UART	equ		MBAR_GPIO+$011	; UART Data Direction Register (8b)
PDDR_DSPI	equ		MBAR_GPIO+$012	; DSPI Data Direction Register (8b)
PDDR_TIMER	equ		MBAR_GPIO+$013	; Timer Data Direction Register (8b)
PDDR_LCDCTL	equ		MBAR_GPIO+$014	; LCD Controller Data Direction Register (8b)
PDDR_LCDDATAH	equ		MBAR_GPIO+$015	; LCD Controller Data High Data Direction Register (8b)
PDDR_LCDDATAM	equ		MBAR_GPIO+$016	; LCD Controller Data Mid  Output Data Register (8b)
PDDR_LCDDATAL	equ		MBAR_GPIO+$017	; LCD Controller Data Low Data Direction Register (8b)

; Port Pin Data/Set Data Registers

PPDSDR_BE	equ		MBAR_GPIO+$018	; Byte Enable Data Set Register (8b)
PPDSDR_CS	equ		MBAR_GPIO+$019	; Chip Select Data Set Register (8b)
PPDSDR_FBCTL	equ		MBAR_GPIO+$01a	; FlexBus Data Set Register (8b)
PPDSDR_I2C	equ		MBAR_GPIO+$01b	; I2C Data Set Register (8b)
PPDSDR_UART	equ		MBAR_GPIO+$01d	; UART Data Set Register (8b)
PPDSDR_DSPI	equ		MBAR_GPIO+$01e	; DSPI Data Set Register (8b)
PPDSDR_TIMER	equ		MBAR_GPIO+$01f	; Timer Data Set Register (8b)
PPDSDR_LCDCTL	equ		MBAR_GPIO+$020	; LCD Controller Data Set Register (8b)
PPDSDR_LCDDATAH	equ		MBAR_GPIO+$021	; LCD Controller Data High Data Direction Register (8b)
PPDSDR_LCDDATAM	equ		MBAR_GPIO+$022	; LCD Controller Data Mid Output Data Register (8b)
PPDSDR_LCDDATAL	equ		MBAR_GPIO+$023	; LCD Controller Data Low Data Direction Register (8b)


; Port Clear Output Data Registers

PCLRR_BE	equ		MBAR_GPIO+$024	; Byte Enable Data Clear Register (8b)
PCLRR_CS	equ		MBAR_GPIO+$025	; Chip Select Data Clear Register (8b)
PCLRR_FBCTL	equ		MBAR_GPIO+$026	; FlexBus Data Clear Register (8b)
PCLRR_I2C	equ		MBAR_GPIO+$027	; I2C Data Clear Register (8b)
PCLRR_UART	equ		MBAR_GPIO+$029	; UART Data Clear Register (8b)
PCLRR_DSPI	equ		MBAR_GPIO+$02a	; DSPI Data Clear Register (8b)
PCLRR_TIMER	equ		MBAR_GPIO+$02b	; Timer Data Clear Register (8b)
PCLRR_LCDCTL	equ		MBAR_GPIO+$02c	; LCD Controller Data Clear Register (8b)
PCLRR_LCDDATAH	equ		MBAR_GPIO+$02d	; LCD Controller Data High Data Clear Register (8b)
PCLRR_LCDDATAM	equ		MBAR_GPIO+$02e	; LCD Controller Data Mid Output Data Register (8b)
PCLRR_LCDDATAL	equ		MBAR_GPIO+$02f	; LCD Controller Data Low Data Clear Register (8b)

; Pin Assignment Registers

PAR_BE		equ		MBAR_GPIO+$030	; Byte Enable Pin Assignment Register (8b)
PAR_BE3		cfbit		PAR_BE,3	;  /FB_BE3 pin assignment.
PAR_BE2		cfbit		PAR_BE,2	;  /FB_BE2 pin assignment.
PAR_BE1		cfbit		PAR_BE,1	;  /FB_BE1 pin assignment.
PAR_BE0		cfbit		PAR_BE,0	;  /FB_BE0 pin assignment.
PAR_CS		equ		MBAR_GPIO+$031	; Chip Select Pin Assignment Register (8b)
PAR_CS3		cfbit		PAR_CS,4	;  /FB_CS3 pin assignment.
PAR_CS2		cfbit		PAR_CS,3	;  /FB_CS2 pin assignment.
PAR_CS1		cffield		PAR_CS,1,2	;  /FB_CS1 pin assignment.
PAR_CS0		cfbit		PAR_CS,0	;  /FB_CS0 pin assignment.
PAR_FBCTL	equ		MBAR_GPIO+$032	; FlexBus Pin Assignment Register (8b)
PAR_OE		cfbit		PAR_FBCTL,7	;  /FB_OE pin assignment.
PAR_TA		cfbit		PAR_FBCTL,6	;  /FB_TA pin assignment.
PAR_RWB		cfbit		PAR_FBCTL,5	;  /FB_R/W pin assignment.
PAR_TS		cffield		PAR_FBCTL,3,2	;  /FB_TS pin assignment.
PAR_I2C		equ		MBAR_GPIO+$033	; I2C Pin Assignment Register (8b)
PAR_SCL		cffield		PAR_I2C,2,2	;  I2C_SCL pin assignment.
PAR_SDA		cffield		PAR_I2C,0,2	;  I2C_SDA pin assignment.
PAR_UART	equ		MBAR_GPIO+$034	; UART Pin Assignment Register (16b)
PAR_U1CTS	cffield		PAR_UART,14,2	;  UART1 CTS pin assignment.
PAR_U1RTS	cffield		PAR_UART,12,2	;  UART1 RTS pin assignment.
PAR_U1RXD	cffield		PAR_UART,10,2	;  UART1 RxD pin assignment.
PAR_U1TXD	cffield		PAR_UART,8,2	;  UART1 TxD pin assignment.
PAR_U0CTS	cffield		PAR_UART,6,2	;  UART0 CTS pin assignment.
PAR_U0RTS	cffield		PAR_UART,4,2	;  UART0 RTS pin assignment.
PAR_U0RXD	cffield		PAR_UART,2,2	;  UART0 RxD pin assignment.
PAR_U0TXD	cffield		PAR_UART,0,2	;  UART0 TxD pin assignment.
PAR_DSPI	equ		MBAR_GPIO+$036	; DSPI Pin Assignment Register (8b)
PAR_PCS0	cffield		PAR_DSPI,6,2	;  DSPI PCS0 pin assignment.
PAR_SIN		cffield		PAR_DSPI,4,2	;  DSPI SIN pin assignment.
PAR_SOUT	cffield		PAR_DSPI,2,2	;  DSPI SOUT pin assignment.
PAR_SCK		cffield		PAR_DSPI,0,2	;  DSPI SCK pin assignment.
PAR_TIMER	equ		MBAR_GPIO+$037	; Timer Pin Assignment Register (8b)
PAR_T3IN	cffield		PAR_TIMER,6,2	;  Timer 3 In pin assignment.
PAR_T2IN	cffield		PAR_TIMER,4,2	;  Timer 2 In pin assignment.
PAR_T1IN	cffield		PAR_TIMER,2,2	;  Timer 1 In pin assignment.
PAR_T0IN	cffield		PAR_TIMER,0,2	;  Timer 0 In pin assignment.
PAR_LCDCTL	equ		MBAR_GPIO+$038	; LCD Controller Pin Assignment Register (8b)
PAR_ACD_OE	cffield		PAR_LCDCTL,3,2	;  LCD_ACD/OE pin assignment.
PAR_FLM_VSYNC	cfbit		PAR_LCDCTL,2	;  LCD_FLM/VSYNC pin assignment.
PAR_LP_HSYNC	cfbit		PAR_LCDCTL,1	;  LCD_LP/HSYNC pin assignment.
PAR_LSCLK	cfbit		PAR_LCDCTL,0	;  LCD_LSCLK pin assignment.
PAR_IRQ		equ		MBAR_GPIO+$039	; IRQ Pin Assignment Register (8b)
PAR_IRQ4	cffield		PAR_IRQ,2,2	;  /IRQ4 pin assignment.
PAR_IRQ1	cffield		PAR_IRQ,0,2	;  /IRQ1 pin assignment.
PAR_LCDH	equ		MBAR_GPIO+$03c	; LCD Controller Data High Pin Assignment Register (32b)
PAR_LD17	cffield		PAR_LCDH,10	;  LD17 pin assignment.
PAR_LD16	cffield		PAR_LCDH,8	;  LD16 pin assignment.
PAR_LD15	cffield		PAR_LCDH,6	;  LD15 pin assignment.
PAR_LD14	cffield		PAR_LCDH,4	;  LD14 pin assignment.
PAR_LD13	cffield		PAR_LCDH,2	;  LD13 pin assignment.
PAR_LD12	cffield		PAR_LCDH,0	;  LD12 pin assignment.
PAR_LCDL	equ		MBAR_GPIO+$040	; LCD Controller Data Low Pin Assignment Register (32b)
PAR_LD11	cffield		PAR_LCDL,30	;  LD11 pin assignment.
PAR_LD10	cffield		PAR_LCDL,28	;  LD10 pin assignment.
PAR_LD9		cffield		PAR_LCDL,26	;  LD9 pin assignment.
PAR_LD8		cffield		PAR_LCDL,24	;  LD8 pin assignment.
PAR_LD7		cffield		PAR_LCDL,22	;  LD7 pin assignment.
PAR_LD6		cffield		PAR_LCDL,20	;  LD6 pin assignment.
PAR_LD5		cffield		PAR_LCDL,18	;  LD5 pin assignment.
PAR_LD4		cffield		PAR_LCDL,16	;  LD4 pin assignment.
PAR_LD3		cffield		PAR_LCDL,14	;  LD3 pin assignment.
PAR_LD2		cffield		PAR_LCDL,12	;  LD2 pin assignment.
PAR_LD1		cffield		PAR_LCDL,10	;  LD1 pin assignment.
PAR_LD0		cffield		PAR_LCDL,8	;  LD0 pin assignment.

; Mode Select Control Registers

MSCR_FLEXBUS	equ		MBAR_GPIO+$044	; FlexBus Mode Select Control Register (8b)
MSCR_DUPPER	cffield		MSCR_FLEXBUS,4,2;  FB_D[31:16] Mode Select Control
MSCR_DLOWER	cffield		MSCR_FLEXBUS,2,2;  FB_D[15:0] Mode Select Control
MSCR_ADDRCTL	cffield		MSCR_FLEXBUS,0,2;  FB_A[23:0], BE/BWE[3:0], OE, R/W, FB_CS[5:0], TA, and TS Mode Select Control
MSCR_SDRAM	equ		MBAR_GPIO+$045	; SDRAM Mode Select Control Register (8b)
MSCR_SDCLKB	cffield		MSCR_SDRAM,4,2	;  SD_CLK Mode Select Control
MSCR_SDCLK	cffield		MSCR_SDRAM,2,2	;  SD_CLK Mode Select Control
MSCR_SDRAM	cffield		MSCR_SDRAM,0,2	;  SD_A10, SD_CAS, SD_CKE, SD_CS0, SD_DQS[3:2], SD_RAS, SD_SDRDQS, SD_WE Mode Select Control

; Drive Strength Control Registers

DSCR_DSPI	equ		MBAR_GPIO+$048	; DSPI Drive Strength Register (8b)
DSE_DSPI	cffield		DSCR_DSPI,0,2
DSCR_TIMER	equ		MBAR_GPIO+$049	; Timer Drive Strength Register (8b)
DSE_TIMER	cffield		DSCR_TIMER,0,2
DSCR_I2C	equ		MBAR_GPIO+$04a	; I2C Drive Strength Register (8b)
DSE_I2C		cffield		DSCR_I2C,0,2
DSCR_LCD	equ		MBAR_GPIO+$04b	; LCD Controller Drive Strength Register (8b)
DSE_LCD		cffield		DSCR_LCD,0,2
DSCR_DEBUG	equ		MBAR_GPIO+$04c	; Debug Drive Strength Register (8b)
DSE_DEBUG	cffield		DSCR_DEBUG,0,2
DSCR_CLKRST	equ		MBAR_GPIO+$04d	; Clock/Reset Drive Strength Register (8b)
DSE_RSTOUT	cffield		DSCR_CLKRST,2,2
DSE_FBCLK	cffield		DSCR_CLKRST,0,2
DSCR_IRQ	equ		MBAR_GPIO+$04e	; IRQ Pins Drive Strength Register (8b)
DSE_IRQ		cffield		DSCR_IRQ,0,2
DSCR_UART	equ		MBAR_GPIO+$04f	; UART Drive Strength Register (8b)
DSE_UART1	cffield		DSCR_UART,2,2
DSE_UART0	cffield		DSCR_UART,0,2


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
; USB On-the-Go

		include		"coldfire/52xxusb.inc"
MBAR_OTG	equ		MBAR+$b0000
		__defusb	"",MBAR_OTG

;----------------------------------------------------------------------------
; LCD Controller

MBAR_LCDC	equ		$ac000
		include		"coldfire/52xxlcdc.inc"

;----------------------------------------------------------------------------
; Touchscreen Controller

MBAR_ASP	equ		MBAR+$a8000

ASP_CR		equ		MBAR_ASP+$000	; ASP Control Register (32b)
ASPE		cfbit		ASP_CR,31	;  ASP conversion enable.
DOZE		cfbit		ASP_CR,23	;  Module Doze Enable.
PENE		cfbit		ASP_CR,20	;  Pen Detection Enable.
MDIS		cfbit		ASP_CR,19	;  Module Disable.
TSE		cfbit		ASP_CR,18	;  Touchscreen enable.
TSTYPE		cffield		ASP_CR,16,2	;  Touchscreen Type.
AZE		cfbit		ASP_CR,15	;  Auto-Zero Enable.
CALA		cfbit		ASP_CR,14	;  Calibration assist.
AUTO		cfbit		ASP_CR,10	;  Auto-round control.
MODE		cffield		ASP_CR,8,2	;  Auto-sampling mode.
APTN		cffield		ASP_CR,0,8	;  ASP sampling pattern.
ASP_SET		equ		MBAR_ASP+$004	; ASP Sampling Setting Register (32b)
REFP		cffield		ASP_SET,28,3	;  Positive reference.
REFN		cffield		ASP_SET,24,3	;  Negative reference.
TRIMC		cffield		ASP_SET,16,8	;  Trim value for ADC.
INP		cffield		ASP_SET,12,3	;  Positive input.
SW		cffield		ASP_SET,0,9	;  Bias switch control.
ASP_TIM		equ		MBAR_ASP+$008	; A/D Sampling Timing Register (32b)
RIDLECNT	cffield		ASP_TIM,16,16	;  Idle time setting between two rounds of measurements
MIDLECNT	cffield		ASP_TIM,8,8	;  Idle time setting between two successive measurements.
BIACNT		cffield		ASP_TIM,4,4	;  Bias phase count.
SPCNT		cffield		ASP_TIM,0,4	;  Sample pulse count.
ASP_ICR		equ		MBAR_ASP+$00C	; ASP Interrupt/DMA Control Register (32b)
FIFO_WM		cffield		ASP_ICR,16,6	;  FIFO watermark.
POVIE		cfbit		ASP_ICR,7	;  Pen FIFO overflow interrupt enable.
PUIE		cfbit		ASP_ICR,5	;  Pen-up interrupt enable.
PDIE		cfbit		ASP_ICR,4	;  Pen-down interrupt enable.
PFLDE		cfbit		ASP_ICR,3	;  Pen FIFO level DMA request enable.
PFLIE		cfbit		ASP_ICR,2	;  Pen FIFO level interrupt request enable.
PFFIE		cfbit		ASP_ICR,1	;  Pen FIFO full interrupt request enable.
PDRIE		cfbit		ASP_ICR,0	;  Pen data ready interrupt enable.
ASP_ISR		equ		MBAR_ASP+$010	; ASP Interrupt Status Register (32b)
POVF		cfbit		ASP_ISR,7	;  Pen sample FIFO overflow flag.
PUF		cfbit		ASP_ISR,5	;  Pen up flag.
PDF		cfbit		ASP_ISR,4	;  Pen down flag.
PFLF		cfbit		ASP_ISR,2	;  Pen FIFO programmable level flag.
PFFF		cfbit		ASP_ISR,1	;  Pen FIFO full flag.
PDRF		cfbit		ASP_ISR,0	;  Pen data ready flag.
ASP_SFIFO	equ		MBAR_ASP+$014	; ASP Sample FIFO (32b)
CHANID		cffield		ASP_SFIFO,12,4	;  Channel ID for corresponding sample data.
SAMPLE		cffield		ASP_SFIFO,0,12	;  Sample.
ASP_FIFOP	equ		MBAR_ASP+$018	; ASP FIFO Pointer (32b)
FFRP		cffield		ASP_FIFOP,16,6	;  FIFO read pointer.
FFWP		cffield		ASP_FIFOP,0,6	;  FIFO write pointer.
ASP_CLKDIV	equ		MBAR_ASP+$01C	; ASP Clock Divider Register (32b)
CLKD		cffield		ASP_CLKDIV,0,7	;  ASP clock divider.

;----------------------------------------------------------------------------
; FlexCAN

MBAR_CAN	equ		MBAR+$20000
		include		"coldfire/52xxcan.inc"

__N		set		0
		rept	16
		__decstr	__NS,__N
RXIMR{"\{__NS}"}	equ	MBAR_CAN+$880+(__N*4)	; Rx Individual Mask Registers (16*32b)
EXTID		cffield		RXIMR{"\{__NS}"},18,11	;  Extended ID mask bits.
STDID		cffield		RXIMR{"\{__NS}"},0,18	;  Standard ID mask bits.
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; Pulse-Width Modulation Module

MBAR_PWM	equ		MBAR+$90000
		include		"coldfire/52xxpwm.inc"

;----------------------------------------------------------------------------
; Synchronous Serial Interface

MBAR_SSI	equ		MBAR+$bc000
		include		"coldfire/52xxssi.inc"

;----------------------------------------------------------------------------
; Real Time Clock

MBAR_RTC	equ		MBAR+$3c000
		include		"coldfire/52xxrtc.inc"

RTC_GOCU	equ		MBAR_RTC+$034	; RTC General Oscillator Clock Upper (32b)
CNT_UPPER	cffield		RTC_GOCU,0,16	;  Upper word of RTC_GOC[31:0]
RTC_GOCL	equ		MBAR_RTC+$038	; RTC General Oscillator Clock Lower (32b)
CNT_LOWER	cffield		RTC_GOCL,0,16	;  Lower word of RTC_GOC[31:0]

;----------------------------------------------------------------------------
; Programmable Interrupt Timer

		include		"coldfire/52xxpit.inc"
		__defpit	"0",MBAR+$80000
		__defpit	"1",MBAR+$84000

;----------------------------------------------------------------------------
; DMA Timer

		include		"coldfire/52xxdtim.inc"
		__defdtim	"0",MBAR+$70000
		__defdtim	"1",MBAR+$74000
		__defdtim	"2",MBAR+$78000
		__defdtim	"3",MBAR+$7c000

;----------------------------------------------------------------------------
; DMA Serial Peripheral Interface

MBAR_DSPI	equ		MBAR+$5c000

DSPI_MCR	equ		MBAR_DSPI+$000	; DSPI module configuration register (32b)
MSTR		cfbit		DSPI_MCR,31	;  Master/slave mode select.
CONT_SCKE	cfbit		DSPI_MCR,30	;  Continuous SCK enable.
DCONF		cffield		DSPI_MCR,28,2	;  DSPI configuration.
FRZ		cfbit		DSPI_MCR,27	;  Freeze.
MTFE		cfbit		DSPI_MCR,26	;  Modified timing format enable.
ROOE		cfbit		DSPI_MCR,24	;  Receive FIFO overflow overwrite enable.
PCSIS7		cfbit		DSPI_MCR,23	;  Peripheral chip select inactive state n
PCSIS6		cfbit		DSPI_MCR,22
PCSIS5		cfbit		DSPI_MCR,21
PCSIS4		cfbit		DSPI_MCR,20
PCSIS3		cfbit		DSPI_MCR,19
PCSIS2		cfbit		DSPI_MCR,18
PCSIS1		cfbit		DSPI_MCR,17
PCSIS0		cfbit		DSPI_MCR,16
MDIS		cfbit		DSPI_MCR,14	;  Module disable.
DIS_TXF		cfbit		DSPI_MCR,13	;  Disable transmit FIFO.
DIS_RXF		cfbit		DSPI_MCR,12	;  Disable receive FIFO.
CLR_TXF		cfbit		DSPI_MCR,11	;  Clear TX FIFO.
CLR_RXF		cfbit		DSPI_MCR,10	;  Clear RX FIFO.
SMPL_PT		cffield		DSPI_MCR,8,2	;  Sample point.
HALT		cfbit		DSPI_MCR,0	;  Halt.
DSPI_TCR	equ		MBAR_DSPI+$008	; DSPI transfer count register (32b)
SPI_TCNT	cffield		DSPI_TCR,16,16	;  SPI transfer counter.
__N		set		0
		rept		8
		__decstr	__NS,__N
DSPI_CTAR{"\{__NS}"}	equ	MBAR_DSPI+$00C+(__N*4)	; DSPI clock and transfer attributes registers (8*32b)
DBR		cfbit		DSPI_CTAR{"\{__NS}"},31		;  Double baud rate.
FMSZ		cffield		DSPI_CTAR{"\{__NS}"},27,4	;  Frame size.
CPOL		cfbit		DSPI_CTAR{"\{__NS}"},26		;  Clock polarity.
CPHA		cfbit		DSPI_CTAR{"\{__NS}"},25		;  Clock phase.
LSBFE		cfbit		DSPI_CTAR{"\{__NS}"},24		;  LSB first enable.
PCSSCK		cffield		DSPI_CTAR{"\{__NS}"},22,2	;  PCS to SCK delay prescaler.
PASC		cffield		DSPI_CTAR{"\{__NS}"},20,2	;  After SCK delay prescaler.
PDT		cffield		DSPI_CTAR{"\{__NS}"},18,2	;  Delay after transfer prescaler.
PBR		cffield		DSPI_CTAR{"\{__NS}"},16,2	;  Baud rate prescaler.
CSSCK		cffield		DSPI_CTAR{"\{__NS}"},12,4	;  PCS to SCK delay scaler.
ASC		cffield		DSPI_CTAR{"\{__NS}"},8,4	;  After SCK delay scaler.
DT		cffield		DSPI_CTAR{"\{__NS}"},4,4	;  Delay after transfer scaler.
BR		cffield		DSPI_CTAR{"\{__NS}"},0,4	;  Baud rate scaler.
__N		set		__N+1
		endm
DSPI_SR		equ		MBAR_DSPI+$02C	; DSPI status register (32b)
TCF		cfbit		DSPI_SR,31	;  Transfer complete flag.
TXRXS		cfbit		DSPI_SR,30	;  TX and RX status.
EOQF		cfbit		DSPI_SR,28	;  End of queue flag.
TFUF		cfbit		DSPI_SR,27	;  Transmit FIFO underflow flag.
TFFF		cfbit		DSPI_SR,25	;  Transmit FIFO fill flag.
RFOF		cfbit		DSPI_SR,19	;  Receive FIFO overflow flag.
RFDF		cfbit		DSPI_SR,17	;  Receive FIFO drain flag.
TXCTR		cffield		DSPI_SR,12,4	;  TX FIFO counter.
TXNXTPTR	cffield		DSPI_SR,8,4	;  Transmit next pointer.
RXCTR		cffield		DSPI_SR,4,4	;  RX FIFO counter.
POPNXTPTR	cffield		DSPI_SR,0,4	;  Pop next pointer.
DSPI_RSER	equ		MBAR_DSPI+$030	; DSPI DMA/interrupt request select and enable register (32b)
TCF_RE		cfbit		DSPI_RSER,31	;  Transmission complete request enable.
EOQF_RE		cfbit		DSPI_RSER,28	;  DSPI finished request enable.
TFUF_RE		cfbit		DSPI_RSER,27	;  Transmit FIFO underflow request enable.
TFFF_RE		cfbit		DSPI_RSER,25	;  Transmit FIFO fill request enable.
TFFF_DIRS	cfbit		DSPI_RSER,24	;  Transmit FIFO fill DMA or interrupt request select.
RFOF_RE		cfbit		DSPI_RSER,19	;  Receive FIFO overflow request enable.
RFDF_RE		cfbit		DSPI_RSER,17	;  Receive FIFO drain request enable.
RFDF_DIRS	cfbit		DSPI_RSER,16	;  Receive FIFO drain DMA or interrupt request select.
DSPI_PUSHR	equ		MBAR_DSPI+$034	; DSPI push TX FIFO register (32b)
CONT		cfbit		DSPI_PUSHR,31	;  Continuous peripheral chip select enable.
CTAS		cffield		DSPI_PUSHR,28,3	;  Clock and transfer attributes select.
EOQ		cfbit		DSPI_PUSHR,27	;  End of queue.
CTCNT		cfbit		DSPI_PUSHR,26	;  Clear SPI_TCNT.
PCS7		cfbit		DSPI_PUSHR,23	;  Peripheral chip select n.
PCS6		cfbit		DSPI_PUSHR,22
PCS5		cfbit		DSPI_PUSHR,21
PCS4		cfbit		DSPI_PUSHR,20
PCS3		cfbit		DSPI_PUSHR,19
PCS2		cfbit		DSPI_PUSHR,18
PCS1		cfbit		DSPI_PUSHR,17
PCS0		cfbit		DSPI_PUSHR,16
TXDATA		cffield		DSPI_PUSHR,0,16	;  Transmit Data
DSPI_POPR	equ		MBAR_DSPI+$038	; DSPI pop RX FIFO register (32b)
RXDATA		cffield		DSPI_POPR,0,16	;  Received data.
__DSPI_TXRX	macro
DSPI_TXFR{"\{__NS}"}	equ	MBAR_DSPI+$03C+(__N*4) ; DSPI transmit FIFO registers (32b)
TXCMD		cffield		DSPI_TXFR{"\{__NS}"},16,16 ;  Transmit command.
TXDATA		cffield		DSPI_TXFR{"\{__NS}"},0,16  ;  Transmit data.
DSPI_RXFR{"\{__NS}"}	equ	MBAR_DSPI+$07C+(__N*4) ; DSPI receive FIFO registers (32b)
RXDATA		cffield		DSPI_RXFR{"\{__NS}"},0,16  ;  Receive data.
		endm
		__enumregs	__DSPI_TXRX,0,15

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
		
		endif				; __mcf52277inc
		