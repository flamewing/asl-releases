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


set(branches "btjf r10.2," "btjt r10.2," "cpjfi r2,(rr14)," "cpjti r2,(rr14)," "djnz r6," "dwjnz RR6," "jrt ")
set(index 0)
foreach(branch IN LISTS branches)
    math(EXPR index "${index} + 1")
    if(index EQUAL 5 OR index EQUAL 7)
        set(length 2)
    else()
        set(length 3)
    endif()
    string(LENGTH " ${branch}" column)
    math(EXPR column "${column} + 1")
    foreach(distance IN ITEMS -129 128)
        check_diagnostic(branch-${index}-${distance} ST9040
            " org 512\n ${branch}PC+${length}+(${distance})" 1370
            "branch displacement ${distance} outside [-128,127]" 4 ${column})
    endforeach()
    check_diagnostic(branch-${index}-primary ST9040 " ${branch}1/0" 1310 "division by 0" 3 "")
endforeach()
check_bytes(original ST9040
 " btjf r10.2,PC+3\n btjt r10.2,PC+3\n cpjfi r2,(rr14),PC+3\n cpjti r2,(rr14),PC+3\n djnz r6,PC+2\n dwjnz RR6,PC+3\n jrt PC+2"
 "af5a00af4a009fe2009ff2006a00c606008b00")
check_bytes(endpoints ST9040
 " org 512\n btjf r10.2,PC+3-128\n btjt r10.2,PC+3+127\n cpjfi r2,(rr14),PC+3-128\n cpjti r2,(rr14),PC+3+127\n djnz r6,PC+2-128\n dwjnz RR6,PC+3+127\n jrt PC+2-128"
 "af5a80af4a7f9fe2809ff27f6a80c6067f8b80")
check_bytes(forward ST9040
 " btjf r10.2,target+1000\n btjt r10.2,target+1003\n cpjfi r2,(rr14),target+1006\n cpjti r2,(rr14),target+1009\n djnz r6,target+1011\n dwjnz RR6,target+1014\n jrt target+1016\ntarget equ -997"
 "af5a00af4a009fe2009ff2006a00c606008b00")
