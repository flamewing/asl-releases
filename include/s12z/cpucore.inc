		ifndef	__s12z_cpucore_inc
__s12z_cpucore_inc	equ	1
		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CPUCORE.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z CPU Core                  *
;*                                                                          *
;****************************************************************************

; -------------------------------------------------------------------------
; Part ID

PARTID0		equ	$0000		; Part ID Bit 31..24
PARTID1		equ	$0001		; Part ID Bit 23..16
PARTID2		equ	$0002		; Part ID Bit 15..8
PARTID3		equ	$0003		; Part ID Bit 7..0

		restore			; re-enable listing

		endif			; __s12z_cpucore_inc
