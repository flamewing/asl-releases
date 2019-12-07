		ifndef	__wdginc	; avoid multiple inclusion
__wdginc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WDG.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx Watchdog Timer             *
;*                                                                          *
;****************************************************************************

WDGR		sfr	0d8h		; Watchdog Register
C		bit	0,WDGR		;  Watchdog Control Bit
SR		bit	1,WDGR		;  Software Reset

		restore
		endif			; __wdginc
