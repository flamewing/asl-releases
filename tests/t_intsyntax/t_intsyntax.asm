	cpu	sc/mp

	; by default, SC/MP uses C notation, i.e. the following
	; is interpreted as octal:

	ldi		044	; 44 octal -> 36 decinal

	; now disable the C-like octal syntax, which means parsing
	; falls back to default decimal:

	intsyntax	-0oct
	ldi		044	; 44 decimal

	; now, enable the National-specific hex syntax with leading zero:

	intsyntax	+0hex
	ldi		044	; 44 hex -> 68 decimal

	; C-octal and National-hex cannot be enabled at once:

	expect	2231
	intsyntax	+0oct
	endexpect

