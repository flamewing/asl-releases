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
    set(expected_location "(${line}):")
    if(NOT "${column}" STREQUAL "")
        string(APPEND expected_location "${column}:")
    endif()
    string(FIND "${diagnostic}" "${expected_location} ${kind} #${id}:" location_pos)
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

# Address syntax and capability details retain their established numeric IDs.
check_diagnostic(index-width 68000 " move.w (a0,d0.b),d1" 1350
    "invalid index-register width b; expected .W or .L" 3 13)
check_diagnostic(index-scale 68020 " move.w (a0,d0.w*3),d1" 1350
    "invalid index scale 3; expected 1, 2, 4 or 8" 3 13)
check_diagnostic(coldfire-scale MCF5204 " move.w (a0,d0.l*8),d1" 1350
    "ColdFire does not support index scale 8" 3 13)
check_diagnostic(unsupported-scale 68000 " move.w (a0,d0.w*2),d1" 1505
    "index scale 2 is unsupported by the selected CPU" 3 13)
check_diagnostic(unsupported-pc-scale 68000 " move.w (*+2,pc,d0.w*4),d1" 1505
    "index scale 4 is unsupported by the selected CPU" 3 17)
check_diagnostic(full-extension 68000 " move.w (0.w,a0,d0.w),d1" 1505
    "requires a full extension word" 3 9)
check_diagnostic(memory-indirect 68000 " move.w ([0,a0],d0.w),d1" 1505
    "memory-indirect addressing is unsupported" 3 9)
check_diagnostic(displacement-width 68020 " move.w (0.b,a0),d1" 1350
    "invalid displacement width b; expected .W or .L" 3 10)
check_diagnostic(duplicate-displacement 68020 " move.w 1(2,a0),d1" 1350
    "duplicate displacement in effective address" 3 11)
check_diagnostic(duplicate-index 68020 " move.w (d0.w,d1.w),d1" 1350
    "only one index register" 3 15)
check_diagnostic(too-many-components 68020 " move.w (a0,d0.w,d1.w),d1" 1350
    "too many register or indirect components" 3 18)
check_diagnostic(four-components 68020 " move.w (a0,d0.w,d1.w,d2.w),d1" 1350
    "too many register or indirect components" 3 23)
check_diagnostic(indirect-duplicate-displacement 68020 " move.w ([1,2,a0],d0.w),d1" 1350
    "duplicate displacement in effective address" 3 13)
check_diagnostic(indirect-duplicate-base 68020 " move.w ([0,pc,pc],d0.w),d1" 1350
    "only one base register" 3 16)
check_diagnostic(indirect-duplicate-index 68020 " move.w ([a0,d0.w,d1.w]),d1" 1350
    "only one index register" 3 19)
check_diagnostic(pair-first-half 68020 " cas2.w a0:d1,d2:d3,(a0):(a1)" 1760
    "this register-pair half must be a data register D0..D7" 3 9)
check_diagnostic(pair-second-half 68020 " cas2.w d0:a0,d2:d3,(a0):(a1)" 1760
    "this register-pair half must be a data register D0..D7" 3 12)
check_diagnostic(pair-colon 68020 " divsl.l d0,d1" 1760
    "register pair requires a colon" 3 13)
check_diagnostic(pair-parentheses 68020 " cas2.w d0:d1,d2:d3,(a0):a1" 1760
    "each indirect register-pair half must be enclosed in parentheses" 3 26)
check_diagnostic(pair-indirect-half 68020 " cas2.w d0:d1,d2:d3,(a0):(42)" 1760
    "this register-pair half must be a data or address register" 3 27)
check_diagnostic(pair-evaluator-error 68020 " fpu on\nbad reg fp0\n cas2.w d0:bad,d2:d3,(a0):(a1)" 1130 "invalid operand size" 5 12)
check_diagnostic(index-evaluator-error 68020 " fpu on\nbad reg fp0\n move.w (a0,bad.w),d1" 1130 "invalid operand size" 5 13)

# Golden bytes captured from the previous build: normalization, aliases, scales,
# full extensions, and data/address registers in indirect CAS2 pairs.
check_bytes(address-normalization-and-pairs 68020
    "idx reg d0\nbase reg a0\n move.w (d0.w,a0),d1\n move.w (a0,a1),d1\n move.w (a0,pc),d1\n move.w (base,idx.w*4),d1\n move.w ([0,a0,a1]),d1\n cas2.w d0:d1,d2:d3,(a0):(a1)\n cas2.w d0:d1,d2:d3,(d4):(a1)\n cas2.w idx:d1,d2:d3,(base):(a1)\n muls.l d2,d0:d1\n divsl.l d2,d0:d1"
    "3230000032309000323b8000323004003230912100000cfc808090c10cfc408090c10cfc808090c14c021c004c421800")

set(ENV{LC_MESSAGES} de)
foreach(item IN ITEMS "index-width|Indexregisterbreite b" "index-scale|Indexskalierung 3" "pair-second-half|D0..D7" "duplicate-displacement|Doppelte Verschiebung")
    string(REPLACE "|" ";" fields "${item}")
    list(GET fields 0 name)
    list(GET fields 1 detail)
    file(TO_NATIVE_PATH "${TEST_DIR}/${name}.asm" source)
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    string(FIND "${output}${error}" "${detail}" detail_pos)
    if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
        message(FATAL_ERROR "German ${name}: ${result}\n${output}${error}")
    endif()
endforeach()
set(ENV{LC_MESSAGES} en)

check_diagnostic(mul-pair-half 68020 " muls.l d2,d0:a0" 1760
    "this register-pair half must be a data register D0..D7" 3 15)
