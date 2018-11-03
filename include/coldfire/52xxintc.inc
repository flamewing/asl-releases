		ifndef	__52xxintcinc		; avoid multiple inclusion
__52xxintcinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXINTC.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Interrupt        *
;*   Controller(s)                                                          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Interrupt Controller 0

		ifdef		MBAR_INTC0
IPRH0		equ		MBAR_INTC0+$00	; Interrupt Pending Register High (32b)
IPRL0		equ		MBAR_INTC0+$04	; Interrupt Pending Register Low (32b)
IMRH0		equ		MBAR_INTC0+$08	; Interrupt Mask Register High (32b)
IMRL0		equ		MBAR_INTC0+$0c	; Interrupt Mask Register Low (32b)
INTFRCH0	equ		MBAR_INTC0+$10	; Interrupt Force Register High (32b)
INTFRCL0	equ		MBAR_INTC0+$14	; Interrupt Force Register Low (32b)
ICONFIG		equ		MBAR_INTC0+$1a	; Interrupt Configuration Register (16b)
ELVLPRI		cffield		ICONFIG,9,7	;  Enable core's priority elevation on priority levels.
EMASK		cfbit		ICONFIG,5	;  Automatically load the level of an interrupt request into the CLMASK when the acknowledge is performed.
SIMR0		equ		MBAR_INTC0+$1c	; Set Interrupt Mask (8b)
SALL		cfbit		SIMR0,6		;  Set all bits in the IMR0 register
SIMR		cffield		SIMR0,0,6	;  Set the corresponding bit in the IMR0 register
CIMR0		equ		MBAR_INTC0+$1d	; Clear Interrupt Mask (8b)
CALL		cfbit		CIMR0,6		;  Clear all bits in the IMR0 register
CIMR		cffield		CIMR0,0,6	;  Clear the corresponding bit in the IMR0 register
CLMASK		equ		MBAR_INTC0+$1e	; Current Level Mask (8b)
SLMASK		equ		MBAR_INTC0+$1f	; Saved Level Mask (8b)
__N		set		0
		rept		64
		__decstr	__NS,__N
ICR0{"\{__NS}"}	equ		MBAR_INTC0+$040+__N; Interrupt Control Register N (8b)
LEVEL		cffield		ICR0{"\{__NS}"},0,3 ;  Interrupt level.
__N		set		__N+1
		endm
SWIACK0		equ		MBAR_INTC0+$e0	; Software Interrupt Acknowledge (8b)
__N		set		1
		rept		6
		__decstr	__NS,__N
L{"\{__NS}"}IACK0	equ	MBAR_INTC0+$e0+(4*__N) ; Interrupt Acknowledge Register N (8b)
__N		set		__N+1
		endm
		endif				; ifdef MBAR_INTC0

;----------------------------------------------------------------------------
; Interrupt Controller 1

		ifdef		MBAR_INTC1
IPRH1		equ		MBAR_INTC1+$00	; Interrupt Pending Register High (32b)
IPRL1		equ		MBAR_INTC1+$04	; Interrupt Pending Register Low (32b)
IMRH1		equ		MBAR_INTC1+$08	; Interrupt Mask Register High (32b)
IMRL1		equ		MBAR_INTC1+$0c	; Interrupt Mask Register Low (32b)
INTFRCH1	equ		MBAR_INTC1+$10	; Interrupt Force Register High (32b)
INTFRCL1	equ		MBAR_INTC1+$14	; Interrupt Force Register Low (32b)
SIMR1		equ		MBAR_INTC1+$1c	; Set Interrupt Mask (8b)
SALL		cfbit		SIMR1,6		;  Set all bits in the IMR1 register
SIMR		cffield		SIMR1,0,6	;  Set the corresponding bit in the IMR1 register
CIMR1		equ		MBAR_INTC1+$1d	; Clear Interrupt Mask (8b)
CALL		cfbit		CIMR1,6		;  Clear all bits in the IMR1 register
CIMR		cffield		CIMR1,0,6	;  Clear the corresponding bit in the IMR1 register
__N		set		1
		rept		63
		__decstr	__NS,__N
ICR1{"\{__NS}"}	equ		MBAR_INTC1+$040+__N; Interrupt Control Register N (8b)
LEVEL		cffield		ICR1{"\{__NS}"},0,3 ;  Interrupt level.
__N		set		__N+1
		endm
SWIACK1		equ		MBAR_INTC1+$e0	; Software Interrupt Acknowledge (8b)
__N		set		1
		rept		7
		__decstr	__NS,__N
L{"\{__NS}"}IACK1	equ	MBAR_INTC1+$e0+(4*__N) ; Interrupt Acknowledge Register N (8b)
__N		set		__N+1
		endm
		endif				; ifdef MBAR_INTC1

;----------------------------------------------------------------------------
; Global IACK Registers

		ifdef		MBAR_INTC
GSWIACK		equ		MBAR_INTC+$e0	; Global Software Interrupt Acknowledge (8b)
__N		set		1
		rept		7
		__decstr	__NS,__N
GL{"\{__NS}"}IACK	equ	MBAR_INTC+$e0+(4*__N) ; Interrupt Acknowledge Register N (8b)
__N		set		__N+1
		endm
		endif				; ifdef MBAR_INTC

;----------------------------------------------------------------------------

		restore				; re-enable listing

                endif                           ; __52xxintcinc
