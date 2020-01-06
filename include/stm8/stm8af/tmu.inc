		ifndef	__stm8aftmuinc	; avoid multiple inclusion
__stm8aftmuinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TMU.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8AF Temp. Memory Unprotection  *                        *
;*                                                                          *
;****************************************************************************

__deftmu	macro	Base
TMU_K1		label	Base+$00	; Key Register 1
TMU_K2		label	Base+$01	; Key Register 2
TMU_K3		label	Base+$02	; Key Register 3
TMU_K4		label	Base+$03	; Key Register 4
TMU_K5		label	Base+$04	; Key Register 5
TMU_K6		label	Base+$05	; Key Register 6
TMU_K7		label	Base+$06	; Key Register 7
TMU_K8		label	Base+$07	; Key Register 8
TMU_CSR		label	Base+$08	; Control/Status Register
		endm

		restore
		endif			; __stm8aftmuinc
