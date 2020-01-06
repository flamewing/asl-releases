		ifndef	__regh16inc		; avoid multiple inclusion
__regh16inc	equ	1

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File regh16.inc                                              *
;*   								            *
;*   Contins register & bit definitions for Hittachi H16                    *
;*                                                                          *
;****************************************************************************

		save
		listing off		 	; disable listing on this file
__savepc	label	*

		switch	MOMCPUNAME
     		case   	"HD641016"
		elsecase
		fatal  "wrong processor type: only HD641016 supported."
		endcase

		if	MOMPASS=1
		 message "Hitachi H16 SFR Definitions (C) 2019 Alfred Arnold"
		endif

		ifndef	ibrvalue
ibrvalue	set	$fe00
		endif

;----------------------------------------------------------------------------
; Chip Select Controller

__cs		struct
ABR		ds.w	1			; Area base register n
ARR		ds.w	1			; Area range register n
AWCR		ds.w	1			; Area wait control register n
WTOE		bit	#7,AWCR			;  Wait Output Enable
ALV		bit	#6,AWCR			;  Access Level
BWC2		bit	#2,AWCR			;  Bus Wait Cycles
BWC1		bit	#1,AWCR
BWC0		bit	#0,AWCR
__cs		endstruct
		org	ibrvalue+$128
CS0		__cs
CS1		__cs
CS2		__cs
CS3		__cs

;----------------------------------------------------------------------------
; Interrupt Controller

IPR0		equ	ibrvalue+$140		; Interrupt priority register 0
S0P1		bit.w	#13,IPR0		;  ASCI0 priority
S0P0		bit.w	#12,IPR0
S1P1		bit.w	#9,IPR0			;  ASCI1 priority
S1P0		bit.w	#8,IPR0
T1P1		bit.w	#9,IPR0			;  Timer1 priority
T1P0		bit.w	#8,IPR0
IPR1		equ	ibrvalue+$142		; Interrupt priority register 1
T2P1		bit.w	#13,IPR1		;  Timer2 priority
T2P0		bit.w	#12,IPR1
DM0P1		bit.w	#5,IPR1			;  DMAC0 priority
DM0P0		bit.w	#4,IPR1
DM1P1		bit.w	#1,IPR1			;  DMAC1 priority
DM1P0		bit.w	#0,IPR1
IPR2		equ	ibrvalue+$144		; Interrupt priority register 2
DM2P1		bit.w	#13,IPR1		;  DMAC2 priority
DM2P0		bit.w	#12,IPR1
DM3P1		bit.w	#9,IPR1			;  DMAC3 priority
DM3P0		bit.w	#8,IPR1
IR0P1		bit.w	#5,IPR1			;  IRQ0 priority
IR0P0		bit.w	#4,IPR1
IR1P1		bit	#1,IPR1			;  IRQ1 priority
IR1P0		bit	#0,IPR1
ICR		equ	ibrvalue+$146		; Interrupt control register
IMOD		bit.w	7,ICR			;  Interrupt Mode
NMIV		bit.w	6,ICR			;  NMI Vector Select
IRQV0		bit.w	5,ICR			;  IRQ0 Vector Select
IRQV1		bit.w	4,ICR			;  IRQ1 Vector Select
IRQIM0		bit.w	3,ICR			;  /IRQ0 Input Mode
IRQIM1		bit.w	2,ICR			;  /IRQ1 Input Mode

;----------------------------------------------------------------------------
; Peripheral Controller

PCR0		equ	ibrvalue+$14e		; Peripheral control register 0
PTF1		bit.w	#12,PCR0		;  Peripheral Terminal Function 1
PTF0		bit.w	#10,PCR0		;  Peripheral Terminal Function 0

;----------------------------------------------------------------------------
; Asynchronous communication interfaces

__aci		struct	dots
TRB		ds.b	1			; TX/RX buffer register
ST0		ds.b	1			; Status register 0
TXINT		bit	#7,ST0			;  TXINT interrupt
RXINT		bit	#6,ST0			;  RXINT interrupt
TXRDY		bit	#1,ST0			;  TX buffer empty
RXRDY		bit	#0,ST0			;  Receive data
ST1		ds.b	1			; Status register 1
IDL		bit	#6,ST1			;  TX idle
CCTS		bit	#3,ST1			;  Change of /CTS
CDCD		bit	#2,ST1			;  Change Of /DCD
BRKD		bit	#1,ST1			;  Break Start Detect
BRKE		bit	#0,ST1			;  Break End Detect
ST2		ds.b	1			; Status register 2
PMP		bit	#6,ST2			;  Parity/Multiprocessor
PE		bit	#5,ST2			;  Parity Error
FRME		bit	#4,ST2			;  Framing Error
OVRN		bit	#3,ST2			;  Overrun Error
ST3		ds.b	1			; Status register 3
CTS		bit	#3,ST3			;  Clear To Send
DCD		bit	#2,ST3			;  Data Carrier Detect
TXENBL		bit	#1,ST3			;  TX Enable
RXENBL		bit	#0,ST3			;  RX Enable
		ds.b	1
