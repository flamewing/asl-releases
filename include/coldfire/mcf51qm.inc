		ifndef	__mcf51qminc		; avoid multiple inclusion
__mcf51qminc	equ	1

		save
		listing	off			; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MCF51QM.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF51QM                  *
;*                                                                          *
;****************************************************************************

MBAR		equ		$ffff8000

;----------------------------------------------------------------------------
; Rapid General Purpose I/O

MBAR_RGPIO	equ		$00c00000

RGPIO_DIR	equ		MBAR_RGPIO+$00		; RGPIO Data Direction Register (16b)
DIR		cffield		RGPIO_DIR,0,15		;  Data direction
RGPIO_DATA	equ		MBAR_RGPIO+$02		; RGPIO Data Register (16b)
DATA		cffield		RGPIO_DATA,0,15		;  RGPIO data
RGPIO_ENB	equ		MBAR_RGPIO+$04		; RGPIO Pin Enable Register (16b)
ENB		cffield		RGPIO_ENB,0,15		;  Enable pin for RGPIO
RGPIO_CLR	equ		MBAR_RGPIO+$06		; RGPIO Clear Data Register (16b)
CLR		cffield		RGPIO_CLR,0,15		;  Clear data
RGPIO_DIR_RO	equ		MBAR_RGPIO+$08		; RGPIO Data Direction Register (ro) (16b)
DIR_RO		cffield		RGPIO_DIR_RO,0,15	;  Data direction
RGPIO_SET	equ		MBAR_RGPIO+$0A		; RGPIO Set Data Register (8b)
SET		cffield		RGPIO_SET,0,15		;  Set data
RGPIO_DIR_RO_2	equ		MBAR_RGPIO+$0C		; RGPIO Data Direction Register (ro, once again?) (8b)
DIR_RO_2	cffield		RGPIO_DIR_RO_2,0,15	;   Data direction
RGPIO_TOG	equ		MBAR_RGPIO+$0E		; RGPIO Toggle Data Register (8b)
TOG		cffield		RGPIO_TOG,0,15		;  Toggle data

;----------------------------------------------------------------------------
; Enhanced General Purpose I/O

__defegpio	macro		ID,Base
PT{ID}_D	equ		Base+$00		; Port Data Register (8b)
		__enumbits	PT{ID}_D,PTD,8		;  Port data bits
PT{ID}_DD	equ		Base+$01		; Port Data Direction Register (8b)
		__enumbits	PT{ID}_DD,PTDD,8	;  Port data direction bits
PT{ID}_PV	equ		Base+$02		; Port Pin Value Register (8b)
		__enumbits	PT{ID}_PV,PTPV,8	;  Port pin value bits
		endm

		__defegpio	"A",MBAR+$0000
		__defegpio	"B",MBAR+$0010
		__defegpio	"C",MBAR+$0020
		__defegpio	"D",MBAR+$0030
		__defegpio	"E",MBAR+$0040
		__defegpio	"F",MBAR+$0050

;----------------------------------------------------------------------------
; Register File

MBAR_RF		equ		MBAR+$0060

;----------------------------------------------------------------------------
; Port Mux Controls

MBAR_MXC	equ		MBAR+$0080

__defmux	macro		ID,Base
PT{ID}PF1	equ		Base+0			; Port x Pin Function 1 Register (8b)
{ID}7		cffield		PT{ID}PF1,4,4		;  Port x.7 Pin Mux Controls
{ID}6		cffield		PT{ID}PF1,0,4		;  Port x.6 Pin Mux Controls
PT{ID}PF2	equ		Base+1			; Port x Pin Function 2 Register (8b)
{ID}5		cffield		PT{ID}PF2,4,4		;  Port x.5 Pin Mux Controls
{ID}4		cffield		PT{ID}PF2,0,4		;  Port x.4 Pin Mux Controls
PT{ID}PF3	equ		Base+2			; Port x Pin Function 3 Register (8b)
{ID}3		cffield		PT{ID}PF3,4,4		;  Port x.3 Pin Mux Controls
{ID}2		cffield		PT{ID}PF3,0,4		;  Port x.2 Pin Mux Controls
PT{ID}PF4	equ		Base+3			; Port x Pin Function 4 Register (8b)
{ID}1		cffield		PT{ID}PF4,4,4		;  Port x.1 Pin Mux Controls
{ID}0		cffield		PT{ID}PF4,0,4		;  Port x.0 Pin Mux Controls
		endm

		__defmux	"A",MBAR_MXC+$00
		__defmux	"B",MBAR_MXC+$04
		__defmux	"C",MBAR_MXC+$08
		__defmux	"D",MBAR_MXC+$0c
		__defmux	"E",MBAR_MXC+$10
		__defmux	"F",MBAR_MXC+$14

;----------------------------------------------------------------------------
; External Interrupt Module

MBAR_IRQ	equ		MBAR+$00a0

IRQ_SC		equ		MBAR_IRQ+$00		; Interrupt status and control register
IRQPDD		cfbit		IRQ_SC,6		;  IRQ pull device disable
IRQEDG		cfbit		IRQ_SC,5		;  IRQ edge select
IRQPE		cfbit		IRQ_SC,4		;  IRQ pin enable
IRQF		cfbit		IRQ_SC,3		;  IRQ flag
IRQACK		cfbit		IRQ_SC,2		;  IRQ acknowledge
IRQIE		cfbit		IRQ_SC,1		;  IRQ interrupt enable
IRQMOD		cfbit		IRQ_SC,0		;  IRQ detection mode

;----------------------------------------------------------------------------
; Low Leakage Wakeup Unit

MBAR_LLWU	equ		MBAR+$00b0

LLWU_PE1	equ		MBAR_LLWU+$00		; LLWU Pin Enable 1 register (8b)
WUPE3		cffield		LLWU_PE1,6,2		;  Wakeup Pin Enable For LLWU_P3
WUPE2		cffield		LLWU_PE1,4,2		;  Wakeup Pin Enable For LLWU_P2
WUPE1		cffield		LLWU_PE1,2,2		;  Wakeup Pin Enable For LLWU_P1
WUPE0		cffield		LLWU_PE1,0,2		;  Wakeup Pin Enable For LLWU_P0
LLWU_PE2	equ		MBAR_LLWU+$01		; LLWU Pin Enable 2 register (8b)
WUPE7		cffield		LLWU_PE2,6,2		;  Wakeup Pin Enable For LLWU_P7
WUPE6		cffield		LLWU_PE2,4,2		;  Wakeup Pin Enable For LLWU_P6
WUPE5		cffield		LLWU_PE2,2,2		;  Wakeup Pin Enable For LLWU_P5
WUPE4		cffield		LLWU_PE2,0,2		;  Wakeup Pin Enable For LLWU_P4
LLWU_PE3	equ		MBAR_LLWU+$02		; LLWU Pin Enable 3 register (8b)
WUPE11		cffield		LLWU_PE3,6,2		;  Wakeup Pin Enable For LLWU_P11
WUPE10		cffield		LLWU_PE3,4,2		;  Wakeup Pin Enable For LLWU_P10
WUPE9		cffield		LLWU_PE3,2,2		;  Wakeup Pin Enable For LLWU_P9
WUPE8		cffield		LLWU_PE3,0,2		;  Wakeup Pin Enable For LLWU_P8
LLWU_PE4	equ		MBAR_LLWU+$03		; LLWU Pin Enable 4 register (8b)
WUPE15		cffield		LLWU_PE4,6,2		;  Wakeup Pin Enable For LLWU_P15
WUPE14		cffield		LLWU_PE4,4,2		;  Wakeup Pin Enable For LLWU_P14
WUPE13		cffield		LLWU_PE4,2,2		;  Wakeup Pin Enable For LLWU_P13
WUPE12		cffield		LLWU_PE4,0,2		;  Wakeup Pin Enable For LLWU_P12
LLWU_ME		equ		MBAR_LLWU+$04		; LLWU Module Enable register (8b)
		__enumbits	LLWU_ME,WUPE,8		;  Wakeup Module Enable For Module 0..7
LLWU_F1		equ		MBAR_LLWU+$05		; LLWU Flag 1 register (8b)
		__enumbits	LLWU_F1,WUF,8		;  Wakeup Flag For LLWU_Pn
LLWU_F2		equ		MBAR_LLWU+$06		; LLWU Flag 2 register (8b)
		__enumbits	LLWU_F2,WUF,8,8
LLWU_F3		equ		MBAR_LLWU+$07		; LLWU Flag 3 register (8b)
		__enumbits	LLWU_F3,MWUF,8		;  Wakeup flag For module n
LLWU_FILT1	equ		MBAR_LLWU+$08		; LLWU Pin Filter 1 register (8b)
FILTF		cfbit		LLWU_FILT1,7		;  Filter Detect Flag
FILTE		cffield		LLWU_FILT1,5,2		;  Digital Filter On External Pin
FILTSEL		cffield		LLWU_FILT1,0,4		;  Filter Pin Select
LLWU_FILT2	equ		MBAR_LLWU+$09		; LLWU Pin Filter 2 register (8b)
FILTF		cfbit		LLWU_FILT2,7		;  Filter Detect Flag
FILTE		cffield		LLWU_FILT2,5,2		;  Digital Filter On External Pin
FILTSEL		cffield		LLWU_FILT2,0,4		;  Filter Pin Select
LLWU_RST	equ		MBAR_LLWU+$0A		; LLWU Reset Enable register (8b)
LLRSTE		cfbit		LLWU_RST,1		;  Low-Leakage Mode RESET Enable
RSTFILT		cfbit		LLWU_RST,0		;  Digital Filter On RESET Pin

;----------------------------------------------------------------------------
; System Integration Module

MBAR_SIM	equ		MBAR+$00c0

