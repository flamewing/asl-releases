		ifndef	__54xxusb2inc		; avoid multiple inclusion
__54xxusb2inc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 54XXUSB2.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF54xx USB 2.0          *
;*                                                                          *
;****************************************************************************

USBAISR		equ		MBAR_USB2+$000	; Application interrupt status register (8b)
EPSTALL		cfbit		USBAISR,7	;  Endpoint stall.
CTROVFL		cfbit		USBAISR,6	;  Counter overflow.
ACK		cfbit		USBAISR,5	;  Received acknowledge.
TRANSERR	cfbit		USBAISR,4	;  Transaction error.
EPHALT		cfbit		USBAISR,3	;  Endpoint halt.
OUT		cfbit		USBAISR,2	;  Received OUT.
IN		cfbit		USBAISR,1	;  Received IN.
SETUP		cfbit		USBAISR,0	;  Received SETUP.
USBAIMR		equ		MBAR_USB2+$001	; Application interrupt mask register (8b)
EPSTALLEN	cfbit		USBAISR,7	;  Endpoint stall interrupt enable.
CTROVFLEN	cfbit		USBAISR,6	;  Counter overflow interrupt enable.
ACKEN		cfbit		USBAISR,5	;  Received acknowledge interrupt enable.
TRANSERREN	cfbit		USBAISR,4	;  Transaction error interrupt enable.
EPHALTEN	cfbit		USBAISR,3	;  Endpoint halt interrupt enable.
OUTEN		cfbit		USBAISR,2	;  Received OUT interrupt enable.
INEN		cfbit		USBAISR,1	;  Received IN interrupt enable.
SETUPEN		cfbit		USBAISR,0	;  Received SETUP interrupt enable.
EPINFO		equ		MBAR_USB2+$003	; Endpoint info register (8b)
EPNUM		cffield		EPINFO,1,3	;  Endpoint number.
EPDIR		cfbit		EPINFO,0	;  Endpoint direction.
CFGR		equ		MBAR_USB2+$004	; Configuration value register (8b)
CFGAR		equ		MBAR_USB2+$005	; Configuration attribute register (8b)
RMTWKEUP	cfbit		CFGAR,6		;  Remote wakeup.
SPEEDR		equ		MBAR_USB2+$006	; Device speed register (8b)
SPEED		cffield		SPEEDR,0,2	;  Device speed.
FRMNUMR		equ		MBAR_USB2+$00e	; USB frame number register (16b)
FRMNUM		cffield		FRMNUMR,0,12	;  Frame Number.
EPTNR		equ		MBAR_USB2+$010	; Endpoint transaction number register (16b)
EP6T		cffield		EPTNR,10,2	;  Endpoint n transactions.
EP5T		cffield		EPTNR,8,2
EP4T		cffield		EPTNR,6,2
EP3T		cffield		EPTNR,4,2
EP2T		cffield		EPTNR,2,2
EP1T		cffield		EPTNR,0,2
IFUR		equ		MBAR_USB2+$014	; Application interface update register (16b)
IFNUM		cffield		IFUR,8,8	;  Interface number.
ALTSET		cffield		IFUR,0,8	;  Alternate setting.
__N		set		0
		rept		32
		 __decstr	__NS,__N
IFR{__NS}	 equ		MBAR_USB2+$040+(__N*2)	; Configuration interface register n (16b)
IFNUM		 cffield	IFR{__NS},8,8	;  Interface number.
ALTSET		 cffield	IFR{__NS},0,8	;  Alternate setting.
__N		 set		__n+1
		endm
