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

check_diagnostic(ez80-page EZ80F91 " assume adl:0,mbase:0\n ld hl,(123456h)" 110
    "address 0x123456 has page byte 0x12, assumed MBASE is 0x0; non-24-bit addressing requires the same 64-KiB page" 4 8 0)
check_diagnostic(ez80-page-adl EZ80F91 " assume adl:1,mbase:1\n ld.sis hl,(123456h)" 110
    "page byte 0x12, assumed MBASE is 0x1" 4 12 0)
check_diagnostic(z180-data Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n ld a,(5000h)" 2270
    "physical address 0x5000 in DATA cannot be translated to a CPU address using Z180 MMU" 4 7)
check_diagnostic(z180-code Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n jp 5000h" 2270
    "physical address 0x5000 in CODE cannot be translated to a CPU address using Z180 MMU" 4 5)
check_diagnostic(z180-pc Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n org 5000h\n jr 5002h" 2271
    "physical address 0x5002 in CODE cannot be translated to a CPU address using Z180 MMU" 5 "")
check_diagnostic(z180-pc-forward Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n org 5000h\n jr target\ntarget equ 5002h" 2271
    "physical address 0x5002 in CODE cannot be translated" 5 "")
check_diagnostic(z280-data Z280 " assume mmumcr:8000h\n ld a,(123h)" 2270
    "physical address 0x123 in DATA cannot be translated to a CPU address using Z280 MMU in user mode" 4 7)
check_diagnostic(z280-separated Z280 " assume mmumcr:0c000h,upd0:0008h\n jp 123h" 2270
    "physical address 0x123 in CODE cannot be translated to a CPU address using Z280 MMU in user mode (separate CODE/DATA mapping)" 4 5)
check_diagnostic(z280-supervisor Z280 " supmode on\n assume mmumcr:800h\n ld a,(123h)" 2270
    "physical address 0x123 in DATA cannot be translated to a CPU address using Z280 MMU in supervisor mode" 5 7)

check_diagnostic(msp-symbolic-high MSP430X " org 10000h\n mov 18002h,r4" 1320
    "non-extended symbolic displacement 32768 (wrapped 20-bit value 0x8000, PC base 0x10002) outside [-32768,32767]" 4 6)
check_diagnostic(msp-symbolic-low MSP430X " org 10000h\n mov 8001h,r4" 1320
    "non-extended symbolic displacement -32769 (wrapped 20-bit value 0xF7FFF, PC base 0x10002) outside [-32768,32767]" 4 6)
check_diagnostic(msp-pc-boundary MSP430X " org 0fffeh\n mov 18000h,r4" 1320
    "PC base 0x10000) outside [-32768,32767]" 4 6)
check_diagnostic(msp-low-pc-target MSP430X " mov 10000h,r4" 1320 "65536 > 65535" 3 6)
check_diagnostic(msp-calla-high MSP430X " calla 32768(r4)" 1320
    "CALLA indexed displacement 32768 (wrapped 20-bit value 0x8000) outside [-32768,32767]" 3 8)
check_diagnostic(msp-calla-low MSP430X " calla -32769(r4)" 1320
    "CALLA indexed displacement -32769 (wrapped 20-bit value 0xF7FFF) outside [-32768,32767]" 3 8)
check_diagnostic(msp-undefined MSP430X " org 10000h\n mov missing,r4" 1010 "symbol undefined" 4 6)

check_bytes(msp-boundaries MSP430X
    " org 10000h\n mov 8002h,r4\n mov 18005h,r4\n calla -32768(r4)\n calla 32767(r4)\n calla 0f8000h(r4)\n movx 90000h,r4"
    "144000801440ff7f541300805413ff7f54130080c01b1440e8ff")
check_bytes(msp-wrap MSP430X " org 0ffff0h\n mov 7ff1h,r4\n mov 0f7ff6h,r4" "1440ff7f14400080")
check_bytes(msp-low-page MSP430X " mov 0ffffh,r4\n mov 0,r4" "1440fdff1440faff")
check_bytes(msp-forward MSP430X " org 10000h\n mov target,r4\ntarget equ 10006h" "14400400")
check_bytes(ez80-widths EZ80F91
    " assume adl:0,mbase:12h\n ld hl,(123456h)\n ld.lil hl,(123456h)\n assume adl:1\n ld hl,(123456h)"
    "2a56345b2a5634122a563412")
check_bytes(z180-mapping Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n ld a,(20000h)\n ld a,(10000h)" "3a00403a00d0")
check_bytes(z280-mapping Z280 " assume mmumcr:8000h,upd0:0008h,upd1:1008h\n ld a,(123h)\n ld a,(100123h)" "3a23013a2311")
check_bytes(z180-forward Z180 " assume cbar:0d4h,cbr:03h,bbr:1ch\n ld a,(target)\ntarget equ 20000h" "3a0040")
check_bytes(ez80-forward EZ80F91 " assume adl:0,mbase:12h\n ld hl,(target)\ntarget equ 123456h" "2a5634")

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "ez80-page|MBASE ist 0x0" "z180-data|Z180-MMU" "z280-supervisor|Supervisormodus" "msp-symbolic-low|-32769" "msp-calla-high|CALLA-Indexverschiebung 32768")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 name)
    list(GET fields 1 detail)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    string(FIND "${output}${error}" "${detail}" detail_pos)
    if((NOT "${result}" STREQUAL "2" AND NOT "${result}" STREQUAL "0") OR detail_pos EQUAL -1)
        message(FATAL_ERROR "German ${name}: ${result}\n${output}${error}")
    endif()
endforeach()
set(ENV{LC_MESSAGES} en)
