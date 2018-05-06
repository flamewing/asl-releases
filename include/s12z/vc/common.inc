		ifndef	__vc_inc
__vc_inc	equ	1
		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File COMMON.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for S912ZVC(A)xxx                  *
;*                                                                          *
;****************************************************************************

		include	"s12z/cpucore.inc"
		include	"s12z/s12zint.inc"
		include	"s12z/s12zmmc.inc"
		include	"s12z/s12zdbg.inc"
		
		if	__housing=64
__portadh_mask	equ	$ff		; 64-pin device: Port AD 0..15
__portadl_mask	equ	$ff
__porte_mask	equ	$03		; Port E 0..1
__portl_mask	equ	$03		; Port L 0..1
__portj_mask	equ	$03		; Port J 0..3
__portp_mask	equ	$ff		; Port P 0..7
__ports_mask	equ	$ff		; Port S 0..7
__portt_mask	equ	$ff		; Port T 0..7
		elseif
__portadh_mask	equ	$03		; 48-pin device: Port AD 0..9
__portadl_mask	equ	$ff
__porte_mask	equ	$03		; Port E 0..1
__portl_mask	equ	$03		; Port L 0..1
__portj_mask	equ	$00		; Port J n/a
__portp_mask	equ	$77		; Port P 6..4, 2..0
__ports_mask	equ	$8f		; Port S 0..3,7
__portt_mask	equ	$9f		; Port T 0..4,7
		endif
		include	"s12z/s12zvcpim.inc"
		
		include	"s12z/ftmrz.inc"
		include	"s12z/s12zecc.inc"
		include "s12z/s12ztim.inc"
		org	$400
TIM1		S12ZTIM
		org	$5c0
TIM0		S12ZTIM
		include	"s12z/s12zpwm.inc"
		org	$480
PWM0		S12ZPWM
		org	$500
PWM1		S12ZPWM
		include	"s12z/s12zadc.inc"
		org	$600
ADC0		S12ZADC

		if	__vca		; DAC & ACM only on VCA models
		include	"s12z/s12zdac.inc"
		include	"s12z/s12zacm.inc"
		org	$690
ACM0		S12ZACM
		org	$698
ACM1		S12ZACM
		endif			; __vca
		
		include	"s12z/s12zcpmu.inc"
		include	"s12z/s12zbats.inc"
		include	"s12z/s12zsci.inc"
		org	$700
SCI0		S12ZSCI
		org	$710
SCI1		S12ZSCI

		include	"s12z/s12zspi.inc"
		org	$780
SPI0		S12ZSPI
		org	$790
		if	__housing=64	; SPI1 only on 64-pin models
SPI1		S12ZSPI
		endif

		include	"s12z/s12ziic.inc"
		include	"s12z/s12zcan.inc"
		org	$800
CAN		S12ZCAN
		include	"s12z/s12zsent.inc"
		
		restore			; re-enable listing

		endif			; __vc_inc
