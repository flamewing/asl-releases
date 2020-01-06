	cpu	8086
	page	0

	segment	code

	org	0

	db	3 dup "abc"
	dw	1
	dw	3 dup (1)
	dw	"abc"
	dw	1 dup ("abc")
	dw	2 dup ("abc")
	dw	3 dup ("abc")
	dd	1
	dd	1.0
	dd	3 dup (1)
	dd	"abc"
	dd	1 dup ("abc")
	dd	2 dup ("abc")
	dd	3 dup ("abc")
	dq	1
	dq	1.0
	dq	3 dup (1)
	dq	"abc"
	dq	1 dup ("abc")
	dq	2 dup ("abc")
	dq	3 dup ("abc")
	dt	1
	dt	1.0
	dt	3 dup (1)
	dt	'a','b','c'
	dt	"abc"
	dt	1 dup ("abc")
	dt	2 dup ("abc")
	dt	3 dup ("abc")

	db	0
	db	255
	db	-128
	db	'A'
	expect	1320
	db	'AB'
	endexpect
	expect	1320
	db	'ABC'
	endexpect
	expect	1320
	db	'ABCD'
	endexpect
	db	"A"
	db	"AB"
	db	"ABC"
	db	"ABCD"

	dw	0
	dw	65535
	dw	-32768
	dw	'A'
	dw	'AB'
	expect	1320
	dw	'ABC'
	endexpect
	expect	1320
	dw	'ABCD'
	endexpect
	dw	"A"
	dw	"AB"
	dw	"ABC"
	dw	"ABCD"

	dd	0
	dd	4294967295
	dd	-2147483648
	dd	'A'
	dd	'AB'
	dd	'ABC'
	dd	'ABCD'
	dd	"A"
	dd	"AB"
	dd	"ABC"
	dd	"ABCD"

	dq	0
	dq	2147483647
	dq	-2147483648
	dq	'A'
	dq	'AB'
	dq	'ABC'
	dq	'ABCD'
	dq	"A"
	dq	"AB"
	dq	"ABC"
	dq	"ABCD"

	dt	0
	dt	2147483647
	dt	-2147483648
	dt	'A'
	dt	'AB'
	dt	'ABC'
	dt	'ABCD'
	dt	"A"
	dt	"AB"
	dt	"ABC"
	dt	"ABCD"
