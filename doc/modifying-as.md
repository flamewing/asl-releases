# Hints for the AS Source Code

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

## **EDITOR's NOTES**

These sections are Alfred Arnold's writing, mostly unedited. My notes are as follows:

1. I do not support K&R at all, and you are a bad person if you suggest I should.
2. A lot of it is made irrelevant due to my changes to use C11 and cmake.

## **END EDITOR's NOTES**

As I already mentioned in the introduction, I release the source code of AS on request. The following shall give a few hints to their usage.

## Language Preliminaries

In the beginning, AS was a program written in Turbo-Pascal. This was roughly at the end of the eighties, and there were a couple of reasons for this choice: First, I was much more used to it than to any C compiler, and compared to Turbo Pascal's IDE, all DOS-based C compilers were just crawling along. In the beginning of 1997 however, it became clear that things had changed: One factor was that Borland had decided to let its confident DOS developers down (once again, explicitly no 'thank you', you boneheads from Borland!) and replaced version 7.0 of Borland Pascal with something called 'Delphi', which is probably a wonderful tool to develop Windows programs which consist of 90% user interface and accidentally a little bit of content, however completely useless for command-line driven programs like AS. Furthermore, my focus of operating systems had made a clear move towards Unix, and I probably could have waited arbitrarily long for a Borland Pascal for Linux (to all those remarking now that Borland would be working on something like that: this is _Vapourware_, don't believe them anything until you can go into a shop and actually buy it!). It was therefore clear that C was the way to go.

After this experience what results the usage of 'island systems' may have, I put a big emphasize on portability while doing the translation to C; however, since AS for example deals with binary data in an exactly format and uses operating system-specific functions at some places which may need adaptions when one compiles AS the first time for a new platform.

AS is tailored for a C compiler that conforms to the ANSI C standard; C++ is explicitly not required. If you are still using a compiler conforming to the outdated Kernighan&Ritchie standard, you should consider getting a newer compiler: The ANSI C standard has been fixed in 1989 and there should be an ANSI C compiler for every contemporary platform, maybe by using the old compiler to build GNU-C. Though there are some switches in the source code to bring it nearer to K&R, this is not an officially supported feature which I only use internally to support a quite antique Unix. Everything left to say about K&R is located in the file `README.KR`.

The inclusion of some additional features not present in the Pascal version (e.g. dynamically loadable message files, test suite, automatic generation of the documentation from _one_ source format) has made the source tree substantially more complicated. I will attempt to unravel everything step by step:

## Encapsulating System dependencies

## **EDITOR's NOTE:** This is Alfred Arnold's writing, mostly unedited. Most of it is made irrelevant due to my changes to use C11 and cmake

As I already mentioned, As has been tailored to provide maximum platform independence and portability (at least I believe so...). This means packing all platform dependencies into as few files as possible. I will describe these files now, and this section is the first one because it is probably one of the most important:

The Build of all components of AS takes place via a central `Makefile`. To make it work, it has to be accompanied by a fitting `Makefile.def` that gives the platform dependent settings like compiler flags. The subdirectory `Makefile.def-samples` contains a couple of includes that work for widespread platforms (but which need not be optimal...). In case your platform is not among them, you may take the file `Makefile.def.tmpl` as a starting point (and send me the result!).

A further component to capture system dependencies is the file `sysdefs.h`. Practically all compilers predefine a couple of preprocessor symbols that describe the target processor and the used operating system. For example, on a Sun Sparc under Solaris equipped with the GNU compiler, the symbols `__sparc` and `__SVR4`. `sysdefs.h` exploits these symbols to provide a homogeneous environment for the remaining, system-independent files. Especially, this covers integer data types of a specific length, but it may also include the (re)definition of C functions which are not present or non-standard-like on a specific platform. It's best to read this files yourself if you like to know which things may occur... Generally, the `#ifdef` statement are ordered in two levels: First, a specific processor platform is selected, the the operating systems are sorted out in such a section.

If you port AS to a new platform, you have to find two symbols typical for this platform and extend `sysdefs.h` accordingly. Once again, I'm interested in the result...

## System-Independent Files

