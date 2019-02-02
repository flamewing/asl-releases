	page	0

	cpu	mcf51qm

	org	$12346
	move.l	(beans,a1),d0
	move.l	beans(a1),d0
	move.l	(beans,a1,d1.l*2),d0
	move.l	beans(a1,d1.l*2),d0
beans	equ	-10
