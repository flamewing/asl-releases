# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_range name cpu body id value minimum maximum line column)
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 4096\n${body}\n")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    set(diagnostic "${output}${error}")
    set(detail "effective address displacement ${value} outside [${minimum},${maximum}]")
    string(FIND "${diagnostic}" "${detail}" detail_pos)
    string(FIND "${diagnostic}" "(${line}):${column}: error #${id}:" location_pos)
    string(REGEX MATCHALL "error #[0-9]+:" errors "${diagnostic}")
    list(LENGTH errors error_count)
    if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1
            OR location_pos EQUAL -1 OR NOT error_count EQUAL 1)
        message(FATAL_ERROR "${name}: expected one #${id} at ${line}:${column}, ${detail}; got ${result}\n${diagnostic}")
    endif()
endfunction()

# Inferred brief/full displacement sizes, including the PC extension-word base.
check_range(an8-high 68000 " move.w 128(a0,d0.w),d1" 1320 128 -128 127 3 9)
check_range(an8-low 68000 " move.w -129(a0,d0.w),d1" 1315 -129 -128 127 3 9)
check_range(an8-large 68000 " move.w 4294967296(a0,d0.w),d1" 1320 4294967296 -128 127 3 9)
check_range(an16-high 68000 " move.w 32768(a0),d1" 1320 32768 -32768 32767 3 9)
check_range(an16-low 68000 " move.w -32769(a0),d1" 1315 -32769 -32768 32767 3 9)
check_range(pc8-high 68000 " move.w (*+130,pc,d0.w),d1" 1320 128 -128 127 3 10)
check_range(pc8-low 68000 " move.w (*-127,pc,d0.w),d1" 1315 -129 -128 127 3 10)
check_range(pc16-high 68000 " move.w (*+32770,pc),d1" 1320 32768 -32768 32767 3 10)
check_range(pc16-low 68000 " move.w (*-32767,pc),d1" 1315 -32769 -32768 32767 3 10)
check_range(pc-index-forward 68000 " move.w target(pc,d0.w),d1\ntarget equ 4608" 1320 510 -128 127 3 9)

# Explicit word-sized PC displacements and memory-indirect base displacements.
check_range(pc-word-high 68000 " move.w (*+32770.w,pc),d1" 1330 32768 -32768 32767 3 10)
check_range(pc-word-low 68020 " move.w (*-32767.w,pc,d0.w),d1" 1330 -32769 -32768 32767 3 10)
check_range(pc-indirect-high 68020 " move.w ([*+32770.w,pc],d0.w),d1" 1330 32768 -32768 32767 3 11)
check_range(an-indirect-low 68020 " move.w ([-32769.w,a0],d0.w),d1" 1330 -32769 -32768 32767 3 11)

# The same localized detail is available with both embedded and external catalogs.
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/an8-high.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Verschiebung der effektiven Adresse 128" detail_pos)
string(FIND "${output}${error}" "[-128,127]" range_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1 OR range_pos EQUAL -1)
    message(FATAL_ERROR "German displacement detail: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

function(check_bytes name cpu body expected)
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 4096\n${body}\n")
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

set(brief " move.w -128(a0,d0.w),d1\n move.w 127(a0,d0.w),d1\n move.w (*+2-128,pc,d0.w),d1\n move.w (*+2+127,pc,d0.w),d1")
check_bytes(brief-boundaries 68000 "${brief}" "323000803230007f323b0080323b007f")
check_bytes(brief-forward 68000 " move.w lo(a0,d0.w),d1\n move.w hi(a0,d0.w),d1\n move.w pclo(pc,d0.w),d1\n move.w pchi(pc,d0.w),d1\nlo equ -128\nhi equ 127\npclo equ 3978\npchi equ 4237" "323000803230007f323b0080323b007f")
check_bytes(word-boundaries 68000 " move.w -32768(a0),d1\n move.w 32767(a0),d1\n move.w (*+2-32768,pc),d1\n move.w (*+2+32767,pc),d1" "3228800032287fff323a8000323a7fff")
check_bytes(extended-index 68020 " move.w 128(a0,d0.w),d1\n move.w (*+130,pc,d0.w),d1" "323001200080323b01200080")

# Soft ranges retain their warning severity and masked encoding.
check_bytes(soft-range 68000 " move.w 256(a0,d0.w),d1" "32300000" -warnranges -n -x)
string(FIND "${assembly_diagnostic}" "warning #260:" warning_pos)
string(FIND "${assembly_diagnostic}" "effective address displacement 256 outside [-128,127]" detail_pos)
if(warning_pos EQUAL -1 OR detail_pos EQUAL -1)
    message(FATAL_ERROR "Soft displacement range detail: ${assembly_diagnostic}")
endif()
