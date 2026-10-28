# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(assemble name body expected_result)
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu 32060\n org 0\n${body}")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -q -n -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "${expected_result}")
        message(FATAL_ERROR "${name}: expected exit ${expected_result}, got ${result}\n${output}${error}")
    endif()
    set(diagnostic "${output}${error}" PARENT_SCOPE)
endfunction()

function(read_binary name result)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.p" object)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.bin" binary)
    execute_process(COMMAND ${TEST_EMULATOR} "${P2BIN_PROGRAM}" "${object}" "${binary}"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${status}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${status}\n${output}${error}")
    endif()
    file(READ "${TEST_DIR}/${name}.bin" bytes HEX)
    set(${result} "${bytes}" PARENT_SCOPE)
endfunction()

foreach(bank IN ITEMS a b)
    if(bank STREQUAL "a")
        set(side 1)
    else()
        set(side 2)
    endif()
    # Automatic selection must use the remaining L/S/D units on the same side.
    assemble(auto_${bank}
        " add ${bank}1,${bank}2,${bank}3\n|| add ${bank}4,${bank}5,${bank}6\n|| add ${bank}7,${bank}8,${bank}9\n nop\n" 0)
    assemble(explicit_${bank}
        " add.l${side} ${bank}1,${bank}2,${bank}3\n|| add.s${side} ${bank}4,${bank}5,${bank}6\n|| add.d${side} ${bank}7,${bank}8,${bank}9\n nop\n" 0)
    read_binary(auto_${bank} actual)
    read_binary(explicit_${bank} expected)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${bank}: automatic unit selection differs from explicit selection")
    endif()

    # Once all candidate units are occupied, preserve the register side so the
    # packet checker diagnoses the L-unit conflict rather than an attribute.
    foreach(dest IN ITEMS 0 1)
        assemble(conflict_${bank}${dest}
            " add.l${side} ${bank}1,${bank}2,${bank}3\n|| add.s${side} ${bank}4,${bank}5,${bank}6\n|| add.d${side} ${bank}7,${bank}8,${bank}9\n|| add ${bank}10,${bank}11,${bank}${dest}\n nop\n" 2)
        string(REGEX MATCHALL "error #[0-9]+:" errors "${diagnostic}")
        list(LENGTH errors count)
        string(FIND "${diagnostic}" "L${side}" unit_pos)
        if(NOT count EQUAL 1 OR NOT diagnostic MATCHES "error #2001:" OR unit_pos EQUAL -1)
            message(FATAL_ERROR "${bank}${dest}: expected one L${side} unit conflict\n${diagnostic}")
        endif()
    endforeach()
endforeach()
