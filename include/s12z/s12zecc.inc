		ifndef	__s12z_ecc_inc
__s12z_ecc_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZECC.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z ECC Memory Controlle      *
;*                                                                          *
;****************************************************************************

ECCSTAT		equ	$03C0		; ECC Status Register
RDY		s12zbit	ECCSTAT,0	;  ECC Ready
ECCIE		equ	$03C1		; ECC Interrupt Enable Register
SBEEIE		s12zbit	ECCIE,0		;  Single bit ECC Error Interrupt Enable
ECCIF		equ	$03C2		; ECC Interrupt Flag Register
SBEEIF		s12zbit	ECCIF,0		;  Single bit ECC Error Interrupt Flag
ECCDPTR		equ	$03C7		; ECC Debug Pointer Register (24 bit)
ECCDPTRH	equ	$03C7		; ECC Debug Pointer Register MSB
ECCDPTRM	equ	$03C8
ECCDPTRL	equ	$03C9		; ECC Debug Pointer Register LSB
ECCDD		equ	$03CC		; ECC Debug Data (16 bit)
ECCDDH		equ	$03CC		; ECC Debug Data MSB
ECCDDL		equ	$03CD		; ECC Debug Data LSB
ECCDE		equ	$03CE		; ECC Debug ECC
DECC		s12zfld	ECCDE,6:0	;  ECC Debug ECC
ECCDCMD		equ	$03CF		; ECC Debug Command
ECCDRR		s12zbit	ECCDCMD,7	;  ECC Disable Read Repair Function
ECCDW		s12zbit	ECCDCMD,1	;  ECC Debug Write Command
ECCDR		s12zbit	ECCDCMD,0	;  ECC Debug Read Command

		restore			; re-enable listing

		endif			; __s12z_ecc_inc
