        cpu     320C40
        page    0

        ; Parallel register fields accept only R0-R7, in either order.
        addf3   *ar4++,r5,r7
        expect  1445
||      stf     r8,*ar5++
        endexpect

        stf     r8,*ar5++
        expect  1445
||      addf3   *ar4++,r5,r7
        endexpect

        addf3   *ar4++,r5,r8
        expect  1445
||      stf     r3,*ar5++
        endexpect

        stf     r3,*ar5++
        expect  1445
||      addf3   *ar4++,r5,r8
        endexpect
