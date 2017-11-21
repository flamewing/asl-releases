		ifndef	__acm30inc
__acm30inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM30.INC                                               *
;*                                                                          *
;*  Contains bit & register definitions for ATmega Analog Comparator @ 0x30 *
;*                                                                          *
;****************************************************************************

ACSR		port    0x30            ; Config/Status Register
ACIS0		equ     0               ; Interrupt-Mode
ACIS1		equ     1
ACIC	 	equ	2		; use Comparator als capture signal for Timer 1?
ACIE		equ     3               ; Interrupt Enable
ACI		equ     4               ; Interrupt Flag
ACO		equ     5               ; Analog Comparator Output
ACBG		equ	6		; bandgap select
ACD		equ	7		; Disable

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
AIN1D		equ	1		; disable digital input on AIN0
AIN0D		equ	0		; disable digital input on AIN1

		restore			; re-enable listing

		endif			; __acm30inc
