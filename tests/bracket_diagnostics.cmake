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

check_diagnostic(expr-unexpected Z80 " db 1)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 6)
check_diagnostic(expr-missing Z80 " db (1" 1300
    "unclosed bracket '('; expected ')' before end of input" 3 5)
check_diagnostic(expr-mismatch Z80 " db (1}" 1300
    "mismatched closing bracket '}'; expected ')'" 3 7)
check_diagnostic(expr-mixed Z80 " db ({1)}" 1300
    "mismatched closing bracket ')'; expected '}'" 3 8)
check_diagnostic(expr-nested Z80 " db ((1)" 1300
    "unclosed bracket '('; expected ')' before end of input" 3 5)
check_diagnostic(expr-order Z80 " db )1(" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 5)
check_diagnostic(expr-curly-missing Z80 " db ({1" 1300
    "unclosed bracket '{'; expected '}' before end of input" 3 6)
check_diagnostic(expr-curly-unexpected Z80 " db 1}" 1300
    "unexpected closing bracket '}'; no closing bracket expected" 3 6)
check_diagnostic(m68k-unexpected 68020 " pea 1]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 7)
check_diagnostic(m68k-missing 68020 " pea (a0" 1300
    "unclosed bracket '('; expected ')' before end of input" 3 6)
check_diagnostic(m68k-mismatch 68020 " pea ([1,a0)" 1300
    "mismatched closing bracket ')'; expected ']'" 3 12)
check_diagnostic(x86-missing 8086 " mov ax,[bx" 1300
    "unclosed bracket '['; expected ']' before end of input" 3 9)
check_diagnostic(repeat-missing 68000 " dc.b [2" 1300
    "unclosed bracket '['; expected ']' before end of input" 3 7)
check_diagnostic(k2-unexpected 78214 " mov a,123]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 11)
check_diagnostic(k3-unexpected 78310 " mov a,123]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 11)
check_diagnostic(k4-unexpected 784026 " mov a,123]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 11)
check_diagnostic(z8-unexpected Z86C03 " ld r0,1234)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 12)
check_diagnostic(pdk-unexpected PMS132 " mov a,12]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 10)

# Malformed delimiters can hide top-level commas from argument splitting.
# Report the delimiter rather than the resulting (misleading) operand count.
check_diagnostic(split-m68k-mismatch 68000 " move.w (a0],d1" 1300
    "mismatched closing bracket ']'; expected ')'" 3 12)
check_diagnostic(split-m68k-missing 68000 " move.w (a0,d1" 1300
    "unclosed bracket '('; expected ')' before end of input" 3 9)
check_diagnostic(split-m68k-unexpected 68000 " move.w a0),d1" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 11)
check_diagnostic(split-m68k-nested 68020 " move.w ([4,a0),d1" 1300
    "mismatched closing bracket ')'; expected ']'" 3 15)
check_diagnostic(split-x86-mismatch 8086 " mov [bx),ax" 1300
    "mismatched closing bracket ')'; expected ']'" 3 9)
check_diagnostic(split-x86-missing 8086 " mov [bx,ax" 1300
    "unclosed bracket '['; expected ']' before end of input" 3 6)
check_diagnostic(split-z80-mismatch Z80 " ld (hl],a" 1300
    "mismatched closing bracket ']'; expected ')'" 3 8)
check_diagnostic(split-either-or KENBAK " jp a,(12],20" 1300
    "mismatched closing bracket ']'; expected ')'" 3 10)
check_diagnostic(split-curly-mismatch 68000 " move.w (a0},d1" 1300
    "mismatched closing bracket '}'; expected ')'" 3 12)
check_diagnostic(split-positioned-count 68000 " sne d0,(a0]" 1300
    "mismatched closing bracket ']'; expected ')'" 3 12)
check_diagnostic(split-macro-expansion 68000
    "bad macro\n move.w (a0],d1\n endm\n bad" 1300
    "mismatched closing bracket ']'; expected ')'" "6) BAD(1" 12)
check_diagnostic(split-count-balanced 68000 " move.w (a0)" 1110
    "expected 2 arguments but got 1" 3 "")
check_diagnostic(split-count-quoted 68000 " move.w '(',d0,d1" 1110
    "expected 2 arguments but got 3" 3 "")

# The error category and exact location are also corrected without -x.
file(TO_NATIVE_PATH "${TEST_DIR}/split-m68k-mismatch.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "(3):12: error #1300:" location_pos)
string(REGEX MATCHALL "error #[0-9]+:" errors "${output}${error}")
list(LENGTH errors error_count)
if(NOT "${result}" STREQUAL "2" OR location_pos EQUAL -1 OR NOT error_count EQUAL 1)
    message(FATAL_ERROR "Default split delimiter diagnostic: ${result}\n${output}${error}")
endif()

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "expr-unexpected|keine schlie" "expr-mismatch|erwartet ')'" "expr-missing|vor Eingabeende" "split-m68k-mismatch|erwartet ')'")
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

# Golden encodings captured with the preceding batch's executable.
check_bytes(valid-expr Z80 " db ((1+2)*3),STRLEN(\"({)}\"),STRLEN('([')" "090402")
check_bytes(valid-m68k 68020 " pea ([4,a0,d1.w*2],8)" "4870132200040008")
check_bytes(valid-repeat 68000 " dc.b [2]7" "0707")
check_bytes(valid-x86 8086 " mov ax,[bx+2]" "8b4702")
check_bytes(valid-split-m68k 68000 " move.w (a0),d1\n move.w #'(',d0" "3210303c0028")
check_bytes(valid-split-inactive 68000 " if 0\n move.w (a0],d1\n endif\n nop" "4e71")
check_bytes(valid-split-macro 68000 "unused macro\n move.w (a0],d1\n endm\n nop" "4e71")
check_bytes(split-no-emission 68000 " expect 1300\n move.w (a0],d1\n endexpect\n nop" "4e71")

