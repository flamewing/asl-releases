	cpu 68000
	padding off

	; Unmatched terminators in inactive conditional branches are ignored.
	if 0
	endm
	endr
	if 1
	endm
	endif
	else
	dc.b 1
	endif

	if 1
	dc.b 2
	else
	endm
	endr
	endif

	; Inactive definitions and repetitions still collect their complete body.
	if 0
unused macro
	rept 2
	dc.b 99
	endm
	endm
	rept 2
	dc.b 99
	endr
	endif

	; Active misplaced terminators must continue reporting an error.
	expect 1808,1808
	endm
	endr
	endexpect
	dc.b 3
