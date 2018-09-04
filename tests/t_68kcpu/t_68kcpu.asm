		cpu	68020
		supmode	on
		page	0

		irp	instr,addq,subq
		irp	size,b,w,l
		instr.size	#1,d4
		instr.size	#3,(a7)
		instr.size	#4,(a0)+
		instr.size	#5,-(a1)
		instr.size	#6,200(a2)
		instr.size	#7,(20,a3,d1)
		instr.size	#8,$1234
		instr.size	#1,$12345678
		endm
		irp	size,w,l
		instr.size	#2,a6
		endm
		endm

		irp	instr,rol,ror,roxl,roxr,asl,asr,lsl,lsr
		irp	size,b,w,l
		instr.size	d3,d4
		instr.size	#7,d4
		endm
		instr		(a7)
		instr		1,(a7)
		instr		#1,(a7)
		instr		(a0)+
		instr		1,(a0)+
		instr		#1,(a0)+
		instr		-(a1)
		instr		1,-(a1)
		instr		#1,-(a1)
		instr		200(a2)
		instr		1,200(a2)
		instr		#1,200(a2)
		instr		(20,a3,d1)
		instr		1,(20,a3,d1)
		instr		#1,(20,a3,d1)
		instr		$1234
		instr		1,$1234
		instr		#1,$1234
		instr		$12345678
		instr		1,$12345678
		instr		#1,$12345678
		endm
