        cpu 68000
        padding off
ival    equ 3
fval    equ 1.5

; Prefixes in binary operands and independent negation precedence.
        dc.l 2*-ival,2+-ival,2--ival,-ival*2
        dc.l ival=-3,ival=4-1,-ival=-3
        dc.l -ival^2,2^-ival

; Prefix chains apply from right to left, including mixed priorities.
        dc.l --ival,---ival,~~ ~~0,~ ~ival
        dc.l -~ival,~-ival,~ ~~0,~~ ~0,-~~0,~~-ival
        dc.l 2*~-ival,2*~~-ival

; Parenthesized operands, calls, floats and unresolved symbols.
        dc.l -(ival+1),2*-abs(-ival),-later,2*-later
        dc.d -fval,2*-fval
later   equ 5

; Short circuit evaluation must still suppress an unevaluated operand.
        dc.l 0&&-undefined_name,1||~-undefined_name

; Preserve the legacy binary precedence and left associativity.
        dc.l 1<<2+1,1+2&4,2^3^2,0|2=2|1
