        cpu     68040
        supmode on
        pmmu    on

        move16  (a1)+,(a3)+
        move16  (a4)+,$1234
        move16  $12345,(a5)+
        move16  (a6),$12345
        move16  $1234,(a7)

        cinva   dc
        cinva   ic
        cinva   dc/ic
        cinvl   dc/ic,(a1)
        cinvp   dc/ic,(a2)
        cpusha  dc
        cpusha  ic
        cpusha  ic/dc
        cpushl  dc/ic,(a3)
        cpushp  dc/ic,(a4)

        pflushn (a2)
        pflush  (a3)
        pflushan
        pflusha

        ptestw  (a2)
        ptestr  (a4)

