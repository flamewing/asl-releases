        cpu     68hc05
        page    0

        brset0  $12,*
        brset1  $12,*+1
        brset2  $12,*+2
        brset3  $12,*+3
        brset4  $12,*+4
        brset5  $12,*+5
        brset6  $12,*+6
        brset7  $12,*+7
        brclr0  $12,*+8
        brclr1  $12,*+9
        brclr2  $12,*+10
        brclr3  $12,*+11
        brclr4  $12,*+12
        brclr5  $12,*+13
        brclr6  $12,*+14
        brclr7  $12,*+15

        bset0   $12
        bset1   $12
        bset2   $12
        bset3   $12
        bset4   $12
        bset5   $12
        bset6   $12
        bset7   $12
        bclr0   $12
        bclr1   $12
        bclr2   $12
        bclr3   $12
        bclr4   $12
        bclr5   $12
        bclr6   $12
        bclr7   $12

        bra     *-1
        brn     *-2
        bhi     *-3
        bls     *-4
        bcc     *-5
        bcs     *-6
        bne     *-7
        beq     *-8
        bhcc    *-9
        bhcs    *-10
        bpl     *-11
        bmi     *-12
        bmc     *-13
        bms     *-14
        bil     *-15
        bih     *-16

        neg     $12
        nega
        negx
        neg     $12,x
        neg     ,x

        com     $12
        coma
        comx
        com     $12,x
        com     ,x

        lsr     $12
        lsra
        lsrx
        lsr     $12,x
        lsr     ,x

        ror     $12
        rora
        rorx
        ror     $12,x
        ror     ,x

        asr     $12
        asra
        asrx
        asr     $12,x
        asr     ,x

        lsl     $12
        lsla
        lslx
        lsl     $12,x
        lsl     ,x

        rol     $12
        rola
        rolx
        rol     $12,x
        rol     ,x

        dec     $12
        deca
        decx
        dec     $12,x
        dec     ,x

        inc     $12
        inca
        incx
        inc     $12,x
        inc     ,x

        tst     $12
        tsta
        tstx
        tst     $12,x
        tst     ,x

        clr     $12
        clra
        clrx
        clr     $12,x
        clr     ,x

        mul
        rti
        rts
        swi
        tax
        clc
        sec
        cli
        sei
        rsp
        nop
        stop
        wait
        txa

        sub     #$12
        sub     $12
        sub     $1234
        sub     $1234,x
        sub     $12,x
        sub     ,x

        cmp     #$12
        cmp     $12
        cmp     $1234
        cmp     $1234,x
        cmp     $12,x
        cmp     ,x

        cpx     #$12
        cpx     $12
        cpx     $1234
        cpx     $1234,x
        cpx     $12,x
        cpx     ,x

        sbc     #$12
        sbc     $12
        sbc     $1234
        sbc     $1234,x
        sbc     $12,x
        sbc     ,x

        and     #$12
        and     $12
        and     $1234
        and     $1234,x
        and     $12,x
        and     ,x

        bit     #$12
        bit     $12
        bit     $1234
        bit     $1234,x
        bit     $12,x
        bit     ,x

        lda     #$12
        lda     $12
        lda     $1234
        lda     $1234,x
        lda     $12,x
        lda     ,x

        sta     $12
        sta     $1234
        sta     $1234,x
        sta     $12,x
        sta     ,x

        eor     #$12
        eor     $12
        eor     $1234
        eor     $1234,x
        eor     $12,x
        eor     ,x

        adc     #$12
        adc     $12
        adc     $1234
        adc     $1234,x
        adc     $12,x
        adc     ,x

        ora     #$12
        ora     $12
        ora     $1234
        ora     $1234,x
        ora     $12,x
        ora     ,x

        add     #$12
        add     $12
        add     $1234
        add     $1234,x
        add     $12,x
        add     ,x

        jmp     $12
        jmp     $1234
        jmp     $1234,x
        jmp     $12,x
        jmp     ,x

        bsr     *
        jsr     $12
        jsr     $1234
        jsr     $1234,x
        jsr     $12,x
        jsr     ,x

        ldx     #$12
        ldx     $12
        ldx     $1234
        ldx     $1234,x
        ldx     $12,x
        ldx     ,x

        stx     $12
        stx     $1234
        stx     $1234,x
        stx     $12,x
        stx     ,x