PPCNT		equ		MBAR_USB2+$080	; USB packet passed count register (12b)
DPCNT		equ		MBAR_USB2+$082	; USB dropped packet counter register (16b)
CRCECNT		equ		MBAR_USB2+$084	; USB CCR error counter register (16b)
BSECNT		equ		MBAR_USB2+$086	; USB bitstuffing error counter register (16b)
PIDECNT		equ		MBAR_USB2+$088	; USB PID error counter register (16b)
FRMECNT		equ		MBAR_USB2+$08a	; USB framing error counter register (16b)
TXPCNT		equ		MBAR_USB2+$08c	; USB transmitted packet counter register (16b)
CNTOVR		equ		MBAR_USB2+$08e	; USB counter overflow register (8b)
TXPCNT		cfbit		CNTOVR,6	;  Transmitted packet counter overflow flag.
FRMECNT		cfbit		CNTOVR,5	;  Framing error counter overflow flag.
PIDECNT		cfbit		CNTOVR,4	;  PID error counter overflow flag.
BSECNT		cfbit		CNTOVR,3	;  Bitstuffing error counter overflow flag.
CRCECNT		cfbit		CNTOVR,2	;  CRC error counter overflow flag.
DPCNT		cfbit		CNTOVR,1	;  Dropped packet counter overflow flag.
PPCNT		cfbit		CNTOVR,0	;  Packet passed counter overflow flag.
EP0ACR		equ		MBAR_USB2+$101	; EP0 attribute control register (8b)
TTYPE		cffield		EP0ACR,0,2	;  Transfer type.
EP0MPSR		equ		MBAR_USB2+$102	; EP0 max packet size register (16b)
ADDTRANS	cffield		EP0MPSR,11,2	;  Additional transactions.
MAXPKTSZ	cffield		EP0MPSR,0,11	;  Maximum packet size.
EP0IFR		equ		MBAR_USB2+$104	; EP0 interface number register (8b)
IFNUM		cffield		EP0IFR,0,7	;  Interface number.
EP0SR		equ		MBAR_USB2+$105	; EP0 status register (8b)
INT		cfbit		EP0SR,7		;  Interrupt.
TXZERO		cfbit		EP0SR,5		;  Transmit a zero byte packet.
CCOMP		cfbit		EP0SR,3		;  Control command complete.
PSTALL		cfbit		EP0SR,2		;  Protocol stall.
ACTIVE		cfbit		EP0SR,1		;  Active.
HALT		cfbit		EP0SR,0		;  Halt.
BMRTR		equ		MBAR_USB2+$106	; bmRequest type register (8b)
DIR		cfbit		BMRTR,7		;  Direction.
TYPE		cffield		BMRTR,2,2	;  TYPE
REC		cffield		BMRTR,0,5	;  Recipient.
BRTR		equ		MBAR_USB2+$107	; bRequest type register (8b)
BREQ		cffield		BRTR,0,8	;  bRequest field.
WVALUER		equ		MBAR_USB2+$108	; wValue register (16b)
WVALUE		cffield		WVALUER,0,16	;  wValue of setup transaction.
WINDEXR		equ		MBAR_USB2+$10a	; wIndex register (16b)
WINDEX		cffield		WINDEXR,0,16	;  wIndex of setup transaction.
WLENGTHR	equ		MBAR_USB2+$10c	; wLength register (16b)
WLENGTH		cffield		WLENGTHR,0,16	;  wLength of setup transaction.
__N		set		1
		rept		6
		 __decstr	__NS,__N
EP{__NS}OUTACR	 equ		MBAR_USB2+$101+(__N*$30)	; EPn OUT attribute control register (8b)
TTYPE		 cffield	EP{__NS}OUTACR,0,2		;  Transfer type.
EP{__NS}OUTMPSR	 equ		MBAR_USB2+$102+(__N*$30)	; EPn OUT max packet size register (16b)
ADDTRANS	 cffield	EP{__NS}OUTMPSR,11,2		;  Additional transactions.
MAXPKTSZ	 cffield	EP{__NS}OUTMPSR,0,11		;  Maximum packet size.
EP{__NS}OUTIFR	 equ		MBAR_USB2+$104+(__N*$30)	; EPn OUT interface number register (8b)
IFNUM		 cffield	EP{__NS}OUTIFR,0,8			;  Interface number.
EP{__NS}OUTSR	 equ		MBAR_USB2+$105+(__N*$30)	; EPn OUT status register (8b)
INT		 cfbit		EP{__NS}OUTSR,7			;  Interrupt.
TXZERO		 cfbit		EP{__NS}OUTSR,5			;  Transmit a zero byte packet.
CCOMP		 cfbit		EP{__NS}OUTSR,3			;  Control command complete.
PSTALL		 cfbit		EP{__NS}OUTSR,2			;  Protocol stall.
ACTIVE		 cfbit		EP{__NS}OUTSR,1			;  Active.
HALT		 cfbit		EP{__NS}OUTSR,0			;  Halt.
EP{__NS}OUTSFR	 equ		MBAR_USB2+$10e+(__N*$30)	; EPn OUT sync frame register (16b)
FRMNUM		 cffield	EP{__NS}OUTSFR,0,11		;  Frame Number.
EP{__NS}INACR	 equ		MBAR_USB2+$119+(__N*$30)	; EPn IN attribute control register (8b)
TTYPE		 cffield	EP{__NS}INACR,0,2		;  Transfer type.
EP{__NS}INMPSR	 equ		MBAR_USB2+$11a+(__N*$30)	; EPn IN max packet size register (16b)
ADDTRANS	 cffield	EP{__NS}INMPSR,11,2		;  Additional transactions.
MAXPKTSZ	 cffield	EP{__NS}INMPSR,0,11		;  Maximum packet size.
EP{__NS}INIFR	 equ		MBAR_USB2+$11c+(__N*$30)	; EPn IN interface number register (8b)
IFNUM		 cffield	EP{__NS}INIFR,0,8		;  Interface number.
EP{__NS}INSR	 equ		MBAR_USB2+$11d+(__N*$30)	; EPn IN status register (8b)
INT		 cfbit		EP{__NS}INSR,7			;  Interrupt.
TXZERO		 cfbit		EP{__NS}INSR,5			;  Transmit a zero byte packet.
CCOMP		 cfbit		EP{__NS}INSR,3			;  Control command complete.
PSTALL		 cfbit		EP{__NS}INSR,2			;  Protocol stall.
ACTIVE		 cfbit		EP{__NS}INSR,1			;  Active.
HALT		 cfbit		EP{__NS}INSR,0			;  Halt.
EP{__NS}INSFR	 equ		MBAR_USB2+$12a+(__N*$30)	; EPn IN sync frame register (16b)
FRMNUM		 cffield	EP{__NS}INSFR,0,11		;  Frame Number.
__N		 set		__N+1
		endm