IE0		ds.b	1			; Interrupt enable register 0
TXINTE		bit	#7,IE0			;  TXINT Interrupt Enable
RXINTE		bit	#6,IE0			;  RXINT Interrupt Enable
TXRDYE		bit	#1,IE0			;  TXRDY Interrupt Enable
RXRDYE		bit	#0,IE0			;  RXRDY Interrupt Enable
IE1		ds.b	1			; Interrupt enable register 1
IDLE		bit	#6,IE1			;  IDL Interrupt Enable
CCTSE		bit	#3,IE1			;  CCTS Interrupt Enable
CDCDE		bit	#2,IE1			;  CDCD Interrupt Enable
BRKDE		bit	#1,IE1			;  BRKD Interrupt Enable
BRKEE		bit	#0,IE1			;  BRKE Interrupt Enable
IE2		ds.b	1			; Interrupt enable register 2
PMPE		bit	#6,IE2			;  PMP Interrupt Enable
PEE		bit	#5,IE2			;  PE Interrupt Enable
FRMEE		bit	#4,IE2			;  FRME Interrupt Enable
OVRNE		bit	#3,IE2			;  OVRN Interrupt Enable
		ds.b	1			;
CMD		ds.b	1			; Command register
MD0		ds.b	1			; Mode register 0
PRTCL2		bit	#7,MD0			;  Protocol Mode
PRTCL1		bit	#6,MD0
PRTCL0		bit	#5,MD0
AUTO		bit	#4,MD0			;  Auto Enable
STOP1		bit	#1,MD0			;  Stop Bit length
STOP0		bit	#0,MD0
MD1		ds.b	1			; Mode register 1
BRATE1		bit	#7,MD1			;  Bit Rate
BRATE0		bit	#6,MD1
TXCHR1		bit	#5,MD1			;  TX Character Length
TXCHR0		bit	#4,MD1
RXCHR1		bit	#3,MD1			;  RX Character Length
RXCHR0		bit	#2,MD1
PMPM1		bit	#1,MD1			;  Parity/Multiprocessor Mode
PMPM0		bit	#0,MD1
MD2		ds.b	1			; Mode register 2
CNCT1		bit	#1,MD2			;  Channel Connection
CNCT0		bit	#0,MD2
CTL		ds.b	1			; Control register
BRK		bit	#3,CTL			;  Send Break
RTS		bit	#0,CTL			;  Request To Send
		ds.b	3
TMC		ds.b	1			; Time constant register
RXS		ds.b	1			; RX clock source register
RXCS2		bit	#6,RXS			;  RX Clock Source Select
RXCS1		bit	#5,RXS
RXCS0		bit	#4,RXS
TXS		ds.b	1			; TX clock source register
TXCS2		bit	#6,TXS			;  TX Clock Source Select
TXCS1		bit	#5,TXS
TXCS0		bit	#4,TXS
TXBR3		bit	#3,TXS			;  TX Baud Rate Select
TXBR2		bit	#2,TXS
TXBR1		bit	#1,TXS
TXBR0		bit	#0,TXS
__aci		endstruct
		org	ibrvalue+$158
ACI0		__aci
		org	ibrvalue+$170
ACI1		__aci

		; ACI Commands:

ACI_CMD_TXRESET		equ	$01		; TX reset
ACI_CMD_TXENABLE	equ	$02		; TX enable
ACI_CMD_TXDISABLE	equ	$03		; TX disable
ACI_CMD_MPBIT_ON	equ	$08		; MP bit on
ACI_CMD_TXBUFCLR	equ	$09		; TX buffer clear
ACI_CMD_RXRESET		equ	$11		; RX reset
ACI_CMD_RXENABLE	equ	$12		; RX enable
ACI_CMD_RXDISABLE	equ	$13		; RX disable
ACI_CMD_SEARCHMPBIT	equ	$16		; Search for MP bit
ACI_CMD_CHANRESET	equ	$21		; Channel reset
ACI_CMD_NOP		equ	$00		; No operation

;----------------------------------------------------------------------------
; Timer

