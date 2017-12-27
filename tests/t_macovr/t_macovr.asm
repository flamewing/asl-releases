	cpu	z80

	; macro expansion off for macros defined in succession

	macexp_dft off

delay	macro	n
	ld	c,n
loop	nop
	djnz	loop
	endm

	; won't be expanded in listing

	delay	10

	; force expansion of complete macro body

	macexp_ovr on

	; will be expanded in listing

	delay	20

	; take back override

	macexp_ovr

	; won't be expanded in listing

	delay	30