SIM_SOPT1	equ		MBAR_SIM+$00		; System Options Register 1 (8b)
REGE		cfbit		SIM_SOPT1,7		;  Voltage Regulator enable
SSTB		cfbit		SIM_SOPT1,6		;  Voltage Regulator standby in stop modes
VSTB		cfbit		SIM_SOPT1,5		;  Voltage Regulator standby in run/wait modes
SIM_SOPT2	equ		MBAR_SIM+$01		; System Options Register 2 (8b)
RAMSIZE		cffield		SIM_SOPT2,4,4		;  Size of RAM array
SIM_SOPT3	equ		MBAR_SIM+$02		; System Options Register 3 (8b)
SWE		cfbit		SIM_SOPT3,1		;  Standby write enable
RWE		cfbit		SIM_SOPT3,0		;  Voltage Regulator write enable
SIM_SOPT4	equ		MBAR_SIM+$03		; System Options Register 4 (8b)
STOPE		cfbit		SIM_SOPT4,5		;  Stop mode enable
WAITE		cfbit		SIM_SOPT4,4		;  Wait mode enable
VLLDBGRE	cfbit		SIM_SOPT4,0		;  Very Low Leakage Debug Request upon Wakeup
SIM_SOPT5	equ		MBAR_SIM+$04		; System Options Register 5 (8b)
MECS		cffield		SIM_SOPT5,2,2		;  MTIM16 external clock source select
F1ECS		cfbit		SIM_SOPT5,1		;  FTM1 external clock select
F0ECS		cfbit		SIM_SOPT5,0		;  FTM0 external clock select
SIM_SOPT6	equ		MBAR_SIM+$05		; System Options Register 6 (8b)
MBSL		cffield		SIM_SOPT6,6,2		;  Mini-FlexBus security level
MTBASE1		cffield		SIM_SOPT6,4,2		;  UART1 TX modulation time base select
RX1IN		cfbit		SIM_SOPT6,3		;  UART1 RX input pin selection
MODTX1		cfbit		SIM_SOPT6,2		;  Modulate T
PTF6PAD		cfbit		SIM_SOPT6,1		;  PTF6 double pad strength
PTC5PAD		cfbit		SIM_SOPT6,0		;  PTC5 pad double pad strength
SIM_SOPT7	equ		MBAR_SIM+$06		; System Options Register 7 (8b)
ADTRGS		cfbit		SIM_SOPT7,6		;  ADC hardware trigger source
ACFTM		cfbit		SIM_SOPT7,5		;  CMP output connection to FTM0 Ch0
I2CDR2		cfbit		SIM_SOPT7,4		;  I2C Link for I2C2 and I2C3
I2CDR0		cfbit		SIM_SOPT7,3		;  I2C Link for I2C0 and I2C1
FTM1SYNC	cfbit		SIM_SOPT7,1		;  FTM1 synchronization trigger
FTM0SYNC	cfbit		SIM_SOPT7,0		;  FTM0 synchronization trigger
SIM_COPC	equ		MBAR_SIM+$0A		; COP Control Register (8b)
COPT		cffield		SIM_COPC,2,2		;  COP watchdog timeout
COPCLKS		cfbit		SIM_COPC,1		;  COP watchdog clock select
COPW		cfbit		SIM_COPC,0		;  COP windowed mode
SIM_SRVCOP	equ		MBAR_SIM+$0B		; Service COP Register (8b)
SRVCOP		cffield		SIM_SRVCOP,0,7
SIM_OSC1	equ		MBAR_SIM+$0D		; Oscillator 1 Control Register (8b)
OSC1EN		cfbit		SIM_OSC1,7		;  Oscillator 1 enable
OSC1RANGE	cffield		SIM_OSC1,3,2		;  Frequency range select
OSC1HGO		cfbit		SIM_OSC1,2		;  High gain oscillator select
OSC1EREFS	cfbit		SIM_OSC1,1		;  External reference select
SIM_SDID	equ		MBAR_SIM+$10		; Device Identification High Register (16b)
SIM_SDIDH	equ		MBAR_SIM+$10		; Device Identification High Register (8b)
SIM_SDIDL	equ		MBAR_SIM+$11		; Device Identification Low Register (8b)
REV		cffield		SIM_SDIDH,4,4		;  Device Revision Number
ID		cffield		SIM_SDID,0,11		;  Device identification number
SIM_SCGC1	equ		MBAR_SIM+$12		; Clock Gate Control Register 1 (8b)
I2C3		cfbit		SIM_SCGC1,7		;  I2C3 clock gate control
I2C2		cfbit		SIM_SCGC1,6		;  I2C2 clock gate control
I2C1		cfbit		SIM_SCGC1,5		;  I2C1 clock gate control
I2C0		cfbit		SIM_SCGC1,4		;  I2C0 clock gate control
SPI1		cfbit		SIM_SCGC1,3		;  SPI1 clock gate control
SPI0		cfbit		SIM_SCGC1,2		;  SPI0 clock gate control
UART1		cfbit		SIM_SCGC1,1		;  UART1 clock gate control
UART0		cfbit		SIM_SCGC1,0		;  UART0 clock gate control
SIM_SCGC2	equ		MBAR_SIM+$13		; Clock Gate Control Register 2 (8b)
CMP		cfbit		SIM_SCGC2,6		;  CMP clock gate control
TSI		cfbit		SIM_SCGC2,5		;  TSI clock gate control
VREF		cfbit		SIM_SCGC2,4		;  VREF clock gate control
ADC		cfbit		SIM_SCGC2,1		;  ADC clock gate control
DAC12B		cfbit		SIM_SCGC2,0		;  12-bit DAC clock gate control
SIM_SCGC3	equ		MBAR_SIM+$14		; Clock Gate Control Register 3 (8b)
CRC		cfbit		SIM_SCGC3,7		;  CRC clock gate control
PDB		cfbit		SIM_SCGC3,6		;  PDB clock gate control
CMT		cfbit		SIM_SCGC3,5		;  CMT clock gate control
MTIM		cfbit		SIM_SCGC3,4		;  MTIM clock gate control
FTM1		cfbit		SIM_SCGC3,3		;  FTM1 clock gate control
FTM0		cfbit		SIM_SCGC3,2		;  FTM0 clock gate control
SIM_SCGC4	equ		MBAR_SIM+$15		; Clock Gate Control Register 4 (8b)
FTFL		cfbit		SIM_SCGC4,7		;  FTFL clock gate control
DMA		cfbit		SIM_SCGC4,3		;  DMA clock gate control
IRQ		cfbit		SIM_SCGC4,1		;  IRQ clock gate control
WDOG		cfbit		SIM_SCGC4,0		;  COP clock gate control
SIM_SCGC5	equ		MBAR_SIM+$16		; Clock Gate Control Register 5 (8b)
RNGB		cfbit		SIM_SCGC5,7		;  RNGB clock gate control
MFBUS		cfbit		SIM_SCGC5,6		;  MFBUS clock gate control
OSC2		cfbit		SIM_SCGC5,2		;  OSC2 clock gate control
OSC1		cfbit		SIM_SCGC5,1		;  OSC1 clock gate control
MCG		cfbit		SIM_SCGC5,0		;  MCG clock gate control
SIM_SCGC6	equ		MBAR_SIM+$17		; Clock Gate Control Register 6 (8b)
PORTF		cfbit		SIM_SCGC6,5		;  Port F clock gate control
PORTE		cfbit		SIM_SCGC6,4		;  Port E clock gate control
PORTD		cfbit		SIM_SCGC6,3		;  Port D clock gate control
PORTC		cfbit		SIM_SCGC6,2		;  Port C clock gate control
PORTB		cfbit		SIM_SCGC6,1		;  Port B clock gate control
PORTA		cfbit		SIM_SCGC6,0		;  Port A clock gate control
SIM_CLKOUT	equ		MBAR_SIM+$1A		; Clockout Register (8b)
CS		cffield		SIM_CLKOUT,4,3		;  CLKOUT pin clock select
CLKOUTDIV	cffield		SIM_CLKOUT,0,3		;  Division of the CLKOUT pin
SIM_CLKDIV0	equ		MBAR_SIM+$1B		; Clock Divider 0 Register (8b)
OUTDIV		cffield		SIM_CLKDIV0,0,3		;  Clock output divider value to generate CPU clock
SIM_SPCR	equ		MBAR_SIM+$20		; Flash Configuration Register (8b)
NVMSIZE		cffield		SIM_SPCR,4,4		;  FlexNVM size
PFSIZE		cffield		SIM_SPCR,0,4		;  Program flash size
SIM_UIDH3	equ		MBAR_SIM+$24		; Unique Identification Register (8b)
SIM_UIDH2	equ		MBAR_SIM+$25		; Unique Identification Register (8b)
SIM_UIDH1	equ		MBAR_SIM+$26		; Unique Identification Register (8b)
SIM_UIDH0	equ		MBAR_SIM+$27		; Unique Identification Register (8b)
SIM_UIDMH3	equ		MBAR_SIM+$28		; Unique Identification Register (8b)
SIM_UIDMH2	equ		MBAR_SIM+$29		; Unique Identification Register (8b)
SIM_UIDMH1	equ		MBAR_SIM+$2A		; Unique Identification Register (8b)
SIM_UIDMH0	equ		MBAR_SIM+$2B		; Unique Identification Register (8b)
SIM_UIDML3	equ		MBAR_SIM+$2C		; Unique Identification Register (8b)
SIM_UIDML2	equ		MBAR_SIM+$2D		; Unique Identification Register (8b)
SIM_UIDML1	equ		MBAR_SIM+$2E		; Unique Identification Register (8b)
SIM_UIDML0	equ		MBAR_SIM+$2F		; Unique Identification Register (8b)
SIM_UIDL3	equ		MBAR_SIM+$30		; Unique Identification Register (8b)
SIM_UIDL2	equ		MBAR_SIM+$31		; Unique Identification Register (8b)
SIM_UIDL1	equ		MBAR_SIM+$32		; Unique Identification Register (8b)
SIM_UIDL0	equ		MBAR_SIM+$33		; Unique Identification Register (8b)

;----------------------------------------------------------------------------
; Power Management Controller

MBAR_PCM	equ		MBAR+$0100

PMC_LVDSC1	equ		MBAR_PCM+$00		; Low Voltage Detect Status And Control 1 register (8b)
LVDF		cfbit		PMC_LVDSC1,7		;  Low-Voltage Detect Flag
LVDACK		cfbit		PMC_LVDSC1,6		;  Low-Voltage Detect Acknowledge
LVDIE		cfbit		PMC_LVDSC1,5		;  Low-Voltage Detect Interrupt Enable
LVDRE		cfbit		PMC_LVDSC1,4		;  Low-Voltage Detect Reset Enable
LVDV		cffield		PMC_LVDSC1,0,2		;  Low-Voltage Detect Voltage Select
PMC_LVDSC2	equ		MBAR_PCM+$01		; Low Voltage Detect Status And Control 2 register (8b)
LVWF		cfbit		PMC_LVDSC2,7		;  Low-Voltage Warning Flag
LVWACK		cfbit		PMC_LVDSC2,6		;  Low-Voltage Warning Acknowledge
LVWIE		cfbit		PMC_LVDSC2,5		;  Low-Voltage Warning Interrupt Enable
LVWV		cffield		PMC_LVDSC2,0,2		;  Low-Voltage Warning Voltage Select
PMC_REGSC	equ		MBAR_PCM+$02		; Regulator Status And Control register (8b)
ACKISO		cfbit		PMC_REGSC,3		;  Acknowledge Isolation
REGONS		cfbit		PMC_REGSC,2		;  Regulator In Run Regulation Status
BGBE		cfbit		PMC_REGSC,0		;  Bandgap Buffer Enable

;----------------------------------------------------------------------------
; Reset Control Module

MBAR_RCM	equ		MBAR+$0110

RCM_SRS0	equ		MBAR_RCM+$00		; System Reset Status Register 0 (8b)
POR		cfbit		RCM_SRS0,7		;  Power-On Reset
PIN		cfbit		RCM_SRS0,6		;  External Reset Pin
WDOG		cfbit		RCM_SRS0,5		;  Watchdog
ILOP		cfbit		RCM_SRS0,4		;  Illegal opcode
ILAD		cfbit		RCM_SRS0,3		;  Illegal address
LOC		cfbit		RCM_SRS0,2		;  Loss-of-Clock Reset
LVD		cfbit		RCM_SRS0,1		;  Low-Voltage Detect Reset
WAKEUP		cfbit		RCM_SRS0,0		;  Low Leakage Wakeup Reset
RCM_SRS1	equ		MBAR_RCM+$01		; System Reset Status Register 1 (8b)
SACKERR		cfbit		RCM_SRS1,5		;  Stop Mode Acknowledge Error Reset
EZPT		cfbit		RCM_SRS1,4		;  EzPort Reset
BDFR		cfbit		RCM_SRS1,3		;  Background Debug Force Reset
RCM_RPFC	equ		MBAR_RCM+$04		; Reset Pin Filter Control register (8b)
RSTFLTSS	cfbit		RCM_RPFC,2		;  Reset Pin Filter Select in Stop Mode
RSTFLTSRW	cffield		RCM_RPFC,0,2		;  Reset Pin Filter Select in Run and Wait Modes
RCM_RPFW	equ		MBAR_RCM+$05		; Reset Pin Filter Width register (8b)
RSTFLTSEL	cffield		RCM_RPFW,0,5		;  Reset Pin Filter Bus Clock Select
RCM_MR		equ		MBAR_RCM+$07		; Mode Register (RCM_MR) (8b)
EZP_MS		cfbit		RCM_MR,1		;  EZP_MS_B pin state
MS		cfbit		RCM_MR,0		;  MS_B Pin State

;----------------------------------------------------------------------------
; System Mode Controller

MBAR_SMC	equ		MBAR+$0118

SMC_PMPROT	equ		MBAR_SMC+$00		; Power Mode Protection register (8b)
AVLP		cfbit		SMC_PMPROT,5		;  Allow Very-Low-Power Modes
ALLS		cfbit		SMC_PMPROT,3		;  Allow Low-Leakage Stop Mode
AVLLS		cfbit		SMC_PMPROT,1		;  Allow Very-Low-Leakage Stop Mode
SMC_PMCTRL	equ		MBAR_SMC+$01		; Power Mode Control register (8b)
LPWUI		cfbit		SMC_PMCTRL,7		;  Low-Power Wake Up On Interrupt
RUNM		cffield		SMC_PMCTRL,5,2		;  Run Mode Control
STOPA		cfbit		SMC_PMCTRL,3		;  Stop Aborted
STOPM		cffield		SMC_PMCTRL,0,3		;  Stop Mode Control
SMC_VLLSCTRL	equ		MBAR_SMC+$02		; VLLS Control register (8b)
RAM2PO		cfbit		SMC_VLLSCTRL,4		;  RAM2 Power Option
VLLSM		cffield		SMC_VLLSCTRL,0,3	;  VLLS Mode Control
SMC_PMSTAT	equ		MBAR_SMC+$03		; Power Mode Status register (8b)
PMSTAT		cffield		SMC_PMSTAT,0,7		;  Current power mode

;----------------------------------------------------------------------------
; OSC Control Register

__defosc	macro		NUM,Base
OSC{NUM}_CR	equ		Base+0			; OSC Control Register (8b)
ERCLKEN		cfbit		OSC{NUM}_CR,7		;  External Reference Enable
EREFSTEN	cfbit		OSC{NUM}_CR,5		;  External Reference Stop Enable
SC2P		cfbit		OSC{NUM}_CR,3		;  Oscillator 2 pF Capacitor Load Configure
SC4P		cfbit		OSC{NUM}_CR,2		;  Oscillator 4 pF Capacitor Load Configure
SC8P		cfbit		OSC{NUM}_CR,1		;  Oscillator 8 pF Capacitor Load Configure
SC16P		cfbit		OSC{NUM}_CR,0		;  Oscillator 16 pF Capacitor Load Configure
OSC{NUM}_TSTCTR	equ		Base+1			; OSC Test Control Register (8b)
OSCEN		cfbit		OSC{NUM}_TSTCTR,3	;  OSC Enable in Test Mode
TSTEN		cfbit		OSC{NUM}_TSTCTR,2	;  VLP OSC Test Enable
ALCTST		cfbit		OSC{NUM}_TSTCTR,1	;  Auto Loop Control Test Enable
CMPM		cfbit		OSC{NUM}_TSTCTR,0	;  AC-Coupling Comparator Operation Mode
		endm

		__defosc	"1",MBAR+$0120
		__defosc	"2",MBAR+$0130

;----------------------------------------------------------------------------
; Universal Asynchronous Receiver/Transmitter (with FIFO)

