		ifndef	__52xxtmrinc		; avoid multiple inclusion
__52xxtmrinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXUART.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Timer            *
;*                                                                          *
;****************************************************************************

__deftimer	macro		n,Base
TMR{n}		equ		Base+$00	; Timer Mode Register (16b)
PS		cffield		TMR{n},8,8	;  Prescaler Value
CE		cffield		TMR{n},6,2	;  Capture Edge and Enable Interrupt
OM		cfbit		TMR{n},5	;  Output Mode
ORI		cfbit		TMR{n},4	;  Output Reference Interrupt Enable
FRR		cfbit		TMR{n},3	;  Free Run/Restart
ICLK		cffield		TMR{n},1,2	;  Input Clock Source for the Timer
RST		cfbit		TMR{n},0	;  Reset Timer
TRR{n} 		equ		Base+$04	; Timer Reference Register (16b)
TCR{n} 		equ		Base+$08	; Timer Capture Register (16b)
TCN{n} 		equ		Base+$0c	; Timer Counter (16b)
TER{n} 		equ		Base+$11	; Timer Event Register (8b)
REF		cfbit		TER{n},1	;  Capture Event
CAP		cfbit		TER{n},0	;  Output Reference Event
		endm

		restore				; re-enable listing

                endif                           ; __52xxtmrinc
