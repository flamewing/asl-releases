		ifndef	__wdm21inc
__wdm21inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGWDM21.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for Watchdog @ 0x21                *
;*                                                                          *
;****************************************************************************

WDTCR		port	0x21		; Watchdog Control Register
WDP0		avrbit	WDTCR,0		; Prescaler
WDP1		avrbit	WDTCR,1
WDP2		avrbit	WDTCR,2
WDE		avrbit	WDTCR,3		; Enable Watchdog

		restore			; re-enable listing

		endif			; __wdm21inc
