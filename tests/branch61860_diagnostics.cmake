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

check_diagnostic(jr-high SC61860 " org $1000\n jr *+257" 1370
    "branch displacement 256 outside [-255,255] (PC base 0x1001)" 4 5)
check_diagnostic(jr-low SC61860 " org $1000\n jr *-255" 1370
    "branch displacement -256 outside [-255,255] (PC base 0x1001)" 4 5)
check_diagnostic(jrnz-high SC61860 " org $1000\n jrnz *+257" 1370
    "branch displacement 256 outside [-255,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrnz-low SC61860 " org $1000\n jrnz *-255" 1370
    "branch displacement -256 outside [-255,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrnc-high SC61860 " org $1000\n jrnc *+257" 1370
    "branch displacement 256 outside [-255,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrnc-low SC61860 " org $1000\n jrnc *-255" 1370
    "branch displacement -256 outside [-255,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrz-high SC61860 " org $1000\n jrz *+257" 1370
    "branch displacement 256 outside [-255,255] (PC base 0x1001)" 4 6)
check_diagnostic(jrz-low SC61860 " org $1000\n jrz *-255" 1370
    "branch displacement -256 outside [-255,255] (PC base 0x1001)" 4 6)
check_diagnostic(jrc-high SC61860 " org $1000\n jrc *+257" 1370
    "branch displacement 256 outside [-255,255] (PC base 0x1001)" 4 6)
check_diagnostic(jrc-low SC61860 " org $1000\n jrc *-255" 1370
    "branch displacement -256 outside [-255,255] (PC base 0x1001)" 4 6)
check_diagnostic(jrp-direction SC61860 " org $1000\n jrp *" 1370
    "branch displacement -1 outside [0,255] (PC base 0x1001)" 4 6)
check_diagnostic(jrnzp-direction SC61860 " org $1000\n jrnzp *" 1370
    "branch displacement -1 outside [0,255] (PC base 0x1001)" 4 8)
check_diagnostic(jrncp-direction SC61860 " org $1000\n jrncp *" 1370
    "branch displacement -1 outside [0,255] (PC base 0x1001)" 4 8)
check_diagnostic(jrzp-direction SC61860 " org $1000\n jrzp *" 1370
    "branch displacement -1 outside [0,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrcp-direction SC61860 " org $1000\n jrcp *" 1370
    "branch displacement -1 outside [0,255] (PC base 0x1001)" 4 7)
check_diagnostic(jrm-direction SC61860 " org $1000\n jrm *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 6)
check_diagnostic(jrnzm-direction SC61860 " org $1000\n jrnzm *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 8)
check_diagnostic(jrncm-direction SC61860 " org $1000\n jrncm *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 8)
check_diagnostic(jrzm-direction SC61860 " org $1000\n jrzm *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 7)
check_diagnostic(jrcm-direction SC61860 " org $1000\n jrcm *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 7)
check_diagnostic(loop-direction SC61860 " org $1000\n loop *+2" 1370
    "branch displacement 1 outside [-255,0] (PC base 0x1001)" 4 7)
check_diagnostic(forward-range SC61860 " org $1000\n jrp target\ntarget equ $1101" 1370
    "branch displacement 256 outside [0,255] (PC base 0x1001)" 4 6)
check_diagnostic(undefined SC61860 " org $1000\n jr missing" 1010 "symbol undefined" 4 5)
check_diagnostic(target-overflow SC61860 " org $1000\n jr $10000" 1320 "65536 > 65535" 4 5)
check_diagnostic(no-wrap SC61860 " org $fffe\n jrp 0" 1370
    "branch displacement -65535 outside [0,255] (PC base 0xFFFF)" 4 6)
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/jrp-direction.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "PC-Basis 0x1001" base_pos)
string(FIND "${output}${error}" "[0,255]" range_pos)
if(NOT "${result}" STREQUAL "2" OR base_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German SC61860: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
check_bytes(both SC61860 " org $1000\n jr *+256\n jr *-254\n jrnz *-254\n jrnc *+256\n jrz *-254\n jrc *+256" "2cff2dff29ff2aff39ff3aff")
check_bytes(directed SC61860 " org $1000\n jrp *+256\n jrm *-254\n jrp *+1\n jrm *+1\n loop *-254" "2cff2dff2c002d002fff")
check_bytes(forward SC61860 " org $1000\n jrp target\ntarget equ $1004" "2c03")
