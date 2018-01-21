		ifndef	__usimb8inc
__usimb8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File USIMB8.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega USI Interface @ 0xb8    *
;*                                                                          *
;****************************************************************************

USIDR		sfr	0xba		; USI Data Register

USISR		sfr	0xb9		; USI Status Register
USICNT0		avrbit	USISR,0		; Counter Value
USICNT1		avrbit	USISR,1
USICNT2		avrbit	USISR,2
USICNT3		avrbit	USISR,3
USIDC		avrbit	USISR,4		; Data Output Collision
USIPF		avrbit	USISR,5		; Stop Condition Flag
USIOIF		avrbit	USISR,6		; Counter Overflow Interrupt Flag
USISIF		avrbit	USISR,7		; Start Condition Interrupt Flag

USICR		sfr	0xb8		; USI Control Register
USITC		avrbit	USICR,0		; Toggle Clock Port Pin
USICLK		avrbit	USICR,1		; Clock Strobe
USICS0		avrbit	USICR,2		; Clock Source Select
USICS1		avrbit	USICR,3
USIWM0		avrbit	USICR,4		; Wire Mode
USIWM1		avrbit	USICR,5
USIOIE		avrbit	USICR,6		; Counter Overflow Interrupt Enable
USISIE		avrbit	USICR,7		; Start Condition Interrupt Enable

		restore			; re-enable listing

		endif			; __usimb8inc
