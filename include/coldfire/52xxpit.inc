		ifndef	__52xxpitinc		; avoid multiple inclusion
__52xxpitinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXPIT.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Programmable     *
;*   Interrupt Timer                                                        *
;*                                                                          *
;****************************************************************************

__defpit	macro		n,Base
PCSR{n}		equ		Base+0		; PIT Control and Status Register (16b)
PRE		cffield		PCSR{n},8,4	;  Prescaler
DOZE		cfbit		PCSR{n},6	;  Doze Mode Bit
DBG		cfbit		PCSR{n},5	;  Debug Mode Bit
OVW		cfbit		PCSR{n},4	;  Overwrite
PIE		cfbit		PCSR{n},3	;  PIT Interrupt Enable
PIF		cfbit		PCSR{n},2	;  PIT Interrupt Flag
RLD		cfbit		PCSR{n},1	;  Reload Bit
EN		cfbit		PCSR{n},0	;  PIT Enable Bit
PMR{n}		equ		Base+2		; PIT Modulus Register (16b)
PCNTR{n}	equ		Base+4		; PIT Count Register (16b)
		endm

		restore				; re-enable listing

                endif                           ; __52xxpitinc
