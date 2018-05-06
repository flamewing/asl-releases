		ifndef	__s12z_int_inc
__s12z_int_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZINT.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Interrupt Controller      *
;*                                                                          *
;****************************************************************************

IVBR		equ	$0010		; Interrupt Vector Base Register
INT_XGPRIO	equ	$0016
INT_CFADDR	equ	$0017		; Interrupt Request Configuration Address Register
INT_CFDATA0	equ	$0018		; Interrupt Request Configuration Data Register 0
INT_CFDATA1	equ	$0019		; Interrupt Request Configuration Data Register 1
INT_CFDATA2	equ	$001a		; Interrupt Request Configuration Data Register 2
INT_CFDATA3	equ	$001b		; Interrupt Request Configuration Data Register 3
INT_CFDATA4	equ	$001c		; Interrupt Request Configuration Data Register 4
INT_CFDATA5	equ	$001d		; Interrupt Request Configuration Data Register 5
INT_CFDATA6	equ	$001e		; Interrupt Request Configuration Data Register 6
INT_CFDATA7	equ	$001f		; Interrupt Request Configuration Data Register 7

		restore			; re-enable listing

		endif			; __s12z_int_inc
