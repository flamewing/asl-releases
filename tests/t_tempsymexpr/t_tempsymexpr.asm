 cpu 68000
 padding off
 expect 1011
 dc.w -
 endexpect
base:
 dc.w ++-base
+ dc.b 1
+ dc.b 2
- dc.w - - base
