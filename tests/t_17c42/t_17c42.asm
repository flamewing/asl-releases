                cpu     17c42
                relaxed on

                retfie
                return
                clrwdt
                nop
                sleep

                movlb   12h
                addlw   12h
                andlw   12h
                iorlw   12h
                movlw   12h
                sublw   12h
                xorlw   12h
                retlw   12h

                addwf   12h,0
                addwfc  12h,1
                andwf   12h,w
                clrf    12h,f
                comf    12h
                daw     12h,0
                decf    12h,1
                incf    12h,w
                iorwf   12h,f
                negw    12h
                rlcf    12h,0
                rlncf   12h,1
                rrcf    12h,w
                rrncf   12h,f
                setf    12h
                subwf   12h,0
                subwfb  12h,1
                swapf   12h,w
                xorwf   12h,f
                decfsz  12h
                dcfsnz  12h,0
                incfsz  12h,1
                infsnz  12h,w

                bcf     12h,1
                bsf     12h,3
                btfsc   12h,5
                btfss   12h,7
                btg     12h,1

                movwf   12h
                cpfseq  12h
                cpfsgt  12h
                tstfsz  12h

                movfp   34h,12h
                movpf   12h,34h

                tablrd  1,1,12h
                tablwt  1,1,12h

                tlrd    1,12h
                tlwt    1,12h

                call    1234h
                goto    1234h
                lcall   0fedch

