		ifndef	__acm2inc
__acm2inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM.INC                                                 *
;*                                                                          *
;* Contains bit & register definitions for 2nd Gen ATmega Analog Comparator *
;*                                                                          *
;****************************************************************************

		include	"avr/ac90.inc"

AINBG		equ	6		; bandgap select

		restore			; re-enable listing

		endif			; __acm2inc
