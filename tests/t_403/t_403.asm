        cpu     ppc403
        page    0
        include stddef60

	org	0x1000


        mtdcr   10,r5
        mtbear  r5
        mfdcr   r5,10
        mfbesr  r5
        wrtee   r10
        wrteei  1

        bdnzl   0x10
        bdztla  7,0x10
        beq     0x10
        beq     2,0x10
        beq	cr2,0x10

        cmpw    r1,r2
        clrlwi  r1,r2,5

        mtspr	pit,r5

        mcrf	cr4,cr5
        mcrf	4,5