USBSR		equ		MBAR_USB2+$400	; USB status register (32b)
SUSP		cfbit		USBSR,7		;  Suspend
ISOERREP	cffield		USBSR,0,4	;  Isochronous error endpoint.
USBCR		equ		MBAR_USB2+$404	; USB control register (32b)
RAMSPLIT	cfbit		USBCR,5		;  RAM split.
RAMEN		cfbit		USBCR,3		;  Descriptor RAM Enable.
RST		cfbit		USBCR,2		;  USB reset.
APPLOCK		cfbit		USBCR,1		;  Application Lock.
RESUME		cfbit		USBCR,0		;  Resume.
DRAMCR		equ		MBAR_USB2+$408	; USB descriptor RAM control register (32b)
START		cfbit		DRAMCR,31	;  Start.
BSY		cfbit		DRAMCR,30	;  Busy.
DSIZE		cffield		DRAMCR,16,11	;  Descriptor size.
DADR		cffield		DRAMCR,0,10	;  Descriptor address.
DRAMDR		equ		MBAR_USB2+$40c	; USB descriptor RAM data register (32b)
DDAT		cffield		DRAMDR,0,8	;  Descriptor data.
USBISR		equ		MBAR_USB2+$410	; USB interrupt status register (32b)
USBIMR		equ		MBAR_USB2+$414	; USB interrupt mask register (32b)
		irp		Reg,USBISR,USBIMR
MSOF		 cfbit		Reg,7		;  Missed start of frame interrupt.
SOF		 cfbit		Reg,6		;  Start of frame interrupt.
RSTSTOP		 cfbit		Reg,5		;  Reset stop.
UPDSOF		 cfbit		Reg,4		;  Updated start of frame.
RES		 cfbit		Reg,3		;  Resume.
SUSP		 cfbit		Reg,2		;  Suspend.
FTUNLCK		 cfbit		Reg,1		;  Frame timer lost lock.
ISOERR		 cfbit		Reg,0		;  Isochronous error.
		endm

__N		set		0
		rept		7
		 __decstr	__NS,__N
EP{__NS}STAT	 equ		MBAR_USB2+$440+(__N*$30)	; EPn status and control register (32b)
BYTECNT		 cffield	EP{__NS}STAT,16,12		;  Byte count.
DIR		 cfbit		EP{__NS}STAT,7			;  Direction.
FLUSH		 cfbit		EP{__NS}STAT,1			;  Flush.
RST		 cfbit		EP{__NS}STAT,0			;  Reset.
EP{__NS}ISR	 equ		MBAR_USB2+$444+(__N*$30)	; EPn interrupt status register (32b)
EP{__NS}IMR	 equ		MBAR_USB2+$448+(__N*$30)	; EPn interrupt mask register (32b)
		 irp		Reg,EP{__NS}ISR,EP{__NS}IMR
