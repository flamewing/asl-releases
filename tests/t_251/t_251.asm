	cpu	80c251
        include reg251

        page	0

        org	0ff0000h

data	equ	30h
;acc     equ     0eeh
Bit1	bit	21h.7
Bit2	bit	s:90h.4
Bit3	bit	data.5
Bit4	bit	acc.3
Bit5	bit	Bit3

        acall	Targ
        ajmp	Targ

        lcall	Targ
        lcall	@wr4
        ljmp	Targ
        ljmp	@wr2

        ecall	Targ
        ecall	@dpx
        ejmp	Targ
        ejmp	@dr16

        sjmp	Targ
        jc	Targ
        jnc	Targ
        jnz	Targ
        jz	Targ
        je	Targ
        jsle	Targ
        jle	Targ
        jne	Targ
        jsg	Targ
        jsge	Targ
        jsl	Targ
        jsle	Targ

        jmp	@a+dptr
        jmp	@wr2
        jmp	@dr16
        jmp	Targ
        jmp	Targ+4000h
        jmp	4000h
        call	@wr4
        call	@dpx
        call	Targ
        call	Targ+4000h

        jb      Bit1,Targ
        jbc	Bit2,Targ
        jnb     Bit4,Targ
        jb	Bit3,Targ
        jnb	s:91h.4,Targ
        jbc	44h.5,Targ

        djnz	r5,Targ
        djnz	acc,Targ
        djnz    data,Targ

        cjne	a,data,Targ
        cjne	a,#'A',Targ
        cjne	r6,#50,Targ
        cjne	@r1,#40h,Targ