check_diagnostic(div-pair-half 68020 " divsl.l d2,d0:a0" 1760
    "this register-pair half must be a data register D0..D7" 3 16)
check_diagnostic(indirect-index-width 68020 " move.w ([a0,d0.b]),d1" 1350
    "invalid index-register width b; expected .W or .L" 3 14)

# Remaining register candidates: MAC word halves/CPU-specific accumulators and
# the data-register destination of long multiply/divide. Keep evaluator errors.
foreach(cpu IN ITEMS MCF5407 MCF5470)
    check_diagnostic(mac-missing-half-${cpu} ${cpu} " mac.w d0,d1.l" 1445
        "word multiply-accumulate requires a register half register.L or register.U" 3 8)
    check_diagnostic(mac-invalid-suffix-${cpu} ${cpu} " mac.w d0.q,d1.l" 1445
        "register half suffix 'q' is invalid; expected L or U" 3 11)
    set(accumulators "ACC or ACC0")
    if(cpu STREQUAL "MCF5470")
        set(accumulators "ACC, ACC0, ACC1, ACC2 or ACC3")
    endif()
    check_diagnostic(mac-invalid-acc-${cpu} ${cpu} " mac.l d0,d1,acc4" 1445
        "operand must be ${accumulators}" 3 14)
    if(cpu STREQUAL "MCF5407")
        check_bytes(mac-register-original-${cpu} ${cpu}
            " mac.w d0.l,d1.u\n msac.w a0.u,a1.l\n mac.l d0,d1,acc\n mac.l d0,d1,acc0"
            "a2000080a2480140a2000800a2000800")
    endif()
    if(cpu STREQUAL "MCF5470")
        check_bytes(mac-register-original-${cpu} ${cpu}
            " mac.w d0.l,d1.u\n msac.w a0.u,a1.l\n mac.l d0,d1,acc\n mac.l d0,d1,acc0\n mac.l d0,d1,acc3\n movclr acc2,d0\n maaac.l d0,d1,acc0,acc3\n mssac.w d0.l,d1.u,acc1,acc2"
            "a2000080a2480140a2000800a2000800a2800810a5c0a200080da280018b")
    endif()
endforeach()
check_diagnostic(mac-base-acc-unavailable MCF5407 " mac.l d0,d1,acc1" 1445 "operand must be ACC or ACC0" 3 14)
check_diagnostic(movclr-acc MCF5470 " movclr acc4,d0" 1445 "operand must be ACC, ACC0, ACC1, ACC2 or ACC3" 3 9)
check_diagnostic(mxxac-first-acc MCF5470 " maaac.l d0,d1,acc4,acc0" 1445 "operand must be ACC, ACC0, ACC1, ACC2 or ACC3" 3 16)
check_diagnostic(mxxac-last-acc MCF5470 " maaac.l d0,d1,acc0,acc4" 1445 "operand must be ACC, ACC0, ACC1, ACC2 or ACC3" 3 21)
check_diagnostic(mul-data-destination 68020 " muls.l d0,a0" 1445 "operand must be D0..D7" 3 12)
check_diagnostic(div-data-destination 68020 " divu.l d0,a0" 1445 "operand must be D0..D7" 3 12)
check_diagnostic(mul-register-size-primary 68020 " fpu on\nbad reg fp0\n muls.l d0,bad" 1130 "invalid operand size" 5 12)
check_diagnostic(mul-register-kind-primary 68020 "bad equ 42\n muls.l d0,bad" 1147 "expected register" 4 12)
check_diagnostic(mul-register-undefined-primary 68020 " muls.l d0,missing" 1445 "invalid register" 3 12)
check_bytes(mul-register-errors-noemit 68020
    " fpu on\nbad reg fp0\nvalue equ 42\n expect 1130,1147,1445\n muls.l d0,bad\n muls.l d0,value\n divu.l d0,a0\n endexpect\n dc.b $12" "12")
check_bytes(mac-register-errors-noemit MCF5470
    " expect 1445,1445,1445,1445\n mac.w d0,d1.l\n mac.w d0.q,d1.l\n movclr acc4,d0\n maaac.l d0,d1,acc0,acc4\n endexpect\n dc.b $12" "12")
check_bytes(mul-single-register-original 68020
    " muls.l d1,d0\n mulu.l d1,d7\n divs.l d1,d0\n divu.l d1,d7\ndest reg d3\n muls.l d1,dest"
    "4c0108004c0170004c4108004c4170074c013800")
check_bytes(mul-forward-register-alias 68020 " muls.l d1,dest\ndest reg d7" "4c017800")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/mac-invalid-suffix-MCF5470.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Suffix 'q'" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German MAC suffix: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# Equal long-multiply destination halves remain a warning with emitted bytes;
# name the actual overlapping register, including register aliases.
check_diagnostic(mul-overlap-low 68020 " muls.l d2,d0:d0" 140
    "both long multiply destinations select D0; destination registers must be distinct" 3 12 0)
check_diagnostic(mul-overlap-high 68020 " mulu.l d2,d7:d7" 140
    "both long multiply destinations select D7; destination registers must be distinct" 3 12 0)
check_diagnostic(mul-overlap-alias 68020 "dest reg d3\n muls.l d2,dest:d3" 140
    "both long multiply destinations select D3; destination registers must be distinct" 4 12 0)
check_bytes(mul-overlap-original 68020 " muls.l d2,d0:d0\n mulu.l d2,d7:d7" "4c020c004c027407")
check_bytes(mul-pairs-and-div-overlap 68020 " muls.l d2,d0:d1\n mulu.l d2,d6:d7\n divs.l d2,d0:d0"
    "4c021c004c0274064c420c00")
