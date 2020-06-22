                ifndef  __regz8inc     ; avoid multiple inclusion
__regz8inc	equ     1

                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGZ8.INC                                               *
;*                                                                          *
;*   Sinn : contains SFR and Bit Definitionen for Zilog Z8 Processors       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Helper Macros

__z8bit		macro	{intlabel},reg,bitpos
__label__	defbit	reg,bitpos
		endm
__z8cbit	macro	{intlabel},reg,bitpos
__label__	equ	1 << bitpos
		endm

__z8bfield	macro	{intlabel},reg,start,count
__label__	equ	((1 << count) - 1) << start
		endm
__z8cbfield	macro	{intlabel},reg,start,count
__label__	equ	((1 << count) - 1) << start
		endm


;----------------------------------------------------------------------------
; Include proper CPU-specific register definitions

		switch	MOMCPUNAME
		case	"Z8601"
RAMEND		 sfr	 7fh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
		 include "z8/z86xx.inc"
		 
		case	"Z8603"
RAMEND		 sfr	 7fh
__hastimer1	 equ	 1
		 include "z8/z86xx.inc"
		 
		case	"Z86C03","Z86E03"
RAMEND		 sfr	 3fh
ROMEND		 label	 1ffh
__hastimer1	 equ	 0
__haswdt	 equ	 0
__hasspi	 equ	 1
		 include "z8/z86cxx.inc"
		 
		;case	"Z8604"
		case	"Z86C06","Z86E06"
RAMEND		 sfr	 7fh
ROMEND		 label	 0fffh
__hastimer1	 equ	 1
__haswdt	 equ	 1
__hasspi	 equ	 1
		 include "z8/z86cxx.inc"
		 
		case	"Z86C08"
RAMEND		 sfr	 7fh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		 include "z8/z86cxx.inc"

		case	"Z86C21"
RAMEND		 sfr	 0efh
ROMEND		 label	 1fffh
__hastimer1	 equ	 1
		 include "z8/z86xx.inc"

		case	"Z86E21"
RAMEND		 sfr	 0efh
ROMEND		 label	 1fffh
__hastimer1	 equ	 1
		 include "z8/z86xx.inc"
		 
		case	"Z86C30"
RAMEND		 sfr	 0efh
ROMEND		 label	 0fffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		 include "z8/z86cxx.inc"
		 
		case	"Z86C31"
RAMEND		 sfr	 7fh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		 include "z8/z86cxx.inc"
		 
		case	"Z86C32"
RAMEND		 sfr	 0efh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		 include "z8/z86cxx.inc"
		 
		case	"Z86C40"
RAMEND		 sfr	 0efh
ROMEND		 label	 0fffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		 include "z8/z86cxx.inc"
		 
		case	"Z88C00","Z88C01"
RAMEND		 sfr	 0bfh
		 include "z8/z88c0x.inc"
		 
		case	"Z8F0130"
RAMEND		 sfr	 0ffh
ROMEND		 label	 03ffh
__hasadc	 equ	 0
		 include "z8/f0830.inc"
		 
		case	"Z8F0131"
RAMEND		 sfr	 0ffh
ROMEND		 label	 03ffh
__hasadc	 equ	 1
		 include "z8/f0830.inc"
		 
		case	"Z8F0230"
RAMEND		 sfr	 0ffh
ROMEND		 label	 07ffh
__hasadc	 equ	 0
		 include "z8/f0830.inc"
		 
		case	"Z8F0231"
RAMEND		 sfr	 0ffh
ROMEND		 label	 07ffh
__hasadc	 equ	 1
		 include "z8/f0830.inc"
		 
		case	"Z8F0430"
RAMEND		 sfr	 0ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 0
		 include "z8/f0830.inc"
		 
		case	"Z8F0431"
RAMEND		 sfr	 0ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 1
		 include "z8/f0830.inc"
		 
		case	"Z8F0830"
RAMEND		 sfr	 0ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 0
		 include "z8/f0830.inc"
		 
		case	"Z8F0831"
RAMEND		 sfr	 0ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 1
		 include "z8/f0830.inc"
		 
		case	"Z8F1232"
