	cpu 8080
	codepage aaa
	expect 375
	db codepage_val(65)
	endexpect
	charset 65,0
	db codepage_val(65)

; Explicit duplication must preserve the exemption from fresh-page warnings.
	codepage zzz,standard
	db codepage_val(65)

; A named read must check that page rather than the currently selected page.
	codepage standard
	codepage bbb
	codepage standard
	expect 375
	db codepage_val(65,"bbb")
	endexpect
	db codepage_val(65,"BBB")
