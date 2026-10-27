cmake_minimum_required(VERSION 3.19)

project(asl-flamewing
    VERSION 1.42.313
    DESCRIPTION "Improved/bugfixed version of Alfred Arnold's The Macro Assembler AS"
    LANGUAGES C
)

# Joins arguments and places the results in ${result_var}.
function(join result_var)
    set(result "")

    foreach(arg ${ARGN})
        set(result "${result}${arg}")
    endforeach()

    set(${result_var} "${result}" PARENT_SCOPE)
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

# include(CheckIPOSupported)
# check_ipo_supported(RESULT iposupported OUTPUT ipoerror)

# if(iposupported)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
# add_link_options(-fuse-ld=lld)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# else()
# message(WARNING "IPO is not supported: ${ipoerror}")
# endif()
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(DEFINED ENV{MSYSTEM_PREFIX})
        set(CMAKE_INSTALL_PREFIX "$ENV{MSYSTEM_PREFIX}" CACHE PATH "Default install prefix" FORCE)
    endif(DEFINED ENV{MSYSTEM_PREFIX})
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(GNUInstallDirs)

# when crosscompiling import the executable targets from a file
if(CMAKE_CROSSCOMPILING)
    set(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file from a native build")
    include(${IMPORT_EXECUTABLES})
endif(CMAKE_CROSSCOMPILING)

include(CheckCCompilerFlag)

function(add_compile_options_safe FLAG)
    string(REGEX REPLACE "[-=+]" "" FLAG_NO_SIGNS ${FLAG}) # <- The variable recieving the result of the test can't have those signs in its name
    check_c_compiler_flag(${FLAG} C_COMPILER_SUPPORTS_${FLAG_NO_SIGNS})

    if(C_COMPILER_SUPPORTS_${FLAG_NO_SIGNS})
        add_compile_options($<$<COMPILE_LANGUAGE:C>:${FLAG}>)
    endif(C_COMPILER_SUPPORTS_${FLAG_NO_SIGNS})
endfunction()

mark_as_advanced(add_compile_options_safe)

# include(CheckIPOSupported)
# check_ipo_supported(RESULT HAVE_IPO OUTPUT error)

# if(HAVE_IPO)
# message(STATUS "IPO / LTO enabled")
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# else()
# message(STATUS "IPO / LTO not supported: <${error}>")
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
# endif()
add_compile_definitions(LIBDIR="${CMAKE_INSTALL_FULL_LIBDIR}/asl")

include(CheckFunctionExists)

if(NOT POW_FUNCTION_EXISTS AND NOT NEED_LINKING_AGAINST_LIBM)
    CHECK_FUNCTION_EXISTS(pow POW_FUNCTION_EXISTS)

    if(NOT POW_FUNCTION_EXISTS)
        unset(POW_FUNCTION_EXISTS CACHE)
        list(APPEND CMAKE_REQUIRED_LIBRARIES m)
        CHECK_FUNCTION_EXISTS(pow POW_FUNCTION_EXISTS)

        if(POW_FUNCTION_EXISTS)
            set(NEED_LINKING_AGAINST_LIBM True CACHE BOOL "" FORCE)
        else()
            message(FATAL_ERROR "Failed making the pow() function available")
        endif()
    endif()
endif()

if(NEED_LINKING_AGAINST_LIBM)
    function(target_link_libm target)
        target_link_libraries(${target} PUBLIC m)
    endfunction()
else()
    function(target_link_libm target)
    endfunction()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU" OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    if(${FORCE_COLORED_OUTPUT})
        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            add_compile_options(-fcolor-diagnostics -fansi-escape-codes)
            add_link_options(-fcolor-diagnostics -fansi-escape-codes)
        elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
            add_compile_options(-fdiagnostics-color=always)
            add_link_options(-fdiagnostics-color=always)
        endif()
    endif()

    if(WARNINGS_ARE_ERRORS)
        add_compile_options(-Werror)
        add_compile_options(-pedantic-errors)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options_safe(-Og)
        add_compile_options_safe(-g3)

        if(NOT C_COMPILER_SUPPORTS_Og)
            add_compile_options(-O0)
        endif()

        if(NOT C_COMPILER_SUPPORTS_g3)
            add_compile_options(-g)
        endif()
    else()
        add_compile_options(-O3)
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Weverything)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c11-extensions>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c23-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c23-extensions>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c2x-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c2x-extensions>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c99-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c99-designator>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c99-extensions>)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)

        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wc11-c2x-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c90-c99-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-c99-c11-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wc++-compat>)

        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Waddress>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Waggregate-return>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Walloc-zero>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Walloca>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wanalyzer-too-complex>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Warith-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wbad-function-cast>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wbidi-chars>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wbidi-chars=any>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-align>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-align=strict>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-qual>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wchkp>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcomments>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcomment>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wconversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdate-time>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-declaration-after-statement>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdisabled-optimization>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdouble-promotion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wduplicated-branches>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wduplicated-cond>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Werror-implicit-function-declaration>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wfloat-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wfloat-equal>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-nonliteral>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-security>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-signedness>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-y2k>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Whsa>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Winit-self>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Winline>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Winvalid-pch>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Winvalid-utf8>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wjump-misses-init>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wlogical-op>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmissing-declarations>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmissing-format-attribute>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=format>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmissing-include-dirs>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmissing-noreturn>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=noreturn>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wmultichar>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wnarrowing>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wnested-externs>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wnull-dereference>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wold-style-definition>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wopenacc-parallelism>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpacked>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpacked-bitfield-compat>)

        # add_compile_option$<COMPILE_LANGUAGE:C>:s($<$<COMPILE_LANGUAGE:C>:-Wpadded>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wredundant-decls>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=compatible-local>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=global>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=local>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshift-negative-value>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsign-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstack-protector>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-aliasing>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-flex-arrays>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-overflow>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=cold>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=const>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=format>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=malloc>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=noreturn>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-attribute=pure>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-final-methods>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsuggest-final-types>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wswitch-default>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wswitch-enum>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-system-headers>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtraditional>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtraditional-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtrampolines>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtrivial-auto-var-init>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wundef>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wunreachable-code>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wunsafe-loop-optimizations>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wunsuffixed-float-constants>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wunused-macros>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wvector-operation-performance>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wvla>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wwrite-strings>)

        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Warray-bounds=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Warray-parameter=2>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wattribute-alias=1>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcatch-value=1>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wdangling-pointer=2>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-overflow=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-truncation=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wimplicit-fallthrough=3>)

        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Woverloaded-virtual>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Woverloaded-virtual=2>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wplacement-new=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshift-overflow=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstringop-overflow=2>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wunused-const-variable=2>)
    endif()
