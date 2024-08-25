    cpu 68000
    page 0
scaled function x,int(x*100.0)
offset function x,sqrt(x)+1.0
nested function x,int(offset(x)*10.0)
rate function x,int(32000.0*x)
    dc.w scaled(1.25),scaled(0.97),nested(9.0),rate(1.3)
    dc.b "\{1.25}"
