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


set(branches "btjf r10.2," "btjt r10.2," "cpjfi r2,(rr14)," "cpjti r2,(rr14)," "djnz r6," "dwjnz RR6," "jrt ")
set(index 0)
foreach(branch IN LISTS branches)
    math(EXPR index "${index} + 1")
    if(index EQUAL 5 OR index EQUAL 7)
        set(length 2)
    else()
        set(length 3)
    endif()
    string(LENGTH " ${branch}" column)
    math(EXPR column "${column} + 1")
    foreach(distance IN ITEMS -129 128)
        check_diagnostic(branch-${index}-${distance} ST9040
            " org 512\n ${branch}PC+${length}+(${distance})" 1370
            "branch displacement ${distance} outside [-128,127]" 4 ${column})
    endforeach()
    check_diagnostic(branch-${index}-primary ST9040 " ${branch}1/0" 1310 "division by 0" 3 "")
endforeach()
check_bytes(original ST9040
 " btjf r10.2,PC+3\n btjt r10.2,PC+3\n cpjfi r2,(rr14),PC+3\n cpjti r2,(rr14),PC+3\n djnz r6,PC+2\n dwjnz RR6,PC+3\n jrt PC+2"
 "af5a00af4a009fe2009ff2006a00c606008b00")
check_bytes(endpoints ST9040
 " org 512\n btjf r10.2,PC+3-128\n btjt r10.2,PC+3+127\n cpjfi r2,(rr14),PC+3-128\n cpjti r2,(rr14),PC+3+127\n djnz r6,PC+2-128\n dwjnz RR6,PC+3+127\n jrt PC+2-128"
 "af5a80af4a7f9fe2809ff27f6a80c6067f8b80")
check_bytes(forward ST9040
 " btjf r10.2,target+1000\n btjt r10.2,target+1003\n cpjfi r2,(rr14),target+1006\n cpjti r2,(rr14),target+1009\n djnz r6,target+1011\n dwjnz RR6,target+1014\n jrt target+1016\ntarget equ -997"
 "af5a00af4a009fe2009ff2006a00c606008b00")
check_bytes(wrap ST9040 " org 65534\n jrt 0" "8b00")
check_bytes(noemit ST9040
 " expect 1370,1370,1370,1370,1370\n btjf r10.2,PC+3+128\n cpjfi r2,(rr14),PC+3+128\n djnz r6,PC+2+128\n dwjnz RR6,PC+3+128\n jrt PC+2+128\n endexpect\n nop" "ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/branch-7-128.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "128" detail_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)


check_diagnostic(postinc-shape ST9040 " adc r13,rr4+" 1350 "operand must be (rn)+ or (rrn)+" 3 10)
foreach(reg IN ITEMS bad rr1 rr15 rr16 rr256 rr4294967296)
    check_diagnostic(postinc-${reg} ST9040 " adc r13,(${reg})+" 1445
        "operand must be r0..r15 or rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 11)
    check_diagnostic(predec-${reg} ST9040 " adc r13,-(${reg})" 1445
        "operand must be rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 12)
endforeach()
check_diagnostic(predec-byte ST9040 " adc r13,-(r4)" 1350
    "operand must be rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 12)
check_diagnostic(native-narrow ST9040 " ld r1,r256" 1010 "symbol undefined" 3 8)
check_diagnostic(native-pair-narrow ST9040 " ldw rr2,rr256" 1010 "symbol undefined" 3 10)
check_bytes(update-original ST9040
 " adc r13,(rr4)+\n adc r13,-(rr4)\n adc (rr4)+,r13\n adc -(rr4),r13"
 "b435ddc235ddb434ddc234dd")
check_bytes(update-noemit ST9040
 " expect 1445,1445,1350\n adc r13,(rr1)+\n adc r13,-(rr1)\n adc r13,-(r4)\n endexpect\n nop" "ff")
check_bytes(native-looking-number ST9040
 "r256 equ 1000\nrr256 equ 1000\n ld r1,r256\n ldw rr2,rr256" "c4f103e8e2f203e8")
check_bytes(native-looking-alias ST9040 "r256 equ R10\nrr256 equ RR10\n ld r1,r256\n ldw rr2,rr256" "c4f1000ae2f2010a")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/predec-rr1.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Operand muss rr0, rr2" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 predecrement: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
