        cpu     68000
        padding off

; Constant lines before, between, and after parameter lines. More than sixteen
; body lines exercise growth of the definition's per-line metadata.
mixed   macro   value

        ; constant comment
        dc.b    $10
        dc.b    $11
        dc.b    $12
        dc.b    $13
        dc.b    $14
        dc.b    $15
        dc.b    $16
        dc.b    $17
        dc.b    value
        dc.b    $18
        dc.b    $19
        dc.b    $1a
        dc.b    $1b
        dc.b    $1c
        dc.b    $1d
        dc.b    $1e
        dc.b    $1f
        dc.b    value+1

        endm
        mixed   $20
        mixed   $22

; No ordinary parameter token: all expansion comes from special parameters.
special macro   args
        dc.b    $30
        dc.ATTRIBUTE ARGCOUNT
        if      ARGCOUNT<>0
        dc.ATTRIBUTE ALLARGS
        endif
        dc.b    $31
        endm
        special.b $32,$33
        special.b

; Internal-label-only replacement must also mark a line as expandable.
labeled macro
__LABEL__ dc.b   $34
        dc.b    $35
        endm
anchor  labeled
        dc.b    anchor

empty   macro
        endm
        empty

; Invocation-local line positions must survive recursive expansion.
recurse macro   count
        dc.b    $40
        if      count>0
        dc.b    count
        recurse count-1
        endif
        dc.b    $41
        endm
        recurse 2

; An outer argument is substituted while a nested macro is being defined.
outer   macro   value
inner   macro   argument
        dc.b    $50
        dc.b    value,argument
        dc.b    $51
        endm
        inner   $53
        endm
        outer   $52

; Definition metadata must be duplicated with a global copy.
        section Example
copied  macro   value,{GLOBAL}
        dc.b    $60
        dc.b    value
        dc.b    $61
        endm
        endsection
        Example_copied $62
