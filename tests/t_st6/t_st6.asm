        cpu     st6218

        nop
        ret
        reti
        stop
        wait

        jrz     pc
        jrnz    pc+1
        jrc     pc+2
        jrnc    pc+3

        ld      a,v
        ld      a,w
        ld      a,x
        ld      a,y
        ld      a,12h
        ld      a,(x)
        ld      a,(y)
        ld      v,a
        ld      12h,a
        ld      (x),a
        ld      (y),a

        ldi     a,12h
        ldi     v,12h
        ldi     12h,12h

        jp      123h
        call    123h

        add     a,v
        add     a,12h
        add     a,(x)
        add     a,(y)

        and     a,v
        and     a,12h
        and     a,(x)
        and     a,(y)

        cp     a,v
        cp     a,12h
        cp     a,(x)
        cp     a,(y)

        sub     a,v
        sub     a,12h
        sub     a,(x)
        sub     a,(y)

        addi    a,12h
        andi    a,12h
        cpi     a,12h
        subi    a,12h

        clr     a
        clr     v
        clr     12h

        com     a
        rlc     a
        sla     a

        inc     a
        inc     v
        inc     12h
        inc     (x)
        inc     (y)

        dec     a
        dec     v
        dec     12h
        dec     (x)
        dec     (y)

vbit	bit	3,v
membit	bit	5,12h

        set     3,v
	set	vbit
        res     5,12h
	res	membit

        jrs     3,v,pc
	jrs	vbit,pc
        jrr     5,12h,pc+1
	jrr	membit,pc+1

	expect	1110,1110
	ascii
	asciz
	endexpect

	ascii	"This has no",' NUL termination.'
	asciz	"This has ",' NUL termination.'

	; if we are on page 0, we may jump anywhere in page 0 (dynamic) or 1 (static)

	assume	prpr:0
	jp	123h
	call	123h
	jp	923h
	call	923h
	expect	110
	 jp	1123h
	endexpect
	expect	110
	 call	1123h
	endexpect

	; similar for page 2.  Note PRPR *MUST* be 2, otherwise we could not
        ; execute from this address:

	org	1000h
	assume	prpr:2
	jp	1123h
	call	1123h
	jp	923h
	call	923h
	expect	110
	 jp	123h
	endexpect
	expect	110
	 call	123h
	endexpect
