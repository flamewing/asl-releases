SKMR		equ		MBAR_SKHA+$00	; SKHA Mode Register (32b)
CTRM		cffield		SKMR,9,4	;  Counter mode modulus.
DKP		cfbit		SKMR,8		;  Disable key parity check.
CM		cffield		SKMR,3,2	;  Cipher mode.
DIR		cfbit		SKMR,2		;  Direction.
ALG		cffield		SKMR,0,2	;  Algorithm.
SKCR		equ		MBAR_SKHA+$04	; SKHA Control Register (32b)
ODMAL		cffield		SKCR,24,6	;  Output DMA Request Level.
IDMAL		cffield		SKCR,16,6	;  Input DMA Request Level.
END		cfbit		SKCR,3		;  Endian Select
ODMA		cfbit		SKCR,2		;  Output DMA Enable
IDMA		cfbit		SKCR,1		;  Input DMA Enable
IE		cfbit		SKCR,0		;  Interrupt enable.
SKCMR		equ		MBAR_SKHA+$08	; SKHA Command Register (32b)
GO		cfbit		SKCMR,3		;  Go.
CI		cfbit		SKCMR,2		;  Clear Interrupt Request.
RI		cfbit		SKCMR,1		;  Reinitialize.
SWR		cfbit		SKCMR,0		;  Software Reset.
SKSR		equ		MBAR_SKHA+$0C	; SKHA Status Register (32b)
OFL		cffield		SKSR,24,8	;  Output FIFO level.
IFL		cffield		SKSR,16,8	;  Input FIFO level.
BUSY		cfbit		SKSR,4		;  Busy.
RD		cfbit		SKSR,3		;  Reset done.
ERR		cfbit		SKSR,2		;  Error interrupt.
DONE		cfbit		SKSR,1		;  Done interrupt.
INT		cfbit		SKSR,0		;  SKHA interrupt.
SKESR		equ		MBAR_SKHA+$10	; SKHA Error Status Register (32b)
DRL		cfbit		SKESR,11	;  DMA request level error.
KRE		cfbit		SKESR,10	;  Key read error.
KPE		cfbit		SKESR,9		;  Key parity error.
ERE		cfbit		SKESR,8		;  Early read.
RMDP		cfbit		SKESR,7		;  Register modified during processing
KSE		cfbit		SKESR,6		;  Key size error
DSE		cfbit		SKESR,5		;  Data size error
IME		cfbit		SKESR,4		;  Illegal mode error
NEOF		cfbit		SKESR,3		;  Non-empty output FIFO upon start.
NEIF		cfbit		SKESR,2		;  Non-empty input FIFO upon done.
OFU		cfbit		SKESR,1		;  Output FIFO underflow
IFO		cfbit		SKESR,0		;  Input FIFO overflow.
SKEMR		equ		MBAR_SKHA+$14	; SKHA Error Status Mask Register (32b)
DRL		cfbit		SKEMR,11	;  DMA request level error.
KRE		cfbit		SKEMR,10	;  Key read error.
KPE		cfbit		SKEMR,9		;  Key parity error.
ERE		cfbit		SKEMR,8		;  Early read.
RMDP		cfbit		SKEMR,7		;  Register modified during processing
KSE		cfbit		SKEMR,6		;  Key size error
DSE		cfbit		SKEMR,5		;  Data size error
IME		cfbit		SKEMR,4		;  Illegal mode error
NEOF		cfbit		SKEMR,3		;  Non-empty output FIFO upon start.
NEIF		cfbit		SKEMR,2		;  Non-empty input FIFO upon done.
OFU		cfbit		SKEMR,1		;  Output FIFO underflow
IFO		cfbit		SKEMR,0		;  Input FIFO overflow.
SKKSR		equ		MBAR_SKHA+$18	; SKHA Key Size Register (32b)
SKDSR		equ		MBAR_SKHA+$1C	; SKHA Data Size Register (32b)
SKIN		equ		MBAR_SKHA+$20	; SKHA Input FIFO (32b)
SKOUT		equ		MBAR_SKHA+$24	; SKHA Output FIFO (32b)
SKKDR1		equ		MBAR_SKHA+$30	; SKHA Key Data Register 1 (32b)
SKKDR2		equ		MBAR_SKHA+$34	; SKHA Key Data Register 2 (32b)
SKKDR3		equ		MBAR_SKHA+$38	; SKHA Key Data Register 3 (32b)
SKKDR4		equ		MBAR_SKHA+$3C	; SKHA Key Data Register 4 (32b)
SKKDR5		equ		MBAR_SKHA+$40	; SKHA Key Data Register 5 (32b)
SKKDR6		equ		MBAR_SKHA+$44	; SKHA Key Data Register 6 (32b)
SKC1		equ		MBAR_SKHA+$70	; SKHA Context 1 (32b)
SKC2		equ		MBAR_SKHA+$74	; SKHA Context 2 (32b)
SKC3		equ		MBAR_SKHA+$78	; SKHA Context 3 (32b)
SKC4		equ		MBAR_SKHA+$7C	; SKHA Context 4 (32b)
SKC5		equ		MBAR_SKHA+$80	; SKHA Context 5 (32b)
SKC6		equ		MBAR_SKHA+$84	; SKHA Context 6 (32b)
SKC7		equ		MBAR_SKHA+$88	; SKHA Context 7 (32b)
SKC8		equ		MBAR_SKHA+$8C	; SKHA Context 8 (32b)
SKC9		equ		MBAR_SKHA+$90	; SKHA Context 9 (32b)
SKC10		equ		MBAR_SKHA+$94	; SKHA Context 10 (32b)
SKC11		equ		MBAR_SKHA+$98	; SKHA Context 11 (32b)
