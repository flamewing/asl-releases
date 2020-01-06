		ifndef	__stm8suidinc	; avoid multiple inclusion
__stm8suidinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File UID.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Acces Line Unique ID        *
;*                                                                          *
;****************************************************************************

__defuid	macro	Base
U_IDSTART	label	Base+$00	; start address unique id
U_ID_XCOORD	label	Base+$00	;  X co-ordinate on the wafer
U_ID_YCOORD	label	Base+$02	;  Y co-ordinate on the wafer
U_ID_WFNUM	label	Base+$04	;  Wafer number
U_ID_LOT	label	Base+$05	;  Lot number
U_IDEND		label	Base+$0c	; end address unique id
		endm

		restore
		endif			; __stm8suidinc
