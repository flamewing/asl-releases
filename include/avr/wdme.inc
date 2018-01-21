		ifndef	__wdmeinc
__wdmeinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File WDME.INC                                                *
;*                                                                          *
;*   Contains Bit & Register Definitions for Enhanced Watchdog              *
;*                                                                          *
;****************************************************************************

WDTCSR		sfr	0x60		; Watchdog Control/Status Register
WDP0		avrbit	WDTCSR,0	; Prescaler
WDP1		avrbit	WDTCSR,1
WDP2		avrbit	WDTCSR,2
WDE		avrbit	WDTCSR,3	; Enable Watchdog
WDCE		avrbit	WDTCSR,4	; Change Enable
WDP3		avrbit	WDTCSR,5
WDIE		avrbit	WDTCSR,6	; Interrupt Enable
WDIF		avrbit	WDTCSR,7	; Interrupt Flag

		restore			; re-enable listing

		endif			; __wdmeinc
