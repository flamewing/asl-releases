	cpu 68000
	padding off

; A one-character range translating to zero must not underflow.
	charset 'A',0
	dc.b "AB"
	charset ' ',0
	dc.b " A"

; A range whose last output is 255 must be accepted.
	charset 0,255,0
	dc.b "A "
	charset 1,255,1
	dc.b "AB"
