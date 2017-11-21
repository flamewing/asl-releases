		ifndef	__eem2inc
__eem2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSKEL.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for EEPROM on newer ATmegas        *
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
EERE            equ     0		; Read Enable
EEPE            equ     1		; Program Enable
EEMPE		equ	2		; EEPROM Master Program Enable
EERIE		equ	3		; enable ready interrupt

		restore			; re-enable listing

		endif			; __eem2inc
