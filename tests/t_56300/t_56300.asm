        cpu     56300
        page    0

        org     $8000

        abs     a		; >=56000
        abs     b

        adc	x,a		; >=56000
        adc	y,a
        adc	x,b
        adc	y,b

        add	x0,a		; >=56000
        add	x1,a
        add	y0,a
        add	y1,a
        add	x,a
        add	y,a
        add	b,a
        add	x0,b		; >=56000
        add	x1,b
        add	y0,b
        add	y1,b
        add	x,b
        add	y,b
        add	a,b

	add	#31,a		; >=56300
        add	#1234,a
        add	#31,b
        add	#1234,b

	addl	b,a		; >=56000
        addl	a,b

        addr	b,a
        addr	a,b

        and	x0,a		; >=56000
	and	x1,a
        and	y0,a
        and	y1,a
        and	x0,b
        and	x1,b
        and	y0,b
        and	y1,b

        and	#31,a		; >=56000
        and	#1234,a
        and	#31,b
        and	#1234,b

        andi    #$12,mr         ; >=56000
        andi    #$12,ccr
        andi    #$12,com
        andi    #$12,eom

        asl     a               ; >=56000
        asl     b
        asl     #$23,a,a        ; >=56300
        asl     #$23,a,b
        asl     #$23,b,a
        asl     #$23,b,b
        asl     a1,a,a          ; >=56300
        asl     b1,a,a
        asl     x0,a,a
        asl     y0,a,a
        asl     x1,a,a
        asl     y1,a,a
        asl     a1,a,b
        asl     b1,a,b
        asl     x0,a,b
        asl     y0,a,b
        asl     x1,a,b
        asl     y1,a,b
        asl     a1,b,a
        asl     b1,b,a
        asl     x0,b,a
        asl     y0,b,a
        asl     x1,b,a
        asl     y1,b,a
        asl     a1,b,b
        asl     b1,b,b
        asl     x0,b,b
        asl     y0,b,b
        asl     x1,b,b
        asl     y1,b,b

        asr     a               ; >=56000
        asr     b
        asr     #$23,a,a        ; >=56300
        asr     #$23,a,b
        asr     #$23,b,a
        asr     #$23,b,b
        asr     a1,a,a          ; >=56300
        asr     b1,a,a
        asr     x0,a,a
        asr     y0,a,a
        asr     x1,a,a
        asr     y1,a,a
        asr     a1,a,b
        asr     b1,a,b
        asr     x0,a,b
        asr     y0,a,b
        asr     x1,a,b
        asr     y1,a,b
        asr     a1,b,a
        asr     b1,b,a
        asr     x0,b,a
        asr     y0,b,a
        asr     x1,b,a
        asr     y1,b,a
        asr     a1,b,b
        asr     b1,b,b
        asr     x0,b,b
        asr     y0,b,b
        asr     x1,b,b
        asr     y1,b,b

        bcc     *+$2000
        bge     *+$20
        bne     r3
        bpl     *-$2000
        bnn     *-$20
        bec     r4
        blc     >*+$20
        bgt     <*-$20
        bcs     *+$2000
        blt     *+$20
        beq     r5
        bmi     *-$2000
        bnr     *-$20
        bes     r6
        bls     >*+$20
        ble     <*-$20
        bhs     *+$2000
        blo     *+$20

        bchg    #2,x:(r1)-n1    ; >=56000
        bchg    #2,y:(r1)-n1
        bchg    #3,x:(r2)+n2
        bchg    #3,y:(r2)+n2
        bchg    #4,x:(r3)-
        bchg    #4,y:(r3)-
        bchg    #5,x:(r4)+
        bchg    #5,y:(r4)+
        bchg    #6,x:(r5)
        bchg    #6,y:(r5)
        bchg    #7,x:(r6+n6)
        bchg    #7,y:(r6+n6)
        bchg    #8,x:-(r7)
        bchg    #8,y:-(r7)
        bchg    #9,x:$1234
        bchg    #9,y:$1234
        bchg    #11,x:$20
        bchg    #11,y:$20
        bchg    #13,x:$ffffca
        bchg    #13,y:$ffffca
        bchg    #7,x:$ffff95    ; >=56300
        bchg    #7,y:$ffff95
        bchg    #10,r6          ; >=56000
        bchg    #10,b

        bclr    #2,x:(r1)-n1    ; >=56000
        bclr    #2,y:(r1)-n1
        bclr    #3,x:(r2)+n2
        bclr    #3,y:(r2)+n2
        bclr    #4,x:(r3)-
        bclr    #4,y:(r3)-
        bclr    #5,x:(r4)+
        bclr    #5,y:(r4)+
        bclr    #6,x:(r5)
        bclr    #6,y:(r5)
        bclr    #7,x:(r6+n6)
        bclr    #7,y:(r6+n6)
        bclr    #8,x:-(r7)
        bclr    #8,y:-(r7)
        bclr    #9,x:$1234
        bclr    #9,y:$1234
        bclr    #11,x:$20
        bclr    #11,y:$20
        bclr    #13,x:$ffffca
        bclr    #13,y:$ffffca
        bclr    #7,x:$ffff95    ; >=56300
        bclr    #7,y:$ffff95
        bclr    #10,r6          ; >=56000
        bclr    #10,b

        bra     *+$2000
        bra     *+$20
        bra     *-$2000
        bra     *-$20
        bra     >*+$20
        bra     >*-$20
        bra     r5

        brclr   #2,x:(r1)-n1,*  ; >=56300
        brclr   #2,y:(r1)-n1,*
        brclr   #3,x:(r2)+n2,*
        brclr   #3,y:(r2)+n2,*
        brclr   #4,x:(r3)-,*
        brclr   #4,y:(r3)-,*
        brclr   #5,x:(r4)+,*
        brclr   #5,y:(r4)+,*
        brclr   #6,x:(r5),*
        brclr   #6,y:(r5),*
        brclr   #7,x:(r6+n6),*
        brclr   #7,y:(r6+n6),*
        brclr   #8,x:-(r7),*
        brclr   #8,y:-(r7),*
        brclr   #11,x:$20,*
        brclr   #11,y:$20,*
        brclr   #13,x:$ffffca,*
        brclr   #13,y:$ffffca,*
        brclr   #7,x:$ffff95,*
        brclr   #7,y:$ffff95,*
        brclr   #10,r6,*
        brclr   #10,b,*

        brkcc
        brkge
        brkne
        brkpl
        brknn
        brkec
        brklc
        brkgt
        brkcs
        brklt
        brkeq
        brkmi
        brknr
        brkes
        brkls
        brkle
        brkhs
        brklo

        brset   #2,x:(r1)-n1,*  ; >=56300
        brset   #2,y:(r1)-n1,*
        brset   #3,x:(r2)+n2,*
        brset   #3,y:(r2)+n2,*
        brset   #4,x:(r3)-,*
        brset   #4,y:(r3)-,*
        brset   #5,x:(r4)+,*
        brset   #5,y:(r4)+,*
        brset   #6,x:(r5),*
        brset   #6,y:(r5),*
        brset   #7,x:(r6+n6),*
        brset   #7,y:(r6+n6),*
        brset   #8,x:-(r7),*
        brset   #8,y:-(r7),*
        brset   #11,x:$20,*
        brset   #11,y:$20,*
        brset   #13,x:$ffffca,*
        brset   #13,y:$ffffca,*
        brset   #7,x:$ffff95,*
        brset   #7,y:$ffff95,*
        brset   #10,r6,*
        brset   #10,b,*

        bscc    *+$2000
        bsge    *+$20
        bsne    r3
        bspl    *-$2000
        bsnn    *-$20
        bsec    r4
        bslc    >*+$20
        bsgt    <*-$20
        bscs    *+$2000
        bslt    *+$20
        bseq    r5
        bsmi    *-$2000
        bsnr    *-$20
        bses    r6
        bsls    >*+$20
        bsle    <*-$20
        bshs    *+$2000
        bslo    *+$20

        bsclr   #2,x:(r1)-n1,*  ; >=56300
        bsclr   #2,y:(r1)-n1,*
        bsclr   #3,x:(r2)+n2,*
        bsclr   #3,y:(r2)+n2,*
        bsclr   #4,x:(r3)-,*
        bsclr   #4,y:(r3)-,*
        bsclr   #5,x:(r4)+,*
        bsclr   #5,y:(r4)+,*
        bsclr   #6,x:(r5),*
        bsclr   #6,y:(r5),*
        bsclr   #7,x:(r6+n6),*
        bsclr   #7,y:(r6+n6),*
        bsclr   #8,x:-(r7),*
        bsclr   #8,y:-(r7),*
        bsclr   #11,x:$20,*
        bsclr   #11,y:$20,*
        bsclr   #13,x:$ffffca,*
        bsclr   #13,y:$ffffca,*
        bsclr   #7,x:$ffff95,*
        bsclr   #7,y:$ffff95,*
        bsclr   #10,r6,*
        bsclr   #10,b,*

        bset    #2,x:(r1)-n1    ; >=56000
        bset    #2,y:(r1)-n1
        bset    #3,x:(r2)+n2
        bset    #3,y:(r2)+n2
        bset    #4,x:(r3)-
        bset    #4,y:(r3)-
        bset    #5,x:(r4)+
        bset    #5,y:(r4)+
        bset    #6,x:(r5)
        bset    #6,y:(r5)
        bset    #7,x:(r6+n6)
        bset    #7,y:(r6+n6)
        bset    #8,x:-(r7)
        bset    #8,y:-(r7)
        bset    #9,x:$1234
        bset    #9,y:$1234
        bset    #11,x:$20
        bset    #11,y:$20
        bset    #13,x:$ffffca
        bset    #13,y:$ffffca
        bset    #7,x:$ffff95    ; >=56300
        bset    #7,y:$ffff95
        bset    #10,r6          ; >=56000
        bset    #10,b

        bsr     *+$2000
        bsr     *+$20
        bsr     *-$2000
        bsr     *-$20
        bsr     >*+$20
        bsr     >*-$20
        bsr     r5

        bsset   #2,x:(r1)-n1,*  ; >=56300
        bsset   #2,y:(r1)-n1,*
        bsset   #3,x:(r2)+n2,*
        bsset   #3,y:(r2)+n2,*
        bsset   #4,x:(r3)-,*
        bsset   #4,y:(r3)-,*
        bsset   #5,x:(r4)+,*
        bsset   #5,y:(r4)+,*
        bsset   #6,x:(r5),*
        bsset   #6,y:(r5),*
        bsset   #7,x:(r6+n6),*
        bsset   #7,y:(r6+n6),*
        bsset   #8,x:-(r7),*
        bsset   #8,y:-(r7),*
        bsset   #11,x:$20,*
        bsset   #11,y:$20,*
        bsset   #13,x:$ffffca,*
        bsset   #13,y:$ffffca,*
        bsset   #7,x:$ffff95,*
        bsset   #7,y:$ffff95,*
        bsset   #10,r6,*
        bsset   #10,b,*

        btst    #2,x:(r1)-n1    ; >=56000
        btst    #2,y:(r1)-n1
        btst    #3,x:(r2)+n2
        btst    #3,y:(r2)+n2
        btst    #4,x:(r3)-
        btst    #4,y:(r3)-
        btst    #5,x:(r4)+
        btst    #5,y:(r4)+
        btst    #6,x:(r5)
        btst    #6,y:(r5)
        btst    #7,x:(r6+n6)
        btst    #7,y:(r6+n6)
        btst    #8,x:-(r7)
        btst    #8,y:-(r7)
        btst    #9,x:$1234
        btst    #9,y:$1234
        btst    #11,x:$20
        btst    #11,y:$20
        btst    #13,x:$ffffca
        btst    #13,y:$ffffca
        btst    #7,x:$ffff95    ; >=56300
        btst    #7,y:$ffff95
        btst    #10,r6          ; >=56000
        btst    #10,b

        clb     a,a             ; >=56300
        clb     a,b
        clb     b,a
        clb     b,b

        clr     a               ; >=56000
        clr     b

        cmp     x0,a            ; >=56000
        cmp     x1,a
        cmp     y0,a
        cmp     y1,a
        cmp     x,a
        cmp     y,a
        cmp     b,a
        cmp     x0,b            ; >=56000
        cmp     x1,b
        cmp     y0,b
        cmp     y1,b
        cmp     x,b
        cmp     y,b
        cmp     a,b

        cmp     #31,a           ; >=56300
        cmp     #1234,a
        cmp     #31,b
        cmp     #1234,b

        cmpm    x0,a            ; >=56000
        cmpm    x1,a
        cmpm    y0,a
        cmpm    y1,a
        cmpm    b,a
        cmpm    x0,b
        cmpm    x1,b
        cmpm    y0,b
        cmpm    y1,b
        cmpm    a,b

        cmpu    x0,a            ; >=56300
        cmpu    x1,a
        cmpu    y0,a
        cmpu    y1,a
        cmpu    b,a
        cmpu    x0,b
        cmpu    x1,b
        cmpu    y0,b
        cmpu    y1,b
        cmpu    a,b

        debug                   ; >=56300
        debugcc
        debugge
        debugne
        debugpl
        debugnn
        debugec
        debuglc
        debuggt
        debugcs
        debuglt
        debugeq
        debugmi
        debugnr
        debuges
        debugls
        debugle
        debughs
        debuglo

        dec     a               ; >=56002
        dec     b

        div     x0,a            ; >=56000
        div     x1,a
        div     y0,a
        div     y1,a
        div     x0,b
        div     x1,b
        div     y0,b
        div     y1,b

        dmacss  +x0,x0,a        ; >=56300
        dmacss  -y0,y0,a
        dmacss  +x1,x0,b
        dmacss  -y1,y0,b
        dmacss  +x1,x1,a
        dmacss  -y1,y1,a
        dmacss  +x0,x1,b
        dmacss  -y0,y1,b
        dmacss  +x0,y1,a
        dmacss  -y0,x0,a
        dmacss  +x1,y0,b
        dmacss  -y1,x1,b
        dmacss  +y1,x0,a
        dmacss  -x0,y0,a
        dmacss  +y0,x1,b
        dmacss  -x1,y1,b

        dmacsu  +x0,x0,a        ; >=56300
        dmacsu  -y0,y0,a
        dmacsu  +x1,x0,b
        dmacsu  -y1,y0,b
        dmacsu  +x1,x1,a
        dmacsu  -y1,y1,a
        dmacsu  +x0,x1,b
        dmacsu  -y0,y1,b
        dmacsu  +x0,y1,a
        dmacsu  -y0,x0,a
        dmacsu  +x1,y0,b
        dmacsu  -y1,x1,b
        dmacsu  +y1,x0,a
        dmacsu  -x0,y0,a
        dmacsu  +y0,x1,b
        dmacsu  -x1,y1,b

        dmacuu  +x0,x0,a        ; >=56300
        dmacuu  -y0,y0,a
        dmacuu  +x1,x0,b
        dmacuu  -y1,y0,b
        dmacuu  +x1,x1,a
        dmacuu  -y1,y1,a
        dmacuu  +x0,x1,b
        dmacuu  -y0,y1,b
        dmacuu  +x0,y1,a
        dmacuu  -y0,x0,a
        dmacuu  +x1,y0,b
        dmacuu  -y1,x1,b
        dmacuu  +y1,x0,a
        dmacuu  -x0,y0,a
        dmacuu  +y0,x1,b
        dmacuu  -x1,y1,b

        do      x:(r1)-n1,*+2   ; >=56000
        do      y:(r1)-n1,*+2
        do      x:(r2)+n2,*+2
        do      y:(r2)+n2,*+2
        do      x:(r3)-,*+2
        do      y:(r3)-,*+2
        do      x:(r4)+,*+2
        do      y:(r4)+,*+2
        do      x:(r5),*+2
        do      y:(r5),*+2
        do      x:(r6+n6),*+2
        do      y:(r6+n6),*+2
        do      x:-(r7),*+2
        do      y:-(r7),*+2
        do      x:$12,*+2
        do      y:$12,*+2
        do      #$78,*+2
        do      #$678,*+2
        do      r4,*+2
        do      forever,*+2     ; >=56300

        dor     x:(r1)-n1,*+3   ; >=56300
        dor     y:(r1)-n1,*+3
        dor     x:(r2)+n2,*+3
        dor     y:(r2)+n2,*+3
        dor     x:(r3)-,*+3
        dor     y:(r3)-,*+3
        dor     x:(r4)+,*+3
        dor     y:(r4)+,*+3
        dor     x:(r5),*+3
        dor     y:(r5),*+3
        dor     x:(r6+n6),*+3
        dor     y:(r6+n6),*+3
        dor     x:-(r7),*+3
        dor     y:-(r7),*+3
        dor     x:$12,*+3
        dor     y:$12,*+3
        dor     #$78,*+3
        dor     #$678,*+3
        dor     r4,*+3
        dor     forever,*+3

        enddo                   ; >=56000

        eor     x0,a            ; >=56000
        eor     y0,a
        eor     x1,a
        eor     y1,a
        eor     x0,b
        eor     y0,b
        eor     x1,b
        eor     y1,b

        eor     #$34,a          ; >=56300
        eor     #$34,b
        eor     #$123456,a
        eor     #$123456,b

        extract a1,a,a          ; >=56300
        extract b1,a,a
        extract x0,a,a
        extract x1,a,a
        extract y0,a,a
        extract y1,a,a
        extract a1,b,a
        extract a1,a,b
        extract #$234567,a,a
        extract #$234567,a,b
        extract #$234567,b,a
        extract #$234567,b,b

        extractu a1,a,a         ; >=56300
        extractu b1,a,a
        extractu x0,a,a
        extractu x1,a,a
        extractu y0,a,a
        extractu y1,a,a
        extractu a1,b,a
        extractu a1,a,b
        extractu #$234567,a,a
        extractu #$234567,a,b
        extractu #$234567,b,a
        extractu #$234567,b,b

        abs     a       ifcc    ; >=56300
        abs     a       ifge
        abs     a       ifne
        abs     a       ifpl
        abs     a       ifnn
        abs     a       ifec
        abs     a       iflc
        abs     a       ifgt
        abs     a       ifcs
        abs     a       iflt
        abs     a       ifeq
        abs     a       ifmi
        abs     a       ifnr
        abs     a       ifes
        abs     a       ifls
        abs     a       ifle
        abs     a       ifhs
        abs     a       iflo

        abs     a       ifcc.u  ; >=56300
        abs     a       ifge.u
        abs     a       ifne.u
        abs     a       ifpl.u
        abs     a       ifnn.u
        abs     a       ifec.u
        abs     a       iflc.u
        abs     a       ifgt.u
        abs     a       ifcs.u
        abs     a       iflt.u
        abs     a       ifeq.u
        abs     a       ifmi.u
        abs     a       ifnr.u
        abs     a       ifes.u
        abs     a       ifls.u
        abs     a       ifle.u
        abs     a       ifhs.u
        abs     a       iflo.u

        illegal                 ; >=56000

        inc     a               ; >=56002
        inc     b

        insert  a1,a0,a         ; >=56300
        insert  b1,a0,a
        insert  x0,a0,a
        insert  y0,a0,a
        insert  x1,a0,a
        insert  y1,a0,a
        insert  a1,b0,a
        insert  a1,x0,a
        insert  a1,y0,a
        insert  a1,x1,a
        insert  a1,y1,a
        insert  a1,a0,b
        insert  #345678,a0,a
        insert  #345678,b0,a
        insert  #345678,x0,a
        insert  #345678,y0,a
        insert  #345678,x1,a
        insert  #345678,y1,a
        insert  #345678,a0,b

        jcc     $123
        jge     $123456
        jne     (r1)-n1
        jpl     (r2)+n2
        jnn     (r3)-
        jec     (r4)+
        jlc     (r5)
        jgt     (r6+n6)
        jcs     -(r7)
        jlt     $123
        jeq     $123456
        jmi     (r1)-n1
        jnr     (r2)+n2
        jes     (r3)-
        jls     (r4)+
        jle     (r5)
        jhs     (r6+n6)
        jlo     -(r7)

        jclr    #2,x:(r1)-n1,*  ; >=56000
        jclr    #2,y:(r1)-n1,*
        jclr    #3,x:(r2)+n2,*
        jclr    #3,y:(r2)+n2,*
        jclr    #4,x:(r3)-,*
        jclr    #4,y:(r3)-,*
        jclr    #5,x:(r4)+,*
        jclr    #5,y:(r4)+,*
        jclr    #6,x:(r5),*
        jclr    #6,y:(r5),*
        jclr    #7,x:(r6+n6),*
        jclr    #7,y:(r6+n6),*
        jclr    #8,x:-(r7),*
        jclr    #8,y:-(r7),*
        jclr    #11,x:$20,*
        jclr    #11,y:$20,*
        jclr    #13,x:$ffffca,*
        jclr    #13,y:$ffffca,*
        jclr    #7,x:$ffff95,*  ; >=56300
        jclr    #7,y:$ffff95,*
        jclr    #10,r6,*        ; >=56000
        jclr    #10,b,*

        jmp     $123            ; >=56000
        jmp     $123456
        jmp     (r1)-n1
        jmp     (r2)+n2
        jmp     (r3)-
        jmp     (r4)+
        jmp     (r5)
        jmp     (r6+n6)
        jmp     -(r7)

        jscc    $123            ; >=56000
        jsge    $123456
        jsne    (r1)-n1
        jspl    (r2)+n2
        jsnn    (r3)-
        jsec    (r4)+
        jslc    (r5)
        jsgt    (r6+n6)
        jscs    -(r7)
        jslt    $123
        jseq    $123456
        jsmi    (r1)-n1
        jsnr    (r2)+n2
        jses    (r3)-
        jsls    (r4)+
        jsle    (r5)
        jshs    (r6+n6)
        jslo    -(r7)

        jsclr   #2,x:(r1)-n1,*  ; >=56000
        jsclr   #2,y:(r1)-n1,*
        jsclr   #3,x:(r2)+n2,*
        jsclr   #3,y:(r2)+n2,*
        jsclr   #4,x:(r3)-,*
        jsclr   #4,y:(r3)-,*
        jsclr   #5,x:(r4)+,*
        jsclr   #5,y:(r4)+,*
        jsclr   #6,x:(r5),*
        jsclr   #6,y:(r5),*
        jsclr   #7,x:(r6+n6),*
        jsclr   #7,y:(r6+n6),*
        jsclr   #8,x:-(r7),*
        jsclr   #8,y:-(r7),*
        jsclr   #11,x:$20,*
        jsclr   #11,y:$20,*
        jsclr   #13,x:$ffffca,*
        jsclr   #13,y:$ffffca,*
        jsclr   #7,x:$ffff95,*  ; >=56300
        jsclr   #7,y:$ffff95,*
        jsclr   #10,r6,*        ; >=56000
        jsclr   #10,b,*

        jset    #2,x:(r1)-n1,*  ; >=56000
        jset    #2,y:(r1)-n1,*
        jset    #3,x:(r2)+n2,*
        jset    #3,y:(r2)+n2,*
        jset    #4,x:(r3)-,*
        jset    #4,y:(r3)-,*
        jset    #5,x:(r4)+,*
        jset    #5,y:(r4)+,*
        jset    #6,x:(r5),*
        jset    #6,y:(r5),*
        jset    #7,x:(r6+n6),*
        jset    #7,y:(r6+n6),*
        jset    #8,x:-(r7),*
        jset    #8,y:-(r7),*
        jset    #11,x:$20,*
        jset    #11,y:$20,*
        jset    #13,x:$ffffca,*
        jset    #13,y:$ffffca,*
        jset    #7,x:$ffff95,*  ; >=56300
        jset    #7,y:$ffff95,*
        jset    #10,r6,*        ; >=56000
        jset    #10,b,*

        jsr     $123            ; >=56000
        jsr     $123456
        jsr     (r1)-n1
        jsr     (r2)+n2
        jsr     (r3)-
        jsr     (r4)+
        jsr     (r5)
        jsr     (r6+n6)
        jsr     -(r7)

        jsset   #2,x:(r1)-n1,*  ; >=56000
        jsset   #2,y:(r1)-n1,*
        jsset   #3,x:(r2)+n2,*
        jsset   #3,y:(r2)+n2,*
        jsset   #4,x:(r3)-,*
        jsset   #4,y:(r3)-,*
        jsset   #5,x:(r4)+,*
        jsset   #5,y:(r4)+,*
        jsset   #6,x:(r5),*
        jsset   #6,y:(r5),*
        jsset   #7,x:(r6+n6),*
        jsset   #7,y:(r6+n6),*
        jsset   #8,x:-(r7),*
        jsset   #8,y:-(r7),*
        jsset   #11,x:$20,*
        jsset   #11,y:$20,*
        jsset   #13,x:$ffffca,*
        jsset   #13,y:$ffffca,*
        jsset   #7,x:$ffff95,*  ; >=56300
        jsset   #7,y:$ffff95,*
        jsset   #10,r6,*        ; >=56000
        jsset   #10,b,*

        lra     r3,x0           ; >=56000
        lra     r3,x1
        lra     r3,y0
        lra     r3,y1
        lra     r3,a0
        lra     r3,b0
        lra     r3,a2
        lra     r3,b2
        lra     r3,a1
        lra     r3,b1
        lra     r3,a
        lra     r3,b
        lra     r3,r5
        lra     r3,n5

        lra     *+2,x0          ; >=56000
        lra     *+2,x1
        lra     *+2,y0
        lra     *+2,y1
        lra     *+2,a0
        lra     *+2,b0
        lra     *+2,a2
        lra     *+2,b2
        lra     *+2,a1
        lra     *+2,b1
        lra     *+2,a
        lra     *+2,b
        lra     *+2,r5
        lra     *+2,n5

        lsl     a               ; >=56000
        lsl     b
        lsl     #$13,a          ; >=56300
        lsl     #$13,b
        lsl     a1,a
        lsl     b1,a
        lsl     x0,a
        lsl     y0,a
        lsl     x1,a
        lsl     y1,a
        lsl     a1,b
        lsl     b1,b
        lsl     x0,b
        lsl     y0,b
        lsl     x1,b
        lsl     y1,b

        lsr     a               ; >=56000
        lsr     b
        lsr     #$13,a          ; >=56300
        lsr     #$13,b
        lsr     a1,a
        lsr     b1,a
        lsr     x0,a
        lsr     y0,a
        lsr     x1,a
        lsr     y1,a
        lsr     a1,b
        lsr     b1,b
        lsr     x0,b
        lsr     y0,b
        lsr     x1,b
        lsr     y1,b

        lua     (r1)-n1,x0      ; >=56000
        lua     (r2)+n2,x1
        lua     (r3)-,y0
        lua     (r4)+,y1
        lua     (r5)-n5,a0
        lua     (r6)+n6,b0
        lua     (r7)-,a2
        lua     (r1)+,b2
        lua     (r2)-n2,a1
        lua     (r3)+n3,b1
        lua     (r4)-,a
        lua     (r5)+,b
        lua     (r6)-n6,r4
        lua     (r7)+n7,n6
        lua     (r2+20),r5      ; >=56300
        lua     (r3-20),n7

        mac     +x0,x0,a        ; >=56000
        mac     -y0,y0,a
        mac     +x1,x0,b
        mac     -y1,y0,b
        mac     +x0,y1,a
        mac     -y0,x0,a
        mac     +x1,y0,b
        mac     -y1,x1,b
        mac     +y1,#8,a        ; >=56300
        mac     -x0,#32,a
        mac     +y0,#128,b
        mac     -x1,#(1<<22),b

        maci    +#$123456,x0,a  ; >=56300
        maci    -#$234567,y0,a
        maci    +#$345678,x1,b
        maci    -#$456789,y1,b

        macsu   +x0,x0,a        ; >=56300
        macsu   -y0,y0,a
        macsu   +x1,x0,b
        macsu   -y1,y0,b
        macsu   +x1,x1,a
        macsu   -y1,y1,a
        macsu   +x0,x1,b
        macsu   -y0,y1,b
        macsu   +x0,y1,a
        macsu   -y0,x0,a
        macsu   +x1,y0,b
        macsu   -y1,x1,b
        macsu   +y1,x0,a
        macsu   -x0,y0,a
        macsu   +y0,x1,b
        macsu   -x1,y1,b

        macuu   +x0,x0,a        ; >=56300
        macuu   -y0,y0,a
        macuu   +x1,x0,b
        macuu   -y1,y0,b
        macuu   +x1,x1,a
        macuu   -y1,y1,a
        macuu   +x0,x1,b
        macuu   -y0,y1,b
        macuu   +x0,y1,a
        macuu   -y0,x0,a
        macuu   +x1,y0,b
        macuu   -y1,x1,b
        macuu   +y1,x0,a
        macuu   -x0,y0,a
        macuu   +y0,x1,b
        macuu   -x1,y1,b

        macr    +x0,x0,a        ; >=56000
        macr    -y0,y0,a
        macr    +x1,x0,b
        macr    -y1,y0,b
        macr    +x0,y1,a
        macr    -y0,x0,a
        macr    +x1,y0,b
        macr    -y1,x1,b
        macr    +y1,#8,a        ; >=56300
        macr    -x0,#32,a
        macr    +y0,#128,b
        macr    -x1,#(1<<22),b

        macri   +#$123456,x0,a  ; >=56300
        macri   -#$234567,y0,a
        macri   +#$345678,x1,b
        macri   -#$456789,y1,b

        max     a,b ifne        ; >=56300
        maxm    a,b

        merge   a1,a            ; >=56300
        merge   b1,a
        merge   x0,a
        merge   y0,a
        merge   x1,a
        merge   y1,a
        merge   a1,b
        merge   b1,b
        merge   x0,b
        merge   y0,b
        merge   x1,b
        merge   y1,b

        move                    ; >=56000

        move    #30,x0          ; >=56000
        move    #31,x1
        move    #32,y0
        move    #33,y1
        move    #34,a0
        move    #35,b0
        move    #36,a2
        move    #37,b2
        move    #38,a1
        move    #39,b1
        move    #40,a
        move    #41,b
        move    #42,r2
        move    #43,n4

        move    n4,x0           ; >=56000
        move    x0,x1
        move    x1,y0
        move    y0,y1
        move    y1,a0
        move    a0,b0
        move    b0,a2
        move    a2,b2
        move    b2,a1
        move    a1,b1
        move    b1,a
        move    a,b
        move    b,r2
        move    r2,n4

        move    (r1)-n1         ; >=56000
        move    (r2)+n2
        move    (r3)-
        move    (r4)+

        move    x:(r1)-n1,x0    ; >=56000
        move    x:(r2)+n2,x1
        move    x:(r3)-,y0
        move    x:(r4)+,y1
        move    x:(r5),a0
        move    x:(r6+n6),b0
        move    x:-(r7),a2
        move    x:$123456,b2
        move    x:#$123456,a1
        move    x:$12,b1
        move    a,x:(r1)-n1
        move    b,x:(r2)+n2
        move    r0,x:(r3)-
        move    r1,x:(r4)+
        move    r2,x:(r5)
        move    r3,x:(r6+n6)
        move    r4,x:-(r7)
        move    r5,x:$123456
        move    r6,x:$12
        move    n0,x:$12

        move    x:(r1+30),x0    ; >=56300
        move    x:(r2-30),x1
        move    x:(r3+300),y0
        move    x:(r4-300),y1
        move    x:(r5+30),n2
        move    x:(r6-30),n2
        move    x0,x:(r1+30)    ; >=56300
        move    x1,x:(r2-30)
        move    y0,x:(r3+300)
        move    y1,x:(r4-300)
        move    n2,x:(r5+30)
        move    n2,x:(r6-30)

        move    x0,x:(r1)-n1 a,y0 ; >=56000
        move    x1,x:(r2)+n2 a,y1
        move    a,x:(r3)-    b,y0
        move    b,x:(r4)+    b,y1
        move    x:(r5),x0    a,y0
        move    x:(r6+n6),x1 a,y1
        move    x:-(r7),a    b,y0
        move    x:$123,b     b,y1
        move    #$1234,a     a,y0

        move    a,x:(r1)-n1  x0,a ; >=56000
        move    b,x:(r2)+n2  x0,b

        move    y:(r1)-n1,x0    ; >=56000
        move    y:(r2)+n2,x1
        move    y:(r3)-,y0
        move    y:(r4)+,y1
        move    y:(r5),a0
        move    y:(r6+n6),b0
        move    y:-(r7),a2
        move    y:$123456,b2
        move    y:#$123456,a1
        move    y:$12,b1
        move    a,y:(r1)-n1
        move    b,y:(r2)+n2
        move    r0,y:(r3)-
        move    r1,y:(r4)+
        move    r2,y:(r5)
        move    r3,y:(r6+n6)
        move    r4,y:-(r7)
        move    r5,y:$123456
        move    r6,y:$12
        move    n0,y:$12

        move    y:(r1+30),x0    ; >=56300
        move    y:(r2-30),x1
        move    y:(r3+300),y0
        move    y:(r4-300),y1
        move    y:(r5+30),n2
        move    y:(r6-30),n2
        move    x0,y:(r1+30)    ; >=56300
        move    x1,y:(r2-30)
        move    y0,y:(r3+300)
        move    y1,y:(r4-300)
        move    n2,y:(r5+30)
        move    n2,y:(r6-30)

        move    a,x0 y0,y:(r1)-n1 ; >=56000
        move    a,x1 y1,y:(r2)+n2
        move    b,x0 a,y:(r3)-
        move    b,x1 b,y:(r4)+
        move    a,x0 y:(r5),y0
        move    a,x1 y:(r6+n6),y1
        move    b,x0 y:-(r7),a
        move    b,x1 y:$123,b
        move    a,x0 #$1234,a

        move    y0,a a,y:(r1)-n1 ; >=56000
        move    y0,b b,y:(r2)+n2

        move    l:(r1)-n1,a10 ; >=56000
        move    l:(r2)+n2,b10
        move    l:(r3)-,x
        move    l:(r4)+,y
        move    l:(r5),a
        move    l:(r6+n6),b
        move    l:-(r7),ab
        move    l:$123456,ba
        move    l:$12,ba

        move    a10,l:(r1)-n1 ; >=56000
        move    b10,l:(r2)+n2
        move    x,l:(r3)-
        move    y,l:(r4)+
        move    a,l:(r5)
        move    b,l:(r6+n6)
        move    ab,l:-(r7)
        move    ba,l:$123456
        move    ba,l:$12

        move    x:(r0)+n0,x0 y:(r4)+n4,y0 ; >=56000
        move    x:(r1)-,x1   y:(r5)-,y1
        move    x:(r2)+,a    y:(r6)+,b
        move    x:(r3),b     y:(r7),a
        move    x0,x:(r0)+n0 y0,y:(r4)+n4
        move    x1,x:(r1)-   y1,y:(r5)-
        move    a,x:(r2)+    b,y:(r6)+
        move    b,x:(r3)     a,y:(r7)

        movec   x:(r1)-n1,m0    ; >=56000
        movec   x:(r2)+n2,m1
        movec   x:(r3)-,m2
        movec   x:(r4)+,m3
        movec   x:(r5),m4
        movec   x:(r6+n6),m5
        movec   x:-(r7),m6
        movec   x:$123456,m7
        movec   x:$12,ep
        movec   r4,vba
        movec   #$123456,sc
        movec   #$12,sz
        movec   y:(r1)-n1,sr
        movec   y:(r2)+n2,omr
        movec   y:(r3)-,sp
        movec   y:(r4)+,ssh
        movec   y:(r5),ssl
        movec   y:(r6+n6),la
        movec   y:-(r7),lc

        movec   m0,x:(r1)-n1    ; >=56000
        movec   m1,x:(r2)+n2
        movec   m2,x:(r3)-
        movec   m3,x:(r4)+
        movec   m4,x:(r5)
        movec   m5,x:(r6+n6)
        movec   m6,x:-(r7)
        movec   m7,x:$123456
        movec   ep,x:$12
        movec   vba,r4
        movec   sr,y:(r1)-n1
        movec   omr,y:(r2)+n2
        movec   sp,y:(r3)-
        movec   ssh,y:(r4)+
        movec   ssl,y:(r5)
        movec   la,y:(r6+n6)
        movec   lc,y:-(r7)

        movem   r4,p:$123456    ; >=56000
        movem   p:$123456,r4
        movem   r4,p:$12
        movem   p:$12,r4

        movep   x:$ffffd2,x:$123456 ; >=56000
        movep   y:$ffffd2,x:$123456
        movep   x:$ffffd2,y:$123456
        movep   y:$ffffd2,y:$123456
        movep   x:$123456,x:$ffffd2
        movep   x:$123456,y:$ffffd2
        movep   y:$123456,x:$ffffd2
        movep   y:$123456,y:$ffffd2

        movep   x:$ffff92,x:$123456
        movep   x:$ffff92,y:$123456
        movep   x:$123456,x:$ffff92
        movep   y:$123456,x:$ffff92

        movep   y:$ffff92,x:$123456
        movep   y:$ffff92,y:$123456
        movep   x:$123456,y:$ffff92
        movep   y:$123456,y:$ffff92

        movep   x:$ffffd2,p:$123456
        movep   y:$ffffd2,p:$123456
        movep   p:$123456,x:$ffffd2
        movep   p:$123456,y:$ffffd2

        movep   x:$ffff92,p:$123456
        movep   y:$ffff92,p:$123456
        movep   p:$123456,x:$ffff92
        movep   p:$123456,y:$ffff92

        movep   x:$ffffd2,r4
        movep   y:$ffffd2,r4
        movep   r4,x:$ffffd2
        movep   r4,y:$ffffd2

        movep   x:$ffff92,r4
        movep   y:$ffff92,r4
        movep   r4,x:$ffff92
        movep   r4,y:$ffff92

        mpy     +x0,x0,a        ; >=56000
        mpy     -y0,y0,a
        mpy     +x1,x0,b
        mpy     -y1,y0,b
        mpy     +x0,y1,a
        mpy     -y0,x0,a
        mpy     +x1,y0,b
        mpy     -y1,x1,b
        mpy     +y1,#8,a        ; >=56300
        mpy     -x0,#32,a
        mpy     +y0,#128,b
        mpy     -x1,#(1<<22),b

        mpysu   +x0,x0,a        ; >=56300
        mpysu   -y0,y0,a
        mpysu   +x1,x0,b
        mpysu   -y1,y0,b
        mpysu   +x1,x1,a
        mpysu   -y1,y1,a
        mpysu   +x0,x1,b
        mpysu   -y0,y1,b
        mpysu   +x0,y1,a
        mpysu   -y0,x0,a
        mpysu   +x1,y0,b
        mpysu   -y1,x1,b
        mpysu   +y1,x0,a
        mpysu   -x0,y0,a
        mpysu   +y0,x1,b
        mpysu   -x1,y1,b

        mpyuu   +x0,x0,a        ; >=56300
        mpyuu   -y0,y0,a
        mpyuu   +x1,x0,b
        mpyuu   -y1,y0,b
        mpyuu   +x1,x1,a
        mpyuu   -y1,y1,a
        mpyuu   +x0,x1,b
        mpyuu   -y0,y1,b
        mpyuu   +x0,y1,a
        mpyuu   -y0,x0,a
        mpyuu   +x1,y0,b
        mpyuu   -y1,x1,b
        mpyuu   +y1,x0,a
        mpyuu   -x0,y0,a
        mpyuu   +y0,x1,b
        mpyuu   -x1,y1,b

        mpyi    +#$123456,x0,a  ; >=56300
        mpyi    -#$234567,y0,a
        mpyi    +#$345678,x1,b
        mpyi    -#$456789,y1,b

        mpyr    +x0,x0,a        ; >=56000
        mpyr    -y0,y0,a
        mpyr    +x1,x0,b
        mpyr    -y1,y0,b
        mpyr    +x0,y1,a
        mpyr    -y0,x0,a
        mpyr    +x1,y0,b
        mpyr    -y1,x1,b
        mpyr    +y1,#8,a        ; >=56300
        mpyr    -x0,#32,a
        mpyr    +y0,#128,b
        mpyr    -x1,#(1<<22),b

        mpyri   +#$123456,x0,a  ; >=56300
        mpyri   -#$234567,y0,a
        mpyri   +#$345678,x1,b
        mpyri   -#$456789,y1,b

        neg     a               ; >=56000
        neg     b

        nop

        norm    r2,a            ; >=56000
        norm    r4,b

        normf   a1,a            ; >=56300
        normf   b1,a
        normf   x0,a
        normf   y0,a
        normf   x1,a
        normf   y1,a
        normf   a1,b
        normf   b1,b
        normf   x0,b
        normf   y0,b
        normf   x1,b
        normf   y1,b

        not     a               ; >=56000
        not     b

        or      x0,a            ; >=56000
        or      x1,a
        or      y0,a
        or      y1,a
        or      x0,b
        or      x1,b
        or      y0,b
        or      y1,b

        or      #31,a           ; >=56000
        or      #1234,a
        or      #31,b
        or      #1234,b

        ori     #$12,mr         ; >=56000
        ori     #$12,ccr
        ori     #$12,com
        ori     #$12,eom

        pflush                  ; >=56300
        pflushun
        pfree
        plock   $123456
        plockr  *
        punlockr *

        rep     x:(r1)-n1       ; >=56000
        rep     x:(r2)+n2
        rep     x:(r3)-
        rep     x:(r4)+
        rep     x:(r5)
        rep     x:(r6+n6)
        rep     x:-(r7)
        rep     x:$12
        rep     y:(r1)-n1
        rep     y:(r2)+n2
        rep     y:(r3)-
        rep     y:(r4)+
        rep     y:(r5)
        rep     y:(r6+n6)
        rep     y:-(r7)
        rep     y:$12
        rep     r4
        rep     #$234

        reset                   ; >=56000

        rnd     a               ; >=56000
        rnd     b

        rol     a               ; >=56000
        rol     b

        ror     a               ; >=56000
        ror     b

        rti                     ; >=56000
        rts

        sbc     x,a             ; >=56000
        sbc     y,a
        sbc     x,b
        sbc     y,b

        stop                    ; >=56000

        sub     x0,a            ; >=56000
        sub     x1,a
        sub     y0,a
        sub     y1,a
        sub     x,a
        sub     y,a
        sub     b,a
        sub     x0,b            ; >=56000
        sub     x1,b
        sub     y0,b
        sub     y1,b
        sub     x,b
        sub     y,b
        sub     a,b

        sub     #31,a           ; >=56300
        sub     #1234,a
        sub     #31,b
        sub     #1234,b

        subl    b,a             ; >=56000
        subl    a,b

        subr    b,a             ; >=56000
        subr    a,b

        tne     b,a             ; >=56000
        tne     x0,a
        tne     y0,a
        tne     x1,a
        tne     y1,a
        tne     a,b
        tne     x0,b
        tne     y0,b
        tne     x1,b
        tne     y1,b

        tne     r2,r3           ; >=56000

        tne     y1,b r2,r3      ; >=56000

        tfr     b,a             ; >=56000
        tfr     x0,a
        tfr     y0,a
        tfr     x1,a
        tfr     y1,a
        tfr     a,b
        tfr     x0,b
        tfr     y0,b
        tfr     x1,b
        tfr     y1,b

        trap                    ; >=56300

        trapcc                  ; >=56300
        trapge
        trapne
        trappl
        trapnn
        trapec
        traplc
        trapgt
        trapcs
        traplt
        trapeq
        trapmi
        trapnr
        trapes
        trapls
        traple
        traphs
        traplo

        tst     a               ; >=56000
        tst     b

        wait

