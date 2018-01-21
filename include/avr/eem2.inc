		ifndef	__eem2inc
__eem2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File EEM2.INC                                                *
;*                                                                          *
;*   Contains Bit & Register Definitions for EEPROM on newer ATmegas        *
;*                                                                          *
;****************************************************************************

		if E2END>=256
EEARL		port	0x21		; EEPROM Address Register
EEARH		port	0x22
		elseif
EEAR		port	0x21
		endif

EEDR		port	0x20		; EEPROM Data Register

EECR            port    0x1f		; EEPROM Control Register
EERE            avrbit	EECR,0		; Read Enable
EEPE            avrbit	EECR,1		; Program Enable
EEMPE		avrbit	EECR,2		; EEPROM Master Program Enable
EERIE		avrbit	EECR,3		; Enable Ready Interrupt

		restore			; re-enable listing

		endif			; __eem2inc
