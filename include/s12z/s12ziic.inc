		ifndef	__s12z_iic_inc
__s12z_iic_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZIIC.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Inter-IC Interface        *
;*                                                                          *
;****************************************************************************

IBAD		equ	$07C0		; IIC Address Register (1..7)
IBFD		equ	$07C1		; IIC Frequency Divider Register
IBCR		equ	$07C2		; IIC Control Register
IBEN		s12zbit	IBCR,7		;  I-Bus Enable
IBIE		s12zbit	IBCR,6		;  I-Bus Interrupt Enable
MS_SL		s12zbit	IBCR,5		;  Master/Slave Mode Select Bit
Tx_Rx		s12zbit	IBCR,4		;  Transmit/Receive Mode Select Bit
TXAK		s12zbit	IBCR,3		;  Transmit Acknowledge Enable
RSTA		s12zbit	IBCR,2		;  Repeat Start
IBSWAI		s12zbit	IBCR,0		;  I Bus Interface Stop in Wait Mode
IBSR		equ	$07C3		; IIC Status Register
TCF		s12zbit	IBSR,7		;  Data Transferring Bit
IAAS		s12zbit	IBSR,6		;  Addressed as a Slave Bit
IBB		s12zbit	IBSR,5		;  Bus Busy Bit
IBAL		s12zbit	IBSR,4		;  Arbitration Lost
SRW		s12zbit	IBSR,2		;  Slave Read/Write
IBIF		s12zbit	IBSR,1		;  I-Bus Interrupt
RXAK		s12zbit	IBSR,0		;  Received Acknowledge
IBDR		equ	$07C4		; IIC Data I/O Register
IBCR2		equ	$07C5		; IIC Control Register 2
GCEN		s12zbit	IBCR2,7		;  General Call Enable
ADTYPE		s12zbit	IBCR2,6		;  Address Type
ADRHI		s12zfld	IBCR2,3:0	;  Slave Address MSB

		restore			; re-enable listing

		endif			; __s12z_iic_inc
