	cpu	atmega8
	page	0

	nop

	expect	360
	dn	?
	endexpect
	expect	360
	dn	?,?
	endexpect
	expect	360
	dn	2 dup (?)
	endexpect
	expect	360
	dn	?,?,?
	endexpect
	expect	360
	dn	3 dup (?)
	endexpect
	dn	?,?,?,?
	dn	4 dup (?)

	expect	360
	db	?
	endexpect
	db	?,?
	db	2 dup (?)
	expect	360
	db	?,?,?
	endexpect
	expect	360
	db	3 dup (?)
	endexpect

	data	1,2,3

	expect	360
	dn	1
	endexpect
	expect	360
	dn	1,2
	endexpect
	expect	360
	dn	1,2,3
	endexpect
	dn	1,2,3,4
	expect	360
	dn	2 dup (1)
	endexpect
	dn	2 dup (1,2)
	expect	360
	dn	2 dup (1,2,3)
	endexpect
	dn	2 dup (1,2,3,4)
	expect	360
	dn	3 dup (1)
	endexpect
	expect	360
	dn	3 dup (1,2)
	endexpect
	expect	360
	dn	3 dup (1,2,3)
	endexpect
	dn	3 dup (1,2,3,4)
	dn	2 dup (1,2 dup (2,9,12,2,9,1,2,9,15,2),3)

	expect	360
	db	1
	endexpect
	db	1,2
	expect	360
	db	1,2,3
	endexpect
	db	1,2,3,4
	db	2 dup (1)
	expect	360
	db	3 dup (1)
	endexpect
	db	2 dup (1,2)
	db	3 dup (1,2)
	db	2 dup (1,2,3)
	expect	360
	db	3 dup (1,2,3)
	endexpect
	db	2 dup (1,2,3,4)
	db	3 dup (1,2,3,4)
	db	2 dup (1,2 dup ("bliblablub"),3)

	dw	1
	dw	1,2
	dw	1,2,3
	dw	1,2,3,4
	dw	2 dup (1)
	dw	3 dup (1)
	dw	2 dup (1,2)
	dw	3 dup (1,2)
	dw	2 dup (1,2,3)
	dw	3 dup (1,2,3)
	dw	2 dup (1,2,3,4)
	dw	3 dup (1,2,3,4)
	dw	2 dup (1,2 dup ("bliblablub"),3)

	dd	1
	dd	1,2
	dd	1,2,3
	dd	1,2,3,4
	dd	2 dup (1)
	dd	3 dup (1)
	dd	2 dup (1,2)
	dd	3 dup (1,2)
	dd	2 dup (1,2,3)
	dd	3 dup (1,2,3)
	dd	2 dup (1,2,3,4)
	dd	3 dup (1,2,3,4)
	dd	2 dup (1,2 dup ("bliblablub"),3)
	dd	2 dup (1.0,2.0,3.0)

	dq	1
	dq	1,2
	dq	1,2,3
	dq	1,2,3,4
	dq	2 dup (1)
	dq	3 dup (1)
	dq	2 dup (1,2)
	dq	3 dup (1,2)
	dq	2 dup (1,2,3)
	dq	3 dup (1,2,3)
	dq	2 dup (1,2,3,4)
	dq	3 dup (1,2,3,4)
	dq	2 dup (1,2 dup ("bliblablub"),3)
	dq	2 dup (1.0,2.0,3.0)

	dt	1
	dt	1,2
	dt	1,2,3
	dt	1,2,3,4
	dt	2 dup (1)
	dt	3 dup (1)
	dt	2 dup (1,2)
	dt	3 dup (1,2)
	dt	2 dup (1,2,3)
	dt	3 dup (1,2,3)
	dt	2 dup (1,2,3,4)
	dt	3 dup (1,2,3,4)
	dt	2 dup (1,2 dup ("bliblablub"),3)
	dt	2 dup (1.0,2.0,3.0)
