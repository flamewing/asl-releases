	cpu	1805

	rldi	4,1234h
	rlxa	5
	rsxd	7

	dbnz	5,$
	rnx	4

	dadd
	dadi	12h
	dadc
	daci	12h
	dsm
	dsmi	12h
	dsmb
	dsbi	12h

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
