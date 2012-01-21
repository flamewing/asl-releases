        cpu     mpc821
        page    0
        include stddef60

	org	0x1000

	mfimmr	r25
	mtimmr	r25

        bdnzl   0x10
        bdztla  7,0x10
        beq     0x10
        beq     2,0x10
        beq	cr2,0x10

        cmpw    r1,r2
        clrlwi  r1,r2,5

        mtspr	iccsr,r5
        mticcsr r5
        mcrf	cr4,cr5
        mcrf	4,5
	mttbl	r3
	mttbu	r3
	mftb	r3
	mftbl	r3
	mftb	r3,268
	mftbu	r3
	mftb	r3,269
	mttb	r3
	mttbl	r3
	mttbu	r3
	mtspr	284,r3
	mtspr	285,r3
	mttb	284,r3
	mttb	285,r3