__defuart	macro		NUM,Base
UART{NUM}_BDH	equ		Base+$00		; UART Baud Rate Registers: High (8b)
LBKDIE		cfbit		UART{NUM}_BDH,7		;  LIN Break Detect Interrupt Enable
RXEDGIE		cfbit		UART{NUM}_BDH,6		;  RxD Input Active Edge Interrupt Enable
SBR		cffield		UART{NUM}_BDH,0,5	;  UART Baud Rate Bits
UART{NUM}_BDL	equ		Base+$01		; UART Baud Rate Registers: Low (8b)
SBR		cffield		UART{NUM}_BDL,0,8	;  UART Baud Rate Bits
UART{NUM}_C1	equ		Base+$02		; UART Control Register 1 (8b)
LOOPS		cfbit		UART{NUM}_C1,7		;  Loop Mode Select
UARTSWAI	cfbit		UART{NUM}_C1,6		;  UART Stops in Wait Mode
RSRC		cfbit		UART{NUM}_C1,5		;  Receiver Source Select
M		cfbit		UART{NUM}_C1,4		;  9-bit or 8-bit Mode Select
WAKE		cfbit		UART{NUM}_C1,3		;  Receiver Wakeup Method Select
ILT		cfbit		UART{NUM}_C1,2		;  Idle Line Type Select
PE		cfbit		UART{NUM}_C1,1		;  Parity Enable
PT		cfbit		UART{NUM}_C1,0		;  Parity Type
UART{NUM}_C2	equ		Base+$03		; UART Control Register 2 (8b)
TIE		cfbit		UART{NUM}_C2,7		;  Transmitter Interrupt or DMA Transfer Enable
TCIE		cfbit		UART{NUM}_C2,6		;  Transmission Complete Interrupt Enable
RIE		cfbit		UART{NUM}_C2,5		;  Receiver Full Interrupt or DMA Transfer Enable
ILIE		cfbit		UART{NUM}_C2,4		;  Idle Line Interrupt Enable
TE		cfbit		UART{NUM}_C2,3		;  Transmitter Enable
RE		cfbit		UART{NUM}_C2,2		;  Receiver Enable
RWU		cfbit		UART{NUM}_C2,1		;  Receiver Wakeup Control
SBK		cfbit		UART{NUM}_C2,0		;  Send Break
UART{NUM}_S1	equ		Base+$04		; UART Status Register 1 (8b)
TDRE		cfbit		UART{NUM}_S1,7		;  Transmit Data Register Empty Flag
TC		cfbit		UART{NUM}_S1,6		;  Transmit Complete Flag
RDRF		cfbit		UART{NUM}_S1,5		;  Receive Data Register Full Flag
IDLE		cfbit		UART{NUM}_S1,4		;  Idle Line Flag
OR		cfbit		UART{NUM}_S1,3		;  Receiver Overrun Flag
NF		cfbit		UART{NUM}_S1,2		;  Noise Flag
FE		cfbit		UART{NUM}_S1,1		;  Framing Error Flag
PF		cfbit		UART{NUM}_S1,0		;  Parity Error Flag
UART{NUM}_S2	equ		Base+$05		; UART Status Register 2 (8b)
LBKDIF		cfbit		UART{NUM}_S2,7		;  LIN Break Detect Interrupt Flag
RXEDGIF		cfbit		UART{NUM}_S2,6		;  RxD Pin Active Edge Interrupt Flag
MSBF		cfbit		UART{NUM}_S2,5		;  Most Significant Bit First
RXINV		cfbit		UART{NUM}_S2,4		;  Receive Data Inversion
RWUID		cfbit		UART{NUM}_S2,3		;  Receive Wakeup Idle Detect
BRK13		cfbit		UART{NUM}_S2,2		;  Break Transmit Character Length
LBKDE		cfbit		UART{NUM}_S2,1		;  LIN Break Detection Enable
RAF		cfbit		UART{NUM}_S2,0		;  Receiver Active Flag
UART{NUM}_C3	equ		Base+$06		; UART Control Register 3 (8b)
R8		cfbit		UART{NUM}_C3,7		;  Received Bit 8
T8		cfbit		UART{NUM}_C3,6		;  Transmit Bit 8
TXDIR		cfbit		UART{NUM}_C3,5		;  Transmitter Pin Data Direction in Single-Wire mode
TXINV		cfbit		UART{NUM}_C3,4		;  Transmit Data Inversion
ORIE		cfbit		UART{NUM}_C3,3		;  Overrun Error Interrupt Enable
NEIE		cfbit		UART{NUM}_C3,2		;  Noise Error Interrupt Enable
FEIE		cfbit		UART{NUM}_C3,1		;  Framing Error Interrupt Enable
PEIE		cfbit		UART{NUM}_C3,0		;  Parity Error Interrupt Enable
UART{NUM}_D	equ		Base+$07		; UART Data Register (8b)
RT		cffield		UART{NUM}_D,0,8		;  Data
UART{NUM}_MA1	equ		Base+$08		; UART Match Address Registers 1 (8b)
MA		cffield		UART{NUM}_MA1,0,8	;  Match Address
UART{NUM}_MA2	equ		Base+$09		; UART Match Address Registers 2 (8b)
MA		cffield		UART{NUM}_MA2,0,8	;  Match Address
UART{NUM}_C4	equ		Base+$0A		; UART Control Register 4 (8b)
MAEN1		cfbit		UART{NUM}_C4,7		;  Match Address Mode Enable 1
MAEN2		cfbit		UART{NUM}_C4,6		;  Match Address Mode Enable 2
M10		cfbit		UART{NUM}_C4,5		;  10-bit Mode select
BRFA		cffield		UART{NUM}_C4,0,5	;  Baud Rate Fine Adjust
UART{NUM}_C5	equ		Base+$0B		; UART Control Register 5 (8b)
TDMAS		cfbit		UART{NUM}_C5,7		;  Transmitter DMA Select
RDMAS		cfbit		UART{NUM}_C5,5		;  Receiver Full DMA Select
UART{NUM}_ED	equ		Base+$0C		; UART Extended Data Register (8b)
NOISY		cfbit		UART{NUM}_ED,7		;  The current received dataword contained in D and C3[R8] was received with noise
PARITYE		cfbit		UART{NUM}_ED,6		;  The current received dataword contained in D and C3[R8] was received with a parity error
UART{NUM}_MODEM	equ		Base+$0D		; UART Modem Register (8b)
RXRTSE		cfbit		UART{NUM}_MODEM,3	;  Receiver request-to-send enable
TXRTSPOL	cfbit		UART{NUM}_MODEM,2	;  Transmitter request-to-send polarity
TXRTSE		cfbit		UART{NUM}_MODEM,1	;  Transmitter request-to-send enable
TXCTSE		cfbit		UART{NUM}_MODEM,0	;  Transmitter clear-to-send enable
UART{NUM}_PFIFO	equ		Base+$10		; UART FIFO Parameters (8b)
TXFE		cfbit		UART{NUM}_PFIFO,7	;  Transmit FIFO Enable
TXFIFOSIZE	cffield		UART{NUM}_PFIFO,4,3	;  Transmit FIFO Buffer Depth
RXFE		cfbit		UART{NUM}_PFIFO,3	;  Receive FIFO Enable
RXFIFOSIZE	cffield		UART{NUM}_PFIFO,0,3	;  Receive FIFO Buffer Depth
UART{NUM}_CFIFO	equ		Base+$11		; UART FIFO Control Register (8b)
TXFLUSH		cfbit		UART{NUM}_CFIFO,7	;  Transmit FIFO/Buffer Flush
RXFLUSH		cfbit		UART{NUM}_CFIFO,6	;  Receive FIFO/Buffer Flush
RXOFE		cfbit		UART{NUM}_CFIFO,2	;  Receive FIFO Overflow Interrupt Enable
TXOFE		cfbit		UART{NUM}_CFIFO,1	;  Transmit FIFO Overflow Interrupt Enable
RXUFE		cfbit		UART{NUM}_CFIFO,0	;  Receive FIFO Underflow Interrupt Enable
UART{NUM}_SFIFO	equ		Base+$12		; UART FIFO Status Register (8b)
TXEMPT		cfbit		UART{NUM}_SFIFO,7	;  Transmit Buffer/FIFO Empty
RXEMPT		cfbit		UART{NUM}_SFIFO,6	;  Receive Buffer/FIFO Empty
RXOF		cfbit		UART{NUM}_SFIFO,2	;  Receiver Buffer Overflow Flag
TXOF		cfbit		UART{NUM}_SFIFO,1	;  Transmitter Buffer Overflow Flag
RXUF		cfbit		UART{NUM}_SFIFO,0	;  Receiver Buffer Underflow Flag
UART{NUM}_TWFIFO equ		Base+$13		; UART FIFO Transmit Watermark (8b)
TXWATER		cffield		UART{NUM}_TWFIFO,0,8	;  Transmit Watermark
UART{NUM}_TCFIFO equ		Base+$14		; UART FIFO Transmit Count (8b)
TXCOUNT		cffield		UART{NUM}_TCFIFO,0,8	;  Transmit Counter
UART{NUM}_RWFIFO equ		Base+$15		; UART FIFO Receive Watermark (8b)
RXWATER		cffield		UART{NUM}_RWFIFO,0,8	;  Receive Watermark
UART{NUM}_RCFIFO equ		Base+$16		; UART FIFO Receive Count (8b)
RXCOUNT		cffield		UART{NUM}_RCFIFO,0,8	;  Receive Counter
UART{NUM}_C7816	equ		Base+$18		; UART 7816 Control Register (8b)
ONACK		cfbit		UART{NUM}_C7816,4	;  Generate NACK on Overflow
ANACK		cfbit		UART{NUM}_C7816,3	;  Generate NACK on Error
INIT		cfbit		UART{NUM}_C7816,2	;  Detect Initial Character
TTYPE		cfbit		UART{NUM}_C7816,1	;  Transfer Type
ISO_7816E	cfbit		UART{NUM}_C7816,0	;  ISO-7816 Functionality Enabled
UART{NUM}_IE7816 equ		Base+$19		; UART 7816 Interrupt Enable Register (8b)
WTE		cfbit		UART{NUM}_IE7816,7	;  Wait Timer Interrupt Enable
CWTE		cfbit		UART{NUM}_IE7816,6	;  Character Wait Timer Interrupt Enable
BWTE		cfbit		UART{NUM}_IE7816,5	;  Block Wait Timer Interrupt Enable
INITDE		cfbit		UART{NUM}_IE7816,4	;  Initial Character Detected Interrupt Enable
GTVE		cfbit		UART{NUM}_IE7816,2	;  Guard Timer Violated Interrupt Enable
TXTE		cfbit		UART{NUM}_IE7816,1	;  Transmit Threshold Exceeded Interrupt Enable
RXTE		cfbit		UART{NUM}_IE7816,0	;  Receive Threshold Exceeded Interrupt Enable
UART{NUM}_IS7816 equ		Base+$1A		; UART 7816 Interrupt Status Register (8b)
WT		cfbit		UART{NUM}_IS7816,7	;  Wait Timer Interrupt
CWT		cfbit		UART{NUM}_IS7816,6	;  Character Wait Timer Interrupt
BWT		cfbit		UART{NUM}_IS7816,5	;  Block Wait Timer Interrupt
INITD		cfbit		UART{NUM}_IS7816,4	;  Initial Character Detected Interrupt
GTV		cfbit		UART{NUM}_IS7816,2	;  Guard Timer Violated Interrupt
TXT		cfbit		UART{NUM}_IS7816,1	;  Transmit Threshold Exceeded Interrupt
RXT		cfbit		UART{NUM}_IS7816,0	;  Receive Threshold Exceeded Interrupt
UART{NUM}_WP7816T0 equ		Base+$1B		; UART 7816 Wait Parameter Register (8b)
WI		cffield		UART{NUM}_WP7816T0,0,8	;  Wait Timer Interrupt
UART{NUM}_WP7816T1 equ		Base+$1B		; UART 7816 Wait Parameter Register (8b)
CWI		cffield		UART{NUM}_WP7816T1,4,4	;  Character Wait Time Integer
BWI		cffield		UART{NUM}_WP7816T1,0,4	;  Block Wait Time Integer
UART{NUM}_WN7816 equ		Base+$1C		; UART 7816 Wait N Register (8b)
GTN		cffield		UART{NUM}_WN7816,0,8	;  Guard Band N
UART{NUM}_WF7816 equ		Base+$1D		; UART 7816 Wait FD Register (8b)
GTFD		cffield		UART{NUM}_WF7816,0,8	;  FD Multiplier
UART{NUM}_ET7816 equ		Base+$1E		; UART 7816 Error Threshold Register (8b)
TXTHRESHOLD	cffield		UART{NUM}_ET7816,4,4	;  Transmit NACK Threshold
RXTHRESHOLD	cffield		UART{NUM}_ET7816,0,4	;  Receive NACK Threshold
UART{NUM}_TL7816 equ		Base+$1F		; UART 7816 Transmit Length Register (8b)
TLEN		cffield		UART{NUM}_TL7816,0,8	;  Transmit Length
		endm
		
		__defuart	"0",MBAR+$0140
		__defuart	"1",MBAR+$0160

;----------------------------------------------------------------------------
; Serial Peripheral Interface (with FIFO)

