		ifndef	__s12z_acm_inc
__s12z_acm_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZACM.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Analog Comparator         *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZACM		struct	dots
PC0		ds.b	1		; ($00) ACMP Control Register 0
ACE		s12zbit	PC0,7		;  ACMP Enable
ACOPE		s12zbit	PC0,6		;  ACMP Output Pin Enable
ACOPS		s12zbit	PC0,5		;  ACMP Output Polarity Select
ACDLY		s12zbit	PC0,4		;  ACMP Input Filter Select for Inputs ACMP_0 and ACMP_1
ACHYS		s12zfld	PC0,2:2		;  ACMP Hysteresis
ACMOD		s12zfld	PC0,2:0		;  ACMP Mode
PC1		ds.b	1		; ($01) ACMP Control Register 1
ACPSEL		s12zfld	PC1,2:4		;  ACMP Positive Input Select
ACNSEL		s12zfld	PC1,2:0		;  ACMP Negative Input Select
PC2		ds.b	1		; ($02) ACMP Control Register 2
ACIE		s12zbit	PC2,0		;  ACMP Interrupt Enable
PS		ds.b	1		; ($03) ACMP Status Register
ACIF		s12zbit	PS,0		;  ACMP Interrupt Flag
		ends	S12ZACM

		restore			; re-enable listing

		endif			; __s12z_acm_inc
