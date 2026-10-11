# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_diagnostic name cpu body id detail line column)
    set(expected_result 2)
    set(kind error)
    if(ARGC GREATER 7)
        set(expected_result "${ARGV7}")
        set(kind warning)
    endif()
    file(WRITE "${TEST_DIR}/${name}.asm" " cpu ${cpu}\n org 4096\n${body}\n")
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    set(diagnostic "${output}${error}")
    string(FIND "${diagnostic}" "${detail}" detail_pos)
    string(FIND "${diagnostic}" "(${line}):${column}: ${kind} #${id}:" location_pos)
    string(REGEX MATCHALL "${kind} #[0-9]+:" errors "${diagnostic}")
    list(LENGTH errors error_count)
    if(NOT "${result}" STREQUAL "${expected_result}" OR detail_pos EQUAL -1
            OR location_pos EQUAL -1 OR NOT error_count EQUAL 1)
        message(FATAL_ERROR "${name}: expected one #${id} at ${line}:${column}, ${detail}; got ${result}\n${diagnostic}")
    endif()
endfunction()

function(check_range name cpu body id value minimum maximum line column)
    check_diagnostic("${name}" "${cpu}" "${body}" "${id}"
        "effective address displacement ${value} outside [${minimum},${maximum}]" "${line}" "${column}")
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

# Branch diagnostics use the actual extension-word base, not the instruction end.
check_diagnostic(branch-short-high 68000 " bra.s *+130" 1370
    "branch displacement 128 outside [-128,127] (PC base 0x1002)" 3 8)
check_diagnostic(branch-short-low 68000 " bra.s *-127" 1370
    "branch displacement -129 outside [-128,127] (PC base 0x1002)" 3 8)
check_diagnostic(branch-word-high 68000 " bra.w *+32770" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1002)" 3 8)
check_diagnostic(branch-auto-high 68000 " bra *+32770" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1002)" 3 6)
check_diagnostic(branch-forward 68000 " bra.s target\ntarget equ 4296" 1370
    "branch displacement 198 outside [-128,127] (PC base 0x1002)" 3 8)
check_diagnostic(db-branch-low 68000 " dbra d0,*-32767" 1370
    "branch displacement -32769 outside [-32768,32767] (PC base 0x1002)" 3 10)
check_diagnostic(fpu-branch 68020 " fpu on\n fbeq.w *+32770" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1002)" 4 9)
check_diagnostic(fpu-db-branch 68020 " fpu on\n fdbeq d0,*+32772" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1004)" 4 11)
check_diagnostic(pmmu-branch 68020 " pmmu on\n supmode on\n pbbs.w *+32770" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1002)" 5 9)
check_diagnostic(pmmu-db-branch 68020 " pmmu on\n supmode on\n pdbbs d0,*+32772" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1004)" 5 11)
check_diagnostic(coldfire-branch MCF5204 " cp0bcbusy *+32770" 1370
    "branch displacement 32768 outside [-32768,32767] (PC base 0x1002)" 3 12)

# Reserved zero BSR uses existing error 1371; a true overflow retains 1370.
check_diagnostic(bsr-zero 68000 " bsr.s *+2" 1371
    "short BSR cannot encode displacement 0; use a word displacement or automatic size" 3 8)
check_diagnostic(bsr-zero-forward 68000 " bsr.s target\ntarget equ 4098" 1371
    "short BSR cannot encode displacement 0" 3 8)

# Alignment is still a warning; instruction and operand locations differ.
check_diagnostic(operand-odd 68000 " move.w 4097,d0" 180
    "operand address 0x1001 must be aligned to 2 bytes" 3 9 0)
check_diagnostic(instruction-odd 68020 " padding off\n org 4097\n nop" 180
    "instruction address 0x1001 must be aligned to 2 bytes" 5 2 0)

# Failed integer/register/expression evaluation produces one primary error.
foreach(case IN ITEMS "0:1315" "9:1320" "16:1320" "-1:1315")
    string(REPLACE ":" ";" fields "${case}")
    list(GET fields 0 count)
    list(GET fields 1 id)
    check_diagnostic("shift-${count}" 68000 " lsl.w #${count},d0" "${id}"
        "immediate shift count ${count} outside [1,8]" 3 9)
