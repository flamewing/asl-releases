		ifndef	__adcinc	; avoid multiple inclusion
__adcinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADC.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx A/D Converter              *
;*                                                                          *
;****************************************************************************

ADR		sfr	0d0h		; A/D Converter Data Register
ADCR		sfr	0d1h		; A/D Converter Control Register
EAI		bit	7,ADCR		;  Enable A/D Interrupt
EOC		bit	6,ADCR		;  End of Conversion
STA		bit	5,ADCR		;  Start of Conversion
PDS		bit	4,ADCR		;  Power Down Selection

		restore
		endif			; __adcinc
