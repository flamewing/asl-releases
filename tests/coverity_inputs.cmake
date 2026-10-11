# Run each fatal parser case in its own process and check for a normal error
# exit, rather than accepting a crash as a successful rejection.
function(check_exit expected)
    execute_process(COMMAND ${TEST_EMULATOR} ${ARGN} RESULT_VARIABLE result
        OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "${expected}")
        message(FATAL_ERROR "${ARGN}: expected ${expected}, got ${result}\n${output}\n${error}")
    endif()
endfunction()

check_exit(0 "${TEST_PROGRAM}" relocations)
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
file(WRITE "${TEST_DIR}/long-register.asm" " cpu 6812\n movb #1,${long_value},1\n")
check_exit(2 "${ASL_PROGRAM}" "${TEST_DIR}/long-register.asm")
file(WRITE "${TEST_DIR}/long-irp.asm" " cpu 6812\n irp value,${long_value}\n invalid_instruction\n endm\n")
check_exit(2 "${ASL_PROGRAM}" "${TEST_DIR}/long-irp.asm")
