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

check_diagnostic(short-high 6502 " org $1000\n bne *+130" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(short-low 6502 " org $1000\n bne *-127" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(forced-short 65CE02 " org $1000\n bne <*+130" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(bit-high 65C02 " org $1000\n bbr0 $12,*+131" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1003)" 4 11)
check_diagnostic(bit-low W65C02S " org $1000\n bbs7 $12,*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 11)
check_diagnostic(bar-high 65C19 " org $1000\n bar $1234,#$45,*+133" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1005)" 4 17)
check_diagnostic(bas-low 65C19 " org $1000\n bas $1234,#$45,*-124" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1005)" 4 17)
check_diagnostic(melps-acc MELPS740 " org $1000\n bbc 0,a,*+130" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 10)
check_diagnostic(melps-memory MELPS740 " org $1000\n bbs 0,$12,*-126" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1003)" 4 12)
check_diagnostic(melps-cli MELPS740 " org $1000\n cli\n bbc 0,a,*+131" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1004)" 5 10)
check_diagnostic(melps-sei MELPS740 " org $1000\n sei\n bbs 0,$12,*-125" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1005)" 5 12)
check_diagnostic(wrap-high 6502 " org $fffe\n bne $80" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x10000)" 4 6)
check_diagnostic(wrap-low 6502 " bne $ff81" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x2)" 3 6)
check_diagnostic(forward-range 6502 " org $1000\n bne target\ntarget equ $1082" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(undefined-target 6502 " bne missing" 1010 "symbol undefined" 3 6)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/melps-cli.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "128" value_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
string(FIND "${output}${error}" "PC-Basis 0x1004" base_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German MELPS740: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

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

# Golden bytes captured with the preceding batch's executable.
check_bytes(short-endpoints 6502 " org $1000\n bne *+129\n bne *-126" "d07fd080")
check_bytes(ce02-selection 65CE02
    " org $1000\n bne *+129\n bne *+200\n bne <*+20\n bne >*+20\n bsr *+200"
    "d07fd3c500d012d3110063c500")
check_bytes(bit-endpoints 65C02 " org $1000\n bbr0 $12,*+130\n bbs7 $12,*-125" "0f127fff1280")
check_bytes(bar-endpoints 65C19 " org $1000\n bar $1234,#$45,*+132\n bas $1234,#$45,*-123"
    "e23412457ff234124580")
check_bytes(melps-endpoints MELPS740
    " org $1000\n bbc 0,a,*+129\n bbs 0,$12,*-125\n cli\n bbc 0,a,*+130\n sei\n bbs 0,$12,*-124"
    "137f07128058ea137f78ea071280")
check_bytes(wrap-end 6502 " org $fffe\n bne 0" "d000")
check_bytes(wrap-start 6502 " bne $ff82" "d080")
check_bytes(forward-short 6502 " org $1000\n bne target\ntarget equ $1004" "d002")
check_bytes(forward-long 65CE02 " org $1000\n bne target\ntarget equ $10c8" "d3c500")


# Byte addresses promoted to an absolute-only instruction need two address bytes.
foreach(cpu IN ITEMS 6502 65SC02 65C02 65CE02 W65C02S 65C19 MELPS740 HUC6280 6502UNDOC)
    check_bytes(promote-${cpu} ${cpu}
        " jmp <$12\n jsr <$34\n lda #1\n jmp <$56" "4c1200203400a9014c5600")
    check_bytes(rejected-not-promoted-${cpu} ${cpu}
        " expect 1350,1350\n jmp #1\n jsr #1\n endexpect\n db $12" "12")
endforeach()
check_bytes(promote-index-x 6502 " ldy <$12,x" "b412")
check_bytes(promote-indirect 65C19 " jpi (<$12)" "0c1200")

check_bytes(promote-index-only 65CE02 " ldz <$12,x" "bb1200")

check_bytes(promote-index-y 6502 " lda <$12,y" "b91200")
