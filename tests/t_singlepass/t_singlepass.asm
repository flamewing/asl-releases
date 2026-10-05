        cpu 68000
        padding off

; Values are inserted verbatim, even when they name another parameter.
verbatim macro first,second
        dc.b "\first\\second\"
        endm
        verbatim SECOND,Z

; Named arguments, defaults, empty replacements and adjacent special tokens.
defaults macro LEFT=A,RIGHT=B
        dc.b "\LEFT\\RIGHT\\ARGCOUNT\\ALLARGS\"
        endm
        defaults
        defaults RIGHT=Z

; SHIFT invalidates cached pointers, lengths, counts and fallback eligibility.
shifting macro item,next,last
        dc.b item
        shift
        dc.b item
        dc.b ARGCOUNT
        dc.b ALLARGS
        shift
        dc.b item
        dc.b ARGCOUNT
        endm
        shifting 1,2,3

; A nested loop can SHIFT its enclosing macro's cached replacement table.
shiftloop macro first,second
        dc.b first
        irp ignored,0
        shift
        endm
        dc.b first
        endm
        shiftloop 1,2

; SHIFT counts supplied arguments even when no formal parameters exist.
countless macro
        dc.b ARGCOUNT
        shift
        dc.b ARGCOUNT
        shift
        dc.b ARGCOUNT
        endm
        countless $11,$22

; A token made inactive by SHIFT must remain encoded, not become an empty value.
inactive macro left,right
        shift
        dc.b "\right\"
        endm
        inactive A,B

; Regular IRP, nested loops, empty values, and iteration-local tables.
        irp item,A,,B
        dc.b "\item\"
        irp inner,X,Y
        dc.b "\item\\inner\"
        endm
        endm

; IRPN refreshes all replacements and pads its final tuple with an empty value.
        irpn 3,left,middle,right,A,B,C,D,E
        dc.b "\left\\middle\\right\"
        endm

; Repeated substitution grows the output beyond the invocation's input buffer.
longtext macro value
        dc.b value,value,value,value
        endm
        longtext "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL"
        longtext "S"

; The include supplies literal control bytes to exercise ordered fallback.
fallback macro first,second
        dc.b first
        endm
        include "control-args.inc"

; EXITM must release cached tables without emitting the remaining body.
early macro value
        dc.b value
        exitm
        dc.b $ff
        endm
        early $e1
