		ifndef	__regfox1inc	; avoid multiple inclusion
__regfox1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGFOX1.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7FOXx1                          *
;*                                                                          *
;*   Source: ST7FOXF1/K1/K2 Data Sheet, Rev. 4, February 2008               *
;*                                                                          *
;****************************************************************************

		include	"regfox.inc"

;----------------------------------------------------------------------------
; Interrupt Vectors

LT_RTC2_vect	label	$ffe2		; Lite Timer RTC2 Interrupt
LT_IC_vect	label	$ffe4		; Lite Timer Input Capture Interrupt
LT_RTC_vect	label	$ffe6		; Lite Timer RTC Interrupt
I2C_vect	label	$ffe8		; I2C Interrupt
AT_OC_vect	label	$ffea		; AT Timer Output Compare Interrupt
AT_IC_vect	label	$ffec		; AT Timer Input Capture Interrupt
AT_OF1_vect	label	$ffee		; AT Timer Overflow 1 Interrupt
AT_OF2_vect	label	$fff0		; AT Timer Overflow 2 Interrupt
EI2_vect	label	$fff2		; External Interrupt Vector EI2
EI1_vect	label	$fff4		; External Interrupt Vector EI1
EI0_vect	label	$fff6		; External Interrupt Vector EI0
AWU_vect	label	$fffa		; Auto Wakeup Interrupt Vector
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; Auto-Reload Timer

BREDGE		bit	BREAKCR1,6	;  Break Input Edge Selection Bit
BA		bit	BREAKCR1,5	;  Break 1 Active Bit
BPEN		bit	BREAKCR1,4	;  Break 1 Pin Enable Bit

		restore
		endif			; __regfox1inc