__defspi	macro		NUM,Base
SPI{NUM}_C1	equ		Base+$00		; SPI control register 1 (8b)
SPIE		cfbit		SPI{NUM}_C1,7		;  SPI interrupt enable
SPE		cfbit		SPI{NUM}_C1,6		;  SPI system enable
SPTIE		cfbit		SPI{NUM}_C1,5		;  SPI transmit interrupt enable
MSTR		cfbit		SPI{NUM}_C1,4		;  Master/slave mode select
CPOL		cfbit		SPI{NUM}_C1,3		;  Clock polarity
CPHA		cfbit		SPI{NUM}_C1,2		;  Clock phase
SSOE		cfbit		SPI{NUM}_C1,1		;  Slave select output enable
LSBFE		cfbit		SPI{NUM}_C1,0		;  LSB first (shifter direction
SPI{NUM}_C2	equ		Base+$01		; SPI control register 2 (8b)
SPMIE		cfbit		SPI{NUM}_C2,7		;  SPI match interrupt enable
SPIMODE		cfbit		SPI{NUM}_C2,6		;  SPI 8-bit or 16-bit mode
TXDMAE		cfbit		SPI{NUM}_C2,5		;  Transmit DMA enable
MODFEN		cfbit		SPI{NUM}_C2,4		;  Master mode-fault function enable
BIDIROE		cfbit		SPI{NUM}_C2,3		;  Bidirectional mode output enable
RXDMAE		cfbit		SPI{NUM}_C2,2		;  Receive DMA enable
SPISWAI		cfbit		SPI{NUM}_C2,1		;  SPI stop in wait mode
SPC0		cfbit		SPI{NUM}_C2,0		;  SPI pin control 0
SPI{NUM}_BR	equ		Base+$02		; SPI baud rate register (8b)
SPPR		cffield		SPI{NUM}_BR,4,3		;  SPI baud rate prescale divisor
SPR		cffield		SPI{NUM}_BR,0,4		;  SPI baud rate divisor
SPI{NUM}_S	equ		Base+$03		; SPI status register (8b)
SPRF		cfbit		SPI{NUM}_S,7		;  SPI read buffer full flag
SPMF		cfbit		SPI{NUM}_S,6		;  SPI match flag
SPTEF		cfbit		SPI{NUM}_S,5		;  SPI transmit buffer empty flag
MODF		cfbit		SPI{NUM}_S,4		;  Master mode fault flag
SPI{NUM}_DH	equ		Base+$04		; SPI data register high (8b)
SPI{NUM}_DL	equ		Base+$05		; SPI data register low (8b)
SPI{NUM}_MH	equ		Base+$06		; SPI match register high (8b)
SPI{NUM}_ML	equ		Base+$07		; SPI match register low (8b)
		endm

MBAR_SPI0	equ		MBAR+$01a0
		__defspi	"0",MBAR_SPI0
RNFULLF		cfbit		SPI0_S,3		;  Receive FIFO nearly full flag
TNEAREF		cfbit		SPI0_S,2		;  Transmit FIFO nearly empty flag
TXFULLF		cfbit		SPI0_S,1		;  Transmit FIFO full flag
RFIFOEF		cfbit		SPI0_S,0		;  SPI read FIFO empty flag
SPI0_C3		equ		MBAR_SPI0+$08		; SPI0 control register 3
TNEAREF_MARK	cfbit		SPI0_C3,5		;  Transmit FIFO nearly empty watermark
RNFULLF_MARK	cfbit		SPI0_C3,4		;  Receive FIFO nearly full watermark
INTCLR		cfbit		SPI0_C3,3		;  Interrupt clearing mechanism select
TNEARIEN	cfbit		SPI0_C3,2		;  Transmit FIFO nearly empty interrupt enable
RNFULLIEN	cfbit		SPI0_C3,1		;  Receive FIFO nearly full interrupt enable
FIFOMODE	cfbit		SPI0_C3,0		;  FIFO mode enable
SPI0_CI		equ		MBAR_SPI0+$09		; SPI0 clear interrupt register
TXFERR		cfbit		SPI0_CI,7		;  Transmit FIFO error flag
RXFERR		cfbit		SPI0_CI,6		;  Receive FIFO error flag
TXFOF		cfbit		SPI0_CI,5		;  Transmit FIFO overflow flag
RXFOF		cfbit		SPI0_CI,4		;  Receive FIFO overflow flag
TNEAREFCI	cfbit		SPI0_CI,3		;  Transmit FIFO nearly empty flag clear interrupt
RNFULLFCI	cfbit		SPI0_CI,2		;  Receive FIFO nearly full flag clear interrupt
SPTEFCI		cfbit		SPI0_CI,1		;  Transmit FIFO empty flag clear interrupt
SPRFCI		cfbit		SPI0_CI,0		;  Receive FIFO full flag clear interrupt

;----------------------------------------------------------------------------
; Serial Peripheral Interface (without FIFO)

MBAR_SPI1	equ		MBAR+$01b0
		__defspi	"1",MBAR_SPI1

;----------------------------------------------------------------------------
; Inter-Integrated IC

__defi2c	macro		NUM,Base
I2C{NUM}_A1	equ		Base+$00		; I2C Address Register 1 (8b)
AD		cffield		I2C{NUM}_A1,1,7		;  Address
I2C{NUM}_F	equ		Base+$01		; I2C Frequency Divider register (8b)
MULT		cffield		I2C{NUM}_F,6,2		;  The MULT bits define the multiplier factor mul
ICR		cffield		I2C{NUM}_F,0,6		;  ClockRate
I2C{NUM}_C1	equ		Base+$02		; I2C Control Register 1 (8b)
IICEN		cfbit		I2C{NUM}_C1,7		;  I2C Enable
IICIE		cfbit		I2C{NUM}_C1,6		;  I2C Interrupt Enable
MST		cfbit		I2C{NUM}_C1,5		;  Master Mode Select
TX		cfbit		I2C{NUM}_C1,4		;  Transmit Mode Select
TXAK		cfbit		I2C{NUM}_C1,3		;  Transmit Acknowledge Enable
RSTA		cfbit		I2C{NUM}_C1,2		;  Repeat START
WUEN		cfbit		I2C{NUM}_C1,1		;  Wakeup Enable
DMAEN		cfbit		I2C{NUM}_C1,0		;  DMA Enable
I2C{NUM}_S	equ		Base+$03		; I2C Status register (8b)
TCF		cfbit		I2C{NUM}_S,7		;  Transfer Complete Flag
IAAS		cfbit		I2C{NUM}_S,6		;  Addressed As A Slave
BUSY		cfbit		I2C{NUM}_S,5		;  Bus Busy
ARBL		cfbit		I2C{NUM}_S,4		;  Arbitration Lost
RAM		cfbit		I2C{NUM}_S,3		;  Range Address Match
SRW		cfbit		I2C{NUM}_S,2		;  Slave Read/Write
IICIF		cfbit		I2C{NUM}_S,1		;  Interrupt Flag
RXAK		cfbit		I2C{NUM}_S,0		;  Receive Acknowledge
I2C{NUM}_D	equ		Base+$04		; I2C Data I/O register (8b)
DATA		cffield		I2C{NUM}_D,0,8		;  Data
I2C{NUM}_C2	equ		Base+$05		; I2C Control Register 2 (8b)
GCAEN		cfbit		I2C{NUM}_C2,7		;  General Call Address Enable
ADEXT		cfbit		I2C{NUM}_C2,6		;  Address Extension
HDRS		cfbit		I2C{NUM}_C2,5		;  High Drive Select
SBRC		cfbit		I2C{NUM}_C2,4		;  Slave Baud Rate Control
RMEN		cfbit		I2C{NUM}_C2,3		;  Range Address Matching Enable
AD		cffield		I2C{NUM}_C2,0,3		;  Slave Address
I2C{NUM}_FLT	equ		Base+$06		; I2C Programmable Input Glitch Filter register (8b)
FLT		cffield		I2C{NUM}_FLT,0,4	;  I2C Programmable Filter Factor
I2C{NUM}_RA	equ		Base+$07		; I2C Range Address register (8b)
RAD		cffield		I2C{NUM}_RA,1,7		;  Range Slave Address
I2C{NUM}_SMB	equ		Base+$08		; I2C SMBus Control and Status register (8b)
FACK		cfbit		I2C{NUM}_SMB,7		;  Fast NACK/ACK Enable
ALERTEN		cfbit		I2C{NUM}_SMB,6		;  SMBus Alert Response Address Enable
SIICAEN		cfbit		I2C{NUM}_SMB,5		;  Second I2C Address Enable
TCKSEL		cfbit		I2C{NUM}_SMB,4		;  Timeout Counter Clock Select
SLTF		cfbit		I2C{NUM}_SMB,3		;  SCL Low Timeout Flag
SHTF1		cfbit		I2C{NUM}_SMB,2		;  SCL High Timeout Flag 1
SHTF2		cfbit		I2C{NUM}_SMB,1		;  SCL High Timeout Flag 2
SHTF2IE		cfbit		I2C{NUM}_SMB,0		;  SHTF2 Interrupt Enable
I2C{NUM}_A2	equ		Base+$09		; I2C Address Register 2 (8b)
SAD		cffield		I2C{NUM}_A2,1,7		;  SMBus Address
I2C{NUM}_SLT	equ		Base+$0A
SSLT		cffield		I2C{NUM}_SLT,0,16	; SCL low timeout value that determines the timeout period of SCL low
I2C{NUM}_SLTH	equ		Base+$0A		; I2C SCL Low Timeout Register High (8b)
I2C{NUM}_SLTL	equ		Base+$0B		; I2C SCL Low Timeout Register Low (8b)
		endm

		__defi2c	"0",MBAR+$01c0
		__defi2c	"1",MBAR+$01d0
		__defi2c	"2",MBAR+$01e0
		__defi2c	"3",MBAR+$01f0

;----------------------------------------------------------------------------
; Multipurpose Clock Generator

MBAR_MCG	equ		MBAR+$0400

MCG_C1		equ		MBAR_MCG+$00		; MCG Control 1 Register (8b)
CLKS		cffield		MCG_C1,6,2		;  Clock Source Select
FRDIV		cffield		MCG_C1,3,3		;  FLL External Reference Divider
IREFS		cfbit		MCG_C1,2		;  Internal Reference Select
IRCLKEN		cfbit		MCG_C1,1		;  Internal Reference Clock Enable
IREFSTEN	cfbit		MCG_C1,0		;  Internal Reference Stop Enable
MCG_C2		equ		MBAR_MCG+$01		; MCG Control 2 Register (8b)
RANGE		cffield		MCG_C2,4,2		;  Frequency Range Select
HGO		cfbit		MCG_C2,3		;  High Gain Oscillator Select
EREFS		cfbit		MCG_C2,2		;  External Reference Select
LP		cfbit		MCG_C2,1		;  Low Power Select
IRCS		cfbit		MCG_C2,0		;  Internal Reference Clock Select
MCG_C3		equ		MBAR_MCG+$02		; MCG Control 3 Register (8b)
SCTRIM		cffield		MCG_C3,0,8		;  Slow Internal Reference Clock Trim Setting
MCG_C4		equ		MBAR_MCG+$03		; MCG Control 4 Register (8b)
DMX32		cfbit		MCG_C4,7		;  DCO Maximum Frequency with 32.768 kHz Reference
DRST_DRS	cffield		MCG_C4,5,2		;  DCO Range Select
FCTRIM		cffield		MCG_C4,1,4		;  Fast Internal Reference Clock Trim Setting
SCFTRIM		cfbit		MCG_C4,0		;  Slow Internal Reference Clock Fine Trim
MCG_C5		equ		MBAR_MCG+$04		; MCG Control 5 Register (8b)
PLLCLKEN	cfbit		MCG_C5,6		;  PLL Clock Enable
PLLSTEN		cfbit		MCG_C5,5		;  PLL Stop Enable
PRDIV		cffield		MCG_C5,0,5		;  PLL External Reference Divider
MCG_C6		equ		MBAR_MCG+$05		; MCG Control 6 Register (8b)
LOLIE		cfbit		MCG_C6,7		;  Loss of Lock Interrrupt Enable
PLLS		cfbit		MCG_C6,6		;  PLL Select
CME		cfbit		MCG_C6,5		;  Clock Monitor Enable
VDIV		cffield		MCG_C6,0,5		;  VCO Divider
MCG_S		equ		MBAR_MCG+$06		; MCG Status Register (8b)
LOLS		cfbit		MCG_S,7			;  Loss of Lock Status
LOCK0		cfbit		MCG_S,6			;  Lock Status
PLLST		cfbit		MCG_S,5			;  PLL Select Status
IREFST		cfbit		MCG_S,4			;  Internal Reference Status
CLKST		cffield		MCG_S,2,2		;  Clock Mode Status
OSCINIT		cfbit		MCG_S,1			;  OSC Initialization
IRCST		cfbit		MCG_S,0			;  Internal Reference Clock Status
MCG_ATC		equ		MBAR_MCG+$08		; MCG Auto Trim Control Register (8b)
ATME		cfbit		MCG_ATC,7		;  Automatic Trim Machine Enable
ATMS		cfbit		MCG_ATC,6		;  Automatic Trim Machine Select
ATMF		cfbit		MCG_ATC,5		;  Automatic Trim machine Fail Flag
MCG_ATCVH	equ		MBAR_MCG+$0A		; MCG Auto Trim Compare Value High Register (8b)
ATCVH		cffield		MCG_ATCVH,0,8		;  ATM Compare Value High
MCG_ATCVL	equ		MBAR_MCG+$0B		; MCG Auto Trim Compare Value Low Register (8b)
ATCVL		cffield		MCG_ATCVL,0,8		;  ATM Compare Value Low

;----------------------------------------------------------------------------
; 16-Bit Modulo Timer

MBAR_MTIM16	equ		MBAR+$0410

MTIM0_SC	equ		MBAR_MTIM16+$00		; MTIM16 status and control register (8b)
TOF		cfbit		MTIM0_SC,7		;  MTIM16 overflow flag
TOIE		cfbit		MTIM0_SC,6		;  MTIM16 overflow interrupt enable
TRST		cfbit		MTIM0_SC,5		;  MTIM16 counter reset
TSTP		cfbit		MTIM0_SC,4		;  MTIM16 counter stop
MTIM0_CLK	equ		MBAR_MTIM16+$01		; MTIM16 clock configuration register (8b)
CLKS		cffield		MTIM0_CLK,4,2		;  Clock source select
PS		cffield		MTIM0_CLK,0,4		;  Clock source prescaler
MTIM0_CNTH	equ		MBAR_MTIM16+$02		; MTIM16 counter register high (8b)
CNTH		cffield		MTIM0_CNTH,0,8		;  MTIM16 count (high byte)
MTIM0_CNTL	equ		MBAR_MTIM16+$03		; MTIM16 counter register low (8b)
CNTL		cffield		MTIM0_CNTL,0,8		;  MTIM16 count (low byte)
MTIM0_MODH	equ		MBAR_MTIM16+$04		; MTIM16 modulo register high (8b)
MODH		cffield		MTIM0_MODH,0,8		;  MTIM16 modulo (high byte)
MTIM0_MODL	equ		MBAR_MTIM16+$05		; MTIM16 modulo register low (8b)
MODL		cffield		MTIM0_MODL,0,8		;  MTIM16 modulo (low byte)

;----------------------------------------------------------------------------
; Carrier Modulator Transmitter

MBAR_CMT	equ		MBAR+$0420

CMT_CGH1	equ		MBAR_CMT+$00		; CMT Carrier Generator High Data Register 1 (8b)
PH		cffield		CMT_CGH1,0,8		;  Primary Carrier High Time Data Value
CMT_CGL1	equ		MBAR_CMT+$01		; CMT Carrier Generator Low Data Register 1 (8b)
PL		cffield		CMT_CGL1,0,8		;  Primary Carrier Low Time Data Value
CMT_CGH2	equ		MBAR_CMT+$02		; CMT Carrier Generator High Data Register 2 (8b)
SH		cffield		CMT_CGH2,0,8		;  Secondary Carrier High Time Data Value
CMT_CGL2	equ		MBAR_CMT+$03		; CMT Carrier Generator Low Data Register 2 (8b)
SL		cffield		CMT_CGL2,0,8		;  Secondary Carrier Low Time Data Value
CMT_OC		equ		MBAR_CMT+$04		; CMT Output Control Register (8b)
IROL		cfbit		CMT_OC,7		;  IRO Latch Control
CMTPOL		cfbit		CMT_OC,6		;  CMT Output Polarity
IROPEN		cfbit		CMT_OC,5		;  IRO Pin Enable
CMT_MSC		equ		MBAR_CMT+$05		; CMT Modulator Status and Control Register (8b)
EOCF		cfbit		CMT_MSC,7		;  End Of Cycle Status Flag
CMTDIV		cffield		CMT_MSC,5,2		;  CMT Clock Divide Prescaler
EXSPC		cfbit		CMT_MSC,4		;  Extended Space Enable
BASE		cfbit		CMT_MSC,3		;  Baseband Enable
FSK		cfbit		CMT_MSC,2		;  FSK Mode Select
EOCIE		cfbit		CMT_MSC,1		;  End of Cycle Interrupt Enable
MCGEN		cfbit		CMT_MSC,0		;  Modulator and Carrier Generator Enable
CMT_CMD1	equ		MBAR_CMT+$06		; CMT Modulator Data Register Mark High (8b)
CMT_CMD2	equ		MBAR_CMT+$07		; CMT Modulator Data Register Mark Low (8b)
CMT_CMD3	equ		MBAR_CMT+$08		; CMT Modulator Data Register Space High (8b)
CMT_CMD4	equ		MBAR_CMT+$09		; CMT Modulator Data Register Space Low (8b)
CMT_PPS		equ		MBAR_CMT+$0A		; CMT Primary Prescaler Register (8b)
PPSDIV		cffield		CMT_PPS,0,4		;  Primary Prescaler Divider
CMT_DMA		equ		MBAR_CMT+$0B		; CMT Direct Memory Access Register (8b)
DMA		cfbit		CMT_DMA,0,1		;  DMA Enable

;----------------------------------------------------------------------------
; 2-channel Flex Timer / PWM Module

__deftimer	macro		NUM,Base,NumChans
FTM{NUM}_SC	equ		Base+$00		; Status and Control (8b)
TOF		cfbit		FTM{NUM}_SC,7		;  Timer Overflow Flag
TOIE		cfbit		FTM{NUM}_SC,6		;  Timer Overflow Interrupt Enable
CPWMS		cfbit		FTM{NUM}_SC,5		;  Center-aligned PWM Select
CLKS		cffield		FTM{NUM}_SC,3,2		;  Clock Source Selection
PS		cffield		FTM{NUM}_SC,0,3		;  Prescale Factor Selection
FTM{NUM}_CNTH	equ		Base+$01		; Counter High (8b)
COUNT_H		cffield		FTM{NUM}_CNTH,0,8	;  Counter value high byte
FTM{NUM}_CNTL	equ		Base+$02		; Counter Low (8b)
COUNT_L		cffield		FTM{NUM}_CNTL,0,8	;  Counter value low byte
FTM{NUM}_MODH	equ		Base+$03		; Modulo High (8b)
MOD_H		cffield		FTM{NUM}_MODH,0,8	;  High byte of the modulo value
FTM{NUM}_MODL	equ		Base+$04		; Modulo Low (8b)
MOD_L		cffield		FTM{NUM}_MODL,0,8	; Low byte of the modulo value
__N		set		0
		rept		NumChans
		__decstr	__NS,__N
FTM{NUM}_C{__NS}SC equ		Base+$05+(__N*3)	; Channel Status and Control (8b)
CHF		cfbit		FTM{NUM}_C{__NS}SC,7	;  Channel Flag
CHIE		cfbit		FTM{NUM}_C{__NS}SC,6	;  Channel Interrupt Enable
MSB		cfbit		FTM{NUM}_C{__NS}SC,5	;  Channel Mode Select
MSA		cfbit		FTM{NUM}_C{__NS}SC,4	;  Channel Mode Select
ELSB		cfbit		FTM{NUM}_C{__NS}SC,3	;  Edge or Level Select
ELSA		cfbit		FTM{NUM}_C{__NS}SC,2	;  Edge or Level Select
DMA		cfbit		FTM{NUM}_C{__NS}SC,0	;  DMA Enable
FTM{NUM}_C{__NS}VH equ		Base+$06+(__N*3)	; Channel Value High (8b)
VAL_H		cffield		FTM{NUM}_C{__NS}VH,0,8	;  Channel Value High Byte
FTM{NUM}_C{__NS}VL equ		Base+$07+(__N*3)	; Channel Value Low (8b)
VAL_L		cffield		FTM{NUM}_C{__NS}VL,0,8	;  Channel Value Low Byte
__N		set		__N+1
		endm
FTM{NUM}_CNTINH	equ		Base+$20		; Counter Initial Value High (8b)
INIT_H		cffield		FTM{NUM}_CNTINH,0,8	;  Counter Initial Value High Byte
FTM{NUM}_CNTINL	equ		Base+$21		; Counter Initial Value Low (8)
INIT_L		cffield		FTM{NUM}_CNTINL,0,8	;  Counter Initial Value Low Byte
FTM{NUM}_STATUS	equ		Base+$22		; Capture and Compare Status (8b)
		__enumbits2	FTM{NUM}_STATUS,CH,F,NumChans
FTM{NUM}_MODE	equ		Base+$23		; Features Mode Selection (8b)
FAULTIE		cfbit		FTM{NUM}_MODE,7		;  Fault Interrupt Enable
FAULTM		cffield		FTM{NUM}_MODE,5,2	;  Fault Control Mode
CAPTEST		cfbit		FTM{NUM}_MODE,4		;  Capture Test Mode Enable
PWMSYNC		cfbit		FTM{NUM}_MODE,3		;  PWM Synchronization Mode
WPDIS		cfbit		FTM{NUM}_MODE,2		;  Write Protection Disable
INIT		cfbit		FTM{NUM}_MODE,1		;  Initialize the Output Channels
FTMEN		cfbit		FTM{NUM}_MODE,0		;  FTM Enable
FTM{NUM}_SYNC	equ		Base+$24		; Synchronization (8b)
SWSYNC		cfbit		FTM{NUM}_SYNC,7		;  PWM Synchronization Software Trigger
TRIG2		cfbit		FTM{NUM}_SYNC,6		;  PWM Synchronization External Trigger 2
TRIG1		cfbit		FTM{NUM}_SYNC,5		;  PWM Synchronization External Trigger 1
TRIG0		cfbit		FTM{NUM}_SYNC,4		;  PWM Synchronization External Trigger 0
SYNCHOM		cfbit		FTM{NUM}_SYNC,3		;  Output Mask Synchronization
REINIT		cfbit		FTM{NUM}_SYNC,2		;  FTM Counter Reinitialization by Synchronization
CNTMAX		cfbit		FTM{NUM}_SYNC,1		;  Maximum Boundary Cycle Enable
CNTMIN		cfbit		FTM{NUM}_SYNC,0		;  Minimum Boundary Cycle Enable
FTM{NUM}_OUTINIT equ		Base+$25		; Initial State for Channel Output (8b)
		__enumbits2	FTM{NUM}_OUTINIT,CH,OI,NumChans
FTM{NUM}_OUTMASK equ		Base+$26		; Output Mask (8b)
		__enumbits2	FTM{NUM}_OUTMASK,CH,M,NumChans
FTM{NUM}_COMBINE0 equ		Base+$27		; Function for Linked Channels (8b)
FAULTEN		cfbit		FTM{NUM}_COMBINE0,6	;  Fault Control Enable
SYNCEN		cfbit		FTM{NUM}_COMBINE0,5	;  Synchronization Enable
DTEN		cfbit		FTM{NUM}_COMBINE0,4	;  Deadtime Enable
DECAP		cfbit		FTM{NUM}_COMBINE0,3	;  Dual Edge Capture Mode Captures
DECAPEN		cfbit		FTM{NUM}_COMBINE0,2	;  Dual Edge Capture Mode Enable
COMP		cfbit		FTM{NUM}_COMBINE0,1	;  Complement of Channel (n)
COMBINE		cfbit		FTM{NUM}_COMBINE0,0	;  Combine Channels
FTM{NUM}_DEADTIME equ		Base+$2B		; Deadtime Insertion Control (8b)
DTPS		cffield		FTM{NUM}_DEADTIME,6,2	;  Deadtime Prescaler Value
DTVAL		cffield		FTM{NUM}_DEADTIME,0,6	;  Deadtime Value
FTM{NUM}_EXTTRIG equ		Base+$2C		; External Trigger (8b)
TRIGF		cfbit		FTM{NUM}_EXTTRIG,7	;  Channel Trigger Flag
INITTRIGEN	cfbit		FTM{NUM}_EXTTRIG,6	;  Initialization Trigger Enable
CH1TRIG		cfbit		FTM{NUM}_EXTTRIG,5	;  Channel 1 Trigger Enable
CH0TRIG		cfbit		FTM{NUM}_EXTTRIG,4	;  Channel 0 Trigger Enable
		__enumbits2	FTM{NUM}_EXTTRIG,CH,TRIG,4,2
FTM{NUM}_POL	equ		Base+$2D		; Channels Polarity (8b)
		__enumbits	FTM{NUM}_POL,POL,NumChans ; Channel n Polarity
FTM{NUM}_FMS	equ		Base+$2E		; Fault Mode Status (8b)
FAULTF		cfbit		FTM{NUM}_FMS,7		;  Fault Detection Flag
WPEN		cfbit		FTM{NUM}_FMS,6		;  Write Protection Enable
FAULTIN		cfbit		FTM{NUM}_FMS,5		;  Fault Inputs
FAULTF3		cfbit		FTM{NUM}_FMS,3		;  Fault Detection Flag 3
FAULTF2		cfbit		FTM{NUM}_FMS,2		;  Fault Detection Flag 2
FAULTF1		cfbit		FTM{NUM}_FMS,1		;  Fault Detection Flag 1
FAULTF0		cfbit		FTM{NUM}_FMS,0		;  Fault Detection Flag 0
FTM{NUM}_FILTER0 equ		Base+$2F		; Input Capture Filter Control (8b)
CHoddFVAL	cffield		FTM{NUM}_FILTER0,4,4	;  Input Filter for Odd Channel
CHevenFVAL	cffield		FTM{NUM}_FILTER0,0,4	;  Input Filter for Even Channel
FTM{NUM}_FILTER1 equ		Base+$30		; Input Capture Filter Control (8b)
CHoddFVAL	cffield		FTM{NUM}_FILTER1,4,4	;  Input Filter for Odd Channel
CHevenFVAL	cffield		FTM{NUM}_FILTER1,0,4	;  Input Filter for Even Channel
FTM{NUM}_FLTFILTER equ		Base+$31		; Fault Input Filter Control (8b)
FFVAL		cffield		FTM{NUM}_FLTFILTER,0,4	;  Fault Input Filter
FTM{NUM}_FLTCTRL equ		Base+$32		; Fault Input Control (8b)
FFLTR3EN	cfbit		FTM{NUM}_FLTCTRL,7	;  Fault Input 3 Filter Enable
FFLTR2EN	cfbit		FTM{NUM}_FLTCTRL,6	;  Fault Input 2 Filter Enable
FFLTR1EN	cfbit		FTM{NUM}_FLTCTRL,5	;  Fault Input 1 Filter Enable
FFLTR0EN	cfbit		FTM{NUM}_FLTCTRL,4	;  Fault Input 0 Filter Enable
		__enumbits2	 FTM{NUM}_FLTCTRL,FAULT,EN,4 ; Fault Input n Enable
FTM{NUM}_QDCTRL	equ		Base+$33		; Quadrature Decoder Control and Status (8b)
PHAFLTREN	cfbit		FTM{NUM}_QDCTRL,7	;  Phase A Input Filter Enable
PHBFLTREN	cfbit		FTM{NUM}_QDCTRL,6	;  Phase B Input Filter Enable
PHAPOL		cfbit		FTM{NUM}_QDCTRL,5	;  Phase A Input Polarity
PHBPOL		cfbit		FTM{NUM}_QDCTRL,4	;  Phase B Input Polarity
QUADMODE	cfbit		FTM{NUM}_QDCTRL,3	;  Quadrature Decoder Mode
QUADIR		cfbit		FTM{NUM}_QDCTRL,2	;  FTM Counter Direction in Quadrature Decoder Mode
TOFDIR		cfbit		FTM{NUM}_QDCTRL,1	;  Timer Overflow Direction in Quadrature Decoder Mode
QUADEN		cfbit		FTM{NUM}_QDCTRL,0	;  Quadrature Decoder Mode Enable
		endm

MBAR_FTM0	equ		MBAR+$0440

		__deftimer	"0",MBAR_FTM0,2

;----------------------------------------------------------------------------
; 6-channel Flex Timer / PWM Module

MBAR_FTM1	equ		MBAR+$0480

		__deftimer	"1",MBAR_FTM1,6

;----------------------------------------------------------------------------
; Low Power Timer

__deflptmr	macro		NUM,Base
LPTMR{NUM}_CSR	equ		Base+$00		; Low Power Timer n Control Status Register (32b)
TCF		cfbit		LPTMR{NUM}_CSR,7	;  Timer Compare Flag
TIE		cfbit		LPTMR{NUM}_CSR,6	;  Timer Interrupt Enable
TPS		cffield		LPTMR{NUM}_CSR,4,2	;  Timer Pin Select
TPP		cfbit		LPTMR{NUM}_CSR,3	;  Timer Pin Polarity
TFC		cfbit		LPTMR{NUM}_CSR,2	;  Timer Free-Running Counter
TMS		cfbit		LPTMR{NUM}_CSR,1	;  Timer Mode Select
TEN		cfbit		LPTMR{NUM}_CSR,0	;  Timer Enable
LPTMR{NUM}_PSR	equ		Base+$04		; Low Power Timer n Prescale Register (32b)
PRESCALE	cffield		LPTMR{NUM}_PSR,3,4	;  Prescale Value
PBYP		cfbit		LPTMR{NUM}_PSR,2	;  Prescaler Bypass
PCS		cffield		LPTMR{NUM}_PSR,0,2	;  Prescaler Clock Select
LPTMR{NUM}_CMR	equ		Base+$08		; Low Power Timer n Compare Register (32b)
COMPARE		cffield		LPTMR{NUM}_CMR,0,16	;  Compare Value
LPTMR{NUM}_CNR	equ		Base+$0c		; Low Power Timer n Counter Register (32b)
COUNTER		cffield		LPTMR{NUM}_CNR,0,16	;  Counter Value
		endm

		__deflptmr	"0",MBAR+$04c0
		__deflptmr	"1",MBAR+$04d0

;----------------------------------------------------------------------------
; Flash Controller

MBAR_FTFL	equ		MBAR+$04e0

FTFL_FOPT	equ		MBAR_FTFL+$00		; Flash Option Register (8b)
OPT		cffield		FTFL_FOPT,0,8		;  Nonvolatile Option
FTFL_FSEC	equ		MBAR_FTFL+$01		; Flash Security Register (8b)
KEYEN		cffield		FTFL_FSEC,6,2		;  Backdoor Key Security Enable
MEEN		cffield		FTFL_FSEC,4,2		;  Mass Erase Enable Bits
FSLACC		cffield		FTFL_FSEC,2,2		;  Freescale Failure Analysis Access Code
SEC		cffield		FTFL_FSEC,0,2		;  Flash Security
FTFL_FCNFG	equ		MBAR_FTFL+$02		; Flash Configuration Register (8b)
CCIE		cfbit		FTFL_FCNFG,7		;  Command Complete Interrupt Enable
RDCOLLIE	cfbit		FTFL_FCNFG,6		;  Read Collision Error Interrupt Enable
ERSAREQ		cfbit		FTFL_FCNFG,5		;  Erase All Request
ERSSUSP		cfbit		FTFL_FCNFG,4		;  Erase Suspend
PFLSH		cfbit		FTFL_FCNFG,2		;  Flash memory configuration
RAMRDY		cfbit		FTFL_FCNFG,1		;  RAM Ready
EEERDY		cfbit		FTFL_FCNFG,0		;  indicates if the EEPROM backup data has been copied to the FlexRAM
FTFL_FSTAT	equ		MBAR_FTFL+$03		; Flash Status Register (8b)
CCIF		cfbit		FTFL_FSTAT,7		;  Command Complete Interrupt Flag
RDCOLERR	cfbit		FTFL_FSTAT,6		;  Flash Read Collision Error Flag
ACCERR		cfbit		FTFL_FSTAT,5		;  Flash Access Error Flag
FPVIOL		cfbit		FTFL_FSTAT,4		;  Flash Protection Violation Flag
MGSTAT0		cfbit		FTFL_FSTAT,0		;  Memory Controller Command Completion Status Flag
__N		set		0
		rept		12
		__hexstr	__NS,__N
FTFL_FCCOB{__NS} equ		MBAR_FTFL+$04+__N	; Flash Common Command Object Register n (8b)
CCOB{__NS}	cffield		FTFL_FCCOB{__NS},0,8	;  command code/parameter
__N		set		__N+1
		endm
__N		set		0
		rept		4
		__decstr	__NS,__N
FTFL_FPROT{__NS} equ		MBAR_FTFL+$10		; Program Flash Protection Register n (8b)
PROT		cffield		FTFL_FPROT{__NS},0,8	;  Program Flash Region Protect
__N		set		__N+1
		endm
FTFL_FDPROT	equ		MBAR_FTFL+$14		; Data Flash Protection Register (8b)
DPROT		cffield		FTFL_FDPROT,0,8		;  Data Flash Region Protect
FTFL_FEPROT	equ		MBAR_FTFL+$15		; EEPROM Protection Register (8b)
EPROT		cffield		FTFL_FEPROT,0,8		;  EEPROM Region Protect

;----------------------------------------------------------------------------
; 12-bit Digital-to-Analog Converter

MBAR_DAC0	equ		MBAR+$0500

__N		set		0
		rept		16
		__decstr	__NS,__N
DAC0_DAT{__NS}H	equ		MBAR_DAC0+(__N*2)	; DAC Data High Register n (8b)
DATA1		cffield		DAC0_DAT{__NS}H,0,4
DAC0_DAT{__NS}L	equ		MBAR_DAC0+(__N*2)+1	; DAC Data Low Register n (8b)
DATA0		cffield		DAC0_DAT{__NS}L,0,8
__N		set		__N+1
		endm
DAC0_SR		equ		MBAR_DAC0+$20		; DAC Status Register (8b)
DACBFWMF	cfbit		DAC0_SR,2		;  DAC Buffer Watermark Flag
DACBFRPTF	cfbit		DAC0_SR,1		;  DAC Buffer Read Pointer Top Position Flag
DACBFRPBF	cfbit		DAC0_SR,0		;  DAC Buffer Read Pointer Bottom Position Flag
DAC0_C0		equ		MBAR_DAC0+$21		; DAC Control Register (8b)
DACEN		cfbit		DAC0_C0,7		;  DAC Enable
DACRFS		cfbit		DAC0_C0,6		;  DAC Reference Select
DACTRGSEL	cfbit		DAC0_C0,5		;  DAC Trigger Select
DACSWTRG	cfbit		DAC0_C0,4		;  DAC Software Trigger
LPEN		cfbit		DAC0_C0,3		;  DAC Low Power Control
DACBWIEN	cfbit		DAC0_C0,2		;  DAC Buffer Watermark Interrupt Enable
DACBTIEN	cfbit		DAC0_C0,1		;  DAC Buffer Read Pointer Top Flag Interrupt Enable
DACBBIEN	cfbit		DAC0_C0,0		;  DAC Buffer Read Pointer Bottom Flag Interrupt Enable
DAC0_C1		equ		MBAR_DAC0+$22		; DAC Control Register 1 (8b)
DMAEN		cfbit		DAC0_C1,7		;  DMA Enable Select
DACBFWM		cffield		DAC0_C1,3,2		;  DAC Buffer Watermark Select
DACBFMD		cffield		DAC0_C1,1,2		;  DAC Buffer Work Mode Select
DACBFEN		cfbit		DAC0_C1,0		;  DAC Buffer Enable
DAC0_C2		equ		MBAR_DAC0+$23		; DAC Control Register 2 (8b)
DACBFRP		cffield		DAC0_C2,4,4		;  DAC Buffer Read Pointer
DACBFUP		cffield		DAC0_C2,0,4		;  DAC Buffer Upper Limit

;----------------------------------------------------------------------------
; High Speed Analog Comparator (includes mux control and 6-bit DAC control)

MBAR_CMP0	equ		MBAR+$0530

CMP0_CR0	equ		MBAR_CMP0+$30		; CMP Control Register 0 (8b)
FILTER_CNT	cffield		CMP0_CR0,4,3		;  Filter Sample Count
HYSTCTR		cffield		CMP0_CR0,0,2		;  Comparator hard block hysteresis control
CMP0_CR1	equ		MBAR_CMP0+$31		; CMP Control Register 1 (8b)
SE		cfbit		CMP0_CR1,7		;  Sample Enable
WE		cfbit		CMP0_CR1,6		;  Windowing Enable
PMODE		cfbit		CMP0_CR1,4		;  Power Mode Select
INV		cfbit		CMP0_CR1,3		;  Comparator INVERT
COS		cfbit		CMP0_CR1,2		;  Comparator Output Select
OPE		cfbit		CMP0_CR1,1		;  Comparator Output Pin Enable
EN		cfbit		CMP0_CR1,0		;  Comparator Module Enable
CMP0_FPR	equ		MBAR_CMP0+$32		; CMP Filter Period Register (8b)
FILT_PER	cffield		CMP0_FPR,0,8		;  Filter Sample Period
CMP0_SCR	equ		MBAR_CMP0+$33		; CMP Status and Control Register (8b)
DMAEN		cfbit		CMP0_SCR,6		;  DMA Enable Control
SMELB		cfbit		CMP0_SCR,5		;  Stop Mode Edge/Level Interrupt Control
IER		cfbit		CMP0_SCR,4		;  Comparator Interrupt Enable Rising
IEF		cfbit		CMP0_SCR,3		;  Comparator Interrupt Enable Falling
CFR		cfbit		CMP0_SCR,2		;  Analog Comparator Flag Rising
CFF		cfbit		CMP0_SCR,1		;  Analog Comparator Flag Falling
COUT		cfbit		CMP0_SCR,0		;  Analog Comparator Output
CMP0_DACCR	equ		MBAR_CMP0+$34		; DAC Control Register (8b)
DACEN		cfbit		CMP0_DACCR,7		;  DAC Enable
VRSEL		cfbit		CMP0_DACCR,6		;  Supply Voltage Reference Source Select
VOSEL		cffield		CMP0_DACCR,0,6		;  DAC Output Voltage Select
CMP0_MUXCR	equ		MBAR_CMP0+$35		; MUX Control Register (8b)
PEN		cfbit		CMP0_MUXCR,7		;  PMUX Enable
MEN		cfbit		CMP0_MUXCR,6		;  MMUX Enable
PSEL		cffield		CMP0_MUXCR,3,3		;  Plus Input Mux Control
MSEL		cffield		CMP0_MUXCR,0,3		;  Minus Input Mux Control

;----------------------------------------------------------------------------
; Programmable Delay Block

MBAR_PDB0	equ		MBAR+$0540

PDB0_SC		equ		MBAR_PDB0+$00		; Status and Control Register (32b)
LDMOD		cffield		PDB0_SC,18,2		;  Load Mode Select
PDBEIE		cfbit		PDB0_SC,17		;  PDB Sequence Error Interrupt Enable
SWTRIG		cfbit		PDB0_SC,16		;  Software Trigger
DMAEN		cfbit		PDB0_SC,15		;  DMA Enable
PRESCALER	cffield		PDB0_SC,12,3		;  Prescaler Divider Select
TRGSEL		cffield		PDB0_SC,8,4		;  Trigger Input Source Select
PDBEN		cfbit		PDB0_SC,7		;  PDB Enable
PDBIF		cfbit		PDB0_SC,6		;  PDB Interrupt Flag
PDBIE		cfbit		PDB0_SC,5		;  PDB Interrupt Enable
MULT		cffield		PDB0_SC,2,2		;  Multiplication Factor Select for Prescaler
CONT		cfbit		PDB0_SC,1		;  Continuous Mode Enable
LDOK		cfbit		PDB0_SC,0		;  Load OK
PDB0_MOD	equ		MBAR_PDB0+$04		; Modulus Register (32b)
MOD		cffield		PDB0_MOD,0,16		;  PDB Modulus
PDB0_CNT	equ		MBAR_PDB0+$08		; Counter Register (32b)
CNT		cffield		PDB0_CNT,0,16		;  PDB Counter
PDB0_IDLY	equ		MBAR_PDB0+$0C		; Interrupt Delay Register (32b)
IDLY		cffield		PDB0_IDLY,0,16		;  PDB Interrupt Delay
PDB0_CH0C1	equ		MBAR_PDB0+$10		; Channel n Control Register 1 (32b)
BB		cffield		PDB0_CH0C1,16,8		;  PDB Channel Pre-Trigger Back-to-Back Operation Enable
TOS		cffield		PDB0_CH0C1,8,8		;  PDB Channel Pre-Trigger Output Select
EN		cffield		PDB0_CH0C1,0,8		;  PDB Channel Pre-Trigger Enable
PDB0_CH0S	equ		MBAR_PDB0+$14		; Channel n Status Register (32b)
CF		cffield		PDB0_CH0S,16,8		;  PDB Channel Flags
ERR		cffield		PDB0_CH0S,0,8		;  PDB Channel Sequence Error Flags
PDB0_CH0DLY0	equ		MBAR_PDB0+$18		; Channel n Delay 0 Register (32b)
DLY		cffield		PDB0_CH0DLY0,0,16	;  PDB Channel Delay
PDB0_CH0DLY1	equ		MBAR_PDB0+$1C		; Channel n Delay 1 Register (32b)
DLY		cffield		PDB0_CH0DLY1,0,16	;  PDB Channel Delay
PDB0_DACINTC0	equ		MBAR_PDB0+$20		; DAC Interval Trigger n Control Register (32b)
EXT		cfbit		PDB0_DACINTC0,1		;  DAC External Trigger Input Enable
TOE		cfbit		PDB0_DACINTC0,0		;  DAC Interval Trigger Enable
PDB0_DACINT0	equ		MBAR_PDB0+$24		; DAC Interval n Register (32b)
INT		cffield		PDB0_DACINT0,0,16	;  DAC Interval
PDB0_PO0EN	equ		MBAR_PDB0+$28		; Pulse-Out n Enable Register (32b)
POEN		cffield		PDB0_PO0EN,0,8		;  PDB Pulse-Out Enable
PDB0_PO0DLY	equ		MBAR_PDB0+$2C		; Pulse-Out n Delay Register (32b)
DLY1		cffield		PDB0_PO0DLY,16,16	;  PDB Pulse-Out Delay 1
DLY2		cffield		PDB0_PO0DLY,0,16	;  PDB Pulse-Out Delay 2

;----------------------------------------------------------------------------
; Cyclic Redundancy Check Generator

MBAR_CRC	equ		MBAR+$0570

CRC_CRC		equ		MBAR_CRC+$00		; CRC Data Register (32b)
HU		cffield		CRC_CRC,24,8		;  CRC High Upper Byte
HL		cffield		CRC_CRC,16,8		;  CRC High Lower Byte
LU		cffield		CRC_CRC,8,8		;  CRC Low Upper Byte
LL		cffield		CRC_CRC,0,8		;  CRC Low Lower Byte
CRC_GPOLY	equ		MBAR_CRC+$04		; CRC Polynomial Register (32b)
HIGH		cffield		CRC_GPOLY,16,16		;  High polynominal half-word
LOW		cffield		CRC_GPOLY,0,16		;  Low polynominal half-word
CRC_CTRL	equ		MBAR_CRC+$08		; CRC Control Register (16b)
TOT		cffield		CRC_CTRL,14,2		;  Type of Transpose for Writes
TOTR		cffield		CRC_CTRL,12,2		;  Type of Transpose for Read
FXOR		cfbit		CRC_CTRL,10		;  Complement Read of CRC data register
WAS		cfbit		CRC_CTRL,9		;  Write CRC data register as seed
TCRC		cfbit		CRC_CTRL,8		;  Width of CRC protocol

;----------------------------------------------------------------------------
; Cryptographic Acceleration Unit

; CAU Registers (CAx)
CASR		equ		 $0
CAA		equ		 $1
CA0		equ		 $2
CA1		equ		 $3
CA2		equ		 $4
CA3		equ		 $5
CA4		equ		 $6
CA5		equ		 $7
CA6		equ		 $8
CA7		equ		 $9
CA8		equ		 $A
; CAU Commands
CNOP		equ		 $000
LDR		equ		 $010
STR		equ		 $020
ADR		equ		 $030
RADR		equ		 $040
ADRA		equ		 $050
XOR		equ		 $060
ROTL		equ		 $070
MVRA		equ		 $080
MVAR		equ		 $090
AESS		equ		 $0A0
AESIS		equ		 $0B0
AESC		equ		 $0C0
AESIC		equ		 $0D0
AESR		equ		 $0E0
AESIR		equ		 $0F0
DESR		equ		 $100
DESK		equ		 $110
HASH		equ		 $120
SHS		equ		 $130
MDS		equ		 $140
SHS2		equ		 $150
ILL		equ		 $1F0
; DESR Fields
IP		equ		 $08		; initial permutation
FP		equ		 $04		; final permutation
KSL1		equ		 $00		; key schedule left 1 bit
KSL2		equ		 $01		; key schedule left 2 bits
KSR1		equ		 $02		; key schedule right 1 bit
KSR2		equ		 $03		; key schedule right 2 bits
; DESK Field
DC		equ		 $01		; decrypt key schedule
CP		equ		 $02		; check parity
; HASH Functions Codes
HFF		equ		 $0		; MD5 F() CA1&CA2 | ~CA1&CA3
HFG		equ		 $1		; MD5 G() CA1&CA3 | CA2&~CA3
HFH		equ		 $2		; MD5 H(), SHA Parity() CA1^CA2^CA3
HFI		equ		 $3		; MD5 I() CA2^(CA1|~CA3)
HFC		equ		 $4		; SHA Ch() CA1&CA2 ^ ~CA1&CA3
HFM		equ		 $5		; SHA Maj() CA1&CA2 ^ CA1&CA3 ^ CA2&CA3
HF2C		equ		 $6		; SHA-256 Ch() CA4&CA5 ^ ~CA4&CA6
HF2M		equ		 $7		; SHA-256 Maj() CA0&CA1 ^ CA0&CA2 ^ CA1&CA2
HF2S		equ		 $8		; SHA-256 Sigma 0 ROTR2(CA0)^ROTR13(CA0)^ROTR22(CA0)
HF2T		equ		 $9		; SHA-256 Sigma 1 ROTR6(CA4)^ROTR11(CA4)^ROTR25(CA4)
HF2U		equ		 $A		; SHA-256 sigma 0 ROTR7(CA8)^ROTR18(CA8)^SHR3(CA8)
HF2V		equ		 $B		; SHA-256 sigma 1 ROTR17(CA8)^ROTR19(CA8)^SHR10(CA8)

;----------------------------------------------------------------------------
; Successive Approximation Analog-to-Digital Converter (16-bit)

MBAR_ADC0	equ		MBAR+$0600

ADC0_SC1A	equ		MBAR_ADC0+$00		; ADC Status and Control Registers 1 (32b)
COCO		cfbit		ADC0_SC1A,7		;  Conversion Complete Flag
AIEN		cfbit		ADC0_SC1A,6		;  Interrupt Enable
DIFF		cfbit		ADC0_SC1A,4		;  Differential Mode Enable
ADCH		cffield		ADC0_SC1A,0,5		;  Input channel select
ADC0_SC1B	equ		MBAR_ADC0+$04		; ADC Status and Control Registers 1 (32b)
COCO		cfbit		ADC0_SC1B,7		;  Conversion Complete Flag
AIEN		cfbit		ADC0_SC1B,6		;  Interrupt Enable
DIFF		cfbit		ADC0_SC1B,4		;  Differential Mode Enable
ADCH		cffield		ADC0_SC1B,0,5		;  Input channel select
ADC0_CFG1	equ		MBAR_ADC0+$08		; ADC Configuration Register 1 (32b)
ADLPC		cfbit		ADC0_CFG1,7		;  Low-Power Configuration
ADIV		cffield		ADC0_CFG1,5,2		;  Clock Divide Select
ADLSMP		cfbit		ADC0_CFG1,4		;  Sample time configuration
MODE		cffield		ADC0_CFG1,2,2		;  Conversion mode selection
ADICLK		cffield		ADC0_CFG1,0,2		;  Input Clock Select
ADC0_CFG2	equ		MBAR_ADC0+$0C		; ADC Configuration Register 2 (32b)
ADACKEN		cfbit		ADC0_CFG2,3		;  Asynchronous Clock Output Enable
ADHSC		cfbit		ADC0_CFG2,2		;  High-Speed Configuration
ADLSTS		cffield		ADC0_CFG2,0,2		;  Long Sample Time Select
ADC0_RA		equ		MBAR_ADC0+$10		; ADC Data Result Register (32b)
ADC0_RB		equ		MBAR_ADC0+$14		; ADC Data Result Register (32b)
ADC0_CV1	equ		MBAR_ADC0+$18		; Compare Value Registers (32b)
ADC0_CV2	equ		MBAR_ADC0+$1C		; Compare Value Registers (32b)
ADC0_SC2	equ		MBAR_ADC0+$20		; Status and Control Register 2 (32b)
ADACT		cfbit		ADC0_SC2,7		;  Conversion Active
ADTRG		cfbit		ADC0_SC2,6		;  Conversion Trigger Select
ACFE		cfbit		ADC0_SC2,5		;  Compare Function Enable
ACFGT		cfbit		ADC0_SC2,4		;  Compare Function Greater Than Enable
ACREN		cfbit		ADC0_SC2,3		;  Compare Function Range Enable
DMAEN		cfbit		ADC0_SC2,2		;  DMA Enable
REFSEL		cffield		ADC0_SC2,0,2		;  Voltage Reference Selection
ADC0_SC3	equ		MBAR_ADC0+$24		; Status and Control Register 3 (32b)
CAL		cfbit		ADC0_SC3,7		;  Calibration
CALF		cfbit		ADC0_SC3,6		;  Calibration Failed Flag
ADCO		cfbit		ADC0_SC3,3		;  Continuous Conversion Enable
AVGE		cfbit		ADC0_SC3,2		;  Hardware Average Enable
AVGS		cffield		ADC0_SC3,0,2		;  Hardware Average Select
ADC0_OFS	equ		MBAR_ADC0+$28		; ADC Offset Correction Register (32b)
OFS		cffield		ADC0_OFS,0,16		;  Offset Error Correction Value
ADC0_PG		equ		MBAR_ADC0+$2C		; ADC Plus-Side Gain Register (32b)
PG		cffield		ADC0_PG,0,16		;  Plus-Side Gain
ADC0_MG		equ		MBAR_ADC0+$30		; ADC Minus-Side Gain Register (32b)
MG		cffield		ADC0_MG,0,16		;  Minus-Side Gain
ADC0_CLPD	equ		MBAR_ADC0+$34		; ADC Plus-Side General Calibration Value Register (32b)
CLPD		cffield		ADC0_CLPD,0,6		;  Calibration Value
ADC0_CLPS	equ		MBAR_ADC0+$38		; ADC Plus-Side General Calibration Value Register (32b)
CLPS		cffield		ADC0_CLPS,0,6		;  Calibration Value
ADC0_CLP4	equ		MBAR_ADC0+$3C		; ADC Plus-Side General Calibration Value Register (32b)
CLP4		cffield		ADC0_CLP4,0,10		;  Calibration Value
ADC0_CLP3	equ		MBAR_ADC0+$40		; ADC Plus-Side General Calibration Value Register (32b)
CLP3		cffield		ADC0_CLP3,0,9		;  Calibration Value
ADC0_CLP2	equ		MBAR_ADC0+$44		; ADC Plus-Side General Calibration Value Register (32b)
CLP2		cffield		ADC0_CLP2,0,8		;  Calibration Value
ADC0_CLP1	equ		MBAR_ADC0+$48		; ADC Plus-Side General Calibration Value Register (32b)
CLP1		cffield		ADC0_CLP1,0,7		;  Calibration Value
ADC0_CLP0	equ		MBAR_ADC0+$4C		; ADC Plus-Side General Calibration Value Register (32b)
CLP0		cffield		ADC0_CLP0,0,6		;  Calibration Value
ADC0_CLMD	equ		MBAR_ADC0+$54		; ADC Minus-Side General Calibration Value Register (32b)
CLMD		cffield		ADC0_CLMD,0,6		;  Calibration Value
ADC0_CLMS	equ		MBAR_ADC0+$58		; ADC Minus-Side General Calibration Value Register (32b)
CLMS		cffield		ADC0_CLMS,0,6		;  Calibration Value
ADC0_CLM4	equ		MBAR_ADC0+$5C		; ADC Minus-Side General Calibration Value Register (32b)
CLM4		cffield		ADC0_CLM4,0,10		;  Calibration Value
ADC0_CLM3	equ		MBAR_ADC0+$60		; ADC Minus-Side General Calibration Value Register (32b)
CLM3		cffield		ADC0_CLM3,0,9		;  Calibration Value
ADC0_CLM2	equ		MBAR_ADC0+$64		; ADC Minus-Side General Calibration Value Register (32b)
CLM2		cffield		ADC0_CLM2,0,8		;  Calibration Value
ADC0_CLM1	equ		MBAR_ADC0+$68		; ADC Minus-Side General Calibration Value Register (32b)
CLM1		cffield		ADC0_CLM1,0,7		;  Calibration Value
ADC0_CLM0	equ		MBAR_ADC0+$6C		; ADC Minus-Side General Calibration Value Register (32b)
CLM0		cffield		ADC0_CLM0,0,6		;  Calibration Value

;----------------------------------------------------------------------------
; Voltage Reference

MBAR_VREF	equ		MBAR+$0670

VREF_TRM	equ		MBAR_VREF+$00		; VREF Trim Register (8b)
TRIM		cffield		VREF_TRM,0,6		;  Trim bits
VREF_SC		equ		MBAR_VREF+$01		; VREF Status and Control Register (8b)
VREFEN		cfbit		VREF_SC,7		;  Internal Voltage Reference enable
REGEN		cfbit		VREF_SC,6		;  Regulator enable
ICOMPEN		cfbit		VREF_SC,5		;  Second order curvature compensation enable
VREFST		cfbit		VREF_SC,2		;  Internal Voltage Reference stable
MODE_LV		cffield		VREF_SC,0,2		;  Buffer Mode selection

;----------------------------------------------------------------------------
; Port I/O Control Module

__defpioctl	macro		ID,Base,Filter
PCTL{ID}_PUE	equ		Base+$00		; Port Pulling Enable Register (8b)
		__enumbits	PCTL{ID}_PUE,PTPUE,8	;  Port internal pulling enable bits
PCTL{ID}_PUS	equ		Base+$01		; Port Pullup/Pulldown Select Register (8b)
		__enumbits	PCTL{ID}_PUS,PTPUS,8	;  Port pullup/pulldown select bits
PCTL{ID}_DS	equ		Base+$02		; Port Drive Strength Enable Register (8b)
		__enumbits	PCTL{ID}_DS,PTDS,8	;  Port output drive strength control bits
PCTL{ID}_SRE	equ		Base+$03		; Port Slew Rate Enable Register (8b)
		__enumbits	PCTL{ID}_SRE,PTSRE,8	;  Port output slow rate enable bits
PCTL{ID}_PFE	equ		Base+$04		; Port Passive Filter Enable Register (8b)
		__enumbits	PCTL{ID}_PFE,PTPFE,8,	;  Port passive input filter enable bits
PCTL{ID}_IC	equ		Base+$05		; Port Interrupt Control Register (8b)
PTDMAEN		cfbit		PCTL{ID}_IC,7		;  DMA enable
PTIE		cfbit		PCTL{ID}_IC,1		;  Interrupt enable
PTMOD		cfbit		PCTL{ID}_IC,0		;  Direction mode for pin interrupt
PCTL{ID}_IPE	equ		Base+$06		; Port Interrupt Pin Enable Register (8b)
		__enumbits	PCTL{ID}_IPE,PTIPE,8	;  Interrupt pin enables
PCTL{ID}_IF	equ		Base+$07		; Port Interrupt Flag Register (8b)
		__enumbits	PCTL{ID}_IF,PTIF,8	;  Interrupt flags
PCTL{ID}_IES	equ		Base+$08		; Interrupt Edge Select Register (8b)
		__enumbits	PCTL{ID}_IES,PTEDG,8	;  Edge selects of pin interrupt
		if		Filter
PCTL{ID}_DFE	equ		Base+$09		; Port Digital Filter Enable Register
		__enumbits	PCTL{ID}_DFE,PTDFE,8	;  Digital filter enables
PCTL{ID}_DFC	equ		Base+$0a		; Port Digital Filter Control Register
PTCLKS		cfbit		PCTL{ID}_DFC,7		;  Clock select bit
PTFF		cffield		PCTL{ID}_DFC,0,5	;  Filter factor bits

		endif
		endm

		__defpioctl	"A",MBAR+$1200,0
		__defpioctl	"B",MBAR+$1210,1
		__defpioctl	"C",MBAR+$1220,1
		__defpioctl	"D",MBAR+$1230,0
		__defpioctl	"E",MBAR+$1240,0
		__defpioctl	"F",MBAR+$1250,0

;----------------------------------------------------------------------------
; Touch Sensing Input

MBAR_TSI0	equ		MBAR+$1400

TSI0_GENCS	equ		MBAR_TSI0+$000		; General Control and Status Register (32b)
LPCLKS		cfbit		TSI0_GENCS,28		;  Low Power Mode Clock Source Selection
LPSCNITV	cffield		TSI0_GENCS,24,4		;  TSI Low Power Mode Scan Interval
NSCN		cffield		TSI0_GENCS,19,5		;  Number of Consecutive Scans per Electrode
PS		cffield		TSI0_GENCS,16,3		;  Electrode oscillator prescaler
EOSF		cfbit		TSI0_GENCS,15		;  End of scan flag
OUTRGF		cfbit		TSI0_GENCS,14		;  Out of Range Flag
EXTERF		cfbit		TSI0_GENCS,13		;  External electrode error occurred
OVRF		cfbit		TSI0_GENCS,12		;  Overrun error flag
SCNIP		cfbit		TSI0_GENCS,9		;  Scan-in-progress status
SWTS		cfbit		TSI0_GENCS,8		;  Software trigger start
TSIEN		cfbit		TSI0_GENCS,7		;  TSI module enable
TSIIE		cfbit		TSI0_GENCS,6		;  TSI interrupt enable
ERIE		cfbit		TSI0_GENCS,5		;  TSI error interrupt Enable
ESOR		cfbit		TSI0_GENCS,4		;  End-of-scan or out-of-range interrupt select
STM		cfbit		TSI0_GENCS,1		;  Scan trigger mode
STPE		cfbit		TSI0_GENCS,0		;  TSI stop enable while in low-power modes
TSI0_SCANC	equ		MBAR_TSI0+$004		; SCAN control register (32b)
REFCHRG		cffield		TSI0_SCANC,27,5		;  Reference oscillator charge current select
CAPTRM		cffield		TSI0_SCANC,24,3		;  Internal capacitance trim value
EXTCHRG		cffield		TSI0_SCANC,19,5		;  External oscillator charge current select
DELVOL		cffield		TSI0_SCANC,16,3		;  Delta voltage select applied to analog oscillators
SMOD		cffield		TSI0_SCANC,8,8		;  Scan modulo
AMCLKDIV	cfbit		TSI0_SCANC,5		;  Active mode clock divider
AMCLKS		cffield		TSI0_SCANC,3,2		;  Active mode clock source
AMPSC		cffield		TSI0_SCANC,0,3		;  Active mode prescaler
TSI0_PEN	equ		MBAR_TSI0+$008		; Pin enable register (32b)
LPSP		cffield		TSI0_PEN,16,4		;  Low-power scan pin
		__enumbits	TSI0_PEN,PEN,16		;  TSI pin n enable
TSI0_STATUS	equ		MBAR_TSI0+$00C		; Status Register (32b)
		__enumbits_o	TSI0_STATUS,ERROF,16,16,-16;  TouchSensing Error Flag n
		__enumbits	TSI0_STATUS,ORNGF,16	;  Touch Sensing Electrode Out-of-Range Flag n
__N		set		0
		rept		8
		__decstr	__NS,__N*2+1
TSI0_CNTR{__NS}	equ		MBAR_TSI0+$100+(__N*4)	; Counter Register (32b)
CTN		cffield		TSI0_CNTR{__NS},16,16	;  TouchSensing channel n counter value
CTN1		cffield		TSI0_CNTR{__NS},0,16	;  TouchSensing channel n-1 counter value
__N		set		__N+1
		endm
__N		set		0
		rept		16
		__decstr	__NS,__N
TSI0_THRESHLD{__NS} equ		MBAR_TSI0+$120+(__N*4)	; Channel n threshold register (32b)
LTHH		cffield		TSI0_THRESHLD{__NS},16,16 ; Low threshold value
HTHH		cffield		TSI0_THRESHLD{__NS},0,16 ; High threshold value
__N		set		__N+1
		endm

;----------------------------------------------------------------------------
; Random Number Generator Block

MBAR_RNGB	equ		MBAR+$18c0

RNG_VER		equ		MBAR_RNGB+$00		; RNGB Version ID Register (32b)
TYPE		cffield		RNG_VER,28,4		;  Random number generator type
MAJOR		cffield		RNG_VER,8,8		;  Major version number
MINOR		cffield		RNG_VER,0,8		;  Minor version number
RNG_CMD		equ		MBAR_RNGB+$04		; RNGB Command Register (32b)
SR		cfbit		RNG_CMD,6		;  Software reset
CE		cfbit		RNG_CMD,5		;  Clear error
CI		cfbit		RNG_CMD,4		;  Clear interrupt
GS		cfbit		RNG_CMD,1		;  Generate seed
ST		cfbit		RNG_CMD,0		;  Self test
RNG_CR		equ		MBAR_RNGB+$08		; RNGB Control Register (32b)
MASKERR		cfbit		RNG_CR,6		;  Mask error interrupt
MASKDONE	cfbit		RNG_CR,5		;  Mask done interrupt
AR		cfbit		RNG_CR,4		;  Auto-reseed
FUFMOD		cffield		RNG_CR,0,2		;  FIFO underflow response mode
RNG_SR		equ		MBAR_RNGB+$0C		; RNGB Status Register (32b)
STATPF		cffield		RNG_SR,24,8		;  Statistics test pass fail
ST_PF		cffield		RNG_SR,21,3		;  Self Test Pass Fail
ERR		cfbit		RNG_SR,16		;  Error
FIFO_SIZE	cffield		RNG_SR,12,4		;  FIFO size
FIFO_LVL	cffield		RNG_SR,8,4		;  FIFO level
NSDN		cfbit		RNG_SR,6		;  New seed done
SDN		cfbit		RNG_SR,5		;  Seed done
STDN		cfbit		RNG_SR,4		;  Self test done
RS		cfbit		RNG_SR,3		;  Reseed needed
SLP		cfbit		RNG_SR,2		;  Sleep
BUSY		cfbit		RNG_SR,1		;  Busy
RNG_ESR		equ		MBAR_RNGB+$10		; RNGB Error Status Register (32b)
FUFE		cfbit		RNG_ESR,4		;  FIFO underflow error
SATE		cfbit		RNG_ESR,3		;  Statistical test error
STE		cfbit		RNG_ESR,2		;  Self test error
OSCE		cfbit		RNG_ESR,1		;  Oscillator error
LFE		cfbit		RNG_ESR,0		;  Linear feedback shift register (LFSR) error
RNG_OUT		equ		MBAR_RNGB+$14		; RNGB Output FIFO (32b)
RANDOUT		cffield		RNG_OUT,0,32		;  Random Output

;----------------------------------------------------------------------------
; Direct Memory Access Controller

MBAR_DMA	equ		MBAR+$6400

DMA_REQC	equ		MBAR_DMA+$000		; DMA Request Control Register (32b)
CFSM0		cfbit		DMA_REQC,31		; Clear state machine control 0
DMAC0		cffield		DMA_REQC,24,4		; DMA channel 0 source
CFSM1		cfbit		DMA_REQC,23		; Clear state machine control 1
DMAC1		cffield		DMA_REQC,16,4		; DMA channel 1 source
CFSM2		cfbit		DMA_REQC,15		; Clear state machine control 2
DMAC2		cffield		DMA_REQC,8,4		; DMA channel 2 source
CFSM3		cfbit		DMA_REQC,7		; Clear state machine control 3
DMAC3		cffield		DMA_REQC,0,4		; DMA channel 3 source
__defdma	macro		NUM,Offset
DMA_SAR{NUM}	equ		MBAR_DMA+Offset+$00	; Source Address Register (32b)
DMA_DAR{NUM}	equ		MBAR_DMA+Offset+$04	; Destination Address Register (32b)
DMA_DSR_BCR{NUM} equ		MBAR_DMA+Offset+$08	; DMA Status Register / Byte Count Register (32b)
CE		cfbit		DMA_DSR_BCR{NUM},30	;  Configuration error
BES		cfbit		DMA_DSR_BCR{NUM},29	;  Bus error on source
BED		cfbit		DMA_DSR_BCR{NUM},28	;  Bus error on destination
REQ		cfbit		DMA_DSR_BCR{NUM},26	;  Request
BSY		cfbit		DMA_DSR_BCR{NUM},25	;  Busy
DONE		cfbit		DMA_DSR_BCR{NUM},24	;  Transactions done
BCR		cffield		DMA_DSR_BCR{NUM},0,24	;  number of bytes yet to be transferred for a given block
DMA_DCR{NUM}	equ		MBAR_DMA+Offset+$0c	; DMA Control Register (32b)
EINT		cfbit		DMA_DCR{NUM},31		;  Enable interrupt on completion of transfer
ERQ		cfbit		DMA_DCR{NUM},30		;  Enable peripheral request
CS		cfbit		DMA_DCR{NUM},29		;  Cycle steal
AA		cfbit		DMA_DCR{NUM},28		;  Auto-align
SINC		cfbit		DMA_DCR{NUM},22		;  Source increment
SSIZE		cffield		DMA_DCR{NUM},20,2	;  Source size
DINC		cfbit		DMA_DCR{NUM},19		;  Destination increment
DSIZE		cffield		DMA_DCR{NUM},17,2	;  Destination size
START		cfbit		DMA_DCR{NUM},16		;  Start transfer
SMOD		cffield		DMA_DCR{NUM},12,4	;  Source address modulo
DMOD		cffield		DMA_DCR{NUM},8,4	;  Destination address modulo
D_REQ		cfbit		DMA_DCR{NUM},7		;  Disable request
LINKCC		cffield		DMA_DCR{NUM},4,2	;  Link channel control
LCH1		cffield		DMA_DCR{NUM},2,2	;  Link channel 1
LCH2		cffield		DMA_DCR{NUM},0,2	;  Link channel 2
		endm
		__defdma	"0",$00
		__defdma	"1",$10
		__defdma	"2",$20
		__defdma	"3",$30

;----------------------------------------------------------------------------
; External Memory Interface

MBAR_MB		equ		MBAR+$6800

__defcs		macro		NUM,Base
FB_CSAR{NUM}	equ		Base+0			; Chip Select n Address Register (32b)
BA		cffield		FB_CSAR{NUM},16,16	;  Base Address bits 16..31
FB_CSMR{NUM}	equ		Base+4			; Chip Select n Mask Register (32b)
BAM		cffield		FB_CSMR{NUM},16,16	;  Base Address Mask
WP		cfbit		FB_CSMR{NUM},8		;  Write Protect
V		cfbit		FB_CSMR{NUM},0		;  Valid
FB_CSCR{NUM}	equ		Base+8			; Chip Select n Control Register (32b)
ASET		cffield		FB_CSCR{NUM},20,2	;  Address Setup
RDAH		cffield		FB_CSCR{NUM},18,2	;  Read Address Hold or Deselect
WRAH		cffield		FB_CSCR{NUM},16,2	;  Write Address Hold or Deselect
WS		cffield		FB_CSCR{NUM},10,6	;  Wait states
MUX		cfbit		FB_CSCR{NUM},9		;  Multiplexed Mode
AA		cfbit		FB_CSCR{NUM},8		;  Auto-Acknowledge Enable
PS		cffield		FB_CSCR{NUM},6,2	;  Port size
		endm

;----------------------------------------------------------------------------
; V1 ColdFire Interrupt Controller

MBAR_INTC	equ		MBAR+$7fc0

INTC_IMRH	equ		MBAR_INTC+$08		; Interrupt Mask Register High (32b)
IMR43		cfbit		INTC_IMRH,11		;  Interrupt mask register 43
IMR42		cfbit		INTC_IMRH,10		;  Interrupt mask register 42
IMR41		cfbit		INTC_IMRH,9		;  Interrupt mask register 41
IMR40		cfbit		INTC_IMRH,8		;  Interrupt mask register 40
IMR39		cfbit		INTC_IMRH,7		;  Interrupt mask register 39
IMR38		cfbit		INTC_IMRH,6		;  Interrupt mask register 38
IMR37		cfbit		INTC_IMRH,5		;  Interrupt mask register 37
IMR36		cfbit		INTC_IMRH,4		;  Interrupt mask register 36
IMR35		cfbit		INTC_IMRH,3		;  Interrupt mask register 35
IMR34		cfbit		INTC_IMRH,2		;  Interrupt mask register 34
IMR33		cfbit		INTC_IMRH,1		;  Interrupt mask register 33
IMR32		cfbit		INTC_IMRH,0		;  Interrupt mask register 32
INTC_IMRL	equ		MBAR_INTC+$0C		; Interrupt Mask Register Low (32b)
IMR31		cfbit		INTC_IMRL,31		;  Interrupt mask register 31
IMR30		cfbit		INTC_IMRL,30		;  Interrupt mask register 30
IMR29		cfbit		INTC_IMRL,29		;  Interrupt mask register 29
IMR28		cfbit		INTC_IMRL,28		;  Interrupt mask register 28
IMR27		cfbit		INTC_IMRL,27		;  Interrupt mask register 27
IMR26		cfbit		INTC_IMRL,26		;  Interrupt mask register 26
IMR25		cfbit		INTC_IMRL,25		;  Interrupt mask register 25
IMR24		cfbit		INTC_IMRL,24		;  Interrupt mask register 24
IMR23		cfbit		INTC_IMRL,23		;  Interrupt mask register 23
IMR22		cfbit		INTC_IMRL,22		;  Interrupt mask register 22
IMR21		cfbit		INTC_IMRL,21		;  Interrupt mask register 21
IMR20		cfbit		INTC_IMRL,20		;  Interrupt mask register 20
IMR19		cfbit		INTC_IMRL,19		;  Interrupt mask register 19
IMR18		cfbit		INTC_IMRL,18		;  Interrupt mask register 18
IMR17		cfbit		INTC_IMRL,17		;  Interrupt mask register 17
IMR16		cfbit		INTC_IMRL,16		;  Interrupt mask register 16
IMR15		cfbit		INTC_IMRL,15		;  Interrupt mask register 15
IMR14		cfbit		INTC_IMRL,14		;  Interrupt mask register 14
IMR13		cfbit		INTC_IMRL,13		;  Interrupt mask register 13
IMR12		cfbit		INTC_IMRL,12		;  Interrupt mask register 12
IMR11		cfbit		INTC_IMRL,11		;  Interrupt mask register 11
IMR10		cfbit		INTC_IMRL,10		;  Interrupt mask register 10
IMR9		cfbit		INTC_IMRL,9		;  Interrupt mask register 9
IMR8		cfbit		INTC_IMRL,8		;  Interrupt mask register 8
IMR7		cfbit		INTC_IMRL,7		;  Interrupt mask register 7
IMR6		cfbit		INTC_IMRL,6		;  Interrupt mask register 6
IMR5		cfbit		INTC_IMRL,5		;  Interrupt mask register 5
IMR4		cfbit		INTC_IMRL,4		;  Interrupt mask register 4
IMR3		cfbit		INTC_IMRL,3		;  Interrupt mask register 3
IMR2		cfbit		INTC_IMRL,2		;  Interrupt mask register 2
IMR1		cfbit		INTC_IMRL,1		;  Interrupt mask register 1
IMR0		cfbit		INTC_IMRL,0		;  Interrupt mask register 0
INTC_FRC	equ		MBAR_INTC+$10		; Force Interrupt Register (8b)
LVL1		cfbit		INTC_FRC,6		;  Force level 1 interrupt
LVL2		cfbit		INTC_FRC,5		;  Force level 2 interrupt
LVL3		cfbit		INTC_FRC,4		;  Force level 3 interrupt
LVL4		cfbit		INTC_FRC,3		;  Force level 4 interrupt
LVL5		cfbit		INTC_FRC,2		;  Force level 5 interrupt
LVL6		cfbit		INTC_FRC,1		;  Force level 6 interrupt
LVL7		cfbit		INTC_FRC,0		;  Force level 7 interrupt
INTC_PL6P7	equ		MBAR_INTC+$18		; INTC Programmable Level 6 Priority Registers (8b)
REQN		cffield		INTC_PL6P7,0,6		;  Request number
INTC_PL6P6	equ		MBAR_INTC+$19		; INTC Programmable Level 6 Priority Registers (8b)
REQN		cffield		INTC_PL6P6,0,6		;  Request number
INTC_WCR	equ		MBAR_INTC+$1B		; INTC Wakeup Control Register (8b)
ENB		cfbit		INTC_WCR,7		;  Enable wakeup signal
MASK		cffield		INTC_WCR,0,3		;  Interrupt mask level
INTC_SIMR	equ		MBAR_INTC+$1C		; Set Interrupt Mask Register (8b)
SALL		cfbit		INTC_SIMR,6		;  Set all
SIMR		cffield		INTC_SIMR,0,6		;  Set IMR
INTC_CIMR	equ		MBAR_INTC+$1D		; Clear Interrupt Mask Register (8b)
CALL		cfbit		INTC_CIMR,6		;  Clear all
CIMR		cffield		INTC_CIMR,0,6		;  Clear IMR
INTC_SFRC	equ		MBAR_INTC+$1E		; INTC Set Interrupt Force Register (8b)
SET		cffield		INTC_SFRC,0,6		;  For data values within the 56-62 range, the corresponding bit in the INTC_FRC register is set
INTC_CFRC	equ		MBAR_INTC+$1F		; INTC Clear Interrupt Force Register (8b)
CLR		cffield		INTC_CFRC,0,6		;  For data values within the 56-62 range, the corresponding bit in the INTC_FRC register is cleared
INTC_SWIACK	equ		MBAR_INTC+$20		; INTC Software IACK Register (8b)
VECN		cffield		INTC_SWIACK,0,7		;  Vector number
__N		set		1
		rept		7
		__decstr	__NS,__N
INTC_LVL{__NS}IACK	equ	MBAR_INTC+$20+(__N*4)	; INTC Level-n IACK Registers (8b)
VECN		cffield		INTC_LVL{__NS}IACK,0,7	;  Vector number
__N		set		__N+1
		endm

;----------------------------------------------------------------------------

		restore

		endif				; __mcf51qminc
