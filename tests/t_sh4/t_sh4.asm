	cpu	sh7750
	supmode	on
	page	0

	; Fixed-Point Transfer Instructions

	mov	#0,r3		; E300
	mov	#-1,r3		; E3FF
	mov	#127,r3		; E37F
	mov	#-128,r3	; E380
	mov	#$ffffffff,r3	; E3FF

	mov.w	@(pc),r4	; 9400
	mov.w	@(510,pc),r4	; 94FF
	expect	1325
	mov.w	@(253,pc),r4
	endexpect
	expect	1320
	mov.w	@(512,pc),r4
	endexpect
	expect	1315
	mov.w	@(-2,pc),r4
	endexpect

	mov.l	@(pc),r5	; D500
	mov.l	@(1020,pc),r5	; D5FF
	expect	1325
	mov.l	@(1022,pc),r5
	endexpect
	expect	1320
	mov.l	@(1024,pc),r5
	endexpect
	expect	1315
	mov.l	@(-4,pc),r5
	endexpect

	mov	r1,r6		; 6613

	mov.b	r1,@r7		; 2710

	mov.w	r1,@r8		; 2811

	mov.l	r1,@r9		; 2912

	mov.b	@r1,r10		; 6A10

	mov.w	@r1,r11		; 6B11

	mov.l	@r1,r12		; 6C12

	mov.b	r1,@-r13	; 2D14

	mov.w	r1,@-r14	; 2E15

	mov.l	r1,@-r2		; 2216

	mov.b	@r1+,r3		; 6314

	mov.w	@r1+,r4		; 6415

	mov.l	@r1+,r5		; 6516

	mov.b	r0,@(15,r6)	; 806F
	expect	1320
	mov.b	r0,@(16,r6)
	endexpect
	expect	1315
	mov.b	r0,@(-1,r6)
	endexpect

	mov.w	r0,@(30,r7)	; 817F
	expect	1325
	mov.w	r0,@(29,r7)
	endexpect
	expect	1320
	mov.w	r0,@(32,r7)
	endexpect
	expect	1315
	mov.w	r0,@(-2,r7)
	endexpect

	mov.l	r1,@(60,r8)	; 181F
	expect	1325
	mov.l	r1,@(57,r8)
	endexpect
	expect	1320
	mov.l	r1,@(64,r8)
	endexpect
	expect	1315
	mov.l	r1,@(-4,r8)
	endexpect

	mov.b	@(15,r9),r0	; 849F
	expect	1320
	mov.b	@(16,r9),r0
	endexpect
	expect	1315
	mov.b	@(-1,r9),r0
	endexpect

	mov.w	@(30,r10),r0	; 85AF
	expect	1325
	mov.w	@(29,r10),r0
	endexpect
	expect	1320
	mov.w	@(32,r10),r0
	endexpect
	expect	1315
	mov.w	@(-2,r10),r0
	endexpect

	mov.l	@(60,r11),r1	; 51BF
	expect	1325
	mov.l	@(57,r11),r1
	endexpect
	expect	1320
	mov.l	@(64,r11),r1
	endexpect
	expect	1315
	mov.l	@(-4,r11),r1
	endexpect

	mov.b	r1,@(r0,r12)	; 0C14

	mov.w	r1,@(r0,r13)	; 0D15

	mov.l	r1,@(r0,r14)	; 0E16

	mov.b	@(r0,r2),r1	; 012C

	mov.w	@(r0,r3),r1	; 013D

	mov.l	@(r0,r4),r1	; 014E

	mov.b	r0,@(255,gbr)	; C0FF
	expect	1320
	mov.b	r0,@(256,gbr)
	endexpect
	expect	1315
	mov.b	r0,@(-1,gbr)
	endexpect

	mov.w	r0,@(510,gbr)	; C1FF
	expect	1325
	mov.w	r0,@(509,gbr)
	endexpect
	expect	1320
	mov.w	r0,@(512,gbr)
	endexpect
	expect	1315
	mov.w	r0,@(-2,gbr)
	endexpect

	mov.l	r0,@(1020,gbr)	; C2FF
	expect	1325
	mov.l	r0,@(1019,gbr)
	endexpect
	expect	1320
	mov.l	r0,@(1024,gbr)
	endexpect
	expect	1315
	mov.l	r0,@(-4,gbr)
	endexpect

	mov.b	@(255,gbr),r0	; C4FF
	expect	1320
	mov.b	@(256,gbr),r0
	endexpect
	expect	1315
	mov.b	@(-1,gbr),r0
	endexpect

	mov.w	@(510,gbr),r0	; C5FF
	expect	1325
	mov.w	@(509,gbr),r0
	endexpect
	expect	1320
	mov.w	@(512,gbr),r0
	endexpect
	expect	1315
	mov.w	@(-2,gbr),r0
	endexpect

	mov.l	@(1020,gbr),r0	; C6FF
	expect	1325
	mov.l	@(1019,gbr),r0
	endexpect
	expect	1320
	mov.l	@(1024,gbr),r0
	endexpect
	expect	1315
	mov.l	@(-4,gbr),r0
	endexpect

	mova	@(1020,pc),r0	; C7FF
	expect	1325
	mova	@(1019,pc),r0
	endexpect
	expect	1320
	mova	@(1024,pc),r0
	endexpect
	expect	1315
	mova	@(-4,pc),r0
	endexpect

	movt	r5		; 0529

	swap.b	r1,r6		; 6618

	swap.w	r1,r7		; 6719

	xtrct	r1,r8		; 281D

	; Arithmetic Operation Instructions

	add	r1,r9		; 391C

	add	#127,r10	; 7A7F
	add	#-128,r10	; 7A80
	add	#$ffffff80,r10	; 7A80
	expect	1350
	add	#128,r10
	endexpect
	expect	1350
	add	#-129,r10
	endexpect

	addc	r1,r11		; 3B1E

	addv	r1,r12		; 3C1F

	cmp/eq	#127,r0		; 887F
	cmp/eq	#-128,r0	; 8880
	cmp/eq	#$ffffff80,r0	; 8880
	expect	1350
	cmp/eq	#128,r13
	endexpect
	expect	1350
	cmp/eq	#-129,r13
	endexpect

	cmp/eq	r1,r14		; 3E10

	cmp/hs	r1,r2		; 3212

	cmp/ge	r1,r3		; 3313

	cmp/hi	r1,r4		; 3416

	cmp/gt	r1,r5		; 3517

	cmp/pz	r6		; 4611

	cmp/pl	r7		; 4715

	cmp/str	r1,r8		; 281C

	div1	r1,r9		; 3914

	div0s	r1,r10		; 2A17

	div0u			; 0019

	dmuls.l	r1,r11		; 3B1D

	dmulu.l	r1,r12		; 3C15

	dt	r13		; 4D10

	exts.b	r1,r14		; 6E1E

	exts.w	r1,r2		; 621F

	extu.b	r1,r3		; 631C

	extu.w	r1,r4		; 641D

	mac.l	@r1+,@r5+	; 051F

	mac.w	@r1+,@r6+	; 461F

	mul.l	r1,r7		; 0717

	muls.w	r1,r8		; 281F

	mulu.w	r1,r9		; 291E

	neg	r1,r10		; 6A1B

	negc	r1,r11		; 6B1A

	sub	r1,r12		; 3C18

	subc	r1,r13		; 3D19

	subv	r1,r14		; 3E1B

	; Logic Operation Instructions

	and	r1,r2		; 2219

	and	#255,r0		; C9FF
	expect	1350
	and	#256,r0
	endexpect
	expect	1350
	and	#-1,r0
	endexpect

	and.b	#255,@(r0,gbr)	; CDFF
	expect	1320
	and.b	#256,@(r0,gbr)
	endexpect
	expect	1315
	and.b	#-129,@(r0,gbr)
	endexpect

	not	r1,r4		; 6417

	or	r1,r5		; 251B

	or	#255,r0		; CBFF
	expect	1350
	or	#256,r0
	endexpect
	expect	1350
	or	#-1,r0
	endexpect

	or.b	#255,@(r0,gbr)	; CFFF
	expect	1320
	or.b	#256,@(r0,gbr)
	endexpect
	expect	1315
	or.b	#-129,@(r0,gbr)
	endexpect

	tas.b	@r6		; 461B

	tst	r1,r7		; 2718

	tst	#255,r0		; C87F
	expect	1350
	tst	#256,r0
	endexpect
	expect	1350
	tst	#-1,r0
	endexpect

	tst.b	#255,@(r0,gbr)	; CCFF
	expect	1320
	tst.b	#256,@(r0,gbr)
	endexpect
	expect	1315
	tst.b	#-129,@(r0,gbr)
	endexpect

	xor	r1,r8		; 281A

	xor	#255,r0		; CAFF
	expect	1350
	xor	#256,r0
	endexpect
	expect	1350
	xor	#-1,r0
	endexpect

	xor.b	#255,@(r0,gbr)	; CEFF
	expect	1320
	xor.b	#256,@(r0,gbr)
	endexpect
	expect	1315
	xor.b	#-129,@(r0,gbr)
	endexpect

	; Shift Instructions

	rotl	r9		; 4904

	rotr	r10		; 4A05

	rotcl	r11		; 4B24

	rotcr	r12		; 4C25

	shad	r1,r13		; 4D1C

	shal	r14		; 4E20

	shar	r2		; 4221

	shld	r1,r3		; 431D

	shll	r4		; 4400

	shlr	r5		; 4501

	shll2	r6		; 4608

	shlr2	r7		; 4709

	shll8	r8		; 4818

	shlr8	r9		; 4919

	shll16	r10		; 4A28

	shlr16	r11		; 4B29

	; Branch Instructions

	bf	*+2		; 8BFF
	bf	*+258		; 8B7F
	expect	1370
	bf	*+260
	endexpect
	bf	*-252		; 8B80
	expect	1370
	bf	*-254
	endexpect

	bf/s	*+2		; 8FFF
	nop
	bf/s	*+258		; 8F7F
	expect	1370
	bf/s	*+260
	nop
	endexpect
	bf/s	*-252		; 8F80
	nop
	expect	1370
	bf/s	*-254
	nop
	endexpect

	bt	*+2		; 89FF
	bt	*+258		; 897F
	expect	1370
	bt	*+260
	endexpect
	bt	*-252		; 8980
	expect	1370
	bt	*-254
	endexpect

	bt/s	*+2		; 8DFF
	nop
	bt/s	*+258		; 8D7F
	expect	1370
	bt/s	*+260
	nop
	endexpect
	bt/s	*-252		; 8D80
	nop
	expect	1370
	bt/s	*-254
	nop
	endexpect

	bra	*+2		; AFFF
	nop
	bra	*+4098		; A7FF
	nop
	expect	1370
	bra	*+4100
	nop
	endexpect
	bra	*-4092		; A800
	nop
	expect	1370
	bra	*-4094
	nop
	endexpect

	braf	r12		; 0C23

	bsr	*+2		; BFFF
	nop
	bsr	*+4098		; B7FF
	nop
	expect	1370
	bsr	*+4100
	nop
	endexpect
	bsr	*-4092		; B800
	nop
	expect	1370
	bsr	*-4094
	nop
	endexpect

	bsrf	r13		; 0D03

	jmp	@r14		; 4E2B
	nop

	jsr	@r2		; 420B
	nop

	rts			; 000B
	nop

	; System Control Instructions

	clrmac			; 0028

	clrs			; 0048

	clrt			; 0008

	ldc	r3,sr		; 430E

	ldc	r4,gbr		; 441E

	ldc	r5,vbr		; 452E

	ldc	r6,ssr		; 463E

	ldc	r7,spc		; 474E

	ldc	r8,dbr		; 48FA

	ldc	r9,r0_bank	; 498E

	ldc	r10,r1_bank	; 4A9E

	ldc	r11,r2_bank	; 4BAE

	ldc	r12,r3_bank	; 4CBE

	ldc	r13,r4_bank	; 4DCE

	ldc	r14,r5_bank	; 4EDE

	ldc	r2,r6_bank	; 42EE

	ldc	r3,r7_bank	; 43FE

	ldc.l	@r4+,sr		; 4407

	ldc.l	@r5+,gbr	; 4517

	ldc.l	@r6+,vbr	; 4627

	ldc.l	@r7+,ssr	; 4737

	ldc.l	@r8+,spc	; 4847

	ldc.l	@r9+,dbr	; 49F6

	ldc.l	@r10+,r0_bank	; 4A87

	ldc.l	@r11+,r1_bank	; 4B97

	ldc.l	@r12+,r2_bank	; 4CA7

	ldc.l	@r13+,r3_bank	; 4DB7

	ldc.l	@r14+,r4_bank	; 4EC7

	ldc.l	@r2+,r5_bank	; 42D7

	ldc.l	@r3+,r6_bank	; 43E7

	ldc.l	@r4+,r7_bank	; 44F7

	lds	r5,mach		; 450A

	lds	r6,macl		; 461A

	lds	r7,pr		; 472A

	lds.l	@r8+,mach	; 4806

	lds.l	@r9+,macl	; 4916

	lds.l	@r10+,pr	; 4A26

	ldtlb			; 0038

	movca.l	r0,@r11		; 0BC3

	nop			; 0009

	ocbi	@r12		; 0C93

	ocbp	@r13		; 0DA3

	ocbwb	@r14		; 0EB3

	pref	@r2		; 0283

	rte			; 002B
	nop

	sets			; 0058

	sett			; 0018

	sleep			; 001B

	stc	sr,r3		; 0302

	stc	gbr,r4		; 0412

	stc	vbr,r5		; 0522

	stc	ssr,r6		; 0632

	stc	spc,r7		; 0742

	stc	sgr,r8		; 083A

	stc	dbr,r9		; 09FA

	stc	r0_bank,r10	; 0A82

	stc	r1_bank,r11	; 0B92

	stc	r2_bank,r12	; 0CA2

	stc	r3_bank,r13	; 0DB2

	stc	r4_bank,r14	; 0EC2

	stc	r5_bank,r2	; 02D2

	stc	r6_bank,r3	; 03E2

	stc	r7_bank,r4	; 04F2

	stc.l	sr,@-r5		; 4503

	stc.l	gbr,@-r6	; 4613

	stc.l	vbr,@-r7	; 4723

	stc.l	ssr,@-r8	; 4833

	stc.l	spc,@-r9	; 4943

	stc.l	sgr,@-r10	; 4A32

	stc.l	dbr,@-r11	; 4BF2

	stc.l	r0_bank,@-r12	; 4C83

	stc.l	r1_bank,@-r13	; 4D93

	stc.l	r2_bank,@-r14	; 4EA3

	stc.l	r3_bank,@-r2	; 42B3

	stc.l	r4_bank,@-r3	; 43C3

	stc.l	r5_bank,@-r4	; 44D3

	stc.l	r6_bank,@-r5	; 45E3

	stc.l	r7_bank,@-r6	; 46F3

	trapa	#255		; C3FF

	; Floating-Point Single-Precision Instructions

	fpu	on
