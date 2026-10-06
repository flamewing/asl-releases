        page    0
        cpu     68000
        if      INSUPMODE<>1
        fatal   "Command-line supervisor mode default was lost"
        endif
        reset
        cpu     68020
        if      INSUPMODE<>1
        fatal   "Supervisor mode was lost when switching CPUs"
        endif
        reset
        supmode off
        cpu     68000
        if      INSUPMODE<>0
        fatal   "Explicit supervisor mode setting was overwritten"
        endif
        nop
