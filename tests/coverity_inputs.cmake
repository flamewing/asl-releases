# Run each fatal parser case in its own process and check for a normal error
# exit, rather than accepting a crash as a successful rejection.
file(MAKE_DIRECTORY "${TEST_DIR}")
function(check_exit expected)
    execute_process(COMMAND ${TEST_EMULATOR} ${ARGN} RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15
        WORKING_DIRECTORY "${TEST_DIR}")
    if(NOT "${result}" STREQUAL "${expected}")
        message(FATAL_ERROR "${ARGN}: expected ${expected}, got ${result}\n${output}\n${error}")
    endif()
endfunction()

function(check_asl expected filename)
    file(TO_NATIVE_PATH "${TEST_DIR}/${filename}" source)
    check_exit(${expected} "${ASL_PROGRAM}" ${ARGN} "${source}")
endfunction()

check_exit(0 "${TEST_PROGRAM}" relocations)
check_exit(0 "${TEST_PROGRAM}" small-path)
check_exit(0 "${TEST_PROGRAM}" catalog valid)
foreach(case negative-length huge-length negative-position countries
        negative-countries empty-table huge-table misaligned-table
        negative-index huge-index index-at-end unterminated long-language truncated)
    check_exit(255 "${TEST_PROGRAM}" catalog "${case}")
endforeach()
foreach(case valid bit-gran nibble-gran)
    check_exit(0 "${TEST_PROGRAM}" record "${case}")
endforeach()
foreach(case zero-gran invalid-bit-gran segment no-segment unknown-cpu)
    check_exit(3 "${TEST_PROGRAM}" record "${case}")
endforeach()

file(MAKE_DIRECTORY "${TEST_DIR}")
file(WRITE "${TEST_DIR}/help.args" "; comment\n  -help\n")
check_exit(0 "${TEST_PROGRAM}" response "@${TEST_DIR}/help.args" help)
check_exit(0 "${TEST_PROGRAM}" response "@${TEST_DIR}/missing.args" error)

# Exercise the bounded register probe and macro diagnostic buffer through ASL.
string(REPEAT "a" 2000 long_value)
file(WRITE "${TEST_DIR}/long-register.asm" " cpu 68HC12\n movb #1,${long_value},1\n")
check_asl(2 long-register.asm)
file(WRITE "${TEST_DIR}/long-irp.asm" " cpu 68HC12\n irp value,${long_value}\n invalid_instruction\n endm\n")
check_asl(2 long-irp.asm)

file(WRITE "${TEST_DIR}/empty.bin" "")
file(WRITE "${TEST_DIR}/data.bin" "test")
check_exit(0 "${TEST_PROGRAM}" chunk "${TEST_DIR}/empty.bin" empty)
check_exit(0 "${TEST_PROGRAM}" chunk "${TEST_DIR}/data.bin" data)
check_exit(0 "${TEST_PROGRAM}" chunk "${TEST_DIR}/missing.bin" error)

file(WRITE "${TEST_DIR}/bad.hex" ":01000000AA00\n")
check_exit(4 "${DAS_PROGRAM}" -hexfile "${TEST_DIR}/bad.hex")
file(WRITE "${TEST_DIR}/truncated.hex" ":01\n")
check_exit(4 "${DAS_PROGRAM}" -hexfile "${TEST_DIR}/truncated.hex")
file(WRITE "${TEST_DIR}/empty.hex" "")
check_exit(0 "${DAS_PROGRAM}" -hexfile "${TEST_DIR}/empty.hex" -help)

file(WRITE "${TEST_DIR}/binclude.asm" " cpu 68HC12\n binclude \"data.bin\",10\n")
check_asl(2 binclude.asm)
file(WRITE "${TEST_DIR}/title.asm" " cpu 68HC12\n title \"Coverity long title regression\"\n nop\n")
check_asl(0 title.asm -L)
file(READ "${TEST_DIR}/title.lst" listing)
if(NOT listing MATCHES "Coverity long title regression")
    message(FATAL_ERROR "Listing title was truncated")
endif()
