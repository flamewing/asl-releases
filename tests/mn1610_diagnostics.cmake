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


foreach(cpu IN ITEMS MN1610 MN1613)
    check_diagnostic(${cpu}-shift-options ${cpu} " sl R0,bad,Z" 1380
        "operand must be RE, SE, CE" 3 8)
    foreach(op IN ITEMS sbit rbit tbit ai si)
        string(LENGTH "${op}" column)
        math(EXPR column "${column} + 6")
        foreach(value IN ITEMS -1 16)
            check_diagnostic(${cpu}-${op}-${value} ${cpu} " ${op} R0,${value}" 1320
                "immediate value ${value} outside [0,15]" 3 ${column})
        endforeach()
        check_diagnostic(${cpu}-${op}-primary ${cpu} " ${op} R0,1/0" 1310 "division by 0" 3 "")
    endforeach()
    check_bytes(${cpu}-forward-immediate ${cpu} " sbit R0,value+1000\nvalue equ -985" "380f")
endforeach()
foreach(reg IN ITEMS R0 R5 Y0 X2)
    check_diagnostic(indirect-${reg} MN1613 " lr R1,(${reg})" 1445
        "operand must be R1..R4, X0, X1" 3 9)
endforeach()
check_diagnostic(indirect-shape MN1613 " lr R1,R2" 1985
    "operand must be (Rn), -(Rn), (Rn)+; n=1..4, X0=R3, X1=R4" 3 8)
check_diagnostic(indirect-update MN1613 " mvwr R0,(R2)+" 1985
    "operand must be (R1), (R2), (R3/X0), (R4/X1)" 3 10)
check_diagnostic(double-register MN1613 " ad DR1,(R1)" 1445 "operand must be DR0" 3 5)
check_diagnostic(double-register-second MN1613 " fix R0,DR1" 1445 "operand must be DR0" 3 9)
check_bytes(shared-original MN1613
 " sl R0,RE\n sr R1,SE\n sl R2,CE,Z\n lr R1,(R2)\n lr R1,-(R2)\n lr R1,(R2)+\n mvwr R0,(X0)\n ad DR0,(R1)\n fix R0,DR0\n sbit R0,0\n sbit R0,15\n ai R0,15"
 "200d210a224f2141218121c17f0a4f041f0f3800380f480f")
check_bytes(shared-noemit MN1613
 " expect 1380,1445,1445,1320,1320\n sl R0,bad,Z\n lr R1,(R0)\n ad DR1,(R1)\n sbit R0,-1\n ai R0,16\n endexpect\n nop" "7808")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/MN1610-sbit--1.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Unmittelbarer Wert -1" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MN1610 immediate: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)


check_diagnostic(mvwr-fixed MN1613 " mvwr R1,(R1)" 1445 "operand must be R0" 3 7)
check_diagnostic(daa-fixed MN1613 " daa R1,(R1)" 1445 "operand must be R0" 3 6)
check_diagnostic(fix-fixed MN1613 " fix R1,DR0" 1445 "operand must be R0" 3 6)
check_diagnostic(flt-fixed MN1613 " flt DR0,R1" 1445 "operand must be R0" 3 10)
check_diagnostic(srbt-fixed MN1613 " srbt R1,R1" 1445 "operand must be R0" 3 7)
check_diagnostic(debp-fixed MN1613 " debp R1,R1" 1445 "operand must be R0" 3 10)
check_diagnostic(blk-first MN1613 " blk (R1),(R1),R0" 1445 "operand must be (R2)" 3 6)
check_diagnostic(blk-second MN1613 " blk (R2),(R2),R0" 1445 "operand must be (R1)" 3 11)
check_diagnostic(blk-third MN1613 " blk (R2),(R1),R1" 1445 "operand must be R0" 3 16)
check_diagnostic(lb-restricted MN1613 " lb CSBR,1" 1445
    "operand must be SSBR, TSR0, TSR1, OSR0, OSR1, OSR2, OSR3" 3 5)
check_diagnostic(setb-restricted MN1613 " setb R0,CSBR" 1445
    "operand must be SSBR, TSR0, TSR1, OSR0, OSR1, OSR2, OSR3" 3 10)
