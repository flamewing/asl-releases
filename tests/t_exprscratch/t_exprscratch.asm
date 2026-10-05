        cpu 68000
        padding off

; Nested functions and operators must not reuse an active parent's buffers.
combine function left,right,(left*3)+right
        dc.l combine(7,combine(2,5))
        dc.l abs(-17)+(3*(4+5))
        dc.d abs(-42.0)
        dc.b "\{combine(1,2)}"

; Grow scratch storage past its initial capacity, then reuse it for short input.
        include "long.inc"
        dc.b long_value
        dc.b "S",1,2

; Preserve forward-reference flags and source positions across assembly passes.
        dc.w target-*
        dc.b $ee
target:

; The generated expression below keeps forty recursive parent frames active.
        dc.l ((((((((((((((((((((((((((((((((((((((((7))))))))))))))))))))))))))))))))))))))))
        dc.b $ff
