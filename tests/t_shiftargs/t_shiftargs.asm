        cpu 68000
        padding off

; A formal name must keep reading excess arguments after SHIFT.
consume macro head
        dc.b ARGCOUNT,head
        shift
        dc.b ARGCOUNT,head
        shift
        dc.b ARGCOUNT,head
        shift
        dc.b ARGCOUNT
        shift
        dc.b ARGCOUNT
        endm
        consume $11,$22,$33

; Count and ALLARGS must agree after shifting a variadic-only macro.
variadic macro
        dc.b ARGCOUNT
        dc.b ALLARGS
        shift
        dc.b ARGCOUNT
        dc.b ALLARGS
        shift
        dc.b ARGCOUNT
        dc.b ALLARGS
        shift
        dc.b ARGCOUNT
        shift
        dc.b ARGCOUNT
        endm
        variadic $41,$42,$43

; Defaults remain usable but do not add to the supplied argument count.
defaults macro head=$51,tail=$52
        dc.b ARGCOUNT,head,tail
        shift
        dc.b ARGCOUNT,head
        shift
        dc.b ARGCOUNT
        shift
        dc.b ARGCOUNT
        endm
        defaults
        defaults tail=$53

; A SHIFT inside an IRP must update the enclosing macro, not the loop count.
nested macro head
        dc.b ARGCOUNT,head
        irp unused,0
        shift
        endm
        dc.b ARGCOUNT,head
        endm
        nested $61,$62

; Recursive forwarding after SHIFT still sees every remaining argument.
recursive macro head
        if ARGCOUNT>0
        dc.b ARGCOUNT,head
        shift
        if ARGCOUNT>0
        recursive ALLARGS
        endif
        endif
        endm
        recursive $71,$72,$73

; Empty remaining arguments must retain their positions in ALLARGS.
holes macro head
        shift
        dc.b ARGCOUNT
        dc.b "<\ALLARGS\>"
        if "\head\"=""
        dc.b $f0
        endif
        shift
        dc.b ARGCOUNT,head
        endm
        holes $11,,$33
