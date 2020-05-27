	cpu	z80

x1	equ	1
x3	equ	3

	if	defined(x1) || defined(x2)
	db	12h
	endif

	if	defined(x1) || defined(x3)
	db	13h
	endif

	if	defined(x1) || defined(x4)
	db	14h
	endif

	if	defined(x2) || defined(x3)
	db	23h
	endif

	if	defined(x2) || defined(x4)
	db	24h
	endif

	if	defined(x3) || defined(x4)
	db	34h
	endif
