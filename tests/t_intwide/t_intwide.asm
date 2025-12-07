 cpu 68000
 padding off
increment function value,value+1
 if intwidth>=128
large equ (1<<100)+$1234
 dc.w increment(large)&$ffff
 dc.b "\{large>>100}"
 else
 dc.w increment($1234)
 dc.b "1"
 endif
