		ifndef	__ee90inc
__ee90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File EE90.INC                                                *
;*                                                                          *
;*   Contains Bit & Register Definitions for EEPROM section on AT90Sxxxx    *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; EEPROM

		if E2END>=256
EEARL		 port	 0x1e		; EEPROM Address Register
EEARH		 port	 0x1f
		elseif
EEAR		 port	 0x1e
		endif

EEDR		port	0x1d		; EEPROM Data Register

EECR            port	0x1c		; EEPROM Control Register
EERE		avrbit	EECR,0		; Read Enable
EEWE		avrbit	EECR,1		; Write Enable

		restore			; re-enable listing

		endif			; __ee90inc
