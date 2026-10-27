# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_bytes name cpu body expected)
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 0\n${body}\n")
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${TEST_DIR}/${name}.asm" -q
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: assembly ${result}\n${output}${error}")
    endif()
    execute_process(COMMAND ${TEST_EMULATOR} "${P2BIN_PROGRAM}" "${TEST_DIR}/${name}.p"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${result}\n${output}${error}")
    endif()
    file(READ "${TEST_DIR}/${name}.bin" actual HEX)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: expected ${expected}, got ${actual}")
    endif()
endfunction()

# Exercise every alias with its own data/storage semantics on both backend families.
# Integers and floating-point values must stay little-endian. Byte aliases
# also accept strings; each storage alias advances the PC by its element size.
set(data [=[alias @DOT@reg r0
 @DOT@byte 0x12,"A"
 @DOT@dc8 0x34,"B"
 @DOT@word 0x1234
 @DOT@dc16 0x5678
 @DOT@long 0x12345678
 @DOT@dc24 0x123456
 @DOT@dc32 0x11223344
 @DOT@dc64 0x0102030405060708
 @DOT@float 1.0
 @DOT@df32 2.0
 @DOT@double 1.0
 @DOT@df64 2.0]=])
set(storage [=[base:
 @DOT@ds8 1
 @DOT@blkb 2
 @DOT@space 3
 @DOT@ds16 1
 @DOT@blkw 2
 @DOT@ds32 1
 @DOT@blkf 2
 @DOT@blkl 3
 @DOT@ds64 1
 @DOT@blkd 2
 @DOT@word *-base]=])

foreach(cpu CR16A CR16B CR16C)
    foreach(form plain dotted)
        set(dot "")
        if(form STREQUAL "dotted")
            set(dot ".")
        endif()
        string(REPLACE "@DOT@" "${dot}" data_source "${data}")
        string(REPLACE "@DOT@" "${dot}" storage_source "${storage}")
        check_bytes(data_${cpu}_${form} ${cpu} "${data_source}"
            "1241344234127856785634125634124433221108070605040302010000803f00000040000000000000f03f0000000000000040")
        check_bytes(storage_${cpu}_${form} ${cpu} "${storage_source}" "3c00")
    endforeach()
endforeach()
