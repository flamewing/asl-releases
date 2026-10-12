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
check_diagnostic(upper-half-register ATMEGA8 " ldi r15,1" 1445
    "register R15 is not allowed here; expected R16..R31" 3 6)
check_diagnostic(multiply-register ATMEGA8 " fmul r24,r16" 1445
    "register R24 is not allowed here; expected R16..R23" 3 7)
check_diagnostic(word-even-register ATMEGA8 " movw r1,r2" 1445
    "expected R0, R2, R4, R6" 3 7)
check_diagnostic(upper-even-register ATMEGA8 " adiw r25,1" 1445
    "expected R24, R26, R28, R30" 3 7)
check_diagnostic(register-alias-subset ATMEGA8 "alias reg r15\n ldi alias,1" 1445
    "register R15 is not allowed here; expected R16..R31" 4 6)
check_diagnostic(register-undefined-primary ATMEGA8 " ldi missing,1" 1445
    "invalid register" 3 6)
check_diagnostic(register-target-primary ATMEGA8
    " cpu 68020\nalias reg d0\n cpu ATMEGA8\n ldi alias,1" 1148
    "register symbol for different target" 6 6)
check_bytes(register-original-subsets ATMEGA8
    " ldi r16,1\n muls r16,r17\n fmul r16,r17\n adiw r26,1\n movw r4,r2"
    "01e00102090311962101")
check_bytes(register-forward-subsets ATMEGA8
    " ldi upper,1\n muls upper,r17\n fmul upper,r17\n adiw pair,1\n movw even,r2\nupper reg r16\npair reg r26\neven reg r4"
    "01e00102090311962101")
check_diagnostic(register-forward-invalid ATMEGA8 " ldi alias,1\nalias reg r15" 1445
    "register R15 is not allowed here; expected R16..R31" 3 6)
check_bytes(register-subset-errors-noemit ATMEGA8
    " expect 1445,1445,1445\n ldi r15,1\n fmul r24,r16\n adiw r25,1\n endexpect\n dw 0x1234" "3412")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/upper-half-register.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Register R15 ist hier nicht erlaubt" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German AVR subset: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(register-forward-divzero ATMEGA8 " ldi alias,1/0\nalias reg r16" 1310
    "division by 0" 3 "")

foreach(pointer IN ITEMS W Z++ X+1)
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${pointer}")
    check_diagnostic("ld-pointer-${name}" ATMEGA8 " ld r0,${pointer}" 1350
        "expected X, X+, -X, Y, Y+, -Y, Z, Z+ or -Z" 3 8)
    check_diagnostic("st-pointer-${name}" ATMEGA8 " st ${pointer},r0" 1350
        "unrecognized data-pointer syntax" 3 5)
endforeach()
foreach(pointer IN ITEMS X X+ -X Y Y+ -Y Z+ -Z)
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${pointer}")
    check_diagnostic("small-ld-${name}" AT90S1200 " ld r0,${pointer}" 1350
        "this CPU accepts only Z without auto-update as its data pointer" 3 8)
    check_diagnostic("small-st-${name}" AT90S1200 " st ${pointer},r0" 1350
        "only Z without auto-update" 3 5)
endforeach()
check_diagnostic(ldd-pointer ATMEGA8 " ldd r0,X+1" 1350
    "expected Y+q or Z+q with displacement q in [0,63]" 3 9)
check_diagnostic(std-pointer ATMEGA8 " std W+1,r0" 1350
    "expected Y+q or Z+q with displacement q in [0,63]" 3 6)
check_diagnostic(ldd-displacement-primary ATMEGA8 " ldd r0,Y+64" 1320
    "64" 3 9)
check_diagnostic(ldd-expression-primary ATMEGA8 " ldd r0,Y+1/0" 1310
    "division by 0" 3 "")
foreach(base IN ITEMS 26 28 30)
    if(base EQUAL 26)
        set(pointer X)
    elseif(base EQUAL 28)
        set(pointer Y)
    else()
        set(pointer Z)
    endif()
    math(EXPR high "${base}+1")
    foreach(reg IN ITEMS ${base} ${high})
        foreach(mode IN ITEMS "${pointer}+" "-${pointer}")
            string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${mode}")
            check_diagnostic("ld-overlap-${reg}-${name}" ATMEGA8 " ld r${reg},${mode}" 140
                "data register R${reg} overlaps ${pointer} (R${high}:R${base}), which is modified by ${mode}" 3 5 0)
            check_diagnostic("st-overlap-${reg}-${name}" ATMEGA8 " st ${mode},r${reg}" 140
                "data register R${reg} overlaps ${pointer} (R${high}:R${base}), which is modified by ${mode}" 3 8 0)
        endforeach()
    endforeach()
