	cpu	8086

	; NOTE: The test values commented out work
        ; on a "typical" x86 system, but may not on
        ; others due to rounding errors when converting
        ; ASCII->binary

;	dw	5.96046447754e-8	; smallest positive subnormal number (01 00)
;	dw	0.000060975552		; largest subnormal number (ff 03)
;	dw	0.000061035156		; smallest positive normal number (00 04)
	dw	65504.0			; largest normal number (ff 7b)
;	dw	0.99951172		; largest number less than one (ff 3b)
	dw	1.0			; one (00 3c)
;	dw	1.00097656		; smallest number larger than one (01 3c)
;	dw	0.33333333333333	; the rounding of 1/3 to nearest (55 35)
	dw	-2.0			; minus two (00 c0)
	dw	0.0			; zero (00 00)

;	dw	1.7e308*100		; positive infinity (00 7c)
;	dw	-1.7e308*100		; negative infinity (00 fc)
;	dw	sqrt(-2)		; NaN (00 fe, but won't assemble anyway...)

