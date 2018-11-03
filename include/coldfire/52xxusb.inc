		ifndef	__52xxusbinc		; avoid multiple inclusion
__52xxusbinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXUSB.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx USB              *
;*                                                                          *
;****************************************************************************

__defusb	macro		P,Base
{P}ID		equ		Base+$000	; Identification Register (32b)
REVISION	cffield		{P}ID,16,8	;  Revision number of the module.
NID		cffield		{P}ID,8,6	;  Ones-complement version of the ID bit field.
ID		cffield		{P}ID,0,6	;  Configuration number.
{P}HWGENERAL	equ		Base+$004	; General Hardware Parameters (32b)
SM		cffield		{P}HWGENERAL,9,2;  Serial mode.
PHYM		cffield		{P}HWGENERAL,6,3;  PHY Mode.
PHYW		cffield		{P}HWGENERAL,4,2;  PHY width.
{P}HWHOST	equ		Base+$008	; Host Hardware Parameters (32b)
TTPER		cffield		{P}HWHOST,24,8	;  Transaction translator periodic contexts.
TTASY		cffield		{P}HWHOST,16,8	;  Transaction translator contexts.
NPORT		cffield		{P}HWHOST,1,3	;  Indicates number of ports in host mode minus 1.
HC		cfbit		{P}HWHOST,0	;  Indicates module is host capable.
{P}HWDEVICE	equ		Base+$00C	; Device Hardware Parameters (32b)
DEVEP		cffield		{P}HWDEVICE,1,5	;  Device endpoints.
DC		cfbit		{P}HWDEVICE,0	;  Indicates the OTG module is device capable.
{P}HWTXBUF	equ		Base+$010	; TX Buffer Hardware Parameters (32b)
TXLC		cfbit		{P}HWTXBUF,31	;  Transmit local context registers.
TXCHANADD	cffield		{P}HWTXBUF,16,8	;  Transmit channel address.
TXADD		cffield		{P}HWTXBUF,8,8	;  Transmit address.
TXBURST		cffield		{P}HWTXBUF,0,8	;  Transmit burst.
{P}HWRXBUF	equ		Base+$014	; RX Buffer Hardware Parameters (32b)
RXADD		cffield		{P}HWRXBUF,8,8	;  Receive address.
RXBURST		cffield		{P}HWRXBUF,0,8	;  Receive burst.
{P}GPTIMER0LD	equ		Base+$080	; General Purpose Timer 0 Load (32b)
GPTLD		cffield		{P}GPTIMER0LD,0,24	;  Specifies the value to be loaded into the countdown timer on a reset.
{P}GPTIMER0CTL	equ		Base+$084	; General Purpose Timer 0 Control (32b)
RUN		cfbit		{P}GPTIMER0CTL,31	;  Timer run.
RST		cfbit		{P}GPTIMER0CTL,30	;  Timer reset.
MODE		cfbit		{P}GPTIMER0CTL,24	;  Timer mode.
GPTCNT		cffield		{P}GPTIMER0CTL,0,24	;  Timer count.
{P}GPTIMER1LD	equ		Base+$088	; General Purpose Timer 1 Load (32b)
GPTLD		cffield		{P}GPTIMER1LD,0,24	;  Specifies the value to be loaded into the countdown timer on a reset.
{P}GPTIMER1CTL	equ		Base+$08C	; General Purpose Timer 1 Control (32b)
RUN		cfbit		{P}GPTIMER1CTL,31	;  Timer run.
RST		cfbit		{P}GPTIMER1CTL,30	;  Timer reset.
MODE		cfbit		{P}GPTIMER1CTL,24	;  Timer mode.
GPTCNT		cffield		{P}GPTIMER1CTL,0,24;  Timer count.
{P}HCIVERSION	equ		Base+$100	; Host Interface Version Number (16b)
{P}CAPLENGTH	equ		Base+$103	; Capability Register Length (8b)
{P}HCSPARAMS	equ		Base+$104	; Host Structural Parameters (32b)
N_TT		cffield		{P}HCSPARAMS,24,4	;  Number of transaction translators.
N_PTT		cffield		{P}HCSPARAMS,20,4	;  Ports per transaction translator.
PI		cfbit		{P}HCSPARAMS,16	;  Port indicators.
N_CC		cffield		{P}HCSPARAMS,12,4	;  Number of companion controllers.
N_PCC		cffield		{P}HCSPARAMS,8,4	;  Number ports per CC.
PPC		cfbit		{P}HCSPARAMS,4	;  Power port control.
N_PORTS		cffield		{P}HCSPARAMS,0,4	;  Number of ports.
{P}HCCPARAMS	equ		Base+$108	; Host Capability Parameters (32b)
EECP		cffield		{P}HCCPARAMS,8,8	;  EHCI extended capabilities pointer.
IST		cffield		{P}HCCPARAMS,4,4	;  Isochronous scheduling threshold.
ASP		cfbit		{P}HCCPARAMS,2	;  Asynchronous schedule park capability.
PFL		cfbit		{P}HCCPARAMS,1	;  Programmable frame list flag.
ADC		cfbit		{P}HCCPARAMS,0	;  64-bit addressing capability.
{P}DCIVERSION	equ		Base+$122	; Device Interface Version Number (16b)
{P}DCCPARAMS	equ		Base+$124	; Device Capability Parameters (32b)
HC		cfbit		{P}DCCPARAMS,8	;  Host capable.
DC		cfbit		{P}DCCPARAMS,7	;  Device Capable.
DEN		cffield		{P}DCCPARAMS,0,4	;  Device endpoint number.
{P}USBCMD	equ		Base+$140	; USB Command (32b)
ITC		cffield		{P}USBCMD,16,8	;  Interrupt threshold control.
FS2		cfbit		{P}USBCMD,15	;  Frame list size.
ATDTW		cfbit		{P}USBCMD,14	;  Add dTD TripWire.
SUTW		cfbit		{P}USBCMD,13	;  Setup TripWire.
ASPE		cfbit		{P}USBCMD,11	;  Asynchronous schedule park mode enable.
ASP		cffield		{P}USBCMD,8,2	;  Asynchronous schedule park mode count.
IAA		cfbit		{P}USBCMD,6	;  Interrupt on async advance doorbell.
ASE		cfbit		{P}USBCMD,5	;  Asynchronous schedule enable.
PSE		cfbit		{P}USBCMD,4	;  Periodic schedule enable.
FS1		cfbit		{P}USBCMD,3	;  Frame list size.
FS0		cfbit		{P}USBCMD,2	;  Frame list size.
RST		cfbit		{P}USBCMD,1	;  Controller reset.
RS		cfbit		{P}USBCMD,0	;  Run/Stop.
{P}USBSTS	equ		Base+$144	; USB Status (32b)
TI1		cfbit		{P}USBSTS,25	;  General purpose timer 1 interrupt.
TI0		cfbit		{P}USBSTS,24	;  General purpose timer 0 interrupt.
UPI		cfbit		{P}USBSTS,19	;  USB host periodic interrupt.
UAI		cfbit		{P}USBSTS,18	;  USB host asynchronous interrupt.
NAKI		cfbit		{P}USBSTS,16	;  NAK interrupt.
AS		cfbit		{P}USBSTS,15	;  Asynchronous schedule status.
PS		cfbit		{P}USBSTS,14	;  Periodic schedule status.
RCL		cfbit		{P}USBSTS,13	;  Reclamation.
HCH		cfbit		{P}USBSTS,12	;  Host controller halted.
SLI		cfbit		{P}USBSTS,8	;  Device-controller suspend.
SRI		cfbit		{P}USBSTS,7	;  SOF received.
URI		cfbit		{P}USBSTS,6	;  USB reset received.
AAI		cfbit		{P}USBSTS,5	;  Interrupt on async advance.
SEI		cfbit		{P}USBSTS,4	;  System error.
FRI		cfbit		{P}USBSTS,3	;  Frame-list rollover.
PCI		cfbit		{P}USBSTS,2	;  Port change detect.
UEI		cfbit		{P}USBSTS,1	;  USB error interrupt.
UI		cfbit		{P}USBSTS,0	;  USB interrupt (USBINT).
{P}USBINTR	equ		Base+$148	; USB Interrupt Enable (32b)
TIE1		cfbit		{P}USBINTR,25	;  General purpose timer 1 interrupt enable.
TIE0		cfbit		{P}USBINTR,24	;  General purpose timer 0 interrupt enable.
UPIE		cfbit		{P}USBINTR,19	;  USB host periodic interrupt enable.
UAIE		cfbit		{P}USBINTR,18	;  USB host asynchronous interrupt enable.
NAKE		cfbit		{P}USBINTR,16	;  NAK interrupt enable.
SLE		cfbit		{P}USBINTR,8	;  Sleep (DC suspend) enable.
SRE		cfbit		{P}USBINTR,7	;  SOF-received enable.
URE		cfbit		{P}USBINTR,6	;  USB-reset enable.
AAE		cfbit		{P}USBINTR,5	;  Interrupt on async advance enable.
SEE		cfbit		{P}USBINTR,4	;  System error enable.
FRE		cfbit		{P}USBINTR,3	;  Frame list rollover enable.
PCE		cfbit		{P}USBINTR,2	;  Port change detect enable.
UEE		cfbit		{P}USBINTR,1	;  USB error interrupt enable.
UE		cfbit		{P}USBINTR,0	;  USB interrupt enable.
{P}FRINDEX	equ		Base+$14C	; USB Frame Index (32b)
{P}PERIODICLISTBASE equ		Base+$154	; Periodic Frame List Base Address (32b)
PERBASE		cffield		{P}PERIODICLISTBASE,12,20 ; Base Address b12..31.
{P}DEVICEADDR	equ		Base+$154	; Device Address (32b)
USBADR		cffield		{P}DEVICEADDR,25,7	;  Device Address.
{P}ASYNCLISTADDR equ		Base+$158	; Current Asynchronous List Address (32b)
ASYBASE		cffield		{P}ASYNCLISTADDR,5,27 ; Link pointer low (LPL) (b5..31).
{P}EPLISTADDR	equ		Base+$158	; Address at Endpoint List (32b)
EPBASE		cffield		{P}EPLISTADDR,11,21 ; Endpoint list address (b11..31).
{P}TTCTRL	equ		Base+$15C	; Host TT Asynchronous Buffer Control (32b)
TTHA		cffield		{P}TTCTRL,24,7	;  TT Hub Address.
{P}BURSTSIZE	equ		Base+$160	; Master Interface Data Burst Size (32b)
TXPBURST	cffield		{P}BURSTSIZE,8,8	;  Programable TX burst length.
RXPBURST	cffield		{P}BURSTSIZE,0,8	;  Programable RX burst length.
{P}TXFILLTUNING	equ		Base+$164	; Host Transmit FIFO Tuning Control (32b)
TXFIFOTHRES	cffield		{P}TXFILLTUNING,16,6 ; FIFO burst threshold.
TXSCHHEALTH	cffield		{P}TXFILLTUNING,8,5 ; Scheduler health counter.
TXSCHOH		cffield		{P}TXFILLTUNING,0,8 ; Scheduler overhead.
{P}CONFIGFLAG	equ		Base+$180	; Configure Flag Register (32b)
{P}PORTSC1	equ		Base+$184	; Port Status/Control (32b)
PTS		cffield		{P}PORTSC1,30,2	;  Port transceiver select.
PSPD		cffield		{P}PORTSC1,26,2	;  Port speed.
PFSC		cfbit		{P}PORTSC1,24	;  Port force full-speed connect.
PHCD		cfbit		{P}PORTSC1,23	;  PHY low power suspend.
WKOC		cfbit		{P}PORTSC1,22	;  Wake on over-current enable.
WKDS		cfbit		{P}PORTSC1,21	;  Wake on disconnect enable.
WLCN		cfbit		{P}PORTSC1,20	;  Wake on connect enable.
PTC		cffield		{P}PORTSC1,16,4	;  Port test control.
PIC		cffield		{P}PORTSC1,14,2	;  Port indicator control.
PO		cfbit		{P}PORTSC1,13	;  Port owner.
PP		cfbit		{P}PORTSC1,12	;  Port power.
LS		cffield		{P}PORTSC1,10,2	;  Line status.
HSP		cfbit		{P}PORTSC1,9	;  High speed port.
PR		cfbit		{P}PORTSC1,8	;  Port reset.
SUSP		cfbit		{P}PORTSC1,7	;  Suspend
FPR		cfbit		{P}PORTSC1,6	;  Force Port Resume.
OCC		cfbit		{P}PORTSC1,5	;  Over-current change.
OCA		cfbit		{P}PORTSC1,4	;  Over-current active.
PEC		cfbit		{P}PORTSC1,3	;  Port enable/disable change.
PE		cfbit		{P}PORTSC1,2	;  Port enabled/disabled.
CSC		cfbit		{P}PORTSC1,1	;  Connect change status.
CCS		cfbit		{P}PORTSC1,0	;  Current connect status.
{P}OTGSC	equ		Base+$1A4	; On-The-Go Status and Control (32b)
DPIE		cfbit		{P}OTGSC,30	;  Data pulse interrupt enable.
1MSE		cfbit		{P}OTGSC,29	;  1 millisecond timer interrupt enable.
BSEIE		cfbit		{P}OTGSC,28	;  B session end interrupt enable.
BSVIE		cfbit		{P}OTGSC,27	;  B session valid interrupt enable.
ASVIE		cfbit		{P}OTGSC,26	;  A session valid interrupt enable.
AVVIE		cfbit		{P}OTGSC,25	;  A VBUS valid interrupt enable.
IDIE		cfbit		{P}OTGSC,24	;  USB ID interrupt enable.
DPIS		cfbit		{P}OTGSC,22	;  Data pulse interrupt status.
1MSS		cfbit		{P}OTGSC,21	;  1 millisecond timer interrupt status.
BSEIS		cfbit		{P}OTGSC,20	;  B session end interrupt status.
BSVIS		cfbit		{P}OTGSC,19	;  B session valid interrupt status.
ASVIS		cfbit		{P}OTGSC,18	;  A session valid interrupt status.
AVVIS		cfbit		{P}OTGSC,17	;  A VBUS valid interrupt status.
IDIS		cfbit		{P}OTGSC,16	;  USB ID interrupt status.
DPS		cfbit		{P}OTGSC,14	;  Data bus pulsing status.
1MST		cfbit		{P}OTGSC,13	;  1 millisecond timer toggle.
BSE		cfbit		{P}OTGSC,12	;  B session end.
BSV		cfbit		{P}OTGSC,11	;  B Session valid.
ASV		cfbit		{P}OTGSC,10	;  A Session valid.
AVV		cfbit		{P}OTGSC,9	;  A VBus valid.
ID		cfbit		{P}OTGSC,8	;  USB ID.
IDPU		cfbit		{P}OTGSC,5	;  ID Pull-up.
DP		cfbit		{P}OTGSC,4	;  Data pulsing.
OT		cfbit		{P}OTGSC,3	;  OTG Termination.
VC		cfbit		{P}OTGSC,1	;  VBUS charge.
VD		cfbit		{P}OTGSC,0	;  VBUS discharge.
{P}USBMODE	equ		Base+$1A8	; USB Mode Register (32b)
SDIS		cfbit		{P}USBMODE,4	;  Stream disable.
SLOM		cfbit		{P}USBMODE,3	;  Setup lockout mode.
ES		cfbit		{P}USBMODE,2	;  Endian select.
CM		cffield		{P}USBMODE,0,2	;  Controller mode.
{P}EPSETUPSR	equ		Base+$1AC	; Endpoint Setup Status Register (32b)
EPSETUPSTAT	cffield		{P}EPSETUPSR,0,4;  Setup endpoint status.
{P}EPPRIME	equ		Base+$1B0	; Endpoint Initialization (32b)
PETB		cffield		{P}EPPRIME,16,4	;  Prime endpoint transmit buffer.
PERB		cffield		{P}EPPRIME,0,4	;  Prime endpoint receive buffer.
{P}EPFLUSH	equ		Base+$1B4	; Endpoint De-initialize (32b)
FETB		cffield		{P}EPFLUSH,16,4	;  Flush endpoint transmit buffer.
FERB		cffield		{P}EPFLUSH,0,4	;  Flush endpoint receive buffer.
{P}EPSR		equ		Base+$1B8	; Endpoint Status Register (32b)
ETBR		cffield		{P}EPSR,16,4	;  Endpoint transmit buffer ready
ERBR		cffield		{P}EPSR,0,4	;  Endpoint receive buffer ready.
{P}EPCOMPLETE	equ		Base+$1BC	; Endpoint Complete (32b)
ETCE		cffield		{P}EPCOMPLETE,16,4	;  Endpoint transmit complete event.
ERCE		cffield		{P}EPCOMPLETE,4,4	;  Endpoint receive complete event.
{P}EPCR0	equ		Base+$1C0	; Endpoint Control Register 0 (32b)
TXE		cfbit		{P}EPCR0,23	;  TX endpoint enable.
TXT		cffield		{P}EPCR0,18,2	;  TX endpoint type.
TXS		cfbit		{P}EPCR0,16	;  TX endpoint stall.
RXE		cfbit		{P}EPCR0,7	;  RX endpoint enable.
RXT		cffield		{P}EPCR0,2,2	;  RX endpoint type.
RXS		cfbit		{P}EPCR0,0	;  RX endpoint stall.
__N		set		1
		rept		3
		__decstr	__NS,__N
{P}EPCR{"\{__NS}"}	equ	Base+$1C0+(__N*4)	; Endpoint Control Register n (32b)
TXE		cfbit		{P}EPCR{"\{__NS}"},23	;  TX endpoint enable.
TXR		cfbit		{P}EPCR{"\{__NS}"},22	;  TX data toggle reset.
TXI		cfbit		{P}EPCR{"\{__NS}"},21	;  TX data toggle inhibit.
TXT		cffield		{P}EPCR{"\{__NS}"},18,2	;  TX endpoint type.
TXD		cfbit		{P}EPCR{"\{__NS}"},17	;  TX endpoint data source.
TXS		cfbit		{P}EPCR{"\{__NS}"},16	;  TX endpoint stall.
RXE		cfbit		{P}EPCR{"\{__NS}"},7	;  RX endpoint enable.
RXR		cfbit		{P}EPCR{"\{__NS}"},6	;  RX data toggle reset.
RXI		cfbit		{P}EPCR{"\{__NS}"},5	;  RX data toggle inhibit.
RXT		cffield		{P}EPCR{"\{__NS}"},2,2	;  RX endpoint type.
RXD		cfbit		{P}EPCR{"\{__NS}"},1	;  RX endpoint data sink.
RXS		cfbit		{P}EPCR{"\{__NS}"},0	;  RX endpoint stall.
__N		set		__N+4
		endm

		endm

		restore				; re-enable listing

                endif                           ; __52xxusbinc
