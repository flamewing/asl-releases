		ifndef	__52xxuartinc		; avoid multiple inclusion
__52xxuartinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXUART.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx UART             *
;*                                                                          *
;****************************************************************************

__defuart	macro		n,Base
UMR1{n}		equ		Base+0		; Mode Register 1 (8b)
RXRTS		cfbit		UMR1{n},7	;  Receiver Request-to-Send Control
RXIRQ		cfbit		UMR1{n},6	;  Receiver Interrupt Select
ERR		cfbit		UMR1{n},5	;  Error Mode
PM		cffield		UMR1{n},3,2	;  Parity Mode
PT		cfbit		UMR1{n},2	;  Parity Type
BC		cffield		UMR1{n},0,2	;  Bits per Character
UMR2{n}		equ		Base+0		; Mode Register 2 (8b)
CM		cffield		UMR2{n},6,2	;  Channel Mode
TXRTS		cfbit		UMR2{n},5	;  Transmitter Ready-to-Send
TXCTS		cfbit		UMR2{n},4	;  Transmitter Clear-to-Send
SB		cffield		UMR2{n},0,4	;  Stop-Bit Length Control
USR{n}		equ		Base+4		; Status Register (8b)
RB		cfbit		USR{n},7	;  Received Break
FE		cfbit		USR{n},6	;  Framing Error
PE		cfbit		USR{n},5	;  Parity Error
OE		cfbit		USR{n},4	;  Overrun Error
TXEMP		cfbit		USR{n},3	;  Transmitter Empty
TXRDY		cfbit		USR{n},2	;  Transmitter Ready
FFULL		cfbit		USR{n},1	;  FIFO Full
RXRDY		cfbit		USR{n},0	;  Receiver Ready
UCSR{n}		equ		Base+4		; Clock-Select Register (8b)
RCS		cffield		UCSR{n},4,4	;  Receiver Clock Select
TCS		cffield		UCSR{n},0,4	;  Transmitter Clock Select
UCR{n}		equ		Base+8		; Command Register (8b)
MISC		cffield		UCR{n},4,3	;  Miscellaneous Commands
TC		cffield		UCR{n},2,2	;  Transmitter Commands
RC		cffield		UCR{n},0,2	;  Receiver Commands
URB{n}		equ		Base+$c		; Receiver Buffer (8b)
UTB{n}		equ		Base+$c		; Transmitter Buffer (8b)
UIPCR{n}	equ		Base+$10	; Input Port Change Register (8b)
COS		cfbit		UIPCR{n},4	;  Change-of-State
CTS		cfbit		UIPCR{n},0	;  Current State
UACR{n}		equ		Base+$10	; Auxiliary Control Register (8b)
IEC		cfbit		UACR{n},0	;  Input Enable Control
UISR{n}		equ		Base+$14	; Interrupt Status Register (8b)
COS		cfbit		UISR{n},7	;  Change-of-State
DB		cfbit		UISR{n},2	;  Delta Break
RXRDY		cfbit		UISR{n},1	;  Receiver Ready or FIFO Full
FFULL		cfbit		UISR{n},1
TXRDY		cfbit		UISR{n},0	;  Transmitter Ready
UIMR{n}		equ		Base+$14	; Interrupt Mask Register (8b)
COS		cfbit		UIMR{n},7	;  Change-of-State
DB		cfbit		UIMR{n},2	;  Delta Break
FFULL		cfbit		UIMR{n},1	;  Receiver Ready or FIFO Full
RXRDY		cfbit		UIMR{n},1
TXRDY		cfbit		UIMR{n},0	;  Transmitter Ready
UBG1{n}		equ		Base+$18	; Baud Rate Generator Prescale MSB (8b)
UDU{n}		equ		Base+$18	;  alternate name
UBG2{n}		equ		Base+$1c	; Baud Rate Generator Prescale LSB (8b)
UDL{n}		equ		Base+$1c	;  alternate name
UIVR{n}		equ		Base+$30	; Interrupt Vector Register (8b, not on all variants)
UIP{n}		equ		Base+$34	; Input Port Register (8b)
CTS		cfbit		UIP{n},0	;  CTS Current State
UOP1{n}		equ		Base+$38	; Output Port Bit Set CMD (8b)
RTS		cfbit		UOP1{n},0	;  set RTS
UOP0{n}		equ		Base+$3c	; Output Port Bit Reset CMD (8b)	
RTS		cfbit		UOP0{n},0	;  reset RTS
		endm

		restore				; re-enable listing

                endif                           ; __52xxuartinc
