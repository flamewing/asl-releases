		ifndef	__wdm60inc
__wdm60inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WDM60.INC                                               *
;*                                                                          *
;*   Contains bit & register definitions for watchdog @ 0x60                *
;*                                                                          *
;****************************************************************************

WDTCR		sfr	0x60		; Watchdog Control Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDCE		equ	4		; change enable

		restore			; re-enable listing

		endif			; __wdm60inc