Targ:

	add	a,#12h
        add	a,data
        add	a,1234h
        add	a,@r1
        add	a,@wr2
        add	a,@dr8
        add	a,r4
        add	a,a
        add	a,r13
	add	r3,r12
	add	wr8,wr12
        add	dr20,dpx
        add	r4,#23h
        add	r11,#23h
        add	wr4,#1234h
        add	dr4,#1234h
        add	r8,data
        add	r11,data
        add	wr8,data
        add     r8,1234h
        add	wr8,55ah
        add	r11,@r1
        add	r2,@wr6
        add	r6,@dr12

        sub	a,r7
        sub	wr4,wr10
        sub	dr12,dr28
        sub	r4,#10
        sub	wr6,#1000
        sub	dr16,#05aa5h
        sub	a,data
        sub	wr8,data
        sub	r12,1243h
        sub	wr10,1342h
        sub	r3,@wr14
        sub	a,@spx

        cmp	a,r7
        cmp	wr4,wr10
        cmp	dr12,dr28
        cmp	r4,#10
        cmp	wr6,#1000
        cmp	dr16,#05aa5h
        cmp	dr16,#-3
        cmp	a,data
        cmp	wr8,data
        cmp	r12,1243h
        cmp	wr10,1342h
        cmp	r3,@wr14
        cmp	a,@spx

        addc	a,r6
        addc	r11,@r0
        addc	a,data
        addc	a,#20h
        subb	a,r6
        subb	r11,@r0
        subb	a,data
        subb	a,#20h

        anl	c,Bit1
        anl	cy,Bit3
        anl	c,/Bit1
        anl	cy,/Bit3
        orl	c,Bit1
        orl	cy,Bit3
        orl	c,/Bit1
        orl	cy,/Bit3

        anl	data,a
        anl	data,r11
        anl	data,#10001111b
	anl	a,#20h
        anl	a,acc
        anl	a,90h
        anl	a,s:90h
        anl	a,@r1
        anl	a,@wr10
        anl	a,@dr12
        anl	a,r4
        anl	a,r10
        anl	a,a
        anl	r10,r4
        anl	wr10,wr6
        anl	r10,#55h
        anl	r11,#0aah
        anl	wr14,#1000100010001001b
        anl	r1,data
        anl	r11,data
        anl	wr4,data
        anl	r10,1234h
        anl	wr2,1234h
        anl	r11,@r1
        anl	r6,@wr14
        anl	r7,@dr16

        orl	data,a
        orl	data,r11
        orl	data,#10001111b
	orl	a,#20h
        orl	a,acc
        orl	a,90h
        orl	a,s:90h
        orl	a,@r1
        orl	a,@wr10
        orl	a,@dr12
        orl	a,r4
        orl	a,r10
        orl	a,a
        orl	r10,r4
        orl	wr10,wr6
        orl	r10,#55h
        orl	r11,#0aah
        orl	wr14,#1000100010001001b
        orl	r1,data
        orl	r11,data
        orl	wr4,data
        orl	r10,1234h
        orl	wr2,1234h
        orl	r11,@r1
        orl	r6,@wr14
        orl	r7,@dr16

        xrl	data,a
        xrl	data,r11
        xrl	data,#10001111b
	xrl	a,#20h
        xrl	a,acc
        xrl	a,90h
        xrl	a,s:90h
        xrl	a,@r1
        xrl	a,@wr10
        xrl	a,@dr12
        xrl	a,r4
        xrl	a,r10
        xrl	a,a
        xrl	r10,r4
        xrl	wr10,wr6
        xrl	r10,#55h
        xrl	r11,#0aah
        xrl	wr14,#1000100010001001b
        xrl	r1,data
        xrl	r11,data
        xrl	wr4,data
        xrl	r10,1234h
        xrl	wr2,1234h
        xrl	r11,@r1
        xrl	r6,@wr14
        xrl	r7,@dr16


        clr	a
        clr	c
        clr	Bit1
        clr	s:92h.6

        cpl	a
        cpl	c
        cpl	Bit1
        cpl	s:92h.6

        setb	c
        setb	Bit1
        setb	s:92h.6

	dec	a
        dec	data
        dec	@r1
        dec	r5,#1
        dec     r10
        dec	r11,#1
        dec	a,#2
        dec	r11,#4
        dec	wr2,#2
        dec	spx,#4

	inc	a
        inc	data
        inc	@r1
        inc	r5,#1
        inc     r10
        inc	r11,#1
        inc	a,#2
        inc	r11,#4
        inc	wr2,#2
        inc	spx,#4
        inc	dptr

        mul	ab
        mul	r5,r7
        mul	wr10,wr14

        div	ab
        div	r5,r7
        div	wr10,wr14

	mov	a,#10
        mov	acc,#20
        mov	@r1,#30
        mov	r4,#40
        mov	data,acc
        mov	s:90h,@r1
        mov	70h,r6
        mov	@r1,data
        mov	r5,data
        mov	a,acc
        mov	a,@r1
        mov	a,r2
        mov	acc,a
        mov	@r1,a
        mov	r4,a
        mov	a,r12
        mov	r12,a
        mov	r11,r5
        mov	r5,r11
        mov	r10,r6
        mov	wr4,wr10
        mov	dr12,dpx
        mov	r11,#50
        mov	r7,#60
        mov	r14,#70
        mov	wr12,#8090
        mov	dr12,#8090
        mov	dr12,#-100
        mov	r11,data
        mov	r4,data
        mov	r15,data
        mov	wr10,acc
        mov	dr16,s:0a0h
        mov	a,1234h
        mov	r10,1234h
        mov	wr14,1234h
        mov	dr20,1243h
        mov	a,@wr10
        mov	r2,@wr10
        mov	a,@dr12
        mov	r4,@dr12
        mov	wr10,@wr12
        mov	wr10,@dr12
        mov	data,r11
        mov	data,r5
        mov	data,r8
        mov	data,wr2
        mov	data,spx
        mov	1234h,a
        mov	1234h,r5
        mov	1234h,wr8
        mov	1234h,dr16
        mov	@wr14,a
        mov	@wr14,r9
        mov	@dr12,a
        mov	@dr12,r8
        mov	@wr14,wr18
        mov	@dr8,wr18
        mov	a,@wr2+10
        mov	r4,@wr2-20
        mov	wr10,@wr2+30
        mov	a,@dpx-40
        mov	r8,@dpx+50
        mov	wr10,@dpx-60
        mov	@wr2+10,a
        mov	@wr2-20,r4
        mov	@wr2+30,wr10
        mov	@dpx-40,a
        mov	@dpx+50,r8
        mov	@dpx-60,wr10
        mov	dptr,#1234h

        movc	a,@a+dptr
        movc	a,@a+pc

        movh	dr12,#55aah

        movs	wr10,a
        movz	wr12,r12

        movx	a,@dptr
        movx	a,@r1
        movx	@dptr,a
        movx	@r0,r11

        pop	data
        pop	acc
        pop	a
        pop	r5
        pop	wr8
        pop	dr20
        push	data
        push	acc
        push	a
        push	r5
        push	wr8
        push	dr20
        push	#20
        pushw	#55aah

        xch	a,r5
        xch	a,data
        xch	a,@r1
        xch	r5,r11
        xch	data,a
        xch	@r1,a
        xchd	a,@r1
        xchd	@r1,r11

        nop
        ret
        reti
        eret
        trap

        da	a
        rl	a
        rlc	a
        rr	a
        rrc	a
        swap	r11

        sll	a
        sll	r6
        sll	wr12
        sra	a
        sra	r6
        sra	wr12
        srl	a
        srl	r6
        srl	wr12