if(assembly_diagnostic MATCHES "unpredictable execution")
    message(FATAL_ERROR "Distinct multiply pairs and divide overlap must not warn: ${assembly_diagnostic}")
endif()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/mul-overlap-high.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Multiplikationsziele verwenden D7" detail_pos)
if(NOT "${result}" STREQUAL "0" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German multiply overlap: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# Family masks identify supported models from the registration table. ColdFire
# ISA failures identify the selected ISA versus the exact permitted set.
check_diagnostic(cpu32-requirement 68020 " lpstop #0" 1500
    "supported CPUs: 68332, 68340, 68360" 3 2)
check_diagnostic(coldfire-requirement 68020 " wddata.w (a0)" 1500
    "supported CPUs: MCF5202, MCF5204, MCF5206, MCF5208" 3 2)
check_diagnostic(excluded-generation 68000 " move.w ccr,d0" 1500
    "supported CPUs: 68010, 68012, MCF5202" 3 2)
check_diagnostic(coldfire-isa-requirement MCF5202 " supmode on\n move.l usp,a0" 1500
    "selected ColdFire ISA A; supported ISAs: A+, B, C" 4 2)
check_bytes(family-errors-noemit 68020
    " expect 1500,1500\n lpstop #0\n wddata.w (a0)\n endexpect\n dc.b $12" "12")
check_bytes(excluded-family-noemit 68000
    " expect 1500\n move.w ccr,d0\n endexpect\n dc.b $12" "12")
check_bytes(isa-error-noemit MCF5202
    " supmode on\n expect 1500\n move.l usp,a0\n endexpect\n dc.b $12" "12")
check_bytes(cpu32-original 68332 " supmode on\n lpstop #0" "f80001c00000")
check_bytes(later-generation-original 68010 " move.w ccr,d0" "42c0")
check_bytes(coldfire-original MCF5202 " supmode on\n wddata.w (a0)" "f450")
foreach(cpu IN ITEMS MCF5208 MCF5407 MCF51QM)
    check_bytes(coldfire-isa-original-${cpu} ${cpu} " supmode on\n move.l usp,a0" "4e68")
endforeach()
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/cpu32-requirement.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Erlaubte CPUs: 68332, 68340, 68360" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German family choices: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# CCR AND/OR generation is reached only after size and capability checks pass.
# The old independent block emitted bytes despite EXPECT'ed size/CPU errors.
foreach(ins IN ITEMS and or andi ori)
    string(LENGTH "${ins}" mnemonic_length)
    math(EXPR column "${mnemonic_length} + 3")
    check_diagnostic(ccr-size-${ins} 68000 " ${ins}.w #1,ccr" 1130
        "supported operand sizes: .B" 3 ${column})
    check_diagnostic(ccr-size-primary-${ins} 68000 " ${ins}.w #1/0,ccr" 1130
        "supported operand sizes: .B" 3 ${column})
    check_diagnostic(ccr-cpu-${ins} MCF5202 " ${ins} #1,ccr" 1500
        "supported CPUs: 68008, 68000, 68010, 68012, 68332, 68340, 68360, 68020, 68030, 68040" 3 2)
endforeach()
check_bytes(ccr-size-errors-noemit 68000
    " expect 1130,1130,1130,1130\n and.w #1,ccr\n or.w #1,ccr\n andi.w #1,ccr\n ori.w #1,ccr\n endexpect\n dc.b $12" "12")
check_bytes(ccr-cpu-errors-noemit MCF5202
    " expect 1500,1500,1500,1500\n and #1,ccr\n or #1,ccr\n andi #1,ccr\n ori #1,ccr\n endexpect\n dc.b $12" "12")
check_bytes(ccr-original-logical 68000
    " and.b #1,ccr\n or.b #2,ccr\n andi.b #3,ccr\n ori.b #4,ccr\n eor.b #5,ccr"
    "023c0001003c0002023c0003003c00040a3c0005")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/ccr-size-and.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Erlaubte Operandengr" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German CCR size choices: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# Direct capability checks use the actual feature/ISA predicates to list models.
check_diagnostic(nbcd-models MCF5407 " nbcd (a0)" 1500
    "supported CPUs: 68008, 68000, 68010, 68012, 68332, 68340, 68360, 68020, 68030, 68040" 3 2)
check_diagnostic(tas-isa-models MCF5208 " tas (a0)" 1500 "MCF5407, MCF5470" 3 2)
check_diagnostic(module-call-models 68030 " callm #1,(a0)" 1500 "supported CPUs: 68020" 3 2)
check_diagnostic(module-return-models 68040 " rtm d0" 1500 "supported CPUs: 68020" 3 2)
check_diagnostic(fpu-external-models MCF5407 " fpu on\n fsin fp0" 1500
    "supported CPUs: 68008, 68000, 68010, 68012, 68020, 68030, 68040" 4 2)
check_diagnostic(fpu-internal-models 68020 " fpu on\n fssqrt fp0" 1500
    "supported CPUs: MCF5202, MCF5204, MCF5206, MCF5208" 4 2)
check_diagnostic(mac-models MCF5202 " mac.l d0,d1" 1500 "supported CPUs: MCF5208, MCF52274, MCF52277" 3 2)
check_diagnostic(emac-models MCF5407 " movclr acc0,d0" 1500 "supported CPUs: MCF5208, MCF52274, MCF52277" 3 2)
check_diagnostic(emac-isa-models MCF5208 " maaac.l d0,d1,acc0,acc1" 1500
    "supported CPUs: MCF5470, MCF5471, MCF5472, MCF5473, MCF5474, MCF5475, MCF51QM" 3 2)
foreach(ins IN ITEMS cp0bcbusy cp1bcbusy cp0ld cp1ld cp0st cp1st cp0nop cp1nop)
    check_diagnostic(coprocessor-models-${ins} 68020 " ${ins} #0" 1500
        "supported CPUs: MCF5202, MCF5204, MCF5206, MCF5208" 3 2)
endforeach()
check_bytes(direct-capability-errors-noemit 68030
    " fpu on\n expect 1500,1500,1500,1500\n callm #1,(a0)\n rtm d0\n fssqrt fp0\n cp0nop\n endexpect\n dc.b $12" "12")
check_bytes(cold-capability-errors-noemit MCF5202
    " expect 1500,1500,1500\n tas (a0)\n nbcd (a0)\n mac.l d0,d1\n endexpect\n dc.b $12" "12")
check_bytes(capability-original-classic 68000
    " nbcd d0\n tas (a0)"
    "48004ad0")
check_bytes(capability-original-module 68020
    " callm #1,(a0)\n rtm d0"
    "06d0000106c0")
check_bytes(capability-original-external-fpu 68020
    " fpu on\n fsin fp0"
    "f200000e")
check_bytes(capability-original-internal-fpu 68040
    " fpu on\n fssqrt fp0"
    "f2000041")
check_bytes(capability-original-cold MCF5407
    " tas (a0)\n cp0bcbusy *+2\n cp0ld #0\n cp0st #0\n cp0nop"
    "4ad0fcc00000fc800000fd800000fc800000")
check_bytes(capability-original-emac MCF5470
    " movclr acc0,d0\n maaac.l d0,d1,acc0,acc1"
    "a1c0a2000805")
check_bytes(capability-original-emac-c MCF51QM
    " maaac.l d0,d1,acc0,acc1"
    "a2000805")

# Displacement and absolute suffixes have different permitted widths.
check_diagnostic(absolute-size-choices 68000 " move.w $1234.b,d0" 1130
    "supported operand sizes: .W, .L" 3 15)
check_diagnostic(displacement-size-choices 68020 " move.w 1.q(a0),d0" 1130
    "supported operand sizes: .B, .W, .L" 3 11)
check_bytes(absolute-original-sizes 68000
    " move.w $1234.w,d0\n move.w $1234.l,d0" "30381234303900001234")
check_bytes(displacement-original-sizes 68020
    " move.w 1.b(a0,d0.w),d0\n move.w 1.w(a0),d0\n move.w 1.l(a0),d0"
    "30300001302800013030017000000001")

# Shared integer/FPU size checks retain their CPU-specific predicates.
foreach(ins IN ITEMS lsl addq addx)
    if(ins STREQUAL "lsl")
        set(args "#1,d0")
    elseif(ins STREQUAL "addq")
        set(args "#1,d0")
    else()
        set(args "d0,d1")
    endif()
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(integer-size-${ins} 68000 " ${ins}.s ${args}" 1130
        "supported operand sizes: .B, .W, .L" 3 ${column})
    check_diagnostic(cold-size-${ins} MCF5407 " ${ins}.w ${args}" 1130
        "supported operand sizes: .L" 3 ${column})
endforeach()
foreach(ins IN ITEMS fmove fadd ftst)
    if(ins STREQUAL "ftst")
        set(args "(a0)")
    else()
        set(args "(a0),fp0")
    endif()
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(float-size-${ins} 68020 " fpu on\n ${ins}.q ${args}" 1130
        "supported operand sizes: .B, .W, .L, .S, .D, .X, .P" 4 ${column})
    foreach(size IN ITEMS x p)
        check_diagnostic(cold-float-size-${ins}-${size} MCF5407 " fpu on\n ${ins}.${size} ${args}" 1130
            "supported operand sizes: .B, .W, .L, .S, .D" 4 ${column})
    endforeach()
endforeach()
check_bytes(integer-original-sizes 68000
    " lsl.b #1,d0\n addq.w #1,d0\n addx.l d0,d1" "e3085240d380")
check_bytes(cold-original-sizes MCF5407
    " lsl.l #1,d0\n addq.l #1,d0\n addx.l d0,d1" "e3885280d380")
check_bytes(float-original-sizes 68020
    " fpu on\n fmove.b (a0),fp0\n fmove.w (a0),fp0\n fmove.l (a0),fp0\n fmove.s (a0),fp0\n fmove.d (a0),fp0\n fmove.x (a0),fp0\n fmove.p (a0),fp0"
    "f2105800f2105000f2104000f2104400f2105400f2104800f2104c00")

# ADD/SUB/CMP rejects byte address registers before emitting any bytes.
foreach(ins IN ITEMS add sub cmp)
    check_diagnostic(alu-source-size-${ins} 68000 " ${ins}.b a0,d0" 1130
        "supported operand sizes: .W, .L" 3 6)
    check_diagnostic(alu-destination-size-${ins} 68000 " ${ins}.b d0,a0" 1130
        "supported operand sizes: .W, .L" 3 6)
    check_diagnostic(alu-general-size-${ins} 68000 " ${ins}.s d0,d1" 1130
        "supported operand sizes: .B, .W, .L" 3 6)
endforeach()
foreach(ins IN ITEMS add sub)
    check_diagnostic(alu-cold-size-${ins} MCF5407 " ${ins}.w d0,d1" 1130
        "supported operand sizes: .L" 3 6)
endforeach()
check_diagnostic(cmp-cold-a-size MCF5202 " cmp.w d0,d1" 1130
    "supported operand sizes: .L" 3 6)
check_diagnostic(cmp-cold-b-size MCF5407 " cmp.s d0,d1" 1130
    "supported operand sizes: .B, .W, .L" 3 6)
check_bytes(alu-size-errors-noemit 68000
    " expect 1130,1130,1130\n add.b a0,d0\n sub.b a1,d2\n cmp.b a2,d3\n endexpect\n dc.b $12" "12")
check_bytes(alu-original-sizes 68000
    " add.w a0,d0\n sub.l a1,d2\n cmp.w a2,d3\n adda.w d0,a0\n suba.l d1,a1\n cmpa.w d2,a2"
    "d0489489b64ad0c093c1b4c2")
check_bytes(cmp-cold-original-sizes MCF5407
    " cmp.b d0,d1\n cmp.w d0,d1\n cmp.l d0,d1" "b200b240b280")

# MOVE size choices depend on the decoded special/address registers.
foreach(item IN ITEMS "usp,a0|.L" "a0,usp|.L" "sr,d0|.W" "d0,sr|.W"
        "ccr,d0|.B, .W" "d0,ccr|.B, .W" "a0,d0|.W, .L" "d0,a0|.W, .L")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 args)
    list(GET parts 1 sizes)
    string(REPLACE "," "-" name "${args}")
    if(args MATCHES "a0,d0|d0,a0")
        set(size b)
    else()
        set(size s)
    endif()
    check_diagnostic(move-size-${name} 68010 " move.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 3 7)
endforeach()
check_diagnostic(move-general-size 68000 " move.s d0,d1" 1130
    "supported operand sizes: .B, .W, .L" 3 7)
check_diagnostic(move-mac-source-size MCF5470 " move.w acc0,d0" 1130
    "supported operand sizes: .L" 3 7)
check_diagnostic(move-mac-destination-size MCF5470 " move.w d0,acc0" 1130
    "supported operand sizes: .L" 3 7)
check_bytes(move-special-original-sizes 68010
    " supmode on\n move.l usp,a0\n move.l a1,usp\n move.w sr,d0\n move.w d1,sr\n move.b ccr,d2\n move.w ccr,d3\n move.b d4,ccr\n move.w d5,ccr\n move.w a0,d0\n move.l d1,a1"
    "4e684e6140c046c142c242c344c444c530082241")
check_bytes(move-mac-original-sizes MCF5470
    " move.l acc0,d0\n move.l d1,acc1\n move.l macsr,ccr" "a180a301a9c0")
check_bytes(move-size-errors-noemit 68010
    " expect 1130,1130,1130,1130\n move.b a0,d0\n move.b d0,a0\n move.s sr,d0\n move.s d0,sr\n endexpect\n dc.b $12" "12")

# Simple integer checks list their precise width constraints.
foreach(item IN ITEMS "lea|w|(a0),a1|.L" "addq|b|#1,a0|.W, .L"
        "cmpm|s|(a0)+,(a1)+|.B, .W, .L" "and|b|#1,sr|.W"
        "or|l|#1,sr|.W" "eor|w|#1,ccr|.B" "eor|b|#1,sr|.W"
        "clr|s|d0|.B, .W, .L" "tst|s|d0|.B, .W, .L"
        "nbcd|w|d0|.B" "tas|w|(a0)|.B" "swap|w|d0|.L"
        "ext|b|d0|.W, .L" "moveq|w|#1,d0|.L" "exg|w|d0,d1|.L")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 size)
    list(GET parts 2 args)
    list(GET parts 3 sizes)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    string(REPLACE "," "-" name "${args}")
    check_diagnostic(simple-size-${ins}-${name} 68020 " ${ins}.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 3 ${column})
endforeach()
check_diagnostic(wddata-size MCF5407 " wddata.s (a0)" 1130
    "supported operand sizes: .B, .W, .L" 3 9)
check_bytes(simple-integer-original-sizes 68020
    " supmode on\n lea.l (a0),a1\n addq.w #1,a0\n cmpm.b (a0)+,(a1)+\n and.w #1,sr\n eor.b #1,ccr\n eor.w #1,sr\n clr.b d0\n tst.w d1\n nbcd.b d0\n tas.b (a0)\n swap.l d0\n ext.w d0\n ext.l d1\n moveq.l #1,d0\n exg.l d0,d1"
    "43d05248b308027c00010a3c00010a7c000142004a4148004ad04840488048c17001c141")
check_bytes(wddata-original-sizes MCF5407
    " supmode on\n wddata.b (a0)\n wddata.w (a0)\n wddata.l (a0)" "f410f450f490")

# Extended integer operations retain their word/long/byte requirements.
foreach(item IN ITEMS "movem|b|d0-d1,(a0)|.W, .L" "mulu|b|d0,d1|.W, .L"
        "divsl|w|d0,d1:d2|.L" "abcd|w|d0,d1|.B" "sbcd|w|d0,d1|.B"
        "chk|b|d0,d1|.W, .L" "link|b|a0,#-4|.W, .L"
        "movep|b|d0,1(a0)|.W, .L" "movec|w|vbr,d0|.L"
        "moves|s|d0,(a0)|.B, .W, .L" "cas|s|d0,d1,(a0)|.B, .W, .L"
        "cas2|b|d0:d1,d2:d3,(a0):(a1)|.W, .L"
        "cmp2|s|(a0),d0|.B, .W, .L" "chk2|s|(a0),d0|.B, .W, .L"
        "extb|w|d0|.L")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 size)
    list(GET parts 2 args)
    list(GET parts 3 sizes)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(extended-size-${ins} 68020 " ${ins}.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 3 ${column})
endforeach()
check_diagnostic(movem-cold-size MCF5407 " movem.w d0-d1,(a0)" 1130
    "supported operand sizes: .L" 3 8)
check_diagnostic(table-size 68332 " tblu.s (a0),d0" 1130
    "supported operand sizes: .B, .W, .L" 3 7)
check_bytes(extended-integer-original-sizes 68020
    " supmode on\n movem.w d0-d1,(a0)\n movem.l (a0),d0-d1\n mulu.w d0,d1\n mulu.l d0,d1\n divsl.l d0,d1:d2\n abcd.b d0,d1\n chk.w d0,d1\n link.w a0,#-4\n link.l a0,#-4\n movep.w d0,1(a0)\n movec.l vbr,d0\n moves.b d0,(a0)\n cas.b d0,d1,(a0)\n cas2.w d0:d1,d2:d3,(a0):(a1)\n cmp2.w (a0),d0\n extb.l d0"
    "489000034cd00003c2c04c0010004c402801c30043804e50fffc4808fffffffc018800014e7a08010e1008000ad000400cfc808090c102d0000049c0")
check_bytes(table-original-size 68332 " tblu.b (a0),d0" "f8100100")
check_bytes(movem-cold-original-size MCF5407 " movem.l d0-d1,(a0)" "48d00003")

# Conditional instructions distinguish size constraints from operand counts.
check_diagnostic(branch-size-choices 68020 " bne.q *+4" 1130
    "supported operand sizes: .B, .W, .L, .S, .X" 3 6)
check_diagnostic(set-size-choices 68020 " sne.w d0" 1130
    "supported operand sizes: .B" 3 6)
check_diagnostic(db-size-choices 68020 " dbne.b d0,*+4" 1130
    "supported operand sizes: .W" 3 7)
check_diagnostic(trap-size-choices 68020 " trapne.s #1" 1130
    "supported operand sizes: .B, .W, .L" 3 9)
check_diagnostic(set-missing-operand 68020 " sne" 1110
    "expected one argument but got 0" 3 2)
check_diagnostic(set-extra-operand 68020 " sne d0,d1" 1110
    "expected one argument but got 2" 3 2)
check_diagnostic(set-size-primary 68020 " sne.w" 1130
    "supported operand sizes: .B" 3 6)
check_bytes(conditional-original-sizes 68020
    " bne.b *+4\n bne.s *+4\n bne.w *+4\n bne.l *+4\n bne.x *+4\n sne.b d0\n dbne.w d0,*+4\n trapne.b\n trapne.w #1\n trapne.l #1"
    "66026602660000026600000266ff0000000256c056c8000256fc56fa000156fb00000001")
check_bytes(set-count-errors-noemit 68020
    " expect 1110,1110\n sne\n sne d0,d1\n endexpect\n dc.b $12" "12")

# Instructions that forbid size suffixes say so explicitly.
foreach(item IN ITEMS "jsr|(a0)" "jmp|(a0)" "unlk|a0" "callm|#1,(a0)"
        "pack|d0,d1,#1" "unpk|d0,d1,#1" "rtm|d0" "bftst|d0{0:1}"
        "bfextu|d0{0:1},d1" "bfins|d0,d1{0:1}")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 args)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(no-suffix-${ins} 68020 " ${ins}.w ${args}" 1130
        "operand size suffix is not allowed here; omit the suffix" 3 ${column})
