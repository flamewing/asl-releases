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

check_diagnostic(short-high 68HC16 " org $9000\n bra *+134" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x9006); must be a multiple of 2" 4 6)
check_diagnostic(short-low 68HC16 " org $9000\n bne *-124" 1370
    "branch displacement -130 outside [-128,127] (PC base 0x9006); must be a multiple of 2" 4 6)
check_diagnostic(long-high 68HC16 " org $9000\n lbra *+32774" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x9006); must be a multiple of 2" 4 7)
check_diagnostic(long-low 68HC16 " org $9000\n lbne *-32764" 1370
    "branch displacement -32770 outside [-32768,32767] (PC base 0x9006); must be a multiple of 2" 4 7)
check_diagnostic(long-subroutine 68HC16 " org $9000\n lbsr *+32774" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x9006); must be a multiple of 2" 4 7)
check_diagnostic(subroutine 68HC16 " org $9000\n bsr *+134" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x9006); must be a multiple of 2" 4 6)
check_diagnostic(bit-8 68HC16 " org $9000\n brset $55,x,#2,*+32774" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x9006)" 4 17)
check_diagnostic(bit-16 68HC16 " org $9000\n brclr $5aa5,x,#2,*-32764" 1370
    "branch displacement -32770 outside [-32768,32767] (PC base 0x9006)" 4 19)
check_diagnostic(bit-abs 68HC16 " org $9000\n brset $1234,#2,*+32774" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x9006)" 4 17)
check_diagnostic(odd-bra 68HC16 " org $9000\n bra *+7" 1325
    "branch displacement 1 must be a multiple of 2 (PC base 0x9006)" 4 6)
check_diagnostic(undefined-bra 68HC16 " bra missing" 1010 "symbol undefined" 3 6)
check_diagnostic(odd-lbra 68HC16 " org $9000\n lbra *+7" 1325
    "branch displacement 1 must be a multiple of 2 (PC base 0x9006)" 4 7)
check_diagnostic(undefined-lbra 68HC16 " lbra missing" 1010 "symbol undefined" 3 7)
check_diagnostic(odd-lbsr 68HC16 " org $9000\n lbsr *+7" 1325
    "branch displacement 1 must be a multiple of 2 (PC base 0x9006)" 4 7)
check_diagnostic(undefined-lbsr 68HC16 " lbsr missing" 1010 "symbol undefined" 3 7)
check_diagnostic(odd-bsr 68HC16 " org $9000\n bsr *+7" 1325
    "branch displacement 1 must be a multiple of 2 (PC base 0x9006)" 4 6)
check_diagnostic(undefined-bsr 68HC16 " bsr missing" 1010 "symbol undefined" 3 6)
check_diagnostic(forward-high 68HC16 " org $9000\n bra target\ntarget equ $9086" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x9006)" 4 6)
check_diagnostic(undefined-bit 68HC16 " brset $55,x,#2,missing" 1010 "symbol undefined" 3 17)
check_diagnostic(target-limit 68HC16 " bra $1000000" 1320 "16777216" 3 6)
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/short-high.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "PC-Basis 0x9006" base_pos)
string(FIND "${output}${error}" "Vielfaches von 2" alignment_pos)
if(NOT "${result}" STREQUAL "2" OR base_pos EQUAL -1 OR alignment_pos EQUAL -1)
    message(FATAL_ERROR "German HC16: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)
# Captured before this batch. Bit branches deliberately permit odd distances.
check_bytes(short 68HC16 " org $1000\n bra *+132\n bne *-122\n bsr *+132" "b07eb680367e")
check_bytes(long 68HC16 " org $9000\n lbra *+32772\n lbne *-32762\n lbsr *+32772" "37807ffe3786800027f97ffe")
check_bytes(bits 68HC16 " org $9000\n brset $55,x,#2,*+132\n brclr $55,x,#2,*+133\n brset $5aa5,x,#2,*+32773\n brclr $1234,#2,*-32762" "8b02557e0a020055007f0b025aa57fff3a0212348000")
check_bytes(forward 68HC16 " org $1000\n bra target\ntarget equ $1008" "b002")

# Provisional range/alignment failures must wait until forward targets resolve.
foreach(item IN ITEMS "bra|b002" "bsr|3602" "lbra|37800002" "lbsr|27f90002")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 mnemonic)
    list(GET fields 1 bytes)
    foreach(offset IN ITEMS 1001 100000)
        check_bytes(forward-${mnemonic}-${offset} 68HC16
            " org $1000\n ${mnemonic} target+${offset}\ntarget equ $1008-${offset}" "${bytes}")
    endforeach()
endforeach()
foreach(item IN ITEMS "brset $55,x,#2|8b025502" "brset $5555,x,#2|0b0255550002" "brset $1234,#2|3b0212340002")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 instruction)
    list(GET fields 1 bytes)
    string(MAKE_C_IDENTIFIER "${instruction}" tag)
    check_bytes(forward-${tag} 68HC16
        " org $1000\n ${instruction},target+100000\ntarget equ $1008-100000" "${bytes}")
endforeach()

# Bank warnings retain fallback bytes and wait for unresolved expressions.
check_diagnostic(bank-window 68HC16 " ldaa $10000" 110
    "address 0x10000 uses bank 1; ASSUME EK=0 permits [0x0,0xFFFF]" 3 7 0)
check_diagnostic(bank-nothing 68HC16 " assume ek:nothing\n ldaa $1234" 110
    "ASSUME EK=NOTHING; no address window is known for address 0x1234" 4 7 0)
check_bytes(unmapped-bank-bytes 68HC16 " ldaa $10000" "17750000")
check_bytes(mapped-bank-bytes 68HC16 " assume ek:1\n ldaa $10000" "17750000")
check_bytes(forward-bank-bytes 68HC16
    " ldaa target+$10000\ntarget equ -$f000" "17751000")
if(assembly_diagnostic MATCHES "warning #110:")
    message(FATAL_ERROR "finally mapped CPU16 address warned: ${assembly_diagnostic}")
endif()

# CPU16 register sets and instruction-specific addressing alternatives.
check_diagnostic(index-choices 68HC16 " ldaa 1,sp" 1445 "X, Y or Z" 3 9)
check_diagnostic(stack-choices 68HC16 " pshm d,sp" 1445 "D, E, X, Y, Z, K or CCR" 3 9)
check_diagnostic(stack-pull-choices 68HC16 " pulm a" 1445 "D, E, X, Y, Z, K or CCR" 3 7)
check_diagnostic(store-forms 68HC16 " staa #1" 1350 "absolute address" 3 7)
check_diagnostic(immediate-forms 68HC16 " andp $1234" 1350 "#value" 3 7)
check_diagnostic(move-pair 68HC16 " movb 1,y,$1234" 1350
    "offset,X,address or address,offset,X" 3 2)
check_bytes(forms-valid 68HC16
    " ldaa #1\n ldaa 1,x\n ldaa >256,y\n ldaa e,z\n ldaa $1234\n pshm d,e,x,y,z,k,ccr\n movb 1,x,$1234\n movb $1234,1,x\n movw $1234,$5678"
    "7501450117550100276517751234347f300112343201123437ff12345678")
check_bytes(forms-error-emission 68HC16
    " expect 1445,1350,1350\n ldaa 1,sp\n staa #1\n movb 1,y,$1234\n endexpect\n db $12" "12")