fsrc1	reg	fr4
fsrc2	reg	fr5
fdest	reg	fr6

	fldi0	fr12		; FC8D
	fldi0	fsrc1		; F48D
	fldi0	fsrc2		; F58D
	fldi0	fdest		; F68D

	fldi1	fr12		; FC9D
	fldi1	fsrc1		; F49D
	fldi1	fsrc2		; F59D
	fldi1	fdest		; F69D

	fmov	fsrc1,fdest	; F64C
	fmov.s	fsrc2,fdest	; F65C
	expect	1131
	fmov.d	fr12,fr11
	endexpect

	fmov	@r7,fdest	; F678
	fmov.s	@r7,fdest	;  "
	expect	1131
	fmov.d	@r7,fdest
	endexpect

	fmov	fsrc1,@r8	; F84A
	fmov.s	fsrc1,@r8	;  "
	expect	1131
	fmov.d	fsrc1,@r8
	endexpect

	fmov	@r9+,fdest	; F699
	fmov.s	@r9+,fdest	;  "
	expect	1131
	fmov.d	@r9+,fdest
	endexpect

	fmov	fsrc1,@-r10	; FA4B
	fmov.s	fsrc1,@-r10	;  "
	expect	1131
	fmov.d	fsrc1,@-r10
	endexpect

	fmov	@(r0,r11),fdest	; F6B6
	fmov.s	@(r0,r11),fdest	;  "
	expect	1131
	fmov.d	@(r0,r11),fdest
	endexpect

	fmov	fsrc1,@(r0,r12)	; FC47
	fmov.s	fsrc1,@(r0,r12)	;  "
	expect	1131
	fmov.d	fsrc1,@(r0,r12)
	endexpect

	fabs	fr12		; FC5D
	fabs	fsrc1		; F45D
	expect	1130
	fabs.d	fsrc2
	endexpect
	fabs.s	fsrc2		; F55D
	fabs.s	fdest		; F65D

	flds	fsrc1,fpul	; F41D
	flds.s	fsrc1,fpul	;  "
	expect	1130
	flds.d	fsrc1,fpul
	endexpect

	fsts	fpul,fdest	; F60D
	fsts.s	fpul,fdest	;  "
	expect	1130
	fsts.d	fpul,fdest
	endexpect

	fadd	fsrc1,fdest	; F640
	fadd.s	fsrc1,fdest	;  "
	expect	1130
	fadd.d	fsrc1,fdest
	endexpect

	fcmp/eq	fsrc1,fdest	; F644
	fcmp/eq.s fsrc1,fdest	;  "
	expect	1130
	fcmp/eq.d fsrc1,fdest
	endexpect

	fcmp/gt	fsrc1,fdest	; F645
	fcmp/gt.s fsrc1,fdest	;  "
	expect	1130
	fcmp/gt.d fsrc1,fdest
	endexpect

	fdiv	fsrc1,fdest	; F643
	fdiv.s	fsrc1,fdest	;  "
	expect	1130
	fdiv.d	fsrc1,fdest
	endexpect

	float	fpul,fdest	; F62D
	float.s	fpul,fdest	;  "
	expect	1130
	float.d	fpul,fdest
	endexpect

	fmac	fr0,fsrc1,fdest	; F64E
	fmac.s	fr0,fsrc1,fdest	;  "
	expect	1445
	fmac	fr1,fsrc1,fdest
	endexpect
	expect	1130
	fmac.d	fr0,fsrc1,fdest
	endexpect

	fmul	fsrc1,fdest	; F642
	fmul.s	fsrc1,fdest	;  "
	expect	1130
	fmul.d	fsrc1,fdest
	endexpect

	fsub	fsrc1,fdest	; F641
	fsub.s	fsrc1,fdest	;  "
	expect	1130
	fsub.d	fsrc1,fdest
	endexpect

	ftrc	fsrc1,fpul	; F43D
	ftrc.s	fsrc1,fpul	;  "
	expect	1130
	ftrc.d	fsrc1,fpul
	endexpect

	expect	1130
	fneg.d	fr12
	endexpect
	fneg	fr12		; FC4D
	fneg	fsrc1		; F44D
	fneg.s	fsrc2		; F54D
	fneg.s	fdest		; F64D

	fsqrt	fr12		; FC6D
	fsqrt	fsrc1		; F46D
	fsqrt.s	fsrc2		; F56D
	expect	1130
	fsqrt.d	fdest
	endexpect
	fsqrt.s	fdest		; F66D

        ; Floating-Point Double-Precision Data Transfer Instructions
	; Note they have the same machine codes as the single
	; precision counterparts, it is assumed that FPSCR.SZ is 1

