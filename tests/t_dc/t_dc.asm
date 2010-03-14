	cpu	68020

	irp	op,b,w,l,q,s,d,x,p
	dc.op	[3]25
	dc.op	"Hello World"
	ds.op	9
	endm

	irp	op,s,d,x,p
	dc.op	1.5
	dc.op	constpi*1000
        dc.op	constpi/1000
        dc.op   -constpi
	endm