endforeach()
check_diagnostic(shift-undefined 68000 " lsl.w #missing,d0" 1010 "symbol undefined" 3 9)
check_diagnostic(shift-register-invalid 68000 " lsl.w a0,d0" 1350 "addressing mode not allowed here" 3 8)
check_diagnostic(memory-shift-size 68000 " lsl.b (a0)" 1130 "memory-form shifts require word size (.W)" 3 6)
check_diagnostic(memory-shift-undefined 68000 " lsl.w #missing,(a0)" 1010 "symbol undefined" 3 9)
check_diagnostic(memory-shift-overflow 68000 " lsl.w #256,(a0)" 1320 "256 > 255" 3 9)
check_diagnostic(memory-shift-count 68000 " lsl.w #2,(a0)" 1391 "memory-form shifts require count 1 (got 2)" 3 8)
check_diagnostic(memory-shift-count-negative 68000 " lsl.w #-1,(a0)" 1391 "memory-form shifts require count 1 (got -1)" 3 8)

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

# Accepted boundary/relaxed encodings remain unchanged.
check_bytes(branch-boundaries 68000
    " bra.s *+129\n bra.s *-126\n bra.w *+32769\n bra.w *-32766"
    "607f608060007fff60008000")
check_bytes(branch-boundaries-forward 68000
    " bra.s hi\n bra.s lo\nhi equ 4225\nlo equ 3972" "607f6080")
check_bytes(bsr-zero-word 68000 " bsr *+2\n bsr.w *+2" "6100000061000000")
check_bytes(branch-zero-nop 68000 " bra.s *+2" "4e71" -n)
string(FIND "${assembly_diagnostic}" "warning #60:" warning_pos)
if(warning_pos EQUAL -1)
    message(FATAL_ERROR "Zero BRA still requires the NOP warning: ${assembly_diagnostic}")
endif()
check_bytes(bsr-next-label 68000 " bsr target\ntarget: nop" "610000024e71")
check_bytes(branch-long 68020 " bra.x *+32770" "60ff00008000")
check_bytes(shifts-known 68000 " lsl.w #1,d0\n lsl.w #8,d0\n lsl.w #1,(a0)" "e348e148e3d0")
check_bytes(shifts-forward 68000
    " lsl.w #lo,d0\n lsl.w #hi,d0\n lsl.w #lo,(a0)\nlo equ 1\nhi equ 8" "e348e148e3d0")
check_bytes(operand-odd-allowed 68020 " move.w 4097,d0" "30381001")
check_bytes(instruction-padding 68000 " padding on\n org 4097\n nop" "004e71")
check_bytes(shift-soft-range 68000 " lsl.w #17,d0" "e348" -warnranges -n -x)
string(FIND "${assembly_diagnostic}" "warning #260:" warning_pos)
string(FIND "${assembly_diagnostic}" "immediate shift count 17 outside [1,8]" detail_pos)
if(warning_pos EQUAL -1 OR detail_pos EQUAL -1)
    message(FATAL_ERROR "Soft shift range detail: ${assembly_diagnostic}")
endif()

# Verify both catalog translations without relying on the host's locale.
set(ENV{LC_MESSAGES} de)
foreach(case IN ITEMS "branch-short-high:Sprungverschiebung 128"
        "bsr-zero:Kurzes BSR kann Verschiebung 0 nicht kodieren"
        "operand-odd:Operandenadresse 0x1001"
        "instruction-odd:Befehlsadresse 0x1001"
        "shift-9:Unmittelbare Schiebeanzahl 9"
        "memory-shift-size:Schiebebefehle mit Speicheroperand"
        "memory-shift-count:Schiebebefehle mit Speicheroperand erfordern Anzahl 1 (erhielt 2)")
    string(REPLACE ":" ";" fields "${case}")
    list(GET fields 0 name)
    list(GET fields 1 detail)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    string(FIND "${output}${error}" "${detail}" detail_pos)
    if((NOT "${result}" STREQUAL "2" AND NOT "${result}" STREQUAL "0") OR detail_pos EQUAL -1)
        message(FATAL_ERROR "German ${name}: ${result}\n${output}${error}")
    endif()
endforeach()
set(ENV{LC_MESSAGES} en)
