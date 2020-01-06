		ifndef	__st7pwmartinc	; avoid multiple inclusion
__st7pwmartinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PWM_ART.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx PWM Auto Reload Timer     *
;*                                                                          *
;****************************************************************************

__defpwmart	macro	Base
PWMDCR3		label	Base+$00	; PWM AR Timer Duty Cycle Register 3
PWMDCR2		label	Base+$01	; PWM AR Timer Duty Cycle Register 2
PWMDCR1		label	Base+$02	; PWM AR Timer Duty Cycle Register 1
PWMDCR0		label	Base+$03	; PWM AR Timer Duty Cycle Register 0
PWMCR		label	Base+$04	; PWM AR Timer Control Register
PWM_OE3		bit	PWMCR,7		; Output Enable
PWM_OE2		bit	PWMCR,6
PWM_OE1		bit	PWMCR,5
PWM_OE0		bit	PWMCR,4
OP3		bit	PWMCR,3		; Output Polarity
OP2		bit	PWMCR,2
OP1		bit	PWMCR,1
OP0		bit	PWMCR,0
ARTCSR		label	Base+$05	; Auto-Reload Timer Control/Status Register
EXCL		bit	ARTCSR,7	;  External Clock
CC		bfield	ARTCSR,4,3	;  Counter Clock Control
TCE		bit	ARTCSR,3	;  Timer Counter Enable
FCRL		bit	ARTCSR,2	;  Force Counter Re-Load
PWM_OIE		bit	ARTCSR,1	;  Overflow Interrupt Enable
OVF		bit	ARTCSR,0	;  Overflow Flag
ARTCAR		label	Base+$06	; Auto-Reload Timer Counter Access Register
ARTARR		label	Base+$07	; Auto-Reload Timer Auto-Reload Register
ARTICCSR	label	Base+$08	; AR Timer Input Capture Control/Status Register
PWM_CS		bfield	ARTICCSR,4,2	;  Capture Sensitivity
PWM_CIE2	bit	ARTICCSR,3	;  Capture Interrupt Enable
PWM_CIE1	bit	ARTICCSR,2
PWM_CF2		bit	ARTICCSR,1	;  Capture Flag
PWM_CF1		bit	ARTICCSR,0
ARTICR1		label	Base+$09	; AR Timer Input Capture Register 1
ARTICR2		label	Base+$0a	; AR Timer Input Capture Register 2
		endm

		restore
		endif			; __st7pwmartinc
