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

check_diagnostic(t370-unexpected 370C010 " push 123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 10)
check_diagnostic(t370-immediate 370C010 " mov a,#123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 12)
check_diagnostic(t8x30x-unexpected 8X305 " xec 123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 9)
check_diagnostic(t9900-unexpected TMS9900 " b @123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 8)
check_diagnostic(tms7-unexpected TMS70C08 " lda @123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 10)
check_diagnostic(tms7-single TMS70C08 " lda @)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 7)
check_diagnostic(ns32k-missing NS32016 " absd r0,ext(123" 1300
    "unclosed bracket '('; expected ')' before end of input" 3 13)
check_diagnostic(t960-square 80960 " st r0,123]" 1300
    "unexpected closing bracket ']'; no closing bracket expected" 3 11)
check_diagnostic(t960-round 80960 " st r0,123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 11)
check_diagnostic(kcpsm-unexpected KCPSM " load s0,123)" 1300
    "unexpected closing bracket ')'; no closing bracket expected" 3 13)
check_diagnostic(vector-opening ATARI_VECTOR " labs [1,2],s0" 1300
    "vector coordinates require opening bracket '('; got '['" 3 7)
check_diagnostic(vector-closing ATARI_VECTOR " labs (1,2)x,s0" 1300
    "vector coordinates require final closing bracket ')'; got 'x'" 3 12)
check_diagnostic(include-quote Z80 " include \"missing" 1300
    "unclosed quote '\"'; expected '\"' before end of file name" 3 10)
check_diagnostic(binclude-quote Z80 " binclude \"missing" 1300
    "unclosed quote '\"'; expected '\"' before end of file name" 3 11)
check_diagnostic(include-single-quote Z80 " include \"" 1300
    "unclosed quote '\"'; expected '\"' before end of file name" 3 10)

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "t370-unexpected|keine schlie" "ns32k-missing|vor Eingabeende" "vector-opening|gefunden '['" "vector-closing|gefunden 'x'" "include-quote|vor Ende des Dateinamens")
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

# Golden bytes captured with the executable from the preceding batch.
check_bytes(valid-370 370C010 " mov #12h,a\n push r5" "2212d805")
check_bytes(valid-8x30x 8X305 " add r11(3),r5" "6529")
check_bytes(valid-9900 TMS9900 " mov @1234h(r6),r1" "c0661234")
check_bytes(valid-tms7 TMS70C08 " lda @1234h(b)" "aa1234")
check_bytes(valid-ns32k NS32016 " absd ext(1000)+2000,r0" "4e33b083e887d0")
check_bytes(valid-960 80960 " ld 12(r3)[r4*2],r5" "84fc28900c000000")
check_bytes(valid-kcpsm KCPSM " input s0,12(s1)" "b010")
check_bytes(valid-vector ATARI_VECTOR " labs (1,2),s0" "02a00100")

file(WRITE "${TEST_DIR}/good.inc" " db 12h\n")
file(WRITE "${TEST_DIR}/good binary.bin" "AZ")
check_bytes(valid-include-quoted Z80 " include \"good.inc\"" "12")
check_bytes(valid-include-unquoted Z80 " include good.inc" "12")
check_bytes(valid-binclude Z80 " binclude \"good binary.bin\"" "415a")
check_bytes(include-recovery Z80 " expect 1300\n include \"missing\n endexpect\n db 12h" "12")
check_bytes(binclude-recovery Z80 " expect 1300\n binclude \"missing\n endexpect\n db 12h" "12")

