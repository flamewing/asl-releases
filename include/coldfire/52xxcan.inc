		ifndef	__52xxcaninc		; avoid multiple inclusion
__52xxcaninc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXCAN.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx CAN              *
;*                                                                          *
;****************************************************************************

CANMCR		equ		MBAR_CAN+$000	; FlexCAN Module Configuration Register (32b)
MDIS		cfbit		CANMCR,31	;  Module disable.
FRZ		cfbit		CANMCR,30	;  Freeze mode enable.
HALT		cfbit		CANMCR,28	;  Halt FlexCAN.
NOTRDY		cfbit		CANMCR,27	;  FlexCAN not ready.
SOFTRST		cfbit		CANMCR,25	;  Soft reset.
FRZACK		cfbit		CANMCR,24	;  Freeze acknowledge.
SUPV		cfbit		CANMCR,23	;  Supervisor/user data space.
LPMACK		cfbit		CANMCR,20	;  Low power mode acknowledge.
BCC		cfbit		CANMCR,16	;  Backwards compatibility configuration.
MAXMB		cffield		CANMCR,0,4	;  Maximum number of message buffers.
CANCTRL		equ		MBAR_CAN+$004	; FlexCAN Control Register (32b)
PRESDIV		cffield		CANCTRL,24,8	;  Prescaler division factor.
RJW		cffield		CANCTRL,22,2	;  Resynchronization jump width.
PSEG1		cffield		CANCTRL,19,3	;  Phase buffer segment 1.
PSEG2		cffield		CANCTRL,16,3	;  Phase buffer segment 2.
BOFFMSK		cfbit		CANCTRL,15	;  Bus off interrupt mask.
ERRMSK		cfbit		CANCTRL,14	;  Error interrupt mask.
CLK_SRC		cfbit		CANCTRL,13	;  Clock source.
LPB		cfbit		CANCTRL,12	;  Loop back.
SMP		cfbit		CANCTRL,7	;  Sampling mode.
BOFFREC		cfbit		CANCTRL,6	;  Bus off recovery mode.
TSYN		cfbit		CANCTRL,5	;  Timer synchronize mode.
LBUF		cfbit		CANCTRL,4	;  Lowest buffer transmitted first.
LOM		cfbit		CANCTRL,3	;  Listen-only mode.
PROPSEG		cffield		CANCTRL,0,3	;  Propagation segment.
TIMER		equ		MBAR_CAN+$008	; Free Running Timer (32b)
RXGMASK		equ		MBAR_CAN+$010	; Rx Global Mask (32b)
RX14MASK	equ		MBAR_CAN+$014	; Rx Buffer 14 Mask (32b)
RX15MASK	equ		MBAR_CAN+$018	; Rx Buffer 15 Mask (32b)
ERRCNT		equ		MBAR_CAN+$01C	; Error Counter Register (32b)
RXECTR		cffield		ERRCNT,8,8	;  Receive error counter.
TXECTR		cffield		ERRCNT,0,8	;  Transmit error counter.
ERRSTAT		equ		MBAR_CAN+$020	; Error and Status Register (32b)
BIT1ERR		cfbit		ERRSTAT,15	;  Bit1 error.
BIT0ERR		cfbit		ERRSTAT,14	;  Bit0 error.
ACKERR		cfbit		ERRSTAT,13	;  Acknowledge error.
CRCERR		cfbit		ERRSTAT,12	;  Cyclic redundancy check error.
FRMERR		cfbit		ERRSTAT,11	;  Message form error.
STFERR		cfbit		ERRSTAT,10	;  Bit stuff error.
TXWRN		cfbit		ERRSTAT,9	;  Transmit error status flag.
RXWRN		cfbit		ERRSTAT,8	;  Receiver error status flag.
IDLE		cfbit		ERRSTAT,7	;  Idle status.
TXRX		cfbit		ERRSTAT,6	;  Transmit/receive status.
FLTCONF		cffield		ERRSTAT,4,2	;  Fault confinement state.
BOFFINT		cfbit		ERRSTAT,2	;  Bus off interrupt.
ERRINT		cfbit		ERRSTAT,1	;  Error interrupt.
IMASK		equ		MBAR_CAN+$028	; Interrupt Mask Register (32b)
BUF15M		cfbit		IMASK,15	;  Buffer interrupt mask.
BUF14M		cfbit		IMASK,14
BUF13M		cfbit		IMASK,13
BUF12M		cfbit		IMASK,12
BUF11M		cfbit		IMASK,11
BUF10M		cfbit		IMASK,10
BUF9M		cfbit		IMASK,9
BUF8M		cfbit		IMASK,8
BUF7M		cfbit		IMASK,7
BUF6M		cfbit		IMASK,6
BUF5M		cfbit		IMASK,5
BUF4M		cfbit		IMASK,4
BUF3M		cfbit		IMASK,3
BUF2M		cfbit		IMASK,2
BUF1M		cfbit		IMASK,1
BUF0M		cfbit		IMASK,0
IFLAG		equ		MBAR_CAN+$030	; Interrupt Flag Register (32b)
BUF15I		cfbit		IFLAG,15	; Buffer interrupt flag.
BUF14I		cfbit		IFLAG,14
BUF13I		cfbit		IFLAG,13
BUF12I		cfbit		IFLAG,12
BUF11I		cfbit		IFLAG,11
BUF10I		cfbit		IFLAG,10
BUF9I		cfbit		IFLAG,9
BUF8I		cfbit		IFLAG,8
BUF7I		cfbit		IFLAG,7
BUF6I		cfbit		IFLAG,6
BUF5I		cfbit		IFLAG,5
BUF4I		cfbit		IFLAG,4
BUF3I		cfbit		IFLAG,3
BUF2I		cfbit		IFLAG,2
BUF1I		cfbit		IFLAG,1
BUF0I		cfbit		IFLAG,0
__N		set		0
		rept		16
		__decstr	__NS,__N
MB{"\{__NS}"}	equ		MBAR_CAN+$080+(__N*16)	; Message Buffers 0-15 (16*128b)
CODE		cffield		MB{"\{__NS}"},24,4	;  Message buffer code.
SRR		cfbit		MB{"\{__NS}"},22	;  Substitute remote request.
IDE		cfbit		MB{"\{__NS}"},21	;  ID extended bit.
RTR		cfbit		MB{"\{__NS}"},20	;  Remote transmission request.
LENGTH		cffield		MB{"\{__NS}"},16,4	;  Length of data in bytes.
TIMESTAMP	cffield		MB{"\{__NS}"},0,16	;  Free-running counter time stamp.
MB{"\{__NS}"}ID	equ		MB{"\{__NS}"}+4
EXTENDED	cffield		MB{"\{__NS}"}ID,18,11	;  Extended frame identifier.
STANDARD	cffield		MB{"\{__NS}"}ID,0,18	;  Standard frame identifier.
__N		set		__N+4
		endm

		restore				; re-enable listing

                endif                           ; __52xxcaninc
