        cpu     8x305

leftobj liv     $12,3,4
left2obj liv	$12,5,6
rightobj riv    $34,5,6
right2obj riv	$34,3,4

	move	r3,ivl		; Register-->Register: ohne Rotation
        add	r11(3),r5	;                      mit Rotation
        and	liv1,r1		; I/O     -->Register: direkt, ohne L„nge
        xor	riv3,6,r4	;                      direkt, mit L„nge
        move	leftobj,r5	;                      symbolisch, ohne L„nge
	add	leftobj,4,r5	;                      symbolisch, mit L„nge (redundant)
        and	r2,liv6		; Register-->I/O       direkt, ohne L„nge
        xor	r3,3,riv4	;                      direkt, mit L„nge
        move	r11,rightobj	;                      symbolisch, ohne L„nge
        add	r11,6,rightobj	;                      symbolisch, mit L„nge (redundant)
	and	liv2,riv4	; Register-->Register: direkt-->direkt, ohne L„nge
        xor	liv2,5,riv4	;                      direkt-->direkt, mit L„nge
	move	riv1,leftobj	;                      direkt-->symbolisch, ohne L„nge
	add	riv1,4,leftobj	;                      direkt-->symbolisch, mit L„nge(redundant)
        and	rightobj,liv5	;                      symbolisch-->direkt, ohne L„nge
        xor	rightobj,6,liv5	;                      symbolisch-->direkt, mit L„nge(redundant)
	move	leftobj,right2obj ;                    symbolisch-->symbolisch, ohne L„ange
        add	leftobj,4,right2obj ;                  symbolisch-->symbolisch, mit L„nge(redundant)

        xec	$55(r5)
        xec	$12(liv3)
        xec	$12(liv3),6
        xec	$12(leftobj)
        xec	$12(leftobj),4

        nzt	r5,*
        nzt	liv3,*
        nzt	liv3,6,*
        nzt	leftobj,*
        nzt	leftobj,4,*

        xmit	$34,r5
        xmit	$12,liv3
        xmit	$12,liv3,6
        xmit	$12,leftobj
        xmit	$12,leftobj,4

        sel	leftobj
        sel	rightobj

        nop

        halt

        xml     2
        xmr     $0f

temp1   riv     @200,7,8
        sel     temp1

