                ifndef  __regst7inc     ; avoid multiple inclusion
__regst7inc	equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGST7.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 Processors                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; General Macros

bfield		macro	{INTLABEL},reg,start,count
__LABEL__	equ	((1<<(count+start))-1)-((1<<start)-1)
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

;----------------------------------------------------------------------------
; Include proper CPU-specific register definitions

		switch	MOMCPUNAME
		case	"ST72251G1", "ST72251G2"
		  include "st7/reg72251.inc"
		case	"ST72311J2", "ST72311J4"
		  include "st7/reg72311.inc"
 		case	"ST72321BR6", "ST72321BR7", "ST72321BR9"
 		  include "st7/reg72321.inc"
 		case	"ST72324J6", "ST72324K6", "ST72324J4", "ST72324K4", "ST72324J2", "ST72324K2"
 		  include "st7/reg72324.inc"
 		case	"ST72325S4", "ST72325S6", "ST72325J7", "ST72325R9"
 		  include "st7/reg72325.inc"
 		case	"ST72521BR6", "ST72521BM9"
 		  include "st7/reg72521.inc"
		elsecase
		  error	"wrong processor type set: only ST72251G1, ST72251G2, ST72311J2, ST72311J4, ST72321BR6, ST72321BR7, ST72321BR9, ST72324J6, ST72324K6, ST72324J4, ST72324K4, ST72324J2, ST72324JK2, ST72325S4, ST72325S6, ST72325J7,"
		  fatal	"ST72325R9, ST72521BR6, ST72521BM9 allowed!"
                endcase

                if      MOMPASS=1
                 message "ST7 Register Definitions (C) 2019 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Post Processing

		; ROM Size can usually be deduced from the part number:
		
		ifndef	ROMEND
ROMEND		 label	$ffff
		endif
		
		ifndef	ROMSTART
                switch	substr(MOMCPUNAME,STRLEN(MOMCPUNAME)-1,1)
		case	"1"			; '1' -> 4K
ROMSTART	label	ROMEND-$0fff
		case	"2"			; '2' -> 8K
ROMSTART	label	ROMEND-$1fff
		case	"4"			; '4' -> 16K
ROMSTART	label	ROMEND-$3fff
		case	"6"			; '6' -> 32K
ROMSTART	label	ROMEND-$7fff
		case	"7"			; '7' -> 48K
ROMSTART	label	ROMEND-$bfff
		case	"9"			; '9' -> 60K
ROMSTART	label	ROMEND-$efff
		elsecase
		fatal	"Cannot deduce ROM size from device name"
		endcase
		endif			; ifndef ROMEND

;----------------------------------------------------------------------------

                restore                 ; allow again

                endif			; __regst7inc
