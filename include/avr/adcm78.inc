		ifndef	__adcm78inc
__adcm78inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM78.INC                                              *
;*                                                                          *
;*   Contains bit & register definitions for ATmega A/D Converter @ 0x78    *
;*                                                                          *
;****************************************************************************

ADMUX		sfr	0x7c		; Multiplexer Selection
REFS1		equ	7		; reference selection bits
REFS0		equ	6
ADLAR		equ	5		; left adjust right
MUX3		equ	3		; multiplexer
MUX2		equ	2
MUX1		equ	1
MUX0		equ	0

ADCSRA		sfr	0x7a		; Control/Status Register A
ADEN		equ	7		; enable ADC
ADSC		equ	6		; start conversion
ADATE		equ	5		; Auto Trigger Enable
ADIF		equ	4		; interrupt flag
ADIE		equ	3		; interrupt enable
ADPS2		equ	2		; prescaler select
ADPS1		equ	1
ADPS0		equ	0

ADCSRB		sfr	0x7b		; Control/Status Register B
ACME		equ	6		; analog multiplexer comparator enable
ADTS2		equ	2		; auto trigger source
ADTS1		equ	1
ADTS0		equ	0

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
ADC0D		equ	0		; disable digital input on ADC0
ADC1D		equ	1		; disable digital input on ADC1
ADC2D		equ	2		; disable digital input on ADC2
ADC3D		equ	3		; disable digital input on ADC3
ADC4D		equ	4		; disable digital input on ADC4
ADC5D		equ	5		; disable digital input on ADC5
ADC6D		equ	6		; disable digital input on ADC6
ADC7D		equ	7		; disable digital input on ADC7

ADCH		sfr	0x79		; Data Register
ADCL		sfr	0x78

		restore			; re-enable listing

		endif			; __adcm78inc
