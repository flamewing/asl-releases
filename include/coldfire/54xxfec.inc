		ifndef	__54xxfecinc		; avoid multiple inclusion
__54xxfecinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 54XXFEC.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF54xx Fast Ethernet    *
;*   Controller                                                             *
;*                                                                          *
;****************************************************************************

		include		"coldfire/5xxxfec.inc"

__def54fec	macro		PR,Base
		__deffec	PR,Base
		
{PR}MDATA	equ		{PR}MMFR	; Alias?

XFUN		cfbit		{PR}EIR,19	;  Transmit FIFO underrun.
XFERR		cfbit		{PR}EIR,18	;  Transmit FIFO error.
RFERR		cfbit		{PR}EIR,17	;  Receive FIFO error.

XFUN		cfbit		{PR}EIMR,19	;  Transmit FIFO underrun.
XFERR		cfbit		{PR}EIMR,18	;  Transmit FIFO error.
RFERR		cfbit		{PR}EIMR,17	;  Receive FIFO error.
		
{PR}RHR		equ		Base+$88	; Receive Hash Register (32b)
FCE		cfbit		{PR}RHR,31	;  Read only view of the flow control enable (FCE) bit in the RCR.
MULTCAST	cfbit		{PR}RHR,30	;  Set if the current receive frame contained a multi-cast destination address.
HASH		cffield		{PR}RHR,24,6	;  Corresponds to the "hash" value of the current receive frame's destination address.
{PR}RFDR	equ		Base+$184	; FEC Receive FIFO Data Register (32b)
{PR}RFSR	equ		Base+$188	; FEC Receive FIFO Status Register (32b)
IP		cfbit		{PR}RFSR,31	;  Illegal pointer.
FRM		cffield		{PR}RFSR,24,4	;  Frame indicator.
FAE		cfbit		{PR}RFSR,23	;  Frame accept error.
RXW		cfbit		{PR}RFSR,22	;  Receive wait condition.
UF		cfbit		{PR}RFSR,21	;  FIFO underflow.
OF		cfbit		{PR}RFSR,20	;  FIFO Overflow.
FRMRDY		cfbit		{PR}RFSR,19	;  Frame ready.
FU		cfbit		{PR}RFSR,18	;  Full.
ALARM		cfbit		{PR}RFSR,17	;  Alarm.
EMT		cfbit		{PR}RFSR,16	;  Empty.
{PR}RFCR	equ		Base+$18c	;  FEC Receive FIFO Control Register (32b)
TIMER		cfbit		{PR}RFCR,28	;  Timer mode enable.
FRMEN		cfbit		{PR}RFCR,27	;  Frame mode enable.
GR		cffield		{PR}RFCR,24,3	;  Last transfer granularity.
IP_MSK		cfbit		{PR}RFCR,23	;  llegal pointer mask.
FAE_MSK		cfbit		{PR}RFCR,22	;  Frame accept error mask.
RXW_MSK		cfbit		{PR}RFCR,21	;  Receive wait condition mask.
UF_MSK		cfbit		{PR}RFCR,20	;  FIFO underflow mask.
OF_MSK		cfbit		{PR}RFCR,19	;  FIFO overflow mask.
{PR}RLRFP	equ		Base+$190	; FEC Receive FIFO Last Read Frame Pointer Register (32b)
LRFP		cffield		{PR}RLRFP,0,10	;  Last read frame pointer.
{PR}RLWFP	equ		Base+$194	; FEC Receive FIFO Last Write Frame Pointer Register (32b)
LWFP		cffield		{PR}RLWFP,0,10	;  Last write frame pointer.
{PR}RFAR	equ		Base+$198	; FEC Receive FIFO Alarm Register (32b)
ALARM		cffield		{PR}RFAR,0,10	;  Alarm pointer.
{PR}RFRP	equ		Base+$19c	; FEC Receive FIFO Read Pointer Register (32b)
READ		cffield		{PR}RFRP,0,10	;  Read pointer.
{PR}RFWP	equ		Base+$1a0	; FEC Receive FIFO Write Pointer Register (32b)
WRITE		cffield		{PR}RFWP,0,10	;  Write pointer.
{PR}TFDR	equ		Base+$1a4	; FEC Transmit FIFO Data Register (32b)
{PR}TFSR	equ		Base+$1a8	; FEC Transmit FIFO Status Register (32b)
IP		cfbit		{PR}TFSR,31	;  Illegal pointer.
FRM		cffield		{PR}TFSR,24,4	;  Frame indicator.
FAE		cfbit		{PR}TFSR,23	;  Frame accept error.
RXW		cfbit		{PR}TFSR,22	;  Receive wait condition.
UF		cfbit		{PR}TFSR,21	;  FIFO underflow.
OF		cfbit		{PR}TFSR,20	;  FIFO Overflow.
FRMRDY		cfbit		{PR}TFSR,19	;  Frame ready.
FU		cfbit		{PR}TFSR,18	;  Full.
ALARM		cfbit		{PR}TFSR,17	;  Alarm.
EMT		cfbit		{PR}TFSR,16	;  Empty.
{PR}TFCR	equ		Base+$1ac	; FEC Transmit FIFO Control Register (32b)
WCTL		cfbit		{PR}TFCR,30	;  Write control.
WFR		cfbit		{PR}TFCR,29	;  Write frame.
TIMER		cfbit		{PR}TFCR,28	;  Timer mode enable.
FRMEN		cfbit		{PR}TFCR,27	;  Frame mode enable.
GR		cffield		{PR}TFCR,24,3	;  Last transfer granularity.
IP_MSK		cfbit		{PR}TFCR,23	;  llegal pointer mask.
FAE_MSK		cfbit		{PR}TFCR,22	;  Frame accept error mask.
UF_MSK		cfbit		{PR}TFCR,20	;  FIFO underflow mask.
OF_MSK		cfbit		{PR}TFCR,19	;  FIFO overflow mask.
TXW_MSK		cfbit		{PR}TFCR,18	;  Masks the Status Register's TXW bit from generating an error.
{PR}TLRFP	equ		Base+$1b0	; FEC Transmit FIFO Last Read Frame Pointer Register (32b)
LRFP		cffield		{PR}TLRFP,0,10	;  Last read frame pointer.
{PR}TLWFP	equ		Base+$1b4	; FEC Transmit FIFO Last Write Frame Pointer Register (32b)
LWFP		cffield		{PR}TLWFP,0,10	;  Last write frame pointer.
{PR}TFAR	equ		Base+$1b8	; FEC Transmit FIFO Alarm Register (32b)
ALARM		cffield		{PR}TFAR,0,10	;  Alarm pointer.
{PR}TFRP	equ		Base+$1bc	; FEC Transmit FIFO Read Pointer Register (32b)
READ		cffield		{PR}TFRP,0,10	;  Read pointer.
{PR}TFWP	equ		Base+$1c0	; FEC Transmit FIFO Write Pointer Register (32b)
WRITE		cffield		{PR}TFWP,0,10	;  Write pointer.
{PR}FRST	equ		Base+$1c4	; FEC FIFO Reset Register (32b)
SW_RST		cfbit		{PR}FRST,25	;  Software Reset.
RST_CTL		cfbit		{PR}FRST,24	;  Reset control.
{PR}CTCWR	equ		Base+$1c8	; FEC CRC and Transmit Frame Control Word Register (32b)
CRC		cfbit		{PR}CTCWR,25	;  CRC enable.
TFCW		cfbit		{PR}CTCWR,24	;  Transmit frame control word enable.
		endm

		restore				; re-enable listing
		
		endif				; __54xxfecinc
