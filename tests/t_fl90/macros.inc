; MACROS.INC
;******************************************************************************
;* öberall gebrauchte Makros                                                  *
;*                                                                            *
;* Alfred Arnold, Oktober 1993                                                *
;******************************************************************************

proc            macro   name,{NoExpand} ; Prozedureintritt
                section name
                forward LocalSize       ; lokal reservierter Speicher auf Stack
LocalSize       eval    0
                public  name
name            label   $
                endm

subproc         macro   name,{NoExpand} ; Prozedureintritt fÅr private Routine
                section name
                forward LocalSize       ; lokal reservierter Speicher auf Stack
LocalSize       eval    0
                public  name:Parent
name            label   $
                endm

endp            macro   name,{NoExpand} ; Prozeduraustritt
LocalSize       eval    0-LocalSize     ; damit man's im Listing lesen kann
                endsection name
                endm

link            macro   reg,count,{NoExpand} ; Stack-Rahmen einrichten
                push    reg             ; alten Basepointer retten
                ld      reg,sp          ; neuen aufbauen
                if      count<>0
                 add    sp,count        ; Platz auf Stack reservieren
                endif
                endm

unlk            macro   reg,{NoExpand}  ; Stack-Rahmen abbauen
                ld      sp,reg          ; Speicherreservierung zurÅcknehmen
                pop     reg             ; alten Basepointer zurÅck
                endm

retd            macro   dist,{NoExpand} ; Return and Deallocate
                if      dist<>0
                 push   hl              ; Arbeitsregister retten
                 ld     hl,(sp+2)       ; RÅcksprungadresse umkopieren
                 ld     (sp+2+dist),hl
                 ld     hl,(sp)         ; Arbeitsregister zurÅck
                 add    sp,2+dist       ; Stack aufrÑumen
                endif
                ret
                endm

DefLocal        macro   Name,Size,{NoExpand} ; eine lokale Variable definieren
LocalSize       eval    LocalSize-Size  ; zÑhlt lok. reservierten Speicher
Name            equ     LocalSize       ; liegt an neuem unteren Ende des Stackrahmens
                endm

