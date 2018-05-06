		ifndef	__s12z_bats_inc
__s12z_bats_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZBATS.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Battery Sensor            *
;*                                                                          *
;****************************************************************************

BATE		equ	$06F0		; BATS Module Enable Register
BVHS		s12zbit	BATE,6		;  BATS Voltage High Select
BVLS		s12zfld	BATE,2:4	;  BATS Voltage Low Select
BSUAE		s12zbit	BATE,3		;  BATS VSUP ADC Connection Enable
BSUSE		s12zbit	BATE,2		;  BATS VSUP Level Sense Enable
BATSR		equ	$06F1		; BATS Module Status Register
BVHC		s12zbit	BATSR,1		;  BATS Voltage Sense High Condition Bit
BVLC		s12zbit	BATSR,0		;  BATS Voltage Sense Low Condition Bit
BATIE		equ	$06F2		; BATS Interrupt Enable Register
BVHIE		s12zbit	BATIE,1		;  BATS Interrupt Enable High
BVLIE		s12zbit	BATIE,0		;  BATS Interrupt Enable Low
BATIF		equ	$06F3		; BATS Interrupt Flag Register
BVHIF		s12zbit	BATIF,1		;  BATS Interrupt Flag High Detect
BVLIF		s12zbit	BATIF,0		;  BATS Interrupt Flag Low Detect

		restore			; re-enable listing

		endif			; __s12z_bats_inc