FU		  cfbit		Reg,8				;  FIFO full.
EMT		  cfbit		Reg,7				;  FIFO empty.
ERR		  cfbit		Reg,6				;  FIFO error.
FIFOHI		  cfbit		Reg,5				;  FIFO high.
FIFOLO		  cfbit		Reg,4				;  FIFO low.
EOT		  cfbit		Reg,2				;  End of transfer.
EOF		  cfbit		Reg,0				;  End of frame.
		 endm
EP{__NS}FRCFGR	 equ		MBAR_USB2+$44c+(__N*$30)	; EPn FIFO RAM configuration register (32b)
BASE		 cffield	EP{__NS}FRCFGR,16,12		;  Base address.
DEPTH		 cffield	EP{__NS}FRCFGR,0,13		;  Depth.
EP{__NS}FDR	 equ		MBAR_USB2+$450+(__N*$30)	; EPn FIFO data register (32b)
EP{__NS}FSR	 equ		MBAR_USB2+$454+(__N*$30)	; EPn FIFO status register (32b)
IP		 cfbit		EP{__NS}FSR,31			;  Illegal pointer.
TXW		 cfbit		EP{__NS}FSR,30			;  Transmit wait states.
FRM		 cffield	EP{__NS}FSR,24,4		;  Frame indicator.
FAE		 cfbit		EP{__NS}FSR,23			;  Frame accept error.
RXW		 cfbit		EP{__NS}FSR,22			;  Receive wait states.
UF		 cfbit		EP{__NS}FSR,21			;  Underflow.
OF		 cfbit		EP{__NS}FSR,20			;  Overflow.
FR		 cfbit		EP{__NS}FSR,19			;  Frame ready.
FU		 cfbit		EP{__NS}FSR,18			;  FIFO full.
ALRM		 cfbit		EP{__NS}FSR,17			;  FIFO alarm.
EMT		 cfbit		EP{__NS}FSR,16			;  FIFO empty.
EP{__NS}FCR	 equ		MBAR_USB2+$458+(__N*$30)	; EPn FIFO control register (32b)
SHAD		 cfbit		EP{__NS}FCR,31			;  Shadow.
WFR		 cfbit		EP{__NS}FCR,29			;  Write end of frame.
TMR		 cfbit		EP{__NS}FCR,28			;  Timer mode.
FRM		 cfbit		EP{__NS}FCR,27			;  Frame mode.
GR		 cffield	EP{__NS}FCR,24,3		;  Granularity.
IPMSK		 cfbit		EP{__NS}FCR,23			;  Masks the IP bit in the EPnFSR register from generating a FIFO error.
FAEMSK		 cfbit		EP{__NS}FCR,22			;  Masks the RXW bit in the EPnFSR register from generating a FIFO error.
RXWMSK		 cfbit		EP{__NS}FCR,21			;  Masks the UF bit in the EPnFSR register from generating a FIFO error
UFMSK		 cfbit		EP{__NS}FCR,20			;  Masks the OF bit in the EPnFSR register from generating a FIFO error.
OFMSK		 cfbit		EP{__NS}FCR,19			;  Masks the TXW bit in the EPnFSR register from generating a FIFO error
TXWMSK		 cfbit		EP{__NS}FCR,18			;  Counter.
COUNTER		 
EP{__NS}FAR	 equ		MBAR_USB2+$45c+(__N*$30)	; EPn FIFO alarm register (32b)
ALAMP		 cffield	EP{__NS}FAR,0,12		;  Alarm pointer.
EP{__NS}FRP	 equ		MBAR_USB2+$460+(__N*$30)	; EPn FIFO read pointer (32b)
RP		 cffield	EP{__NS}FRP,0,12		;  Read Pointer
EP{__NS}FWP	 equ		MBAR_USB2+$464+(__N*$30)	; EPn FIFO write pointer (32b)
WP		 cffield	EP{__NS}FWP,0,12		;  Write Pointer
EP{__NS}LRFP	 equ		MBAR_USB2+$468+(__N*$30)	; EPn last read frame pointer (32b)
LRFP		 cffield	EP{__NS}LRFP,0,12		;  Last read frame pointer.
EP{__NS}LWFP	 equ		MBAR_USB2+$46c+(__N*$30)	; EPn last write frame pointer (32b)
LWFP		 cffield	EP{__NS}LWFP,0,12		;  Last write frame pointer.
__N		 set		__N+1
		endm

		restore				; re-enable listing

                endif                           ; __54xxusb2inc
