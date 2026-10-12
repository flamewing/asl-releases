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
check_diagnostic(div-source 052001 " div a,b" 1445 "operand must be X" 3 6)
check_diagnostic(div-destination 052001 " div x,a" 1445 "operand must be B" 3 8)
check_diagnostic(move-first 052001 " move x,x,u" 1445 "operand must be Y" 3 7)
check_diagnostic(move-second 052001 " move y,y,u" 1445 "operand must be X" 3 9)
check_diagnostic(move-third 052001 " move y,x,s" 1445 "operand must be U" 3 11)
check_diagnostic(bset-first 052001 " bseta y,u" 1445 "operand must be X" 3 8)
check_diagnostic(bset-second 052001 " bsetd x,s" 1445 "operand must be U" 3 10)
check_diagnostic(dec-register 052001 " dec a,jnz,0" 1445 "operand must be B or X" 3 6)
check_diagnostic(branch-short-upper 052001 " bra 130" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x2)" 3 6)
check_diagnostic(branch-short-lower 052001 " org 127\n bra 0" 1370
 "branch displacement -129 outside [-128,127] (PC base 0x81)" 4 6)
check_diagnostic(branch-long-upper 052001 " lbra 32771" 1370
 "branch displacement 32768 outside [-32768,32767] (PC base 0x3)" 3 7)
check_diagnostic(branch-long-lower 052001 " org 32766\n lbra 0" 1370
 "branch displacement -32769 outside [-32768,32767]" 4 7)
check_diagnostic(dec-target 052001 " dec b,jnz,130" 1370
 "branch displacement 128 outside [-128,127]" 3 12)
check_diagnostic(branch-primary 052001 " bra 1/0" 1310 "division by 0" 3 "")
check_bytes(register-branch-original 052001
 " div x,b\n move y,x,u\n bmove y,x,u\n bseta x,u\n bsetd x,u\n dec b,jnz,*+2\n dec x,jnz,*+2\n bra *+2\n lbra *+3"
 "b5b7b6cfd0ac00ad006000680000")
check_bytes(branch-forward-arithmetic 052001 " bra target+1000\ntarget equ -1000" "60fe")
check_bytes(branch-long-forward-arithmetic 052001 " lbra target+40000\ntarget equ -40000" "68fffd")
check_bytes(dec-forward-arithmetic 052001 " dec x,jnz,target+1000\ntarget equ -1000" "adfe")
check_bytes(register-errors-noemit 052001
 " expect 1445,1445,1445,1370\n div a,b\n move y,x,s\n dec a,jnz,0\n bra 130\n endexpect\n fcb $12" "12")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/div-source.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Operand muss X sein" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Konami register: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

foreach(mode IN ITEMS -x --x x+ x++)
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${mode}")
    check_diagnostic("update-offset-${name}" 052001 " lda 5,${mode}" 1350
        "displacement 5 is not allowed with pointer auto-update; expected zero or an empty displacement" 3 6)
    check_diagnostic("update-primary-${name}" 052001 " lda 1/0,${mode}" 1310
        "division by 0" 3 "")
endforeach()
check_diagnostic(address-mask 052001 " sta #1" 1350
 "operand must be address, base, displacement/A/B/D,base" 3 6)
check_diagnostic(address-components 052001 " lda 1,wrong" 1350
 "operand must be #expression; address, base" 3 6)
check_diagnostic(dec-keyword 052001 " dec x,wrong,0" 1350 "operand must be JNZ" 3 8)
check_diagnostic(indirect-extra 052001 " lda [1,x,2]" 1350
 "at least 3 indirect-address components; expected 1 or 2 inside []" 3 6)
check_diagnostic(indirect-excess 052001 " lda [1,x,2,3]" 1350
 "at least 3 indirect-address components" 3 6)
check_diagnostic(indirect-outside 052001 " lda [1,x],y" 1350
 "an address enclosed in [] must be a single operand" 3 6)
check_bytes(addresses-original 052001
 " lda #1\n lda 1\n lda [1]\n lda x\n lda 1,x\n lda a,y\n lda b,u\n lda d,s\n lda -x\n lda --y\n lda u+\n lda s++\n lda [1,x]\n lda [0,-x]\n sta 1"
 "100112c40112cc01122612240112b012d112e71222123312501261122c01122a3ac401")
check_bytes(update-forward-arithmetic 052001
 " lda offset+1000,x+\noffset equ -1000" "1220")
check_bytes(address-errors-noemit 052001
 " expect 1350,1350,1350,1350,1310\n lda 5,x+\n sta #1\n lda [1,x,2]\n lda [1,x],y\n lda 1/0,-x\n endexpect\n fcb $12" "12")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/update-offset-x_.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Verschiebung 5" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Konami update: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(immediate-mask 052001 " andcc 1" 1350 "operand must be #expression" 3 8)

check_diagnostic(short-index-boundary 052001 " lda <127,x" 1340
 "effective address displacement 127 outside [-128,126]" 3 6)
check_diagnostic(short-pc-boundary 052001 " lda <130,pc" 1340
 "address displacement 127 outside [-128,126] (PC base 0x3)" 3 6)
check_diagnostic(short-page 052001 " assume dpr:$12\n lda <$1300" 1340
 "address 0x1300 uses bank 19; ASSUME DPR=18 permits [0x1200,0x12FF]" 4 6)
check_diagnostic(short-page-disabled 052001 " assume dpr:NOTHING\n lda <$1234" 1340
 "address 0x1234 cannot use direct addressing; ASSUME DPR=NOTHING" 4 6)
check_diagnostic(short-primary 052001 " lda <1/0,pc" 1310 "division by 0" 3 "")
check_bytes(short-pc-forward 052001 " lda <target+1000,pc\ntarget equ -1000" "1274fd")
check_bytes(short-known-invalid-noemit 052001
 " expect 1340,1340\n lda <127,x\n lda <130,pc\n endexpect\n fcb $12" "12")

check_bytes(short-original 052001
 " assume dpr:$12\n lda <$1234\n lda >$1300\n lda <126,x\n lda <-128,x\n lda <*+129,pc\n assume dpr:NOTHING\n lda $1234"
 "12c4341207130012247e12248012747e12071234")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/short-page-disabled.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "ASSUME DPR=NOTHING" detail_pos)
string(FIND "${output}${error}" "direkte Adressierung" mode_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR mode_pos EQUAL -1)
    message(FATAL_ERROR "German Konami DPR: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
