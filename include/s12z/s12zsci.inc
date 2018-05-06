		ifndef	__s12z_sci_inc
__s12z_sci_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZSCI.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Serial Communications Ifc *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZSCI		struct	dots
		union			; union works desired way because @ offset 0!
		struct
BD		ds.w	0		; ($00) SCI Baud Rate Register (16 bit)
BDH		ds.b	1		; ($00) SCI Baud Rate Register MSB
BDL		ds.b	1		; ($01) SCI Baud Rate Register LSB
CR1		ds.b	1		; ($02) SCI Control Register 1
LOOPS		s12zbit	CR1,7		;  Loop Select Bit
SCISWAI		s12zbit	CR1,6		;  SCI Stop in Wait Mode Bit
RSRC		s12zbit	CR1,5		;  Receiver Source Bit
M		s12zbit	CR1,4		;  Data Format Mode Bit
WAKE		s12zbit	CR1,3		;  Wakeup Condition Bit
ILT		s12zbit	CR1,2		;  Idle Line Type Bit
PE		s12zbit	CR1,1		;  Parity Enable Bit
PT		s12zbit	CR1,0		;  Parity Type Bit
		ends
		struct
ASR1		ds.b	1		; ($00) SCI Alternative Status Register 1
RXEDGIF		s12zbit	ASR1,7		;  Receive Input Active Edge Interrupt Flag
BERRV		s12zbit	ASR1,2		;  Bit Error Value
BERRIF		s12zbit	ASR1,1		;  Bit Error Interrupt Flag
BKDIF		s12zbit	ASR1,0		;  Break Detect Interrupt Flag
ACR1		ds.b	1		; ($01) SCI Alternative Control Register 1
RXEDGIE		s12zbit	ACR1,7		;  Receive Input Active Edge Interrupt Enable
BERRIE		s12zbit	ACR1,1		;  Bit Error Interrupt Enable
BKDIE		s12zbit	ACR1,0		;  Break Detect Interrupt Enable
ACR2		ds.b	1		; ($02) SCI Alternative Control Register 2
IREN		s12zbit	ACR2,7		;  Infrared Enable Bit
TNP		s12zfld	ACR2,2:5	;  Transmitter Narrow Pulse Bits
BERRM		s12zfld	ACR2,2:1	;  Bit Error Mode
BKDFE		s12zbit	ACR2,0		;  Break Detect Feature Enable
		ends
		endunion
CR2		ds.b	1		; ($03) SCI Control Register 2
TIE		s12zbit	CR2,7		;  Transmitter Interrupt Enable Bit
TCIE		s12zbit	CR2,6		;  Transmission Complete Interrupt Enable Bit
RIE		s12zbit	CR2,5		;  Receiver Full Interrupt Enable Bit
ILIE		s12zbit	CR2,4		;  Idle Line Interrupt Enable Bit
TE		s12zbit	CR2,3		;  Transmitter Enable Bit
RE		s12zbit	CR2,2		;  Receiver Enable Bit
RWU		s12zbit	CR2,1		;  Receiver Wakeup Bit
SBK		s12zbit	CR2,0		;  Send Break Bit
SR1		ds.b	1		; ($04) SCI Status Register 1
TDRE		s12zbit	SR1,7		;  Transmit Data Register Empty Flag
TC		s12zbit	SR1,6		;  Transmit Complete Flag
RDRF		s12zbit	SR1,5		;  Receive Data Register Full Flag
IDLE		s12zbit	SR1,4		;  Idle Line Flag
OR		s12zbit	SR1,3		;  Overrun Flag
NF		s12zbit	SR1,2		;  Noise Flag
FE		s12zbit	SR1,1		;  Framing Error Flag
PF		s12zbit	SR1,0		;  Parity Error Flag
SR2		ds.b	1		; ($05) SCI Status Register 2
AMAP		s12zbit	SR2,7		;  Alternative Map
TXPOL		s12zbit	SR2,4		;  Transmit Polarity
RXPOL		s12zbit	SR2,3		;  Receive Polarity
BRK13		s12zbit	SR2,2		;  Break Transmit Character Length
TXDIR		s12zbit	SR2,1		;  Transmitter Pin Data Direction in Single-Wire Mode
RAF		s12zbit	SR2,0		;  Receiver Active Flag
DR		ds.w	0		; ($06) SCI Data Register (16 bits)
DRH		ds.b	1		; ($06) SCI Data Register MSB
DRL		ds.b	1		; ($07) SCI Data Register LSB
		ends	S12ZSCI

		restore			; re-enable listing

		endif			; __s12z_sci_inc
