		ifndef	__19f0wkhinc
__19f0wkhinc	equ	1
		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 19F0WKH.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S912ZVCA19F0WKH                *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2START		equ	$100000		; 2 KByte EEPROM
E2END		equ	$1007ff
RAMSTART	equ	$001000		; 12 KByte RAM
RAMEND		equ	$003fff
FLASHSTART	equ	$fd0000		; 192 KByte Flash ROM
FLASHEND	equ	$ffffff

__housing	equ	64
__vca		equ	1
		include	"s12z/vc/common.inc"

		restore			; re-enable listing

		endif			; __19f0wkhinc
