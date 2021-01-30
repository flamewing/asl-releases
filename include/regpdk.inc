                ifndef  __regpadaukinc     ; avoid multiple inclusion
__regpadaukinc	equ     1

                save
                listing off   		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGPADAUK.INC                                           *
;*                                                                          *
;*   Sinn : contains SFR and Bit Definitionen for Padauk MCUs               *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Helper Macros

_bfield		macro	{INTLABEL},reg,start,count
__LABEL__	equ	((1<<(count+start))-1)-((1<<start)-1)
		endm

;----------------------------------------------------------------------------
; include proper CPU-specific register definitions

		switch	MOMCPUNAME
		case	"PMC150","PMS150"
		  include "pdk/pmx150.inc"

		case	"PMC131","PMS131","PMS130"
		  include "pdk/pmx130.inc"

		case	"PMS132","PMS132B"
		  include "pdk/pms132.inc"

		case	"PMS133","PMS134"
		  include "pdk/pms133.inc"

		case	"PMS152"
		  include "pdk/pms152.inc"

		case	"PFS154","PMS154B","PMS154C"
		  include "pdk/pxs154.inc"

		case	"PFS173"
		  include "pdk/pfs173.inc"

		case	"PMS232","PMC232"
		  include "pdk/pmx232.inc"

		case	"PMS234","PMC234"
		  include "pdk/pmx234.inc"

		case	"PMC251"
		  include "pdk/pmc251.inc"

		case	"PMC271","PMS271"
		  include "pdk/pmx271.inc"

		case	"PMC884","MCS11"
		  include "pdk/pmc884.inc"

		case	"DF69"
		  include  "pdk/df69.inc"

		elsecase
		  error  "wrong processor type set: only PMC150, PMS150, PMC131, PMS131, PMS130, PMS132, PMS132B,"
		  error  "PMS133, PMS134, PMS152, PFS154, PMS154B, PMS154C, PFS173, PMS232, PMC232, PMS234,"
                  fatal  "PMC234, PMC251, PMC271, PMS271, PMC884, MCS11, DF69 allowed."
                endcase

                if      MOMPASS=1
                 message "Padauk SFR Definitions (C) 2020 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Common Stuff in CPU Core

flag		sfr	0x00		; ACC Status Flag Register
OV		bit	io(flag).3	;  Overflow
AC		bit	io(flag).3	;  Auxiliary Carry
C		bit	io(flag).1	;  Carry
CF		bit	C
Z		bit	io(flag).0	;  Zero

sp		sfr	0x02		; Stack Pointer

		ifndef	__numcpus
__numcpus	equ	1
		endif

		if	__numcpus>1
RESET0_vect	 label	0x0000		; Reset FPP0
RESET1_vect	 label	0x0001		; Reset FPP1
		elseif
RESET_VECT	 label	0x0000		; Reset
		endif
		if	__numcpus>7
RESET2_vect	 label	0x0002		; Reset FPP2
RESET3_vect	 label	0x0003		; Reset FPP3
RESET4_vect	 label	0x0004		; Reset FPP4
RESET5_vect	 label	0x0005		; Reset FPP5
RESET6_vect	 label	0x0006		; Reset FPP6
RESET7_vect	 label	0x0007		; Reset FPP7
		endif
INT_vect	label	0x0010		; Interrupt

		if	__numcpus>1
fppen		 sfr	0x01		; FPP Unit Enable Register
gdio		 sfr	0x07		; General Data for I/O
		endif

		if	__numcpus>7
fppres		 sfr	0x3f		; FPPA Reset Register
		endif

;----------------------------------------------------------------------------

                restore                 ; allow listing again

                endif			; __regpadaukinc
