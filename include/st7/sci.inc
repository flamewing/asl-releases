		ifndef	__st7sciinc	; avoid multiple inclusion
__st7sciinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SCI.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 SCI                           *
;*                                                                          *
;****************************************************************************

SCISR		label	$0050		; SCI Status Register
TDRE		bit	SCISR,7		;  Transmit data register empty
TC		bit	SCISR,6		;  Transmission complete
RDRF		bit	SCISR,5		;  Received data ready flag
IDLE		bit	SCISR,4		;  Idle line detect
OR		bit	SCISR,3		;  Overrun error
NF		bit	SCISR,2		;  Noise flag
FE		bit	SCISR,1		;  Framing error
SCIDR		label	$0051		; SCI Data Register
SCIBRR		label	$0052		; SCI Baud Rate Register
SCP		bfield	SCIBRR,6,2	;  First SCI Prescaler [1:0]
SCT		bfield	SCIBRR,3,3	;  SCI Transmitter rate divisor [2:0]
SCR		bfield	SCIBRR,0,3	;  SCI Receiver rate divisor [2:0]
SCICR1		label	$0053		; SCI Control Register 1
R8		bit	SCICR1,7	;  Receive data bit 8
T8		bit	SCICR1,6	;  Transmit data bit 8
M		bit	SCICR1,4	;  Word length
WAKE		bit	SCICR1,3	;  Wake-Up method
SCICR2		label	$0054		; SCI Control Register 2
TIE		bit	SCICR2,7	;  Transmitter interrupt enable
TCIE		bit	SCICR2,6	;  Transmission complete interrupt enable
RIE		bit	SCICR2,5	;  Receiver interrupt enable
ILIE		bit	SCICR2,4	;  Idle line interrupt enable
TE		bit	SCICR2,3	;  Transmitter enable
RE		bit	SCICR2,2	;  Receiver enable
RWU		bit	SCICR2,1	;  Receiver wake-up
SBK		bit	SCICR2,0	;  Send break
SCIERPR		label	$0055		; SCI Extended Receive Prescaler Register
SCIETPR		label	$0057		; SCI Extended Transmit Prescaler Register

		restore
		endif			; __st7sciinc
