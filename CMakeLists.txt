cmake_minimum_required(VERSION 3.19)

project(asl-flamewing
    VERSION 1.42.212
    DESCRIPTION "Improved/bugfixed version of Alfred Arnold's The Macro Assembler AS"
    LANGUAGES C
)

# Joins arguments and places the results in ${result_var}.
function(join result_var)
    set(result "")
    foreach (arg ${ARGN})
        set(result "${result}${arg}")
    endforeach ()
    set(${result_var} "${result}" PARENT_SCOPE)
endfunction()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_C_EXTENSIONS FALSE)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
# include(CheckIPOSupported)
# check_ipo_supported(RESULT iposupported OUTPUT ipoerror)

# if(iposupported)
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
#     add_link_options(-fuse-ld=lld)
#     set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
# else()
#     message(WARNING "IPO is not supported: ${ipoerror}")
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
        add_compile_options($<COMPILE_LANGUAGE:C>:"${FLAG}")
    endif(C_COMPILER_SUPPORTS_${FLAG_NO_SIGNS})
endfunction()

add_compile_definitions(LIBDIR="${CMAKE_INSTALL_FULL_LIBDIR}")

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

if (NEED_LINKING_AGAINST_LIBM)
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
        add_compile_options_safe(-pedantic-errors)
    endif()

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        add_compile_options_safe(-Og)
        add_compile_options_safe(-g3)
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
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wc99-c11-compat>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wc++-compat>)

        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Waddress>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Waggregate-return>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Walloc-zero>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Walloca>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wanalyzer-too-complex>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Warith-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wbad-function-cast>)
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wbidi-chars>)
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wbidi-chars=any>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-align>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-align=strict>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcast-qual>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wchkp>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcomments>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wcomment>)
        #add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wconversion>)
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
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Winvalid-utf8>)
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
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wopenacc-parallelism>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpacked>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wpacked-bitfield-compat>)
        # add_compile_option$<COMPILE_LANGUAGE:C>:s($<$<COMPILE_LANGUAGE:C>:-Wpadded>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wredundant-decls>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=compatible-local>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=global>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshadow=local>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wshift-negative-value>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wsign-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstack-protector>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wstrict-aliasing>)
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wstrict-flex-arrays>)
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
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wswitch-enum>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-system-headers>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtraditional>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtraditional-conversion>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wtrampolines>)
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wtrivial-auto-var-init>)
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
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Wdangling-pointer=2>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-overflow=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat-truncation=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wformat=1>)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wimplicit-fallthrough=3>)
        # add_compile_options($<$<COMPILE_LANGUAGE:C>:-Woverloaded-virtual>)
        # add_compile_options_safe($<$<COMPILE_LANGUAGE:C>:-Woverloaded-virtual=2>)
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

INCLUDE (CheckIncludeFiles)
set(CHECK_LOCALE TRUE)
if (CMAKE_CROSSCOMPILING)
    if (DEFINED EMSCRIPTEN)
        if (${EMSCRIPTEN})
            set(CHECK_LOCALE FALSE)
        endif(${EMSCRIPTEN})
    endif()
endif()
if(${CHECK_LOCALE})
    CHECK_INCLUDE_FILES ("windef.h;winbase.h;winnls.h" W32_NLS)
    CHECK_INCLUDE_FILES("locale.h;langinfo.h" LOCALE_NLS)
    CHECK_INCLUDE_FILES(os2.h OS2_NLS)
else()
    set(LOCALE_NLS FALSE)
    set(OS2_NLS FALSE)
    set(W32_NLS FALSE)
endif(${CHECK_LOCALE})
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)
set(ARCHPRNAME "${CMAKE_SYSTEM_PROCESSOR}")
set(ARCHSYSNAME "${CMAKE_SYSTEM_NAME}")
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

# only build the generator if not crosscompiling
if(NOT CMAKE_CROSSCOMPILING)
    set(RESCOMP_OBJECTS
        rescomp.c
        endian.c
        endian.h
        strutil.c
        strutil.h
        dynstr.c
        dynstr.h
        bpemu.c
        bpemu.h
    )

    add_executable(rescomp
        ${RESCOMP_OBJECTS}
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

function(define_rescomp_target STEM)
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${STEM}.msg
        COMMAND rescomp ${STEM}.res -m ${CMAKE_BINARY_DIR}/${STEM}.msg
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS rescomp ${STEM}.res ${ARGN}
    )
    add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${STEM}.rsc
        COMMAND rescomp ${STEM}.res -h ${CMAKE_BINARY_DIR}/${STEM}.rsc
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        DEPENDS rescomp ${STEM}.res ${ARGN}
    )
