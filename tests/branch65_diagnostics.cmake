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

# Address mapping failures expose the physical/CPU address and assumptions.
check_diagnostic(huc-unmapped HUC6280 " lda $10000" 110
    "physical address 0x10000 (8-KiB page 8) is not selected by ASSUME MPR0=0, MPR1=1" 3 6 0)
check_diagnostic(huc-short HUC6280 " lda <$2000" 1320
    "physical address 0x2000 maps to CPU address 0x2000 outside [0,255]" 3 7)
check_diagnostic(ce-base-page 65CE02 " assume b:$12\n lda <$1312" 1320
    "address 0x1312 uses bank 19; ASSUME B=18 permits [0x1200,0x12FF]" 4 7)
check_bytes(huc-mapped HUC6280 " assume mpr0:8\n lda $10012" "a512")
check_bytes(huc-unmapped-bytes HUC6280 " lda $10000" "a500")
check_bytes(huc-forward HUC6280 " lda target+$10000\ntarget equ -$ff80" "a580")
if(assembly_diagnostic MATCHES "warning #110:")
    message(FATAL_ERROR "finally mapped HUC address warned: ${assembly_diagnostic}")
endif()
check_bytes(ce-base-valid 65CE02 " assume b:$12\n lda <$1212" "a512")

check_diagnostic(huc-nothing HUC6280 " assume mpr0:nothing\n lda 0" 1350
    "ASSUME MPR0 does not support NOTHING" 3 14)
check_diagnostic(huc-primary-error HUC6280 " lda missing" 1010 "symbol undefined" 3 6)
check_bytes(huc-forced-mapped HUC6280 " assume mpr0:8\n lda <$10080" "a580")
check_bytes(ce-forward-base 65CE02
    " assume b:$12\n lda <target+1000\ntarget equ $1212-1000" "a512")

# Bracket diagnostics locate the current/expected character before decoding.
foreach(cpu IN ITEMS 6502 65SC02 65C02 65CE02 W65C02S 65C19 MELPS740 HUC6280 6502UNDOC)
    check_diagnostic(unclosed-${cpu} ${cpu} " lda (12,x" 1300
        "unclosed bracket '('; expected ')' before end of input" 3 6)
    check_diagnostic(mismatch-${cpu} ${cpu} " lda (12,x]" 1300
        "mismatched closing bracket ']'; expected ')'" 3 11)
    check_diagnostic(unexpected-${cpu} ${cpu} " lda 12,x)" 1300
        "unexpected closing bracket ')'; no closing bracket expected" 3 10)
    if(NOT cpu STREQUAL "65C19")
        check_bytes(short-indirect-${cpu} ${cpu} " lda (1),y" "b101")
    endif()
    check_bytes(quoted-brackets-${cpu} ${cpu} " lda #'('\n lda #']'" "a928a95d")
    check_bytes(bracket-noemit-${cpu} ${cpu}
        " expect 1300,1300\n lda (12,x\n lda (12,x]\n endexpect\n db $12" "12")
endforeach()
check_bytes(stack-indirect-valid 65CE02 " lda (1,sp),y" "e201")

check_diagnostic(short-indirect-unsupported 65C19 " lda (1),y" 1500 "instruction not supported on 65C19" 3 "")

# Special-page and indirect-pointer placement diagnostics retain their IDs.
check_diagnostic(special-page-low MELPS740 " assume sp:$ff\n jsr \\$fe12" 1315
    "address 0xFE12 uses bank 254; ASSUME SP=255 permits [0xFF00,0xFFFF]" 4 6)
check_diagnostic(special-page-high MELPS740 " assume sp:$12\n jsr \\$1312" 1315
    "address 0x1312 uses bank 19; ASSUME SP=18 permits [0x1200,0x12FF]" 4 6)
check_bytes(special-page-valid MELPS740 " assume sp:$ff\n jsr \\$ff12" "2212")
check_bytes(special-page-forward MELPS740
    " assume sp:$ff\n jsr \\target+1\ntarget equ $ff11" "2212")
foreach(cpu IN ITEMS 6502 65SC02 65CE02 W65C02S 65C19 MELPS740 HUC6280 6502UNDOC)
    check_diagnostic(pointer-boundary-${cpu} ${cpu} " jmp ($12ff)" 1900
        "indirect pointer address 0x12FF ends in 0xff" 3 6)
    check_bytes(pointer-before-${cpu} ${cpu} " jmp ($12fe)" "6cfe12")
    check_bytes(pointer-forward-${cpu} ${cpu}
        " jmp (target+$ff)\ntarget equ $1101" "6c0012")
    check_bytes(pointer-noemit-${cpu} ${cpu}
        " expect 1900\n jmp ($12ff)\n endexpect\n db $12" "12")
endforeach()
check_bytes(pointer-c02-exception 65C02 " jmp ($12ff)" "6cff12")

# HuC6280 TST inserts its mask without dropping the final address byte.
check_bytes(huc-tst-all-forms HUC6280
    " tst #$12,$34\n tst #$56,$78,x\n tst #$9a,$12bc\n tst #$de,$1234,x\n db $78"
    "831234a35678939abc12b3de341278")
check_bytes(huc-tst-forward-mask HUC6280
    " tst #mask,$34\n tst #$56,target,x\nmask equ $12\ntarget equ $1234" "831234b3563412")
check_bytes(huc-tst-label-offset HUC6280
    " tst #1,$1234\n lda #after\nafter: db $12" "93013412a90612")
check_bytes(melps-tst-unchanged MELPS740 " tst $12" "6412")
check_diagnostic(huc-tst-primary HUC6280 " tst #missing,$34" 1010 "symbol undefined" 3 7)
