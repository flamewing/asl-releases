        cpu 68000
        padding off

; Recursive calls must resume each suspended invocation at its own next line.
walk macro value
        dc.b value
        if value>0
        walk value-1
        endif
        dc.b $80+value
        endm
        walk 2
        walk 1

; Loop input and SHIFT must leave the enclosing macro's cursor untouched.
mixed macro head
        dc.b head
        irp item,$31,$32
        dc.b item
        endm
        shift
        dc.b head
        dc.b $40
        endm
        mixed $11,$12
        mixed $21,$22

; An early exit must not advance the next invocation's initial cursor.
early macro value
        dc.b value
        if value=0
        exitm
        endif
        dc.b $50+value
        endm
        early 0
        early 1

; Empty bodies never dereference a null cursor.
empty macro
        endm
        empty
        dc.b $ff
