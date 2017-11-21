		ifndef	__wdm21inc
__wdm21inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGWDM21.INC                                            *
;*                                                                          *
;*   Contains bit & register definitions for Watchdog @ 0x21                *
;*                                                                          *
;****************************************************************************

WDTCR		port	0x21		; Watchdog Control Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog

		restore			; re-enable listing

		endif			; __wdm21inc
