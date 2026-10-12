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

foreach(cpu IN ITEMS 6805 68HC05 68HC08 68HCS08)
    check_diagnostic(high-${cpu} ${cpu} " org $1000\n bra *+130" 1370
        "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
    check_diagnostic(low-${cpu} ${cpu} " org $1000\n bne *-127" 1370
        "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
endforeach()

# Every compare/decrement address form, including the SP prefix byte.
foreach(item IN ITEMS
        "compare-a| cbeqa #$12,*+131|3|13"
        "compare-x| cbeqx #$12,*+131|3|13"
        "compare-direct| cbeq $20,*+131|3|11"
        "compare-post| cbeq x+,*+130|2|10"
        "compare-indexed| cbeq $40,x+,*+131|3|14"
        "compare-sp| cbeq $40,sp,*+132|4|14"
        "decrement-a| dbnza *+130|2|8"
        "decrement-x| dbnzx *+130|2|8"
        "decrement-direct| dbnz $20,*+131|3|11"
        "decrement-index| dbnz ,x,*+130|2|10"
        "decrement-offset| dbnz $40,x,*+131|3|13"
        "decrement-sp| dbnz $40,sp,*+132|4|14")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 name)
    list(GET fields 1 body)
    list(GET fields 2 base)
    list(GET fields 3 column)
    check_diagnostic(${name} 68HCS08 " org $1000\n${body}" 1370
        "branch displacement 128 outside [-128,127] (PC base 0x100${base})" 4 ${column})
endforeach()
check_diagnostic(compare-low 68HC08 " org $1000\n cbeqa #$12,*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 13)
check_diagnostic(decrement-low 68HC08 " org $1000\n dbnz $40,sp,*-125" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1004)" 4 14)
check_diagnostic(bit-suffix 68HC05 " org $1000\n brset0 $12,*+131" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1003)" 4 13)
check_diagnostic(bit-explicit 68HC08 " org $1000\n brclr 7,$20,*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 14)
check_diagnostic(no-wrap-13 6805 " org $1ffe\n bra 0" 1370
    "branch displacement -8192 outside [-128,127] (PC base 0x2000)" 4 6)
check_diagnostic(no-wrap-16 68HC08 " org $fffe\n bra 0" 1370
    "branch displacement -65536 outside [-128,127] (PC base 0x10000)" 4 6)
check_diagnostic(forward-range 68HC08 " org $1000\n cbeq $40,sp,target\ntarget equ $1084" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 14)
check_diagnostic(undefined 68HC08 " dbnz $40,sp,missing" 1010 "symbol undefined" 3 14)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/compare-sp.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "128" value_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
string(FIND "${output}${error}" "PC-Basis 0x1004" base_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German HCS08: ${result}\n${output}${error}")
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

# Golden bytes captured with the preceding executable.
foreach(cpu IN ITEMS 6805 68HC05 68HC08 68HCS08)
    check_bytes(relative-${cpu} ${cpu} " org $1000\n bra *+129\n bne *-126" "207f2680")
endforeach()
foreach(cpu IN ITEMS 68HC08 68HCS08)
    check_bytes(compare-${cpu} ${cpu}
        " org $1000\n cbeqa #$12,*+130\n cbeqx #$34,*-125\n cbeq $20,*+130\n cbeq x+,*-126\n cbeq $40,x+,*+130\n cbeq $40,sp,*-124"
        "41127f51348031207f718061407f9e614080")
    check_bytes(decrement-${cpu} ${cpu}
        " org $1000\n dbnza *+129\n dbnzx *-126\n dbnz $20,*+130\n dbnz ,x,*-126\n dbnz $40,x,*+130\n dbnz $40,sp,*-124"
        "4b7f5b803b207f7b806b407f9e6b4080")
endforeach()
check_bytes(bit-suffix-valid 68HC05 " org $1000\n brset0 $12,*+130\n brclr7 $12,*-125" "00127f0f1280")
check_bytes(bit-explicit-valid 68HC08 " org $1000\n brset 0,$20,*+130\n brclr 7,$20,*-125" "00207f0f2080")
check_bytes(forward-valid 68HC08 " org $1000\n cbeq $40,sp,target\ntarget equ $1008" "9e614004")


# Remaining generic register/address diagnostics identify the actual contract.
foreach(cpu IN ITEMS 6805 68HC05 68HC08 68HCS08)
    if(cpu STREQUAL "6805" OR cpu STREQUAL "68HC05")
        set(choices "X")
        check_diagnostic(sp-unavailable-${cpu} ${cpu} " lda 1,sp" 1445 "X" 3 8)
    else()
        set(choices "X or SP")
        check_diagnostic(cbeq-register-${cpu} ${cpu} " cbeq 1,y,*" 1445 "X+ or SP" 3 9)
        check_diagnostic(ais-immediate-${cpu} ${cpu} " ais 1" 1350 "#value" 3 6)
        check_diagnostic(aix-immediate-${cpu} ${cpu} " aix 1" 1350 "#value" 3 6)
    endif()
    check_diagnostic(index-register-${cpu} ${cpu} " lda 1,y" 1445 "${choices}" 3 8)
    check_diagnostic(store-immediate-${cpu} ${cpu} " sta #1" 1350 "direct address in [0,255]" 3 6)
    check_bytes(no-error-bytes-${cpu} ${cpu}
        " expect 1445,1350\n lda 1,y\n sta #1\n endexpect\n db $12" "12")
endforeach()
