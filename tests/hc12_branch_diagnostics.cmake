# SPDX-License-Identifier: GPL-2.0-only
cmake_minimum_required(VERSION 3.19)

file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_diagnostic name cpu body id detail line column)
    set(expected_result 2)
    set(kind error)
    if(ARGC GREATER 7)
        set(expected_result "${ARGV7}")
        set(kind warning)
    endif()
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 0\n${body}\n")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    set(diagnostic "${output}${error}")
    string(FIND "${diagnostic}" "${detail}" detail_pos)
    if(column STREQUAL "")
        set(location "(${line}): ${kind} #${id}:")
    else()
        set(location "(${line}):${column}: ${kind} #${id}:")
    endif()
    string(FIND "${diagnostic}" "${location}" location_pos)
    string(REGEX MATCHALL "${kind} #[0-9]+:" errors "${diagnostic}")
    list(LENGTH errors error_count)
    if(NOT "${result}" STREQUAL "${expected_result}" OR detail_pos EQUAL -1
            OR location_pos EQUAL -1 OR NOT error_count EQUAL 1)
        message(FATAL_ERROR "${name}: expected one #${id} at ${line}:${column}, ${detail}; got ${result}\n${diagnostic}")
    endif()
endfunction()

check_diagnostic(short-high 68HC12 " org $1000\n bra *+130" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(short-low 68HC12X " org $1000\n bra *-127" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(loop-high 68HC12 " org $1000\n dbne x,*+259" 1370
    "branch displacement 256 outside [-256,255] (PC base 0x1003)" 4 9)
check_diagnostic(loop-low 68HC12 " org $1000\n tbne d,*-254" 1370
    "branch displacement -257 outside [-256,255] (PC base 0x1003)" 4 9)
check_diagnostic(bit-direct 68HC12 " org $1000\n brset $20,#$40,*+132" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 17)
check_diagnostic(bit-extended 68HC12 " org $1000\n brclr $2000,#$40,*-124" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1005)" 4 19)
check_diagnostic(bit-indexed-short 68HC12 " org $1000\n brset 0,x,#$40,*+132" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 17)
check_diagnostic(bit-indexed-byte 68HC12 " org $1000\n brclr 16,x,#$40,*+133" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1005)" 4 18)
check_diagnostic(bit-indexed-word 68HC12 " org $1000\n brset 1000,x,#$40,*+134" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1006)" 4 20)
check_diagnostic(bit-pc 68HC12 " org $1000\n brset *,pc,#$40,*+132" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 18)
check_diagnostic(no-short-wrap 68HC12 " org $fffe\n bra 0" 1370
    "branch displacement -65536 outside [-128,127] (PC base 0x10000)" 4 6)
check_diagnostic(wide-target 68HC12X " org $1000\n bra $10000" 1370
    "branch displacement 61438 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(forward-range 68HC12 " org $1000\n bra target\ntarget equ $1082" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(undefined 68HC12 " bra missing" 1010 "symbol undefined" 3 6)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/loop-high.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "256" value_pos)
string(FIND "${output}${error}" "[-256,255]" range_pos)
string(FIND "${output}${error}" "PC-Basis 0x1003" base_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German HC12: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

function(check_bytes name cpu body expected)
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 0\n${body}\n")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -q ${ARGN}
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: assembly ${result}\n${output}${error}")
    endif()
    set(assembly_diagnostic "${output}${error}" PARENT_SCOPE)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.p" object)
    execute_process(COMMAND ${TEST_EMULATOR} "${P2BIN_PROGRAM}" "${object}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${result}\n${output}${error}")
    endif()
    file(READ "${TEST_DIR}/${name}.bin" actual HEX)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: expected ${expected}, got ${actual}")
    endif()
endfunction()

# Golden encodings captured before this batch.
check_bytes(short-and-long 68HC12 " org $1000\n bra *+129\n bra *-126\n lbra *+40000"
    "207f208018209c3c")
check_bytes(loop-endpoints 68HC12 " org $1000\n dbne x,*+258\n tbne d,*-253" "0425ff047400")
check_bytes(bit-modes 68HC12
    " org $1000\n brset $20,#$40,*+131\n brclr $2000,#$40,*-123\n brset 0,x,#$40,*+131\n brclr 16,x,#$40,*-123\n brset 1000,x,#$40,*+133"
    "4e20407f1f200040800e00407f0fe01040800ee203e8407f")
check_bytes(x-branches 68HC12X " org $1000\n bra *+129\n dbne x,*-253\n lbra *+40000"
    "207f04350018209c3c")
check_bytes(forward-valid 68HC12 " org $1000\n bra target\ntarget equ $1004" "2002")
check_bytes(long-wrap 68HC12 " org $fffc\n lbra 0" "18200000")


# All short/loop/bit opcodes defer provisional ranges until forward resolution.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    set(body " org $1000")
    set(definitions "")
    set(bytes "")
    set(pc 4096)
    set(serial 0)
    foreach(item IN ITEMS "bgt|2e" "bge|2c" "beq|27" "ble|2f" "blt|2d"
            "bhi|22" "bhs|24" "bcc|24" "bne|26" "bls|23" "blo|25" "bcs|25"
            "bmi|2b" "bvs|29" "bra|20" "bpl|2a" "brn|21" "bvc|28" "bsr|07"
            "dbeq x,|0405" "dbne x,|0425" "ibeq x,|0485" "ibne x,|04a5"
            "tbeq x,|0445" "tbne x,|0465")
        string(REPLACE "|" ";" fields "${item}")
        list(GET fields 0 instruction)
        list(GET fields 1 prefix)
        if(instruction MATCHES ",$")
            set(separator "")
            set(length 3)
        else()
            set(separator " ")
            set(length 2)
        endif()
        foreach(offset IN ITEMS 1000 100000 -1000 -100000)
            math(EXPR serial "${serial}+1")
            if(offset LESS 0)
                set(sign "-")
                math(EXPR magnitude "-${offset}")
            else()
                set(sign "+")
                set(magnitude ${offset})
            endif()
            string(APPEND body "\n ${instruction}${separator}target${serial}${sign}${magnitude}")
            math(EXPR target "${pc}+${length}-${offset}")
            string(APPEND definitions "\ntarget${serial} equ ${target}")
            string(APPEND bytes "${prefix}00")
            math(EXPR pc "${pc}+${length}")
        endforeach()
    endforeach()
    foreach(item IN ITEMS "$20,#$40|4e2040|4" "$2000,#$40|1e200040|5"
            "0,x,#$40|0e0040|4" "16,x,#$40|0ee01040|5" "1000,x,#$40|0ee203e840|6")
        string(REPLACE "|" ";" fields "${item}")
        list(GET fields 0 operands)
        list(GET fields 1 prefix)
        list(GET fields 2 length)
        foreach(mnemonic IN ITEMS brset brclr)
            set(instruction_prefix "${prefix}")
            if(mnemonic STREQUAL "brclr")
                string(SUBSTRING "${prefix}" 0 2 opcode)
                string(SUBSTRING "${prefix}" 2 -1 operand_bytes)
                if(opcode STREQUAL "4e")
                    set(opcode "4f")
                elseif(opcode STREQUAL "1e")
                    set(opcode "1f")
                else()
                    set(opcode "0f")
                endif()
                set(instruction_prefix "${opcode}${operand_bytes}")
            endif()
        foreach(offset IN ITEMS 1000 100000 -1000 -100000)
            math(EXPR serial "${serial}+1")
            if(offset LESS 0)
                set(sign "-")
                math(EXPR magnitude "-${offset}")
            else()
                set(sign "+")
                set(magnitude ${offset})
            endif()
            string(APPEND body "\n ${mnemonic} ${operands},target${serial}${sign}${magnitude}")
            math(EXPR target "${pc}+${length}-${offset}")
            string(APPEND definitions "\ntarget${serial} equ ${target}")
            string(APPEND bytes "${instruction_prefix}00")
            math(EXPR pc "${pc}+${length}")
        endforeach()
        endforeach()
    endforeach()
    check_bytes(forward-all-${cpu} ${cpu} "${body}${definitions}" "${bytes}")
endforeach()

# Final range failure must not leave a partial bit-branch instruction behind.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    foreach(item IN ITEMS "$20,#$40|4" "$2000,#$40|5" "0,x,#$40|4"
            "16,x,#$40|5" "1000,x,#$40|6" "*,pc,#$40|4")
        string(REPLACE "|" ";" fields "${item}")
        list(GET fields 0 operands)
        list(GET fields 1 length)
        string(MAKE_C_IDENTIFIER "${operands}" tag)
        math(EXPR high "${length}+128")
        math(EXPR low "129-${length}")
        foreach(mnemonic IN ITEMS brset brclr)
            check_bytes(noemit-${cpu}-${mnemonic}-${tag} ${cpu}
                " org $1000\n expect 1370,1370\n ${mnemonic} ${operands},*+${high}\n ${mnemonic} ${operands},*-${low}\n endexpect\n db $12" "12")
        endforeach()
    endforeach()
endforeach()

# Address forms, base registers and bracket failures identify the exact case.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    check_diagnostic(base-register-${cpu} ${cpu} " ldaa 1,a" 1445 "X, Y, SP or PC" 3 9)
    check_diagnostic(empty-base-${cpu} ${cpu} " ldaa 1," 1445 "X, Y, SP or PC" 3 9)
    check_diagnostic(auto-register-${cpu} ${cpu} " ldaa 1,+a" 1445 "X, Y or SP (auto-update excludes PC)" 3 9)
    check_diagnostic(auto-pc-${cpu} ${cpu} " ldaa 1,pc+" 1445 "auto-update excludes PC" 3 9)
    check_diagnostic(indirect-base-${cpu} ${cpu} " ldaa [12,a]" 1445 "X, Y, SP or PC" 3 11)
    check_diagnostic(indirect-expression-${cpu} ${cpu} " ldaa [missing,x]" 1010 "symbol undefined" 3 8)
    check_diagnostic(indirect-form-${cpu} ${cpu} " ldaa [12]" 1350
        "[D,X/Y/SP/PC] or [displacement,X/Y/SP/PC]" 3 7)
    check_diagnostic(store-immediate-${cpu} ${cpu} " staa #1" 1350 "extended address" 3 7)
    check_diagnostic(unclosed-address-${cpu} ${cpu} " ldaa [12,x" 1300
        "unclosed bracket '['; expected ']' before end of input" 3 7)
    check_diagnostic(mismatch-address-${cpu} ${cpu} " ldaa [12,x)" 1300
        "mismatched closing bracket ')'; expected ']'" 3 12)
    check_bytes(original-address-${cpu} ${cpu}
        " ldaa [d,x]\n ldaa [12,y]\n ldaa 1,+x\n ldaa #1\n ldaa 1,pc" "a6e7a6eb000ca6208601a6d5")
    check_bytes(address-noemit-${cpu} ${cpu}
        " expect 1445,1445,1350,1300\n ldaa 1,\n ldaa [12,a]\n staa #1\n ldaa [12,x)\n endexpect\n db $12" "12")
endforeach()

foreach(cpu IN ITEMS 68HC12 68HC12X)
    check_bytes(quoted-brackets-${cpu} ${cpu} " ldaa ['[',x]\n ldaa [']',x]" "a6e3005ba6e3005d")
endforeach()

# TRAP's accepted domain has two intervals; auto-update has its own bound.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    foreach(value IN ITEMS 0 47 58 63)
        check_diagnostic(trap-${value}-${cpu} ${cpu} " trap #${value}" 1320
            "TRAP code ${value} outside [48,57] or [64,255]" 3 7)
    endforeach()
    check_diagnostic(trap-final-forward-${cpu} ${cpu} " trap #code\ncode equ 58" 1320
        "TRAP code 58 outside [48,57] or [64,255]" 3 7)
    check_diagnostic(auto-count-high-${cpu} ${cpu} " ldaa 9,x+" 1320 "auto-update count 9 outside [-8,8]" 3 7)
    check_diagnostic(auto-count-low-${cpu} ${cpu} " ldaa -9,-x" 1315 "auto-update count -9 outside [-8,8]" 3 7)
    check_bytes(trap-auto-boundaries-${cpu} ${cpu}
        " trap #48\n trap 57\n trap #64\n trap 255\n ldaa -8,x+\n ldaa 0,x+\n ldaa 8,x+"
        "18301839184018ffa638a600a637")
    check_bytes(trap-forward-${cpu} ${cpu}
        " trap #code+58\ncode equ -10\n ldaa count+9,x+\ncount equ -1" "1830a637")
    check_bytes(trap-errors-noemit-${cpu} ${cpu}
        " expect 1320,1320\n trap #47\n trap #58\n endexpect\n db $12" "12")
endforeach()

# BTAS uses BSET memory modes, never an immediate address operand.
check_diagnostic(btas-immediate-address 68HC12X " btas #1,#$40" 1350 "word offset,X/Y/SP/PC" 3 7)
check_diagnostic(btas-indirect-address 68HC12X " btas [d,x],#$40" 1350 "extended address" 3 7)
check_bytes(btas-word-offset 68HC12X
    " btas 1000,x,#$40\n btas -1000,y,#$40\n btas >1,sp,#$40"
    "1837e203e8401837eafc18401837f2000140")
check_bytes(btas-mask-errors-noemit 68HC12X
    " expect 1350,1350\n btas #1,#$40\n btas [d,x],#$40\n endexpect\n db $12" "12")

# BTAS PC-relative addressing includes both opcode bytes and the mask.
check_bytes(btas-pc-widths 68HC12X
    " org $1000\n btas *,pc,#$40\n btas *+19,pc,#$40\n btas *-12,pc,#$40\n btas *+20,pc,#$40\n btas *-13,pc,#$40\n btas *+260,pc,#$40\n btas *-251,pc,#$40\n btas *+261,pc,#$40\n btas *-252,pc,#$40\n btas <*,pc,#$40\n btas >*,pc,#$40"
    "1837dc401837cf401837d0401837f80f401837f9ee401837f8ff401837f900401837fa00ff401837fafefe401837f9fb401837fafffa40")
check_bytes(btas-pc-forward 68HC12X
    " org $1000\n btas target+1000,pc,#$40\ntarget equ $1004-1000" "1837c040")
check_bytes(btas-pc-after-label 68HC12X
    " org $1000\n btas after,pc,#$40\nafter: db $12" "1837c04012")

# Loop registers exclude CCR, PC and register halves. Transfers and SEX use
# CPU-specific names, with SEX's byte source and word destination constraints.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    foreach(reg IN ITEMS missing ccr pc xl)
        check_diagnostic(loop-register-${reg}-${cpu} ${cpu} " dbne ${reg},*" 1445
            "operand must be A, B, D, X, Y or SP" 3 7)
    endforeach()
    set(all_registers "A, B, CCR, CCRL, D, X, Y, SP, XL, YL, SPL")
    set(byte_registers "A, B, CCR, CCRL, XL, YL, SPL")
    set(word_registers "D, X, Y, SP")
    if(cpu STREQUAL "68HC12X")
        set(all_registers "A, B, CCR, CCRL, D, X, Y, SP, XL, XH, YL, YH, SPL, SPH, CCRH, CCRW")
        set(byte_registers "A, B, CCR, CCRL, XL, XH, YL, YH, SPL, SPH, CCRH")
        set(word_registers "D, X, Y, SP, CCRW")
    endif()
    foreach(ins IN ITEMS tfr exg)
        foreach(reg IN ITEMS missing pc)
            check_diagnostic(${ins}-source-${reg}-${cpu} ${cpu} " ${ins} ${reg},d" 1445
                "operand must be ${all_registers}" 3 6)
            check_diagnostic(${ins}-destination-${reg}-${cpu} ${cpu} " ${ins} a,${reg}" 1445
                "operand must be ${all_registers}" 3 8)
        endforeach()
    endforeach()
    foreach(reg IN ITEMS missing a pc)
        check_diagnostic(sex-destination-${reg}-${cpu} ${cpu} " sex a,${reg}" 1445
            "operand must be ${word_registers}" 3 8)
    endforeach()
    foreach(reg IN ITEMS missing d pc)
        check_diagnostic(sex-source-${reg}-${cpu} ${cpu} " sex ${reg},d" 1445
            "operand must be ${byte_registers}" 3 6)
    endforeach()
    check_bytes(register-errors-noemit-${cpu} ${cpu}
        " expect 1445,1445,1445,1445\n dbne pc,*\n tfr pc,d\n exg a,pc\n sex d,x\n endexpect\n db $12" "12")
    if(cpu STREQUAL "68HC12")
        check_bytes(accepted-registers-${cpu} ${cpu}
            " dbeq a,*\n dbeq b,*\n dbeq d,*\n dbeq x,*\n dbeq y,*\n dbeq sp,*\n dbne a,*\n dbne b,*\n dbne d,*\n dbne x,*\n dbne y,*\n dbne sp,*\n ibeq a,*\n ibeq b,*\n ibeq d,*\n ibeq x,*\n ibeq y,*\n ibeq sp,*\n ibne a,*\n ibne b,*\n ibne d,*\n ibne x,*\n ibne y,*\n ibne sp,*\n tbeq a,*\n tbeq b,*\n tbeq d,*\n tbeq x,*\n tbeq y,*\n tbeq sp,*\n tbne a,*\n tbne b,*\n tbne d,*\n tbne x,*\n tbne y,*\n tbne sp,*\n tfr a,b\n tfr d,x\n tfr xl,a\n tfr b,sp\n exg a,b\n exg d,x\n exg xl,a\n exg b,sp\n sex a,d\n sex b,x\n sex ccr,y"
            "0410fd0411fd0414fd0415fd0416fd0417fd0430fd0431fd0434fd0435fd0436fd0437fd0490fd0491fd0494fd0495fd0496fd0497fd04b0fd04b1fd04b4fd04b5fd04b6fd04b7fd0450fd0451fd0454fd0455fd0456fd0457fd0470fd0471fd0474fd0475fd0476fd0477fdb701b745b750b717b781b7c5b7d0b797b704b715b726")
    endif()
    if(cpu STREQUAL "68HC12X")
        check_bytes(accepted-registers-${cpu} ${cpu}
            " dbeq a,*\n dbeq b,*\n dbeq d,*\n dbeq x,*\n dbeq y,*\n dbeq sp,*\n dbne a,*\n dbne b,*\n dbne d,*\n dbne x,*\n dbne y,*\n dbne sp,*\n ibeq a,*\n ibeq b,*\n ibeq d,*\n ibeq x,*\n ibeq y,*\n ibeq sp,*\n ibne a,*\n ibne b,*\n ibne d,*\n ibne x,*\n ibne y,*\n ibne sp,*\n tbeq a,*\n tbeq b,*\n tbeq d,*\n tbeq x,*\n tbeq y,*\n tbeq sp,*\n tbne a,*\n tbne b,*\n tbne d,*\n tbne x,*\n tbne y,*\n tbne sp,*\n tfr a,b\n tfr d,x\n tfr xl,a\n tfr b,sp\n exg a,b\n exg d,x\n exg xl,a\n exg b,sp\n sex a,d\n sex b,x\n sex ccr,y\n tfr a,xh\n exg b,xl\n tfr ccrw,sp\n sex a,sp"
            "0410fd0411fd0414fd0415fd0416fd0417fd0430fd0431fd0434fd0435fd0436fd0437fd0490fd0491fd0494fd0495fd0496fd0497fd04b0fd04b1fd04b4fd04b5fd04b6fd04b7fd0450fd0451fd0454fd0455fd0456fd0457fd0470fd0471fd0474fd0475fd0476fd0477fdb701b745b750b717b781b7c5b7d0b797b704b715b726b70db79db72fb707")
    endif()
endforeach()
check_diagnostic(unavailable-register 68HC12 " tfr xh,a" 1445
    "operand must be A, B, CCR, CCRL, D, X, Y, SP, XL, YL, SPL" 3 6)

# Rejected register pairs identify both operands and source-specific partners.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    check_diagnostic(pair-no-partner-${cpu} ${cpu} " tfr ccr,xl" 1760
        "register pair ccr,xl not permitted; allowed destinations for ccr: A, B, CCR, CCRL, D, X, Y, SP" 3 10)
    check_diagnostic(exchange-pair-${cpu} ${cpu} " exg xl,d" 1760
        "register pair xl,d not permitted; allowed destinations for xl: A, B, CCR, CCRL" 3 9)
    check_diagnostic(sex-pair-${cpu} ${cpu} " sex xl,d" 1760
        "register pair xl,d not permitted; xl has no valid destination register for this instruction" 3 9)
    check_bytes(pair-errors-noemit-${cpu} ${cpu}
        " expect 1760,1760,1760\n tfr ccr,xl\n exg xl,d\n sex xl,d\n endexpect\n db $12" "12")
    if(cpu STREQUAL "68HC12")
        check_bytes(all-accepted-pairs-${cpu} ${cpu}
            " tfr A,A\n tfr A,B\n tfr A,CCR\n tfr A,CCRL\n tfr A,D\n tfr A,X\n tfr A,Y\n tfr A,SP\n tfr B,A\n tfr B,B\n tfr B,CCR\n tfr B,CCRL\n tfr B,D\n tfr B,X\n tfr B,Y\n tfr B,SP\n tfr CCR,A\n tfr CCR,B\n tfr CCR,CCR\n tfr CCR,CCRL\n tfr CCR,D\n tfr CCR,X\n tfr CCR,Y\n tfr CCR,SP\n tfr CCRL,A\n tfr CCRL,B\n tfr CCRL,CCR\n tfr CCRL,CCRL\n tfr CCRL,D\n tfr CCRL,X\n tfr CCRL,Y\n tfr CCRL,SP\n tfr D,A\n tfr D,B\n tfr D,CCR\n tfr D,CCRL\n tfr D,D\n tfr D,X\n tfr D,Y\n tfr D,SP\n tfr X,A\n tfr X,B\n tfr X,CCR\n tfr X,CCRL\n tfr X,D\n tfr X,X\n tfr X,Y\n tfr X,SP\n tfr Y,A\n tfr Y,B\n tfr Y,CCR\n tfr Y,CCRL\n tfr Y,D\n tfr Y,X\n tfr Y,Y\n tfr Y,SP\n tfr SP,A\n tfr SP,B\n tfr SP,CCR\n tfr SP,CCRL\n tfr SP,D\n tfr SP,X\n tfr SP,Y\n tfr SP,SP\n tfr XL,A\n tfr XL,B\n tfr XL,CCR\n tfr XL,CCRL\n tfr YL,A\n tfr YL,B\n tfr YL,CCR\n tfr YL,CCRL\n tfr SPL,A\n tfr SPL,B\n tfr SPL,CCR\n tfr SPL,CCRL\n exg A,A\n exg A,B\n exg A,CCR\n exg A,CCRL\n exg A,D\n exg A,X\n exg A,Y\n exg A,SP\n exg B,A\n exg B,B\n exg B,CCR\n exg B,CCRL\n exg B,D\n exg B,X\n exg B,Y\n exg B,SP\n exg CCR,A\n exg CCR,B\n exg CCR,CCR\n exg CCR,CCRL\n exg CCR,D\n exg CCR,X\n exg CCR,Y\n exg CCR,SP\n exg CCRL,A\n exg CCRL,B\n exg CCRL,CCR\n exg CCRL,CCRL\n exg CCRL,D\n exg CCRL,X\n exg CCRL,Y\n exg CCRL,SP\n exg D,A\n exg D,B\n exg D,CCR\n exg D,CCRL\n exg D,D\n exg D,X\n exg D,Y\n exg D,SP\n exg X,A\n exg X,B\n exg X,CCR\n exg X,CCRL\n exg X,D\n exg X,X\n exg X,Y\n exg X,SP\n exg Y,A\n exg Y,B\n exg Y,CCR\n exg Y,CCRL\n exg Y,D\n exg Y,X\n exg Y,Y\n exg Y,SP\n exg SP,A\n exg SP,B\n exg SP,CCR\n exg SP,CCRL\n exg SP,D\n exg SP,X\n exg SP,Y\n exg SP,SP\n exg XL,A\n exg XL,B\n exg XL,CCR\n exg XL,CCRL\n exg YL,A\n exg YL,B\n exg YL,CCR\n exg YL,CCRL\n exg SPL,A\n exg SPL,B\n exg SPL,CCR\n exg SPL,CCRL\n sex A,D\n sex A,X\n sex A,Y\n sex A,SP\n sex B,D\n sex B,X\n sex B,Y\n sex B,SP\n sex CCR,D\n sex CCR,X\n sex CCR,Y\n sex CCR,SP\n sex CCRL,D\n sex CCRL,X\n sex CCRL,Y\n sex CCRL,SP"
            "b700b701b702b702b704b705b706b707b710b711b712b712b714b715b716b717b720b721b722b722b724b725b726b727b720b721b722b722b724b725b726b727b740b741b742b742b744b745b746b747b750b751b752b752b754b755b756b757b760b761b762b762b764b765b766b767b770b771b772b772b774b775b776b777b750b751b752b752b760b761b762b762b770b771b772b772b780b781b782b782b784b785b786b787b790b791b792b792b794b795b796b797b7a0b7a1b7a2b7a2b7a4b7a5b7a6b7a7b7a0b7a1b7a2b7a2b7a4b7a5b7a6b7a7b7c0b7c1b7c2b7c2b7c4b7c5b7c6b7c7b7d0b7d1b7d2b7d2b7d4b7d5b7d6b7d7b7e0b7e1b7e2b7e2b7e4b7e5b7e6b7e7b7f0b7f1b7f2b7f2b7f4b7f5b7f6b7f7b7d0b7d1b7d2b7d2b7e0b7e1b7e2b7e2b7f0b7f1b7f2b7f2b704b705b706b707b714b715b716b717b724b725b726b727b724b725b726b727")
    endif()
    if(cpu STREQUAL "68HC12X")
        check_bytes(all-accepted-pairs-${cpu} ${cpu}
            " tfr A,A\n tfr A,B\n tfr A,CCR\n tfr A,CCRL\n tfr A,D\n tfr A,X\n tfr A,Y\n tfr A,SP\n tfr A,XH\n tfr A,YH\n tfr A,SPH\n tfr A,CCRH\n tfr B,A\n tfr B,B\n tfr B,CCR\n tfr B,CCRL\n tfr B,D\n tfr B,X\n tfr B,Y\n tfr B,SP\n tfr B,XL\n tfr B,YL\n tfr B,SPL\n tfr CCR,A\n tfr CCR,B\n tfr CCR,CCR\n tfr CCR,CCRL\n tfr CCR,D\n tfr CCR,X\n tfr CCR,Y\n tfr CCR,SP\n tfr CCRL,A\n tfr CCRL,B\n tfr CCRL,CCR\n tfr CCRL,CCRL\n tfr CCRL,D\n tfr CCRL,X\n tfr CCRL,Y\n tfr CCRL,SP\n tfr D,A\n tfr D,B\n tfr D,CCR\n tfr D,CCRL\n tfr D,D\n tfr D,X\n tfr D,Y\n tfr D,SP\n tfr D,CCRW\n tfr X,A\n tfr X,B\n tfr X,CCR\n tfr X,CCRL\n tfr X,D\n tfr X,X\n tfr X,Y\n tfr X,SP\n tfr X,CCRW\n tfr Y,A\n tfr Y,B\n tfr Y,CCR\n tfr Y,CCRL\n tfr Y,D\n tfr Y,X\n tfr Y,Y\n tfr Y,SP\n tfr Y,CCRW\n tfr SP,A\n tfr SP,B\n tfr SP,CCR\n tfr SP,CCRL\n tfr SP,D\n tfr SP,X\n tfr SP,Y\n tfr SP,SP\n tfr SP,CCRW\n tfr XL,A\n tfr XL,B\n tfr XL,CCR\n tfr XL,CCRL\n tfr XH,A\n tfr YL,A\n tfr YL,B\n tfr YL,CCR\n tfr YL,CCRL\n tfr YH,A\n tfr SPL,A\n tfr SPL,B\n tfr SPL,CCR\n tfr SPL,CCRL\n tfr SPH,A\n tfr CCRH,A\n tfr CCRW,D\n tfr CCRW,X\n tfr CCRW,Y\n tfr CCRW,SP\n tfr CCRW,CCRW\n exg A,A\n exg A,B\n exg A,CCR\n exg A,CCRL\n exg A,D\n exg A,X\n exg A,Y\n exg A,SP\n exg A,XH\n exg A,YH\n exg A,SPH\n exg A,CCRH\n exg B,A\n exg B,B\n exg B,CCR\n exg B,CCRL\n exg B,D\n exg B,X\n exg B,Y\n exg B,SP\n exg B,XL\n exg B,YL\n exg B,SPL\n exg CCR,A\n exg CCR,B\n exg CCR,CCR\n exg CCR,CCRL\n exg CCR,D\n exg CCR,X\n exg CCR,Y\n exg CCR,SP\n exg CCRL,A\n exg CCRL,B\n exg CCRL,CCR\n exg CCRL,CCRL\n exg CCRL,D\n exg CCRL,X\n exg CCRL,Y\n exg CCRL,SP\n exg D,A\n exg D,B\n exg D,CCR\n exg D,CCRL\n exg D,D\n exg D,X\n exg D,Y\n exg D,SP\n exg D,CCRW\n exg X,A\n exg X,B\n exg X,CCR\n exg X,CCRL\n exg X,D\n exg X,X\n exg X,Y\n exg X,SP\n exg X,CCRW\n exg Y,A\n exg Y,B\n exg Y,CCR\n exg Y,CCRL\n exg Y,D\n exg Y,X\n exg Y,Y\n exg Y,SP\n exg Y,CCRW\n exg SP,A\n exg SP,B\n exg SP,CCR\n exg SP,CCRL\n exg SP,D\n exg SP,X\n exg SP,Y\n exg SP,SP\n exg SP,CCRW\n exg XL,A\n exg XL,B\n exg XL,CCR\n exg XL,CCRL\n exg XH,A\n exg YL,A\n exg YL,B\n exg YL,CCR\n exg YL,CCRL\n exg YH,A\n exg SPL,A\n exg SPL,B\n exg SPL,CCR\n exg SPL,CCRL\n exg SPH,A\n exg CCRH,A\n exg CCRW,D\n exg CCRW,X\n exg CCRW,Y\n exg CCRW,SP\n exg CCRW,CCRW\n sex A,D\n sex A,X\n sex A,Y\n sex A,SP\n sex B,D\n sex B,X\n sex B,Y\n sex B,SP\n sex CCR,D\n sex CCR,X\n sex CCR,Y\n sex CCR,SP\n sex CCRL,D\n sex CCRL,X\n sex CCRL,Y\n sex CCRL,SP"
            "b700b701b702b702b704b705b706b707b70db70eb70fb70ab710b711b712b712b714b715b716b717b71db71eb71fb720b721b722b722b724b725b726b727b720b721b722b722b724b725b726b727b740b741b742b742b744b745b746b747b74ab750b751b752b752b754b755b756b757b75ab760b761b762b762b764b765b766b767b76ab770b771b772b772b774b775b776b777b77ab750b751b752b752b758b760b761b762b762b768b770b771b772b772b778b728b72cb72db72eb72fb72ab780b781b782b782b784b785b786b787b78db78eb78fb78ab790b791b792b792b794b795b796b797b79db79eb79fb7a0b7a1b7a2b7a2b7a4b7a5b7a6b7a7b7a0b7a1b7a2b7a2b7a4b7a5b7a6b7a7b7c0b7c1b7c2b7c2b7c4b7c5b7c6b7c7b7cab7d0b7d1b7d2b7d2b7d4b7d5b7d6b7d7b7dab7e0b7e1b7e2b7e2b7e4b7e5b7e6b7e7b7eab7f0b7f1b7f2b7f2b7f4b7f5b7f6b7f7b7fab7d0b7d1b7d2b7d2b7d8b7e0b7e1b7e2b7e2b7e8b7f0b7f1b7f2b7f2b7f8b7a8b7acb7adb7aeb7afb7aab704b705b706b707b714b715b716b717b724b725b726b727b724b725b726b727")
    endif()
endforeach()
check_diagnostic(base-no-extension-pair 68HC12 " tfr a,xl" 1760
    "allowed destinations for a: A, B, CCR, CCRL, D, X, Y, SP" 3 8)
check_diagnostic(extended-other-half-pair 68HC12X " tfr a,xl" 1760
    "allowed destinations for a: A, B, CCR, CCRL, D, X, Y, SP, XH, YH, SPH, CCRH" 3 8)
check_diagnostic(extended-word-pair 68HC12X " tfr ccrw,a" 1760
    "allowed destinations for ccrw: D, X, Y, SP, CCRW" 3 11)
check_diagnostic(extended-sex-pair 68HC12X " sex xh,d" 1760
    "xh has no valid destination register for this instruction" 3 9)
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/pair-no-partner-68HC12.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Registerpaar ccr,xl ist nicht erlaubt; erlaubte Zielregister" pair_pos)
if(NOT "${result}" STREQUAL "2" OR pair_pos EQUAL -1)
    message(FATAL_ERROR "German HC12 pair: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# TBL/ETBL defer provisional five-bit values; settled failures give the value
# and range, including the PC base when relevant. Forced wider modes stay invalid.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    set(forward_body " org $1000")
    set(forward_expected "")
    foreach(ins IN ITEMS tbl etbl)
        if(ins STREQUAL "tbl")
            set(opcode "183d")
        else()
            set(opcode "183f")
        endif()
        foreach(base IN ITEMS x y sp pc)
            set(label "target_${ins}_${base}")
            if(base STREQUAL "pc")
                string(APPEND forward_body "\n ${ins} ${label}+1000,${base}\n${label} equ *-1000")
                string(APPEND forward_expected "${opcode}c0")
            else()
                string(APPEND forward_body "\n ${ins} ${label}+1000,${base}\n${label} equ -1000")
                if(base STREQUAL "x")
                    set(postbyte "00")
                elseif(base STREQUAL "y")
                    set(postbyte "40")
                else()
                    set(postbyte "80")
                endif()
                string(APPEND forward_expected "${opcode}${postbyte}")
            endif()
        endforeach()
        if(ins STREQUAL "tbl")
            set(column 6)
        else()
            set(column 7)
        endif()
        check_diagnostic(${ins}-displacement-high-${cpu} ${cpu} " ${ins} 16,x" 1350
            "effective address displacement 16 outside [-16,15]" 3 ${column})
        check_diagnostic(${ins}-displacement-low-${cpu} ${cpu} " ${ins} -17,y" 1350
            "effective address displacement -17 outside [-16,15]" 3 ${column})
        check_diagnostic(${ins}-pc-high-${cpu} ${cpu} " org $1000\n ${ins} *+19,pc" 1350
            "address displacement 16 outside [-16,15] (PC base 0x1003)" 4 ${column})
        check_diagnostic(${ins}-pc-low-${cpu} ${cpu} " org $1000\n ${ins} *-14,pc" 1350
            "address displacement -17 outside [-16,15] (PC base 0x1003)" 4 ${column})
        check_diagnostic(${ins}-final-forward-high-${cpu} ${cpu} " ${ins} target+1000,x\ntarget equ -984" 1350
            "effective address displacement 16 outside [-16,15]" 3 ${column})
    endforeach()
    check_bytes(restricted-forward-${cpu} ${cpu} "${forward_body}" "${forward_expected}")
    check_diagnostic(restricted-undefined-${cpu} ${cpu} " etbl missing,x" 1010 "symbol undefined" 3 7)
    check_diagnostic(restricted-forced-wide-${cpu} ${cpu} " etbl >1,x" 1350 "5-bit offset,X/Y/SP/PC" 3 7)
    check_bytes(restricted-errors-noemit-${cpu} ${cpu}
        " expect 1350,1350\n tbl 16,x\n etbl -17,y\n endexpect\n db $12" "12")
    check_bytes(restricted-original-${cpu} ${cpu}
        " org $1000\n tbl -16,x\n tbl 15,x\n tbl -16,y\n tbl 15,y\n tbl -16,sp\n tbl 15,sp\n tbl *-13,pc\n tbl *+18,pc\n tbl a,x\n tbl b,y\n tbl d,sp\n tbl 1,x+\n etbl -16,x\n etbl 15,x\n etbl -16,y\n etbl 15,y\n etbl -16,sp\n etbl 15,sp\n etbl *-13,pc\n etbl *+18,pc\n etbl a,x\n etbl b,y\n etbl d,sp\n etbl 1,x+"
        "183d10183d0f183d50183d4f183d90183d8f183dd0183dcf183de4183ded183df6183d30183f10183f0f183f50183f4f183f90183f8f183fd0183fcf183fe4183fed183ff6183f30")
endforeach()

# Optional whitespace-separated operands must not split quoted characters or
# parenthesized expressions. Empty/unseparated operands never scan before start.
foreach(cpu IN ITEMS 68HC12 68HC12X)
    check_bytes(space-separated-original-${cpu} ${cpu}
        " movb #1 $2000\n movw #$1234 $2002\n movb 0,x $2004\n movb $2006 0,y\n bset $20 #$40\n bclr 0,x #$40\n brset $20 #$40 *+4\n brclr 0,x #$40 *+4\n movb 32,$2000\n movb #32,$2000\n bset $20,#32\n brset $20,#32,*+4"
        "180b012000180312342002180d00200418094020064c20400d00404e2040000f004000180c00202000180b2020004c20204e202000")
    check_bytes(quoted-space-operands-${cpu} ${cpu}
        " movb ' ',$2000\n movb #' ' $2000\n bset $20,#' '\n brset $20,#' ' *+4"
        "180c00202000180b2020004c20204e202000")
    check_bytes(parenthesized-space-operands-${cpu} ${cpu}
        " movb (16 + 16),$2000\n movb #(16 + 16) $2000\n bset $20,#(16 + 16)\n brset $20,#(16 + 16) *+4"
        "180c00202000180b2020004c20204e202000")
    check_bytes(tab-separated-operands-${cpu} ${cpu}
        " movb #1\t$2000\n bset $20\t#$40\n brset $20\t#$40\t*+4"
        "180b0120004c20404e204000")
    check_bytes(empty-split-operands-${cpu} ${cpu}
        " movb ,$2000\n movb #1,\n bset $20," "180c00002000180b0100004c2000")
    check_diagnostic(empty-split-count-${cpu} ${cpu} " brset ," 1110
        "expected between 3 and 4 arguments but got 2" 3 "")
endforeach()
