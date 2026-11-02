# SPDX-License-Identifier: GPL-2.0-only
file(MAKE_DIRECTORY "${TEST_DIR}")

function(check_scope name cpu body expected)
    set(source "${TEST_DIR}/${name}.asm")
    set(object "${TEST_DIR}/${name}.p")
    set(binary "${TEST_DIR}/${name}.bin")
    file(WRITE "${source}" " cpu ${cpu}\n${body}\n")
    execute_process(COMMAND ${TEST_EMULATOR} "${ASL_PROGRAM}" "${source}"
        -n -q -x -werror -o "${object}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: assembly ${result}\n${output}${error}")
    endif()
    execute_process(COMMAND ${TEST_EMULATOR} "${P2BIN_PROGRAM}" "${object}" "${binary}"
        RESULT_VARIABLE result OUTPUT_VARIABLE output ERROR_VARIABLE error TIMEOUT 15)
    if(NOT "${result}" STREQUAL "0")
        message(FATAL_ERROR "${name}: conversion ${result}\n${output}${error}")
    endif()
    file(READ "${binary}" actual HEX)
    if(NOT actual STREQUAL expected)
        message(FATAL_ERROR "${name}: expected ${expected}, got ${actual}")
    endif()
endfunction()

set(definitions "empty macro\n endm\ncommentonly macro\n ; no emitted code\n endm\nearly macro\n exitm\n dc.b $ff\n endm\nglobalempty macro {GLOBALSYMBOLS}\n endm\n")
set(empty_macro " empty")
set(comment_macro " commentonly")
set(early_exit " early")
set(global_macro " globalempty")
set(empty_rept " rept 2\n endr")
set(empty_irp " irp item,1,2\n endm")
set(empty_irpn " irpn 2,item,other,1,2,3,4\n endm")
set(empty_irpc " irpc item,\"AB\"\n endm")
set(empty_while " while 1\n endm")
set(skipped_rept " rept 0\n dc.b $ff\n endr")
set(skipped_while " while 0\n dc.b $ff\n endm")
set(live_rept " rept 2\n empty\n endr")
set(live_irp " irp item,1,2\n empty\n endm")
set(live_irpn " irpn 2,item,other,1,2,3,4\n empty\n endm")
set(live_irpc " irpc item,\"AB\"\n empty\n endm")
set(exit_rept " rept 2\n exitm\n dc.b $ff\n endr")
set(exit_irp " irp item,1,2\n exitm\n dc.b $ff\n endm")
set(exit_irpn " irpn 2,item,other,1,2,3,4\n exitm\n dc.b $ff\n endm")
set(exit_irpc " irpc item,\"AB\"\n exitm\n dc.b $ff\n endm")
set(exit_while " while 1\n exitm\n dc.b $ff\n endm")

foreach(cpu IN ITEMS 68000 Z80)
    if(cpu STREQUAL "68000")
        set(branch bra.s)
        set(backward "60fe60fc60fe60fc")
        set(forward "600460024e71600460024e71")
        set(nested "60fe60fe60fc60fe60fc60f4")
    else()
        set(branch jr)
        set(backward "18fe18fc18fe18fc")
        set(forward "18031801001803180100")
        set(nested "18fe18fe18fc18fe18fc18f4")
    endif()

    # Empty expansions must not pop the enclosing iteration's local scope.
    # Controls cover expansions that do open a scope, EXITM, global symbols,
    # and loops that never execute their nonempty bodies.
    foreach(kind IN ITEMS empty_macro comment_macro early_exit global_macro
            empty_rept empty_irp empty_irpn empty_irpc empty_while
            skipped_rept skipped_while live_rept live_irp live_irpn live_irpc
            exit_rept exit_irp exit_irpn exit_irpc exit_while)
        check_scope("${cpu}-${kind}" "${cpu}"
            "${definitions}Code:\n rept 2\n.r:\n ${branch} .r\n${${kind}}\n ${branch} .r\n endr"
            "${backward}")
    endforeach()

    # Labels defined after the empty macro must remain private per iteration.
    check_scope("${cpu}-forward" "${cpu}"
        "${definitions}Code:\n rept 2\n ${branch} .r\n empty\n ${branch} .r\n nop\n.r:\n endr"
        "${forward}")

    # Preserve both the macro's scope and its nested repetition scopes.
    check_scope("${cpu}-nested" "${cpu}"
        "${definitions}outer macro\n.r:\n ${branch} .r\n rept 2\n.s:\n ${branch} .s\n empty\n ${branch} .s\n endr\n ${branch} .r\n endm\n outer"
        "${nested}")

    # A global-symbol macro must leave its caller's scope in place as well.
    check_scope("${cpu}-global-caller" "${cpu}"
        "${definitions}shared macro {GLOBALSYMBOLS}\n empty\n ${branch} .r\n endm\nCode:\n rept 2\n.r:\n ${branch} .r\n shared\n endr"
        "${backward}")
endforeach()
