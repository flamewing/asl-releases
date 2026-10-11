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

check_diagnostic(16C54-goto 16C54 " goto 512" 1320
    "PIC word target 512 outside [0,511] for 16C54" 3 7)
check_diagnostic(16C54-call 16C54 " call 512" 1320
    "PIC word target 512 outside [0,511] for 16C54" 3 7)
check_diagnostic(16C55-goto 16C55 " goto 512" 1320
    "PIC word target 512 outside [0,511] for 16C55" 3 7)
check_diagnostic(16C55-call 16C55 " call 512" 1320
    "PIC word target 512 outside [0,511] for 16C55" 3 7)
check_diagnostic(16C56-goto 16C56 " goto 1024" 1320
    "PIC word target 1024 outside [0,1023] for 16C56" 3 7)
check_diagnostic(16C56-call 16C56 " call 1024" 1320
    "PIC word target 1024 outside [0,1023] for 16C56" 3 7)
check_diagnostic(16C57-goto 16C57 " goto 2048" 1320
    "PIC word target 2048 outside [0,2047] for 16C57" 3 7)
check_diagnostic(16C57-call 16C57 " call 2048" 1320
    "PIC word target 2048 outside [0,2047] for 16C57" 3 7)
check_diagnostic(16C64-goto 16C64 " goto 2048" 1320
    "PIC word target 2048 outside [0,2047] for 16C64" 3 7)
check_diagnostic(16C64-call 16C64 " call 2048" 1320
    "PIC word target 2048 outside [0,2047] for 16C64" 3 7)
check_diagnostic(16C84-goto 16C84 " goto 1024" 1320
    "PIC word target 1024 outside [0,1023] for 16C84" 3 7)
check_diagnostic(16C84-call 16C84 " call 1024" 1320
    "PIC word target 1024 outside [0,1023] for 16C84" 3 7)
check_diagnostic(16C873-goto 16C873 " goto 4096" 1320
    "PIC word target 4096 outside [0,4095] for 16C873" 3 7)
check_diagnostic(16C873-call 16C873 " call 4096" 1320
    "PIC word target 4096 outside [0,4095] for 16C873" 3 7)
check_diagnostic(16C874-goto 16C874 " goto 4096" 1320
    "PIC word target 4096 outside [0,4095] for 16C874" 3 7)
check_diagnostic(16C874-call 16C874 " call 4096" 1320
    "PIC word target 4096 outside [0,4095] for 16C874" 3 7)
check_diagnostic(16C876-goto 16C876 " goto 8192" 1320
    "PIC word target 8192 outside [0,8191] for 16C876" 3 7)
check_diagnostic(16C876-call 16C876 " call 8192" 1320
    "PIC word target 8192 outside [0,8191] for 16C876" 3 7)
check_diagnostic(16C877-goto 16C877 " goto 8192" 1320
    "PIC word target 8192 outside [0,8191] for 16C877" 3 7)
check_diagnostic(16C877-call 16C877 " call 8192" 1320
    "PIC word target 8192 outside [0,8191] for 16C877" 3 7)
check_diagnostic(call-bit 16C54 " call $100" 1905
    "CALL target 0x100 has bit 8 set; bit 8 must be clear" 3 7)
check_diagnostic(call-bit-page 16C57 " call $700" 1905
    "CALL target 0x700 has bit 8 set; bit 8 must be clear" 3 7)
check_diagnostic(wrapped-target 16C84 " goto -1" 1320
    "PIC word target 65535 outside [0,1023] for 16C84" 3 7)
check_diagnostic(unsigned-target 16C54 " goto -1" 1315 "-1 < 0" 3 7)
check_diagnostic(undefined 16C84 " call missing" 1010 "symbol undefined" 3 7)
check_diagnostic(scale-name ATARI_VECTOR " svec (0,0),s4,z0" 1320 "SVEC argument 4 outside [0,3]" 3 13)
check_diagnostic(scale-expr ATARI_VECTOR " svec (0,0),4,z0" 1320 "SVEC argument 4 outside [0,3]" 3 13)
check_diagnostic(x-alignment ATARI_VECTOR " svec (-1,256),s0,z0" 1325
    "SVEC coordinates (-1,256) must each be multiples of 256" 3 7)
check_diagnostic(y-alignment ATARI_VECTOR " svec (256,1),s0,z0" 1325
    "SVEC coordinates (256,1) must each be multiples of 256" 3 7)
check_diagnostic(scale-primary ATARI_VECTOR " svec (0,0),1/0,z0" 1310 "division by 0" 3 "")
set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "16C54-goto|[0,511]" "call-bit|Bit 8" "x-alignment|(-1,256)")
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
check_bytes(pic54 16C54 " goto $1ff\n call $ff" "ff0bff09")
check_bytes(pic57 16C57 " goto $7ff\n call $600" "a305c305ff0ba305c3050009")
check_bytes(pic84 16C84 " goto $3ff\n call $100" "ff2b0021")
check_bytes(pic877 16C877 " goto $1fff\n call $800" "8a150a16ff2f8a150020")
check_bytes(vector ATARI_VECTOR " svec (768,-768),s3,z15\n svec (0,0),s0,z0" "fbfb00f0")
check_bytes(forward-pic 16C84 " goto target\ntarget equ $3ff" "ff2b")
