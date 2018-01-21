		ifndef	__wdm60inc
__wdm60inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WDM60.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for Watchdog @ 0x60                *
;*                                                                          *
;****************************************************************************

WDTCR		sfr	0x60		; Watchdog Control Register
WDP0		avrbit	WDTCR,0		; Prescaler
WDP1		avrbit	WDTCR,1
WDP2		avrbit	WDTCR,2
WDE		avrbit	WDTCR,3		; Enable Watchdog
WDCE		avrbit	WDTCR,4		; Change Enable

		restore			; re-enable listing

		endif			; __wdm60inc
