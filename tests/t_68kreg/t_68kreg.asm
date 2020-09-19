	cpu	68020
	fpu	on

regd1	equ	d1
regd2	equ	d2
regd4	equ	d4
rega1	equ	a1
rega2	equ	a2
regsp	equ	sp
regfp1	equ	fp1
regfp2	equ	fp2
regfp4	equ	fp4
regfpcr	equ	fpcr
regfpsr	equ	fpsr
regfpiar equ	fpiar

myreg	reg	rega2

	move.l	d1,d2
	move.l	regd1,regd2
	move.l	a1,a2
	move.l	rega1,rega2

	mulu.l	d4,d1:d2
	mulu.l	d4,regd1:regd2

	cas2.l	d1:d2,d1:d2,(a1):(a2)
	cas2.l	regd1:regd2,regd1:regd2,(rega1):(rega2)

	bfins	d4,(a1){d1:d2}
	bfins	regd4,(rega1){regd1:regd2}

	fmove	(a1),fp2
	fmove	(rega1),regfp2

	movem	(a1)+,d1-d2/d4
	movem	(rega1)+,regd1-regd2/regd4
	movem	d1-d2/d4,-(sp)
	movem	regd1-regd2/regd4,-(regsp)

	fmovem	(a1)+,fp1-fp2/fp4
	fmovem	(rega1)+,regfp1-regfp2/regfp4
	fmovem	(a1)+,fpcr/fpsr/fpiar
	fmovem	(rega1)+,regfpcr/regfpsr/regfpiar
	fmovem	(a1)+,d4
	fmovem	(rega1)+,regd4
	fmovem	fp1-fp2/fp4,-(sp)
	fmovem	regfp1-regfp2/regfp4,-(regsp)
	fmovem	fpcr/fpsr/fpiar,-(sp)
	fmovem	regfpcr/regfpsr/regfpiar,-(regsp)
	fmovem	d4,-(sp)
	fmovem	regd4,-(regsp)
