		ifndef	__acminc
__acminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM.INC                                                 *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega Analog Comparator       *
;*                                                                          *
;****************************************************************************

		include	"avr/ac90.inc"

ACBG		avrbit	ACSR,6		; Bandgap Select

		restore			; re-enable listing

		endif			; __acminc
