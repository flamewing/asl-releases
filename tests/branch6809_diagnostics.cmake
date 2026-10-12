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

foreach(cpu IN ITEMS 6809 6809UNDOC 6309)
    check_diagnostic(high-${cpu} ${cpu} " org $1000\n bra *+130" 1370
        "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
    check_diagnostic(low-${cpu} ${cpu} " org $1000\n bne *-127" 1370
        "branch displacement -129 outside [-128,127] (PC base 0x1002)" 4 6)
    check_diagnostic(subroutine-${cpu} ${cpu} " org $1000\n bsr *+200" 1370
        "branch displacement 198 outside [-128,127] (PC base 0x1002)" 4 6)
endforeach()
check_diagnostic(wrap-high 6809 " org $fffe\n bra $80" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x10000)" 4 6)
check_diagnostic(wrap-low 6809 " bne $ff81" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x2)" 3 6)
check_diagnostic(forward-range 6809 " org $1000\n bra target\ntarget equ $1082" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 4 6)
check_diagnostic(undefined 6809 " bne missing" 1010 "symbol undefined" 3 6)

set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/low-6309.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "-129" value_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
string(FIND "${output}${error}" "PC-Basis 0x1002" base_pos)
if(NOT "${result}" STREQUAL "2" OR value_pos EQUAL -1 OR range_pos EQUAL -1 OR base_pos EQUAL -1)
    message(FATAL_ERROR "German 6309: ${result}\n${output}${error}")
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

# Golden bytes captured before this batch; branch opcodes are shared.
foreach(cpu IN ITEMS 6809 6809UNDOC 6309)
    check_bytes(short-${cpu} ${cpu} " org $1000\n bra *+129\n bne *-126\n bsr *+2" "207f26808d00")
    check_bytes(long-${cpu} ${cpu}
        " org $1000\n lbra *+32770\n lbsr $9003\n lbne *+32771\n lbeq 0"
        "167fff177ffd10267fff1027eff2")
    check_bytes(long-low-${cpu} ${cpu} " org $9000\n lbra *-32765\n lbne *-32764" "16800010268000")
endforeach()
check_bytes(wrap-short 6809 " org $fffe\n bra 0" "2000")
check_bytes(wrap-long 6809 " org $fffc\n lbne 0" "10260000")
check_bytes(forward-short 6809 " org $1000\n bne target\ntarget equ $1004" "2602")
check_bytes(forward-long 6809 " org $1000\n lbne target\ntarget equ $1200" "102601fc")


# Forced indexed ranges identify values and PC bases without rejecting provisional
# forward expressions. Direct-page failures identify the selected DPR window.
foreach(cpu IN ITEMS 6809 6809UNDOC 6309)
    check_diagnostic(forced-pc-high-${cpu} ${cpu} " org $1000\n lda <*+131,pc" 1320
        "address displacement 128 outside [-128,127] (PC base 0x1003)" 4 6)
    check_diagnostic(forced-pc-low-${cpu} ${cpu} " org $1000\n lda <*-126,pcr" 1320
        "address displacement -129 outside [-128,127] (PC base 0x1003)" 4 6)
    check_diagnostic(forced-pc-prefix-${cpu} ${cpu} " org $1000\n ldy <*+132,pc" 1320
        "address displacement 128 outside [-128,127] (PC base 0x1004)" 4 6)
    check_diagnostic(forced-five-high-${cpu} ${cpu} " lda <<16,x" 1340
        "effective address displacement 16 outside [-16,15]" 3 6)
    check_diagnostic(forced-five-low-${cpu} ${cpu} " lda <<-17,y" 1340
        "effective address displacement -17 outside [-16,15]" 3 6)
    check_diagnostic(forced-byte-high-${cpu} ${cpu} " lda <128,u" 1340
        "effective address displacement 128 outside [-128,127]" 3 6)
    check_diagnostic(direct-page-high-${cpu} ${cpu} " assume dpr:$12\n lda <$1312" 1340
        "address 0x1312 uses bank 19; ASSUME DPR=18 permits [0x1200,0x12FF]" 4 6)
    check_diagnostic(direct-page-low-${cpu} ${cpu} " assume dpr:$12\n lda <$1112" 1340
        "address 0x1112 uses bank 17; ASSUME DPR=18 permits [0x1200,0x12FF]" 4 6)
    check_diagnostic(direct-page-nothing-${cpu} ${cpu} " assume dpr:nothing\n lda <$1212" 1340
        "address 0x1212 cannot use direct addressing; ASSUME DPR=NOTHING" 4 6)
    check_diagnostic(final-forward-five-${cpu} ${cpu} " lda <<target+1000,x\ntarget equ -984" 1340
        "effective address displacement 16 outside [-16,15]" 3 6)
    check_diagnostic(final-forward-pc-${cpu} ${cpu} " org $1000\n lda <target+1000,pc\ntarget equ $1083-1000" 1320
        "address displacement 128 outside [-128,127] (PC base 0x1003)" 4 6)
    check_diagnostic(forced-pc-undefined-${cpu} ${cpu} " lda <missing,pc" 1010 "symbol undefined" 3 7)
    check_bytes(forced-forward-${cpu} ${cpu}
        " org $1000\n lda <target+1000,pc\ntarget equ $1003-1000\n lda <offset+1000,x\noffset equ -1000\n lda <<five+1000,y\nfive equ -1000\n assume dpr:$12\n lda <address+1000\naddress equ $1212-1000"
        "a68c00a68800a6209612")
    check_bytes(forced-range-errors-noemit-${cpu} ${cpu}
        " expect 1340,1340,1320\n lda <<16,x\n lda <128,y\n lda <*+131,pc\n endexpect\n db $12" "12")
    check_bytes(forced-original-${cpu} ${cpu}
        " org $1000\n lda <<-16,x\n lda <<15,y\n lda <-128,u\n lda <127,s\n lda >128,x\n lda <*+130,pc\n lda <*-125,pcr\n lda >*+131,pc\n assume dpr:$12\n lda <$1212"
        "a610a62fa6c880a6e87fa6890080a68c7fa68c80a68d007f9612")
endforeach()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/direct-page-nothing-6309.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "keine direkte Adressierung verwenden; ASSUME DPR=NOTHING" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German direct page: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