check_bytes(wrap ST9040 " org 65534\n jrt 0" "8b00")
check_bytes(noemit ST9040
 " expect 1370,1370,1370,1370,1370\n btjf r10.2,PC+3+128\n cpjfi r2,(rr14),PC+3+128\n djnz r6,PC+2+128\n dwjnz RR6,PC+3+128\n jrt PC+2+128\n endexpect\n nop" "ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/branch-7-128.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "128" detail_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)


check_diagnostic(postinc-shape ST9040 " adc r13,rr4+" 1350 "operand must be (rn)+ or (rrn)+" 3 10)
foreach(reg IN ITEMS bad rr1 rr15 rr16 rr256 rr4294967296)
    check_diagnostic(postinc-${reg} ST9040 " adc r13,(${reg})+" 1445
        "operand must be r0..r15 or rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 11)
    check_diagnostic(predec-${reg} ST9040 " adc r13,-(${reg})" 1445
        "operand must be rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 12)
endforeach()
check_diagnostic(predec-byte ST9040 " adc r13,-(r4)" 1350
    "operand must be rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 12)
check_diagnostic(native-narrow ST9040 " ld r1,r256" 1010 "symbol undefined" 3 8)
check_diagnostic(native-pair-narrow ST9040 " ldw rr2,rr256" 1010 "symbol undefined" 3 10)
check_bytes(update-original ST9040
 " adc r13,(rr4)+\n adc r13,-(rr4)\n adc (rr4)+,r13\n adc -(rr4),r13"
 "b435ddc235ddb434ddc234dd")
check_bytes(update-noemit ST9040
 " expect 1445,1445,1350\n adc r13,(rr1)+\n adc r13,-(rr1)\n adc r13,-(r4)\n endexpect\n nop" "ff")
check_bytes(native-looking-number ST9040
 "r256 equ 1000\nrr256 equ 1000\n ld r1,r256\n ldw rr2,rr256" "c4f103e8e2f203e8")
check_bytes(native-looking-alias ST9040 "r256 equ R10\nrr256 equ RR10\n ld r1,r256\n ldw rr2,rr256" "c4f1000ae2f2010a")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/predec-rr1.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Operand muss rr0, rr2" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 predecrement: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

foreach(op IN ITEMS band bld bor bxor)
    string(LENGTH "${op}" column)
    math(EXPR column "${column} + 3")
    check_diagnostic(${op}-complement ST9040 " ${op} r4.!5,r8.2" 1350
        "requires an uncomplemented bit operand; remove '!'" 3 ${column})
endforeach()
foreach(op IN ITEMS bcpl bres bset btset)
    string(LENGTH "${op}" column)
    math(EXPR column "${column} + 3")
    check_diagnostic(${op}-complement ST9040 " ${op} r4.!5" 1350
        "requires an uncomplemented bit operand; remove '!'" 3 ${column})
endforeach()
foreach(op IN ITEMS btjf btjt)
    check_diagnostic(${op}-complement ST9040 " ${op} r4.!5,PC+3" 1350
        "requires an uncomplemented bit operand; remove '!'" 3 7)
endforeach()
foreach(op IN ITEMS spp srp srp0 srp1)
    string(LENGTH "${op}" column)
    math(EXPR column "${column} + 3")
    check_diagnostic(${op}-immediate ST9040 " ${op} 3" 1350
        "requires an immediate value with # before the expression" 3 ${column})
    check_diagnostic(${op}-primary ST9040 " ${op} #1/0" 1310 "division by 0" 3 "")
endforeach()
check_bytes(bits-original ST9040
 " band r4.5,r8.2\n band r4.5,r8.!2\n bld r4.5,r8.!2\n bld r4.5,r8.2\n bor r4.5,r8.2\n bor r4.5,r8.!2\n bxor r4.5,r8.2\n bxor r4.5,r8.!2\n bcpl r4.5\n bres r4.5\n bset r4.5\n spp #5\n srp #3\n srp0 #3\n srp1 #3"
 "1fb4581fb448f258b4f258a40fb4580fb4486fb4486fb4586fa41fa40fa4c716c718c71cc71d")
check_bytes(fixed-noemit ST9040
 " expect 1350,1350,1350,1350,1350\n band r4.!5,r8.2\n bset r4.!5\n btjf r4.!5,PC+3\n spp 5\n srp 3\n endexpect\n nop" "ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/band-complement.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "nicht komplementierten Bitoperanden" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 complemented bit: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(pair-offset ST9040 " ld r1,r2(rr4)" 1350
    "operand must be rr0, rr2, rr4, rr6, rr8, rr10, rr12, rr14" 3 8)
check_diagnostic(numeric-base ST9040 " ld r1,(100)" 1350
    "indirect base must be a register in register address space" 3 9)
check_diagnostic(base-primary ST9040 " ld r1,(1/0)" 1310 "division by 0" 3 "")
foreach(disp IN ITEMS 1 -1 255)
    check_diagnostic(zero-offset-${disp} ST9040 " ld r1,${disp}(R10)" 1320
        "address form (Rn) requires displacement 0; got ${disp}" 3 8)
endforeach()
check_diagnostic(indirect-descriptor ST9040 "odd reg 511\n ld r1,(odd)" 1350
    "register descriptor 0x1FF is neither a byte in [0,255] nor an even pair in [0x100,0x1ff]" 4 9)
check_diagnostic(direct-descriptor ST9040 "odd reg 511\n ld r1,odd" 1350
    "register descriptor 0x1FF is neither a byte in [0,255] nor an even pair in [0x100,0x1ff]" 4 8)
check_diagnostic(mode-choices ST9040 " adc r1,rr2" 1350
    "operand must be Rn, rn, (rn), (rrn), (rrn)+, -(rrn), d8(rrn), d16(rrn), rrm(rrn), [0..65535], #..." 3 9)
check_bytes(address-original ST9040
 " ld r1,r2\n ld r1,R10\n ld r1,(r2)\n ld r1,(rr2)\n ld r1,5(rr2)\n ld r1,300(rr2)\n ld r1,rr4(rr2)\n ld r1,#10"
 "18d2180ae412b5127ff305d17ff2012cd16052f11c0a")
check_bytes(address-noemit ST9040
 "odd reg 511\n expect 1350,1350,1320,1350,1350,1350\n ld r1,r2(rr4)\n ld r1,(100)\n ld r1,1(R10)\n ld r1,(odd)\n ld r1,odd\n adc r1,rr2\n endexpect\n nop" "ff")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/numeric-base.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Indirekte Basis muss ein Register" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German ST9 register base: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_bytes(failed-displacements-noemit ST9040
 " expect 1310,1310,1310\n ld r1,1/0(rr4)\n pea 1/0(RR4)\n push 1/0(R10)\n endexpect\n nop" "ff")
check_diagnostic(pair-displacement-primary ST9040 " ld r1,1/0(rr4)" 1310 "division by 0" 3 "")
check_diagnostic(general-pair-displacement-primary ST9040 " pea 1/0(RR4)" 1310 "division by 0" 3 "")
check_diagnostic(byte-displacement-primary ST9040 " push 1/0(R10)" 1310 "division by 0" 3 "")
check_bytes(forward-register ST9040
 " push (alias)\n ld r1,alias\nalias reg R10" "f70a180a")
check_bytes(forward-register-direct ST9040 " push alias\nalias reg R10" "660a")
check_bytes(forward-pair-register ST9040 " pea 5(alias)\nalias reg RR4" "8f010405")
check_bytes(forward-displacements ST9040
 " push delta+1000(R10)\n ld r1,delta+1000(rr4)\n pea delta+1005(RR4)\ndelta equ -1000" "f70ab5148f010405")
check_bytes(forward-register-convergence ST9040
 " jrt done\n push (alias)\ndone: nop\nalias reg R10" "8b02f70aff")
check_diagnostic(forward-wrong-base ST9040 " push (alias)\nalias equ 10" 1350
    "indirect base must be a register in register address space" 3 8)
check_diagnostic(missing-base ST9040 " push (missing)" 1010 "symbol undefined" 3 8)
check_diagnostic(missing-direct ST9040 " push missing" 1010 "symbol undefined" 3 7)

check_bytes(descriptor-original ST9040
 " ld r1,R254\n push (R254)\n push R254\n ldw rr2,RR254" "18fef7fe66feeffed2")
check_bytes(byte-descriptor-255 ST9040 " ld r1,R255\n push (R255)\n push R255" "18fff7ff66ff")
check_bytes(alias-descriptor-255 ST9040
 "alias reg 255\n ld r1,alias\n push (alias)\n push alias" "18fff7ff66ff")
check_bytes(forward-descriptor-255 ST9040
 " ld r1,alias\n push (alias)\n push alias\nalias reg 255" "18fff7ff66ff")
check_diagnostic(byte-not-pair ST9040 " ldw rr2,R255" 1350 "operand must be RRn" 3 10)
check_bytes(byte-not-pair-noemit ST9040 " expect 1350\n ldw rr2,R255\n endexpect\n nop" "ff")
