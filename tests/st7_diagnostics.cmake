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


foreach(cpu IN ITEMS ST7 STM8)
    if(cpu STREQUAL ST7)
        set(length 3)
        set(false_code "0b1200")
        set(true_code "0a1200")
        set(stale_codes "0a12010b1200")
    else()
        set(length 5)
        set(false_code "720b001200")
        set(true_code "720a001200")
        set(stale_codes "720a001201720b001200")
    endif()
    check_bytes(${cpu}-packed-first ${cpu} "mybit bit $12,#5\n btjf mybit,PC+${length}" "${false_code}")
    check_bytes(${cpu}-packed-true ${cpu} "mybit bit $12,#5\n btjt mybit,PC+${length}" "${true_code}")
    check_bytes(${cpu}-packed-stale ${cpu}
        "mybit bit $12,#5\n btjt $12,#5,PC+${length}+1\n btjf mybit,PC+${length}" "${stale_codes}")
    check_bytes(${cpu}-packed-constant ${cpu}
        "mybit bit $12,#5\n btjf mybit,${length}" "${false_code}")
    check_diagnostic(${cpu}-packed-primary ${cpu} "mybit bit $12,#5\n btjf mybit,1/0" 1310 "division by 0" 4 "")
    check_diagnostic(${cpu}-packed-missing ${cpu} "mybit bit $12,#5\n btjf mybit,missing" 1010 "symbol undefined" 4 13)
    check_bytes(${cpu}-packed-primary-noemit ${cpu}
        "mybit bit $12,#5\n expect 1310\n btjf mybit,1/0\n endexpect\n nop" "9d")
endforeach()
check_bytes(ST7-original ST7
 " btjf $12,#5,PC+3\n btjt [$12],#2,PC+4\n jra PC+2\n jrh PC+2" "0b12009204120020002900")
check_bytes(STM8-original STM8
 " btjf $1234,#5,PC+5\n btjt $1234,#2,PC+5\n jra PC+2\n jrh PC+3" "720b12340072041234002000902900")

foreach(cpu IN ITEMS ST7 STM8)
    set(index 0)
    foreach(branch IN ITEMS "jra " "jrh " "btjf $12,#5," "btjt $12,#5," "btjf mybit,")
        math(EXPR index "${index} + 1")
        if(index LESS 3)
            set(length 2)
            if(cpu STREQUAL STM8 AND index EQUAL 2)
                set(length 3)
            endif()
        elseif(cpu STREQUAL ST7)
            set(length 3)
        else()
            set(length 5)
        endif()
        string(LENGTH " ${branch}" column)
        math(EXPR column "${column} + 1")
        foreach(distance IN ITEMS -129 128)
            check_diagnostic(${cpu}-branch-${index}-${distance} ${cpu}
                "mybit bit $12,#5\n org 512\n ${branch}PC+${length}+(${distance})" 1370
                "branch displacement ${distance} outside [-128,127]" 5 ${column})
        endforeach()
        check_diagnostic(${cpu}-branch-${index}-primary ${cpu}
            "mybit bit $12,#5\n ${branch}1/0" 1310 "division by 0" 4 "")
        math(EXPR target "${length} - 1000")
        if(index EQUAL 1)
            set(bytes "2000")
        elseif(index EQUAL 2 AND cpu STREQUAL ST7)
            set(bytes "2900")
        elseif(index EQUAL 2)
            set(bytes "902900")
        elseif(cpu STREQUAL ST7 AND index EQUAL 4)
            set(bytes "0a1200")
        elseif(cpu STREQUAL ST7)
            set(bytes "0b1200")
        elseif(index EQUAL 4)
            set(bytes "720a001200")
        else()
            set(bytes "720b001200")
        endif()
        check_bytes(${cpu}-branch-${index}-forward ${cpu}
            "mybit bit $12,#5\n ${branch}target+1000\ntarget equ ${target}" "${bytes}")
    endforeach()
endforeach()
check_bytes(ST7-endpoints ST7
 " org 512\n jra PC+2-128\n jrh PC+2+127\n btjf $12,#5,PC+3-128\n btjt [$12],#2,PC+4+127" "2080297f0b12809204127f")
check_bytes(STM8-endpoints STM8
 " org 512\n jra PC+2-128\n jrh PC+3+127\n btjf $1234,#5,PC+5-128\n btjt $1234,#2,PC+5+127" "208090297f720b123480720412347f")
check_bytes(ST7-wrap ST7 " org 65534\n jra 0" "2000")
check_bytes(branch-errors-noemit STM8
 "mybit bit $12,#5\n org 512\n expect 1370,1370,1370\n jra PC+2+128\n btjf $12,#5,PC+5+128\n btjf mybit,PC+5+128\n endexpect\n nop" "9d")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/STM8-branch-2-128.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung 128" detail_pos)
string(FIND "${output}${error}" "PC-Basis 0x203" base_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German STM8 branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

foreach(distance IN ITEMS 65536 -65536 65792 -65792)
    check_diagnostic(wide-jra-${distance} STM8
        " org $20000\n jra PC+2+(${distance})" 1370
        "branch displacement ${distance} outside [-128,127]" 4 6)
    check_diagnostic(wide-jrh-${distance} STM8
        " org $20000\n jrh PC+3+(${distance})" 1370
        "branch displacement ${distance} outside [-128,127]" 4 6)
    check_diagnostic(wide-bit-${distance} STM8
        " org $20000\n btjf $12,#5,PC+5+(${distance})" 1370
        "branch displacement ${distance} outside [-128,127]" 4 14)
    check_diagnostic(wide-packed-${distance} STM8
        "mybit bit $12,#5\n org $20000\n btjf mybit,PC+5+(${distance})" 1370
        "branch displacement ${distance} outside [-128,127]" 5 13)
endforeach()
check_bytes(wide-noemit STM8
 " org $20000\nmybit bit $12,#5\n expect 1370,1370,1370,1370\n jra PC+2+$10000\n jrh PC+3-$10000\n btjf $12,#5,PC+5+$10000\n btjf mybit,PC+5-$10000\n endexpect\n nop" "9d")
check_bytes(STM8-top-wrap STM8 " org $fffffe\n jra 0" "2000")
check_bytes(STM8-bank-crossing STM8 " org $ffff\n jrh $10002" "902900")
check_bytes(STM8-high-pc STM8
 " org $20000\n jra PC+2-128\n jrh PC+3+127\n btjf $1234,#5,PC+5-128\n btjt $1234,#2,PC+5+127" "208090297f720b123480720412347f")
