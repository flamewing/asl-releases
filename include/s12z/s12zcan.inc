		ifndef	__s12z_can_inc
__s12z_can_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZCAN.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z CAN Interface             *
;*                                                                          *
;****************************************************************************

; -------------------------------------------------------------------------
; CANx

S12ZCAN		struct	dots
CTL0		ds.b	1		; ($00) MSCAN Control Register 0
RXFRM		s12zbit	CTL0,7		;  Received Frame Flag
RXACT		s12zbit	CTL0,6		;  Receiver Active Status
CSWAI		s12zbit	CTL0,5		;  CAN Stops in Wait Mode
SYNCH		s12zbit	CTL0,4		;  Synchronized Status
TIME		s12zbit	CTL0,3		;  Timer Enable
WUPE		s12zbit	CTL0,2		;  Wake-Up Enable
SLPRQ		s12zbit	CTL0,1		;  Sleep Mode Request
INITRQ		s12zbit	CTL0,0		;  Initialization Mode Request
CTL1		ds.b	1		; ($01) MSCAN Control Register 1
CANE		s12zbit	CTL1,7		;  MSCAN Enable
CLKSRC		s12zbit	CTL1,6		;  MSCAN Clock Source
LOOPB		s12zbit	CTL1,5		;  Loopback Self Test Mode
LISTEN		s12zbit	CTL1,4		;  Listen Only Mode
BORM		s12zbit	CTL1,3		;  Bus-Off Recovery Mode
WUPM		s12zbit	CTL1,2		;  Wake-Up Mode
SLPAK		s12zbit	CTL1,1		;  Sleep Mode Acknowledge
INITAK		s12zbit	CTL1,0		;  Initialization Mode Acknowledge
BTR0		ds.b	1		; ($02) MSCAN Bus Timing Register 0
SJW		s12zfld	BTR0,2:6	;  Synchronization Jump Width
BRP		s12zfld	BTR0,6:0	;  Baud Rate Prescaler
BTR1		ds.b	1		; ($03) MSCAN Bus Timing Register 1
SAMP		s12zbit	BTR1,7		;  Sampling
TSEG2		s12zfld	BTR1,3:4	;  Time Segment 2
TSEG1		s12zfld	BTR1,4:0	;  Time Segment 1
RFLG		ds.b	1		; ($04) MSCAN Receiver Flag Register
WUPIF		s12zbit	RFLG,7		;  Wake-Up Interrupt Flag
CSCIF		s12zbit	RFLG,6		;  CAN Status Change Interrupt Flag
RSTAT		s12zfld	RFLG,2:4	;  Receiver Status
TSTAT		s12zfld	RFLG,2:2	;  Transmitter Status
OVRIF		s12zbit	RFLG,1		;  Overrun Interrupt Flag
RXF		s12zbit	RFLG,0		;  Receive Buffer Full Flag
RIER		ds.b	1		; ($05) MSCAN Receiver Interrupt Enable Register
WUPIE		s12zbit	RIER,7		;  Wake-Up Interrupt Enable
CSCIE		s12zbit	RIER,6		;  CAN Status Change Interrupt Enable
RSTATE		s12zfld	RIER,2:4	;  Receiver Status Change Enable
TSTATE		s12zfld	RIER,2:2	;  Transmitter Status Change Enable
OVRIE		s12zbit	RIER,1		;  Overrun Interrupt Enable
RXFIE		s12zbit	RIER,0		;  Receiver Full Interrupt Enable
TFLG		ds.b	1		; ($06) MSCAN Transmitter Flag Register
TXE2		s12zbit	TFLG,2		;  Transmitter Buffer n Empty
TXE1		s12zbit	TFLG,1
TXE0		s12zbit	TFLG,0
TIER		ds.b	1		; ($07) MSCAN Transmitter Interrupt Enable Register
TXEIE2		s12zbit	TIER,2		;  Transmitter n Empty Interrupt Enable
TXEIE1		s12zbit	TIER,1
TXEIE0		s12zbit	TIER,0
TARQ		ds.b	1		; ($08) MSCAN Transmitter Message Abort Request Register
ABTRQ2		s12zbit	TARQ,2		;  Abort Request n
ABTRQ1		s12zbit	TARQ,1
ABTRQ0		s12zbit	TARQ,0
TAAK		ds.b	1		; ($09) MSCAN Transmitter Message Abort Acknowledge Register
ABTAK2		s12zbit	TAAK,2		;  Abort Acknowledge n
ABTAK1		s12zbit	TAAK,1
ABTAK0		s12zbit	TAAK,0
TBSEL		ds.b	1		; ($0a) MSCAN Transmit Buffer Selection Register
TX2		s12zbit	TBSEL,2		;  Transmit Buffer Select n
TX1		s12zbit	TBSEL,1
TX0		s12zbit	TBSEL,0
IDAC		ds.b	1		; ($0b) MSCAN Identifier Acceptance Control Register
IDAM		s12zfld	IDAC,2:4	;  Identifier Acceptance Mode
IDHIT		s12zfld	IDAC,3:0	;  Identifier Acceptance Hit Indicator
		ds.b	1
