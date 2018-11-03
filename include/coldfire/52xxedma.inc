		ifndef	__52xxedmainc		; avoid multiple inclusion
__52xxedmainc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXEDMA.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx EDMA             *
;*                                                                          *
;****************************************************************************

EDMA_CR		equ		MBAR_EDMA+$000	; Control Register (32b)
ERCA		cfbit		EDMA_CR,2	;  Enable Round Robin Channel Arbitration
EDBG		cfbit		EDMA_CR,1	;  Enable Debug
EDMA_ES		equ		MBAR_EDMA+$004	; Error Status Register (32b)
VLD		cfbit		EDMA_ES,31	;  Logical OR of all EDMA_ERR Status Bits
CPE		cfbit		EDMA_ES,14	;  Channel Priority Error
ERRCHN		cffield		EDMA_ES,8,4	;  Error Channel Number
SAE		cfbit		EDMA_ES,7	;  Source Address Error
SOE		cfbit		EDMA_ES,6	;  Source Offset Error
DAE		cfbit		EDMA_ES,5	;  Destination Address Error
DOE		cfbit		EDMA_ES,4	;  Destination Offset Error
NCE		cfbit		EDMA_ES,3	;  NBYTES/CITER Configuration Error
SGE		cfbit		EDMA_ES,2	;  Scatter/Gather Configuration Error
SBE		cfbit		EDMA_ES,1	;  Source Bus Error
DBE		cfbit		EDMA_ES,0	;  Destination Bus Error
EDMA_ERQ	equ		MBAR_EDMA+$00E	; Enable Request Register (16b)
EDMA_EEI	equ		MBAR_EDMA+$016	; Enable Error Interrupt Register (16b)
EDMA_SERQ	equ		MBAR_EDMA+$018	; Set Enable Request (8b)
SAER		cfbit		EDMA_SERQ,6	;  Set All Enable Requests
SERQ		cffield		EDMA_SERQ,0,4	;  Set Enable Request
EDMA_CERQ	equ		MBAR_EDMA+$019	; Clear Enable Request (8b)
CAER		cfbit		EDMA_CERQ,6	;  Clear All Enable Requests
CERQ		cffield		EDMA_CERQ,0,4	;  Clear Enable Request
EDMA_SEEI	equ		MBAR_EDMA+$01A	; Set Enable Error Interrupt Register (8b)
SAEE		cfbit		EDMA_CERR,6	;  Sets All Enable Error Interrupts
SEEI		cffield		EDMA_CERR,0,4	;  Set Enable Error Interrupt
EDMA_CEEI	equ		MBAR_EDMA+$01B	; Clear Enable Error Interrupt Register (8b)
CAEE		cfbit		EDMA_CEEI,6	;  Clear All Enable Error Interrupts
CEEI		cffield		EDMA_CEEI,0,4	;  Clear Enable Error Interrupt
EDMA_CINT	equ		MBAR_EDMA+$01C	; Clear Interrupt Request Register (8b)
CAIR		cfbit		EDMA_CINT,6	;  Clear All Interrupt Requests
CINT		cffield		EDMA_CINT,0,4	;  Clear Interrupt Request
EDMA_CERR	equ		MBAR_EDMA+$01D	; Clear Error Register (8b)
CAEI		cfbit		EDMA_CERR,6	;  Clear All Error Indicators
CERR		cffield		EDMA_CERR,0,4	;  Clear Error Indicator
EDMA_SSRT	equ		MBAR_EDMA+$01E	; Set START Bit Register (8b)
SAST		cfbit		EDMA_SSRT,6	;  Set All START Bits
SSRT		cffield		EDMA_SSRT,0,4	;  Set START Bit
EDMA_CDNE	equ		MBAR_EDMA+$01F	; Clear DONE Status Bit Register (8b)
CADN		cfbit		EDMA_CDNE,6	;  Clears All DONE Bits
CDNE		cffield		EDMA_CDNE,0,4	;  Clear DONE Bit
EDMA_INT	equ		MBAR_EDMA+$026	; Interrupt Request Register (32b)
EDMA_ERR	equ		MBAR_EDMA+$02E	; Error Register (32b)
__N		set		0
		rept		16
		__decstr	__NS,__N
