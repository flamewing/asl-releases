# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

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

check_bytes(cli-sei-bit MELPS740
    " cli\n bbc 0,a,*+4\n sei\n bbs 0,a,*+4\n cli\n bbc 0,$12,*+4\n sei\n bbs 0,$12,*+4"
    "58ea130178ea030158ea17120078ea071200")
check_bytes(arithmetic-flags MELPS740
    " adc #$12\n sec\n adc #$34\n clc\n adc #$56\n cld\n sbc #$78\n sec\n sbc #$9a\n clc\n sbc #$bc\n cld"
    "6912ea386934ea186956ead8e978ea38e99aea18e9bcead8")
check_bytes(unaffected MELPS740 " cli\n lda #$12\n sei\n nop\n sec\n clc\n cld\n plp"
    "58a91278ea3818d828ea")
check_bytes(other-cpu 6502 " adc #$12\n sec\n sbc #$34\n clc\n cli\n bne *+2"
    "691238e9341858d000")
check_bytes(forward MELPS740 " cli\n bbc 0,a,target\ntarget:\n nop" "58ea1300ea")

