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
check_diagnostic(br-upper mcore " br 2050" 1370
 "branch displacement 2048 outside [-2048,2046] (PC base 0x2); must be a multiple of 2" 3 5)
check_diagnostic(br-lower mcore " org 2048\n br 0" 1370
 "branch displacement -2050 outside [-2048,2046]" 4 5)
check_diagnostic(br-odd mcore " br 3" 1375
 "branch displacement 1 must be a multiple of 2 (PC base 0x2)" 3 5)
check_diagnostic(loop-upper mcore " loopt r4,2" 1370
 "branch displacement 0 outside [-32,-2]" 3 11)
check_diagnostic(loop-lower mcore " org 32\n loopt r4,0" 1370
 "branch displacement -34 outside [-32,-2]" 4 11)
check_diagnostic(loop-odd mcore " loopt r4,1" 1375
 "branch displacement -1 must be a multiple of 2" 3 11)
check_diagnostic(jmpi-upper mcore " jmpi [1024]" 1370
 "address displacement 1024 outside [0,1020] (PC base 0x0)" 3 7)
check_diagnostic(jsri-lower mcore " org 2\n jsri [0]" 1370
 "address displacement -4 outside [0,1020] (PC base 0x4)" 4 7)
check_diagnostic(jmpi-align mcore " jmpi [2]" 1325
 "operand address 0x2 must be aligned to 4 bytes" 3 7)
check_diagnostic(lrm-upper mcore " lrm r4,[1024]" 1370
 "address displacement 1024 outside [0,1020] (PC base 0x0)" 3 9)
check_diagnostic(lrm-lower mcore " org 2\n lrm r4,[0]" 1370
 "address displacement -4 outside [0,1020] (PC base 0x4)" 4 9)
check_diagnostic(lrm-align mcore " lrm r4,[2]" 1325
 "operand address 0x2 must be aligned to 4 bytes" 3 9)
check_diagnostic(instruction-align mcore " org 1\n br *+2" 180
 "instruction address 0x1 must be aligned to 2 bytes" 4 2 0)
check_diagnostic(branch-primary mcore " br 1/0" 1310 "division by 0" 3 "")
check_diagnostic(literal-primary mcore " lrm r4,[1/0]" 1310 "division by 0" 3 "")
check_bytes(original mcore
 " br *+2\n bf *+2\n bt *+2\n bsr *+2\n loopt r4,*\n align 4\n jmpi [*+4]\n jsri [*+2]\n align 4\n lrm r4,[*+4]\n lrm r4,[*+2]"
 "f000e800e000f800044fffff7001700074017400")
check_bytes(forward mcore
 " br target+10000\n loopt r4,target+10000\n jmpi [target+10004]\n lrm r4,[target+10008]\ntarget equ -10000"
 "f7ff044e70007400")
check_bytes(noemit mcore
 " expect 1370,1375,1370,1325\n br 2050\n loopt r4,*+1\n jmpi [*+1024]\n lrm r4,[2]\n endexpect\n dc.b 18" "12")
check_bytes(branch-endpoints mcore " org 2046\n br 0\n br 4096" "f400f3ff")
check_bytes(loop-endpoints mcore " org 30\n loopt r4,0\n loopt r4,*" "0440044f")
check_bytes(literal-endpoints mcore " org 2\n jmpi [4]\n jsri [1024]" "700070ff")
check_bytes(lrm-endpoints mcore " lrm r4,[0]\n lrm r4,[1024]" "740074ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/br-upper.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung 2048" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MCore branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
