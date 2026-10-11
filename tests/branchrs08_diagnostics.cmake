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

# All seven rejection paths, including the operand-count variants.
foreach(item IN ITEMS
        "relative| bra *+130|2|6"
        "compare-a| cbeqa #$12,*+131|3|13"
        "compare-direct| cbeq $20,*+131|3|11"
        "compare-x| cbeq x,*+131|3|9"
        "compare-index| cbeq ,x,*+131|3|10"
        "decrement-a| dbnza *+130|2|8"
        "decrement-x| dbnzx *+131|3|8"
        "decrement-direct| dbnz $20,*+131|3|11"
        "decrement-index| dbnz ,x,*+131|3|10"
        "decrement-dx| dbnz d[x],*+131|3|12"
        "bit-suffix| brset0 $12,*+131|3|13"
        "bit-explicit| brclr 7,$20,*+131|3|14"
        "bit-dx| brset 0,d[x],*+131|3|15"
        "bit-x| brclr 0,x,*+131|3|12")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 name)
    list(GET fields 1 body)
    list(GET fields 2 base)
    list(GET fields 3 column)
    check_diagnostic(${name} 68RS08 " org $1000\n${body}" 1370
        "branch displacement 128 outside [-128,127] (PC base 0x100${base})" 4 ${column})
endforeach()
check_diagnostic(low 68RS08 " org $1000\n bne *-127" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(compare-low 68RS08 " org $1000\n cbeqa #$12,*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 13)
check_diagnostic(no-wrap 68RS08 " org $3ffe\n bra 0" 1370
    "branch displacement -16384 outside [-128,127] (PC base 0x4000)" 4 6)
check_diagnostic(forward-range 68RS08 " org $1000\n cbeq $20,target\ntarget equ $1083" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1003)" 4 11)
check_diagnostic(undefined 68RS08 " dbnz $20,missing" 1010 "symbol undefined" 3 11)
check_diagnostic(target-limit 68RS08 " bra $4000" 1320 "16384" 3 6)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/compare-a.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "PC-Basis 0x1003" base_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
if(NOT "${result}" STREQUAL "2" OR base_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German RS08: ${result}\n${output}${error}")
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

# Captured with the preceding executable, including BRN's pseudo encoding.
check_bytes(relative 68RS08 " org $1000\n bra *+129\n bne *-126\n bsr *+129\n brn *+129" "307f3680ad7f3000")
check_bytes(compare 68RS08 " org $1000\n cbeqa #$12,*+130\n cbeq $20,*-125\n cbeq x,*+130\n cbeq ,x,*-125" "41127f312080310f7f310e80")
check_bytes(decrement 68RS08 " org $1000\n dbnza *+129\n dbnzx *-125\n dbnz $20,*+130\n dbnz ,x,*-125\n dbnz d[x],*+130" "4b7f3b0f803b207f3b0e803b0e7f")
check_bytes(bits 68RS08 " org $1000\n brset0 $12,*+130\n brclr 7,$20,*-125\n brset 0,d[x],*+130\n brclr 0,x,*-125" "00127f0f2080000e7f010f80")
check_bytes(forward 68RS08 " org $1000\n cbeq $20,target\ntarget equ $1008" "312005")


# Suffix bit syntax must recognize the address operand, as explicit bits do.
check_bytes(suffix-brset-x 68RS08 " brset0 x,*+3\n brset1 x,*+3\n brset2 x,*+3\n brset3 x,*+3\n brset4 x,*+3\n brset5 x,*+3\n brset6 x,*+3\n brset7 x,*+3" "000f00020f00040f00060f00080f000a0f000c0f000e0f00")
check_bytes(suffix-brset-dx 68RS08 " brset0 d[x],*+3\n brset1 d[x],*+3\n brset2 d[x],*+3\n brset3 d[x],*+3\n brset4 d[x],*+3\n brset5 d[x],*+3\n brset6 d[x],*+3\n brset7 d[x],*+3" "000e00020e00040e00060e00080e000a0e000c0e000e0e00")
check_bytes(suffix-brclr-x 68RS08 " brclr0 x,*+3\n brclr1 x,*+3\n brclr2 x,*+3\n brclr3 x,*+3\n brclr4 x,*+3\n brclr5 x,*+3\n brclr6 x,*+3\n brclr7 x,*+3" "010f00030f00050f00070f00090f000b0f000d0f000f0f00")
check_bytes(suffix-brclr-dx 68RS08 " brclr0 d[x],*+3\n brclr1 d[x],*+3\n brclr2 d[x],*+3\n brclr3 d[x],*+3\n brclr4 d[x],*+3\n brclr5 d[x],*+3\n brclr6 d[x],*+3\n brclr7 d[x],*+3" "010e00030e00050e00070e00090e000b0e000d0e000f0e00")
check_diagnostic(suffix-x-high 68RS08 " org $1000\n brset0 x,*+131" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1003)" 4 11)
check_diagnostic(suffix-dx-low 68RS08 " org $1000\n brclr7 d[x],*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 14)
check_diagnostic(suffix-undefined 68RS08 " brset0 x,missing" 1010 "symbol undefined" 3 11)
check_bytes(suffix-forward 68RS08 " brclr7 d[x],target\ntarget equ 5" "0f0e02")
