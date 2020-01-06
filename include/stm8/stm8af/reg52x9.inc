		ifndef	__stm8af52x9inc	; avoid multiple inclusion
__stm8af52x9inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG52X9.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8AF5268/69/8X/AX               *
;*   source: DocID14395 Rev 15                                              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Everything like 62xx variants...

		include	"reg62x9.inc"

;----------------------------------------------------------------------------
; ...plus CAN

CAN_RX_vect	label	$8028		; beCAN RX interrupt
CAN_TX_vect	label	$802c		; beCAN TX/ER/SC interrupt

		include "stm8/stm8s/becan.inc"
		__defbecan $5420

                restore                 ; allow again

                endif			; __stm8af52x9inc
