	cpu 68000
	padding off

	; String mappings must preserve unsigned bytes, including zero and bit 7.
	charset 65,"\x00\x7f\x80\xff"
	dc.b "ABCD"
	dc.b 'C','D'
	charset
	dc.b "ABCD"