endforeach()
foreach(item IN ITEMS "fsave|(a0)" "frestore|(a0)" "fnop| ")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 args)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(no-suffix-${ins} 68020 " fpu on\n ${ins}.w ${args}" 1130
        "operand size suffix is not allowed here; omit the suffix" 4 ${column})
endforeach()
foreach(item IN ITEMS "psave|(a0)" "prestore|(a0)" "pflusha| " "pflushan| "
        "pflush|#1,#1" "pflushs|#1,#1" "ploadr|#1,(a0)" "ploadw|#1,(a0)"
        "ptestr|#1,(a0),#1" "ptestw|#1,(a0),#1")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 args)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(no-suffix-${ins} 68020 " ${ins}.w ${args}" 1130
        "operand size suffix is not allowed here; omit the suffix" 3 ${column})
endforeach()
check_diagnostic(no-suffix-intouch MCF5407 " intouch.w (a0)" 1130
    "operand size suffix is not allowed here; omit the suffix" 3 10)
check_bytes(no-suffix-original-core 68020
    " jsr (a0)\n jmp (a0)\n unlk a0\n callm #1,(a0)\n pack d0,d1,#1\n rtm d0\n bftst d0{0:1}\n bfextu d0{0:1},d1\n bfins d0,d1{0:1}"
    "4e904ed04e5806d000018340000106c0e8c00001e9c01001efc10001")
