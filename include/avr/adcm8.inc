		ifndef	__adcm8inc
__adcm8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM8.INC                                               *
;*                                                                          *
;*   Contains bit & register definitions for ATmega8 A/D converter          *
;*                                                                          *
;****************************************************************************

ADMUX		port	0x07		; Multiplexer Selection
REFS1		equ	7		; reference selection bits
REFS0		equ	6
ADLAR		equ	5		; left adjust right
MUX3		equ	3		; multiplexer
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSR		port	0x06		; control/status register
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADFR		equ	5		; free running select
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		; bits in SFIOR
ACME		equ	3		; analog comparator multiplexer enable

		restore			; re-enable listing

		endif			; __adcm8inc