check_bytes(fixed-original MN1613
 " mvwr R0,(R1)\n daa R0,(R1)\n fix R0,DR0\n flt DR0,R0\n srbt R0,R1\n debp R1,R0\n blk (R2),(R1),R0\n lb SSBR,1\n setb R0,SSBR"
 "7f085f041f0f1f073f713ff13f170f1700010f10")
check_bytes(fixed-noemit MN1613
 " expect 1445,1445,1445,1445,1445,1445,1445,1445,1445,1445,1445\n mvwr R1,(R1)\n daa R1,(R1)\n fix R1,DR0\n flt DR0,R1\n srbt R1,R1\n debp R1,R1\n blk (R1),(R1),R0\n blk (R2),(R2),R0\n blk (R2),(R1),R1\n lb CSBR,1\n setb R0,CSBR\n endexpect\n nop" "7808")
check_bytes(csbr-unrestricted MN1613 " stb CSBR,1\n cpyb R0,CSBR" "0f8700010f80")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/blk-first.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Operand muss (R2) sein" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MN1613 block pointer: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(general-register MN1610 " mv BAD,R0" 1980 "operand must be R0..R4, X0, X1, SP, STR, IC" 3 5)
check_diagnostic(excluded-ic MN1610 " mv IC,R0" 1980 "operand must be R0..R4, X0, X1, SP, STR" 3 5)
check_diagnostic(excluded-str MN1610 " l STR,1" 1980 "operand must be R0..R4, X0, X1, SP" 3 4)
check_diagnostic(skip-names MN1610 " mv R0,R1,bad" 1360 "operand must be SKP, M, PZ, Z, E, NZ, NE, MZ, P, EZ, ENZ, OZ, ONZ, LMZ, LP, LPZ, LM" 3 11)
check_diagnostic(shift-ambiguous MN1610 " sl R0,bad" 1090 "operand must be RE, SE, CE, SKP, M, PZ" 3 8)
check_diagnostic(carry-explicit MN1613 " ad DR0,(R1),bad,Z" 1985 "operand must be 0, 1, C" 3 14)
check_diagnostic(carry-ambiguous MN1613 " ad DR0,(R1),bad" 1360 "operand must be 0, 1, C, SKP, M, PZ" 3 14)
check_diagnostic(base-bb MN1613 " lr R0,bad,(R1)" 1962 "operand must be CSBR, SSBR, TSR0, TSR1" 3 8)
check_diagnostic(base-br MN1613 " cpyb R0,bad" 1962 "operand must be CSBR, SSBR, TSR0, TSR1, OSR0, OSR1, OSR2, OSR3" 3 10)
check_diagnostic(special-register MN1613 " cpys R0,bad" 1980 "operand must be SBRB, ICB, NPP" 3 10)
check_diagnostic(hardware-register MN1613 " cpyh R0,bad" 1980 "operand must be TCR, TIR, TSR, SCR, SSR, SOR, SIR, IISR" 3 10)
check_bytes(names-original MN1613
 " mv R0,SP\n mv X0,X1,Z\n sl R0,RE\n sl R0,Z\n sl R0,CE,Z\n lr SP,TSR1,(X1)+\n ad DR0,(X0),C,Z\n cpyb R2,OSR3\n cpys R3,NPP\n cpyh R4,IISR\n cpyh R4,SOR\n cpyh R4,SIR"
 "780d7b4c200d204c204f25f34f4e0ff20fab3fe43fd43fd4")
check_bytes(names-noemit MN1613
 " expect 1980,1980,1980,1360,1090,1985,1360,1962,1962,1980,1980\n mv BAD,R0\n mv IC,R0\n l STR,1\n mv R0,R1,bad\n sl R0,bad\n ad DR0,(R1),bad,Z\n ad DR0,(R1),bad\n lr R0,bad,(R1)\n cpyb R0,bad\n cpys R0,bad\n cpyh R0,bad\n endexpect\n nop" "7808")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/hardware-register.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Operand muss TCR, TIR" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MN1613 hardware register: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
