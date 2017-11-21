		ifndef	__usimb8inc
__usimb8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File USIMB8.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATmega USI Interface @ 0xb8    *
;*                                                                          *
;****************************************************************************

USIDR		sfr	0xba		; USI Data Register

USISR		sfr	0xb9		; USI Status Register
USICNT0		equ	0		; Counter Value
USICNT1		equ	1
USICNT2		equ	2
USICNT3		equ	3
USIDC		equ	4		; Data Output Collision
USIPF		equ	5		; Stop Condition Flag
USIOIF		equ	6		; Counter Overflow Interrupt Flag
USISIF		equ	7		; Start Condition Interrupt Flag

USICR		sfr	0xb8		; USI Control Register
USITC		equ	0		; Toggle Clock Port Pin
USICLK		equ	1		; Clock Strobe
USICS0		equ	2		; Clock Source Select
USICS1		equ	3
USIWM0		equ	4		; Wire Mode
USIWM1		equ	5		
USIOIE		equ	6		; Counter Overflow Interrupt Enable
USISIE		equ	7		; Start Condition Interrupt Enable

		restore			; re-enable listing

		endif			; __usimb8inc
