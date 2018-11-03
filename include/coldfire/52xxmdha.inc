		ifndef	__52xxmdhainc		; avoid multiple inclusion
__52xxmdhainc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXMDHA.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Message Digest   *
;*   Hardware Accelerator                                                   *
;*                                                                          *
;****************************************************************************

MDMR		equ		MBAR_MDHA+$00	; MDHA Mode Register (32b)
SSL		cfbit		MDMR,10		;  Secure socket layer MAC.
MACFULL		cfbit		MDMR,9		;  Message authentication code full.
SWAP		cfbit		MDMR,8		;  Swap message digest.
OPAD		cfbit		MDMR,7		;  Outer padding of message.
IPAD		cfbit		MDMR,6		;  Inner padding of message.
INIT		cfbit		MDMR,5		;  Initialization.
MAC		cffield		MDMR,3,2	;  Message authentication code.
PDATA		cfbit		MDMR,2		;  Pad data bit.
ALG		cfbit		MDMR,0		;  Algorithm.
MDCR		equ		MBAR_MDHA+$04	; MDHA Control Register (32b)
DMAL		cffield		MDCR,16,5	;  DMA request level.
END		cfbit		MDCR,2		;  Endian select
DMA		cfbit		MDCR,1		;  DMA enable.
IE		cfbit		MDCR,0		;  Interrupt enable.
MDCMR		equ		MBAR_MDHA+$08	; MDHA Command Register (32b)
GO		cfbit		MDCMR,3		;  Go.
CI		cfbit		MDCMR,2		;  Clear IRQ.
RI		cfbit		MDCMR,1		;  Re-initialize.
SWR		cfbit		MDCMR,0		;  Software reset.
MDSR		equ		MBAR_MDHA+$0C	; MDHA Status Register (32b)
IFL		cffield		MDSR,16,8	;  Input FIFO level.
APD		cffield		MDSR,13,3	;  Auto pad state.
FS		cffield		MDSR,8,3	;  FIFO size.
GNW		cfbit		MDSR,7		;  Get next word.
HSH		cfbit		MDSR,6		;  Hashing.
BUSY		cfbit		MDSR,4		;  Busy.
RD		cfbit		MDSR,3		;  Reset interrupt.
ERR		cfbit		MDSR,2		;  Error interrupt.
DONE		cfbit		MDSR,1		;  Done interrupt.
INT		cfbit		MDSR,0		;  MDHA single interrupt.
MDISR		equ		MBAR_MDHA+$10	; MDHA Interrupt Status Registers (32b)
DRL		cfbit		MDISR,10	;  DMA request level error.
GTDS		cfbit		MDISR,9		;  Greater than data size error.
ERE		cfbit		MDISR,8		;  Early read error.
RMDP		cfbit		MDISR,7		;  Register modified during processing.
DSE		cfbit		MDISR,5		;  Illegal data size.
IME		cfbit		MDISR,4		;  Illegal mode interrupt.
NEIF		cfbit		MDISR,2		;  Non-empty input FIFO upon done.
IFO		cfbit		MDISR,0		;  Input FIFO Overflow.
MDIMR		equ		MBAR_MDHA+$14	; MDHA Interrupt Mask Registers (32b)
DRL		cfbit		MDIMR,10	;  DMA request level error.
GTDS		cfbit		MDIMR,9		;  Greater than data size error.
ERE		cfbit		MDIMR,8		;  Early read error.
RMDP		cfbit		MDIMR,7		;  Register modified during processing.
DSE		cfbit		MDIMR,5		;  Illegal data size.
IME		cfbit		MDIMR,4		;  Illegal mode interrupt.
NEIF		cfbit		MDIMR,2		;  Non-empty input FIFO upon done.
IFO		cfbit		MDIMR,0		;  Input FIFO Overflow.
MDDSR		equ		MBAR_MDHA+$1C	; MDHA Data Size Register (32b)
MDIN		equ		MBAR_MDHA+$20	; MDHA Input FIFO (32b)
MDA0		equ		MBAR_MDHA+$30	; MDHA Message Digest A0 Register (32b)
MDB0		equ		MBAR_MDHA+$34	; MDHA Message Digest B0 Register (32b)
MDC0		equ		MBAR_MDHA+$38	; MDHA Message Digest C0 Register (32b)
MDD0		equ		MBAR_MDHA+$3C	; MDHA Message Digest D0 Register (32b)
MDE0		equ		MBAR_MDHA+$40	; MDHA Message Digest E0 Register (32b)
MDMDS		equ		MBAR_MDHA+$44	; MDHA Message Data Size Register (32b)
MDA1		equ		MBAR_MDHA+$70	; MDHA Message Digest A1 Register (32b)
MDB1		equ		MBAR_MDHA+$74	; MDHA Message Digest B1 Register (32b)
MDC1		equ		MBAR_MDHA+$78	; MDHA Message Digest C1 Register (32b)
MDD1		equ		MBAR_MDHA+$7C	; MDHA Message Digest D1 Register (32b)
MDE1		equ		MBAR_MDHA+$80	; MDHA Message Digest E1 Register (32b)

		restore				; re-enable listing

                endif                           ; __52xxmdhainc
