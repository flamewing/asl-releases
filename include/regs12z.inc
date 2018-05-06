		ifndef	__regs12zinc	; avoid multiple inclusion
__regs12zinc	equ	1

		save
		listing	off		; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGS12Z.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for AVR Processors                    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Bits with the name as given in the datasheets contain the plain bit
; position within the register; with the prefix _bit_..., they hold
; register and bit position

s12zbit		macro   {intlabel},reg,pos
__LABEL__	defbit.ATTRIBUTE	reg,pos
		endm

s12zfld		macro	{intlabel},reg,widthandstart
__LABEL__	defbitfield.ATTRIBUTE	reg,widthandstart
		endm

;----------------------------------------------------------------------------
; include proper CPU-specific register definitions

		if	MOMPASS=1
		  message "NXP S12Z SFR Definitions (C) 2018 Alfred Arnold"
		endif

		switch	MOMCPUNAME

		case "S912ZVC19F0MKH"
		  include "s12z/vc/19f0mkh.inc"
		case "S912ZVC19F0MLF"
		  include "s12z/vc/19f0mlf.inc"
		case "S912ZVCA19F0MKH"
		  include "s12z/vca/19f0mkh.inc"
		case "S912ZVCA19F0MLF"
		  include "s12z/vca/19f0mlf.inc"
		case "S912ZVCA19F0WKH"
		  include "s12z/vca/19f0wkh.inc"
		case "S912ZVH128F2CLQ"
		  include "s12z/vh/128f2clq.inc"
		case "S912ZVH128F2CLL"
		  include "s12z/vh/128f2cll.inc"
		case "S912ZVH64F2CLQ"
		  include "s12z/vh/64f2clq.inc"
		case "S912ZVHY64F1CLQ"
		  include "s12z/vhy/64f1clq.inc"
		case "S912ZVHY32F1CLQ"
		  include "s12z/vhy/32f1clq.inc"
		case "S912ZVHY64F1CLL"
		  include "s12z/vhy/64f1cll.inc"
		case "S912ZVHY32F1CLL"
		  include "s12z/vhy/32f1cll.inc"
		case "S912ZVHL64F1CLQ"
		  include "s12z/vhl/64f1clq.inc"
		case "S912ZVHL32F1CLQ"
		  include "s12z/vhl/32f1clq.inc"
		case "S912ZVHL64F1CLL"
		  include "s12z/vhl/64f1cll.inc"
		case "S912ZVHL32F1CLL"
		  include "s12z/vhl/32f1cll.inc"
		case "S912ZVFP64F1CLQ"
		  include "s12z/vfp/64f1clq.inc"
		case "S912ZVFP64F1CLL"
		  include "s12z/vfp/64f1cll.inc"
		case "S912ZVH128F2VLQ"
		  include "s12z/vh/128f2vlq.inc"
		case "S912ZVH128F2VLL"
		  include "s12z/vh/128f2vll.inc"
		case "S912ZVH64F2VLQ"
		  include "s12z/vh/64f2vlq.inc"
		case "S912ZVHY64F1VLQ"
		  include "s12z/vhy/64f1vlq.inc"
		case "S912ZVHY32F1VLQ"
		  include "s12z/vhy/32f1vlq.inc"
		case "S912ZVHY64F1VL"
		  include "s12z/vhy/64f1vl.inc"
		case "S912ZVHY32F1VLL"
		  include "s12z/vhy/32f1vll.inc"
		case "S912ZVHL64F1VLQ"
		  include "s12z/vhl/64f1vlq.inc"
		  
		elsecase
		  error	"wrong processor type set: only S912ZVH128F2CLQ, S912ZVH128F2CLL, S912ZVH64F2CLQ, S912ZVHY64F1CLQ, S912ZVHY32F1CLQ, S912ZVHY64F1CLL, S912ZVHY32F1CLL, S912ZVHL64F1CLQ, S912ZVHL32F1CLQ, S912ZVHL64F1CLL, S912ZVHL32F1CLL, S912ZVFP64F1CLQ,"
		  fatal	"S912ZVFP64F1CLL, S912ZVH128F2VLQ, S912ZVH128F2VLL, S912ZVH64F2VLQ, S912ZVHY64F1VLQ, S912ZVHY32F1VLQ, S912ZVHY64F1VL, S912ZVHY32F1VLL, S912ZVHL64F1VLQ allowed!"
		endcase

;----------------------------------------------------------------------------

                restore                 ; wieder erlauben

                endif			; __regs12zinc
