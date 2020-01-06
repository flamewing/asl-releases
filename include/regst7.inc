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
 		case	"ST72344K2", "ST72344K4"
 		  include "st7/reg72344.inc"
 		case	"ST72345C4"
 		  include "st7/reg72345.inc"
 		case	"ST72361AR4", "ST72361AR6", "ST72361AR7", "ST72361AR9"
 		  include  "st7/reg72361.inc"
 		case	"ST72521BR6", "ST72521BM9"
 		  include "st7/reg72521.inc"
		case	"ST7232AK1", "ST7232AK2", "ST7232AJ1", "ST7232AJ2"
		  include  "st7/reg7232a.inc"
		case	"ST7FOXK1"
		  include  "st7/regfox1.inc"
		case	"ST7FOXK2"
		  include  "st7/regfox2.inc"
		case	"ST7LITES2Y0","ST7LITES5Y0","ST7LITE02Y0","ST7LITE05Y0","ST7LITE09Y0"
		  include  "st7/reglit0x.inc"
		case	"ST7LITE10F1","ST7LITE15F1","ST7LITE19F1"
		  include  "st7/reglit1x.inc"
		case	"ST7LITE10BF0","ST7LITE15BF0","ST7LITE15BF1","ST7LITE19BF0","ST7LITE19BF1"
		  include  "st7/reglt1bx.inc"
		case	"ST7LITE20F2","ST7LITE25F2","ST7LITE29F2"
		  include  "st7/reglit2x.inc"
		case	"ST7LITE30F2","ST7LITE35F2","ST7LITE39F2"
		  include  "st7/reglit3x.inc"
		case	"ST7LITE49K2"
		  include  "st7/reglit4x.inc"
		case	"ST7MC1K2","ST7MC1K4"
		  include  "st7/regmc1.inc"
		case	"ST7MC2N6","ST7MC2S4","ST7MC2S6","ST7MC2S7","ST7MC2S9","ST7MC2R6","ST7MC2R7","ST7MC2R9","ST7MC2M9"
		  include  "st7/regmc2.inc"
		elsecase
		  error	"wrong processor type set: only ST72251G1, ST72251G2, ST72311J2, ST72311J4, ST72321BR6, ST72321BR7, ST72321BR9, ST72324J6, ST72324K6, ST72324J4, ST72324K4, ST72324J2, ST72324JK2, ST72325S4, ST72325S6, ST72325J7,"
		  error	"ST72325R9, ST72521BR6, ST72521BM9, ST7232AK1, ST7232AK2, ST7232AJ1, ST7232AJ2, ST7FOXK1, ST7FOXK2, ST7LITES2Y0, ST7LITES5Y0, ST7LITE02Y0, ST7LITE05Y0, ST7LITE09Y0, ST7LITE10F1, ST7LITE15F1, ST7LITE19F1, ST7LITE10BF0,"
		  fatal	"ST7LITE15BF0, ST7LITE15BF1, ST7LITE19BF0, ST7LITE19BF1, ST7LITE20F2, ST7LITE25F2, ST7LITE29F2, ST7LITE30F2, ST7LITE35F2, ST7LITE39F2, ST7LITE49K2 allowed!"
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
		case	"0"			; '0' -> 2K
ROMSTART	label	ROMEND-$07ff
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
