		ifndef	__acm2inc
__acm2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM.INC                                                 *
;*                                                                          *
;* Contains Bit & Register Definitions for 2nd Gen ATmega Analog Comparator *
;*                                                                          *
;****************************************************************************

		include	"avr/ac90.inc"

AINBG		avrbit	ACSR,6		; Bandgap Select

		restore			; re-enable listing

		endif			; __acm2inc
