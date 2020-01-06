		ifndef	__st7adc10inc	; avoid multiple inclusion
__st7adc10inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADC10.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx 10 Bit ADC                *
;*                                                                          *
;****************************************************************************

__defadc10	macro	Base
ADCCSR		label	Base+$00	; Control/Status Register
EOC		bit	ADCCSR,7	;  End Of Conversion
SPEED		bit	ADCCSR,6	;  ADC Clock Selection
ADON		bit	ADCCSR,5	;  A/D Converter On
CH3		bit	ADCCSR,3	;  Channel Selection
CH2		bit	ADCCSR,2
CH1		bit	ADCCSR,1
CH0		bit	ADCCSR,0
ADCDRH		label	Base+$01	; Data Register High
ADCDRL		label	Base+$02	; Data Register Low
		endm

		restore
		endif			; __st7adc10inc
