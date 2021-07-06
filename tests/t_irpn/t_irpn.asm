	cpu	68000
	page	0

	; basic 68000 modes

	expect 1807
		irpn 0
			dc.w	0
		endm
	endexpect

	irpn 1,val0,0,1,2,3
		dc.w	val0
	endm

	irpn 2,val0,val1,0,1,2,3
		dc.l	val0
		dc.w	val1
	endm

	irpn 3,val0,val1,val2,0,1,2,3
		dc.l	val0
		if "val1"<>""
			dc.w	val1,val2
		endif
	endm
	END