elseif(MSVC)
    if(WARNINGS_ARE_ERRORS)
        add_compile_options("/WX")
    endif()

    # add_compile_options("/W4")
    # add_compile_options("/wd4018") # warning C4018: '>': signed/unsigned mismatch
    # add_compile_options("/wd4127") # warning C4127: conditional expression is constant
    # add_compile_options("/wd4244") # warning C4244: 'initializing': conversion from 'int' to 'char', possible loss of data
    # add_compile_options("/wd4251")

    # # Clang: -Wshorten-64-to-32 -Wimplicit-int-conversion
    # add_compile_options("/wd4267") # warning C4267: 'return': conversion from 'size_t' to 'int', possible loss of data
    # add_compile_options("/wd4389") # warning C4389: '==': signed/unsigned mismatch
    # add_compile_options("/wd4482")
    # add_compile_options("/wd4512")
    # add_compile_options("/wd4701") # warning C4701: potentially uninitialized local variable 'err' used
    # add_compile_options("/wd4706") # warning C4706: assignment within conditional expression
    # add_compile_options("/wd4800") # warning C4800: 'const SymbolDatabase *' : forcing value to bool 'true' or 'false' (performance warning)
endif()

include(Profiling)

if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # If Microsoft SDK is installed create script run-msbuild.bat that
    # calls SetEnv.cmd to set up build environment and runs msbuild.
    # It is useful when building Visual Studio projects with the SDK
    # toolchain rather than Visual Studio.
    include(FindSetEnv)

    if(WINSDK_SETENV)
        set(MSBUILD_SETUP "call \"${WINSDK_SETENV}\"")
    endif()

    # Set FrameworkPathOverride to get rid of MSB3644 warnings.
    join(netfxpath "C:\\Program Files\\Reference Assemblies\\Microsoft\\Framework\\.NETFramework\\v4.0")
    file(WRITE run-msbuild.bat "
        ${MSBUILD_SETUP}
        ${CMAKE_MAKE_PROGRAM} -p:FrameworkPathOverride=\"${netfxpath}\" %*")
endif()

INCLUDE(CheckIncludeFiles)
set(CHECK_LOCALE TRUE)

if(CMAKE_CROSSCOMPILING)
    if(DEFINED EMSCRIPTEN)
        if(${EMSCRIPTEN})
            set(CHECK_LOCALE FALSE)
        endif(${EMSCRIPTEN})
    endif(DEFINED EMSCRIPTEN)
endif(CMAKE_CROSSCOMPILING)

if(${CHECK_LOCALE})
    CHECK_INCLUDE_FILES("windef.h;winbase.h;winnls.h" W32_NLS)
    CHECK_INCLUDE_FILES("locale.h;langinfo.h" LOCALE_NLS)
    if(LOCALE_NLS)
        # Some SDKs provide langinfo.h before the target OS supports nl_langinfo
        # (for example, Android API levels below 26).
        include(CheckSymbolExists)
        check_symbol_exists(nl_langinfo "locale.h;langinfo.h" HAVE_NL_LANGINFO)
        set(LOCALE_NLS "${HAVE_NL_LANGINFO}")
    endif()
    CHECK_INCLUDE_FILES(os2.h OS2_NLS)
else()
    set(LOCALE_NLS FALSE)
    set(OS2_NLS FALSE)
    set(W32_NLS FALSE)
endif(${CHECK_LOCALE})

CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
set(ARCHPRNAME "${CMAKE_SYSTEM_PROCESSOR}")
set(ARCHSYSNAME "${CMAKE_SYSTEM_NAME}")
CHECK_INCLUDE_FILES("endian.h" HAVE_ENDIAN_H)
CHECK_INCLUDE_FILES("sys/param.h" HAVE_SYS_PARAM_H)
if(MINGW)
    # The CRT printf family treats long double as double without MinGW's wrappers.
    add_compile_definitions(__USE_MINGW_ANSI_STDIO=1)
endif()

include(CheckTypeSize)
include(CheckCSourceCompiles)
check_c_source_compiles("int main(void) { __int128 n = 1; return (int)n; }" AS_COMPILER_HAS_INT128)
option(AS_USE_INT128 "Use 128-bit expressions (changes signed address comparisons)." OFF)
if(AS_USE_INT128 AND NOT AS_COMPILER_HAS_INT128)
    message(FATAL_ERROR "128-bit expressions are unsupported by this compiler")
