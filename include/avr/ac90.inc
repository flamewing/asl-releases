		ifndef	__ac90inc
__ac90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File AC90.INC                                                *
;*                                                                          *
;*   Contains bit & register definitions for AT90Sxxxx Analog Comparator    *
;*                                                                          *
;****************************************************************************

ACSR		port    0x08            ; Komparator-Steuer/Statusregister:
ACIS0		equ     0               ;  Interrupt-Mode
ACIS1		equ     1
		ifdef	TCNT1L		; if timer 1 present...
ACIC	         equ     2              ;  ...use Comparator als capture signal for Timer 1?
		endif
ACIE		equ     3               ;  Interrupt Enable
ACI		equ     4               ;  Interrupt Flag
ACO		equ     5               ;  Analog Comparator Output
ACD		equ	7		;  Disable

		restore			; re-enable listing

		endif			; __ac90inc
