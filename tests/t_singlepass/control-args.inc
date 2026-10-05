; First argument contains token 2 as literal bytes $1e,$01,$03.
; Sequential expansion must substitute the second argument into it.
        fallback "",F
; IRPN must refresh fallback eligibility for each tuple.
        irpn 2,first,second,"",F,"G",H
        dc.b first
        endm
