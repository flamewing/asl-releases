		ifndef	__stm8cpuregsinc	; avoid multiple inclusion
__stm8cpuregsinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CPUREGS.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for Memory Mapping of STM8 CPU Regs   *
;*                                                                          *
;****************************************************************************

__defcpuregs 	macro	Base
REG_A		label	Base+$00	; Register A
REG_PCE		label	Base+$01	; Register PCE
REG_PCH		label	Base+$02	; Register PCH
REG_PCL		label	Base+$03	; Register PCL
REG_XH		label	Base+$04	; Register XH
REG_XL		label	Base+$05	; Register XL
REG_YH		label	Base+$06	; Register YH
REG_YL		label	Base+$07	; Register YL
REG_SPH		label	Base+$08	; Register SPH
REG_SPL		label	Base+$09	; Register SPL
REG_CCR		label	Base+$0a	; Register CCR
CFG_CGR		label	Base+$60	; Global configuration register
		endm

		restore
		endif			; __stm8cpuregsinc
