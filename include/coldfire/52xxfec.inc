		ifndef	__52xxfecinc		; avoid multiple inclusion
__52xxfecinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXFEC.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Fast Ethernet    *
;*   Controller                                                             *
;*                                                                          *
;****************************************************************************

		include		"coldfire/5xxxfec.inc"
		__deffec	"",MBAR_FEC	; No 52xx/53xx device with more than one FEC

TXB		cfbit		EIR,26		;  Transmit Buffer Interrupt
RXF		cfbit		EIR,25		;  Receive Frame Interrupt
RXB		cfbit		EIR,24		;  Receive Buffer Interrupt
EBERR		cfbit		EIR,22		;  Ethernet Bus Error
UN		cfbit		EIR,19		;  Transmit FIFO Underrun

TXB		cfbit		EIMR,26		;  Transmit Buffer Interrupt
RXF		cfbit		EIMR,25		;  Receive Frame Interrupt
RXB		cfbit		EIMR,24		;  Receive Buffer Interrupt
EBERR		cfbit		EIMR,22		;  Ethernet Bus Error
UN		cfbit		EIMR,19		;  Transmit FIFO Underrun

RDAR		equ		MBAR_FEC+$010	; Receive Descriptor Active Register (32b)
RDAR		cfbit		RDAR,24
TDAR		equ		MBAR_FEC+$014	; Transmit Descriptor Active Register (32b)
TDAR		cfbit		TDAR,24
FRBR		equ		MBAR_FEC+$14C	; FIFO Receive Bound Register (32b)
R_BOUND		cffield		FRBR,2,8	;  Highest valid FIFO RAM Address
FRSR		equ		MBAR_FEC+$150	; FIFO Receive FIFO Start Register (32b)
R_FSTART	cffield		FRSR,2,8	;  Address of First Receive FIFO Location
ERDSR		equ		MBAR_FEC+$180	; Pointer to Receive Descriptor Ring (32b)
ETDSR		equ		MBAR_FEC+$184	; Pointer to Transmit Descriptor Ring (32b)
EMRBR		equ		MBAR_FEC+$188	; Maximum Receive Buffer Size (32b)
R_BUF_SIZE	cffield		EMRBR,4,7	; Maximum size of Receive Buffer

		restore				; re-enable listing

                endif                           ; __52xxfecinc
