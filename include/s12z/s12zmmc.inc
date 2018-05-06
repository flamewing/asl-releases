		ifndef	__s12z_mmc_inc
__s12z_mmc_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZMMC.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Memory Management Cntrler *
;*                                                                          *
;****************************************************************************

MODE		equ	$0070		; Mode Register
MODC		s12zbit	MODE,7		;  Mode Select Bit
MMCECH		equ	$0080		; Error Code Register MSB
ITR		s12zfld	MMCECH,4:4	;  Initiator Field
TGT		s12zfld	MMCECH,4:0	;  Target Field
MMCECL		equ	$0081		; Error Code Register LSB
ACC		s12zfld	MMCECL,4:4	;  Access Type Field
ERR		s12zfld	MMCECL,4:0	;  Error Type Field
MMCCCRH		equ	$0082		; Captured S12ZCPU Condition Code Register MSB
CPUU		s12zbit	MMCCCRH,7	;  S12ZCPU User State Flag
MMCCCRL		equ	$0083		; Captured S12ZCPU Condition Code Register LSB
CPUX		s12zbit	MMCCCRL,6	;  S12ZCPU X-Interrupt Mask
CPUI		s12zbit	MMCCCRL,4	;  S12ZCPU I-Interrupt Mask
MMCPC		equ	$0085		; Captured S12ZCPU Program Counter (24 bit)
MMCPCH		equ	$0085		; Captured S12ZCPU Program Counter MSB
MMCPCM		equ	$0086
MMCPCL		equ	$0087		; Captured S12ZCPU Program Counter LSB

		restore			; re-enable listing

		endif			; __s12z_mmc_inc
