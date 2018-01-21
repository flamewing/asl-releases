		ifndef	__acm30inc
__acm30inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ACM30.INC                                               *
;*                                                                          *
;*  Contains Bit & Register Definitions for ATmega Analog Comparator @ 0x30 *
;*                                                                          *
;****************************************************************************

ACSR		port    0x30            ; Config/Status Register
ACIS0		avrbit	ACSR,0		; Interrupt-Mode
ACIS1		avrbit	ACSR,1
ACIC	 	avrbit	ACSR,2		; Use Comparator as Capture Signal for Timer 1?
ACIE		avrbit	ACSR,3		; Interrupt Enable
ACI		avrbit	ACSR,4		; Interrupt Flag
ACO		avrbit	ACSR,5		; Analog Comparator Output
ACBG		avrbit	ACSR,6		; Bandgap Select
ACD		avrbit	ACSR,7		; Disable

DIDR1		sfr	0x7f		; Digital Input Disable Register 1
AIN1D		avrbit	DIDR1,1		; Disable Digital Input on AIN0
AIN0D		avrbit	DIDR1,0		; Disable Digital Input on AIN1

		restore			; re-enable listing

		endif			; __acm30inc
