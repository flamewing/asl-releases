		ifndef	__52xxi2cinc		; avoid multiple inclusion
__52xxi2cinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXI2C.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx I2C Interface    *
;*                                                                          *
;****************************************************************************

I2ADR		equ		MBAR_I2C+0	; Address Register (8b)
IADR		equ		I2ADR
ADR		cffield		I2ADR,1,7	;  Slave Address
I2FDR		equ		MBAR_I2C+4	; Frequency Divider Register (8b)
IFDR		equ		I2FDR
IC		cffield		I2ADR,0,6	;  I2C Clock Rate
I2CR		equ		MBAR_I2C+8	; Control Register (8b)
IEN		cfbit		I2CR,7		;  I2C Enable
IIEN		cfbit		I2CR,6		;  I2C Interrupt Enable
MSTA		cfbit		I2CR,5		;  Master/Slave Mode Select Bit
MTX		cfbit		I2CR,4		;  Transmit/Receive Mode Select Bit
TXAK		cfbit		I2CR,3		;  Transmit Acknowledge Enable
RSTA		cfbit		I2CR,2		;  Repeat Start
I2SR		equ		MBAR_I2C+12	; Status Register (8b)
ICF		cfbit		I2SR,7		;  I2C Data Transferring Bit
IAAS		cfbit		I2SR,6		;  I2C Addressed as a Slave Bit
IBB		cfbit		I2SR,5		;  I2C Bus Busy Bit
IAL		cfbit		I2SR,4		;  I2C Arbitration Lost
SRW		cfbit		I2SR,2		;  Slave Read/Write
IIF		cfbit		I2SR,1		;  I2C Interrupt
RXAK		cfbit		I2SR,0		;  Received Acknowledge
I2DR		equ		MBAR_I2C+16	; Data I/O Register (8b)

		restore				; re-enable listing

                endif                           ; __52xxi2cinc
