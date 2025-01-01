    cpu 68000
    page 0
scaled function x,int(x*100.0)
offset function x,sqrt(x)+1.0
nested function x,int(offset(x)*10.0)
rate function x,int(32000.0*x)
    dc.w scaled(1.25),scaled(0.97),nested(9.0),rate(1.3)
    dc.b "\{1.25}"

many function a,b,c,d,e,a+b+c+d+e
    dc.w many(1,2,3,4,5)
    expect 1490
    dc.w many(1,2)
    endexpect
    expect 1491
dupname function x,x,x+x
    endexpect

; Legacy register arguments leave a user function unevaluated. IF takes its
; established fallback branch, rather than evaluating an integer-only path.
isreg function operand,symtype(operand)==8
    if isreg(d2)
    dc.w $1234
    else
    dc.w 1&d2
    endif
