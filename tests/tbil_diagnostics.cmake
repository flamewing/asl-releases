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


check_diagnostic(signed-high tbil " org $100\n br .+33" 1370
    "branch displacement 32 outside [-32,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(signed-low tbil " org $100\n br .-32" 1370
    "branch displacement -33 outside [-32,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bv-high tbil " org $100\n bv .+33" 1370
    "branch displacement 32 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bv-low tbil " org $100\n bv ." 1370
    "branch displacement -1 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bn-high tbil " org $100\n bn .+33" 1370
    "branch displacement 32 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bn-low tbil " org $100\n bn ." 1370
    "branch displacement -1 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(be-high tbil " org $100\n be .+33" 1370
    "branch displacement 32 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(be-low tbil " org $100\n be ." 1370
    "branch displacement -1 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bc-high tbil " org $100\n bc .+33,\"A\"" 1370
    "branch displacement 32 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(bc-low tbil " org $100\n bc .,\"A\"" 1370
    "branch displacement -1 outside [0,31] (PC base 0x101); zero is reserved" 4 5)
check_diagnostic(br-zero tbil " br .+1" 1371
    "branch displacement 0 is reserved; * selects the explicit zero-field form" 3 5)
check_diagnostic(bv-zero tbil " bv .+1" 1371
    "branch displacement 0 is reserved; * selects the explicit zero-field form" 3 5)
check_diagnostic(bn-zero tbil " bn .+1" 1371
    "branch displacement 0 is reserved; * selects the explicit zero-field form" 3 5)
check_diagnostic(be-zero tbil " be .+1" 1371
    "branch displacement 0 is reserved; * selects the explicit zero-field form" 3 5)
check_diagnostic(bc-zero tbil " bc .+1,\"A\"" 1371
    "branch displacement 0 is reserved; * selects the explicit zero-field form" 3 5)

check_diagnostic(literal-range tbil [=[ charset 65,128
 pc "BA"]=] 1320 "translated string byte 2 has value 128 outside [0,127]" 4 5)
check_diagnostic(control-literal tbil [=[ charset 65,128
 pc "^AA"]=] 1320 "translated string byte 2 has value 128 outside [0,127]" 4 5)
check_diagnostic(branch-literal tbil [=[ charset 65,128
 bc .+2,"A"]=] 1320 "translated string byte 1 has value 128 outside [0,127]" 4 9)
check_diagnostic(undefined tbil " br missing" 1010 "symbol undefined" 3 5)
check_diagnostic(forward-zero tbil " br after\n:after\n no" 1371 "branch displacement 0 is reserved" 3 5)
check_diagnostic(primary tbil " bc .+2,missing" 1010 "symbol undefined" 3 9)
set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "signed-high|PC-Basis 0x101" "br-zero|Nullfeld" "literal-range|[0,127]")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 name)
    list(GET fields 1 detail)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    string(FIND "${output}${error}" "${detail}" detail_pos)
    if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
        message(FATAL_ERROR "German ${name}: ${result}\n${output}${error}")
    endif()
endforeach()
set(ENV{LC_MESSAGES} en)
check_bytes(ranges tbil [=[ org $100
 br .+32
 br .-31
 bv .+32
 bn .+2
 be .+2
 bc .+2,"A"
 br *
 bv *]=] "7f60bfc1e181c160a0")
check_bytes(literal tbil [=[ pc "ABC"
 pc "^@^?"]=] "244142c32400ff")
check_bytes(forward tbil " br after\n no\n:after\n no" "610808")
