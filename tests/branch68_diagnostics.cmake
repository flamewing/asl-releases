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

foreach(cpu IN ITEMS 6800 6801 6301 6811 68HC11K4)
    check_diagnostic(high-${cpu} ${cpu} " org $1000\n bra *+130" 1370
        "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
    check_diagnostic(low-${cpu} ${cpu} " org $1000\n bne *-127" 1370
        "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
endforeach()
check_diagnostic(bit-direct 6811 " org $1000\n brset $20,#$40,*+132" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 17)
check_diagnostic(bit-x 6811 " org $1000\n brclr 0,x,#$40,*-125" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1004)" 4 17)
check_diagnostic(bit-y 6811 " org $1000\n brset 0,y,#$40,*+133" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1005)" 4 17)
check_diagnostic(bit-spaces 6811 " org $1000\n brset $20 #$40 *+132" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 4 17)
check_diagnostic(wrap-high 6800 " org $fffe\n bra $80" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x10000)" 4 6)
check_diagnostic(wrap-low 6800 " bra $ff81" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x2)" 3 6)
check_diagnostic(bit-wrap-high 6811 " org $fffb\n brset 0,y,#$40,$80" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x10000)" 4 17)
check_diagnostic(forward-range 6800 " org $1000\n bra target\ntarget equ $1082" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(bit-forward-range 6811 " org $1000\n brset 0,y,#$40,target\ntarget equ $1085" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1005)" 4 17)
check_diagnostic(undefined 6800 " bra missing" 1010 "symbol undefined" 3 6)
check_diagnostic(bit-undefined 6811 " brclr 0,x,#$40,missing" 1010 "symbol undefined" 3 17)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/bit-y.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "128" value_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
string(FIND "${output}${error}" "PC-Basis 0x1005" base_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German HC11: ${result}\n${output}${error}")
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

# Baseline bytes captured before this change, with shared branch opcodes.
foreach(cpu IN ITEMS 6800 6801 6301 6811 68HC11K4)
    check_bytes(endpoints-${cpu} ${cpu} " org $1000\n bra *+129\n bne *-126\n bsr *+2" "207f26808d00")
endforeach()
check_bytes(bit-modes 6811
    " org $1000\n brset $20,#$40,*+131\n brclr 0,x,#$40,*-124\n brset 0,y,#$40,*+132"
    "1220407f1f004080181e00407f")
check_bytes(wrap-end 6800 " org $fffe\n bra 0" "2000")
check_bytes(wrap-start 6801 " bra $ff82" "2080")
check_bytes(forward-valid 6800 " org $1000\n bra target\ntarget equ $1004" "2002")
check_bytes(bit-forward-valid 6811 " org $1000\n brset 0,y,#$40,target\ntarget equ $1008" "181e004003")
check_bytes(bit-other-endpoints 6811
    " org $1000\n brclr $20,#$40,*-124\n brset 0,x,#$40,*+131\n brclr 0,y,#$40,*-123"
    "132040801e00407f181f004080")
check_bytes(bit-wrap 6811 " org $fffb\n brset 0,y,#$40,0" "181e004000")

