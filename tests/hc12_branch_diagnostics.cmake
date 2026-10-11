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

