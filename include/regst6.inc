                ifndef  __regst6inc     ; avoid multiple inclusion
__regst6inc	equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGST6.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST6 Processors                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Until bit field symbols are introduced...

bfield		macro	{INTLABEL},reg,start,count
__LABEL__	equ	((1<<(count+start))-1)-((1<<start)-1)
		endm

;----------------------------------------------------------------------------
; Include proper CPU-specific register definitions

		switch	MOMCPUNAME
		case	"ST6200", "ST6201", "ST6203"
		  include "st6/reg6200.inc"
		case	"ST6208", "ST6209", "ST6210", "ST6220"
		  include "st6/reg6208.inc"
		case	"ST6215", "ST6225"
		  include "st6/reg6215.inc"
		case	"ST6218"
		  include "st6/reg6218.inc"
		case	"ST6228"
		  include "st6/reg6228.inc"
		case	"ST6230","ST6232"
		  include "st6/reg6230.inc"
		case	"ST6235"
		  include "st6/reg6235.inc"
		case	"ST6240"
		  include "st6/reg6240.inc"
		case	"ST6242"
		  include "st6/reg6242.inc"
		case	"ST6245"
		  include "st6/reg6245.inc"
		case	"ST6246"
		  include "st6/reg6246.inc"
		case	"ST6252","ST6262"
		  include "st6/reg6252.inc"
		case	"ST6253","ST6260","ST6263"
		  include "st6/reg6253.inc"
		case	"ST6255","ST6265"
		  include "st6/reg6255.inc"
		case	"ST6280"
		  include "st6/reg6280.inc"
		case	"ST6285"
		  include "st6/reg6285.inc"
		elsecase
		  error	"wrong processor type set: only ST6200, ST6201, ST6203, ST6208, ST6209, ST6210, ST6215, ST6218, ST6220, ST6225, ST6228, ST6230, ST6232, ST6235, ST6240, ST6242, ST6245, ST6246, ST6252, ST6253, ST6255, ST6260, ST6262, ST6263,"
		  fatal "ST6265, ST6280, ST6285 allowed!"
                endcase

                if      MOMPASS=1
                 message "ST6 Register Definitions (C) 2019 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; CPU

RegX		sfr	80h		; already known as internal symbols, just for completeness
RegY		sfr	81h
RegV		sfr	82h
RegW		sfr	83h
RegA		sfr	0ffh

DRWR		sfr	0c9h		; Data ROM Window register
RomBase		sfr	40h		; Data ROM Window

;----------------------------------------------------------------------------
; Post Processing

		ifndef	ROMEND
ROMEND		 label	0fffh
		endif

		if	ROMEND>0fffh
PRPR		sfr	0cah		; Program ROM Page Register
		endif
		
		include	"st6/instr.inc"

;----------------------------------------------------------------------------

                restore                 ; allow again

                endif			; __regst6inc
