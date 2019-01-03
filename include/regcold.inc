		ifndef	__regcoldinc	; avoid multiple inclusion
__regcoldinc	equ	1

		save
		listing	off		; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGCOLD.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire Processors               *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Bits with the name as given in the datasheets contain the plain bit
; position within the register; with the prefix _bit_..., they hold
; register and bit position

cfbit		macro   {intlabel},reg,pos
reg.__LABEL__	equ	pos
		endm

cffield		macro	{intlabel},reg,start,width
reg.__LABEL__	equ	start
		endm

;----------------------------------------------------------------------------
; commonly used macros

__hexstr	macro		DEST,SRC
DEST		set		"\{SRC}"
		endm

__decstr	macro		DEST,SRC
		if		(SRC)<10
DEST		set		"\{SRC}"
		elseif
DEST		set		"\{(SRC)/10}\{(SRC)#10}"
		endif
		endm

__dec02str	macro		DEST,SRC
DEST		set		"\{(SRC)/10}\{(SRC)#10}"
		endm

__enumregs	macro		ARG,START,END
__N		set		START
		rept		END-START+1
		__decstr	__NS,__N
		ARG
__N		set		__N+1
		endm
		endm

__enumbits	macro		Reg,Prefix,Cnt,NameOffs=0
__N		set		0
		rept		Cnt
		__decstr	__NS,__N+NameOffs
Prefix{"\{__NS}"} cfbit		Reg,__N
__N		set		__N+1
		endm
		endm

__enumbits_o	macro		Reg,Prefix,Start,Cnt,NameOffs=0
__N		set		Start
		rept		Cnt
		__decstr	__NS,__N+(NameOffs)
Prefix{"\{__NS}"} cfbit		Reg,__N
__N		set		__N+1
		endm
		endm

__enumbits2	macro		Reg,Prefix,Postfix,Cnt,NameOffs=0
__N		set		0
		rept		Cnt
		__decstr	__NS,__N+NameOffs
Prefix{"\{__NS}"}Postfix cfbit		Reg,__N
__N		set		__N+1
		endm
		endm

;----------------------------------------------------------------------------
; include proper CPU-specific register definitions

		if	MOMPASS=1
		  message "ColdFire SFR Definitions (C) 2018 Alfred Arnold"
		endif

		switch	MOMCPUNAME

		case "MCF5202"
		  ; tolerate 5202, but define no registers at all
		case "MCF5204"
		  include "coldfire/mcf5204.inc"
		case "MCF5206"
		  include "coldfire/mcf5206.inc"
		case "MCF5208"
		  include "coldfire/mcf5208.inc"
		case "MCF52274","MCF52277"
		  include "coldfire/mcf52277.inc"
		case "MCF5307"
		  include "coldfire/mcf5307.inc"
		case "MCF5329"
		  include "coldfire/mcf5329.inc"
		case "MCF5373"
		  include "coldfire/mcf5373.inc"
		case "MCF5407"
		  include "coldfire/mcf5407.inc"
		case "MCF5470"
		  include "coldfire/mcf5470.inc"
		case "MCF5471"
		  include "coldfire/mcf5471.inc"
		case "MCF5472"
		  include "coldfire/mcf5472.inc"
		case "MCF5473"
		  include "coldfire/mcf5473.inc"
		case "MCF5474"
		  include "coldfire/mcf5474.inc"
		case "MCF5475"
		  include "coldfire/mcf5475.inc"
		case "MCF51QM"
		  include "coldfire/mcf51qm.inc"
		  
		elsecase
		  error	"wrong processor type set: only MCF5204, MCF5206, MCF5208, MCF52274, MCF52277, MCF5307, MCF5329, MCF5373, MCF5407, MCF5470 to MCF5475, MCF51QM allowed!"
		endcase

;----------------------------------------------------------------------------

                restore                 ; re-enable listing

                endif			; __regcoldinc