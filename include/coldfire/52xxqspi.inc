		ifndef	__52xxqspiinc		; avoid multiple inclusion
__52xxqspiinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXQSPI.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Queued SPI       *
;*                                                                          *
;****************************************************************************

QMR		equ		MBAR_QSPI+0	; Mode Register (16b)
MSTR		cfbit		QMR,15		;  Master Mode Enable
BITS		cffield		QMR,10,4	;  Transfer Size
CPOL		cfbit		QMR,9		;  Clock Polarity
CPHA		cfbit		QMR,8		;  Clock Phase
BAUD		cffield		QMR,0,8		;  Baud Rate Divider
QDLYR		equ		MBAR_QSPI+4	; Delay Register (16b)
SPE		cfbit		QDLYR,15	;  QSPI Enable
QCD		cffield		QDLYR,8,7	;  QSPI_CLK Delay
DTL		cffield		QDLYR,0,8	;  Delay After Transfer
QWR		equ		MBAR_QSPI+8	; Wrap Register (16b)
HALT		cfbit		QWR,15		;  Halt Transfers
WREN		cfbit		QWR,14		;  Wraparound Enable
WRTO		cfbit		QWR,13		;  Wraparound Location
CSIV		cfbit		QWR,12		;  QSPI_CS Inactive Level
ENDQP		cffield		QWR,8,4		;  End of Queue Pointer
CPTQP		cffield		QWR,4,4		;  Completed Queue Entry Pointer
NEWQP		cffield		QWR,0,4		;  Start of Queue Pointer
QIR		equ		MBAR_QSPI+12	; Interrupt Register (16b)
WCEFB		cfbit		QIR,15		;  Write Collision Access Error Enable
ABRTB		cfbit		QIR,14		;  Abort Access Error Enable
ABRTL		cfbit		QIR,12		;  Abort Lock-Out
WCEFE		cfbit		QIR,11		;  Write Collision
ABRTE		cfbit		QIR,10		;  Abort (ABRT) Interrupt Enable
SPIFE		cfbit		QIR,8		;  QSPI Finished (SPIF) Interrupt Enable
WCEF		cfbit		QIR,3		;  Write Collision Error Flag
ABRT		cfbit		QIR,2		;  Abort Flag
SPIF		cfbit		QIR,0		;  QSPI Finished Flag
QAR		equ		MBAR_QSPI+16	; Address Register (16b)
ADDR		cffield		QAR,0,6		;  Address used to read/write the QSPI RAM
QDR		equ		MBAR_QSPI+20	; Data Register (16b)

		restore				; re-enable listing

                endif                           ; __52xxqspiinc