endif()
set(AS_HAVE_INT128 ${AS_USE_INT128})
check_type_size("long double" AS_LONG_DOUBLE_SIZE LANGUAGE C)
# Select the supported x87 representation by precision, not by OS name.
check_c_source_compiles("#include <float.h>
#if LDBL_MANT_DIG != 64 || LDBL_MAX_EXP != 16384
#error unsupported extended representation
#endif
int main(void) { return 0; }" AS_IEEE_EXTENDED)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

add_library(ST_OBJECTS
    OBJECT
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    nonzstring.c
    nonzstring.h
    dynstr.c
    dynstr.h
    stdhandl.c
    as_runtime.c
    as_runtime.h
    stdhandl.h
    strcomp.c
    strcomp.h
    as_endian.c
    as_endian.h
    bpemu.c
    bpemu.h
)
target_include_directories(ST_OBJECTS
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

# only build the generator if not crosscompiling
if(NOT CMAKE_CROSSCOMPILING)
    set(RESCOMP_OBJECTS
        rescomp.c
    )

    add_executable(rescomp
        ${RESCOMP_OBJECTS}
        $<TARGET_OBJECTS:ST_OBJECTS>
    )
    target_include_directories(rescomp
        PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
    )
    set_target_properties(rescomp
        PROPERTIES
        OUTPUT_NAME rescomp
    )
endif(NOT CMAKE_CROSSCOMPILING)

option(ASL_EMBED_MESSAGES "Embed message catalogs in executable files" ON)
if(ASL_EMBED_MESSAGES)
    add_compile_definitions(_USE_MSH)
endif()

function(define_rescomp_target STEM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${STEM}.msg ${CMAKE_BINARY_DIR}/${STEM}.msh
        COMMAND rescomp ${STEM}.mes -m ${CMAKE_BINARY_DIR}/${STEM}.msg -i ${CMAKE_BINARY_DIR}/${STEM}.msh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS rescomp ${STEM}.mes ${ARGN}
    )
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${STEM}.rsc
        COMMAND rescomp ${STEM}.mes -h ${CMAKE_BINARY_DIR}/${STEM}.rsc
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS rescomp ${STEM}.mes ${ARGN}
    )
    add_custom_target(${STEM}_res ALL
        SOURCES ${CMAKE_BINARY_DIR}/${STEM}.msg ${CMAKE_BINARY_DIR}/${STEM}.msh ${CMAKE_BINARY_DIR}/${STEM}.rsc
    )
endfunction(define_rescomp_target)

define_rescomp_target(ioerrs header.mes)
define_rescomp_target(cmdarg header.mes)
define_rescomp_target(tools header.mes)
define_rescomp_target(as header.mes)
define_rescomp_target(das header.mes)
define_rescomp_target(plist tools2.mes header.mes)
define_rescomp_target(alink tools2.mes header.mes)
define_rescomp_target(pbind tools2.mes header.mes)
define_rescomp_target(p2hex tools2.mes header.mes)
define_rescomp_target(p2bin tools2.mes header.mes)

add_library(CMDLINE_COMMON
    OBJECT
    msg_level.c
    msg_level.h
    cmdarg.c
    cmdarg.h
    version.c
    version.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)
target_include_directories(CMDLINE_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
add_dependencies(CMDLINE_COMMON cmdarg_res ioerrs_res)

add_library(NLS_OBJECTS
    OBJECT
    nls.c
    nls.h
    nlmessages.c
    nlmessages.h
    chardefs.c
    chardefs.h
)
target_include_directories(NLS_OBJECTS
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_library(ADDRSPACE_COMMON
    addrspace.c
    addrspace.h
)
target_include_directories(ADDRSPACE_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_library(AS_DAS_COMMON
    console.c
    console.h
    cpulist.c
    cpulist.h
    trees.c
    trees.h
)
target_include_directories(AS_DAS_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_library(TOOLS_COMMON
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
)
target_include_directories(TOOLS_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
add_dependencies(TOOLS_COMMON tools_res)

add_library(CHUNKS_COMMON
    chunks.c
    chunks.h
)
target_include_directories(CHUNKS_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

add_library(HEADIDS_COMMON
    headids.c
    headids.h
)
target_include_directories(HEADIDS_COMMON
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

include(SelectBackends)

add_library(AS_OBJECTS
    ${ASL_BACKEND_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/backend_registry.c
    as.c
    as.h
    ${CMAKE_BINARY_DIR}/as.msg
    ${CMAKE_BINARY_DIR}/as.rsc
    onoff_common.c
    onoff_common.h
    fwd_refs.c
    fwd_refs.h
    fwd_sym.c
    fwd_sym.h
    assume.c
    assume.h
    asmdef.c
    asmdef.h
    striter.c
    striter.h
    asmsub.c
    asmsub.h
    asmerr.c
    asmerr.h
    lstmacroexp.c
    lstmacroexp.h
    asmpars.c
    asmpars.h
    asmmac.c
    asmmac.h
    asmstructs.c
    asmstructs.h
    asmcode.c
    asmcode.h
    asmlist.c
    asmlist.h
    asmlabel.c
    asmlabel.h
    asmdebug.c
    asmdebug.h
    asmif.c
    asmif.h
    asmfnums.c
    asmfnums.h
    asmrelocs.c
    asmrelocs.h
    tempresult.c
    tempresult.h
    operator.c
    operator.h
    cpu2phys.c
    cpu2phys.h
    function.c
    function.h
    asminclist.c
    asminclist.h
    symbolsize.c
    symbolsize.h
    asmitree.c
    asmitree.h
    asmallg.c
    asmallg.h
    intformat.c
    intformat.h
    sha1.c
    sha1.h
    chartrans.c
    chartrans.h
    codenano.h
    code61860.h
    code62015.h
    codepalm.h
    codepdp11.h
    codepseudo.c
    codepseudo.h
    decpseudo.c
    decpseudo.h
    intpseudo.c
    intpseudo.h
    motpseudo.c
    motpseudo.h
    tipseudo.c
    tipseudo.h
    natpseudo.c
    natpseudo.h
    fourpseudo.c
    fourpseudo.h
    ibmfloat.c
    ibmfloat.h
    milfloat.c
    milfloat.h
    necfloat.c
    necfloat.h
    tifloat.c
    tifloat.h
    as_float.c
    as_float.h
    bfloat.c
    bfloat.h
    ieeefloat.c
    ieeefloat.h
    decfloat.c
    decfloat.h
    codevars.c
    codevars.h
    dyn_array.h
    codenone.h
    code68k.h
    code68k.hpp
    code56k.h
    code601.h
    codemcore.h
    codexgate.h
    code68.h
    code6805.h
    codeuc43.h
    codetbil.h
    codec16c.h
    codecr16.h
    code340xx.h
    codewe32.h
    codewe32.hpp
    int_type.c
    int_type.h
    codevax.h
    coderx.h
    codeko09.h
    code6809.h
    code6812.h
    codes12z.h
    code6816.h
    code68rs08.h
    codeh8_3.h
    codeh8_5.h
    code7000.h
    code7000.hpp
    code6100.h
    code65.h
    codeh16.h
    codeh16.hpp
    code7700.h
    codehmcs400.h
    code4500.h
    codem16.h
    codem16c.h
    code4004.h
    code8008.h
    code48.h
    code51.h
    code96.h
    code85.h
    code86.h
    code960.h
    code8x30x.h
    code2650.h
    codexa.h
    codeavr.h
    code29k.h
    code166.h
    codev60.h
    codez80.h
    codez80.hpp
    codez8.h
    codez8.hpp
    codez8000.h
    codez8000.hpp
    codekcpsm.h
    codekcp3.h
    codemic8.h
    code96c141.h
    code90c141.h
    code87c800.h
    code870c.h
    code42c00.h
    code42c00.hpp
    code47c00.h
    code97c241.h
    code9331.h
    code16c5x.h
    code16c8x.h
    code17c4x.h
    codesx20.h
    codepdk.h
    codest6.h
    codest7.h
    codest9.h
    code6804.h
    code3201x.h
    code3202x.h
    code3203x.h
    code3205x.h
    code3254x.h
    code3206x.h
    code3206x.hpp
    code9900.h
    codetms7.h
    code370.h
    codemsp.h
    codetms1.h
    code78c10.h
    code75xx.h
    code75k0.h
    code78k0.h
    code78k2.h
    code78k3.h
    code78k4.h
    code7720.h
    code77230.h
    codescmp.h
    code807x.h
    codecop4.h
    codecop8.h
    codesc14xxx.h
    codens32k.h
    codens32k.hpp
    codeace.h
    codef8.h
    code53c8xx.h
    codefmc8.h
    codefmc16.h
    codemn1610.h
    codemn2610.h
    codeol40.h
    codeol50.h
    code1802.h
    codevector.h
    codexcore.h
    code1750.h
    codekenbak.h
    codecp3f.h
    aplfloat.c
    aplfloat.h
    literals.c
    literals.h
    codeimp16.h
    codepps4.h
    dynstr_nls.c
    dynstr_nls.h
    codecp1600.h
    math64.c
    math64.h
    errmsg.c
    errmsg.h
)
target_include_directories(AS_OBJECTS
    PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)
add_dependencies(AS_OBJECTS as_res)

set(DAS_OBJECTS
    das.c
    ${CMAKE_BINARY_DIR}/das.msg
    ${CMAKE_BINARY_DIR}/das.rsc
    dasmdef.c
    dasmdef.h
    deco68.c
    deco68.h
    deco87c800.c
    deco87c800.h
    deco4004.c
    deco4004.h
    codechunks.c
    codechunks.h
    entryaddress.c
    entryaddress.h
    invaddress.c
    invaddress.h
)

set(PLIST_OBJECTS
    plist.c
    ${CMAKE_BINARY_DIR}/plist.msg
    ${CMAKE_BINARY_DIR}/plist.rsc
)

set(ALINK_OBJECTS
    alink.c
    ${CMAKE_BINARY_DIR}/alink.msg
    ${CMAKE_BINARY_DIR}/alink.rsc
)

set(PBIND_OBJECTS
    pbind.c
    ${CMAKE_BINARY_DIR}/pbind.msg
    ${CMAKE_BINARY_DIR}/pbind.rsc
)

set(P2HEX_OBJECTS
    p2hex.c
    ${CMAKE_BINARY_DIR}/p2hex.msg
    ${CMAKE_BINARY_DIR}/p2hex.rsc
)

set(P2BIN_OBJECTS
    p2bin.c
    ${CMAKE_BINARY_DIR}/p2bin.msg
    ${CMAKE_BINARY_DIR}/p2bin.rsc
)

function(define_executable NAME)
    add_executable(${NAME} ${ARGN})
    target_include_directories(${NAME}
        PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
    )
    set_target_properties(${NAME}
        PROPERTIES
        OUTPUT_NAME ${NAME}
    )

    if(CMAKE_CROSSCOMPILING)
        if(DEFINED EMSCRIPTEN)
            if(${EMSCRIPTEN})
                target_link_libraries(${NAME} PUBLIC nodefs.js noderawfs.js)
                # Preserve native command-line environment and locale overrides.
                target_link_options(${NAME} PRIVATE
                    "SHELL:--pre-js \"${CMAKE_SOURCE_DIR}/cmake/emscripten-node-env.js\""
                )
                set_property(TARGET ${NAME} APPEND PROPERTY LINK_DEPENDS
                    "${CMAKE_SOURCE_DIR}/cmake/emscripten-node-env.js"
                )
            endif(${EMSCRIPTEN})
        endif(DEFINED EMSCRIPTEN)
    endif(CMAKE_CROSSCOMPILING)
endfunction(define_executable)

add_library(asl_library STATIC
    $<TARGET_OBJECTS:AS_OBJECTS>
    $<TARGET_OBJECTS:AS_DAS_COMMON>
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:CHUNKS_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(asl_library)

add_library(ASL::asl ALIAS asl_library)
set_target_properties(asl_library PROPERTIES OUTPUT_NAME asl EXPORT_NAME asl)
target_include_directories(asl_library PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/asl>
)
define_executable(asl asl_main.c)
target_link_libraries(asl PRIVATE asl_library)

include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/ASLConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/ASLConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/ASL"
)
write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/ASLConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
)
install(TARGETS asl_library EXPORT ASLTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES asl_library.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/asl)
install(EXPORT ASLTargets NAMESPACE ASL:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ASL)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ASLConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/ASLConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ASL
)

if(EMSCRIPTEN)
    # Recursive expression evaluation needs more than Emscripten's 64 KiB stack.
    set(ASL_EMSCRIPTEN_STACK_SIZE 1048576 CACHE STRING "Assembler WebAssembly stack size in bytes")
    target_link_options(asl_library INTERFACE "-sSTACK_SIZE=${ASL_EMSCRIPTEN_STACK_SIZE}")
endif()

if(DEFINED ENV{MSYSTEM_PREFIX})
    define_executable(asw asl_main.c)
    target_link_libraries(asw PRIVATE asl_library)
    set(ASW_TARGET asw)
else()
    set(ASW_TARGET "")
endif(DEFINED ENV{MSYSTEM_PREFIX})

define_executable(dasl
    ${DAS_OBJECTS}
    $<TARGET_OBJECTS:AS_DAS_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:CHUNKS_COMMON>
)
add_dependencies(dasl das_res)

define_executable(plist
    ${PLIST_OBJECTS}
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(plist)
add_dependencies(plist plist_res)

define_executable(alink
    ${ALINK_OBJECTS}
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(alink)
add_dependencies(alink alink_res)

define_executable(pbind
    ${PBIND_OBJECTS}
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(pbind)
add_dependencies(pbind pbind_res)

define_executable(p2hex
    ${P2HEX_OBJECTS}
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:CHUNKS_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(p2hex)
add_dependencies(p2hex p2hex_res)

define_executable(p2bin
    ${P2BIN_OBJECTS}
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:CHUNKS_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
)
target_link_libm(p2bin)
add_dependencies(p2bin p2bin_res)

define_executable(test_driver
    test_driver.c
)
option(ASL_TEST_USE_LIBRARY "Assemble regression fixtures in the test driver process" ON)
if(ASL_TEST_USE_LIBRARY)
    target_compile_definitions(test_driver PRIVATE ASL_TEST_USE_LIBRARY)
    target_link_libraries(test_driver PRIVATE asl_library)
endif()

file(GLOB_RECURSE ALL_SOURCE_FILES *.c *.h *.hpp)

add_custom_target(
    asl-format
    COMMAND clang-format -style=file -i ${ALL_SOURCE_FILES}
)

add_custom_target(
    asl-tidy
    COMMAND clang-tidy --use-color ${ALL_SOURCE_FILES}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/asl
)

if(DEFINED ENV{MSYSTEM_PREFIX})
    install(
        FILES
        man/asl.1
        RENAME
        asw.1
        DESTINATION
        ${CMAKE_INSTALL_MANDIR}/man1
    )
endif(DEFINED ENV{MSYSTEM_PREFIX})

install(
    TARGETS
    alink
    asl
    ${ASW_TARGET}
    dasl
    pbind
    p2bin
    p2hex
    plist
    DESTINATION
    ${CMAKE_INSTALL_BINDIR}
)

if(NOT ASL_EMBED_MESSAGES)
install(
    FILES
    ${CMAKE_BINARY_DIR}/alink.msg
    ${CMAKE_BINARY_DIR}/as.msg
    ${CMAKE_BINARY_DIR}/das.msg
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/pbind.msg
    ${CMAKE_BINARY_DIR}/p2bin.msg
    ${CMAKE_BINARY_DIR}/p2hex.msg
    ${CMAKE_BINARY_DIR}/plist.msg
    ${CMAKE_BINARY_DIR}/tools.msg
    DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/asl
)
endif()

if(NOT CMAKE_CROSSCOMPILING)
    export(
        TARGETS rescomp
        FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake
    )
endif(NOT CMAKE_CROSSCOMPILING)

find_program(PANDOC_EXECUTABLE pandoc)
mark_as_advanced(PANDOC_EXECUTABLE)

install(FILES COPYING DESTINATION ${CMAKE_INSTALL_DOCDIR})

# TODO: Convert man pages to Markdown and install them with
# include(PandocMan)
# add_pandoc_man("${CMAKE_SOURCE_DIR}/utility.1.md")
# Source: https://github.com/rnpgp/cmake-modules/blob/main/PandocMan.cmake
install(
    DIRECTORY
    man/
    DESTINATION
    ${CMAKE_INSTALL_MANDIR}/man1
)

if(EXISTS ${PANDOC_EXECUTABLE})
    message(STATUS "Pandoc found: ${PANDOC_EXECUTABLE}, documents will be generated.")
    INCLUDE(pandocology)
    set(DOCUMENT_SOURCES
        doc/as.md
        doc/introduction.md
        doc/assembler-usage.md
        doc/pseudo-instructions.md
        doc/processor-specific-hints.md
        doc/file-formats.md
        doc/utility-programs.md
        doc/error-messages.md
        doc/pseudo-instructions-and-integer-syntax.md
        doc/predefined-symbols.md
        doc/shipped-include-files.md
        doc/frequently-asked-questions.md
        doc/modifying-as.md
        doc/programming-examples.md
        doc/bibliography.md
    )
    set(DOCUMENT_DEPENDS
        doc/asl.css
        doc/dosbat.xml
        doc/convert-links.lua
    )
    add_document(
        TARGET html_document
        OUTPUT_FILE asl.html
        SOURCES ${DOCUMENT_SOURCES}
        RESOURCE_DIRS
        ${CMAKE_SOURCE_DIR}/doc
        DEPENDS
        ${DOCUMENT_DEPENDS}
        PANDOC_DIRECTIVES
        -f gfm+footnotes -t html5 --toc --toc-depth=4 --wrap=preserve
        --syntax-definition=doc/dosbat.xml
        -c doc/asl.css
        -L doc/convert-links.lua
        --metadata title="Flamewing's Enhanced AS" --metadata lang="en"
        --self-contained --standalone
    )
    add_document(
        TARGET odt_document
        OUTPUT_FILE asl.odt
        SOURCES ${DOCUMENT_SOURCES}
        RESOURCE_DIRS
        ${CMAKE_SOURCE_DIR}/doc
        DEPENDS
        ${DOCUMENT_DEPENDS} doc/asl-template.odt
        PANDOC_DIRECTIVES
        -f gfm+footnotes -t odt --toc --toc-depth=4 --wrap=preserve
        --syntax-definition=doc/dosbat.xml
        -c doc/asl.css
        -L doc/convert-links.lua
        --metadata title="Flamewing's Enhanced AS" --metadata lang="en"
        --self-contained --standalone
        --reference-doc=doc/asl-template.odt
    )
    # Both the wrapper and the product target can copy shared document inputs.
    # Order the product target too: ordering only the wrapper leaves MSBuild
    # free to build its product dependency alongside HTML.
    add_dependencies(odt_document html_document)
    add_dependencies(asl.odt_product html_document)
    install(
        FILES
        ${CMAKE_BINARY_DIR}/asl.html
        ${CMAKE_BINARY_DIR}/asl.odt
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
    )

# ### PDF generation requires lualatex.
# ### Also, PDF output needs some love to make it look good. Tables are not
# ### rendered correctly because Pandoc uses Latex to convert to PDF, and
# ### Latex simply does not wrap lines in tables.
# ##find_package(LATEX COMPONENTS LUALATEX)
# ##if(NOT LATEX_FOUND)
# ##    message(STATUS "Lualatex not found: PDF document will not be generated.")
# ##else()
# ##    add_document(
# ##        TARGET pdf_document
# ##        OUTPUT_FILE asl.pdf
# ##        SOURCES ${DOCUMENT_SOURCES}
# ##        RESOURCE_DIRS
# ##        ${CMAKE_SOURCE_DIR}/doc
# ##        DEPENDS
# ##        ${DOCUMENT_DEPENDS}
# ##        PANDOC_DIRECTIVES
# ##        -f gfm+footnotes -t pdf --toc --toc-depth=4 --wrap=preserve
# ##        --syntax-definition=doc/dosbat.xml
# ##        -c doc/asl.css
# ##        -L doc/convert-links.lua
# ##        --self-contained --standalone
# ##        --pdf-engine=lualatex
# ##        --metadata title="Flamewing's Enhanced AS" --variable lang="en"
# ##        --variable fontsize="12pt"
# ##        --variable papersize="a4"
# ##        --variable mainfont="DejaVuSerif"
# ##        --variable sansfont="DejaVuSans"
# ##        --variable monofont="DejaVuSansMono"
# ##        --variable mathfont="TeXGyreDejaVuMath-Regular"
# ##        --variable margin-left="1cm"
# ##        --variable margin-right="1cm"
# ##        --variable margin-top="1cm"
# ##        --variable margin-bottom="2cm"
# ##    )
# ##    install(
# ##        FILES
# ##        ${CMAKE_BINARY_DIR}/asl.pdf
# ##        DESTINATION ${CMAKE_INSTALL_DOCDIR}
# ##    )
# ##endif()
else()
    message(STATUS "Pandoc not found: documents will not be generated.")
endif()

enable_testing()
include(BackendTests)
include(tests/RegressionTests.cmake)

asl_test_available(library_driver can_run REQUIRES M68K)
if(ASL_TEST_USE_LIBRARY AND can_run)
    add_test(NAME library_driver COMMAND ${CMAKE_COMMAND}
        "-DTEST_PROGRAM=$<TARGET_FILE:test_driver>"
        "-DASL_PROGRAM=$<TARGET_FILE:asl>"
        "-DP2BIN_PROGRAM=$<TARGET_FILE:p2bin>"
        "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
        "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/LibraryDriver"
        -P "${CMAKE_SOURCE_DIR}/tests/library_driver.cmake"
    )
    set_tests_properties(library_driver PROPERTIES ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR}")
endif()

define_executable(test_asl_library tests/library.c)
target_link_libraries(test_asl_library PRIVATE asl_library)
asl_test_available(library_lifecycle can_run REQUIRES M68K Z80)
if(can_run)
    add_test(NAME library_lifecycle COMMAND ${CMAKE_COMMAND}
        "-DTEST_PROGRAM=$<TARGET_FILE:test_asl_library>"
        "-DASL_PROGRAM=$<TARGET_FILE:asl>"
        "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
        "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/Library"
        -P "${CMAKE_SOURCE_DIR}/tests/library.cmake"
    )
    set_tests_properties(library_lifecycle PROPERTIES
        ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};LC_MESSAGES=en;LC_ALL=C"
    )
endif()

add_test(NAME backend_selection COMMAND ${CMAKE_COMMAND}
    "-DASL_PROGRAM=$<TARGET_FILE:asl>"
    "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
    "-DENABLED_BACKENDS=${ASL_ENABLED_BACKENDS}"
    -P "${CMAKE_SOURCE_DIR}/tests/backend_selection.cmake"
)
set_tests_properties(backend_selection PROPERTIES
    ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};LC_ALL=C"
)

asl_add_script_test(st7_diagnostics
    DIRECTORY ST7Diagnostics
    REQUIRES ST7
)

asl_add_script_test(mn1610_diagnostics
    DIRECTORY MN1610Diagnostics
    REQUIRES MN1610
)

asl_add_script_test(st9_diagnostics
    DIRECTORY ST9Diagnostics
    REQUIRES ST9
)

asl_add_script_test(tc9331_diagnostics
    DIRECTORY TC9331Diagnostics
    REQUIRES 9331
)

asl_add_script_test(tlcs90_diagnostics
    DIRECTORY TLCS90Diagnostics
    REQUIRES 90C141
)

asl_add_script_test(mcore_diagnostics
    DIRECTORY MCoreDiagnostics
    REQUIRES MCORE
)

asl_add_script_test(z8_diagnostics
    DIRECTORY Z8Diagnostics
    REQUIRES M68K Z8
)

asl_add_script_test(konami_diagnostics
    DIRECTORY KonamiDiagnostics
    REQUIRES KONAMI
)

asl_add_script_test(imp16_diagnostics
    DIRECTORY IMP16Diagnostics
    REQUIRES AVR IMP16
)

asl_add_script_test(avr_diagnostics
    DIRECTORY AVRDiagnostics
    REQUIRES AVR M68K
)

asl_add_script_test(m68k_diagnostics
    DIRECTORY M68KDiagnostics
    REQUIRES AVR M68K
)

asl_add_script_test(numeric_diagnostics
    DIRECTORY NumericDiagnostics
    REQUIRES 51 601 6812 M6800 PPS4
)

asl_add_script_test(data_diagnostics
    DIRECTORY DataDiagnostics
    REQUIRES 51 M6800 M68K S12Z Z80
)

asl_add_script_test(function_diagnostics
    DIRECTORY FunctionDiagnostics
    REQUIRES M68K
)

asl_add_script_test(cpu_operand_diagnostics
    DIRECTORY CPUOperandDiagnostics
    REQUIRES 3205X AVR
)

asl_add_script_test(branchhc16_diagnostics
    DIRECTORY BranchHC16Diagnostics
    REQUIRES 6816
)

asl_add_script_test(ucom_diagnostics
    DIRECTORY UCOMDiagnostics
    REQUIRES UC43
)

asl_add_script_test(kcpsm_diagnostics
    DIRECTORY KCPSMDiagnostics
    REQUIRES KCPSM
)

asl_add_script_test(cp3f_diagnostics
    DIRECTORY CP3FDiagnostics
    REQUIRES CP3F
)

asl_add_script_test(hmcs400_diagnostics
    DIRECTORY HMCS400Diagnostics
    REQUIRES HMCS400
)

asl_add_script_test(am29k_diagnostics
    DIRECTORY AM29KDiagnostics
    REQUIRES 29K
)

asl_add_script_test(cop4_diagnostics
    DIRECTORY COP4Diagnostics
    REQUIRES COP4
)

asl_add_script_test(mico8_diagnostics
    DIRECTORY MICO8Diagnostics
    REQUIRES MICO8
)

asl_add_script_test(tms3201x_diagnostics
    DIRECTORY TMS3201XDiagnostics
    REQUIRES 3201X
)

asl_add_script_test(tms3202x_diagnostics
    DIRECTORY TMS3202XDiagnostics
    REQUIRES 3202X
)

asl_add_script_test(nec75xx_diagnostics
    DIRECTORY NEC75XXDiagnostics
    REQUIRES 75XX
)

asl_add_script_test(xgate_diagnostics
    DIRECTORY XGATEDiagnostics
    REQUIRES XGATE
)

asl_add_script_test(cop8_diagnostics
    DIRECTORY COP8Diagnostics
    REQUIRES COP8
)

asl_add_script_test(f8_diagnostics
    DIRECTORY F8Diagnostics
    REQUIRES F8
)

asl_add_script_test(olms40_diagnostics
    DIRECTORY OLMS40Diagnostics
    REQUIRES OLMS40
)

asl_add_script_test(im6100_diagnostics
    DIRECTORY IM6100Diagnostics
    REQUIRES 6100
)

asl_add_script_test(palm_diagnostics
    DIRECTORY PALMDiagnostics
    REQUIRES PALM
)

asl_add_script_test(xcore_diagnostics
    DIRECTORY XCoreDiagnostics
    REQUIRES XCORE
)

asl_add_script_test(mil1750_diagnostics
    DIRECTORY MIL1750Diagnostics
    REQUIRES 1750
)

asl_add_script_test(upd772x_diagnostics
    DIRECTORY UPD772xDiagnostics
    REQUIRES 7720
)

asl_add_script_test(olms50_diagnostics
    DIRECTORY OLMS50Diagnostics
    REQUIRES OLMS50
)

asl_add_script_test(ns32000_diagnostics
    DIRECTORY NS32000Diagnostics
    REQUIRES NS32K
)


asl_add_script_test(vax_diagnostics
    DIRECTORY VAXDiagnostics
    REQUIRES VAX
)


asl_add_script_test(melps7700_diagnostics
    DIRECTORY MELPS7700Diagnostics
    REQUIRES 7700
)


asl_add_script_test(ins8070_diagnostics
    DIRECTORY INS8070Diagnostics
    REQUIRES 807X
)


asl_add_script_test(tlcs870c_diagnostics
    DIRECTORY TLCS870CDiagnostics
    REQUIRES 870C
)

asl_add_script_test(cr16_diagnostics
    DIRECTORY CR16Diagnostics
    REQUIRES CR16
)

asl_add_script_test(national_pseudo
    DIRECTORY NationalPseudo
    REQUIRES CR16 CR16C
)

asl_add_script_test(mcs96_diagnostics
    DIRECTORY MCS96Diagnostics
    REQUIRES 96
)

asl_add_script_test(powerpc_diagnostics
    DIRECTORY PowerPCDiagnostics
    REQUIRES 601
)


asl_add_script_test(tms320c54x_diagnostics
    DIRECTORY TMS320C54xDiagnostics
    REQUIRES 32054X
)


asl_add_script_test(intel8008_diagnostics
    DIRECTORY Intel8008Diagnostics
    REQUIRES 8008
)


asl_add_script_test(mn1610alt_diagnostics
    DIRECTORY MN1610AltDiagnostics
    REQUIRES MN2610
)


asl_add_script_test(signetics2650_diagnostics
    DIRECTORY Signetics2650Diagnostics
    REQUIRES 2650
)


asl_add_script_test(pdp11_diagnostics
    DIRECTORY PDP11Diagnostics
    REQUIRES PDP11
)


asl_add_script_test(cp1600_diagnostics
    DIRECTORY CP1600Diagnostics
    REQUIRES CP1600
)


asl_add_script_test(tms9900_diagnostics
    DIRECTORY TMS9900Diagnostics
    REQUIRES 9900
)

asl_add_script_test(signetics8x30x_diagnostics
    DIRECTORY Signetics8x30xDiagnostics
    REQUIRES 8X30X
)

asl_add_script_test(sx20_diagnostics
    DIRECTORY SX20Diagnostics
    REQUIRES SX20
)

asl_add_script_test(tms370_diagnostics
    DIRECTORY TMS370Diagnostics
    REQUIRES 370
)

asl_add_script_test(ti_pseudo_diagnostics
    DIRECTORY TIPseudoDiagnostics
    REQUIRES 3202X 3203X
)

asl_add_script_test(i960_diagnostics
    DIRECTORY I960Diagnostics
    REQUIRES 960
)

asl_add_script_test(sc62015_diagnostics
    DIRECTORY SC62015Diagnostics
    REQUIRES 62015
)

asl_add_script_test(kenbak_diagnostics
    DIRECTORY KENBAKDiagnostics
    REQUIRES KENBAK
)

asl_add_script_test(f2mc8l_diagnostics
    DIRECTORY F2MC8LDiagnostics
    REQUIRES F2MC8
)

asl_add_script_test(st6_diagnostics
    DIRECTORY ST6Diagnostics
    REQUIRES ST6
)

asl_add_script_test(padauk_diagnostics
    DIRECTORY PadaukDiagnostics
    REQUIRES PDK
)

asl_add_script_test(scmp_diagnostics
    DIRECTORY SCMPDiagnostics
    REQUIRES SCMP
)

asl_add_script_test(nec77230_diagnostics
    DIRECTORY NEC77230Diagnostics
    REQUIRES 77230
)

asl_add_script_test(tms320c5x_diagnostics
    DIRECTORY TMS320C5xDiagnostics
    REQUIRES 3205X
)

asl_add_script_test(tms7000_diagnostics
    DIRECTORY TMS7000Diagnostics
    REQUIRES TMS7
)

asl_add_script_test(ace_diagnostics
    DIRECTORY ACEDiagnostics
    REQUIRES ACE
)

asl_add_script_test(tlcs42_diagnostics
    DIRECTORY TLCS42Diagnostics
    REQUIRES 42C00
)

asl_add_script_test(operand4004_diagnostics
    DIRECTORY Operand4004Diagnostics
    REQUIRES 4004
)

asl_add_script_test(tbil_diagnostics
    DIRECTORY TBILDiagnostics
    REQUIRES TBIL
)

asl_add_script_test(branch61860_diagnostics
    DIRECTORY Branch61860Diagnostics
    REQUIRES 61860
)

asl_add_script_test(fixed_operand_diagnostics
    DIRECTORY FixedOperandDiagnostics
    REQUIRES 1802 KCPSM3 NANO
)

asl_add_script_test(pic_vector_diagnostics
    DIRECTORY PICVectorDiagnostics
    REQUIRES 16C5X 16C8X VECTOR
)

asl_add_script_test(equate_data_diagnostics
    DIRECTORY EquateDataDiagnostics
    REQUIRES 1802 4004
)

asl_add_script_test(expression_range_diagnostics
    DIRECTORY ExpressionRangeDiagnostics
    REQUIRES 68RS08
)

asl_add_script_test(shared_contract_diagnostics
    DIRECTORY SharedContractDiagnostics
    REQUIRES 166 68RS08
)

asl_add_script_test(operandrs08_diagnostics
    DIRECTORY OperandRS08Diagnostics
    REQUIRES 68RS08
)

asl_add_script_test(operand6804_diagnostics
    DIRECTORY Operand6804Diagnostics
    REQUIRES 6804
)

asl_add_script_test(branch6804_diagnostics
    DIRECTORY Branch6804Diagnostics
    REQUIRES 6804
)

asl_add_script_test(branchrs08_diagnostics
    DIRECTORY BranchRS08Diagnostics
    REQUIRES 68RS08
)

asl_add_script_test(branch6809_diagnostics
    DIRECTORY Branch6809Diagnostics
    REQUIRES 6809 KONAMI
)

asl_add_script_test(branch6805_diagnostics
    DIRECTORY Branch6805Diagnostics
    REQUIRES 6805
)

asl_add_script_test(branch68_diagnostics
    DIRECTORY Branch68Diagnostics
    REQUIRES M6800
)

asl_add_script_test(hc12_branch_diagnostics
    DIRECTORY HC12BranchDiagnostics
    REQUIRES 6812
)

asl_add_script_test(melps740_nop
    DIRECTORY MELPS740NOP
    REQUIRES 6502
)

asl_add_script_test(branch65_diagnostics
    DIRECTORY Branch65Diagnostics
    REQUIRES 6502
)

asl_add_script_test(backend_delimiter_diagnostics
    DIRECTORY BackendDelimiterDiagnostics
    REQUIRES 370 8X30X 960 9900 KCPSM NS32K TMS7 VECTOR Z80
)

asl_add_script_test(bracket_diagnostics
    DIRECTORY BracketDiagnostics
    REQUIRES 78K2 78K3 78K4 86 KENBAK M68K PDK Z8 Z80
)

asl_add_script_test(mapping_diagnostics
    DIRECTORY MappingDiagnostics
    REQUIRES MSP Z80
)

define_executable(test_coverity_inputs
    tests/coverity_inputs.c
    codechunks.c
    $<TARGET_OBJECTS:ST_OBJECTS>
    $<TARGET_OBJECTS:NLS_OBJECTS>
    $<TARGET_OBJECTS:CMDLINE_COMMON>
    $<TARGET_OBJECTS:TOOLS_COMMON>
    $<TARGET_OBJECTS:HEADIDS_COMMON>
    $<TARGET_OBJECTS:ADDRSPACE_COMMON>
)
target_link_libm(test_coverity_inputs)
asl_test_available(coverity_inputs can_run REQUIRES 6812 96 M16 M16C MSP RX Z80)
if(can_run)
    add_test(NAME coverity_inputs COMMAND ${CMAKE_COMMAND}
        "-DTEST_PROGRAM=$<TARGET_FILE:test_coverity_inputs>"
        "-DASL_PROGRAM=$<TARGET_FILE:asl>"
        "-DDAS_PROGRAM=$<TARGET_FILE:dasl>"
        "-DP2BIN_PROGRAM=$<TARGET_FILE:p2bin>"
        "-DP2HEX_PROGRAM=$<TARGET_FILE:p2hex>"
        "-DALINK_PROGRAM=$<TARGET_FILE:alink>"
        "-DPBIND_PROGRAM=$<TARGET_FILE:pbind>"
        "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
        "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/CoverityInputs"
        -P "${CMAKE_SOURCE_DIR}/tests/coverity_inputs.cmake"
    )
    set_tests_properties(coverity_inputs PROPERTIES
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};ASL_COVERITY_TEST_CMD="
    )
endif()

define_executable(test_driver_io tests/test_driver_io.c)
if(LOCALE_NLS)
    define_executable(test_nls_locale tests/nls_locale.c chardefs.c $<TARGET_OBJECTS:ST_OBJECTS>)
    target_link_libm(test_nls_locale)
    add_test(NAME nls_locale COMMAND test_nls_locale)
    set_tests_properties(nls_locale PROPERTIES ENVIRONMENT "TZ=UTC0")
endif()
add_test(NAME test_driver_io COMMAND ${CMAKE_COMMAND}
    "-DTEST_PROGRAM=$<TARGET_FILE:test_driver_io>"
    "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
    "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/DriverIO"
    -P "${CMAKE_SOURCE_DIR}/tests/test_driver_io.cmake"
)

define_executable(test_forward_refs
    tests/forward_refs.c trees.c
)
add_test(NAME forward_refs COMMAND test_forward_refs)

define_executable(test_strmaxprep2
    tests/strmaxprep2.c
    strutil.c
    dynstr.c
)
add_test(NAME strmaxprep2 COMMAND test_strmaxprep2)

define_executable(test_formatter_contracts tests/formatter_contracts.c strutil.c dynstr.c)
add_test(NAME formatter_contracts COMMAND test_formatter_contracts)

define_executable(test_strcomp_span
    tests/strcomp_span.c
    strcomp.c strutil.c dynstr.c
)
add_test(NAME strcomp_span COMMAND test_strcomp_span)

define_executable(test_space_classification
    tests/space_classification.c strutil.c dynstr.c
)
add_test(NAME space_classification COMMAND test_space_classification)

define_executable(test_strref tests/strref.c strutil.c dynstr.c)
add_test(NAME strref COMMAND test_strref)

define_executable(test_strcomp_view tests/strcomp_view.c striter.c strutil.c dynstr.c)
add_test(NAME strcomp_view COMMAND test_strcomp_view)

define_executable(test_intconst_span tests/intconst_span.c intformat.c int_type.c strutil.c dynstr.c)
add_test(NAME intconst_span COMMAND test_intconst_span)

define_executable(test_formatter
    tests/formatter.c
    strutil.c
    dynstr.c
)
add_test(NAME formatter COMMAND test_formatter)
set_tests_properties(formatter PROPERTIES TIMEOUT 15)

define_executable(test_float_subnormal
    tests/float_subnormal.c
    as_float.c bfloat.c ieeefloat.c strutil.c dynstr.c
)
target_link_libm(test_float_subnormal)
add_test(NAME float_subnormal COMMAND test_float_subnormal)

define_executable(test_cpu2phys
    tests/cpu2phys.c
    cpu2phys.c tempresult.c nonzstring.c strutil.c dynstr.c
)
add_test(NAME cpu2phys COMMAND test_cpu2phys)

define_executable(test_tempresult_float
    tests/tempresult_float.c tempresult.c nonzstring.c strutil.c dynstr.c
)
target_link_libm(test_tempresult_float)
add_test(NAME tempresult_float COMMAND test_tempresult_float)

asl_test_available(assembler_cli can_run REQUIRES M68K)
if(can_run)
    add_test(NAME assembler_cli COMMAND ${CMAKE_COMMAND}
        "-DASL_PROGRAM=$<TARGET_FILE:asl>"
        "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
        "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/AssemblerCLI"
        -P "${CMAKE_SOURCE_DIR}/tests/assembler_cli.cmake"
    )
    set_tests_properties(assembler_cli PROPERTIES
        ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};LC_ALL=C"
    )
endif()

add_test(NAME utility_cli COMMAND ${CMAKE_COMMAND}
    "-DPLIST_PROGRAM=$<TARGET_FILE:plist>"
    "-DPBIND_PROGRAM=$<TARGET_FILE:pbind>"
    "-DP2BIN_PROGRAM=$<TARGET_FILE:p2bin>"
    "-DP2HEX_PROGRAM=$<TARGET_FILE:p2hex>"
    "-DALINK_PROGRAM=$<TARGET_FILE:alink>"
    "-DDASL_PROGRAM=$<TARGET_FILE:dasl>"
    "-DRESCOMP_PROGRAM=$<TARGET_FILE:rescomp>"
    "-DTEST_EMULATOR=${CMAKE_CROSSCOMPILING_EMULATOR}"
    "-DTEST_DIR=${CMAKE_BINARY_DIR}/Testing/UtilityCLI"
    -P "${CMAKE_SOURCE_DIR}/tests/utility_cli.cmake"
)
set_tests_properties(utility_cli PROPERTIES
    ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR};LC_ALL=C"
)

define_executable(test_nonzstring
    tests/nonzstring.c
    nonzstring.c strutil.c dynstr.c
)
add_test(NAME nonzstring COMMAND test_nonzstring)

if(DEFINED ENV{VERBOSE})
    set(VERBOSE_MODE $ENV{VERBOSE})
else()
    set(VERBOSE_MODE "0")
endif()

file(GLOB test_dirs RELATIVE "${CMAKE_SOURCE_DIR}/tests" "${CMAKE_SOURCE_DIR}/tests/*")

if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    set(EMULATOR "")
elseif(DEFINED EMSCRIPTEN)
    if(${EMSCRIPTEN})
        set(EMULATOR "${CMAKE_CROSSCOMPILING_EMULATOR} ")
    else()
        set(EMULATOR "")
    endif()
else()
    set(EMULATOR "")
endif()

foreach(TEST_TARGET ${test_dirs})
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/${TEST_TARGET}/${TEST_TARGET}.asm" AND EXISTS "${CMAKE_SOURCE_DIR}/tests/${TEST_TARGET}/${TEST_TARGET}.ori")
        if(NOT DEFINED ASL_TEST_${TEST_TARGET}_REQUIRES)
            message(FATAL_ERROR "Missing backend requirements for regression ${TEST_TARGET}")
        endif()
        asl_test_available(${TEST_TARGET} can_run REQUIRES ${ASL_TEST_${TEST_TARGET}_REQUIRES})
        if(NOT can_run)
            continue()
        endif()
        add_test(
            NAME ${TEST_TARGET}
            COMMAND
            test_driver ${VERBOSE_MODE} ${CMAKE_SOURCE_DIR}/tests/${TEST_TARGET} ${TEST_TARGET} ${CMAKE_SOURCE_DIR}/include "${EMULATOR}$<TARGET_FILE:asl>" "${EMULATOR}$<TARGET_FILE:p2bin>" ${CMAKE_BINARY_DIR}/Testing
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        set_property(
            TEST ${TEST_TARGET}
            PROPERTY
            ENVIRONMENT "AS_MSGPATH=${CMAKE_BINARY_DIR}"
        )
    endif()
endforeach()

asl_report_test_selection()