DCHPRI{__NS}	equ		MBAR_EDMA+$100+__N ; Channel n Priority Register (8b)
ECP		cfbit		DCHPRI{__NS},7	;  Enable Channel Preemption
CHPRI		cffield		DCHPRI{__NS},0,4 ;  Channel n Arbitration Priority
TCD{__NS}	equ		MBAR_EDMA+$1000+(__N*32) ; Transfer Control Descriptor (256b)
TCD{__NS}_SADDR	equ		TCD{__NS}+0	;  Source Address (32b)
TCD{__NS}_ATTR	equ		TCD{__NS}+4	;  Transfer Attributes (16b)
SMOD		cffield		TCD{__NS}_ATTR,11,5 ;   Source Address Modulo
SSIZE		cffield		TCD{__NS}_ATTR,8,3 ;   Source data Transfer Size
DMOD		cffield		TCD{__NS}_ATTR,3,5 ;   Destination Address Modulo
DSIZE		cffield		TCD{__NS}_ATTR,0,3 ;   Destination Data Transfer Size
TCD{__NS}_SOFF	equ		TCD{__NS}+6	;  Signed Source Address Offset (16b)
TCD{__NS}_NBYTES	equ	TCD{__NS}+8	;  Minor Byte Count (32b)
TCD{__NS}_SLAST	equ		TCD{__NS}+12	;  Last Source Address Adjustment (32b)
TCD{__NS}_DADDR	equ		TCD{__NS}+16	;  Destination Address (32b)
TCD{__NS}_CITER	equ		TCD{__NS}+20	;  Current Minor Loop Link, Major Loop Count (16b)
E_LINK		cfbit		TCD{__NS}_CITER,15 ;   Enable Channel-to-Channel Linking
LINKCH		cffield		TCD{__NS}_CITER,9,4 ;   Link Channel Number
TCD{__NS}_DOFF	equ		TCD{__NS}+22	;  Signed Destination Address Offset (16b)
TCD{__NS}_DLAST_SGA	equ	TCD{__NS}+24	;  Last Destination Address Adjustment/Scatter Gather Address (32b)
TCD{__NS}_BITER	equ		TCD{__NS}+28	;  Beginning Minor Loop Link, Major Loop Count (16b)
E_LINK		cfbit		TCD{__NS}_BITER,15 ;   Enables Channel-to-Channel Linking
LINKCH		cffield		TCD{__NS}_BITER,9,4 ;   Link Channel Number
TCD{__NS}_CSR	equ		TCD{__NS}+30	;  Control and Status (16b)
BWC		cffield		TCD{__NS}_CSR,14,2 ;   Bandwidth Control
MAJOR_LINKCH	cffield		TCD{__NS}_CSR,8,4 ;   Link Channel Number
DONE		cfbit		TCD{__NS}_CSR,7 ;   Channel Done
ACTIVE		cfbit		TCD{__NS}_CSR,6 ;   Channel Active
MAJOR_E_LINK	cfbit		TCD{__NS}_CSR,5 ;   Enable Channel-to-Channel Linking
E_SG		cfbit		TCD{__NS}_CSR,4 ;   Enable Scatter/Gather Processing
D_REQ		cfbit		TCD{__NS}_CSR,3 ;   Disable Request
INT_HALF	cfbit		TCD{__NS}_CSR,2 ;   Enable an interrupt when major counter is half complete
INT_MAJOR	cfbit		TCD{__NS}_CSR,1 ;   Enable an interrupt when major iteration count completes
START		cfbit		TCD{__NS}_CSR,0 ;   Channel Start
__N		set		__N+1
		endm

		restore				; re-enable listing

                endif                           ; __52xxedmainc