check_bytes(no-suffix-original-fpu 68020
    " supmode on\n fpu on\n fsave (a0)\n frestore (a0)\n fnop" "f310f350f2800000")
check_bytes(no-suffix-original-pmmu 68020
    " supmode on\n pmmu on\n fullpmmu on\n psave (a0)\n prestore (a0)\n pflusha\n pflush #1,#1\n ploadr #1,(a0)\n ptestr #1,(a0),#1"
    "f110f150f0002400f0003031f0102211f0108611")
check_bytes(no-suffix-original-pmmu40 68040
    " supmode on\n pmmu on\n pflushan" "f510")
check_bytes(no-suffix-original-cold MCF5407
    " supmode on\n intouch (a0)" "f428")
set(ENV{LC_MESSAGES} de)
file(TO_NATIVE_PATH "${TEST_DIR}/no-suffix-jsr.asm" source)
execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}" -n -q -x
    RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
string(FIND "${output}${error}" "Suffix weglassen" detail_pos)
if(NOT "${result}" STREQUAL "2" OR detail_pos EQUAL -1)
    message(FATAL_ERROR "German forbidden suffix: ${result}\n${output}${error}")
endif()
set(ENV{LC_MESSAGES} en)

# Bit/FPU widths are selected from the decoded operand class and native FP size.
check_diagnostic(bit-data-size 68020 " btst.b #1,d0" 1130
    "supported operand sizes: .L" 3 7)
