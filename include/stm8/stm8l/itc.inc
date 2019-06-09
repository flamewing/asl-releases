		ifndef	__stm8litcinc	; avoid multiple inclusion
__stm8litcinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ITC.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L Interrupt Controller        *
;*                                                                          *
;****************************************************************************

__defitc	macro	PrioBase,NumPrio
__B		set	0
__N		set	0
__V		set	0
		rept	NumPrio
		 if	__B=0
		  __decstr __NS,__N+1
ITC_SPR{__NS}	  label	PrioBase+__N
		 endif
		 __decstr __VS,__V
VECT{__VS}SPR	 bfield	ITC_SPR{__NS},__B,2
__V		 set	__V+1
__B		 set	__B+2
		 if	__B=8
__B		  set	0
__N		  set	__N+1
		 endif
		endm
		endm

		restore
		endif			; __stm8litcinc
