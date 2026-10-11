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

# HC12 auto-update has a narrower domain than its signed-byte expression.
check_diagnostic(hc12-high 68HC12 " ldaa 9,x+" 1320 "auto-update count 9 outside [-8,8]" 3 7)
check_diagnostic(hc12-low 68HC12 " ldaa -9,-x" 1315 "auto-update count -9 outside [-8,8]" 3 7)
check_diagnostic(hc12-undefined 68HC12 " ldaa missing,x+" 1010 "symbol undefined" 3 7)
check_diagnostic(hc12-byte-overflow 68HC12 " ldaa 128,x+" 1320 "range overflow" 3 7)

# Aligned field endpoints differ for unconditional and conditional PowerPC branches.
check_diagnostic(ppc-high MPC601 " b *+33554432" 1320
    "branch displacement 33554432 outside [-33554432,33554428]" 3 4)
check_diagnostic(ppc-low MPC601 " b *-33554436" 1315
    "branch displacement -33554436 outside [-33554432,33554428]" 3 4)
check_diagnostic(ppc-conditional-high MPC601 " bc 20,0,*+32768" 1320
    "branch displacement 32768 outside [-32768,32764]" 3 10)
check_diagnostic(ppc-conditional-low MPC601 " bc 20,0,*-32772" 1315
    "branch displacement -32772 outside [-32768,32764]" 3 10)
check_diagnostic(ppc-absolute-high MPC601 " ba 33554432" 1320
    "absolute branch address 33554432 outside [-33554432,33554428]" 3 5)
check_diagnostic(ppc-absolute-conditional MPC601 " bca 20,0,-32772" 1315
    "absolute branch address -32772 outside [-32768,32764]" 3 11)
check_diagnostic(ppc-even-misaligned MPC601 " b *+2" 1375
    "branch displacement 2 must be a multiple of 4" 3 4)
check_diagnostic(ppc-absolute-misaligned MPC601 " ba 2" 1375
    "absolute branch address 2 must be a multiple of 4" 3 5)
check_diagnostic(ppc-conditional-misaligned MPC601 " bc 20,0,*-2" 1375
    "branch displacement -2 must be a multiple of 4" 3 10)
check_diagnostic(ppc-near-high-misaligned MPC601 " b *+33554430" 1375
    "branch displacement 33554430 must be a multiple of 4" 3 4)
check_diagnostic(ppc-undefined MPC601 " b missing" 1010 "symbol undefined" 3 4)

# Use the exact page base chosen by each caller, including PC after the opcode.
check_diagnostic(page11 8051 " org 2047\n ajmp 2048" 1910
    "addresses 0x7FF (page 0x0) and 0x800 (page 0x800) are on different pages; page size 0x800" 4 7)
check_diagnostic(page19 80C390 " org 524287\n ajmp 0" 1910
    "addresses 0x80002 (page 0x80000) and 0x0 (page 0x0) are on different pages; page size 0x80000" 4 7)
check_diagnostic(page16 80C251 " org 65534\n ljmp 0" 1910
    "addresses 0x10001 (page 0x10000) and 0x0 (page 0x0) are on different pages; page size 0x10000" 4 7)
check_diagnostic(page-wrapper PPS-4 " org 64\n t 0" 1910
    "addresses 0x40 (page 0x40) and 0x0 (page 0x0) are on different pages; page size 0x40" 4 "")
check_diagnostic(page-unknown 8051 " ajmp missing" 1010 "symbol undefined" 3 7)
check_diagnostic(placement-ajmp 80C504 " org 2046\n ajmp 1792" 1900
    "AJMP/ACALL on 80C504 cannot start at 0x7FE (offset 0x7fe in a 2-KiB page)" 4 2)
check_diagnostic(placement-acall 80C504 " org 4094\n acall 3840" 1900
    "AJMP/ACALL on 80C504 cannot start at 0xFFE (offset 0x7fe in a 2-KiB page)" 4 2)
check_diagnostic(struct-zero 6301 "Record struct\nfield rmb 1\n endstruct\narray Record [0]" 1315
    "array dimension 0 must be at least 1" 6 15)
check_diagnostic(struct-unknown 6301 "Record struct\nfield rmb 1\n endstruct\narray Record [missing]" 1820
    "must be evaluatable in first pass" 6 15)

# Golden bytes captured before the changes, plus resolved forward symbols.
check_bytes(hc12-boundaries 68HC12
    " ldaa -8,x+\n ldaa -1,x+\n ldaa 0,x+\n ldaa 1,x+\n ldaa 8,x+\n ldaa 8,-x\n ldaa 8,y-\n ldaa -8,+sp"
    "a638a63fa600a630a637a628a678a6a8")
check_bytes(hc12-forward 68HC12 " ldaa count,x+\ncount equ 8" "a637")
check_bytes(ppc-boundaries MPC601
    " b *-33554432\n b *+33554428\n ba -33554432\n ba 33554428\n bc 20,0,*-32768\n bc 20,0,*+32764\n bca 20,0,-32768\n bca 20,0,32764"
    "4a00000049fffffc4a00000249fffffe4280800042807ffc4280800242807ffe")
check_bytes(ppc-forward MPC601 " b target\ntarget: b target" "4800000448000000")
check_bytes(page11-boundary 8051 " org 2047\n ajmp 2047" "e1ff")
check_bytes(page19-boundary 80C390 " org 524287\n ajmp 524288" "010000")
check_bytes(page-forward 8051 " ajmp target\ntarget: nop" "010200")
check_bytes(placement-neighbors 80C504 " org 2045\n ajmp 1792\n org 2047\n acall 1792" "e100f100")
check_bytes(placement-other-cpu 8051 " org 2046\n ajmp 1792" "e100")
check_bytes(struct-one 6301 "Record struct\nfield rmb 1\n endstruct\narray Record [1]\n ldaa array_0_field" "b61000")

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "hc12-high|Auto-Update-Anzahl 9" "ppc-even-misaligned|Vielfaches von 4" "ppc-high|33554432" "page19|Seitengr" "placement-ajmp|80C504" "struct-zero|Arraydimension 0")
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

check_diagnostic(ppc-link-alignment MPC601 " bl *+2" 1375
    "branch displacement 2 must be a multiple of 4" 3 5)
check_diagnostic(ppc-absolute-conditional-alignment MPC601 " bcla 20,0,2" 1375
    "absolute branch address 2 must be a multiple of 4" 3 12)
check_diagnostic(struct-negative 6301 "Record struct\nfield rmb 1\n endstruct\narray Record [-1]" 1315
    "range underflow" 6 15)
