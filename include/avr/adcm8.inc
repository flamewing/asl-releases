		ifndef	__adcm8inc
__adcm8inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM8.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega8 A/D Converter          *
;*                                                                          *
;****************************************************************************

ADMUX		port	0x07		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX3		avrbit	ADMUX,3		; Multiplexer
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x06		; Control/Status Register
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADFR		avrbit	ADCSR,5		; Free Running Select
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

ADCH		port	0x05		; Data Register
ADCL		port	0x04

ACME		avrbit	SFIOR,3		; Analog Comparator Multiplexer Enable

		restore			; re-enable listing

		endif			; __adcm8inc
