		ifndef	__128f2vllinc
__128f2vllinc	equ	1
		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 128F2VLL.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for S912ZVH128F2VLL                *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2START		label	$100000		; 4 KByte EEPROM
E2END		label	$100fff
RAMSTART	label	$001000		; 8 KByte RAM
RAMEND		label	$002fff
FLASHSTART	label	$fe0000		; 128 KByte Flash ROM
FLASHEND	label	$ffffff

		restore			; re-enable listing

		endif			; __128f2vllinc