endfunction(define_rescomp_target)

define_rescomp_target(ioerrs header.res)
define_rescomp_target(cmdarg header.res)
define_rescomp_target(tools header.res)
define_rescomp_target(as header.res)
define_rescomp_target(das header.res)
define_rescomp_target(plist tools2.res header.res)
define_rescomp_target(alink tools2.res header.res)
define_rescomp_target(pbind tools2.res header.res)
define_rescomp_target(p2hex tools2.res header.res)
define_rescomp_target(p2bin tools2.res header.res)

set(CODE_OBJECTS
    ${CMAKE_BINARY_DIR}/as.msg
    ${CMAKE_BINARY_DIR}/as.rsc
    codepseudo.c
    codepseudo.h
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
    ieeefloat.c
    ieeefloat.h
    vaxfloat.c
    vaxfloat.h
    codevars.c
    codevars.h
    code68k.c
    code68k.h
    code68k.hpp
    code56k.c
    code56k.h
    code601.c
    code601.h
    codemcore.c
    codemcore.h
    codexgate.c
    codexgate.h
    code68.c
    code68.h
    code6805.c
    code6805.h
    code6809.c
    code6809.h
    code6812.c
    code6812.h
    codes12z.c
    codes12z.h
    code6816.c
    code6816.h
    code68rs08.c
    code68rs08.h
    codeh8_3.c
    codeh8_3.h
    codeh8_5.c
    codeh8_5.h
    code7000.c
    code7000.h
    code65.c
    code65.h
    codeh16.c
    codeh16.h
    codeh16.hpp
    code7700.c
    code7700.h
    codehmcs400.c
    codehmcs400.h
    code4500.c
    code4500.h
    codem16.c
    codem16.h
    codem16c.c
    codem16c.h
    code4004.c
    code4004.h
    code8008.c
    code8008.h
    code48.c
    code48.h
    code51.c
    code51.h
    code96.c
    code96.h
    code85.c
    code85.h
    code86.c
    code86.h
    code960.c
    code960.h
    code8x30x.c
    code8x30x.h
    code2650.c
    code2650.h
    codexa.c
    codexa.h
    codeavr.c
    codeavr.h
    code29k.c
    code29k.h
    code166.c
    code166.h
    codez80.c
    codez80.h
    codez80.hpp
    codez8.c
    codez8.h
    codez8.hpp
    codez8000.c
    codez8000.h
    codez8000.hpp
    codekcpsm.c
    codekcpsm.h
    codekcp3.c
    codekcp3.h
    codemic8.c
    codemic8.h
    code96c141.c
    code96c141.h
    code90c141.c
    code90c141.h
    code87c800.c
    code87c800.h
    code870c.c
    code870c.h
    code47c00.c
    code47c00.h
    code97c241.c
    code97c241.h
    code9331.c
    code9331.h
    code16c5x.c
    code16c5x.h
    code16c8x.c
    code16c8x.h
    code17c4x.c
    code17c4x.h
    codesx20.c
    codesx20.h
    codepdk.c
    codepdk.h
    codest6.c
    codest6.h
    codest7.c
    codest7.h
    codest9.c
    codest9.h
    code6804.c
    code6804.h
    code3201x.c
    code3201x.h
    code3202x.c
    code3202x.h
    code3203x.c
    code3203x.h
    code3205x.c
    code3205x.h
    code3254x.c
    code3254x.h
    code3206x.c
    code3206x.h
    code3206x.hpp
    code9900.c
    code9900.h
    codetms7.c
    codetms7.h
    code370.c
    code370.h
    codemsp.c
    codemsp.h
    codetms1.c
    codetms1.h
    code78c10.c
    code78c10.h
    code75xx.c
    code75xx.h
    code75k0.c
    code75k0.h
    code78k0.c
    code78k0.h
    code78k2.c
    code78k2.h
    code78k3.c
    code78k3.h
    code78k4.c
    code78k4.h
    code7720.c
    code7720.h
    code77230.c
    code77230.h
    codescmp.c
    codescmp.h
    code807x.c
    code807x.h
    codecop4.c
    codecop4.h
    codecop8.c
    codecop8.h
    codesc14xxx.c
    codesc14xxx.h
    codens32k.c
    codens32k.h
    codens32k.hpp
    codeace.c
    codeace.h
    codef8.c
    codef8.h
    code53c8xx.c
    code53c8xx.h
    codefmc8.c
    codefmc8.h
    codefmc16.c
    codefmc16.h
    codemn1610.c
    codemn1610.h
    codemn2610.c
    codemn2610.h
    codeol40.c
    codeol40.h
    codeol50.c
    codeol50.h
    code1802.c
    code1802.h
    codevector.c
    codevector.h
    codexcore.c
    codexcore.h
    code1750.c
    code1750.h
    codekenbak.c
    codekenbak.h
    codecp1600.c
    codecp1600.h
)