...represent the largest part of all modules. Describing all functions in detail is beyond the scope of this description (those who want to know more probably start studying the sources, my programming style isn't that horrible either...), which is why I can only give a short list at this place with all modules their function:

### Modules Used by AS

#### as.c

This file is AS's root: it contains the _main()_ function of AS, the processing of all command line options, the overall control of all passes and parts of the macro processor.

#### asmallg.c

This module processes all statements defined for all processor targets, e.g. `EQU` and `ORG`. The `CPU` pseudo-op used to switch among different processor targets is also located here.

#### asmcode.c

This module contains the bookkeeping needed for the code output file. It exports an interface that allows to open and close a code file and offers functions to write code to (or take it back from) the file. An important job of this module is to buffer the write process, which speeds up execution by writing the code in larger blocks.

#### asmdebug.c

AS can optionally generate debug information for other tools like simulators or debuggers, allowing a backward reference to the source code. They get collected in this module and can be output after assembly in one of several formats.

#### asmdef.c

This modules only contains declarations of constants used in different places and global variables.

#### asmfnums.c

AS assigns internally assigns incrementing numbers for each used source file. These numbers are used for quick referencing. Assignment of numbers and the conversion between names and numbers takes place here.

#### asmif.c

Here ara ll routines located controlling conditional assembly. The most important exported variable is a flag called `IfAsm` which controls whether code generation is currently turned on or off.

#### asminclist.c

This module holds the definition of the list structure that allows AS to print the nesting of include files to the assembly list file.

#### asmitree.c

When searching for the mnemonic used in a line of code, a simple linear comparison with all available machine instructions (as it is still done in most code generators, for reasons of simplicity and laziness) is not necessary the most effective method. This module defines two improved structures (binary tree and hash table) which provide a more efficient search and are destined to replace the simple linear search on a step-by-step basis...priorities as needed...

#### asmmac.c

Routines to store and execute macro constructs are located in this module. The real macro processor is (as already mentioned) in `as.c`.

#### asmpars.c

Here we really go into the innards: This module stores the symbol tables (global and local) in two binary trees. Further more, there is a quite large procedure `EvalExpression` which analyzes and evaluates a (formula) expression. The procedure returns the result (integer, floating point, or string) in a variant record. However, to evaluate expressions during code generation, one should better use the functions `EvalIntExpression`, `EvalFloatExpression`, and `EvalStringExpression`. Modifications for the sake of adding new target processors are unnecessary in this modules and should be done with extreme care, since you are touching something like 'AS's roots'.

#### asmsub.c

This module collects a couple of commonly used subroutines which primarily deal with error handling and 'advanced' string processing.

#### bpemu.c

As already mentioned at the beginning, AS originally was a program written in Borland Pascal. For some intrinsic functions of the compiler, it was simpler to emulate those than to touch all places in the source code where they are used. Well...

#### chunks.c

This module defines a data type to deal with a list of address ranges. This functionality is needed by AS for allocation lists; furthermore, P2BIN and P2HEX use such lists to warn about overlaps.

#### cmdarg.c

This module implements the overall mechanism of command line arguments. It needs a specification of allowed arguments, splits the command line and triggers the appropriate callbacks. In detail, the mechanism includes the following:

- Processing of arguments located in an environment variable or a corresponding file;
- Return of a set describing which command line arguments have not been processed;
- A backdoor for situations when an overlaying IDE converts the passed command line completely into upper or lower case.

#### codepseudo.c

You will find at this place pseudo instructions that are used by a subset of code generators. On the one hand, this is the Intel group of `DB..DT`, and on the other hand their counterparts for 8/16 bit CPUs from Motorola or Rockwell. Someone who wants to extend AS by a processor fitting into one of these groups can get the biggest part of the necessary pseudo instructions with one call to this module.

#### codevars.c

For reasons of memory efficiency, some variables commonly used by diverse code generators.

#### as_endian.c

Yet another bit of machine dependence, however one you do not have to spend attention on: This module automatically checks at startup whether a host machine is little or big endian. Furthermore, checks are made if the type definitions made for integer variables in `sysdefs.h` really result in the correct lengths.

#### headids.c

At this place, all processor families supported by AS are collected with their header IDs (see [Code Files](file-formats.md#code-files)) and the output format to be used by default by P2HEX. The target of this table is to centralize the addition of a new processor as most as possible, i.e. in contrast to earlier versions of AS, no further modifications of tool sources are necessary.

#### ioerrs.c

The conversion from error numbers to clear text messages is located here. I hope I'll never hit a system that does not define the numbers as macros, because I would have to rewrite this module completely...

#### nlmessages.c

The C version of AS reads all messages from files at runtime after the language to be used is clear. The format of message files is not a simple one, but instead a special compact and pre-indexed format that is generated at runtime by a program called 'rescomp' (we will talk about it later). This module is the counterpart to rescomp that reads the correct language part into a character field and offers functions to access the messages.

#### nls.c

This module checks which country-dependent settings (date and time format, country code) are present at runtime. Unfortunately, this is a highly operating system-dependent task, and currently, there are only three methods defines: The MS-DOS method, the OS/2 method and the typical Unix method via _locale_ functions. For all other systems, there is unfortunately currently only `NO_NLS` available...

#### stdhandl.c

On the one hand, here is a special open function located knowing the special strings `!0...!2` as file names and creating duplicates of the standard file handles _stdin, stdout,_ and _stderr_. On the other hand, investigations are done whether the standard output has been redirected to a device or a file. On no-Unix systems, this unfortunately also incorporates some special operations.

#### stringlists.c

This is just a little 'hack' that defines routines to deal with linear lists of strings, which are needed e.g. in the macro processor of AS.

#### strutil.c

Some commonly needed string operations have found their home here.

#### version.c

The currently valid version is centrally stored here for AS and all other tools.

#### code????.c

These modules form the main part of AS: each module contains the code generator for a specific processor family.

### Additional Modules for the Tools

#### hex.c

A small module to convert integer numbers to hexadecimal strings. It's not absolutely needed in C any more (except for the conversion of _long long_ variables, which unfortunately not all `printf()`'s support), but it somehow survived the porting from Pascal to C.

#### p2bin.c

The sources of P2BIN.

#### p2hex.c

The sources of P2HEX.

#### pbind.c

The sources of BIND.

#### plist.c

The sources of PLIST.

#### toolutils.c

All subroutines needed by several tools are collected here, e.g. for reading of code files.

## Modules Needed During the Build of AS

#### rescomp.c

This is AS's 'resource compiler', i.e. the tool that converts a readable file with string resources into a fast, indexed format.

#### test_driver.c

This is used by _ctest_ to run tests.

## Generation of Message Files

As already mentioned, the C source tree of AS uses a dynamic load principle for all (error) messages. In contrast to the Pascal sources where all messages were bundled in an include file and compiled into the programs, this method eliminates the need to provide AS in multiple language variants; there is only one version which checks for the language to be used upon runtime and loads the corresponding component from the message files. Just to remind: Under DOS and OS/2, the `COUNTRY` setting is queried, while under Unix, the environment variables `LC_MESSAGES, LC_ALL,` and `LANG` are checked.

### Format of the Source Files

A source file for the message compiler _rescomp_ usually has the suffix
`.res`. The message compiler generates one or two files from a source:

- a binary file which is read at runtime by AS resp. its tools
- optionally one further C header file assigning an index number to all messages. These index numbers in combination with an index table in the binary file allow a fast access to to individual messages at runtime.

The source file for the message compiler is a pure ASCII file and can therefore be modified with any editor. It consists of a sequence of control commands with parameters. Empty lines and lines beginning with a semicolon are ignored. Inclusion of other files is possible via the `Include` statement:

```asm
    Include <Datei>
```

The first two statements in every source file must be two statements describing the languages defined in the following. The more important one is `Langs`, e.g.:

```asm
    Langs DE(049) EN(001,061)
```

describes that two languages will be defined in the rest of the file. The first one shall be used under Unix when the language has been set to `DE` via environment variable. Similarly, It shall be used under DOS and OS/2 when the country code was set to 049. Similarly, the second set shall be used for the settings `DE` resp. 061 or 001. While multiple 'telephone numbers' may point to a single language, the assignment to a Unix language code is a one-to-one correspondence. This is no problem in practice since the `LANG` variables Unix uses describe subversions via appendices, e.g.:

```asm
    de.de
    de.ch
    en.us
```

AS only compares the beginning of the strings and therefore still comes to the right decision. The `Default` statement defines the language that shall be used if either no language has been set at all or a language is used that is not mentioned in the argument list of `Langs`. This is typically the english language:

```asm
    Default EN
```

These definitions are followed by an arbitrary number of `Message` statements, i.e. definitions of messages:

```asm
    Message ErrName
     ": Fehler "
     ": error "
```

In case _n_ languages were announced via the `Langs` statement, the message compiler takes **exactly** the following _n_ as the strings to be stored. It is therefore impossible to leave out certain languages for individual messages, and an empty line following the strings should in no way be misunderstood as an end marker for the list; inserted lines between statements only serve purposes of better readability. It is however allowed to split individual messages across multiple lines in the source file; all lines except for the last one have to be ended with a backslash as continuation character:

```asm
    Message TestMessage2
     "Dies ist eine" \
     "zweizeilige Nachricht"
     "This is a" \
     "two-line message"
```

As already mentioned, source files are pure ASCII files; national special characters may be placed in message texts (and the compiler will correctly pass them to the resulting file), a big disadvantage however is that such a file is not fully portable any more: in case it is ported to another system using a different coding for national special characters, the user will probably be confronted with funny characters at runtime...special characters should therefore always be written via special sequences borrowed from HTML resp. SGML (see the following table). Linefeeds can be inserted into a line via `\n`, similar to C.

| Sequence...                           | results in...         |
| :------------------------------------ | :-------------------- |
| `&auml; &ouml; &uuml;`                | "a "o "u (Umlauts)    |
| `&Auml; &Ouml; &Uuml;`                | "A "O "U              |
| `&szlig;`                             | "s (sharp s)          |
| `&agrave; &egrave; &igrave; &ograve;` | á é í ó               |
| `&ugrave;`                            | ú                     |
| `&Agrave; &Egrave; &Igrave; &Ograve;` | Á É Í Ó               |
| `&Ugrave;`                            | Ú (Grave accent)      |
| `&aacute; &eacute; &iacute; &oacute;` | à è ì ò               |
| `&uacute;`                            | ù                     |
| `&Aacute; &Eacute; &Iacute; &Oacute;` | À È Ì Ò               |
| `&Uacute;`                            | Ù (Acute accent)      |
| `&acirc; &ecirc; &icirc; &ocirc;`     | â ê î ô               |
| `&ucirc;`                             | û                     |
| `&Acirc; &Ecirc; &Icirc; &Ocirc;`     | Â Ê Î Ô               |
| `&Ucirc;`                             | Û (Circumflex accent) |
| `&ccedil; &Ccedil;`                   | ç Ç (Cedilla)         |
| `&ntilde; &Ntilde;`                   | ñ Ñ                   |
| `&aring; &Aring;`                     | åÅ                    |
| `&aelig; &Aelig;`                     | æÆ                    |
| `&iquest; &iexcl;`                    | inverted ! or ?       |

## Creation of Documentation

A source distribution of AS contains this documentation in Markdown format.

## Test Suite

Since AS deals with binary data of a precisely defined structure, it is naturally sensitive for system and compiler dependencies. To reach at least a minimum amount of secureness that everything went right during compilation, a set of test sources is provided in the subdirectory `tests` that allows to test the freshly built assembler. These programs are primarily trimmed to find faults in the translation of the machine instruction set, which are commonplace when integer lengths vary. Target-independent features like the macro processors or conditional assembly are only casually tested, since I assume that they work everywhere when they work for me...

The test run is started via a simple `ctest --build-run-dir build --test-dir build -j`. Each test program is assembled, converted to a binary file, and compared to a reference image. A test is considered to be passed if and only if the reference image and the newly generated one are identical on a bit-by-bit basis. At the end of the test, the assembly time for every test is printed (those who want may extend the file BENCHES with these results), accompanied with a success or failure message. Track down every error that occurs, even if it occurs in a processor target you are never going to use! It is always possible that this points to an error that may also come up for other targets, but by coincidence not in the test cases.

## Adding a New Target Processor

The probably most common reason to modify the source code of AS is to add a new target processor. Apart from adding the new module to the Makefile, there are few places in other modules that need a modification. The new module will do the rest by registering itself in the list of code generators. I will describe the needed steps in a cookbook style in the following sections:

#### Choosing the Processor's Name

The name chosen for the new processor has to fulfill two criteria:

1.  The name must not be already in use by another processor. If one starts AS without any parameters, a list of the names already in use will be printed.
2.  If the name shall appear completely in the symbol `MOMCPU`, it may not contain other letters than A..F (except right at the beginning). The variable `MOMCPUNAME` however will always report the full name during assembly. Special characters are generally disallowed, lowercase letters will be converted by the `CPU` command to uppercase letters and are therefore senseless in the processor name.

The first step for registration is making an entry for the new processor (family) in the file `headids.c`. As already mentioned, this file is also used by the tools and specifies the code ID assigned to a processor family, along with the default hex file format to be used. I would like to have some coordination before choosing the ID...

#### Definition of the Code Generator Module

The unit's name that shall be responsible for the new processor should bear at least some similarity to the processor's name (just for the sake of uniformity) and should be named in the style of `code....`. The head with include statements is best taken from another existing code generator.

Except for an initialization function that has to be called at the beginning of the `main()` function in module `as.c`, the new module neither has to export variables nor functions as the complete communication is done at runtime via indirect calls. They are simply done by a call to the function `AddCPU` for each processor type that shall be treated by this unit:

```c
        CPUxxxx:=AddCPU('XXXX',SwitchTo_xxxx);
```

`'XXXX'` is the name chosen for the processor which later must be used in assembler programs to switch AS to this target processor. `SwitchTo_xxxx` (abbreviated as the "switcher" in the following) is a procedure without parameters that is called by AS when the switch to the new processor actually takes place. `AddCPU` delivers an integer value as result that serves as an internal "handle" for the new processor. The global variable `MomCPU` always contains the handle of the target processor that is currently set. The value returned by `AddCPU` should be stored in a private variable of type `CPUVar` (called `CPUxxxx` in the example above). In case a code generator module implements more than one processor (e.g. several processors of a family), the module can find out which instruction subset is currently allowed by comparing `MomCPU` against the stored handles.

The switcher's task is to "reorganize" AS for the new target processor. This is done by changing the values of several global variables:

- `ValidSegs`: Not all processors have all address spaces defined by AS. This set defines which subset the `SEGMENT` instruction will enable for the currently active target processor. At least the `CODE` segment has to be enabled. The complete set of allowed segments can be looked up the file `fileformat.h` (`Seg....` constants).
- `SegInits`: This array stores the initial program counter values for the individual segments (i.e. the values the program counters will initially take when there is no `ORG` statement). There are only a few exceptions (like logically separated address spaces that physically overlap) which justify other initial values than 0.
- `Grans`: This array specifies the size of the smallest addressable element in bytes for each segment, i.e. the size of an element that increases an address by 1. Most processors need a value of 1, even if they are 16- or 32-bit processors, but the PICs and signal processors are cases where higher values are required.
- `ListGrans`: This array specifies the size of byte groups that shall be shown in the assembly listing. For example, instruction words of the 68000 are always 2 bytes long though the code segment's granularity is 1. The `ListGran` entry therefore has to be set to 2.
- `SegLimits`: This array stores the highest possible address for each segment, e.g. 65535 for a 16-bit address space. This array need not be filled in case the code generator takes over the `ChkPC` method.
- `ConstMode`: This variable may take the values `ConstModeIntel`, `ConstModeMoto`, or `ConstModeC` and rules which syntax has to be used to specify the base of integer constants.
- `PCSymbol`: This variable contains the string an assembler program may use to to get the current value of the program counter. Intel processors for example usually use a dollar sign.
- `TurnWords`: If the target processor uses big-endian addressing and one of the fields in `ListGran` is larger than one, set this flag to true to get the correct byte order in the code output file.
- `SetIsOccupied`: Some processors have a `SET` machine instruction. If this callback is set to a non-NULL value, the code generator may report back whether `SET` shall not be interpreted as pseudo instruction. The return value may be constant `True` or or e.g. depend on the number of argument if a differentiation is possible.
- `HeaderID`: This variable contains the ID that is used to mark the current processor family in the the code output file (see the description of the code format described by AS). I urge to contact me before selecting the value to avoid ambiguities. Values outside the range of $01..$7f should be avoided as they are reserved for special purposes (like a future extension to allow linkable code). Even though this value is still hard-coded in most code generators, the preferred method is now to fetch this value from `headids.h` via `FindFamilyByName`.
- `NOPCode`: There are some situations where AS has to fill unused code areas with NOP statements. This variable contains the machine code of the NOP statement.
- `DivideChars`: This string contains the characters that are valid separation characters for instruction parameters. Only extreme exotics like the DSP56 require something else than a single comma in this string.
- `HasAttrs`: Some processors like the 68k series additionally split an instruction into mnemonic and attribute. If the new processor also does something like that, set this flag to true and AS will deliver the instructions' components readily split in the string variables `OpPart` and `AttrPart`. If this flag is however set to false, no splitting will take place and the instruction will be delivered as a single piece in `OpPart`. `AttrPart` will stay empty in this case. One really should set this flag to false if the target processor does not have attributes as one otherwise looses the opportunity to use macros with a name containing dots (e.g. to emulate other assemblers).
- `AttrChars`: In case `HasAttrs` is true, this string has to contain all characters that can separate mnemonic and attribute. In most cases, this string only contains a single dot.

Do not assume that any of these variables has a predefined value; set them **all**!!

Apart from these variables, some function pointers have to be set that form the link form AS to the "active" parts of the code generator:

- `MakeCode`: This routine is called after a source line has been split into mnemonic and parameters. The mnemonic is stored into the variable `OpPart`, and the parameters can be looked up in the array `ArgStr`. The number of arguments may be read from `ArgCnt`. The binary code has to be stored into the array `BAsmCode`, its length into `CodeLen`. In case the processor is word oriented like the 68000 (i.e. the `ListGran` element corresponding to the currently active segment is 2), the field may be addressed word-wise via `WAsmCode`. There is also `DAsmCode` for extreme cases... The code length has to be given in units corresponding to the current segment's granularity.
- `SwitchFrom`: This parameter-less procedure enables the code generator module to do "cleanups" when AS switches to another target processor. This hook allows e.g. to free memory that has been allocated in the generator and that is not needed as long as the generator is not active. It may point to an empty procedure in the simplest case. One example for the usage of this hook is the module `CODE370` that builds its instruction tables dynamically and frees them again after usage.
- `IsDef`: Some processors know additional instructions that impose a special meaning on a label in the first row like `EQU` does. One example is the `BIT` instruction found in an 8051 environment. This function has to return TRUE if such a special instruction is present. In the simplest case (no such instructions), the routine may return a constant FALSE.

Optionally, the code generator may additionally set the following function pointers:

- `ChkPC` : Though AS internally treats all program counters as either 32 or 64 bits, most processors use an address space that is much smaller. This function informs AS whether the current program counter has exceeded its allowed range. This routine may of course be much more complicated in case the target processor has more than one address space. One example is in module `code16c8x.c`. In case everything is fine, the function has to return TRUE, otherwise FALSE. The code generator only has to implement this function if it did not set up the array `SegLimits`. This may e.g. become necessary when the allowed range of addresses in a segment is non-continuous.
- `InternSymbol` : Some processors, e.g. such with a register bank in their internal RAM, predefine such 'registers' as symbols, and it wouldn't make much sense to define them in a separate include file with 256 or maybe more `EQU`s. This hook allows access to the code generator of AS: It obtains an expression as an ASCII string and sets up the passed structure of type _TempResult_ accordingly when one of these 'built-in' symbols is detected. The element `Typ` has to be set to `TempNone` in case the check failed. Errors messages from this routine should be avoided as unidentified names could signify ordinary symbols (the parser will check this afterwards). Be extreme careful with this routine as it allows you to intervene into the parser's heart!
- `DissectBit` : In case the target platform supports bit objects, i.e. objects that pack both a register or memory address and a bit position into one integer number, this is the callback to dissect such a packed representation and transform it back into a source-code like, human-readable form. This provides better readability of the listing.
- `DissectReg` : In case the target platform supports register symbols, this is the callback that translates register number and size back to a source-code like, human-readable form. Again, this function is used for the listing.
- `QualifyQuote` : This optional callback allows to define on a per-platform base situations when a single quotation character does _not_ lead in a character string. An example for this is the Z80's alternate register bank, which is written as `AF'`, or the hexadecimal constant syntax `H'...` used on some Hitachi processors.

By the way: People who want to become immortal may add a copyright string. This is done by adding a call to the procedure `AddCopyright` in the module's initialization part (right next to the `AddCPU` calls):

```c
       AddCopyright(
          "Intel 80986 code generator (C) 2010 Jim Bonehead");
```

The string passed to `AddCopyright` will be printed upon program start in addition to the standard message.

If needed, the unit may also use its initialization part to hook into a list of procedures that are called prior to each pass of assembly. Such a need for example arises when the module's code generation depends on certain flags that can be modified via pseudo instructions. An example is a processor that can operate in either user or supervisor mode. In user mode, some instructions are disabled. The flag that tells AS whether the following code executes in user or supervisor mode might be set via a special pseudo instruction. But there must also be an initialization that assures that all passes start with the same state. The hook offered via `AddInitPassProc` offers a chance to do such initializations. The callback function passed to it is called before a new pass is started.

The function chain built up via calls to `AddCleanUpProc` operates similar to `AddInitPassProc`: It enables code generators to do clean-ups after assembly (e.g. freeing of literal tables). This makes sense when multiple files are assembled with a single call of AS. Otherwise, one would risk to have 'junk' in tables from the previous run. No module currently uses this feature.

#### Writing the Code Generator itself

Now we finally reached the point where your creativity is challenged: It is up to you how you manage to translate mnemonic and parameters into a sequence of machine code. The symbol tables are of course accessible (via the formula parser) just like everything exported from `ASMSUB`. Some general rules (take them as advises and not as laws...):

- Try to split the instruction set into groups of instructions that have the same operand syntax and that differ only in a few bits of their machine code. For example, one can do all instructions without parameters in a single table this way.
- Most processors have a fixed spectrum of addressing modes. Place the parsing of an address expression in a separate routine so you an reuse the code.
- The subroutine `WrError` defines a lot of possible error codes and can be easily extended. Use this! It is no good to simply issue a "syntax error" on all error conditions!

Studying other existing code generators should also prove to be helpful.

#### Modifications of Tools

A microscopic change to the tolls' sources is still necessary, namely to the routine `Granularity()` in `toolutils.c`: in case one of the processor's address spaces has a granularity different to 1, the switch statement in this place has to be adapted accordingly, otherwise PLIST, P2BIN, and P2HEX start counting wrong...

## Localization to a New Language

You are interested in this topic? Wonderful! This is an issue that is often neglected by other programmers, especially when they come from the country on the other side of the big lake...

The localization to a new language can be split into two parts: the adaption of program messages and the translation of the manual. The latter one is definitely a work of gigantic size, however, the adaption of program messages should be a work doable on two or three weekends, given that one knows both the new and one of the already present messages. Unfortunately, this translation cannot be done on a step-by-step basis because the resource compiler currently cannot deal with a variable amount of languages for different messages, so the slogan is 'all or nothing'.

The first operation is to add the new language to `header.res`. The two-letter-abbreviation used for this language is best fetched from the nearest Unix system (in case you don't work on one anyway...), the international telephone prefix from a DOS manual.

When this is complete, one can rebuild all necessary parts with a simple `cmake --build build -j` and obtains an assembler that supports one more language. Do not forget to forward the results to me. This way, all users will benefit from this with the next release :-)

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->