MISC		ds.b	1		; ($0d) MSCAN Miscellaneous Register
BOHOLD		s12zbit	MISC,0		;  Bus-off State Hold Until User Request
RXERR		ds.b	1		; ($0e) MSCAN Receive Error Counter
TXERR		ds.b	1		; ($0f) MSCAN Transmit Error Counter
IDAR0		ds.b	1		; ($10) MSCAN Identifier Acceptance Register 0
IDAR1		ds.b	1		; ($11) MSCAN Identifier Acceptance Register 1
IDAR2		ds.b	1		; ($12) MSCAN Identifier Acceptance Register 2
IDAR3		ds.b	1		; ($13) MSCAN Identifier Acceptance Register 3
IDMR0		ds.b	1		; ($14) MSCAN Identifier Mask Register 0
IDMR1		ds.b	1		; ($15) MSCAN Identifier Mask Register 1
IDMR2		ds.b	1		; ($16) MSCAN Identifier Mask Register 2
IDMR3		ds.b	1		; ($17) MSCAN Identifier Mask Register 3
IDAR4		ds.b	1		; ($18) MSCAN Identifier Acceptance Register 4
IDAR5		ds.b	1		; ($19) MSCAN Identifier Acceptance Register 5
IDAR6		ds.b	1		; ($1a) MSCAN Identifier Acceptance Register 6
IDAR7		ds.b	1		; ($1b) MSCAN Identifier Acceptance Register 7
IDMR4		ds.b	1		; ($1c) MSCAN Identifier Mask Register 4
IDMR5		ds.b	1		; ($1d) MSCAN Identifier Mask Register 5
IDMR6		ds.b	1		; ($1e) MSCAN Identifier Mask Register 6
IDMR7		ds.b	1		; ($1f) MSCAN Identifier Mask Register 7
		ends	S12ZCAN

; -------------------------------------------------------------------------
; CANPHY

CPDR		equ	$0990		; Port CP Data Register
CPDR7		s12zbit	CPDR,7		;  Port CP Data Bit 7
CPDR1		s12zbit	CPDR,1		;  Port CP Data Bit 1
CPDR0		s12zbit	CPDR,0		;  Port CP Data Bit 0
CPCR		equ	$0991		; CAN Physical Layer Control Register
CPE		s12zbit	CPCR,7		;  CAN Physical Layer Enable
SPE		s12zbit	CPCR,6		;  Split Enable
WUPE		s12zfld	CPCR,2:4	;  Wake-Up Receiver Enable and Filter Select
SLR		s12zfld	CPCR,3:0	;  Slew Rate
CPSR		equ	$0993		; CAN Physical Layer Status Register
CPCHVH		s12zbit	CPSR,7		;  CANH Voltage Failure High Status Bit
CPCHVL		s12zbit	CPSR,6		;  CANH Voltage Failure Low Status Bit
CPCLVH		s12zbit	CPSR,5		;  CANL Voltage Failure High Status Bit
CPCLVL		s12zbit	CPSR,4		;  CANL Voltage Failure Low Status Bit
CPDT		s12zbit	CPSR,3		;  CPTXD-Dominant Timeout Status Bit
CPIE		equ	$0996		; CAN Physical Layer Interrupt Enable Register
CPVFIE		s12zbit	CPIE,4		;  CAN Physical Layer Voltage-Failure Interrupt Enable
CPDTIE		s12zbit	CPIE,3		;  CPTXD-Dominant Timeout Interrupt Enable
CPOCIE		s12zbit	CPIE,0		;  CAN Physical Layer Over-current Interrupt Enable
CPIF		equ	$0997		; CAN Physical Layer Interrupt Flag Register
CHVHIF		s12zbit	CPIF,7		;  CANH Voltage Failure High Interrupt Flag
CHVLIF		s12zbit	CPIF,6		;  CANH Voltage Failure Low Interrupt Flag
CLVHIF		s12zbit	CPIF,5		;  CANL Voltage Failure High Interrupt Flag
CLVLIF		s12zbit	CPIF,4		;  CANL Voltage Failure Low Interrupt Flag
CPDTIF		s12zbit	CPIF,3		;  CAN CPTXD-Dominant Timeout Interrupt Flag
CHOCIF		s12zbit	CPIF,1		;  CANH Over-Current Interrupt Flag
CLOCIF		s12zbit	CPIF,0		;  CANL Over-Current Interrupt Flag

		restore			; re-enable listing

		endif			; __s12z_can_inc
