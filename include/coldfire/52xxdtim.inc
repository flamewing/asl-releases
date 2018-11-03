		ifndef	__52xxdtiminc		; avoid multiple inclusion
__52xxdtiminc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXDTIM.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx DMA Timer        *
;*                                                                          *
;****************************************************************************

__defdtim	macro		n,Base
DTMR{n}		equ		Base+0		; DMA Timer n Mode Register (16b)
PS		cffield		DTMR{n},8,8	;  Prescaler Value
CE		cffield		DTMR{n},6,2	;  Capture Edge
OM		cfbit		DTMR{n},5	;  Output Mode
ORRI		cfbit		DTMR{n},4	;  Output Reference Request, Interrupt Enable
FRR		cfbit		DTMR{n},3	;  Free Run/Restart
CLK		cffield		DTMR{n},1,2	;  Input Clock Source for the Timer
RST		cfbit		DTMR{n},0	;  Reset Timer
DTXMR{n}	equ		Base+2		; DMA Timer n Extended Mode Register (8b)
DMAEN		cfbit		DTXMR{n},7	;  DMA Request
HALTED		cfbit		DTXMR{n},6	;  Controls the counter when the core is halted (not on all models).
MODE16		cfbit		DTXMR{n},0	;  Select the Increment Mode for the Timer
DTER{n}		equ		Base+3		; DMA Timer n Event Register (8b)
REF		cfbit		DTER{n},1	;  Output Reference Event
CAP		cfbit		DTER{n},0	;  Capture Event
DTRR{n}		equ		Base+4		; DMA Timer n Reference Register (32b)
DTCR{n}		equ		Base+8		; DMA Timer n Capture Register (32b)
DTCN{n}		equ		Base+12		; DMA Timer n Counter Register (32b)
		endm

		restore				; re-enable listing

                endif                           ; __52xxdtiminc
