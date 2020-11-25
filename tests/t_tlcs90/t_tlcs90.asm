	cpu	90c141

	irp	instr,mul,div
	instr	hl,c
	instr	hl,34h
	instr	hl,(de)
	instr	hl,(iy-5)
	instr	hl,(hl+a)
	instr	hl,(1234h)
	instr	hl,(0ff34h)
	endm

