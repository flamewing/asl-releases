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

# W indexing must stop when displacement evaluation fails. EXPECT exposes any
# instruction bytes that would otherwise survive a rejected expression.
check_bytes(w-evaluation-errors-noemit 6309
    " expect 1320,1315,1310,1320\n lda 65536,w\n lda -32769,w\n lda 1/0,w\n lda [65536,w]\n endexpect\n db $12" "12")
check_diagnostic(w-range-primary 6309 " lda 65536,w" 1320 "65536 > 65535" 3 6)
check_diagnostic(w-division-primary 6309 " lda 1/0,w" 1310 "division by 0" 3 "")
check_bytes(w-original-encodings 6309
    " lda ,w\n lda 1,w\n lda -1,w\n lda [1,w]\n lda [,w]\n lda ,--w\n lda [,w++]\n lda value+1000,w\nvalue equ -999"
    "a68fa6af0001a6afffffa6b00001a690a6efa6d0a6af0001")

# Width errors locate the destination and name both registers. Update errors
# identify the instruction restriction or allowed source/destination patterns.
foreach(cpu IN ITEMS 6809 6809UNDOC 6309)
    check_diagnostic(tfr-width-${cpu} ${cpu} " tfr x,a" 1131
        "register a has width 8 bits; x requires 16 bits" 3 8)
    check_diagnostic(exg-width-${cpu} ${cpu} " exg a,x" 1131
        "register x has width 16 bits; a requires 8 bits" 3 8)
    check_diagnostic(exg-source-update-${cpu} ${cpu} " exg x+,y" 1350
        "register auto-update is not allowed with EXG; use registers without + or -" 3 6)
    check_diagnostic(exg-destination-update-${cpu} ${cpu} " exg x,y+" 1350
        "register auto-update is not allowed with EXG; use registers without + or -" 3 8)
    if(NOT cpu STREQUAL "6309")
        check_diagnostic(tfr-source-update-${cpu} ${cpu} " tfr x+,y+" 1350
            "register auto-update is not allowed with TFR" 3 6)
    endif()
    check_bytes(register-width-errors-noemit-${cpu} ${cpu}
        " expect 1131,1131,1350\n tfr x,a\n exg a,x\n exg x+,y\n endexpect\n db $12" "12")
    if(cpu STREQUAL "6809")
        check_bytes(register-transfer-original-${cpu} ${cpu}
            " tfr x,y\n exg a,b\n tfr z,a\n tfr x,z"
            "1f121e891fd81f1d")
    endif()
    if(cpu STREQUAL "6809UNDOC")
        check_bytes(register-transfer-original-${cpu} ${cpu}
            " tfr x,y\n exg a,b\n tfr z,a\n tfr x,z"
            "1f121e891fd81f1d")
    endif()
    if(cpu STREQUAL "6309")
        check_bytes(register-transfer-original-${cpu} ${cpu}
            " tfr x,y\n exg a,b\n tfr z,a\n tfr x,z\n addr d,x\n subr a,b\n addr z,x\n tfr x+,y+\n tfm x-,y-\n tfm x+,y\n tfm x,y+"
            "1f121e891fd81f1d1030011032891030d1113812113912113a12113b12")
    endif()
endforeach()
check_diagnostic(alu-register-width 6309 " addr x,a" 1131
    "register a has width 8 bits; x requires 16 bits" 3 9)
check_diagnostic(alu-register-width-reverse 6309 " subr a,x" 1131
    "register x has width 16 bits; a requires 8 bits" 3 9)
check_diagnostic(tfm-no-update 6309 " tfm x,y" 1350
    "source register update 0 and destination register update 0 not permitted; allowed pairs (+1,+1), (-1,-1), (+1,0), (0,+1)" 3 8)
check_diagnostic(tfm-mixed-update 6309 " tfm x+,y-" 1350
    "source register update 1 and destination register update -1 not permitted" 3 9)
check_diagnostic(tfm-single-decrement 6309 " tfm x-,y" 1350
    "source register update -1 and destination register update 0 not permitted" 3 9)
check_bytes(tfm-errors-noemit 6309
    " expect 1350,1350,1350,1131\n tfm x,y\n tfm x+,y-\n tfm x-,y\n addr x,a\n endexpect\n db $12" "12")
check_diagnostic(konami-register-update 052001 " tfr x+,y" 1350
    "register auto-update is not allowed with TFR" 3 6)
