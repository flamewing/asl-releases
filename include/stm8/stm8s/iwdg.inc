		ifndef	__stm8siwdginc	; avoid multiple inclusion
__stm8siwdginc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File IWDG.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Independent Watchdog        *
;*                                                                          *
;****************************************************************************

__defiwdg	macro	Base
IWDG_KR		label	Base+$00	; IWDG key register
IWDG_KEY	bfield	IWDG_KR,0,8	;  Key value
IWDG_PR		label	Base+$01	; IWDG prescaler register
IWDG__PR	bfield	IWDG_PR,0,3	;  Prescaler divider
IWDG_RLR	label	Base+$02	; IWDG reload register
IWDG_RL		bfield	IWDG_RLR,0,8	;  Watchdog counter reload value
		endm

		restore
		endif			; __stm8siwdginc
