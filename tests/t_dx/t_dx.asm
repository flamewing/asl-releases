	cpu	8086

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