endforeach()
check_bytes(data-pointers-original ATMEGA8
 " ld r0,X\n ld r1,X+\n ld r2,-X\n ld r3,Y\n ld r4,Y+\n ld r5,-Y\n ld r6,Z\n ld r7,Z+\n ld r8,-Z\n st X,r0\n st X+,r1\n st -X,r2\n st Y,r3\n st Y+,r4\n st -Y,r5\n st Z,r6\n st Z+,r7\n st -Z,r8\n ldd r0,Y+0\n ldd r1,Z+63\n std Y+63,r2\n std Z+0,r3"
 "0c901d902e90388049905a906080719082900c921d922e92388249925a92608271928292088017ac2fae3082")
check_bytes(data-overlap-original ATMEGA8
 " ld r26,X+\n ld r27,-X\n ld r28,Y+\n ld r29,-Y\n ld r30,Z+\n ld r31,-Z\n st X+,r26\n st -X,r27\n st Y+,r28\n st -Y,r29\n st Z+,r30\n st -Z,r31"
 "ad91be91c991da91e191f291ad93be93c993da93e193f293" -n -x)
string(REGEX MATCHALL "warning #140:" warnings "${assembly_diagnostic}")
list(LENGTH warnings count)
if(NOT count EQUAL 12)
    message(FATAL_ERROR "expected twelve overlap warnings: ${assembly_diagnostic}")
endif()
check_bytes(small-pointer-valid AT90S1200 " ld r0,Z\n st Z,r0" "00800082")
check_bytes(pointer-errors-noemit AT90S1200
 " expect 1350,1350\n ld r0,X\n st Z+,r0\n endexpect\n dw 0x1234" "3412")

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/ldd-pointer.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Erwartet wird Y+q oder Z+q" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German AVR pointer: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(core-classic AT90S1200 " adiw r24,1" 1500
    "selected CPU AT90S1200; supported core classes: classic AT90S, ATtiny, ATtiny 16K, ATmega" 3 2)
check_diagnostic(core-tiny AT90S8515 " movw r0,r2" 1500
    "supported core classes: ATtiny, ATtiny 16K, ATmega" 3 2)
check_diagnostic(core-tiny16k ATTINY13 " jmp 0" 1500
    "supported core classes: ATtiny 16K, ATmega" 3 2)
check_diagnostic(core-mega ATTINY13 " mul r0,r1" 1500
    "supported core classes: ATmega" 3 2)
check_diagnostic(core-disjoint AT90S1200 " ijmp" 1500
    "supported core classes: minimal ATtiny, classic AT90S, ATtiny, ATtiny 16K, ATmega" 3 2)
check_diagnostic(core-before-register AT90S1200 " push missing" 1500
    "selected CPU AT90S1200" 3 2)
check_diagnostic(bit-count-empty ATMEGA8 " sbi" 1110 "expected between 1 and 2" 3 2)
check_diagnostic(bit-count-extra ATMEGA8 " sbi 1,2,3" 1110 "got 3" 3 2)
check_diagnostic(bit-definition-count ATMEGA8 "flag bit" 1110 "expected between 1 and 2" 3 6)
check_bytes(core-original ATTINY167 " movw r0,r2\n jmp 0\n lpm r0,Z" "01010c9400000490")
check_bytes(core-disjoint-valid ATTINY10 " ijmp\n push r16" "09940f93")
check_bytes(bit-count-valid ATMEGA8 " sbi 1,2\n sbi 1.2" "0a9a0a9a")
check_bytes(core-count-noemit AT90S1200
 " expect 1500,1110\n push r0\n sbi\n endexpect\n dw 0x1234" "3412")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/core-classic.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "unterst" detail_pos)
