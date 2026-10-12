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
check_diagnostic(jr-upper Z8601 " jr 130" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x2)" 3 5)
check_diagnostic(jr-conditional Z8601 " jr nz,130" 1370
 "branch displacement 128 outside [-128,127]" 3 8)
check_diagnostic(djnz-upper Z8601 " djnz r3,130" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x2)" 3 10)
check_diagnostic(cpijne-upper Z88C00 " cpijne r3,@r5,131" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 16)
check_diagnostic(btjnz-upper eZ8 " btjnz 5,r7,131" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 13)
check_diagnostic(btjrt-upper Z88C00 " btjrt 131,r5,#7" 1370
 "branch displacement 128 outside [-128,127] (PC base 0x3)" 3 8)
check_diagnostic(jr-lower Z8601 " org 127\n jr 0" 1370
 "branch displacement -129 outside [-128,127] (PC base 0x81)" 4 5)
check_diagnostic(btjrt-lower Z88C00 " org 126\n btjrt 0,r5,#7" 1370
 "branch displacement -129 outside [-128,127]" 4 8)
check_diagnostic(jr-primary Z8601 " jr 1/0" 1310 "division by 0" 3 "")
check_bytes(branch-classic-original Z8601 " jr $+2\n djnz r3,$+2" "8b003a00")
check_bytes(branch-super-original Z88C00
 " cpijne r3,@r5,$+3\n cpije r3,@r5,$+3\n btjrf $+3,r5,#7\n btjrt $+3,r5,#7"
 "d25300c25300375e00375f00")
check_bytes(branch-encore-original eZ8 " btjnz 5,r7,$+3\n btjz 5,@r7,$+3" "f6d700f75700")
check_bytes(jr-forward Z8601 " jr target+1000\ntarget equ -1000" "8bfe")
check_bytes(djnz-forward Z8601 " djnz r3,target+1000\ntarget equ -1000" "3afe")
check_bytes(cpijne-forward Z88C00 " cpijne r3,@r5,target+1000\ntarget equ -1000" "d253fd")
check_bytes(btjnz-forward eZ8 " btjnz 5,r7,target+1000\ntarget equ -1000" "f6d7fd")
check_bytes(btjrt-forward Z88C00 " btjrt target+1000,r5,#7\ntarget equ -1000" "375ffd")
check_bytes(branch-errors-noemit Z8601
 " expect 1370,1370\n jr 130\n djnz r3,130\n endexpect\n db 18" "12")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/jr-upper.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Sprungverschiebung 128" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Z8 branch: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(core-disjoint Z8601 " halt" 1500
    "supported core classes: Z8 CMOS, Z8 Encore/eZ8" 3 2)
check_diagnostic(core-super Z8601 " enter" 1500
    "supported core classes: Super8" 3 2)
check_diagnostic(core-encore Z8601 " brk" 1500
    "supported core classes: Z8 Encore/eZ8" 3 2)
check_diagnostic(core-classic eZ8 " wdh" 1500
    "supported core classes: Z8 NMOS, Z8 CMOS" 3 2)
check_diagnostic(core-before-operands Z8601 " cpijne missing,@r5,0" 1500
    "supported core classes: Super8" 3 2)
check_diagnostic(bit-count-empty eZ8 "flag defbit" 1110
    "expected between 1 and 2 arguments but got 0" 3 6)
check_diagnostic(bit-count-extra eZ8 "flag defbit r0,1,2" 1110 "got 3" 3 6)
check_bytes(core-cmos-original Z86C03 " halt\n stop" "7f6f")
check_bytes(core-super-original Z88C00 " enter\n exit\n bitc r3,7" "1f2f573e")
check_bytes(core-encore-original eZ8 " brk\n halt\n stop" "007f6f")
check_bytes(core-count-noemit Z8601
    " expect 1500,1110\n halt\nflag defbit\n endexpect\n db 18" "12")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/core-disjoint.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Kernklassen: Z8 CMOS, Z8 Encore/eZ8" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Z8 core: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_diagnostic(working-index-window Z8601 " assume rp:32\n ld r0,1(00h)" 1445
 "address 0x0 outside working-register windows: RP=0x20:[0x20,0x2F]" 4 10)
check_diagnostic(forced-working-window Z8601 " assume rp:32\n ld r0,!7" 110
 "address 0x7 outside working-register windows: RP=0x20:[0x20,0x2F]" 4 8 0)
check_diagnostic(forced-super-window Z88C00 " assume rp0:32,rp1:64\n ld r0,!7" 110
 "RP0=0x20:[0x20,0x27]; RP1=0x40:[0x40,0x47]" 4 8 0)
check_diagnostic(forced-encore-window eZ8 " assume rp:18\n ld r0,!7" 110
 "RP=0x12:[0x210,0x21F]" 4 8 0)
check_diagnostic(forced-working-disabled Z8601 " assume rp:NOTHING\n ld r0,!7" 110
 "address 0x7 outside working-register windows: RP=NOTHING" 4 8 0)
check_diagnostic(register-target-primary Z8601
 " cpu 68020\nalias reg d0\n cpu Z8601\n ld r0,alias" 1148
 "register symbol for different target" 6 8)
check_bytes(forced-working-index Z8601
 " assume rp:32\n ld r0,!0\n ld r0,!7\n ld r0,!15" "08e008e708ef" -n -x)
string(REGEX MATCHALL "warning #110:" warnings "${assembly_diagnostic}")
list(LENGTH warnings count)
if(NOT count EQUAL 3)
    message(FATAL_ERROR "expected three forced-work warnings: ${assembly_diagnostic}")
endif()
check_bytes(forced-super-bank Z88C00
 " assume rp0:32,rp1:64\n ld r0,!64\n ld r0,!7" "08c808c7" -n -x)
check_bytes(forced-unknown-no-warning Z8601
 " assume rp:32\n ld r0,!address+1000\naddress equ -968" "08e0" -n -x)
if(assembly_diagnostic MATCHES "warning #110:")
    message(FATAL_ERROR "resolved working address emitted a premature warning: ${assembly_diagnostic}")
endif()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/forced-working-window.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Arbeitsregisterfenster: RP=0x20" detail_pos)
if(NOT "${result}" STREQUAL "0" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German Z8 window: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

check_bytes(index-working-original Z8601 " assume rp:32\n ld r0,1(20h)" "c70001")
check_bytes(index-working-forward Z8601 " assume rp:32\n ld r0,1(index)\nindex equ 32" "c70001")
check_bytes(index-pair-original Z88C00 " assume rp:32\n ldc r0,1(20h)" "e70001")
check_bytes(index-pair-forward Z88C00 " assume rp:32\n ldc r0,1(index)\nindex equ 32" "e70001")
check_diagnostic(index-working-forward-invalid Z8601 " assume rp:32\n ld r0,1(index)\nindex equ 0" 1445
 "address 0x0 outside working-register windows" 4 10)
check_bytes(index-window-noemit Z8601
 " assume rp:32\n expect 1445\n ld r0,1(00h)\n endexpect\n db 18" "12")
