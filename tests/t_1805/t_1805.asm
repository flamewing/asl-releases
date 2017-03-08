	cpu	1805a

	rldi	4,1234h
	rlxa	5
	rsxd	7

	dbnz	5,$
	rnx	4

	bci	$
	bxi	$	

	ldc
	gec
	stpc
	dtc
	stm
	scm1
	scm2
	spm1
	spm2
	etq
	xie
	xid
	cie
	cid
	dsav

	scal	4,8765h
	sret	5

	cpu	1805a

	dadd
	dadi	12h
	dadc
	daci	12h
	dsm
	dsmi	12h
	dsmb
	dsbi	12h