RAMEND		 sfr	 0ffh
ROMEND		 label	 2fffh
__hasadc	 equ	 0
		 include "z8/f0830.inc"
		 
		case	"Z8F1233"
RAMEND		 sfr	 0ffh
ROMEND		 label	 2fffh
__hasadc	 equ	 1
		 include "z8/f0830.inc"

		case	"Z8F0113"
RAMEND		 sfr	 0ffh
ROMEND		 label	 3ffh
__hasadc	 equ	 0
		 include "z8/f0823.inc"

		case	"Z8F0123"
RAMEND		 sfr	 0ffh
ROMEND		 label	 3ffh
__hasadc	 equ	 1
		 include "z8/f0823.inc"

		case	"Z8F0213"
RAMEND		 sfr	 1ffh
ROMEND		 label	 7ffh
__hasadc	 equ	 0
		 include "z8/f0823.inc"

		case	"Z8F0223"
RAMEND		 sfr	 1ffh
ROMEND		 label	 7ffh
__hasadc	 equ	 1
		 include "z8/f0823.inc"

		case	"Z8F0413"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 0
		 include "z8/f0823.inc"

		case	"Z8F0423"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 1
		 include "z8/f0823.inc"

		case	"Z8F0813"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 0
		 include "z8/f0823.inc"

		case	"Z8F0823"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 1
		 include "z8/f0823.inc"

		case	"Z8F011A"
RAMEND		 sfr	 0ffh
ROMEND		 label	 3ffh
__hasadc	 equ	 0
		 include "z8/f082a.inc"

		case	"Z8F012A"
RAMEND		 sfr	 0ffh
ROMEND		 label	 3ffh
__hasadc	 equ	 1
		 include "z8/f082a.inc"

		case	"Z8F021A"
RAMEND		 sfr	 1ffh
ROMEND		 label	 7ffh
__hasadc	 equ	 0
		 include "z8/f082a.inc"

		case	"Z8F022A"
RAMEND		 sfr	 1ffh
ROMEND		 label	 7ffh
__hasadc	 equ	 1
		 include "z8/f082a.inc"

		case	"Z8F041A"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 0
		 include "z8/f082a.inc"

		case	"Z8F042A"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 1
		 include "z8/f082a.inc"

		case	"Z8F081A"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 0
		 include "z8/f082a.inc"

		case	"Z8F082A"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 1
		 include "z8/f082a.inc"

		case	"Z8F0411"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 0
__hasspi	 equ	 0
		 include "z8/f0822.inc"

		case	"Z8F0412"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 0
__hasspi	 equ	 1
		 include "z8/f0822.inc"

		case	"Z8F0421"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 1
__hasspi	 equ	 0
		 include "z8/f0822.inc"

		case	"Z8F0422"
RAMEND		 sfr	 3ffh
ROMEND		 label	 0fffh
__hasadc	 equ	 1
__hasspi	 equ	 1
		 include "z8/f0822.inc"

		case	"Z8F0811"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 0
__hasspi	 equ	 0
		 include "z8/f0822.inc"

		case	"Z8F0812"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 0
__hasspi	 equ	 1
		 include "z8/f0822.inc"

		case	"Z8F0821"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 1
__hasspi	 equ	 0
		 include "z8/f0822.inc"

		case	"Z8F0822"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
__hasadc	 equ	 1
__hasspi	 equ	 1
		 include "z8/f0822.inc"
		 
		case	"Z8F0880"
RAMEND		 sfr	 3ffh
ROMEND		 label	 1fffh
		 include "z8/f1680.inc"

		case	"Z8F1680"
RAMEND		 sfr	 7ffh
ROMEND		 label	 3fffh
		 include "z8/f1680.inc"

		case	"Z8F2480"
RAMEND		 sfr	 7ffh
ROMEND		 label	 5fffh
		 include "z8/f1680.inc"

		case	"Z8F1621"
RAMEND		 sfr	 7ffh
ROMEND		 label	 3fffh
__hastimer3	 equ	 0
		 include "z8/f64xx.inc"

		case	"Z8F1622"
