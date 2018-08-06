		cpu	8086

		; Exec Addresses: Code(0000h), Data(1000h)

		segment	code
		org	0000h
entry1		nop
		segment	data
		org	1000h
data1		db	?

		; Exec Addresses: Code(0800h), Data(1000h)

		segment	code
		phase	$+800h
entry2		nop
		segment	data
data2		db	?

		; Exec Addresses: Code(0800h), Data(1800h)

		segment	code
entry3		nop
		segment	data
		phase	$+800h
data3		db	?

		; Exec Addresses: Code(1000h), Data(1800h)

		segment	code
		phase	$+800h
entry4		nop
		segment	data
data4		db	?

		; Exec Addresses: Code(1000h), Data(2000h)

		segment	code
entry5		nop
		segment	data
		phase	$+800h
data5		db	?

		; Exec Addresses: Code(0800h), Data(2000h)

		segment	code
		dephase
entry6		nop
		segment	data
data6		db	?

		; Exec Addresses: Code(0800h), Data(1800h)

		segment	code
entry7		nop
		segment data
		dephase
data7		db	?

		; Exec Addresses: Code(0000h), Data(1800h)

		segment	code
		dephase
entry8		nop
		segment	data
data8		db	?

		; Exec Addresses: Code(0000h), Data(1000h)

		segment	code
entry9		nop
		segment data
		dephase
data9		db	?

		segment	code


		irp	addr,entry1,entry2,entry3,entry4,entry5,entry6,entry7,entry8,entry9
		call	addr
		endm

		irp	addr,data1,data2,data3,data4,data5,data6,data7,data8,data9
		mov	[addr],0
		endm
