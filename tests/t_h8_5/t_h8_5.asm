                cpu     hd6475348
                page    0
                maxmode on
                padding off

                assume  br:0,dp:0

                add.b   r5,sp
                add.w   r2,r4
                add.b   @r4,r2
                add.w   @r4,r2
                add.b   @(20,r4),r2
                add.w   @(20:8,r4),r2
                add.b   @(20:16,r4),r2
                add.w   @(2000,r4),r2
                add.b   @(2000:16,r4),r2
                add.w   @-r4,r2
                add.b   @r4+,r2
                add.w   @20,r2
                add.b   @20:16,r2
                add.w   @2000,r2
                add.b   #5,r2
                add.w   #100,r2
                add.b:g #1,r2
                add.w:g #-1,r2
                add.b   #1,r2
                add.w   #-1,r2

                adds.w  #$10,r3

                addx.b  @($20,r4),r0

                and.b   @$f8:8,r1

                andc.b  #$fe,ccr

                bra     *+20
                bra     *+2000
                bt      *+20
                bt      *+2000
                brn     *+20
                brn     *+2000
                bf      *+20
                bf      *+2000
                bhi     *+20
                bhi     *+2000
                bls     *+20
                bls     *+2000
                bcc     *+20
                bcc     *+2000
                bhs     *+20
                bhs     *+2000
                bcs     *+20
                bcs     *+2000
                blo     *+20
                blo     *+2000
                bne     *+20
                bne     *+2000
                beq     *+20
                beq     *+2000
                bvc     *+20
                bvc     *+2000
                bvs     *+20
                bvs     *+2000
                bpl     *+20
                bpl     *+2000
                bmi     *+20
                bmi     *+2000
                bge     *+20
                bge     *+2000
                blt     *+20
                blt     *+2000
                bgt     *+20
                bgt     *+2000
                ble     *+20
                ble     *+2000

                bclr.b  #7,@$ff00

                bnot.w  r0,r1

                bset.b  #0,@r1+

                bsr     *+20
                bsr     *+2000

                btst.b  r0,@$f0:8

                clr.w   @($1000,r5)

                cmp:g.b #$aa,@-r3
                cmp.b   #$aa,@-r3
                cmp:e.b #$00,r0
                cmp.b   #$00,r0
                cmp:i.w #$ffff,r1
                cmp.w   #$ffff,r1

                dadd    r0,r1

                divxu.w @r3,r0

                dsub    r2,r3

                exts    r0

                extu    r1

                jmp     @(#$10,r4)

                jsr     @($fff,r3)

                ldc.b   #$01,dp

                ldm     @sp+,(r0,r2-r4)

                link    fp,#-4

                mov:g.w r0,@r1
                mov.w   r0,@r1
                mov:e.b #$55,r0
                mov.b   #$55,r0
                mov:f.b @(4,r6),r0
                mov.b   @(4,r6),r0
                mov:f.b r0,@(4,r6)
                mov.b   r0,@(4,r6)
                mov:i.w #$ff00,r5
                mov.w   #$ff00,r5
                mov:l.b @$a0,r0
                mov.b   @$a0,r0
                mov:s.w r0,@$a0:8
                mov.w   r0,@$a0:8

                movfpe  @$f000,r0

                movtpe  r0,@r1

                mulxu.b r0,r1

                neg.w   r0

                nop

                not.b   @($10,r2)

                or.b    @$f0:8,r1

                orc.w   #$0700,sr

                pjmp    @r4

                pjsr    @$010000

                prtd    #8

                prts

                rotl.w  r0

                rotr.b  @r1

                rotxl.w @($02,r1)

                rotxr.b @$fa:8

                rtd     #400

                rte

                rts

                scb/eq  r4,*-20

                shal.b  @r2+

                shar.w  @$ff00

                shll.b  r1

                shlr.w  @-r1

                sleep

                stc.b   br,@-sp

                stm     (r0-r3),@-sp

                sub.w   @r1,r0

                subs.w  #2,r2

                subx.w  @r2+,r0

                swap    r0

                tas     @$f000

                trapa   #4

                trap/vs

                tst     @($1000,r1)

                unlk    fp

                xch     r0,r1

                xor.b   @$a0:8,r0

                xorc.b  #$01,ccr

                bra     *+126
                bra     *+127
                bra     *+128
                bra     *+129
                bra     *+130
                bra     *+131
                bra     *+132
                bra     *+133
                bra     *+134
                bra     *-123
                bra     *-124
                bra     *-125
                bra     *-126
                bra     *-127
                bra     *-128
                bra     *-129
                bra     *-130
                bra     *-131

