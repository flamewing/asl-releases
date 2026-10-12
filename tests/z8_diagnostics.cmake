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
check_diagnostic(jr-upper Z8601 " jr 130" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x2)" 3 5)
check_diagnostic(jr-conditional Z8601 " jr nz,130" 1370
 "branch displacement 128 outside [-128,127]" 3 8)
check_diagnostic(djnz-upper Z8601 " djnz r3,130" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x2)" 3 10)
check_diagnostic(cpijne-upper Z88C00 " cpijne r3,@r5,131" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 16)
check_diagnostic(btjnz-upper eZ8 " btjnz 5,r7,131" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 13)
check_diagnostic(btjrt-upper Z88C00 " btjrt 131,r5,#7" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 8)
check_diagnostic(jr-lower Z8601 " org 127\n jr 0" 1370
 "branch displacement -129 outside [-128,127] (PC base 0x81)" 4 5)
check_diagnostic(btjrt-lower Z88C00 " org 126\n btjrt 0,r5,#7" 1370
 "branch displacement -129 outside [-128,127]" 4 8)
check_diagnostic(jr-primary Z8601 " jr 1/0" 1310 "division by 0" 3 "")
check_bytes(branch-classic-original Z8601 " jr $+2\n djnz r3,$+2" "8b003a00")
check_bytes(branch-super-original Z88C00
 " cpijne r3,@r5,$+3\n cpije r3,@r5,$+3\n btjrf $+3,r5,#7\n btjrt $+3,r5,#7"
 "d25300c25300375e00375f00")
check_bytes(branch-encore-original eZ8 " btjnz 5,r7,$+3\n btjz 5,@r7,$+3" "f6d700f75700")
check_bytes(jr-forward Z8601 " jr target+1000\ntarget equ -1000" "8bfe")
check_bytes(djnz-forward Z8601 " djnz r3,target+1000\ntarget equ -1000" "3afe")
check_bytes(cpijne-forward Z88C00 " cpijne r3,@r5,target+1000\ntarget equ -1000" "d253fd")
check_bytes(btjnz-forward eZ8 " btjnz 5,r7,target+1000\ntarget equ -1000" "f6d7fd")
check_bytes(btjrt-forward Z88C00 " btjrt target+1000,r5,#7\ntarget equ -1000" "375ffd")
check_bytes(branch-errors-noemit Z8601
 " expect 1370,1370\n jr 130\n djnz r3,130\n endexpect\n db 18" "12")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/jr-upper.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung 128" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Z8 branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
