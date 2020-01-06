		ifndef	__regfox2inc	; avoid multiple inclusion
__regfox2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGFOX2.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7FOXx2                          *
;*                                                                          *
;*   Source: ST7FOXF1/K1/K2 Data Sheet, Rev. 4, February 2008               *
;*                                                                          *
;****************************************************************************

		include	"regfox.inc"

;----------------------------------------------------------------------------
; Interrupt Vectors

LT_vect		label	$ffe0		; Lite Timer Interrupt
TIMA_vect	label	$ffe2		; 16-bit Timer Interrupt
SPI_vect	label	$ffe4		; SPI Interrupt
I2C_vect	label	$ffe6		; I2C Interrupt
AT_OF2_vect	label	$ffe8		; AT Timer Overflow 2 Interrupt
AT_OF1_vect	label	$ffea		; AT Timer Overflow 1 Interrupt
AT_OC_vect	label	$ffec		; AT Timer Output Compare Interrupt, shared with...
AT_IC_vect	label	$ffec		; AT Timer Input Capture Interrupt
EI2_vect	label	$ffee		; External Interrupt Vector EI2
EI1_vect	label	$fff0		; External Interrupt Vector EI1
EI0_vect	label	$fff2		; External Interrupt Vector EI0
AWU_vect	label	$fffa		; Auto Wakeup Interrupt Vector
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; Auto-Reload Timer

BR1SEL		bit	BREAKCR1,7	;  Break 1 Input Selection
BR1EDGE		bit	BREAKCR1,6	;  Break 1 Input Edge Selection Bit
BA1		bit	BREAKCR1,5	;  Break 1 Active Bit
BP1EN		bit	BREAKCR1,4	;  Break 1 Pin Enable Bit
BREAKCR2	label	$002c		; Break Control Register 2
BR2SEL		bit	BREAKCR2,7	;  Break 2 Input Selection
BR2EDGE		bit	BREAKCR2,6	;  Break 2 Input Edge Selection Bit
BA2		bit	BREAKCR2,5	;  Break 2 Active Bit
BP2EN		bit	BREAKCR2,4	;  Break 2 Pin Enable Bit
SWBR2		bit	BREAKCR2,1	;  Switch Break For Counter 2 Bit
SWBR1		bit	BREAKCR2,0	;  Switch Break For Counter 2 Bit

;----------------------------------------------------------------------------
; Timer A

		include	"timer.inc"
		__deftimer "TA",$0054

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0070

		restore
		endif			; __regfox2inc
