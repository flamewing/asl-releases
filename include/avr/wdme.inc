		ifndef	__wdmeinc
__wdmeinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSKEL.INC                                             *
;*                                                                          *
;*   Contains bit & register definitions for enhanced Watchdog              *
;*                                                                          *
;****************************************************************************

WDTCSR		sfr	0x60		; Watchdog Control/Status Register
WDP0		equ	0		; Prescaler
WDP1		equ	1
WDP2		equ	2
WDE		equ	3		; Enable watchdog
WDCE		equ	4		; change enable
WDP3		equ	5
WDIE		equ	6		; interrupt enable
WDIF		equ	7		; interrupt flag

		restore			; re-enable listing

		endif			; __wdmeinc
