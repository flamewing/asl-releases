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

check_diagnostic(tiny 68RS08 " add <$10" 1131
    "address 16 outside [0,15] for forced short addressing (<)" 3 6)
check_diagnostic(short 68RS08 " lda <$20" 1131
    "address 32 outside [0,31] for forced short addressing (<)" 3 6)
check_diagnostic(direct 68RS08 " adc $100" 1340
    "address 256 outside [0,255] for this addressing form" 3 6)
check_diagnostic(no-short 68RS08 " adc <$10" 1340
    "forced short addressing (<) is not available for this operand" 3 6)
check_diagnostic(index-displacement 68RS08 " adc 1,x" 1445
    "indexed addressing requires ,X" 3 6)
check_diagnostic(index-register 68RS08 " adc ,y" 1445
    "indexed addressing requires ,X" 3 7)
check_diagnostic(compare-displacement 68RS08 " cbeq 1,x,*" 1445
    "indexed addressing requires ,X" 3 7)
check_diagnostic(compare-register 68RS08 " cbeq ,y,*" 1445
    "indexed addressing requires ,X" 3 8)
check_diagnostic(store-immediate 68RS08 " stx #$12" 1350
    "allowed addressing forms: expr" 3 6)
check_diagnostic(store-index 68RS08 " stx ,x" 1350
    "allowed addressing forms: expr" 3 7)
check_diagnostic(jump-immediate 68RS08 " jmp #$12" 1350
    "allowed addressing forms: expr" 3 6)
check_diagnostic(compare-immediate 68RS08 " cbeq #$12,*" 1350
    "allowed addressing forms: expr, X" 3 7)
check_diagnostic(primary 68RS08 " stx #1/0" 1310 "division by 0" 3 "")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/short.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Adresse 32" value_pos)
string(FIND "${output}${error}" "[0,31]" range_pos)
string(FIND "${output}${error}" "(<)" forced_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR forced_pos EQUAL -1)
    message(FATAL_ERROR "German forced address: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
check_bytes(valid 68RS08 " add <$f\n lda <$1f\n adc $ff\n jmp $100\n cbeq ,x,pcend\npcend:" "6fdfb9ffbc0100310e00")
check_bytes(forward-short 68RS08 " add <address\naddress equ 15" "6f")
