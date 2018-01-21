		ifndef	__eeminc
__eeminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File EEM.INC                                                 *
;*                                                                          *
;*   Contains Bit & Register Definitions for EEPROM on Older ATmegas        *
;*                                                                          *
;****************************************************************************

		include	"avr/ee90.inc"

EEMWE		avrbit	EECR,2		; EEPROM Master Write Enable
EERIE		avrbit	EECR,3		; Enable Ready Interrupt

		restore			; re-enable listing

		endif			; __eeminc
