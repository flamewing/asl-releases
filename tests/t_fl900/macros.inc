; MACROS.INC
;******************************************************************************
;* šberall gebrauchte Makros                                                  *
;*                                                                            *
;* Alfred Arnold, Oktober 1993                                                *
;******************************************************************************

proc            macro   name            ; Prozedureintritt
                section name
                public  name
name            label   $
                endm

endp            macro   name            ; Prozeduraustritt
                endsection name
                endm
