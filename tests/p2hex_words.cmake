# SPDX-License-Identifier: GPL-2.0-only
cmake_minimum_required(VERSION 3.19)
file(MAKE_DIRECTORY "${TEST_DIR}")
set(ENV{P2HEXCMD} "")
file(TO_NATIVE_PATH "${TEST_DIR}/words.p" input)
# The existing object fixture contains the repeating byte sequence 00..FF.
execute_process(COMMAND ${TEST_EMULATOR} "${TEST_PROGRAM}" object 8193 "${input}"
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "word fixture: ${result}\n${output}${error}")
endif()

function(check_hex name format range expected)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.hex" target)
    execute_process(COMMAND ${TEST_EMULATOR} "${P2HEX_PROGRAM}" "${input}" "${target}"
        -F "${format}" -r "${range}" ${ARGN}
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${result}\n${output}${error}")
    endif()
    file(READ "${TEST_DIR}/${name}.hex" actual)
    string(REPLACE "\r\n" "\n" actual "${actual}")
    if(format STREQUAL "DSK")
        # Only the header embeds the platform-specific output filename.
        string(FIND "${actual}" "\n" header_end)
        if(header_end EQUAL -1)
            message(FATAL_ERROR "${name}: missing DSK header")
        endif()
        math(EXPR data_start "${header_end} + 1")
        string(SUBSTRING "${actual}" ${data_start} -1 actual)
    endif()
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: expected\n${expected}got\n${actual}")
    endif()
endfunction()

check_hex(atmel_odd ATMEL 0-4 "000000:0100\n000002:0302\n000004:0004\n")
check_hex(atmel_one ATMEL 4-4 "000004:0004\n")
check_hex(atmel_one_high ATMEL 255-255 "0000FF:00FF\n")
check_hex(atmel_high ATMEL 128-131 "000080:8180\n000082:8382\n")
check_hex(atmel_boundary ATMEL 253-257 "0000FD:FEFD\n0000FF:00FF\n000101:0001\n")
# Multiple DSK lines exercise both word decoding and the per-line checksum.
check_hex(dsk_words DSK 0-7 "90000M0100M030270402F\n90004M0504M070670C0AF\n:\n" -l 4)
