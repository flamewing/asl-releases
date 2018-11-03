		ifndef	__54xxsecinc		; avoid multiple inclusion
__54xxsecinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 54XXSEC.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF54xx Security Engine  *
;*                                                                          *
;****************************************************************************

; Arbiter/Controller Control register space

MBAR_SCTRL	equ		MBAR_SEC+$1000

EUACRH		equ		MBAR_SCTRL+$00	; EU Assignment Control Register High (32b)
RNG		cffield		EUACRH,24,4	;  RNG Assignment
MDEU		cffield		EUACRH,8,4	;  MDEU Assignment
AFEU		cffield		EUACRH,0,4	;  AFEU Assignment
EUACRL		equ		MBAR_SCTRL+$04	; EU Assignment Control Register Low (32b)
DEU		cffield		EUACRL,24,4	;  DEU Assignment
AESU		cffield		EUACRL,16,4	;  AESU Assignment
SIMRH		equ		MBAR_SCTRL+$08	; SEC Interrupt Mask Register High (32b)
CHA_1_ERR	cfbit		SIMRH,31	;  Channel 1 Error Interrupt Mask
CHA_1_DN	cfbit		SIMRH,30	;  Channel 1 Done Interrupt Mask
CHA_0_ERR	cfbit		SIMRH,29	;  Channel 0 Error Interrupt Mask
CHA_0_DN	cfbit		SIMRH,28	;  Channel 0 Done Interrupt Mask
AERR		cfbit		SIMRH,27	;  Assignment Error Interrupt Mask
SIMRL		equ		MBAR_SCTRL+$0C	; SEC Interrupt Mask Register Low (32b)
RNG_ERR		cfbit		SIMRL,25	;  RNG Error Interrupt Mask
RNG_DN		cfbit		SIMRL,24	;  RNG Done Interrupt Mask
AFEU_ERR	cfbit		SIMRL,21	;  AFEU Error Interrupt Mask
AFEU_DN		cfbit		SIMRL,20	;  AFEU Done Interrupt Mask
MDEU_ERR	cfbit		SIMRL,17	;  MDEU Error Interrupt Mask
MDEU_DN		cfbit		SIMRL,16	;  MDEU Done Interrupt Mask
AESU_ERR	cfbit		SIMRL,13	;  AESU Error Interrupt Mask
AESU_DN		cfbit		SIMRL,12	;  AESU Done Interrupt Mask
DEU_ERR		cfbit		SIMRL,9		;  DEU Error Interrupt Mask
DEU_DN		cfbit		SIMRL,8		;  DEU Done Interrupt Mask
TEA		cfbit		SIMRL,6		;  Transfer Error Acknowledge Mask
SISRH		equ		MBAR_SCTRL+$10	; SEC Interrupt Status Register High (32b)
CHA_1_ERR	cfbit		SISRH,31	;  Channel 1 Error Interrupt Status
CHA_1_DN	cfbit		SISRH,30	;  Channel 1 Done Interrupt Status
CHA_0_ERR	cfbit		SISRH,29	;  Channel 0 Error Interrupt Status
CHA_0_DN	cfbit		SISRH,28	;  Channel 0 Done Interrupt Status
AERR		cfbit		SISRH,27	;  Assignment Error Interrupt Status
SISRL		equ		MBAR_SCTRL+$14	; SEC Interrupt Status Register Low (32b)
RNG_ERR		cfbit		SISRL,25	;  RNG Error Interrupt Status
RNG_DN		cfbit		SISRL,24	;  RNG Done Interrupt Status
AFEU_ERR	cfbit		SISRL,21	;  AFEU Error Interrupt Status
AFEU_DN		cfbit		SISRL,20	;  AFEU Done Interrupt Status
MDEU_ERR	cfbit		SISRL,17	;  MDEU Error Interrupt Status
MDEU_DN		cfbit		SISRL,16	;  MDEU Done Interrupt Status
AESU_ERR	cfbit		SISRL,13	;  AESU Error Interrupt Status
AESU_DN		cfbit		SISRL,12	;  AESU Done Interrupt Status
DEU_ERR		cfbit		SISRL,9		;  DEU Error Interrupt Status
DEU_DN		cfbit		SISRL,8		;  DEU Done Interrupt Status
TEA		cfbit		SISRL,6		;  Transfer Error Acknowledge Status
SICRH		equ		MBAR_SCTRL+$18	; SEC Interrupt Control Register High (32b)
CHA_1_ERR	cfbit		SICRH,31	;  Channel 1 Error Interrupt Control
CHA_1_DN	cfbit		SICRH,30	;  Channel 1 Done Interrupt Control
CHA_0_ERR	cfbit		SICRH,29	;  Channel 0 Error Interrupt Control
CHA_0_DN	cfbit		SICRH,28	;  Channel 0 Done Interrupt Control
AERR		cfbit		SICRH,27	;  Assignment Error Interrupt Control
SICRL		equ		MBAR_SCTRL+$1C	; SEC Interrupt Control Register Low (32b)
RNG_ERR		cfbit		SICRL,25	;  RNG Error Interrupt Control
RNG_DN		cfbit		SICRL,24	;  RNG Done Interrupt Control
AFEU_ERR	cfbit		SICRL,21	;  AFEU Error Interrupt Control
AFEU_DN		cfbit		SICRL,20	;  AFEU Done Interrupt Control
MDEU_ERR	cfbit		SICRL,17	;  MDEU Error Interrupt Control
MDEU_DN		cfbit		SICRL,16	;  MDEU Done Interrupt Control
AESU_ERR	cfbit		SICRL,13	;  AESU Error Interrupt Control
AESU_DN		cfbit		SICRL,12	;  AESU Done Interrupt Control
DEU_ERR		cfbit		SICRL,9		;  DEU Error Interrupt Control
DEU_DN		cfbit		SICRL,8		;  DEU Done Interrupt Control
TEA		cfbit		SICRL,6		;  Transfer Error Acknowledge Control
SIDR		equ		MBAR_SCTRL+$20	; SEC ID Register (32b)
VERSION		cffield		SIDR,0,32	;  Version of the SEC
EUASRH		equ		MBAR_SCTRL+$28	; EU Assignment Status Register High (32b)
RNG		cffield		EUASRH,24,4	;  Actual RNG Assignment
MDEU		cffield		EUASRH,8,4	;  Actual MDEU Assignment
AFEU		cffield		EUASRH,0,4	;  Actual AFEU Assignment
EUASRL		equ		MBAR_SCTRL+$2C	; EU Assignment Status Register Low (32b)
DESU		cffield		EUASRL,24,4	;  Actual DEU Assignment
AESU		cffield		EUASRL,16,4	;  Actual AESU Assignment
SMCR		equ		MBAR_SCTRL+$30	; SEC Master Control Register (32b)
SWR		cfbit		SMCR,24		;  Software Reset
CURR_CHAN	cffield		SMCR,4,4	;  Current Channel.
MEAR		equ		MBAR_SCTRL+$38	; Master Error Address Register (32b)
ADDRESS		cffield		MEAR,0,32	;  Target address of the transaction when TEA was received.

