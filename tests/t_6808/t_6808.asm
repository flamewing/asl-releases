        cpu     68HC08
        page    0

        adc     #$55
        adc     $20
        adc     <$20
        adc     >$20
        adc     $2030
        adc     >$2030
        adc     ,x
        adc     0,x
        adc     <0,x
        adc     $40,x
        adc     <$40,x
        adc     >$40,x
        adc     $4050,x
        adc     >$4050,x
        adc     ,sp
        adc     0,sp
        adc     <0,sp
        adc     $40,sp
        adc     <$40,sp
        adc     >$40,sp
        adc     $4050,sp
        adc     >$4050,sp

        add     #$55
        add     $20
        add     $2030
        add     ,x
        add     $40,x
        add     $4050,x
        add     $40,sp
        add     $4050,sp

        aix     #34

        ais     #-16

        and     #$55
        and     $20
        and     $2030
        and     ,x
        and     $40,x
        and     $4050,x
        and     $40,sp
        and     $4050,sp

        asla
        aslx
        asl     $20
        asl     ,x
        asl     $40,x
        asl     $40,sp

        asra
        asrx
        asr     $20
        asr     ,x
        asr     $40,x
        asr     $40,sp

        bcc     *

        bclr    0,$20
        bclr    1,$20
        bclr    2,$20
        bclr    3,$20
        bclr    4,$20
        bclr    5,$20
        bclr    6,$20
        bclr    7,$20

        bcs     *

        beq     *

        bge     *

        bgt     *

        bhcc    *

        bhcs    *

        bhi     *

        bhs     *

        bih     *

        bil     *

        bit     #$55
        bit     $20
        bit     $2030
        bit     ,x
        bit     $40,x
        bit     $4050,x
        bit     $40,sp
        bit     $4050,sp

        ble     *

        blo     *

        bls     *

        blt     *

        bmc     *

        bmi     *

        bms     *

        bne     *

        bpl     *

        bra     *

        brclr   0,$20,*
        brclr   1,$20,*
        brclr   2,$20,*
        brclr   3,$20,*
        brclr   4,$20,*
        brclr   5,$20,*
        brclr   6,$20,*
        brclr   7,$20,*

        brn     *

        brset   0,$20,*
        brset   1,$20,*
        brset   2,$20,*
        brset   3,$20,*
        brset   4,$20,*
        brset   5,$20,*
        brset   6,$20,*
        brset   7,$20,*

        bset    0,$20
        bset    1,$20
        bset    2,$20
        bset    3,$20
        bset    4,$20
        bset    5,$20
        bset    6,$20
        bset    7,$20

        bsr     *

        cbeq    $20,*
        cbeq    x+,*
        cbeq    $40,x+,*
        cbeq    $40,sp,*

        cbeqa   #$55,*
        cbeqx   #$66,*

        clc

        cli

        clra
        clrx
        clrh
        clr     $20
        clr     ,x
        clr     $40,x
        clr     $40,sp

        cmp     #$55
        cmp     $20
        cmp     $2030
        cmp     ,x
        cmp     $40,x
        cmp     $4050,x
        cmp     $40,sp
        cmp     $4050,sp

        coma
        comx
        com     $20
        com     ,x
        com     $40,x
        com     $40,sp

        cphx    #$55aa
        cphx    $20

        cpx     #$55
        cpx     $20
        cpx     $2030
        cpx     ,x
        cpx     $40,x
        cpx     $4050,x
        cpx     $40,sp
        cpx     $4050,sp

        daa

        dbnza   *
        dbnzx   *

        dbnz    $20,*
        dbnz    ,x,*
        dbnz    $40,x,*
        dbnz    $40,sp,*

        deca
        decx
        dec     $20
        dec     ,x
        dec     $40,x
        dec     $40,sp

        div

        eor     #$55
        eor     $20
        eor     $2030
        eor     ,x
        eor     $40,x
        eor     $4050,x
        eor     $40,sp
        eor     $4050,sp

        inca
        incx
        inc     $20
        inc     ,x
        inc     $40,x
        inc     $40,sp

        jmp     $20
        jmp     $2030
        jmp     ,x
        jmp     $40,x
        jmp     $4050,x

        jsr     $20
        jsr     $2030
        jsr     ,x
        jsr     $40,x
        jsr     $4050,x

        lda     #$55
        lda     $20
        lda     $2030
        lda     ,x
        lda     $40,x
        lda     $4050,x
        lda     $40,sp
        lda     $4050,sp

        ldhx    #$55aa
        ldhx    $20

        ldx     #$55
        ldx     $20
        ldx     $2030
        ldx     ,x
        ldx     $40,x
        ldx     $4050,x
        ldx     $40,sp
        ldx     $4050,sp

        lsla
        lslx
        lsl     $20
        lsl     ,x
        lsl     $40,x
        lsl     $40,sp

        lsra
        lsrx
        lsr     $20
        lsr     ,x
        lsr     $40,x
        lsr     $40,sp

        mov     #$55,$20
        mov     $20,$40
        mov     x+,$20
        mov     $40,x+

        mul

        nega
        negx
        neg     $20
        neg     ,x
        neg     $40,x
        neg     $40,sp

        nop

        nsa

        ora     #$55
        ora     $20
        ora     $2030
        ora     ,x
        ora     $40,x
        ora     $4050,x
        ora     $40,sp
        ora     $4050,sp

        psha

        pshh

        pshx

        pula

        pulh

        pulx

        rola
        rolx
        rol     $20
        rol     ,x
        rol     $40,x
        rol     $40,sp

        rora
        rorx
        ror     $20
        ror     ,x
        ror     $40,x
        ror     $40,sp

        rsp

        rti

        rts

        sbc     #$55
        sbc     $20
        sbc     $2030
        sbc     ,x
        sbc     $40,x
        sbc     $4050,x
        sbc     $40,sp
        sbc     $4050,sp

        sec

        sei

        sta     $20
        sta     $2030
        sta     ,x
        sta     $40,x
        sta     $4050,x
        sta     $40,sp
        sta     $4050,sp

        sthx    $20

        stop

        stx     $20
        stx     $2030
        stx     ,x
        stx     $40,x
        stx     $4050,x
        stx     $40,sp
        stx     $4050,sp

        sub     #$55
        sub     $20
        sub     $2030
        sub     ,x
        sub     $40,x
        sub     $4050,x
        sub     $40,sp
        sub     $4050,sp

        swi

        tap

        tax

        tpa

        tsta
        tstx
        tst     $20
        tst     ,x
        tst     $40,x
        tst     $40,sp

        tsx

        txa

        txs

        wait
