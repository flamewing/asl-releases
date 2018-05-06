		ifndef	__19f0mkhinc
__19f0mkhinc	equ	1
		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 19F0MKH.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S912ZVCA19F0MKH                *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2START		label	$100000		; 2 KByte EEPROM
E2END		label	$1007ff
RAMSTART	label	$001000		; 12 KByte RAM
RAMEND		label	$003fff
FLASHSTART	label	$fd0000		; 192 KByte Flash ROM
FLASHEND	label	$ffffff

__housing	equ	64
__vca		equ	1
		include	"s12z/vc/common.inc"

		restore				; re-enable listing

		endif				; __19f0mkhinc
