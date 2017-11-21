		ifndef	__adcm16inc
__adcm16inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM16.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATmega16-like A/D Converter    *
;*                                                                          *
;****************************************************************************

ADMUX		port	0x07		; Multiplexer Selection
REFS1		equ	7		; reference selection bits
REFS0		equ	6
ADLAR		equ	5		; left adjust right
MUX4		equ	4
MUX3		equ	3		; multiplexer
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; Auto Trigger Enable
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

		; bits in SFIOR
ACME		equ	3		; analog comparator mux enable

		restore			; re-enable listing

		endif			; __adcm16inc