set(DECODE_OBJECTS
    deco68.c
    deco68.h
    deco87c800.c
    deco87c800.h
    deco4004.c
    deco4004.h
)

set(ST_OBJECTS
    strutil.c
    strutil.h
    stdhandl.c
    stdhandl.h
    stringlists.c
    stringlists.h
    strcomp.c
    strcomp.h
)

set(NLS_OBJECTS
    nls.c
    nls.h
    nlmessages.c
    nlmessages.h
    chardefs.c
    chardefs.h
)

set(ASM_OBJECTS
    as.c
    as.h
    ${CMAKE_BINARY_DIR}/as.msg
    ${CMAKE_BINARY_DIR}/as.rsc
    asmdef.c
    asmdef.h
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
    addrspace.c
    addrspace.h
)

set(AS_OBJECTS
    ${CMAKE_BINARY_DIR}/as.msg
    ${CMAKE_BINARY_DIR}/as.rsc
    version.c
    version.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    math64.c
    math64.h
    chunks.c
    chunks.h
    console.c
    console.h
    cpulist.c
    cpulist.h
    errmsg.c
    errmsg.h
    trees.c
    trees.h
    nonzstring.c
    nonzstring.h
    dynstr.c
    dynstr.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
    headids.c
    headids.h
    sha1.c
    sha1.h
)

set(DASM_OBJECTS
    das.c
    ${CMAKE_BINARY_DIR}/das.msg
    ${CMAKE_BINARY_DIR}/das.rsc
    dasmdef.c
    dasmdef.h
)

set(DAS_OBJECTS
    version.c
    version.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    chunks.c
    chunks.h
    trees.c
    trees.h
    codechunks.c
    codechunks.h
    console.c
    console.h
    entryaddress.c
    entryaddress.h
    invaddress.c
    invaddress.h
    cpulist.c
    cpulist.h
    nonzstring.c
    nonzstring.h
    dynstr.c
    dynstr.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)

set(PLIST_OBJECTS
    plist.c
    ${CMAKE_BINARY_DIR}/plist.msg
    ${CMAKE_BINARY_DIR}/plist.rsc
    version.c
    version.h
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
    addrspace.c
    addrspace.h
    headids.c
    headids.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    dynstr.c
    dynstr.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)

set(ALINK_OBJECTS
    alink.c
    ${CMAKE_BINARY_DIR}/alink.msg
    ${CMAKE_BINARY_DIR}/alink.rsc
    version.c
    version.h
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
    addrspace.c
    addrspace.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    dynstr.c
    dynstr.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)

set(PBIND_OBJECTS
    pbind.c
    ${CMAKE_BINARY_DIR}/pbind.msg
    ${CMAKE_BINARY_DIR}/pbind.rsc
    version.c
    version.h
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
    addrspace.c
    addrspace.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    dynstr.c
    dynstr.h
    stdhandl.c
    stdhandl.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)

set(P2HEX_OBJECTS
    p2hex.c
    ${CMAKE_BINARY_DIR}/p2hex.msg
    ${CMAKE_BINARY_DIR}/p2hex.rsc
    version.c
    version.h
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
    addrspace.c
    addrspace.h
    headids.c
    headids.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    dynstr.c
    dynstr.h
    chunks.c
    chunks.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
)

