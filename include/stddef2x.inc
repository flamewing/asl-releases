		save
		listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei STDDEF2X.INC                                           *
;*   								            *
;*   Sinn : enthaelt die globalen Register-Definitionen fuer die            *
;*          TMS320C2x Prozessoren                                           *
;* 									    *
;*   letzte Aenderungen : 20.08.1996                                        *
;*                                                                          *
;****************************************************************************

		ifndef  stddef2xinc     ; verhindert Mehrfacheinbindung

stddef2xinc     equ     1

		if      (MomCPU<>320C25H) && (MomCPU<>320C26H) && (MomCPU<>320C28H)
		 fatal  "Falscher Prozessortyp eingestellt: nur 320C25, 320C26 und 320C28 erlaubt!"
		endif

		if      mompass=1
		 message "TMS320C2x Register-Definitionen (C) 1994,96 Thomas Sailer"
		endif

;----------------------------------------------------------------------------
		segment	data
		org	0

DRR		res	1
DXR		res	1
TIM		res	1
PRD		res	1
IMR		res	1
GREG		res	1

		if (MomCPU=320C26H)
B2_D		equ	60h
B0_D		equ	200h
B1_D		equ	400h
B3_D		equ	600h
B0_P		equ	0fa00h
B1_P		equ	0fc00h
B3_P		equ	0fe00h
		endif

;---------------------------------------------------------------------------

                endif

                restore                 ; wieder erlauben

