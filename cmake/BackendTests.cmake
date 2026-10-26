# SPDX-License-Identifier: GPL-2.0-only
function(asl_test_available name result)
    cmake_parse_arguments(TEST "" "" "REQUIRES" ${ARGN})
    if(TEST_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unexpected arguments for ${name}: ${TEST_UNPARSED_ARGUMENTS}")
    endif()
    set(missing "")
    foreach(family IN LISTS TEST_REQUIRES)
        if(NOT family IN_LIST ASL_KNOWN_BACKENDS)
            message(FATAL_ERROR "Test ${name} requires unknown backend '${family}'")
        endif()
        if(NOT family IN_LIST ASL_ENABLED_BACKENDS)
            list(APPEND missing "${family}")
        endif()
    endforeach()
    if(missing)
        string(JOIN ", " missing_names ${missing})
        set_property(GLOBAL APPEND PROPERTY ASL_OMITTED_TESTS "${name}: ${missing_names}")
        set(${result} FALSE PARENT_SCOPE)
    else()
        set(${result} TRUE PARENT_SCOPE)
    endif()
endfunction()

macro(asl_regression_test name)
    cmake_parse_arguments(TEST "" "" "REQUIRES" ${ARGN})
    # Validate declarations even when their source fixture is absent.
    foreach(family IN LISTS TEST_REQUIRES)
        if(NOT family IN_LIST ASL_KNOWN_BACKENDS)
            message(FATAL_ERROR "Test ${name} requires unknown backend '${family}'")
        endif()
    endforeach()
    set(ASL_TEST_${name}_REQUIRES "${TEST_REQUIRES}")
endmacro()

function(asl_add_script_test name)
    cmake_parse_arguments(TEST "" "DIRECTORY" "REQUIRES" ${ARGN})
    asl_test_available(${name} can_run REQUIRES ${TEST_REQUIRES})
    if(NOT can_run)
        return()
    endif()
    add_test(NAME ${name} COMMAND ${CMAKE_COMMAND}
        "-DASL_PROGRAM=$<TARGET_FILE:asl>"
        "-DP2BIN_PROGRAM=$<TARGET_FILE:p2bin>"
        "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
        "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/${TEST_DIRECTORY}"
        -P "${CMAKE_SOURCE_DIR}/tests/${name}.cmake"
    )
    set_tests_properties(${name} PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};LC_MESSAGES=en;LC_ALL=C"
    )
endfunction()

function(asl_report_test_selection)
    get_property(omitted GLOBAL PROPERTY ASL_OMITTED_TESTS)
    # Each record is a single line, including multi-family requirements.
    string(REPLACE ";" "\n" report "${omitted}")
    file(WRITE "${CMAKE_BINARY_DIR}/disabled-tests.txt" "${report}")
    get_property(tests DIRECTORY PROPERTY TESTS)
    list(LENGTH tests count)
    message(STATUS "ASL registered ${count} tests; omitted test requirements: ${CMAKE_BINARY_DIR}/disabled-tests.txt")
endfunction()