check_bytes(konami-register-transfer-original 052001 " tfr x,y\n exg a,b" "3f323e10")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/tfm-mixed-update.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Quellregister-Update 1 und Zielregister-Update -1" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German transfer updates: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# Detailed indexed syntax, address masks and CPU requirements; no secondary
# mode error when the displacement evaluator has already reported failure.
foreach(cpu IN ITEMS 6809 6809UNDOC 6309)
    foreach(base IN ITEMS x x+ --y)
        check_diagnostic(missing-component-${base}-${cpu} ${cpu} " lda ${base}" 1110
            "base addressing requires a displacement component; use ,base or enable PLAINBASE ON" 3 6)
    endforeach()
    check_diagnostic(auto-nonzero-${cpu} ${cpu} " lda 7,x+" 1350
        "auto-update requires zero displacement; specified displacement 7" 3 6)
    check_diagnostic(auto-undefined-primary-${cpu} ${cpu} " lda missing,x+" 1010 "symbol undefined" 3 6)
    check_diagnostic(indirect-five-${cpu} ${cpu} " lda [<<1,x]" 1350
        "allowed addressing forms: [<offset,X/Y/U/S]; [>offset,X/Y/U/S]" 3 7)
    check_diagnostic(pc-five-${cpu} ${cpu} " lda <<*+1,pc" 1350
        "allowed addressing forms: <target,PC/PCR; >target,PC/PCR" 3 6)
    check_diagnostic(absolute-five-${cpu} ${cpu} " lda <<$12" 1350
        "allowed addressing forms: <<offset,X/Y/U/S" 3 6)
    check_diagnostic(invalid-base-${cpu} ${cpu} " lda 1,a" 1350
        "allowed addressing forms: #value; <address (ASSUME DPR); >address; offset,X/Y/U/S" 3 8)
    check_diagnostic(store-immediate-${cpu} ${cpu} " sta #1" 1350
        "allowed addressing forms: <address (ASSUME DPR); >address; offset,X/Y/U/S" 3 6)
    check_diagnostic(rmw-immediate-${cpu} ${cpu} " inc #1" 1350
        "allowed addressing forms: <address (ASSUME DPR); >address; offset,X/Y/U/S" 3 6)
    check_diagnostic(lea-absolute-${cpu} ${cpu} " leax $12" 1350
        "allowed addressing forms: offset,X/Y/U/S" 3 7)
    check_diagnostic(unclosed-index-${cpu} ${cpu} " lda [1,x" 1300
        "unclosed bracket '['; expected ']' before end of input" 3 6)
    check_diagnostic(mismatched-index-${cpu} ${cpu} " lda (1,x]" 1300
        "mismatched closing bracket ']'; expected ')'" 3 10)
    if(NOT cpu STREQUAL "6309")
        check_diagnostic(w-pre-requires-${cpu} ${cpu} " lda ,--w" 1505 "supported by 6309 and above" 3 7)
        check_diagnostic(w-post-requires-${cpu} ${cpu} " lda ,w++" 1505 "supported by 6309 and above" 3 7)
    else()
        check_diagnostic(w-missing-component ${cpu} " lda w" 1110 "use ,base or enable PLAINBASE ON" 3 6)
    endif()
    if(NOT cpu STREQUAL "6809UNDOC")
        check_diagnostic(hcf-cpu-${cpu} ${cpu} " hcf" 1500 "supported only by 6809UNDOC" 3 2)
    endif()
    check_bytes(auto-forward-zero-${cpu} ${cpu}
        " org $1000\n lda value+1000,x+\nvalue equ -1000" "a680")
    check_bytes(address-mode-errors-noemit-${cpu} ${cpu}
        " expect 1110,1350,1350,1350,1350,1300\n lda x\n lda 7,x+\n lda [<<1,x]\n lda <<*+1,pc\n sta #1\n lda [1,x\n endexpect\n db $12" "12")
    if(cpu STREQUAL "6809")
        check_bytes(address-forms-original-${cpu} ${cpu}
            " lda #1\n sta $12\n lda $1234\n lda ,x\n lda a,y\n lda -16,u\n lda 16,s\n lda 128,x\n lda ,--x\n lda ,y++\n lda [1,u]\n lda [$1234]\n leax *+3,pc\n plainbase on\n lda x\n lda x+\n plainbase off"
            "86019712b61234a684a6a6a650a6e810a6890080a683a6a1a6d801a69f1234308c00a684a680")
    endif()
    if(cpu STREQUAL "6809UNDOC")
        check_bytes(address-forms-original-${cpu} ${cpu}
            " lda #1\n sta $12\n lda $1234\n lda ,x\n lda a,y\n lda -16,u\n lda 16,s\n lda 128,x\n lda ,--x\n lda ,y++\n lda [1,u]\n lda [$1234]\n leax *+3,pc\n plainbase on\n lda x\n lda x+\n plainbase off"
            "86019712b61234a684a6a6a650a6e810a6890080a683a6a1a6d801a69f1234308c00a684a680")
    endif()
    if(cpu STREQUAL "6309")
        check_bytes(address-forms-original-${cpu} ${cpu}
            " lda #1\n sta $12\n lda $1234\n lda ,x\n lda a,y\n lda -16,u\n lda 16,s\n lda 128,x\n lda ,--x\n lda ,y++\n lda [1,u]\n lda [$1234]\n leax *+3,pc\n plainbase on\n lda x\n lda x+\n plainbase off\n lda e,x\n lda f,y\n lda w,u\n lda 1,w"
            "86019712b61234a684a6a6a650a6e810a6890080a683a6a1a6d801a69f1234308c00a684a680a687a6aaa6cea6af0001")
    endif()
endforeach()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/auto-nonzero-6309.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Auto-Update erfordert eine Verschiebung von null; angegebene Verschiebung 7" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German auto displacement: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
