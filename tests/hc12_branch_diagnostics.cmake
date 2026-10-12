# SPDX-License-Identifier: GPL-2.0-only
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
