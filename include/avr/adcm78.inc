		ifndef	__adcm78inc
__adcm78inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADCM78.INC                                              *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega A/D Converter @ 0x78    *
;*                                                                          *
;****************************************************************************

ADMUX		sfr	0x7c		; Multiplexer Selection
REFS1		avrbit	ADMUX,7		; Reference Selection Bits
REFS0		avrbit	ADMUX,6
ADLAR		avrbit	ADMUX,5		; Left Adjust Right
MUX3		avrbit	ADMUX,3		; Multiplexer
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSRA		sfr	0x7a		; Control/Status Register A
ADEN		avrbit	ADCSRA,7	; Enable ADC
ADSC		avrbit	ADCSRA,6	; Start Conversion
ADATE		avrbit	ADCSRA,5	; Auto Trigger Enable
ADIF		avrbit	ADCSRA,4	; Interrupt Flag
ADIE		avrbit	ADCSRA,3	; Interrupt Enable
ADPS2		avrbit	ADCSRA,2	; Prescaler Select
ADPS1		avrbit	ADCSRA,1
ADPS0		avrbit	ADCSRA,0

ADCSRB		sfr	0x7b		; Control/Status Register B
ACME		avrbit	ADCSRB,6	; Analog Multiplexer Comparator Enable
ADTS2		avrbit	ADCSRB,2	; Auto Trigger Source
ADTS1		avrbit	ADCSRB,1
ADTS0		avrbit	ADCSRB,0

DIDR0		sfr	0x7e		; Digital Input Disable Register 0
ADC0D		avrbit	DIDR0,0		; Disable Digital Input on ADC0
ADC1D		avrbit	DIDR0,1		; Disable Digital Input on ADC1
ADC2D		avrbit	DIDR0,2		; Disable Digital Input on ADC2
ADC3D		avrbit	DIDR0,3		; Disable Digital Input on ADC3
ADC4D		avrbit	DIDR0,4		; Disable Digital Input on ADC4
ADC5D		avrbit	DIDR0,5		; Disable Digital Input on ADC5
ADC6D		avrbit	DIDR0,6		; Disable Digital Input on ADC6
ADC7D		avrbit	DIDR0,7		; Disable Digital Input on ADC7

ADCH		sfr	0x79		; Data Register
ADCL		sfr	0x78

		restore			; re-enable listing

		endif			; __adcm78inc
