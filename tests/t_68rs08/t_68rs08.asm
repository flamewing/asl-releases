        cpu     68RS08
        page    0

        add     <$0a
        add     >$0a
        lda     <$1a
        lda     >$1a

        adc     #$55
        adc     $20
        adc     ,x
        adc     x

        add     #$55
        add     $20
        add     $0a
        add     ,x
        add     x

        and     #$55
        and     $20
        and     ,x
        and     x

        asla

        bcc     *

        bclr    0,$20
        bclr    1,$20
        bclr    2,$20
        bclr    3,$20
        bclr    4,$20
        bclr    5,$20
        bclr    6,$20
        bclr    7,$20
        bclr    0,d[x]
        bclr    1,d[x]
        bclr    2,d[x]
        bclr    3,d[x]
        bclr    4,d[x]
        bclr    5,d[x]
        bclr    6,d[x]
        bclr    7,d[x]
        bclr    0,x
        bclr    1,x
        bclr    2,x
        bclr    3,x
        bclr    4,x
        bclr    5,x
        bclr    6,x
        bclr    7,x

        bcs     *

        beq     *

        bgnd

        bhs     *

        blo     *

        bne     *

        bra     *

        brclr   0,$20,*
        brclr   1,$20,*
        brclr   2,$20,*
        brclr   3,$20,*
        brclr   4,$20,*
        brclr   5,$20,*
        brclr   6,$20,*
        brclr   7,$20,*
        brclr   0,d[x],*
        brclr   1,d[x],*
        brclr   2,d[x],*
        brclr   3,d[x],*
        brclr   4,d[x],*
        brclr   5,d[x],*
        brclr   6,d[x],*
        brclr   7,d[x],*
        brclr   0,x,*
        brclr   1,x,*
        brclr   2,x,*
        brclr   3,x,*
        brclr   4,x,*
        brclr   5,x,*
        brclr   6,x,*
        brclr   7,x,*

        brn     *

        brset   0,$20,*
        brset   1,$20,*
        brset   2,$20,*
        brset   3,$20,*
        brset   4,$20,*
        brset   5,$20,*
        brset   6,$20,*
        brset   7,$20,*
        brset   0,d[x],*
        brset   1,d[x],*
        brset   2,d[x],*
        brset   3,d[x],*
        brset   4,d[x],*
        brset   5,d[x],*
        brset   6,d[x],*
        brset   7,d[x],*
        brset   0,x,*
        brset   1,x,*
        brset   2,x,*
        brset   3,x,*
        brset   4,x,*
        brset   5,x,*
        brset   6,x,*
        brset   7,x,*

        bset    0,$20
        bset    1,$20
        bset    2,$20
        bset    3,$20
        bset    4,$20
        bset    5,$20
        bset    6,$20
        bset    7,$20
        bset    0,d[x]
        bset    1,d[x]
        bset    2,d[x]
        bset    3,d[x]
        bset    4,d[x]
        bset    5,d[x]
        bset    6,d[x]
        bset    7,d[x]
        bset    0,x
        bset    1,x
        bset    2,x
        bset    3,x
        bset    4,x
        bset    5,x
        bset    6,x
        bset    7,x

        bsr     *

        cbeq    $20,*
        cbeq    ,x,*
        cbeq    x,*
        cbeqa   #$55,*

        clc

        clr     $20
        clr     $17
        clr     ,x
        clra
        clrx

        cmp     #$55
        cmp     $20
        cmp     ,x
        cmp     x

        coma

        dbnz    $20,*
        dbnz    ,x,*
        dbnza   *
        dbnzx   *

        dec     $20
        dec     $0a
        dec     ,x
        dec     x
        deca
        decx

        eor     #$55
        eor     $20
        eor     ,x
        eor     x

        inc     $20
        inc     $0a
        inc     ,x
        inc     x
        inca
        incx

        jmp     $2030

        jsr     $2030

        lda     #$55
        lda     $20
        lda     $17
        lda     ,x

        ldx     #$55
        ldx     $20
        ldx     ,x

        lsla

        lsra

        mov     #$55,$20
        mov     $20,$40
        mov     d[x],$20
        mov     $20,d[x]
        mov     #$55,d[x]

        nop

        ora     #$55
        ora     $20
        ora     ,x
        ora     x

        rola

        rora

        rts

        sbc     #$55
        sbc     $20
        sbc     ,x
        sbc     x

        sec
	
        sha
	
        sla

        sta     $20
        sta     $17
        sta     ,x
        sta     x

        stop

        stx     $20

        sub     #$55
        sub     $20
        sub     $0a
        sub     ,x
        sub     x

        tax

        tst     $20
        tst     ,x
        tst     x
        tsta
        tstx

        txa

        wait
