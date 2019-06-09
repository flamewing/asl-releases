		ifndef	__stm8lcomp01inc	; avoid multiple inclusion
__stm8lcomp01inc equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File COMP01.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 Comprator(s)             *
;*                                                                          *
;****************************************************************************

__defcomp01	macro	Base
COMP_CR		label	Base+$00	; Comparator control register
COMP_IC1_BK	bit	COMP_CR,7	;  Input capture 1/ break selection
COMP_CNF_TIM	bfield	COMP_CR,5,2	;  Comparator 1/2 output connected to Timer 2 and 3 input capture or break
COMP_POL	bit	COMP_CR,4	;  Comparator polarity
COMP_COMPREF	bit	COMP_CR,3	;  Comparator reference
COMP_COMP2_EN	bit	COMP_CR,2	;  Second comparator enable
COMP_COMP1_EN	bit	COMP_CR,1	;  First comparator enable
COMP_BIAS_EN	bit	COMP_CR,0	;  Bias enable
COMP_CSR	label	Base+$01	; Comparator status register
COMP_ITEN2	bit	COMP_CSR,7	;  Second comparator interrupt enable
COMP_CEF2	bit	COMP_CSR,6	;  Second comparator event flag
COMP_ITEN1	bit	COMP_CSR,5	;  First comparator interrupt enable
COMP_CEF1	bit	COMP_CSR,4	;  First comparator event flag
COMP_COMP2_OUT	bit	COMP_CSR,1	;  Second comparator output
COMP_COMP1_OUT	bit	COMP_CSR,0	;  First comparator output
COMP_CCS	label	Base+$02	; Comparator channel selection register
COMP_COMP2_CH4	bit	COMP_CCS,7	;  Comparator switch enable
COMP_COMP2_CH3	bit	COMP_CCS,6
COMP_COMP2_CH2	bit	COMP_CCS,5
COMP_COMP2_CH1	bit	COMP_CCS,4
COMP_COMP1_CH4	bit	COMP_CCS,3
COMP_COMP1_CH3	bit	COMP_CCS,2
COMP_COMP1_CH2	bit	COMP_CCS,1
COMP_COMP1_CH1	bit	COMP_CCS,0
		endm

		restore
		endif			; __stm8l01compinc