check_diagnostic(bit-memory-size 68020 " btst.l #1,(a0)" 1130
    "supported operand sizes: .B" 3 7)
foreach(item IN ITEMS "fadd|s|fp0,fp1|.X" "fmove|s|fp0,fp1|.X"
        "fmove|w|d0,fpcr|.L" "fmove|w|fpcr,d0|.L"
        "fmovecr|s|#1,fp0|.X" "fsincos|q|(a0),fp0:fp1|.B, .W, .L, .S, .D, .X, .P"
        "fmovem|s|fp0-fp1,(a0)|.X" "fmovem|s|(a0),fp0-fp1|.X"
        "fmovem|w|fpcr,(a0)|.L" "fmovem|w|(a0),fpcr|.L"
        "fbne|b|*+4|.W, .L, .X" "fdbne|b|d0,*+6|.W"
        "fsne|w|d0|.B" "ftrapne|s|#1|.B, .W, .L")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 size)
    list(GET parts 2 args)
    list(GET parts 3 sizes)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    string(REGEX REPLACE "[^A-Za-z0-9_-]" "_" name "${args}")
    check_diagnostic(fpu-specific-size-${ins}-${name} 68020 " fpu on\n ${ins}.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 4 ${column})
endforeach()
foreach(ins IN ITEMS fadd fmove fmovem)
    if(ins STREQUAL "fmovem")
        set(args "fp0,(a0)")
    else()
        set(args "fp0,fp1")
    endif()
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(cold-native-size-${ins} MCF5407 " fpu on\n ${ins}.s ${args}" 1130
        "supported operand sizes: .D" 4 ${column})
endforeach()
check_bytes(fpu-specific-original-sizes 68020
    " fpu on\n fadd.x fp0,fp1\n fmove.x fp0,fp1\n fmove.l d0,fpcr\n fmove.l fpcr,d0\n fmovecr.x #1,fp0\n fsincos.b (a0),fp0:fp1\n fmovem.x fp0-fp1,(a0)\n fmovem.x (a0),fp0-fp1\n fmovem.l fpcr,(a0)\n fmovem.l (a0),fpcr\n fbne.w *+4\n fdbne.w d0,*+6\n fsne.b d0\n ftrapne.b\n ftrapne.w #1"
    "f20000a2f2000080f2009000f200b000f2005c01f21058b0f210f0c0f210d0c0f210b000f2109000f28e0002f248000e0002f240000ef27c000ef27a000e0001")
check_bytes(cold-native-original-sizes MCF5407
    " fpu on\n fadd.d fp0,fp1\n fmove.d fp0,fp1\n fmovem.d fp0,(a0)" "f20000a2f2000080f210f080")
check_bytes(bit-original-sizes 68020
    " btst.l #1,d0\n btst.b #1,(a0)" "0800000108100001")

# PMMU register widths and conditional sizes come from the selected form.
foreach(item IN ITEMS "pflushr|w|(a0)|.Q" "pvalid|w|val,(a0)|.L"
        "pbbs|b|*+4|.W, .L, .X" "pdbbs|b|d0,*+6|.W"
        "psbs|w|d0|.B" "ptrapbs|s|#1|.B, .W, .L")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 size)
    list(GET parts 2 args)
    list(GET parts 3 sizes)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(pmmu-size-${ins} 68020 " pmmu on\n fullpmmu on\n ${ins}.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 5 ${column})
endforeach()
foreach(item IN ITEMS "cal|.B" "psr|.W" "tc|.L" "crp|.Q")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 reg)
    list(GET parts 1 size)
    check_diagnostic(pmove-source-size-${reg} 68020 " pmmu on\n fullpmmu on\n pmove.s ${reg},(a0)" 1130
        "supported operand sizes: ${size}" 5 8)
    check_diagnostic(pmove-destination-size-${reg} 68020 " pmmu on\n fullpmmu on\n pmove.s (a0),${reg}" 1130
        "supported operand sizes: ${size}" 5 8)