__timer		struct	dots
UCR		ds.w	1			; Upcount register
CCRA		ds.w	1			; Count compare register A
CCRB		ds.w	1			; Count compare register B
CNTR		ds.w	1			; Control register
MDS1		bit	#15,CNTR		;  Mode Select
MDS0		bit	#14,CNTR
CSS2		bit	#13,CNTR		;  Clock Source Select
CSS1		bit	#12,CNTR
CSS0		bit	#11,CNTR
TSS1		bit	#10,CNTR		;  Trigger Source Select
TSS0		bit	#9,CNTR
DMA		bit	#8,CNTR			;  Direct Memory Access Request
CIE		bit	#7,CNTR			;  Compare Interrupt Enable
OIE		bit	#6,CNTR			;  Overflow Interrupt Enable
MIE		bit	#5,CNTR			;  Measurement End Interrupt Enable
OLB		bit	#4,CNTR			;  Output Level
OLA		bit	#3,CNTR
UCE		bit	#2,CNTR			;  Upcount Enable
UCRC		bit	#1,CNTR			;  Upcount Register Clear
STP		bit	#0,CNTR			;  Stop Point
STR		ds.w	1			; Status register
CF		bit	#3,STR			;  Compare Flag
OF		bit	#2,STR			;  Overflow Flag
MF		bit	#1,STR			;  Measurement End Flag
OLS		bit	#0,STR			;  Output Level Status
__timer		endstruct
		org	ibrvalue+$18e
TIMER1		__timer
TIMER2		__timer

;----------------------------------------------------------------------------
; DMA Controller

__dmactrl	struct	dots
MADR		ds.l	1			; Memory address register
DADR		ds.l	1			; Device/Next block address register
ETCR		ds.w	1			; Execute transfer count register
BTCR		ds.w	1			; Base transfer count register
CHCRA		ds.w	1			; Channel control register
MRC		bit	#15,CHCRA		;  Memory Address Register Change
MS1		bit	#14,CHCRA		;  Memory Space
MS0		bit	#13,CHCRA
DC2		bit	#12,CHCRA		;  Device Classification
DC1		bit	#11,CHCRA
DC0		bit	#10,CHCRA
DPS		bit	#9,CHCRA		;  Device Port Size
OPS		bit	#8,CHCRA		;  Operand Size
RQS		bit	#7,CHCRA		;  Request Signal Sense
BM		bit	#6,CHCRA		;  Bus Mode
DIR		bit	#5,CHCRA		;  Direction
P		bit	#4,CHCRA		;  Priority
BIE		bit	#3,CHCRA		;  BTF Interrupt Enable
BTF		bit	#2,CHCRA		;  Block Transfer Finished
TIE		bit	#1,CHCRA		;  TF Interrupt Enable
TF		bit	#0,CHCRA		;  Transfer Finished
CHCRB		ds.w	1			; Channel control register B
DRC1		bit	#15,CHCRB		;  Device Address Register Change
DRC0		bit	#14,CHCRB
DS1		bit	#13,CHCRB		;  Device Space
DS0		bit	#12,CHCRB
DBER		bit	#11,CHCRB		;  DMA Bus Error
__dmactrl	endstruct
		org	ibrvalue+$1b0
DMAC0		__dmactrl
DMAC1		__dmactrl
DMAC2		__dmactrl
DMAC3		__dmactrl

OPCR		equ	ibrvalue+$1f0		; Operation control register
WE0		bit.w	#15,OPCR		;  Write Enable Channel 0
DE0		bit.w	#14,OPCR		;  DMA Enable Channel 0
SUC0		bit.w	#13,OPCR		;  Successive Operation Channel 0
WE1		bit.w	#11,OPCR		;  Write Enable Channel 1
DE1		bit.w	#10,OPCR		;  DMA Enable Channel 1
SUC1		bit.w	#9,OPCR			;  Successive Operation Channel 1
WE2		bit.w	#7,OPCR			;  Write Enable Channel 2
DE2		bit.w	#6,OPCR			;  DMA Enable Channel 2
SUC2		bit.w	#5,OPCR			;  Successive Operation Channel 2
WE3		bit.w	#3,OPCR			;  Write Enable Channel 3
DE3		bit.w	#2,OPCR			;  DMA Enable Channel 3
SUC3		bit.w	#1,OPCR			;  Successive Operation Channel 3
DME		bit.w	#0,OPCR			;  DMA Master Enable

;----------------------------------------------------------------------------
; Memory Controller

MCR		equ	ibrvalue+$1f8		; Memory control register
WTIE		bit.w	#15,MCR			;  /WAIT Input Enable
ASWC		bit.w	#14,MCR			;  AS Wait Control
IWC1		bit.w	#13,MCR			;  Interrupt Wait Cycle
IWC0		bit.w	#12,MCR
ROPL		bit.w	#8,MCR			;  Refresh Priority Label
REFE		bit.w	#7,MCR			;  Refresh Enable
RWC1		bit.w	#6,MCR			;  Refresh Wait Cycle
RWC0		bit.w	#5,MCR
RRN1		bit.w	#4,MCR			;  Refresh Request Number
RRN0		bit.w	#3,MCR
CYC2		bit.w	#2,MCR			;  Refresh Cycle
CYC1		bit.w	#1,MCR
CYC0		bit.w	#0,MCR

;----------------------------------------------------------------------------

		org	__savepc
		restore				; re-enable listing

		endif				; __regh16inc
