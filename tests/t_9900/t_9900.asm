        cpu     tms9900
        page    0
	supmode on

        a       r5,r3

        a       wr5,wr3
        ab      wr6,*wr10
        c       wr7,*wr2+
        cb      wr8,@1234h
        s       wr9,@1234h(wr6)

        sb      *wr10,wr9
        soc     *wr11,*wr6
        socb    *wr12,*wr1+
        szc     *wr13,@1234h
        szcb    *wr14,@1234h(wr14)

        mov     *wr15+,wr10
        movb    *wr0+,*wr6
        a       *wr1+,*wr6+
        ab      *wr2+,@1234h
        c       *wr3+,@1234h(wr7)

        cb      *wr4+,wr5
        s       *wr5+,*wr13
        sb      *wr6+,*wr13+
        soc     *wr7+,@1234h
        socb    *wr8+,@1234h(wr12)

        szc     @1234h,wr10
        szcb    @1234h,*wr2
        mov     @1234h,*wr3+
        movb    @1234h,@2345h
        a       @1234h,@2345h(wr8)

        ab      @1234h(wr9),wr5
        c       @1234h(wr10),*wr1
        cb      @1234h(wr11),*wr14+
        s       @1234h(wr12),@2345h
        sb      @1234h(wr13),@2345h(wr5)


        coc	wr12,wr5
        czc	*wr4,wr10
        xor	*wr12+,wr7
        mpy	@1234h,wr4
        div	@200(wr4),wr6
        xop	@2345h,wr5

        mpys	wr5
        divs	*wr9+

        b	@1234h
        bl	*wr5
        blwp	*wr7+
        clr	wr4
        seto	@1234h(wr8)
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

        sla	wr1,1
        sra	wr5,wr0
        src	wr6,10
        srl	wr10,15

        ai	wr5,1234h
        andi	wr10,2345h
        ci	wr15,3456h
        li	wr5,4567h
        ori	wr10,5678h

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


table	equ	1234h
oldval	equ	2345h
newval	equ	3456h
new	equ	4567h
loc	equ	5678h
count	equ	6789h
list	equ	789ah
tran	equ	89abh
testbits equ	9abch
testbit equ	0abcdh
ones	equ	0bcdeh
temp	equ	0cdefh
change	equ	0def0h
bits	equ	0ef01h
testva	equ	0f012h
prt	equ	r3

        a	r5,@table
        ab	3,*2+
	ai	6,0ch
        s	@oldval,@newval
	sb	*6+,1
        mpy	@new,5
        div	@loc,2
        inc	@count
        inct	5
        dec	@count
        dect	prt
	abs	@list(7)
        neg	5
        b	*3
        bl	@tran
        blwp	@tran
        rtwp
        jmp	$
        jh	$
        jl	$
        jhe	$
        jle	$
        jgt	$
        jlt	$
        jeq	$
        jne	$
        joc	$
        jnc	$
        jno	$
        jop	$
        x       @tran
        c	r5,@table
        cb	3,*2+
	ci	9,0f330h
	coc	@testbits,8
        czc	@testbit,8
        rset
        idle
        ckof
        ckon
        lrex
        sbo	20
        sbz	30
        tb	40
        ldcr	@1234h,7
        stcr	@1234h,7
        li	7,5
        limi	3
        lwpi	12h
        lmf	r5,0
        mov	7,7
        mov	@ones,9
        movb	@temp,3
        swpb	*0+
        stst	7
        stwp	r5
        andi	0,6d03h
        ori	5,60d3h
        xor	@change,2
        inv	11
        clr	*0bh
        seto	3
        soc	3,@new
        socb	5,8
        szc	5,7
        SZCB	@bits,@testva
        sra	5,0
        sla	10,5
        srl	0,3
        src	2,7
        xop	*4,wr4
        lds	@bits
        ldd	@bits