string(FIND "${output}${error}" "klassischer AT90S" core_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR core_pos EQUAL -1)
    message(FATAL_ERROR "German AVR core: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(branch-short-upper ATMEGA8 " breq 65" 1370
 "branch word displacement 64 outside [-64,63] (next-instruction word address 0x1)" 3 7)
check_diagnostic(branch-bit-upper ATMEGA8 " brbs 0,65" 1370
 "branch word displacement 64 outside [-64,63]" 3 9)
check_diagnostic(branch-long-upper ATMEGA8 " rjmp 2049" 1370
 "branch word displacement 2048 outside [-2048,2047]" 3 7)
check_diagnostic(branch-call-lower ATMEGA8 " org 2048\n rcall 0" 1370
 "branch word displacement -2049 outside [-2048,2047] (next-instruction word address 0x801)" 4 8)
check_diagnostic(branch-short-lower ATMEGA8 " org 64\n brne 0" 1370
 "branch word displacement -65 outside [-64,63]" 4 7)
check_diagnostic(branch-byte-upper ATMEGA8:codesegsize=0 " breq 130" 1370
 "branch word displacement 64 outside [-64,63] (next-instruction word address 0x1)" 3 7)
check_diagnostic(branch-byte-unaligned ATMEGA8:codesegsize=0 " rjmp 3" 1325
 "operand address 0x3 must be aligned to 2 bytes" 3 7)
check_diagnostic(instruction-byte-unaligned ATMEGA8:codesegsize=0 " org 1\n nop" 180
 "instruction address 0x1 must be aligned to 2 bytes" 4 2 0)
check_diagnostic(branch-primary ATMEGA8 " breq 1/0" 1310 "division by 0" 3 "")
check_bytes(branch-original ATMEGA8
 " breq 64\n brbs 0,65\n rjmp 2050\n rcall 0" "f9f1f8f1ffc7fcdf")
check_bytes(branch-byte-original ATMEGA8:codesegsize=0
 " breq 128\n brbs 0,130\n rjmp 4100\n rcall 0" "f9f1f8f1ffc7fcdf")
check_bytes(branch-forward ATMEGA8 " breq target\n brbs 0,target\n rjmp target\n rcall target\ntarget: nop"
 "19f010f001c000d00000")
check_bytes(branch-noemit ATMEGA8
 " expect 1370,1370,1370\n breq 65\n brbs 0,65\n rjmp 2049\n endexpect\n dw 0x1234" "3412")
check_bytes(instruction-padding ATMEGA8:codesegsize=0 " padding on\n org 1\n nop" "000000")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/branch-short-upper.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung in Worten 64" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German AVR branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

foreach(mode IN ITEMS word byte)
    if(mode STREQUAL word)
        set(cpu ATMEGA8)
        set(last 4095)
    else()
        set(cpu ATMEGA8:codesegsize=0)
        set(last 8190)
    endif()
    foreach(instruction IN ITEMS breq rjmp rcall)
        if(instruction STREQUAL breq)
            set(forward "01f0")
            set(backward "f1f3")
        elseif(instruction STREQUAL rjmp)
            set(forward "00c0")
            set(backward "fecf")
        else()
            set(forward "00d0")
            set(backward "fedf")
        endif()
        check_bytes("wrap-${mode}-${instruction}-end" ${cpu}
            " wrapmode on\n org ${last}\n ${instruction} 0" "${forward}")
        check_bytes("wrap-${mode}-${instruction}-start" ${cpu}
            " wrapmode on\n ${instruction} ${last}" "${backward}")
    endforeach()
    check_bytes("wrap-${mode}-bit-end" ${cpu}
        " wrapmode on\n org ${last}\n brbs 0,0" "00f0")
    check_bytes("wrap-${mode}-bit-start" ${cpu}
        " wrapmode on\n brbc 0,${last}" "f0f7")
endforeach()
check_bytes(wrap-small-byte AT90S1200:codesegsize=0
    " wrapmode on\n org 1022\n breq 0" "01f0")
check_bytes(wrap-byte-forward-label ATMEGA8:codesegsize=0
    " wrapmode on\n org 8190\n breq target\ntarget equ 0" "01f0")
check_diagnostic(wrap-off-byte ATMEGA8:codesegsize=0
    " org 8190\n breq 0" 1370 "branch word displacement -4096 outside [-64,63]" 4 7)
