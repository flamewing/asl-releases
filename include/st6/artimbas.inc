		ifndef	__artimbasinc	; avoid multiple inclusion
__artimbasinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ARTIMBAS.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx Basic AR Timer (in 6285)   *
;*                                                                          *
;****************************************************************************

__defartimbas	macro	Base
ARMCR		sfr	Base+05h	; AR Mode Control Register
TCLD		bit	7,ARMCR		;  Timer Load Bit
TEN		bit	6,ARMCR		;  Timer Clock Enable
CPIE		bit	3,ARMCR		;  Compare Interrupt Enable
OVIE		bit	2,ARMCR		;  Overflow Interrupt
ARMC		bfield	ARMCR,0,2	;  Mode Control Bits
ARSC0		sfr	Base+06h	; AR Status Control Register 0
CPF		bit	1,ARSC0		;  Compare Interrupt Flag
OVF		bit	0,ARSC0		;  Overflow Interrupt Flag
ARSC1		sfr	Base+07h	; AR Status Control Register 1
AR_PS		bfield	ARSC1,5,3	;  Prescaler Division Selection
CC		bfield	ARSC1,0,2	;  Clock Source Select
ARLR		sfr	Base+0bh	; AR Load Register
ARRC		sfr	Base+09h	; AR Reload/Capture Register
ARCP		sfr	Base+0ah	; AR Compare Register
		endm

		restore
		endif			; __artimbasinc
