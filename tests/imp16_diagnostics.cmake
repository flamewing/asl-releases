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
check_diagnostic(imp-subset IMP-16L " and ac2,0" 1445
    "accumulator AC2 is not allowed here; expected AC0..AC1 (0..1)" 3 6)
check_diagnostic(pace-subset IPC-16 " and ac1,0" 1445
    "accumulator AC1 is not allowed here; expected AC0 (0)" 3 6)
check_diagnostic(pace-indirect IPC-16 " ld ac1,@1" 1445
    "accumulator AC1 is not allowed here; expected AC0 (0)" 3 5)
check_diagnostic(index-low IMP-16L " ld ac0,1(ac0)" 1445
    "accumulator AC0 is not allowed here; expected AC2..AC3 (2..3)" 3 11)
check_diagnostic(index-long-low IMP-16L " mpy 1(ac1)" 1445
    "accumulator AC1 is not allowed here; expected AC2..AC3 (2..3)" 3 8)
check_diagnostic(numeric-primary IMP-16L " push 4" 1320 "4 > 3" 3 7)
check_diagnostic(index-primary IMP-16L " ld ac0,1(1/0)" 1310 "division by 0" 3 "")
check_diagnostic(register-target-primary IMP-16L
    " cpu ATMEGA8\nalias reg r0\n cpu IMP-16L\n push alias" 1148
    "register symbol for different target" 6 7)
check_diagnostic(register-missing IMP-16L " push missing" 1010 "symbol undefined" 3 7)
check_bytes(imp-register-original IMP-16L
 " ld ac0,1(ac2)\n and ac1,0\n push ac3\n li ac2,1\n radd ac1,ac2\n shl ac2,3"
 "018200640043014e0036035e")
check_bytes(pace-register-original IPC-16
 " ld ac0,@1\n and ac0,0\n push ac3\n li ac2,1\n radd ac1,ac2\n shl ac2,3"
 "01a000a800630152406a062a")
check_bytes(imp-register-aliases IMP-16L
 " ld ac0,1(index)\n and lower,0\n push upper\n li index,1\n radd lower,index\n shl index,3\nindex equ ac2\nlower equ ac1\nupper equ ac3"
 "018200640043014e0036035e")
check_bytes(imp-register-numeric IMP-16L
 " ld 0,1(2)\n and 1,0\n push 3\n li 2,1\n radd 1,2\n shl 2,3"
 "018200640043014e0036035e")
check_bytes(register-errors-noemit IMP-16L
 " expect 1445,1445,1320\n and ac2,0\n ld ac0,1(ac0)\n push 4\n endexpect\n dw 4660" "3412")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/imp-subset.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Akkumulator AC2 ist hier nicht erlaubt" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German IMP register: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(forward-subset-invalid IMP-16L " and alias,0\nalias equ ac2" 1445
    "expected AC0..AC1 (0..1)" 3 6)
check_diagnostic(forward-index-invalid IMP-16L " ld ac0,1(alias)\nalias equ ac0" 1445
    "expected AC2..AC3 (2..3)" 3 11)
check_bytes(pace-forward-alias IPC-16 " ld alias,@1\nalias equ ac0" "01a0")
foreach(body IN ITEMS " push 4" " li 4,1" " radd 4,ac0" " radd ac0,4" " shl 4,1" " ld 4,0")
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${body}")
    if(body STREQUAL " push 4")
        set(column 7)
    elseif(body STREQUAL " li 4,1" OR body STREQUAL " ld 4,0")
        set(column 5)
    elseif(body STREQUAL " shl 4,1")
        set(column 6)
    elseif(body STREQUAL " radd ac0,4")
        set(column 11)
    else()
        set(column 7)
    endif()
    check_diagnostic("primary-${name}" IMP-16L "${body}" 1320 "4 > 3" 3 ${column})
endforeach()

check_diagnostic(short-no-indirect IMP-16L " and ac0,@1" 1350
 "allowed forms without @: address, displacement(AC2/AC3), address(PC) or #literal" 3 10)
check_diagnostic(long-no-indirect IMP-16L " mpy @1" 1350 "allowed forms without @" 3 6)
check_diagnostic(long-no-literal IMP-16L " ldb #1" 1350
 "explicit PC-relative and #literal forms and @ are not allowed" 3 6)
check_diagnostic(long-no-pc IMP-16L " ldb 1(pc)" 1350
 "expected address or displacement(AC2/AC3)" 3 6)
check_diagnostic(extended-capability IMP-16C/200 " mpy 1" 1500
 "supported CPUs: IMP-16C/300, IMP-16P/300, IMP-16L" 3 2)
check_diagnostic(jsri-address IMP-16L " jsri 128" 1315
 "address 0x80 outside allowed ranges: [0x0,0x7f], [0xff80,0xffff]" 3 7)
check_diagnostic(jsrp-address IMP-16L " jsrp 384" 1320
 "address 0x180 outside allowed ranges: [0x0,0x7f], [0x100,0x17f]" 3 7)
check_diagnostic(boc-upper IMP-16L " boc int,129" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x1)" 3 10)
check_diagnostic(boc-lower IMP-16L " org 128\n boc int,0" 1370
 "branch displacement -129 outside [-128,127] (PC base 0x81)" 4 10)
check_bytes(memory-original IMP-16L
 " mpy 4096\n mpy 1(ac2)\n ldb 1(ac3)\n jsri 127\n jsri 65408\n jsrp 127\n jsrp 383\n boc int,135"
 "8004001080060100c0070100ff0380037f037f037c10")
check_bytes(memory-errors-noemit IMP-16L
 " expect 1350,1350,1350,1350,1315,1320,1370\n and ac0,@1\n mpy @1\n ldb #1\n ldb 1(pc)\n jsri 128\n jsrp 384\n boc int,129\n endexpect\n dw 4660" "3412")
check_bytes(boc-forward IMP-16L " boc int,target\ntarget: push ac0" "00100040")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/jsri-address.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Adresse 0x80" detail_pos)
string(FIND "${output}${error}" "[0xff80,0xffff]" range_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German IMP range: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
