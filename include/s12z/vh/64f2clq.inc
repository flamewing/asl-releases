		ifndef	__64f2clqinc
__64f2clqinc	equ	1
		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 64F2CLQ.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S912ZVH64F2CLQ                 *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

E2START		label	$100000		; 4 KByte EEPROM
E2END		label	$100fff
RAMSTART	label	$001000		; 4 KByte RAM
RAMEND		label	$001fff
FLASHSTART	label	$ff0000		; 64 KByte Flash ROM
FLASHEND	label	$ffffff

		restore			; re-enable listing

		endif			; __64f2clqinc
