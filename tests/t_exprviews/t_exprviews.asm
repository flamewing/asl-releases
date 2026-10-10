        cpu 68000
        padding off

tri     function a,b,c,a+b*c
many    function a,b,c,d,e,a+b+c+d+e
join    function left,right,left+right

; Nested calls and nonterminated argument slices, including quoted commas.
        dc.l tri(1,2,tri(3,4,5)),many(1,2,3,4,5),tri(12,34,5)
        dc.b join("a,b","c"),join("(","[")

; Borrowed children preserve parentheses, precedence and short-circuit order.
        dc.l ( 1 + (2*3) ),8-3-1,1<<2+1
        dc.l 0&&undefined_name,1||undefined_name

; The parent storage remains live through forward-reference evaluation.
        dc.w target-*
        dc.b $ee
target:

; Reject excess built-in arguments before storing a fourth operand view.
        expect 1490
        dc.l abs(1,2,3,4)
        endexpect
