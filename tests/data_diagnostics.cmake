# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_diagnostic name cpu body id detail line column)
    set(expected_result 2)
    set(kind error)
    if(ARGC GREATER 7)
        set(expected_result "${ARGV7}")
        set(kind warning)
    endif()
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 4096\n${body}\n")
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
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 4096\n${body}\n")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -q ${ARGN}
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: assembly ${result}\n${output}${error}")
    endif()
    set(assembly_diagnostic "${output}${error}" PARENT_SCOPE)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.p" object)
    execute_process(COMMAND ${TEST_EMULATOR} "${P2BIN_PROGRAM}" "${object}" ${P2BIN_FLAGS}
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${result}\n${output}${error}")
    endif()
    file(READ "${TEST_DIR}/${name}.bin" actual HEX)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: expected ${expected}, got ${actual}")
    endif()
endfunction()

# The integer directives accept signed negatives and unsigned positives.
check_diagnostic(moto-byte-low 68000 " dc.b -129" 1315 "-129 < -128" 3 7)
check_diagnostic(moto-byte-high 68000 " dc.b 256" 1320 "256 > 255" 3 7)
check_diagnostic(moto-word-low 68000 " dc.w -32769" 1315 "-32769 < -32768" 3 7)
check_diagnostic(moto-word-high 68000 " dc.w 65536" 1320 "65536 > 65535" 3 7)
check_diagnostic(moto-pointer-low S912ZVH128F2CLQ " dc.p -8388609" 1315 "-8388609 < -8388608" 3 7)
check_diagnostic(moto-pointer-high S912ZVH128F2CLQ " dc.p 16777216" 1320 "16777216 > 16777215" 3 7)
check_diagnostic(moto-long-low 68000 " dc.l -2147483649" 1315 "-2147483649 < -2147483648" 3 7)
check_diagnostic(moto-long-high 68000 " dc.l 4294967296" 1320 "4294967296 > 4294967295" 3 7)
check_diagnostic(moto-repeat 68000 " dc.w [3]-32769" 1315 "-32769 < -32768" 3 10)
check_diagnostic(moto-list 68000 " dc.b 1,256" 1320 "256 > 255" 3 9)
check_diagnostic(moto-byt 6301 " fcb [2]-129" 1315 "-129 < -128" 3 9)
check_diagnostic(moto-adr 6301 " fdb [2]65536" 1320 "65536 > 65535" 3 9)
check_diagnostic(intel-bit-low "8051:bitsegsize=1" " segment bitdata\n org 0\n d1 -1" 1315 "-1 < 0" 5 5)
check_diagnostic(intel-bit-high "8051:bitsegsize=1" " segment bitdata\n org 0\n d1 2" 1320 "2 > 1" 5 5)
check_diagnostic(intel-nibble-low Z80 " dn -9" 1315 "-9 < -8" 3 5)
check_diagnostic(intel-nibble-high Z80 " dn 16" 1320 "16 > 15" 3 5)
check_diagnostic(intel-byte-low Z80 " db -129" 1315 "-129 < -128" 3 5)
check_diagnostic(intel-byte-high Z80 " db 256" 1320 "256 > 255" 3 5)
check_diagnostic(intel-word-low Z80 " dw -32769" 1315 "-32769 < -32768" 3 5)
check_diagnostic(intel-word-high Z80 " dw 65536" 1320 "65536 > 65535" 3 5)
check_diagnostic(intel-long-low Z80 " dd -2147483649" 1315 "-2147483649 < -2147483648" 3 5)
check_diagnostic(intel-long-high Z80 " dd 4294967296" 1320 "4294967296 > 4294967295" 3 5)
check_diagnostic(intel-nested-dup Z80 " db 2 dup (1,256)" 1320 "256 > 255" 3 14)
check_diagnostic(intel-undefined Z80 " db missing" 1010 "symbol undefined" 3 5)
check_diagnostic(moto-undefined 68000 " dc.b missing" 1010 "symbol undefined" 3 7)

# Range failures stay hard errors even when expression range warnings are enabled.
file(TO_NATIVE_PATH "${TEST_DIR}/moto-byte-low.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -q -n -x -warnranges
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "error #1315:" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "directive hard range: ${result}\n${output}${error}")
endif()

# Golden output captured from the preceding build, including repeats/strings/padding.
check_bytes(moto-layout 68000
    " dc.b -128,255,[2]1,\"ABC\"\n dc.w -32768,65535,[2]1\n dc.l -2147483648,4294967295\n dc.q -1,9223372036854775807"
    "80ff0101414243008000ffff0001000180000000ffffffffffffffffffffffff7fffffffffffffff")
check_bytes(intel-layout Z80
    " dn -8,15\n db -128,255,2 dup (1,2),\"ABC\"\n dw -32768,65535\n dd -2147483648,4294967295\n dq -1,9223372036854775807\n do -1"
    "f880ff010201024142430080ffff00000080ffffffffffffffffffffffffffffffffffffff7fffffffffffffffffffffffffffffffff")
check_bytes(moto-byte-layout 6301
    " fcb -128,255,[2]1,\"ABC\"\n fdb -32768,65535,[2]1"
    "80ff01014142438000ffff00010001")
set(P2BIN_FLAGS -segment bitdata)
check_bytes(bit-layout "8051:bitsegsize=1"
    " segment bitdata\n org 0\n d1 1,0,1,0,1,0,1,0\n dn -8,15\n db -128,255\n dw -32768,65535\n dd -2147483648,4294967295\n dq -1"
    "55f880ff0080ffff00000080ffffffffffffffffffffffff")
unset(P2BIN_FLAGS)
check_bytes(moto-forward 68000 " dc.b [2]value\nvalue equ 255" "ffff")
check_bytes(intel-forward Z80 " db 2 dup (value)\nvalue equ 255" "ffff")
check_diagnostic(intel-forward-low Z80 " db value\nvalue equ -129" 1315 "-129 < -128" 3 5)
check_diagnostic(moto-forward-high 68000 " dc.b value\nvalue equ 256" 1320 "256 > 255" 3 7)

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "moto-byte-low|-129 < -128" "intel-word-high|65536 > 65535")
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
