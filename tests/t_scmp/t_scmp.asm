	cpu	sc/mp

        lde
        xae
        ane
        ore
        xre
        dae
        ade
        cae

        sio
        sr
        srl
        rr
        rrl

        halt
        ccl
        scl
        dint
        ien
        csa
        cas
        nop

        ldi	0x12
        ani	0x23
        ori	0x34
        xri	0x45
        dai	0x56
        adi	0x67
        cai	0x78
        dly	0x89

        xpal	pc
        xpah	p2
        xppc	p1

	ld	e(pc)
        st	@e(p2)
        and	10(p1)
        or	@-20(p3)
        xor	vari
vari:	dad	-30(p2)
	add	@40(p1)
        cad	vari

	jmp	vari
        jp	10(p2)
        jz	vari
        jnz	vari

        ild	vari
        dld	-5(p2)

;        org     0xfff
;        ldi     0x20

