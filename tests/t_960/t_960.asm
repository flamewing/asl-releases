	cpu	80960
	org	0

	fpu	on
	supmode on

start:
	flushreg
	fmark
	mark
	ret
	syncf
	faultno
	faultg
	faulte
	faultge
	faultl
	faultne
	faultle
	faulto

	addc	r3,r4,r5	; lokale Register
	addc	g3,g4,g5	; globale Register
	addc	g3,g5		; impl. src2=dest
	addc	10,r4,g10	; src1 immediate
	addc	g2,20,g4	; src2 immediate

	addr	r3,r4,r5	; Float: normale Register
	addr	fp2,r4,r5	; Float-Register
	addr	r3,fp1,r5
	addr	r3,r4,fp3
	addr	0,r4,r5		; Float-Konstanten
	addr	r3,1,r5

	addc	sp,fp,rip	; Sonderregister

	addi	r3,r4,r5	; nur noch mal zum Testen der Opcodes... 
	addo	r3,r4,r5	

	calls	g2		; nur ein Operand:
	calls	4

	chkbit	7,r5		; kein Ziel

	classr	fp1		; ein Float-Operand
	classrl	g4

	cosr	fp1		; dito mit Ziel
	cosr	g4,fp2

	modpc	r3,r4,r5	; nur im Supervisor-Mode erlaubt

	bbc	r4,g5,$+32	; COBR-Anweisungen
	bbs	10,r10,$+32
	cmpobge 10,r4,$+32
	testne	r5

	b	$-32		; CTRL-Anweisungen
	call	$+32
	bal	$+64
	ble	$-64

	bx	(r5)		; MEMA ohne disp
	bx	2345(r5)	; MEMA mit disp
        bx	2345		; MEMA ohne base
	bx	-30(r5)		; MEMB base+disp
	bx	5000(r5)	; dito positiv
	bx	$(ip)		; PC-relativ
	bx	[r4]		; nur Index
	bx	[r4*1]		; Scaling
	bx      [r4*2]
        bx      [r4*4]
        bx      [r4*8]
	bx	[r4*16]
	bx	0(r5)[r4]	; base+index
	bx	12345678h	; nur disp
	bx	450[r6*4]	; disp+index
	bx	123(r5)[r6*4]	; volles Programm

	st	r7,123(r5)[r6*4]; mit 2 Ops
	ld	123(r5)[r6*4],r7

	db	1
	align	4
	db	1,2
	align	4
	db	1,2,3
	align	4
	db	1,2,3,4
	dw	1
	align	4
	dw	1,2
	dd	1
	dd	1.0
	dq	1.0
	dt	1.0
	dw	0

	space	32

	word	1,2,3,4,5
