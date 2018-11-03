		ifndef	__52xxrnginc		; avoid multiple inclusion
__52xxrnginc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXRNG.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Random Number    *
;*   Generator                                                              *
;*                                                                          *
;****************************************************************************

RNGCR		equ		MBAR_RNG+0	; RNG Control Register (32b)
SLM		cfbit		RNGCR,4		;  Sleep mode.
CI		cfbit		RNGCR,3		;  Clear interrupt.
IM		cfbit		RNGCR,2		;  Interrupt mask.
HA		cfbit		RNGCR,1		;  High assurance.
GO		cfbit		RNGCR,0		;  Go bit.
RNGSR		equ		MBAR_RNG+4	; RNG Status Register (32b)
OFS		cffield		RNGSR,16,8	;  Output FIFO size.
OFL		cffield		RNGSR,8,8	;  Output FIFO level.
SLP		cfbit		RNGSR,4		;  Sleep.
EI		cfbit		RNGSR,3		;  Error interrupt.
FUF		cfbit		RNGSR,2		;  FIFO underflow.
LRS		cfbit		RNGSR,1		;  Last read status.
SV		cfbit		RNGSR,0		;  Security violation.
RNGER		equ		MBAR_RNG+8	; RNG Entropy Register (32b)
ENT		cffield		RNGER,0,32	;  Entropy input.
RNGOUT		equ		MBAR_RNG+12	; RNG Output FIFO (32b)

		restore				; re-enable listing

                endif                           ; __52xxrnginc
