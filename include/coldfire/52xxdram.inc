		ifndef	__52xxdraminc		; avoid multiple inclusion
__52xxdraminc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXDRAM.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx DRAM Controller  *
;*                                                                          *
;****************************************************************************

SDMR		equ		MBAR_SDRAM+0	; Mode/Extended Mode Register (32b)
BK		cffield		SDMR,30,2	;  Bank Address
AD		cffield		SDMR,18,12	;  Address
CMD		cfbit		SDMR,16		;  Command
SDCR		equ		MBAR_SDRAM+4	; Control Register (32b)
MODE_EN		cfbit		SDCR,31		;  SDRAM Mode Register Programming Enable
CKE		cfbit		SDCR,30		;  Clock Enable
DDR_MODE	cfbit		SDCR,29		;  DDR Mode Select
REF_EN		cfbit		SDCR,28		;  Refresh Enable
ADDR_MUX	cffield		SDCR,24,2	;  Use of Internal Address Bits A[27:24]
OE_RULE		cfbit		SDCR,22		;  Drive Rule Selection
REF_CNT		cffield		SDCR,16,6	;  Refresh Rate
MEM_PS		cfbit		SDCR,13		;  Memory Data Port Size
DQS_OE		cffield		SDCR,10,2	;  DQS Output Enable
IREF		cfbit		SDCR,2		;  Initiate Refresh Command
IPALL		cfbit		SDCR,1		;  Initiate Precharge All Command
SDCFG1		equ		MBAR_SDRAM+8	; Configuration Register 1 (32b)
SRD2RWP		cffield		SDCFG1,28,4	;  Single Read to Read/Write/Precharge Delay
SWT2RWP		cffield		SDCFG1,24,3	;  Single Write to Read/Write/Precharge Delay
RD_LAT		cffield		SDCFG1,20,4	;  Read CAS Latency
ACT2RW		cffield		SDCFG1,16,3	;  Active to Read/Write Delay
PRE2ACT		cffield		SDCFG1,12,3	;  Precharge to Active Delay
REF2ACT		cffield		SDCFG1,8,4	;  Refresh to Active Delay
WT_LAT		cffield		SDCFG1,4,3	;  Write Latency
SDCFG2		equ		MBAR_SDRAM+12	; Configuration Register 2 (32b)
BRD2RP		cffield		SDCFG2,28,4	;  Burst Read to Read/Precharge Delay
BWT2RWP		cffield		SDCFG2,24,4	;  Burst Write to Read/Write/Precharge Delay
BRD2W		cffield		SDCFG2,20,4	;  Burst Read to Write Delay
BL		cffield		SDCFG2,16,4	;  Burst Length
__defsdcs	macro		N,Addr
SDCS{N}		equ		Addr		; Chip Select Configuration (32b)
CSBA		cffield		SDCS{N},20,12	;  Chip-Select Base Address
CSSZ		cffield		SDCS{N},0,4	;  Chip Select Size
		endm
		__defsdcs	"0",MBAR_SDRAM+16
		__defsdcs	"1",MBAR_SDRAM+20

		restore				; re-enable listing

                endif                           ; __52xxdraminc
