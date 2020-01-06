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

__defsci	macro	Base,ERPROffs
SCISR		label	Base+$01	; SCI Status Register
TDRE		bit	SCISR,7		;  Transmit Data register Empty
TC		bit	SCISR,6		;  Transmission Complete
RDRF		bit	SCISR,5		;  Received Data Ready Flag
IDLE		bit	SCISR,4		;  Idle Line Detect
OR		bit	SCISR,3		;  Overrun Error
NF		bit	SCISR,2		;  Noise Flag
FE		bit	SCISR,1		;  Framing Error
SCIDR		label	Base+$01	; SCI Data Register
SCIBRR		label	Base+$02	; SCI Baud Rate Register
SCP		bfield	SCIBRR,6,2	;  First SCI Prescaler [1:0]
SCT		bfield	SCIBRR,3,3	;  SCI Transmitter Rate Divisor [2:0]
SCR		bfield	SCIBRR,0,3	;  SCI Receiver Rate Divisor [2:0]
SCICR1		label	Base+$03	; SCI Control Register 1
R8		bit	SCICR1,7	;  Receive Data Bit 8
T8		bit	SCICR1,6	;  Transmit Data Bit 8
M		bit	SCICR1,4	;  Word Length
WAKE		bit	SCICR1,3	;  Wake-Up Method
SCICR2		label	Base+$04	; SCI Control Register 2
TIE		bit	SCICR2,7	;  Transmitter Interrupt Enable
TCIE		bit	SCICR2,6	;  Transmission Complete Interrupt Enable
RIE		bit	SCICR2,5	;  Receiver Interrupt Enable
ILIE		bit	SCICR2,4	;  Idle line interrupt Enable
TE		bit	SCICR2,3	;  Transmitter Enable
RE		bit	SCICR2,2	;  Receiver Enable
RWU		bit	SCICR2,1	;  Receiver Wake-Up
SBK		bit	SCICR2,0	;  Send Break
SCIERPR		label	Base+ERPROffs	; SCI Extended Receive Prescaler Register
SCIETPR		label	Base+$07	; SCI Extended Transmit Prescaler Register
		endm

		restore
		endif			; __st7sciinc
