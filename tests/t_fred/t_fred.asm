		cpu	8086
		page	0

i8		equ	5h		; 4 bits
i8c_1		equ	~5h		; -> 0ah
i8c_2		equ	~i8
i8c_3		equ	~i8c_2		; -> 5h
		dd	i8c_1, i8c_3	; 0A 00 00 00  05 00 00 00

i16		equ	55h		; 8 bits
i16c_1		equ	~55h		; -> 0aah
i16c_2		equ	~i16
i16c_3		equ	~i16c_2		; -> 55h
		dd	i16c_1, i16c_3	; AA 00 00 00  55 00 00 00

i24		equ	555555h		; 24 bits
i24c_1		equ	~555555h	; -> 0aaaaaah
i24c_2		equ	~i24
i24c_3		equ	~i24c_2		; -> 555555h
		dd	i24c_1, i24c_3	; AA AA AA 00  55 55 55 00

		; For Intel hex syntax, an additional leading zero has
		; to be added if the constant would otherwise start
		; with a letter.  This extra leading zero does not
		; account to the number of digits:

i12		equ	0aaah		; 12 bits
i12_2		equ	~i12		; -> 555h
i12a		equ	0555h		; 16 bits!
i12a_2		equ	~i12a		; -> 0faaah
		dd	i12_2, i12a_2	; 55 05 00 00  AA FA 00 00

		; similar for octal constants

i6		equ	5o		; 3 bits
i6c_1		equ	~5o		; -> 2o
i6c_2		equ	~i6
i6c_3		equ	~i6c_1		; -> 5
		dd	i6c_1, i6c_3	; 02 00 00 00  05 00 00 00

		; no bit length deduction for decimal constants

i10		equ	15		; n bits
i10_1		equ	~15		; -> 0ffff...fff0h
i10_2		equ	~i10
i10_3		equ	~i10_1		; -> 15
		dd	i10_1&0ffffffffh, i10_3	; F0 FF FF FF  0F 00 00 00