set(P2BIN_OBJECTS
    p2bin.c
    ${CMAKE_BINARY_DIR}/p2bin.msg
    ${CMAKE_BINARY_DIR}/p2bin.rsc
    version.c
    version.h
    toolutils.c
    toolutils.h
    ${CMAKE_BINARY_DIR}/tools.msg
    ${CMAKE_BINARY_DIR}/tools.rsc
    addrspace.c
    addrspace.h
    endian.c
    endian.h
    bpemu.c
    bpemu.h
    stringlists.c
    stringlists.h
    strutil.c
    strutil.h
    dynstr.c
    dynstr.h
    chunks.c
    chunks.h
    cmdarg.c
    cmdarg.h
    ${CMAKE_BINARY_DIR}/cmdarg.msg
    ${CMAKE_BINARY_DIR}/cmdarg.rsc
    ioerrs.c
    ioerrs.h
    ${CMAKE_BINARY_DIR}/ioerrs.msg
    ${CMAKE_BINARY_DIR}/ioerrs.rsc
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
    if (CMAKE_CROSSCOMPILING)
        if (DEFINED EMSCRIPTEN)
            if (${EMSCRIPTEN})
                target_link_libraries(${NAME} PUBLIC nodefs.js noderawfs.js)
            endif(${EMSCRIPTEN})
        endif()
    endif()
endfunction(define_executable)

define_executable(asl
    ${ASM_OBJECTS} ${AS_OBJECTS} ${ST_OBJECTS} ${CODE_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(asl)

if(DEFINED ENV{MSYSTEM_PREFIX})
    define_executable(asw
        ${ASM_OBJECTS} ${AS_OBJECTS} ${ST_OBJECTS} ${CODE_OBJECTS} ${NLS_OBJECTS}
    )
    target_link_libm(asw)
    set(ASW_TARGET asw)
else()
    set(ASW_TARGET "")
endif(DEFINED ENV{MSYSTEM_PREFIX})

define_executable(dasl
    ${DASM_OBJECTS} ${DAS_OBJECTS} ${ST_OBJECTS} ${DECODE_OBJECTS} ${NLS_OBJECTS}
)

define_executable(plist
    ${PLIST_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(plist)

define_executable(alink
    ${ALINK_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(alink)

define_executable(pbind
    ${PBIND_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(pbind)


define_executable(p2hex
    ${P2HEX_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(p2hex)


define_executable(p2bin
    ${P2BIN_OBJECTS} ${NLS_OBJECTS}
)
target_link_libm(p2bin)

define_executable(test_driver
    test_driver.c
)

file(GLOB_RECURSE ALL_RESOURCE_FILES *.c *.h *.hpp)

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

if(NOT CMAKE_CROSSCOMPILING)
    export(
        TARGETS rescomp
        FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake
    )
endif(NOT CMAKE_CROSSCOMPILING)

# TODO: Add generation and installation of documents
# They should go to ${CMAKE_INSTALL_DOCDIR}
# List of documents:
# - COPYING                         -> ${CMAKE_INSTALL_DOCDIR}/COPYING
# - doc_COM/biblio.tex              -> ${CMAKE_INSTALL_DOCDIR}/biblio.tex
# - doc_COM/ps*.tex                 -> ${CMAKE_INSTALL_DOCDIR}/ps*.tex
# - doc_COM/taborg*.tex             -> ${CMAKE_INSTALL_DOCDIR}/taborg*.tex
# - doc_EN/as.{doc,html,dvi,ps,pdf} -> ${CMAKE_INSTALL_DOCDIR}/as_EN.{doc,html,dvi,ps,pdf}
# - doc_DE/as.{doc,html,dvi,ps,pdf} -> ${CMAKE_INSTALL_DOCDIR}/as_DE.{doc,html,dvi,ps,pdf}

enable_testing()

if (DEFINED ENV{VERBOSE})
    set(VERBOSE_MODE $ENV{VERBOSE})
else()
    set(VERBOSE_MODE "0")
endif()

file(GLOB test_dirs RELATIVE "${CMAKE_SOURCE_DIR}/tests" "${CMAKE_SOURCE_DIR}/tests/*")

if(NOT DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    set(EMULATOR "")
elseif (DEFINED EMSCRIPTEN)
    if (${EMSCRIPTEN})
        set(EMULATOR "${CMAKE_CROSSCOMPILING_EMULATOR} ")
    else()
        set(EMULATOR "")
    endif()
else()
    set(EMULATOR "")
endif()

foreach(TEST_TARGET ${test_dirs})
    if(EXISTS "${CMAKE_SOURCE_DIR}/tests/${TEST_TARGET}/${TEST_TARGET}.asm" AND EXISTS "${CMAKE_SOURCE_DIR}/tests/${TEST_TARGET}/${TEST_TARGET}.ori")
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