; Crypto-channels

__defsecchan	macro		N,Base
CCCR{N}		equ		Base+$0c	; Crypto-Channel Configuration Register n (32b)
BURST_SIZE	cffield		CCCR{N},8,3	;  Burst Size
WE		cfbit		CCCR{N},4	;  Writeback Enable.
NE		cfbit		CCCR{N},3	;  Fetch Next Descriptor Enable.
NT		cfbit		CCCR{N},2	;  Channel DONE Notification Type.
CDIE		cfbit		CCCR{N},1	;  Channel DONE Interrupt Enable.
RST		cfbit		CCCR{N},0	;  Reset Crypto-Channel.
CCPSRH{N}	equ		Base+$10	; Crypto-Channel Pointer Status Register High n (32b)
STATE		cffield		CCPSRH{N},0,8	;  State of the crypto-channel state machine.
CCPSRL{N}	equ		Base+$14	; Crypto-Channel Pointer Status Register Low n (32b)
STAT		cfbit		CCPSRL{N},26	;  Crypto-Channel Static Mode Enable.
MI		cfbit		CCPSRL{N},25	;  Multi_EU_IN.
MO		cfbit		CCPSRL{N},24	;  Multi_EU_OUT.
PR		cfbit		CCPSRL{N},23	;  Primary request.
SR		cfbit		CCPSRL{N},22	;  Secondary request.
PG		cfbit		CCPSRL{N},21	;  Primary EU granted.
SG		cfbit		CCPSRL{N},20	;  Secondary EU granted.
PRD		cfbit		CCPSRL{N},19	;  Primary EU reset done.
SRD		cfbit		CCPSRL{N},18	;  Secondary EU reset done.
PD		cfbit		CCPSRL{N},17	;  Primary EU done.
SD		cfbit		CCPSRL{N},16	;  Secondary EU done.
TEA		cfbit		CCPSRL{N},13	;  Transfer error acknowledge.
PERR		cfbit		CCPSRL{N},12	;  Pointer not complete error.
DERR		cfbit		CCPSRL{N},10	;  Descriptor error. The
SERR		cfbit		CCPSRL{N},9	;  Static assignment error.
EUERR		cfbit		CCPSRL{N},8	;  EU error.
PAIR_PTR	cffield		CCPSRL{N},0,8	;  Descriptor buffer register length/pointer pair
CDPR{N}		equ		Base+$44	; Crypto-Channel Current Descriptor Pointer Register n (32b)
FR{N}		equ		Base+$4c	; Fetch Register n (32b)
FETCH_ADDR	cffield		FR{N},0,32	;  Fetch address.
CDBUF{N}	equ		Base+$80	; Crypto-Channel Descriptor Buffer n (8*32b)
		endm

