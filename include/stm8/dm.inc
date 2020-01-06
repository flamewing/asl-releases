		ifndef	__stm8sdminc	; avoid multiple inclusion
__stm8sdminc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File DM.INC                                                  *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Debug Module                *
;*                                                                          *
;****************************************************************************

__defdm		macro	Base
DM_BK1RE	label	Base+$00	; DM breakpoint 1 register extended byte
DM_BK1RH	label	Base+$01	; DM breakpoint 1 register high byte
DM_BK1RL	label	Base+$02	; DM breakpoint 1 register low byte
DM_BK2RE	label	Base+$03	; DM breakpoint 2 register extended byte
DM_BK2RH	label	Base+$04	; DM breakpoint 2 register high byte
DM_BK2RL	label	Base+$05	; DM breakpoint 2 register low byte
DM_CR1		label	Base+$06	; DM debug module control register 1
DM_CR2		label	Base+$07	; DM debug module control register 2
DM_CSR1		label	Base+$08	; DM debug module control/status register 1
DM_CSR2		label	Base+$09	; DM debug module control/status register 2
DM_ENFCTR	label	Base+$0a	; DM enable function register
		endm

		restore
		endif			; __stm8sdminc
