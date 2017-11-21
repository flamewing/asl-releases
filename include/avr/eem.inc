		ifndef	__eeminc
__eeminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSKEL.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for EEPROM on older ATmegas        *
;*                                                                          *
;****************************************************************************

		include	"avr/ee90.inc"

		; additional bits in EECR
EEMWE		equ	2		; EEPROM Master Write Enable
EERIE		equ	3		; enable ready interrupt

		restore			; re-enable listing

		endif			; __eeminc
