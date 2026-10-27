; CID 915444: -1 marks a required immediate, never an encoded default.
        cpu     96c141
        page    0
        supmode on

        expect  1110
        ldf
        endexpect
        ei                      ; default 0
        swi                     ; default 7
        ldf     0
        ldf     7
        ei      7
        swi     0

        maxmode on
        expect  1110
        ldf
        endexpect
        ldf     3
        expect  1320
        ldf     4
        endexpect
