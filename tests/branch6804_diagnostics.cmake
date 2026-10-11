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


check_diagnostic(short-high 6804 " org $100\n bne PC+17" 1370
    "branch displacement 16 outside [-16,15] (PC base 0x101)" 4 6)
check_diagnostic(short-low 6804 " org $100\n beq PC-16" 1370
    "branch displacement -17 outside [-16,15] (PC base 0x101)" 4 6)
check_diagnostic(bit-high 6804 " org $100\n brset 0,$20,PC+131" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x103)" 4 14)
check_diagnostic(bit-low 6804 " org $100\n brclr 7,$20,PC-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x103)" 4 14)
check_diagnostic(forward-high 6804 " org $100\n bne target\ntarget equ $111" 1370
    "branch displacement 16 outside [-16,15] (PC base 0x101)" 4 6)
check_diagnostic(undefined 6804 " bne missing" 1010 "symbol undefined" 3 6)
check_diagnostic(wrap 6804 " bne -32768" 1370
    "branch displacement 32767 outside [-16,15] (PC base 0x1)" 3 6)
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/short-high.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "PC-Basis 0x101" base_pos)
string(FIND "${output}${error}" "[-16,15]" range_pos)
if(NOT "${result}" STREQUAL "2" OR base_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German 6804: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
# Captured before this batch.
check_bytes(relative 6804 " org $100\n bne PC+16\n beq PC-15" "0f30")
check_bytes(bits 6804 " org $100\n brset 0,$20,PC+130\n brclr 7,$20,PC-125" "c8207fc72080")
check_bytes(forward 6804 " org $100\n bne target\ntarget equ $104" "03")