MBAR_SCHAN0	equ		MBAR_SEC+$2000
		__defsecchan	"0",MBAR_SCHAN0

; Crypto-channel 2

MBAR_SCHAN1	equ		MBAR_SEC+$3000
		__defsecchan	"1",MBAR_SCHAN1

; ArcFour Execution Unit

MBAR_AFEU	equ		MBAR_SEC+$8000
AFRCR		equ		MBAR_AFEU+$18	; AFEU Reset Control Register (32b)
AFSR		equ		MBAR_AFEU+$28	; AFEU Status Register (32b)
AFISR		equ		MBAR_AFEU+$30	; AFEU Interrupt Status Register (32b)
AFIMR		equ		MBAR_AFEU+$38	; AFEU Interrupt Mask Register (32b)
		irp		Reg,AFISR,AFIMR
ME		 cfbit		Reg,31		;  Mode error.
AE		 cfbit		Reg,30		;  Address error.
OFE		 cfbit		Reg,29		;  Output FIFO error.
IFE		 cfbit		Reg,28		;  Input FIFO error.
IFO		 cfbit		Reg,26		;  Input FIFO overflow.
OFU		 cfbit		Reg,25		;  Output FIFO underflow.
IE		 cfbit		Reg,20		;  Internal error.
ERE		 cfbit		Reg,19		;  Early read error.
CE		 cfbit		Reg,18		;  Context error.
KSE		 cfbit		Reg,17		;  Key size error.
DSE		 cfbit		Reg,16		;  Data size error.
		endm

; DES Execution Unit

MBAR_DEU	equ		MBAR_SEC+$a000
DRCR		equ		MBAR_DEU+$18	; DEU Reset Control Register (32b)
DEU_DSR		equ		MBAR_DEU+$28	; DEU Status Register (32b)
DISR		equ		MBAR_DEU+$30	; DEU Interrupt Status Register (32b)
DEU_DIMR	equ		MBAR_DEU+$38	; DEU Interrupt Mask Register (32b)
		irp		Reg,DISR,DIMR
ME		 cfbit		Reg,31		;  Mode error.
AE		 cfbit		Reg,30		;  Address error.
OFE		 cfbit		Reg,29		;  Output FIFO error.
IFE		 cfbit		Reg,28		;  Input FIFO error.
IFO		 cfbit		Reg,26		;  Input FIFO overflow.
OFU		 cfbit		Reg,25		;  Output FIFO underflow.
KPE		 cfbit		Reg,21		;  Key Parity Error.
IE		 cfbit		Reg,20		;  Internal error.
ERE		 cfbit		Reg,19		;  Early read error.
CE		 cfbit		Reg,18		;  Context error.
KSE		 cfbit		Reg,17		;  Key size error.
DSE		 cfbit		Reg,16		;  Data size error.
		endm

