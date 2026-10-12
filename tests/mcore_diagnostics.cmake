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
check_diagnostic(br-upper mcore " br 2050" 1370
 "branch displacement 2048 outside [-2048,2046] (PC base 0x2); must be a multiple of 2" 3 5)
check_diagnostic(br-lower mcore " org 2048\n br 0" 1370
 "branch displacement -2050 outside [-2048,2046]" 4 5)
check_diagnostic(br-odd mcore " br 3" 1375
 "branch displacement 1 must be a multiple of 2 (PC base 0x2)" 3 5)
check_diagnostic(loop-upper mcore " loopt r4,2" 1370
 "branch displacement 0 outside [-32,-2]" 3 11)
check_diagnostic(loop-lower mcore " org 32\n loopt r4,0" 1370
 "branch displacement -34 outside [-32,-2]" 4 11)
check_diagnostic(loop-odd mcore " loopt r4,1" 1375
 "branch displacement -1 must be a multiple of 2" 3 11)
check_diagnostic(jmpi-upper mcore " jmpi [1024]" 1370
 "address displacement 1024 outside [0,1020] (PC base 0x0)" 3 7)
check_diagnostic(jsri-lower mcore " org 2\n jsri [0]" 1370
 "address displacement -4 outside [0,1020] (PC base 0x4)" 4 7)
check_diagnostic(jmpi-align mcore " jmpi [2]" 1325
 "operand address 0x2 must be aligned to 4 bytes" 3 7)
check_diagnostic(lrm-upper mcore " lrm r4,[1024]" 1370
 "address displacement 1024 outside [0,1020] (PC base 0x0)" 3 9)
check_diagnostic(lrm-lower mcore " org 2\n lrm r4,[0]" 1370
 "address displacement -4 outside [0,1020] (PC base 0x4)" 4 9)
check_diagnostic(lrm-align mcore " lrm r4,[2]" 1325
 "operand address 0x2 must be aligned to 4 bytes" 3 9)
check_diagnostic(instruction-align mcore " org 1\n br *+2" 180
 "instruction address 0x1 must be aligned to 2 bytes" 4 2 0)
check_diagnostic(branch-primary mcore " br 1/0" 1310 "division by 0" 3 "")
check_diagnostic(literal-primary mcore " lrm r4,[1/0]" 1310 "division by 0" 3 "")
check_bytes(original mcore
 " br *+2\n bf *+2\n bt *+2\n bsr *+2\n loopt r4,*\n align 4\n jmpi [*+4]\n jsri [*+2]\n align 4\n lrm r4,[*+4]\n lrm r4,[*+2]"
 "f000e800e000f800044fffff70017f0074017400")
check_bytes(forward mcore
 " br target+10000\n loopt r4,target+10000\n jmpi [target+10004]\n lrm r4,[target+10008]\ntarget equ -10000"
 "f7ff044e70007400")
check_bytes(noemit mcore
 " expect 1370,1375,1370,1325\n br 2050\n loopt r4,*+1\n jmpi [*+1024]\n lrm r4,[2]\n endexpect\n dc.b 18" "12")
