        cpu 68000
        padding off

; Precedence, associativity and overlapping operator spellings.
        dc.l 1+2*3
        dc.l 20-5-3
        dc.l 64>>2>>1
        dc.l 2^3^2
        dc.l 5&3,5&&3
        dc.l 4|1,4||0
        dc.l 5!3,5!!3,0!!3
        dc.l ~~0,~~7
        dc.l 2=2,2==2,2<>2
        dc.l 2<3,2<=2,3>2,3>=3
        dc.l ~0
        dc.l 1<<2+1
        dc.l 20/2*3

; Operators inside strings must not become expression operators.
        dc.l "a+b"="a+b"
        dc.d 2.5+1.5
        dc.b "x+y"
