        cpu     st6225

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

        set     3,v
        res     5,12h

        jrs     3,v,pc
        jrr     5,12h,pc+1
