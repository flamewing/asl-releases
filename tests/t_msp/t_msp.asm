        cpu     msp430

        include regmsp.inc

ede     equ     0f016h
toni    equ     01114h

        mov     r12,r7
        mov     pc,r7
        mov     sp,r7
        mov     sr,r7

        mov     2(r5),6(r6)

        mov     ede,toni

        mov     &ede,&toni

        mov     @r10,0(r11)

        mov     @r10+,0(r11)

        mov     #45,toni
        mov     #0,toni
        mov     #1,toni
        mov     #2,toni
        mov     #4,toni
        mov     #8,toni
        mov     #-1,toni

        dadd    #45,r4

        rrc     r5
        rra.b   toni
        push    pc
        swpb    &ede
        call    1234h
        sxt     @r5+

        reti

        jmp     234h
        jne     $
        jn      $+2

;----------------------------------
; emulierte Befehle

        adc     r6
        adc.w   r6
        adc.b   r6
        dadc    @r4
        dadc.w  @r4
        dadc.b  @r4
        dec     toni
        dec.w   toni
        dec.b   toni
        decd    &toni
        decd.w  &toni
        decd.b  &toni
        inc     ede
        inc.w   ede
        inc.b   ede
        incd    &ede
        incd.w  &ede
        incd.b  &ede
        sbc     55h(r9)
        sbc.w   55h(r9)
        sbc.b   55h(r9)

        inv     @r6
        inv.w   @r6
        inv.b   @r6
        rla     r5
        rla.w   r5
        rla.b   r5
        rlc     @r14
        rlc.w   @r14
        rlc.b   @r14

        clr     0(r10)
        clr.w   0(r10)
        clr.b   0(r10)
        clrc
        clrn
        clrz
        pop     sr
        setc
        setn
        setz
        tst     toni
        tst.w   toni
        tst.b   toni

        br      r5
        dint
        eint
        nop
        ret

        padding on
        .byte   1,2,3,4
        .byte   "Hello world"
        .byte   "Hello world!"
        .word   1,2,3,4
        .bss    20
        .bss    21

        padding off
        .byte   1,2,3,4
        .byte   "Hello world"
        .byte   "Hello world!"
        .word   1,2,3,4
        .bss    20
        .bss    21

