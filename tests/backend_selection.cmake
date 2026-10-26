# SPDX-License-Identifier: GPL-2.0-only
cmake_minimum_required(VERSION 3.19)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" -cpu list
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
if(NOT "${result}" STREQUAL "0")
    message(FATAL_ERROR "CPU list failed: ${result}\n${output}${error}")
endif()

# Check the usual reduced build and both cross-backend implementation dependencies.
# Linking support code must not register the support family's processors.
foreach(pair IN ITEMS "NONE:NONE" "M68K:68000" "Z80:Z80" "KONAMI:052001"
        "6809:6809" "CR16:CR16A" "CR16C:CR16C")
    string(REPLACE ":" ";" fields "${pair}")
    list(GET fields 0 family)
    list(GET fields 1 cpu)
    if(output MATCHES "(^|[ \r\n\t])${cpu}([ \r\n\t]|$)")
        set(present TRUE)
    else()
        set(present FALSE)
    endif()
    if(family IN_LIST ENABLED_BACKENDS)
        if(NOT present)
            message(FATAL_ERROR "Enabled family ${family} omits ${cpu}:\n${output}")
        endif()
    elseif(present)
        message(FATAL_ERROR "Disabled family ${family} registers ${cpu}:\n${output}")
    endif()
endforeach()