dsrc1	equ	dr4
dsrc2	equ	dr6
ddest	equ	dr8

	fmov	dsrc1,ddest	; F84C
	fmov.d	dsrc2,ddest	; F86C
	expect	1131
	fmov.s	dr12,dr10
	endexpect

	fmov	@r13,ddest	; F8D8
	fmov.d	@r13,ddest	;  "
	expect	1131
	fmov.s	@r13,ddest
	endexpect

	fmov	dsrc1,@r14	; FE4A
	fmov.d	dsrc1,@r14	;  "
	expect	1131
	fmov.s	dsrc1,@r14
	endexpect

	fmov	@r1+,ddest	; F819
	fmov.d	@r1+,ddest	;  "
	expect	1131
	fmov.s	@r1+,ddest
	endexpect

	fmov	dsrc1,@-r2	; F24B
	fmov.d	dsrc1,@-r2	;  "
	expect	1131
	fmov.s	dsrc1,@-r2
	endexpect

	fmov	@(r0,r3),ddest	; F836
	fmov.d	@(r0,r3),ddest	;  "
	expect	1131
	fmov.s	@(r0,r3),ddest
	endexpect

	fmov	dsrc1,@(r0,r4)	; F447
	fmov.d	dsrc1,@(r0,r4)	;  "
	expect	1131
	fmov.s	dsrc1,@(r0,r4)
	endexpect

        ; Floating-Point Double-Precision Instructions
	; Note they have the same machine codes as the single
	; precision counterparts, it is assumed that FPSCR.PR is 1

	fabs	dr12		; FC5D
	fabs	dsrc1		; F45D
	expect	1130
	fabs.s	dsrc2
	endexpect
	fabs.d	dsrc2		; F65D
	fabs.d	ddest		; F85D

	fadd	dsrc1,ddest	; F840
	fadd.d	dsrc1,ddest	;  "
	expect	1130
	fadd.s	dsrc1,ddest
	endexpect

	fcmp/eq	dsrc1,ddest	; F844
	fcmp/eq.d dsrc1,ddest	;  "
	expect	1130
	fcmp/eq.s dsrc1,ddest
	endexpect

	fcmp/gt	dsrc1,ddest	; F845
	fcmp/gt.d dsrc1,ddest	;  "
	expect	1130
	fcmp/gt.s dsrc1,ddest
	endexpect

	fdiv	dsrc1,ddest	; F843
	fdiv.d	dsrc1,ddest	;  "
	expect	1130
	fdiv.s	dsrc1,ddest
	endexpect

	fcnvds	dsrc1,fpul	; F4BD
	fcnvds.d dsrc1,fpul	;  "
	expect	1130
	fcnvds.s dsrc1,fpul
	endexpect

	fcnvsd	fpul,ddest	; F8AD
	fcnvsd.d fpul,ddest	;  "
	expect	1130
	fcnvsd.s fpul,ddest
	endexpect

	float	fpul,ddest	; F82D
	float.d	fpul,ddest	;  "
	expect	1130
	float.s	fpul,ddest
	endexpect

	fmul	dsrc1,ddest	; F842
	fmul.d	dsrc1,ddest	;  "
	expect	1130
	fmul.s	dsrc1,ddest
	endexpect

	fneg	dr12		; FC4D
	fneg	dsrc1		; F44D
	expect	1130
	fneg.s	dsrc2
	endexpect
	fneg.d	dsrc2		; F64D
	fneg.d	ddest		; F84D

	fsqrt	dr12		; FC6D
	fsqrt	dsrc1		; F46D
	expect	1130
	fsqrt.s	dsrc2
	endexpect
	fsqrt.d	dsrc2		; F66D
	fsqrt.d	ddest		; F86D

	fsub	dsrc1,ddest	; F841
	fsub.d	dsrc1,ddest	;  "
	expect	1130
	fsub.s	dsrc1,ddest
	endexpect

	ftrc	dsrc1,fpul	; F43D
	ftrc.d	dsrc1,fpul	;  "
	expect	1130
	ftrc.s	dsrc1,fpul
	endexpect

	; Floating-Point Control Instructions

	lds	r7,fpscr	; 476A
	lds	r8,fpul		; 485A

	lds.l	@r9+,fpscr	; 4966
	lds.l	@r10+,fpul	; 4A56

	sts	fpscr,r11	; 0B6A
	sts	fpul,r12	; 0C5A

	sts.l	fpscr,@-r13	; 4D62
	sts.l	fpul,@-r14	; 4E52

	; Floating-Point Graphics Acceleration Instructions

	fmov	dsrc1,xd2	; F34C
	fmov.d	dsrc1,xd2	;  "
	expect	1131
	fmov.s	dsrc1,xd2
	endexpect

	fmov	xd4,ddest	; F85C
	fmov.d	xd4,ddest	;  "
	expect	1131
	fmov.s	xd4,ddest
	endexpect

	fmov	xd6,xd8		; F97C
	fmov.d	xd6,xd8		;  "
	expect	1131
	fmov.s	xd6,xd8
	endexpect

	fmov	@r5,xd10	; FB58
	fmov.d	@r5,xd10	;  "
	expect	1131
	fmov.s	@r5,xd10
	endexpect

	fmov	@r6+,xd12	; FD69
	fmov.d	@r6+,xd12	;  "
	expect	1131
	fmov.s	@r6+,xd12
	endexpect

	fmov	@(r0,r7),xd14	; FF76
	fmov.d	@(r0,r7),xd14	;  "
	expect	1131
	fmov.s	@(r0,r7),xd14
	endexpect

	fmov	xd2,@r8		; F83A
	fmov.d	xd2,@r8		;  "
	expect	1131
	fmov.s	xd2,@r8
	endexpect

	fmov	xd4,@-r9	; F95B
	fmov.d	xd4,@-r9	;  "
	expect	1131
	fmov.s	xd4,@-r9
	endexpect

	fmov	xd4,@(r0,r10)	; FA57
	fmov.d	xd4,@(r0,r10)	;  "
	expect	1131
	fmov.s	xd4,@(r0,r10)
	endexpect

	fipr	fv8,fv4		; F6ED

	ftrv	xmtrx,fv12	; FDFD

	frchg			; FBFD

	fschg			; F3FD

	; The few new instructions on SH7780 (SH4A core):

	cpu	sh7780

	movco	r0,@r7		; 0773
	movco.l	r0,@r7		;  "
	expect	1130
	movco.w	r0,@r7
	endexpect
	expect	1350
	movco	r2,@r7
	endexpect

	movli	@r8,r0		; 0863
	movli.l	@r8,r0		;  "
	expect	1130
	movli.w	@r8,r0
	endexpect
	expect	1350
	movli	@r8,r2
	endexpect

	movua	@r9,r0		; 49A9
	movua.l	@r9,r0		;  "
	expect	1130
	movua.w	@r9,r0
	endexpect
	expect	1350
	movua	@r9,r2
	endexpect

	movua	@r10+,r0	; 4AE9
	movua.l	@r10+,r0	;  "
	expect	1130
	movua.w	@r10+,r0
	endexpect
	expect	1350
	movua	@r10+,r2
	endexpect

	icbi	@r11		; 0BE3

	ldc	r12,sgr		; 4C3A

	ldc	@r13+,sgr	; 4D36
	ldc.l	@r13+,sgr	;  "

	prefi	@r14		; 0ED3

	synco			; 00AB

	fsrra	fr1		; F17D

	fsca	fpul,dr2	; F2FD

	fpchg			; F7FD

