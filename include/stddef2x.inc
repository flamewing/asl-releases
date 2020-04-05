		ifndef  stddef2xinc     ; avoid multiple inclusion
stddef2xinc     equ     1

		save
		listing off   ; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF2X.INC                                            *
;*   								            *
;*   Holds register definitions for TMS320C2x Processors                    *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"320C25") && (MOMCPUNAME<>"320C26") && (MOMCPUNAME<>"320C28")
		 fatal  "wrong target set: only 320C25, 320C26 und 320C28 supported"
		endif

                if      MOMPASS=1
		 message "TMS320C2x Register Definitions (C) 1994 Thomas Sailer"
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

                if (MOMCPUNAME="320C26")
B2_D		equ	60h
B0_D		equ	200h
B1_D		equ	400h
B3_D		equ	600h
B0_P		equ	0fa00h
B1_P		equ	0fc00h
B3_P		equ	0fe00h
		endif

;---------------------------------------------------------------------------

                restore                 ; re-allow listing

                endif			; stddef2xinc