RAMEND		 sfr	 7ffh
ROMEND		 label	 3fffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F2421"
RAMEND		 sfr	 7ffh
ROMEND		 label	 5fffh
__hastimer3	 equ	 0
		 include "z8/f64xx.inc"

		case	"Z8F2422"
RAMEND		 sfr	 7ffh
ROMEND		 label	 5fffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F3221"
RAMEND		 sfr	 7ffh
ROMEND		 label	 7fffh
__hastimer3	 equ	 0
		 include "z8/f64xx.inc"

		case	"Z8F3222"
RAMEND		 sfr	 7ffh
ROMEND		 label	 7fffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F4821"
RAMEND		 sfr	 0effh
ROMEND		 label	 0bfffh
__hastimer3	 equ	 0
		 include "z8/f64xx.inc"

		case	"Z8F4822"
RAMEND		 sfr	 0effh
ROMEND		 label	 0bfffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F4823"
RAMEND		 sfr	 0effh
ROMEND		 label	 0bfffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F6421"
RAMEND		 sfr	 0effh
ROMEND		 label	 0ffffh
__hastimer3	 equ	 0
		 include "z8/f64xx.inc"

		case	"Z8F6422"
RAMEND		 sfr	 0effh
ROMEND		 label	 0ffffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F6423"
RAMEND		 sfr	 0effh
ROMEND		 label	 0ffffh
__hastimer3	 equ	 1
		 include "z8/f64xx.inc"

		case	"Z8F1681"
RAMEND		 sfr	 7ffh
ROMEND		 label	 3fffh
__haslcd	 equ	 0
		 include "z8/f6482.inc"

		case	"Z8F1682"
RAMEND		 sfr	 7ffh
ROMEND		 label	 3fffh
__haslcd	 equ	 1
		 include "z8/f6482.inc"

		case	"Z8F3281"
RAMEND		 sfr	 0effh
ROMEND		 label	 7fffh
__haslcd	 equ	 0
		 include "z8/f6482.inc"

		case	"Z8F3282"
RAMEND		 sfr	 0effh
ROMEND		 label	 7fffh
__haslcd	 equ	 1
		 include "z8/f6482.inc"

		case	"Z8F6081"
RAMEND		 sfr	 0effh
ROMEND		 label	 0efffh
__haslcd	 equ	 0
		 include "z8/f6482.inc"

		case	"Z8F6082"
RAMEND		 sfr	 0effh
ROMEND		 label	 0efffh
__haslcd	 equ	 1
		 include "z8/f6482.inc"

		case	"Z8F6481"
RAMEND		 sfr	 0effh
ROMEND		 label	 0ffffh
__haslcd	 equ	 0
		 include "z8/f6482.inc"

		case	"Z8F6482"
RAMEND		 sfr	 0effh
ROMEND		 label	 0ffffh
__haslcd	 equ	 1
		 include "z8/f6482.inc"


		elsecase
		 error   "wrong processor type set: only Z8601, Z86[C,E]03, Z8604, Z86(C,E)06, Z8608, Z86C30, Z86C31, Z86C32, Z86C40, Z88C0000, Z88C0020, Z8F0130, Z8F0131, Z8F0230, Z8F0231, Z8F0430, Z8F0431, Z8F0830, Z8F0831, Z8F1232, Z8F1233"
		 error	 "Z8F0113, Z8F0123, Z8F0213, Z8F0223, Z8F0413, Z8F0423, Z8F0813, Z8F0823, Z8F011A, Z8F012A, Z8F021A, Z8F022A, Z8F041A, Z8F042A, Z8F081A, Z8F081A, Z8F0411, Z8F0412, Z8F0421, Z8F0422, Z8F0811, Z8F0812, Z8F0821, Z8F0822"
		 error   "Z8F0880, Z8F1680, Z8F2480"
		 fatal   "allowed!"
                endcase

                if      MOMPASS=1
                 message "Zilog-Z8-SFR-Definitionen (C) 2020 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Constant Memory Addresses

		ifdef	ROMEND		; Device without internal ROM? 
ROMSTART	label	0		; Start Address Internal ROM
		endif

;----------------------------------------------------------------------------

                restore                 ; wieder erlauben

                endif			; __regz8inc