endforeach()
foreach(suffix IN ITEMS "" ".q" ".w")
    string(REPLACE "." "_" name "${suffix}")
    check_diagnostic(pflushr-disabled${name} 68020 " pmmu off\n pflushr${suffix} (a0)" 1502
        "PMMU instructions are not enabled" 4 2)
endforeach()
check_diagnostic(pflushr-disabled-primary 68020 " pmmu off\n pflushr.w missing" 1502
    "PMMU instructions are not enabled" 4 2)
check_bytes(pflushr-disabled-noemit 68020
    " pmmu off\n expect 1502,1502,1502\n pflushr (a0)\n pflushr.q (a0)\n pflushr.w (a0)\n endexpect\n dc.b $12" "12")
check_bytes(pmmu-specific-original-sizes 68020
    " supmode on\n pmmu on\n fullpmmu on\n pflushr.q (a0)\n pmove.b cal,(a0)\n pmove.w psr,(a0)\n pmove.l tc,(a0)\n pmove.q crp,(a0)\n pmove.b (a0),cal\n pmove.w (a0),psr\n pmove.l (a0),tc\n pmove.q (a0),crp\n pvalid.l val,(a0)\n pbbs.w *+4\n pdbbs.w d0,*+6\n psbs.b d0\n ptrapbs.b\n ptrapbs.w #1"
    "f010a000f0105200f0106200f0104200f0104e00f0105000f0106000f0104000f0104c00f0102800f0800002f04800000002f0400000f07c0000f07a00000001")

