		ifndef	__stm8swwdginc	; avoid multiple inclusion
__stm8swwdginc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WWDG.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Window Watchdog             *
;*                                                                          *
;****************************************************************************

__defwwdg	macro	Base
WWDG_CR		label	Base+$00	; WWDG control register
WWDG_WDGA	bit	WWDG_CR,7	;  Activation bit
WWDG_T		bfield	WWDG_CR,0,7	;  7-bit counter
WWDG_WR		label	Base+$01	; WWDR window register
WWDG_W		bfield	WWDG_WR,0,7	;  7-bit window value
		endm

		restore
		endif			; __stm8swwdginc
