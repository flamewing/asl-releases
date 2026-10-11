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

# Exact counts where parsing finished; conservative lower bounds at early exits.
check_diagnostic(builtin-extra 68000 " dc.l SIN(1,2)" 1490
    "function SIN: argument count 1 required, got 2" 3 7)
check_diagnostic(builtin-few 68000 " dc.l STRSTR(\"a,b\")" 1490
    "function STRSTR: argument count 2 required, got 1" 3 7)
check_diagnostic(builtin-three 68000 " dc.l SUBSTR(\"a,b\",1)" 1490
    "function SUBSTR: argument count 3 required, got 2" 3 7)
check_diagnostic(builtin-quoted 68000 " dc.l STRLEN(\"a,b\",\"c\")" 1490
    "function STRLEN: argument count 1 required, got 2" 3 7)
check_diagnostic(builtin-cap 68000 " dc.l SIN(1,2,3,4,5)" 1490
    "function SIN: argument count 1 required, got at least 4" 3 7)
check_diagnostic(builtin-cap-empty-tail 68000 " dc.l SIN(1,2,3,)" 1490
    "function SIN: argument count 1 required, got at least 4" 3 7)
check_diagnostic(builtin-cap-unevaluated 68000 " dc.l SIN(1,2,3,missing)" 1490
    "function SIN: argument count 1 required, got at least 4" 3 7)
check_diagnostic(builtin-range 68000 " dc.l CODEPAGE_VAL(1,2,3)" 1490
    "function CODEPAGE_VAL: argument count must be in [1,2], got 3" 3 7)
check_diagnostic(builtin-range-cap 68000 " dc.l CODEPAGE_VAL(1,2,3,4,5)" 1490
    "function CODEPAGE_VAL: argument count must be in [1,2], got at least 4" 3 7)
check_diagnostic(builtin-nested 68000 " dc.l INT(SIN(1,2))" 1490
    "function SIN: argument count 1 required, got 2" 3 11)
check_diagnostic(builtin-trailing-empty 68000 " dc.l INT(1,)" 1490
    "function INT: argument count 1 required, got 2" 3 7)
check_diagnostic(user-few 68000 "sum function a,b,a+b\n dc.l sum(1)" 1490
    "function sum: argument count 2 required, got 1" 4 7)
check_diagnostic(user-empty 68000 "sum function a,b,a+b\n dc.l sum()" 1490
    "function sum: argument count 2 required, got 0" 4 7)
check_diagnostic(user-extra 68000 "sum function a,b,a+b\n dc.l sum(1,2,3,4)" 1490
    "function sum: argument count 2 required, got at least 3" 4 7)
check_diagnostic(user-extra-unevaluated 68000 "sum function a,b,a+b\n dc.l sum(1,2,missing)" 1490
    "function sum: argument count 2 required, got at least 3" 4 7)
check_diagnostic(user-zero-extra 68000 "zero function 7\n dc.l zero(1,2)" 1490
    "function zero: argument count 0 required, got at least 1" 4 7)
check_diagnostic(user-many-few 68000 "many function a,b,c,d,e,a+b+c+d+e\n dc.l many(1,2,3,4)" 1490
    "function many: argument count 5 required, got 4" 4 7)
check_diagnostic(user-nested 68000 "sum function a,b,a+b\n dc.l sum(1,sum(2))" 1490
    "function sum: argument count 2 required, got 1" 4 13)
check_diagnostic(unknown-parser-cap 68000 " dc.l what(1,2,3,4)" 1490
    "function WHAT: parser accepts at most 3 arguments, got at least 4" 3 7)
check_diagnostic(unknown-function 68000 " dc.l what(1,2,3)" 1860 "unknown function" 3 7)
check_diagnostic(builtin-forward-arity 68000 " dc.l SIN(1,missing)" 1490
    "function SIN: argument count 1 required, got 2" 3 7)
check_diagnostic(user-evaluation-first 68000 "sum function a,a\n dc.l sum(missing)" 1010 "symbol undefined" 4 11)
check_diagnostic(builtin-zero-divisor 68000 " dc.l SIN(1/0,2)" 1310 "division by 0" 3 "")

# Golden bytes from the previous build: quotes, nested calls, >3 user arguments,
# zero-argument functions, forward references, and legacy empty/trailing slots.
check_bytes(valid-functions 68000
    "sum function a,b,a+b\njoin function a,b,a+b\nmany function a,b,c,d,e,a+b+c+d+e\nzero function 7\n dc.l sum(1,2),sum(1,2,),zero(),many(1,2,3,4,5),STRLEN(join(\"a,b\",\"c\")),STRLEN(SUBSTR(\"a,b,c\",1,3))\n dc.d SIN(),SIN(0),COS(0)"
    "0000000300000003000000070000000f0000000400000003000000000000000000000000000000003ff0000000000000")
check_bytes(empty-slots 68000 "sum function a,b,a+b\n dc.l sum(1,,),sum(,2),INT()"
    "000000010000000200000000")
check_bytes(user-forward 68000 "sum function a,b,a+b\n dc.l sum(value,2)\nvalue equ 3" "00000005")

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "builtin-extra|Funktion SIN: Argumentzahl 1" "builtin-range|[1,2]" "user-extra|mindestens 3" "unknown-parser-cap|mindestens 4")
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
