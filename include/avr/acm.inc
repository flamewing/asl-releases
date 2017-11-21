		ifndef	__acminc
__acminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM.INC                                                 *
;*                                                                          *
;*   Contains bit & register definitions for ATmega Analog Comparator       *
;*                                                                          *
;****************************************************************************

		include	"avr/ac90.inc"

ACBG		equ	6		; bandgap select

		restore			; re-enable listing

		endif			; __acminc
