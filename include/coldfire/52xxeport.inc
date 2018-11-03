		ifndef	__52xxeportinc		; avoid multiple inclusion
__52xxeportinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXEPORT.INC                                           *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Programmable     *
;*   Interrupt Timer                                                        *
;*                                                                          *
;****************************************************************************

EPPAR		equ		MBAR_EPORT+$00	; Pin Assignment Register (16b)
EPPA7		cffield		EPPAR,14,2
EPPA6		cffield		EPPAR,12,2
EPPA5		cffield		EPPAR,10,2
EPPA4		cffield		EPPAR,8,2
EPPA3		cffield		EPPAR,6,2
EPPA2		cffield		EPPAR,4,2
EPPA1		cffield		EPPAR,2,2
EPPA0		cffield		EPPAR,0,2
EPDDR		equ		MBAR_EPORT+$02	; Data Direction Register (8b)
		__enumbits	EPDDR,EPDD,8
EPIER		equ		MBAR_EPORT+$03	; Interrupt Enable Register (8b)
		__enumbits	EPIER,EPIE,8
EPDR		equ		MBAR_EPORT+$04	; Data Register (8b)
		__enumbits	EPDR,EPD,8
EPPDR		equ		MBAR_EPORT+$05	; Pin Data Register (8b)
		__enumbits	EPDDR,EPPD,8
EPFR		equ		MBAR_EPORT+$06	; Flag Register (8b)
		__enumbits	EPFR,EPF,8

		restore				; re-enable listing

                endif                           ; __52xxeportinc
