		ifndef	__stm8lcompinc	; avoid multiple inclusion
__stm8lcompinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File COMP.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Comparator(s)               *
;*                                                                          *
;****************************************************************************

__defcomp	macro	Base
COMP_CSR1	label	Base+$00	; Comparator control and status register 1
COMP_IE1	bit	COMP_CSR1,5	;  Comparator 1 interrupt enable
COMP_EF1	bit	COMP_CSR1,4	;  Comparator 1 event flag
COMP_CMP1OUT	bit	COMP_CSR1,3	;  Comparator 1 output
COMP_STE	bit	COMP_CSR1,2	;  Schmitt trigger enable
COMP_CMP1	bfield	COMP_CSR1,0,2	;  Comparator 1 configuration
COMP_CSR2	label	Base+$01	; Comparator control and status register 2
COMP_IE2	bit	COMP_CSR2,5	;  Comparator 2 Interrupt Enable
COMP_EF2	bit	COMP_CSR2,4	;  Comparator 2 event flag
COMP_CMP2OUT	bit	COMP_CSR2,3	;  Comparator 2 output
COMP_SPEED	bit	COMP_CSR2,2	;  Comparator 2 speed mode
COMP_CMP2	bfield	COMP_CSR2,0,2	;  Comparator 2 configuration
COMP_CSR3	label	Base+$02	; Comparator control and status register 3
COMP_OUTSEL	bfield	COMP_CSR3,6,2	;  Comparator 2 output selection
COMP_INSEL	bfield	COMP_CSR3,3,3	;  Comparator 2 inverting input selection
COMP_VREFEN	bit	COMP_CSR3,2	;  Internal reference voltage VREFINT enable
COMP_WNDWE	bit	COMP_CSR3,1	;  Window mode enable
COMP_VREFOUTEN	bit	COMP_CSR3,0	;  VREFINT output enable
COMP_CSR4	label	Base+$03	; Comparator control and status register 4
COMP_NINVTRIG	bfield	COMP_CSR4,3,3	;  COMP2 non inverting input
COMP_INVTRIG	bfield	COMP_CSR4,0,3	;  COMP2 inverting input
COMP_CSR5	label	Base+$04	; Comparator control and status register 5
COMP_DACTRIG	bfield	COMP_CSR5,3,3	;  DAC outputs
COMP_VREFTRIG	bfield	COMP_CSR5,0,3	;  VREFINT outputs
		endm

		restore
		endif			; __stm8lcompinc