# Remaining ColdFire width checks list their exact accepted sizes.
check_diagnostic(stldsr-size MCF5208 " stldsr.l #1" 1130
    "supported operand sizes: .W" 3 9)
foreach(item IN ITEMS "mvs|l|d0,d1|.B, .W" "mvz|l|d0,d1|.B, .W"
        "mac|b|d0,d1|.W, .L" "msac|b|d0,d1|.W, .L"
        "movclr|w|acc0,d0|.L" "maaac|b|d0,d1,acc0,acc1|.W, .L"
        "cp0bcbusy|l|*+2|.W")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 size)
    list(GET parts 2 args)
    list(GET parts 3 sizes)
    string(LENGTH "${ins}" length)
    math(EXPR column "${length}+3")
    check_diagnostic(cold-specific-size-${ins} MCF5470 " ${ins}.${size} ${args}" 1130
        "supported operand sizes: ${sizes}" 3 ${column})
endforeach()
check_bytes(stldsr-original-size MCF5208 " stldsr.w #1" "40e746fc0001")
check_bytes(cold-specific-original-sizes MCF5470
    " mvs.b d0,d1\n mvz.w d0,d1\n mac.w d0.l,d1.u\n mac.l d0,d1\n movclr.l acc0,d0\n maaac.w d0.l,d1.u,acc0,acc1\n maaac.l d0,d1,acc0,acc1\n cp0bcbusy.w *+2"
    "730073c0a2000080a2000800a1c0a2000085a2000805fcc00000")

# The shared mode rejection lists forms from the caller's actual allowed mask.
check_diagnostic(lea-address-choices 68020 " lea.l d0,a0" 1350
    "operand must be (An), d(An), d(An,Xn), d(PC), d(PC,Xn), addr.W/addr.L" 3 8)
check_diagnostic(jsr-address-choices 68020 " jsr #1" 1350
    "operand must be (An), d(An), d(An,Xn), d(PC), d(PC,Xn), addr.W/addr.L" 3 6)
check_diagnostic(move-destination-choices 68020 " move.w d0,#1" 1350
    "operand must be D0..D7, A0..A7, (An), (An)+, -(An), d(An), d(An,Xn), addr.W/addr.L" 3 12)
check_diagnostic(fpu-control-source-choices 68020 " fpu on\n fmove.l fp0,a0" 1350
    "operand must be FPCR/FPSR/FPIAR" 4 10)
check_diagnostic(cold-memory-shift-choices MCF5407 " lsl.l (a0)" 1350
    "operand must be D0..D7" 3 8)
check_diagnostic(mode-undefined-primary 68020 " lea.l missing,a0" 1010
    "symbol undefined" 3 8)
check_bytes(address-original-choices 68020
    " lea.l (a0),a1\n lea.l 1(a0),a1\n lea.l 1(a0,d0.w),a1\n lea.l *+2(pc),a1\n lea.l 1(pc,d0.w),a1\n lea.l $1234,a1"
    "43d043e8000143f0000143fa000043fb0120eff143f81234")

# Scale evaluation failure is distinct from an operand that is not a scale.
check_diagnostic(mac-scale-choices MCF5470 " maaac.l d0,d1,bad,acc0,acc1" 1350
    "MAC scale must be <<0, <<1, >>0 or >>1" 3 16)
foreach(item IN ITEMS "mac|d0,d1|16" "maaac|d0,d1|18")
    string(REPLACE "|" ";" parts "${item}")
    list(GET parts 0 ins)
    list(GET parts 1 args)
    list(GET parts 2 column)
    if(ins STREQUAL "maaac")
        set(tail ",acc0,acc1")
    else()
        set(tail "")
    endif()
    check_diagnostic(scale-overflow-${ins} MCF5470 " ${ins}.l ${args},<<2${tail}" 1320
        "2 > 1" 3 ${column})
    check_diagnostic(scale-undefined-${ins} MCF5470 " ${ins}.l ${args},<<missing${tail}" 1010
        "symbol undefined" 3 ${column})
    check_diagnostic(scale-divzero-${ins} MCF5470 " ${ins}.l ${args},<<1/0${tail}" 1310
        "division by 0" 3 "")
endforeach()
check_bytes(mac-scale-original-forms MCF5470
    " mac.l d0,d1\n mac.l d0,d1,<<\n mac.l d0,d1,<<0\n mac.l d0,d1,<<1\n mac.l d0,d1,>>\n mac.l d0,d1,>>0\n mac.l d0,d1,>>1\n maaac.l d0,d1,<<0,acc0,acc1\n maaac.l d0,d1,<<1,acc0,acc1\n maaac.l d0,d1,>>0,acc0,acc1\n maaac.l d0,d1,>>1,acc0,acc1"
    "a2000800a2000a00a2000800a2000a00a2000e00a2000800a2000e00a2000805a2000a05a2000805a2000e05")
check_bytes(mac-scale-empty-forms MCF5470
    " mac.l d0,d1,\n maaac.l d0,d1,,acc0,acc1" "a2000800a2000805")
check_bytes(mac-scale-forward MCF5470
    " mac.l d0,d1,<<count\n maaac.l d0,d1,>>count,acc0,acc1\ncount equ 1"
    "a2000a00a2000e05")
check_bytes(mac-scale-errors-noemit MCF5470
    " expect 1320,1320\n mac.l d0,d1,<<2\n maaac.l d0,d1,<<2,acc0,acc1\n endexpect\n dc.b $12" "12")
