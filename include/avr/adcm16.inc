		ifndef	__adcm16inc
__adcm16inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM16.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega16-like A/D Converter    *
;*                                                                          *
;****************************************************************************

ADMUX		port	0x07		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX4		avrbit	ADMUX,4
MUX3		avrbit	ADMUX,3		; Multiplexer
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSRA		port	0x06		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

ACME		avrbit	SFIOR,3		; Analog Comparator Mux Enable

		restore			; re-enable listing

		endif			; __adcm16inc
