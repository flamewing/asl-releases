; ****************************************************************************
; EQU - Anweisungen fuer alle Module
; ****************************************************************************

        ifndef defgequ_inc
defgequ_inc             equ 000H

; ----------------------------------------------------------------------------
StringEnde              equ 000H
TableEnd                equ 0FFH
TableAnf                equ 0FEH
Rob_NextMsg             set 001H
Frs_NextMsg             set 001H
TeachRob_NextMsg        set 001H
TeachFrs_NextMsg        set 001H
MemRob_NextMsg          set 001H
MemFrs_NextMsg          set 001H
NextModul               set 010H        ; Modul < 10H : = Message from PC!!
                                        ; Doku beachten.
; ----------------------------------------------------------------------------
; Definition der Netzwerkadressen
;
uC_Rob          equ       001H    ; Microcontroller des Roboters
uC_Frs          equ       002H    ; Microcontroller der Fraese
uC_Teach_Rob    equ       003H    ; Microcontroller Teach-In Roboter
uC_Teach_Frs    equ       004H    ; Microcontroller Teach-In Fraese
uC_Teach_Key    equ       005H    ; Microcontroller Teach-In Tastatur

uC_PC_SIO       equ       00EH    ; Microcontroller PC

; ----------------------------------------------------------------------------

          elseif
            fatal "Die Datei DEFGEQU.INC ist mehrmals eingebunden"
          endif

; ----------------------------------------------------------------------------

NoParamSingle  equ 000H
ParamSingle1   equ 001H
ParamSingle2   equ 002H
ParamSingle3   equ 004H
ParamSingle4   equ 008H
ParamSingle5   equ 010H
ParamSingle6   equ 020H

;
; ****************************************************************************
