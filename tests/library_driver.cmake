# SPDX-License-Identifier: GPL-2.0-only
cmake_minimum_required(VERSION 3.19)
set(source "${TEST_DIR}/source with spaces")
set(includes "${TEST_DIR}/include with spaces")
set(destination "${TEST_DIR}/output")
file(MAKE_DIRECTORY "${source}" "${includes}" "${destination}")
file(WRITE "${includes}/support.inc" "marker equ 1\n")
file(WRITE "${source}/fixture.asm" " include \"support.inc\"\n dc.b VALUE\n")
file(WRITE "${source}/fixture.ori" "a b")
file(WRITE "${source}/asflags" "-cpu 68000 -D 'VALUE=\"a b\"'\n")
set(ENV{ASCMD} "")
string(JOIN " " p2bin_command ${TEST_EMULATOR} "${P2BIN_PROGRAM}")
execute_process(COMMAND ${TEST_EMULATOR} "${TEST_PROGRAM}" 0 "${source}" fixture
    "${includes}" "${ASL_PROGRAM}" "${p2bin_command}" "${destination}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${result}" STREQUAL "0" OR NOT output MATCHES "Assembling in process:")
    message(FATAL_ERROR "In-process driver arguments: ${result}\n${output}${error}")
endif()

file(WRITE "${source}/asflags" "-cpu 68000 -D 'unterminated\n")
execute_process(COMMAND ${TEST_EMULATOR} "${TEST_PROGRAM}" 0 "${source}" fixture
    "${includes}" "${ASL_PROGRAM}" "${p2bin_command}" "${destination}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${result}" STREQUAL "1" OR NOT output MATCHES "Unterminated quote"
        OR EXISTS "${destination}/fixture.bin")
    message(FATAL_ERROR "Malformed driver arguments: ${result}\n${output}${error}")
endif()
