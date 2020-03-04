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

__z8bfield	macro	{intlabel},reg,start,count
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
		case	"Z8604"
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
		case	"Z86C30"
RAMEND		 sfr	 0efh
ROMEND		 label	 0fffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		case	"Z86C31"
RAMEND		 sfr	 7fh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
		case	"Z86C32"
RAMEND		 sfr	 0efh
ROMEND		 label	 7ffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		case	"Z86C40"
RAMEND		 sfr	 0efh
ROMEND		 label	 0fffh
__hastimer1	 equ	 1
__haswdt	 equ	 0
__hasspi	 equ	 0
		case	"Z88C0000"
RAMEND		 sfr	 0bfh
		 include "z8/z88c00xx.inc"
		case	"Z88C0020"
RAMEND		 sfr	 0bfh
ROMEND		 label	 1fffh
		 include "z8/z88c00xx.inc"
		elsecase
		 error   "wrong processor type set: only Z8601, Z86[C,E]03, Z8604, Z86(C,E)06, Z8608, Z86C30, Z86C31, Z86C32, Z86C40, Z88C0000, Z88C0020"
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
; Constant Vectors

RESET_vect	label   0               ; Reset Entry

;----------------------------------------------------------------------------

                restore                 ; wieder erlauben

                endif			; __regz8inc
