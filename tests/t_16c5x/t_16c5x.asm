        cpu     16c57
        page    0
        relaxed on

        clrw
        nop
        option
        sleep

        andlw   12h
        iorlw   12h
        movlw   12h
        retlw   12h
        xorlw   12h

        addwf   12h
        andwf   12h,0
        comf    12h,1
        decf    12h,w
        decfsz  12h,f
        incf    12h
        incfsz  12h,0
        iorwf   12h,1
        movf    12h,w
        rlf     12h,f
        rrf     12h
        subwf   12h,0
        swapf   12h,1
        xorwf   12h,w

        bcf     10h,3
        bsf     17h,5
        btfsc   12h,7
        btfss   08h,1

        clrf    12h
        movwf   12h

        tris    5
        tris    6
        tris    7

        call    234h
        goto    123h

