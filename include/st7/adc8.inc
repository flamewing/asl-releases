		ifndef	__st7adc8inc	; avoid multiple inclusion
__st7adc8inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADC8.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx 8 Bit ADC                 *
;*                                                                          *
;****************************************************************************

ADCDR		label	$0070		; Data Register
ADCCSR		label	$0071		; Control/Status Register
COCO		bit	ADCCSR,7	;  Conversion Complete
ADON		bit	ADCCSR,5	;  A/D converter On
CH2		bit	ADCCSR,2	;  Channel Selection
CH1		bit	ADCCSR,1
CH0		bit	ADCCSR,0

		restore
		endif			; __st7adc8inc
