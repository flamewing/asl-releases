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
    set(expected_location "(${line}):")
    if(NOT "${column}" STREQUAL "")
        string(APPEND expected_location "${column}:")
    endif()
    string(FIND "${diagnostic}" "${expected_location} ${kind} #${id}:" location_pos)
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
check_diagnostic(upper-half-register ATMEGA8 " ldi r15,1" 1445
    "register R15 is not allowed here; expected R16..R31" 3 6)
check_diagnostic(multiply-register ATMEGA8 " fmul r24,r16" 1445
    "register R24 is not allowed here; expected R16..R23" 3 7)
check_diagnostic(word-even-register ATMEGA8 " movw r1,r2" 1445
    "expected R0, R2, R4, R6" 3 7)
check_diagnostic(upper-even-register ATMEGA8 " adiw r25,1" 1445
    "expected R24, R26, R28, R30" 3 7)
check_diagnostic(register-alias-subset ATMEGA8 "alias reg r15\n ldi alias,1" 1445
    "register R15 is not allowed here; expected R16..R31" 4 6)
check_diagnostic(register-undefined-primary ATMEGA8 " ldi missing,1" 1445
    "invalid register" 3 6)
check_diagnostic(register-target-primary ATMEGA8
    " cpu 68020\nalias reg d0\n cpu ATMEGA8\n ldi alias,1" 1148
    "register symbol for different target" 6 6)
check_bytes(register-original-subsets ATMEGA8
    " ldi r16,1\n muls r16,r17\n fmul r16,r17\n adiw r26,1\n movw r4,r2"
    "01e00102090311962101")
check_bytes(register-forward-subsets ATMEGA8
    " ldi upper,1\n muls upper,r17\n fmul upper,r17\n adiw pair,1\n movw even,r2\nupper reg r16\npair reg r26\neven reg r4"
    "01e00102090311962101")
check_diagnostic(register-forward-invalid ATMEGA8 " ldi alias,1\nalias reg r15" 1445
    "register R15 is not allowed here; expected R16..R31" 3 6)
check_bytes(register-subset-errors-noemit ATMEGA8
    " expect 1445,1445,1445\n ldi r15,1\n fmul r24,r16\n adiw r25,1\n endexpect\n dw 0x1234" "3412")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/upper-half-register.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Register R15 ist hier nicht erlaubt" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German AVR subset: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(register-forward-divzero ATMEGA8 " ldi alias,1/0\nalias reg r16" 1310
    "division by 0" 3 "")
