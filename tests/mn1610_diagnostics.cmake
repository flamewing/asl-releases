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


foreach(cpu IN ITEMS MN1610 MN1613)
    check_diagnostic(${cpu}-pointer-byte ${cpu} " l R0,(256)(X0)" 1320
        "effective address displacement 256 outside [0,255]" 3 8)
    check_diagnostic(${cpu}-ic ${cpu} " l R0,128(IC)" 1320
        "effective address displacement 128 outside [-128,127]" 3 7)
    check_diagnostic(${cpu}-index ${cpu} " l R0,-1(X0)" 1320
        "effective address displacement -1 outside [0,255]" 3 7)
    check_diagnostic(${cpu}-pointer-register ${cpu} " l R0,(7)(IC)" 310
        "operand must be R3/X0 or R4/X1" 3 11 0)
    foreach(distance IN ITEMS -129 128)
        math(EXPR target "512 + ${distance}")
        check_diagnostic(${cpu}-pcrel-${distance} ${cpu} " org 512\n l R0,${target}" 1370
            "address displacement ${distance} outside [-128,127] (PC base 0x200)" 4 7)
    endforeach()
    check_diagnostic(${cpu}-primary ${cpu} " l R0,1/0(IC)" 1310 "division by 0" 3 "")
    check_bytes(${cpu}-original ${cpu}
        " l R0,1\n l R0,2(IC)\n l R0,(3)\n l R0,(4(IC))\n l R0,5(X0)\n l R0,6(X1)\n l R0,(7)(X0)\n l R0,(8)(X1)"
        "c001c802d003d804e005e806f007f808")
    check_bytes(${cpu}-forward ${cpu}
        " org 512\n l R0,target+1000\ntarget equ -486" "c802")
    check_bytes(${cpu}-forward-index ${cpu}
        " l R0,value+1000(X0)\n l R0,(value+1000)(X1)\n l R0,value+1000(IC)\nvalue equ -998" "e002f802c802")
    check_bytes(${cpu}-endpoints ${cpu}
        " org 512\n l R0,384\n l R0,640\n l R0,-128(IC)\n l R0,127(IC)\n l R0,0(X0)\n l R0,255(X1)\n l R0,(0)(X0)\n l R0,(255)(X1)"
        "c880c87fc880c87fe000e8fff000f8ff")
    check_bytes(${cpu}-noemit ${cpu}
        " org 512\n expect 1370,1320,1320,1320\n l R0,640\n l R0,128(IC)\n l R0,-1(X0)\n l R0,(256)(X0)\n endexpect\n nop" "7808")
endforeach()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/MN1610-ic.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Verschiebung der effektiven Adresse 128" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MN1610 index: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

