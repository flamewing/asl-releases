        cpu     tms9900
        page    0

        a       r5,r3

        a       wr5,wr3
        ab      wr6,*wr10
        c       wr7,*wr2+
        cb      wr8,@1234h
        s       wr9,1234h(wr6)

        sb      *wr10,wr9
        soc     *wr11,*wr6
        socb    *wr12,*wr1+
        szc     *wr13,1234h
        szcb    *wr14,@1234h(wr14)

        mov     *wr15+,wr10
        movb    *wr0+,*wr6
        a       *wr1+,*wr6+
        ab      *wr2+,1234h
        c       *wr3+,1234h(wr7)

        cb      *wr4+,wr5
        s       *wr5+,*wr13
        sb      *wr6+,*wr13+
        soc     *wr7+,1234h
        socb    *wr8+,1234h(wr12)

        szc     1234h,wr10
        szcb    1234h,*wr2
        mov     1234h,*wr3+
        movb    1234h,2345h
        a       1234h,2345h(wr8)

        ab      1234h(wr9),wr5
        c       1234h(wr10),*wr1
        cb      1234h(wr11),*wr14+
        s       1234h(wr12),2345h
        sb      1234h(wr13),2345h(wr5)


        coc	wr12,wr5
        czc	*wr4,wr10
        xor	*wr12+,wr7
        mpy	1234h,wr4
        div	200(wr4),wr6
        xop	2345h,wr5

        mpys	wr5
        divs	*wr9+

        b	1234h
        bl	*wr5
        blwp	*wr7+
        clr	wr4
        seto	1234h(wr8)
        inv	wr7
        neg	*wr15
        abs	wr3
        swpb	wr9
        inc	*wr12+
        inct	*wr12+
        dec	*wr12+
        dect	*wr12+
        x	*wr6

        ldcr	wr5,10
        stcr	wr6,16

        sbo	10
        sbz	-33
        tb	34h

        jeq	lab
        jgt	lab
        jh	lab
        jhe	lab
        jl	lab
        jle	lab
lab:	jlt	lab
	jmp	lab
        jnc	lab
        jne	lab
        jno	lab
        joc	lab
        jop	lab

        sla	1,wr1
        sra	wr0,wr5
        src	10,wr6
        srl	15,wr10

        ai	1234h,wr5
        andi	2345h,wr10
        ci	3456h,wr15
        li	4567h,wr5
        ori	5678h,wr10

        lwpi	1234h
        limi	2345h

        stst	wr2
        lst	wr4
        stwp	wr6
        lwp	wr8

        rtwp
        idle
        rset
        ckof
        ckon
        lrex

        padding on

        byte    1,2,3,4,5
        byte    "Hello World"
        byte    1,2,3,4,5,6
        byte    "Hello World!"
        word    1,2,3,4,5

        padding off

        byte    1,2,3,4,5
        byte    "Hello World"
        byte    1,2,3,4,5,6
        byte    "Hello World!"
        word    1,2,3,4,5

