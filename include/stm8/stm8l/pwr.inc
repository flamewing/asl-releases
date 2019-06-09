		ifndef	__stm8lpwrinc	; avoid multiple inclusion
__stm8lpwrinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PWR.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Power Management            *
;*                                                                          *
;****************************************************************************

__defpwr	macro	Base
PWR_CSR1	label	Base+$00	; Power control and status register 1
PVDOF		bit	PWR_CSR1,6	;  PVD output flag
PVDIF		bit	PWR_CSR1,5	;  PVD interrupt flag
PVDIEN		bit	PWR_CSR1,4	;  PVD interrupt enable
PLS		bfield	PWR_CSR1,1,3	;  PVD level selection
PVDE		bit	PWR_CSR1,0	;  Power voltage detector (PVD) enable
PWR_CSR2	label	Base+$01	; Power control and status register 2
FWU		bit	PWR_CSR2,2	;  Fast wake up configuration
ULP		bit	PWR_CSR2,1	;  Ultra-low-power configuration
VREFINTF	bit	PWR_CSR2,0	;  Internal reference voltage status flag
		endm

		restore
		endif			; __stm8lpwrinc
