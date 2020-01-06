		ifndef	__artimerinc	; avoid multiple inclusion
__artimerinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ARTIMER.INC                                             *
;*                                                                          *
;* contains SFR and Bit Definitions for ST62xx Full AR Timer (all but 6285) *
;*                                                                          *
;****************************************************************************

		include	"artimbas.inc"

__defartimer	macro	Base
		__defartimbas Base
PWMOE		bit	5,ARMCR		;  PWM Output Enable
EIE		bit	4,ARMCR		;  External Interrupt Enable
EF		bit	2,ARSC0		;  External Interrupt Flag
SL		bfield	ARSC1,2,2	;  Timer Input Edge Control
		endm

		restore
		endif			; __artimerinc
