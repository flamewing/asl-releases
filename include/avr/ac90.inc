		ifndef	__ac90inc
__ac90inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File AC90.INC                                                *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT90Sxxxx Analog Comparator    *
;*                                                                          *
;****************************************************************************

ACSR		port	0x08            ; Analog Comparator ControlStatus Register
ACIS0		avrbit	ACSR,0		; Interrupt-Mode
ACIS1		avrbit	ACSR,1
		ifdef	TCNT1L		; if Timer 1 present...
ACIC	         avrbit	 ACSR,2		; ...use Comparator as Capture Signal for Timer 1?
		endif
ACIE		avrbit	ACSR,3		; Interrupt Enable
ACI		avrbit	ACSR,4		; Interrupt Flag
ACO		avrbit	ACSR,5		; Analog Comparator Output
ACD		avrbit	ACSR,7		; Disable

		restore			; re-enable listing

		endif			; __ac90inc