; Message Digest Execution Unit

MBAR_MDEU	equ		MBAR_SEC+$c000
MDRCR		equ		MBAR_MDEU+$18	; MDEU Reset Control Register (32b)
MDSR		equ		MBAR_MDEU+$28	; MDEU Status Register (32b)
MDISR		equ		MBAR_MDEU+$30	; MDEU Interrupt Status Register (32b)
MDIMR		equ		MBAR_MDEU+$38	; MDEU Interrupt Mask Register (32b)
		irp		Reg,MDISR,MDIMR
ME		 cfbit		Reg,31		;  Mode Error.
AE		 cfbit		Reg,30		;  Address Error.
IFO		 cfbit		Reg,26		;  Input FIFO Overflow.
IE		 cfbit		Reg,20		;  Internal Error.
ERE		 cfbit		Reg,19		;  Early Read Error.
CE		 cfbit		Reg,18		;  Context Error.
KSE		 cfbit		Reg,17		;  Key Size Error.
DSE		 cfbit		Reg,16		;  Data Size Error.
		endm

; Random Number Generator

MBAR_RNG	equ		MBAR_SEC+$e000
RNGRCR		equ		MBAR_RNG+$18	; RNG Reset Control Register (32b)
RNGSR		equ		MBAR_RNG+$28	; RNG Status Register (32b)
RNGISR		equ		MBAR_RNG+$30	; RNG Interrupt Status Register (32b)
RNGIMR		equ		MBAR_RNG+$38	; RNG Interrupt Mask Register (32b)
		irp		Reg,RNGISR,RNGIMR
ME		 cfbit		Reg,31		;  Mode Error.
AE		 cfbit		Reg,30		;  Address Error.
OFU		 cfbit		Reg,25		;  Output FIFO underflow.
IE		 cfbit		Reg,20		;  Internal error.
		endm

; AES Execution Unit

MBAR_AESU	equ		MBAR_SEC+$12000
AESRCR		equ		MBAR_AESU+$18	; AESU Reset Control Register (32b)
AESSR		equ		MBAR_AESU+$28	; AESU Status Register (32b)
AESISR		equ		MBAR_AESU+$30	; AESU Interrupt Status Register (32b)
AESIMR		equ		MBAR_AESU+$38	; AESU Interrupt Mask Register (32b)
		irp		Reg,AESISR,AESIMR
ME		 cfbit		Reg,31		;  Mode Error.
AE		 cfbit		Reg,30		;  Address Error.
OFE		 cfbit		Reg,29		;  Output FIFO error.
IFE		 cfbit		Reg,28		;  Input FIFO error.
IFO		 cfbit		Reg,26		;  Input FIFO Overflow.
OFU		 cfbit		Reg,25		;  Output FIFO underflow.
IE		 cfbit		Reg,20		;  Internal Error.
ERE		 cfbit		Reg,19		;  Early Read Error.
CE		 cfbit		Reg,18		;  Context Error.
KSE		 cfbit		Reg,17		;  Key Size Error.
DSE		 cfbit		Reg,16		;  Data Size Error.
		endm

; Common Bits in all EUs

		irp		Reg,AFRCR,DRCR,MDRCR,RNGRCR,AESRCR
RI		 cfbit		Reg,26		;  Reset interrupt.
MI		 cfbit		Reg,25		;  Module initialization.
SR		 cfbit		Reg,24		;  Software reset.
		endm

		irp		Reg,AFSR,DSR,MDSR,RNGSR,AESSR
HALT		 cfbit		Reg,29		;  Halt.
IFW		 cfbit		Reg,28		;  Input FIFO writable. (not on RNG)
OFR		 cfbit		Reg,27		;  Output FIFO readable. (not on MDEEU)
IE		 cfbit		Reg,26		;  Interrupt error.
ID		 cfbit		Reg,25		;  Interrupt done. (not on RNG)
RD		 cfbit		Reg,24		;  Reset done.
		endm

		restore				; re-enable listing

                endif                           ; __54xxsecinc
