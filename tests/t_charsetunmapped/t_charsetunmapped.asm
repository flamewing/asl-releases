	cpu 68000
	padding off
	charset '[',
	expect 2300
	dc.b "["
	endexpect
	charset 'a','z',
	expect 2300
	move.b #'a',d0
	endexpect
	dc.b codepage_val(91)+1,codepage_val(97)+1
	charset
	dc.b "[a"
	charset 'A',0
	dc.b "A"