check_bytes(branch-endpoints mcore " org 2046\n br 0\n br 4096" "f400f3ff")
check_bytes(loop-endpoints mcore " org 30\n loopt r4,0\n loopt r4,*" "0440044f")
check_bytes(literal-endpoints mcore " org 2\n jmpi [4]\n jsri [1024]" "70007fff")
check_bytes(lrm-endpoints mcore " lrm r4,[0]\n lrm r4,[1024]" "740074ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/br-upper.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung 2048" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MCore branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(ldm-index mcore " ldm r2-r15,(r1)" 1445 "register R1 is not allowed here; expected R0" 3 14)
check_diagnostic(ldm-first mcore " ldm r0-r15,(r0)" 1445 "register R0 is not allowed here; expected R1..R14" 3 6)
check_diagnostic(ldm-last mcore " ldm r2-r14,(r0)" 1445 "register R14 is not allowed here; expected R15" 3 9)
check_diagnostic(ldq-index mcore " ldq r4-r7,(r5)" 1445 "expected R0..R3, R8..R15" 3 13)
check_diagnostic(ldq-first mcore " ldq r3-r7,(r0)" 1445 "expected R4" 3 6)
check_diagnostic(ldq-last mcore " ldq r4-r8,(r0)" 1445 "expected R7" 3 9)
check_diagnostic(ldm-indirect mcore " ldm r2-r15,r0" 1350 "expected a parenthesized register from R0" 3 13)
check_diagnostic(ldq-indirect mcore " ldq r4-r7,r0" 1350 "expected a parenthesized register from R0..R3, R8..R15" 3 12)
check_diagnostic(ldm-range mcore " ldm r2,(r0)" 1350 "expected register range first-last; first from R1..R14, last from R15" 3 6)
check_diagnostic(ldq-range mcore " ldq r4,(r0)" 1350 "expected register range first-last; first from R4, last from R7" 3 6)
foreach(index IN ITEMS 16 32 65536 4294967296 18446744073709551616)
    check_diagnostic(reg-${index} mcore " mov r1,r${index}" 1445 "r${index}" 3 9)
    if(index GREATER 31)
        check_diagnostic(control-${index} mcore " supmode on\n mfcr r1,cr${index}" 1440 "operand must be CR0..CR31, PSR, VBR" 4 10)
    endif()
endforeach()
# CR16 is valid, so only actual control-register outliers belong in this loop.
check_bytes(register-aliases mcore
 " ldm low-r15,(base)\n ldq quad-last,(index)\n mov r1,r65536\nlow reg r2\nbase reg r0\nquad reg r4\nlast reg r7\nindex reg r8\nr65536 reg r3"
 "006200481231")
check_bytes(register-errors-noemit mcore
 " expect 1445,1445,1350,1440\n ldm r0-r15,(r0)\n ldq r4-r7,(r5)\n ldm r2,r0\n mfcr r1,cr65536\n endexpect\n dc.b 18" "12")

check_diagnostic(register-primary mcore " ldm 1/0-r15,(r0)" 1445 "1/0" 3 6)
check_diagnostic(index-primary mcore " ldm r2-r15,(1/0)" 1445 "1/0" 3 14)
check_bytes(known-looking-alias mcore "r65536 reg r3\n mov r1,r65536" "1231")
check_bytes(control-endpoints mcore " supmode on\n mfcr r1,cr0\n mfcr r1,cr31\n mtcr r1,psr" "100111f11801")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/ldm-index.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Register R1 ist hier nicht erlaubt; erwartet R0" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MCore register: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(address-syntax mcore " ldw r4,r3" 1350 "expected a parenthesized address with one R0..R15 base register" 3 9)
check_diagnostic(address-multiple mcore " ldw r4,(r3,r2)" 1350 "effective address contains more than one base register" 3 13)
check_diagnostic(address-missing mcore " ldw r4,(12)" 1350 "effective address requires one R0..R15 base register" 3 9)
check_diagnostic(address-size mcore " ld.q r4,(r3)" 1130 "supported operand sizes: .B (8), .H (16), .W (32)" 3 5)
check_diagnostic(address-primary mcore " ldw r4,(r3,1/0)" 1310 "division by 0" 3 "")
check_diagnostic(address-align mcore " ldw r4,(r3,6)" 1325 "address displacement 6 must be a multiple of 4" 3 9)
check_diagnostic(address-half-align mcore " ldh r4,(r3,3)" 1325 "address displacement 3 must be a multiple of 2" 3 9)
check_diagnostic(address-upper mcore " ldw r4,(r3,64)" 1320 "effective address displacement 64 outside [0,60]" 3 9)
check_diagnostic(address-lower mcore " ldw r4,(r3,-4)" 1315 "effective address displacement -4 outside [0,60]" 3 9)
check_diagnostic(literal-syntax mcore " jmpi 4" 1350 "operand must be [address]" 3 7)
check_diagnostic(lrm-syntax mcore " lrm r4,4" 1350 "operand must be [address]" 3 9)
check_diagnostic(immediate-primary mcore " addi r1,1/0" 1310 "division by 0" 3 "")
check_diagnostic(immediate-lower mcore " asri r1,0" 1315 "immediate value 0 outside [1,31]" 3 10)
check_diagnostic(immediate-offset-lower mcore " addi r1,0" 1315 "immediate value 0 outside [1,32]" 3 10)
check_diagnostic(immediate-offset-upper mcore " addi r1,33" 1320 "immediate value 33 outside [1,32]" 3 10)
check_bytes(address-forward-base mcore " ldw r4,(base,12)\nbase reg r3" "8433")
check_bytes(address-forward-displacement mcore " ldw r4,(r3,amount-10000)\namount equ 10012" "8433")
check_bytes(address-endpoints mcore
 " ldb r4,(r3,0)\n ldb r4,(15,r3)\n ldh r4,(r3,30)\n ldw r4,(60,r3)" "a403a4f3c4f384f3")
check_bytes(immediate-endpoints mcore
 " asri r1,1\n asri r1,31\n addi r1,1\n addi r1,32" "3a113bf1200121f1")
check_bytes(remaining-errors-noemit mcore
 " expect 1350,1350,1130,1315,1320,1325\n ldw r4,(r3,r2)\n ldw r4,(12)\n ld.q r4,(r3)\n asri r1,0\n addi r1,33\n ldw r4,(r3,6)\n endexpect\n dc.b 18" "12")

check_diagnostic(aggregate-positive-overflow mcore
 " ldw r4,(r3,2147483647,2147483647,2)" 1320
 "effective address displacement 4294967296 outside [0,60]" 3 9)
check_diagnostic(aggregate-negative-overflow mcore
 " ldw r4,(r3,-2147483648,-2147483648)" 1315
 "effective address displacement -4294967296 outside [0,60]" 3 9)
check_diagnostic(aggregate-unsigned-wrap mcore " ldw r4,(r3,4294967295,1)" 1320
 "effective address displacement 4294967296 outside [0,60]" 3 9)
check_bytes(aggregate-cancellation mcore
 " ldw r4,(r3,2147483647,2147483647,-2147483647,-2147483647,12)" "8433")
check_bytes(aggregate-overflow-noemit mcore
 " expect 1320,1315\n ldw r4,(r3,2147483647,2147483647,2)\n ldw r4,(r3,-2147483648,-2147483648)\n endexpect\n dc.b 18" "12")

# JMPI and JSRI must differ; JSRI saves the return address in R15.
check_bytes(literal-jump-versus-call mcore " jmpi [4]\n jsri [4]" "70017f00")
check_bytes(literal-call-forward mcore " jsri [target+10004]\ntarget equ -10000" "7f01")
