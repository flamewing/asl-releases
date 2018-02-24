	cpu	8080

	; note: CHARSET also acts on arguments of CHARSET itself,
        ; so 'charset 'a','z',1 | charset 'd',255 won't work as expected.

	charset	'a','c',1
	charset	'd',255
	charset	'e','z',5
	db	"drei droege drachen drehen durch"

	; alternate way of doing it: 100 is ASCII code of 'd'

	charset
	charset	'a','z',1
	charset	100,255
	db	"drei droege drachen drehen durch"

	; back to normality:

	charset
	db	"drei droege drachen drehen durch"
