        cpu     6502undoc

        nop
        nop     #$12
	nop	$12
        nop	$12,x
        nop	$1234
        nop	$1234,x

	jam
        crs
        kil

        slo	$12
        slo	$12,x
        slo	$1234
        slo	$1234,x
        slo	$12,y
        slo	$1234,y
        slo	($12,x)
        slo	($12),y

	anc	#$12

        rla	$12
        rla	$12,x
        rla	$1234
        rla	$1234,x
        rla	$12,y
        rla	$1234,y
        rla	($12,x)
        rla	($12),y

        sre	$12
        sre	$12,x
        sre	$1234
        sre	$1234,x
        sre	$12,y
        sre	$1234,y
        sre	($12,x)
        sre	($12),y

	asr	#$12

        rra	$12
        rra	$12,x
        rra	$1234
        rra	$1234,x
        rra	$12,y
        rra	$1234,y
        rra	($12,x)
        rra	($12),y

	arr	#$12

        sax	$12
        sax	$12,y
        sax	$1234
        sax	($12,x)

        ane	#$12

        sha	$12,x
        sha	$1234,x
        sha	$12,y
        sha	$1234,y

        shs	$12,y
        shs	$1234,y

        shy	$12,y
        shy	$1234,y

        shx	$12,x
        shx	$1234,x

        lax	$12
        lax	$12,y
        lax	$1234
        lax	$1234,y
        lax	($12,x)
        lax	($12),y

	lxa	#$12

        lae	$12,y
        lae	$1234,y

        dcp	$12
        dcp	$12,x
        dcp	$1234
        dcp	$1234,x
        dcp	$12,y
        dcp	$1234,y
        dcp	($12,x)
        dcp	($12),y

        sbx	#$12

        isb	$12
        isb	$12,x
        isb	$1234
        isb	$1234,x
        isb	$12,y
        isb	$1234,y
        isb	($12,x)
        isb	($12),y

        end     *

