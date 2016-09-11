		save
		listing	off		; do not list this file in listing

;****************************************************************************
;*                                                                          *
;*   AS - File REG6303.INC                                                  *
;*   								            *
;*   Purpose : contains register addresses for Hitachi 6303                 *
;* 									    *
;****************************************************************************
;* $Id: reg6303.inc,v 1.1 2016/08/27 09:32:56 alfred Exp $                  *
;**************************************************************************** 
;* $Log: reg6303.inc,v $
;* Revision 1.1  2016/08/27 09:32:56  alfred
;* - add register definitions for Hitachi/Renesas 6303
;*
;****************************************************************************

		ifndef	_reg6303.inc
_reg6303inc	equ	1

		if	MOMCPUNAME<>"6301"
		 fatal	 "wrong processor type: only 6301 allowed"
		endif

		if MOMPASS=1
		 message "Hitachi/Renesas 6303 Definitions (C) 2016 Alfred Arnold"
		endif

;-----------------------------------------
; Ports

P1DDR		equ	$0000
P2DDR		equ	$0001
PORT1		equ	$0002
PORT2		equ	$0003
P3DDR		equ	$0004
P4DDR		equ	$0005
PORT3		equ	$0006
PORT4		equ	$0007
RP5CR		equ	$0014
P5DDR		equ	$0020
PORT5		equ	$0015
P6CSR		equ	$0021
P6DDR		equ	$0016
PORT6		equ	$0017
PORT7		equ	$0018

; ------------------------------------------
; Timer

TCSR1		equ	$0008
FRCH		equ	$0009
FRCL		equ	$000a
OCR1H		equ	$000b
OCR1L		equ	$000c
ICRH		equ	$000d
ICRL		equ	$000e

TCSR2		equ	$000f
OCR2H		equ	$0019
OCR2L		equ	$001a

TCSR3		equ	$001b
TCONR		equ	$001c
T2CNT		equ	$001d
TRCSR2		equ	$001e

RMCR		equ	$0010
TRCSR1		equ	$0011
RDR		equ	$0012
TDR		equ	$0013

TSTREG		equ	$001f

		endif			; _reg6303

		restore			; listing on again
