# Assembler Usage

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

> _Scotty: Captain, we din' can reference it!_
>
> _Kirk: Analysis, Mr. Spock?_
>
> _Spock: Captain, it doesn't appear in the symbol table._
>
> _Kirk: Then it's of external origin?_
>
> _Spock: Affirmative._
>
> _Kirk: Mr. Sulu, go to pass two._
>
> _Sulu: Aye aye, sir, going to pass two._

## Delivery

This fork is available as source from [flamewing/asl-releases](https://github.com/flamewing/asl-releases) and as platform-specific [snapshot archives](https://github.com/flamewing/asl-releases/releases). Source builds use CMake and a C11 compiler; see [the build instructions](../README.md#create-and-install-the-package). The executable is named `asl` (`asl.exe` on Windows); MSYS2 builds also provide `asw.exe`.

Current snapshot archives place the executables, `COPYING` and generated HTML/ODT manuals together. They do not include the source checkout's complete include-file tree or manpages. Obtain those from the source distribution, or use a CMake installation when you need the full installed layout.

A CMake installation uses the following paths relative to its install prefix. The directory variables can be changed when configuring CMake; `lib` can be `lib64` or another platform-selected library directory.

| Installed item | Default location | Purpose |
| --- | --- | --- |
| `asl`, `alink`, `dasl`, `pbind`, `p2bin`, `p2hex`, `plist` | `bin` | Assembler, linker, disassembler and binary-file utilities; Windows adds `.exe` |
| `asw.exe` | `bin` | Additional assembler executable in MSYS2 builds |
| Include-file tree | `include/asl` | Register definitions and macros; see [Shipped Include Files](shipped-include-files.md) |
| Manpages | `share/man/man1` | Command-line reference pages |
| `COPYING` | `share/doc/asl-flamewing` | License text |
| `asl.html`, `asl.odt` | `share/doc/asl-flamewing` | Manuals generated when Pandoc is available |
| `as.msg`, `das.msg`, utility catalogs and shared catalogs | `lib/asl` | Installed only with `ASL_EMBED_MESSAGES=OFF` |

Catalogs are embedded by default, so normal executables do not require companion `.msg` files. External-catalog builds use the message search described under [Help and message levels](#help-and-message-levels); set `AS_MSGPATH` to the catalog directory when running from a relocated installation. Include files are separate from message catalogs. Add the installed include directory with `-i /prefix/include/asl` when a source uses those definitions.

The `rescomp` resource compiler is a build-time host tool and is not part of the installed executable set. Its exported target allows a cross-build to generate catalogs without executing target binaries on the build host.

## Start-Up Command, Parameters

Use `asl [options] source [source ...]` to assemble files, `asl -help` for help, `asl -version` for version/build information, and `asl -cpu list` for the available targets. On Windows the executable has an `.exe` suffix; MSYS2 also builds `asw`. A filename without an extension receives `.asm`, and the default code output replaces that extension with `.p`. Set the CPU in the source with `CPU` or supply `-cpu name`.

```sh
asl -cpu 68000 -i include -o output.p source.asm
asl -L -olist source.lst -g map source.asm
asl '-nx' '*.asm'
asl -help
asl -cpu list
```

Quote shell metacharacters and paths containing spaces. Quoting a wildcard lets the assembler expand it; unquoted patterns may instead be expanded by the shell. Prefix a source filename beginning with `-`, `+` or `@` with a directory such as `./` so it is treated as a filename. On Windows use a backslash directory prefix such as `.\source.asm`; slash-prefixed arguments are also recognized as options in Windows builds. Unix builds accept absolute paths beginning with `/`.

Options may appear before, between or after filenames. They are parsed before assembly and apply to every input file; output/list/share filename options maintain ordered filename lists. Use separate invocations when sources need different option settings. A leading `-` enables an option and `+` negates it or invokes its documented reset behavior. Multi-letter names are case-insensitive, while single-letter options distinguish case, for example `-l` versus `-L`. Single-letter options can be combined, as in `-nx` or `-xx`. The supported help spelling is `-help`; GNU-style `--help` and a `--` end-of-options marker are not implemented.

Optional arguments can consume the following filename. Write `-g map source.asm`, or place argument-free `-g` after the source. The same care applies to `-E [name]` and `-splitbyte [character]`. `-r` consumes a following valid pass number but leaves a nonnumeric filename for assembly. Argument-taking options such as `-o`, `-i`, `-t` and `-noicemask` require a separate argument.

### Option Reference

| Option | Effect |
| --- | --- |
| `-A` | balance remaining macro/structure trees |
| `-a` | share file formatted for AS |
| `-alias <new>=<old>` | define processor alias |
| `-bigendian` | turn on BIGENDIAN mode by default |
| `-C` | generate cross reference list |
| `-c` | share file formatted for C |
| `-compmode` | enable COMPMODE by default |
| `-cpu <name>` | set target processor |
| `-D <symbol>[,symbol]` | predefine symbols |
| `-E [name]` | target file for error list, |
| `-extracomments` | enable EXTRACOMMENTS mode by default |
| `-fred` | force restricted extension of digits |
| `+G` | suppress code generation |
| `-g [map\|atmel\|noice]` | write debug info |
| `-gnuerrors` | error messages in GNU format |
| `-help` | print help and exit |
| `-h` | use lower case in hexadecimal output |
| `-I` | generate include nesting list |
| `-i <path>` | add include directory (repeatable) |
| `-intsyntax <arg>[,arg]` | modify syntax for integer constants |
| `-L` | listing to file |
| `-l` | listing to console |
| `-listline-prefix <fmt>` | set format of listing per-line prefix |
| `-listing <off\|on\|purecode\|noskipped>` | limit listing of assembled code |
| `-listradix <2...36>` | number system in listing |
| `-list-macro-handles` | list macro's symbol handles |
| `-[no-]list-unknown-values` | display unknown values in listing as question marks |
| `-M` | extract macro definitions |
| `-maxerrors <number>` | terminate assembly after <number> errors |
| `-maxinclevel <number>` | limit include nesting level to <number> |
| `-maxsympass <number>` | unresolved symbols allowed through pass (default 1) |
| `-n` | add error #s to error messages |
| `-noicemask <mask>` | set NoICE segment mask |
| `-olist <name>` | change name of list file |
| `-o <name>` | change name of code file |
| `-p` | share file formatted for Pascal |
| `-P` | write macro processor output |
| `-packing` | enable PACKING by default |
| `-plainbase` | enable PLAINBASE mode by default |
| `-q, -quiet` | silent operation |
| `-r [pass]` | report additional passes from pass (default 1) |
| `-radix <2...36>` | set default number system |
| `-relaxed` | allow arbitrary integer syntax by default |
| `-screenheight <#>` | set screen height for paging of '-help' |
| `-s` | generate section list |
| `-shareout <name>` | change name of share file |
| `-splitbyte [char]` | use split byte display of numbers |
| `-supmode` | allow privileged instructions by default |
| `-t <mask>` | enable listing parts; +t disables them |
| `-U` | case-sensitive operation |
| `-u` | generate usage list |
| `-underscore-macroargs` | allow underbars in macro arguments |
| `-v` | verbose operation |
| `-version` | print version info and exit |
| `-w` | suppress warnings |
| `-warnranges` | warn instead of failing eligible range checks |
| `-Werror` | treat warnings as errors |
| `-w[no-]implicit-sign-extension` | enable/disable sign extension warnings |
| `-w[no-]relative` | enable/disable warnings about possible relative jump |
| `-x` | extended error messages |
| `-X` | internal trace to as.deb (developer option) |
| `-Y` | branch error suppression (see manual) |

Symbols use hash tables in this fork. `-A` balances remaining tree-based definitions, such as macros/structures; it does not replace the symbol hash tables or change assembly semantics. `-D name` defines TRUE, and `-D name=value` evaluates a symbol-free constant expression. Set `-U` before `-D` when case preservation matters. Repeated `-i` options add directories; a path-list argument uses `:` in Unix builds and `;` in Windows builds. See [INCLUDE](pseudo-instructions.md#include) for include lookup order.

`-x` adds contextual diagnostic detail, and `-xx` also displays the offending source line; `+x` reduces the level. `-n` adds diagnostic numbers. `-q` suppresses normal progress while leaving errors visible; repeating it suppresses source MESSAGE output too. `-warnranges` softens eligible evaluator range checks, while directive-specific hard range checks remain errors. `-maxerrors 0` means unlimited errors, `-maxinclevel` defaults to 200, and `-maxsympass` defaults to 1. Listing-mask values accepted by `-t` are 0 through 511: `-t mask` enables bits, `+t mask` disables them, and the default mask is 511.

### Preset Options and Key Files

`ASCMD` supplies options before the command line, allowing command-line options to override them. It can contain options directly or an `@filename` key-file reference. A command-line `@filename` is expanded at that argument's position. Key files contain whitespace-separated options, one or more lines at a time; each option and its argument must be on the same line. Lines starting with `;` are comments, and key files cannot include other key files. The line buffer is 1024 bytes including its terminator. Key-file/environment parsing does not implement shell quoting; use the real command line for paths with spaces.

```sh
ASCMD='-L -i include' asl source.asm
asl @build-options.key source.asm
asl +L source.asm
```

On PowerShell use `$env:ASCMD = '-L -i include'` to set the preset options; remove that variable when you no longer want those defaults. ASCMD and key files can also supply source filenames; these are collected with the command-line sources. The case-conversion prefixes `-#u` and `-~I` force the following single-letter option to uppercase (`-U`) or lowercase (`-i`) for environments that alter command-line case.

### Message Catalogs and Language

The default embedded-catalog build needs no companion `.msg` files. External-catalog builds search the current directory, then an explicitly supplied executable directory, then AS_MSGPATH (or PATH when AS_MSGPATH is unset), and finally the compiled-in library directory. Set `AS_MSGPATH` to one catalog directory rather than a path list. Failure to load a required external catalog aborts startup.

Language selection checks `LC_MESSAGES`, then `LC_ALL`, then `LANG`; supported languages are German and English, with English as the default. Country information can also select a catalog. These variables are consulted by the catalog reader independently of whether the operating system has that locale installed. Node-based WebAssembly command-line builds forward the host environment before C startup.

### Exit Status

| Status | Meaning |
| --- | --- |
| 0 | Successful assembly (possibly warnings), or successful help/version/CPU-list request |
| 1 | No input source files were supplied |
| 2 | Assembly errors; output for the failing source is not a usable successful assembly |
| 3 | Fatal assembly/I/O failure |
| 4 | Invalid command-line or preset option, or key-file startup failure |
| 255 | Catalog/resource initialization failure or other internal initialization failure |

## Format of the Input Files

Like most assemblers, AS expects exactly one instruction per line (blank lines are naturally allowed as well). Source lines are read into a dynamically growing buffer; they are not truncated at the former 255-character limit. Individual instructions and directives still impose their own operand and value limits.

A single line has following format:

```sh
[label[:]] <mnemonic>[.attr] [param[,param...]] [;comment]
```

A trailing backslash joins a physical source line to the next line. The backslash and line ending are removed, and the remaining text is concatenated without an inserted separator. Continued lines use the same dynamically growing input buffer. Line references in error messages always relate to the last line of such a composed source line.

The colon for the label is optional, in case the label starts in the first column (the consequence is that a machine or pseudo instruction must not start in column 1). It is necessary to set the colon in case the label does not start in the first column so that AS is able to distinguish it from a mnemonic. In the latter case, there must be at least one space between colon and mnemonic if the processor belongs to a family that supports an attribute that denotes an instruction format and is separated from the mnemonic by a colon. This restriction is necessary to avoid ambiguities: a distinction between a mnemonic with format and a label with mnemonic would otherwise be impossible.

Some signal processor families from Texas Instruments optionally use a double line ( `||` ) in place of the label to signify the parallel execution with the previous instruction(s). If these two assembler instructions become a single instruction word at machine level (C3x/C4x), an additional label in front of the second instruction of course does not make sense and is not allowed. The situation is different for the C6x with its instruction packets of variable length: If someone wants to jump into the middle of an instruction packet (bad style, if you ask me...), he has to place the necessary label _before_ into a separate line. The same is valid for conditions, which however may be combined with the double line in a single source line.

The attribute is used by a couple of processors to specify variations or different encodings of a certain instruction. The most prominent usage of the attribute is is the specification of the operand size, for example in the case of the 680x0 family:

| attribute | arithmetic-logic instruction        | jump instruction    |
| :-------- | :---------------------------------- | :------------------ |
| B         | byte (8 bits)                       | 8-bit-displacement  |
| W         | word (16 bits)                      | 16-bit-displacement |
| L         | long word (32 bits)                 | 16-bit-displacement |
| Q         | quad word (64 bits)                 | ------              |
| C         | half precision (16 bits)            | ------              |
| S         | single precision (32 bits)          | 8-bit-displacement  |
| D         | double precision (64 bits)          | ------              |
| X         | extended precision (80/96 bits)     | 32-bit-displacement |
| P         | decimal floating point (80/96 bits) | ------              |

Since this manual is not also meant as a user's manual for the processor families supported by AS, this is unfortunately not the place to enumerate all possible attributes for all families. It should however be mentioned that in general, not all instructions of a given instruction set allow all attributes and that the omission of an attribute generally leads to the usage of the "natural" operand size of a processor family. For more thorough studies, consult a reasonable programmer's manual, e.g. [^Williams] for the 68K's.

In the case of TLCS-9000, H8/500, and M16(C), the attribute serves both as an operand size specifier (if it is not obvious from the operands) and as a description of the instruction format to be used. A colon has to be used to separate the format from the operand size, e.g. like this:

```sh
add.w:g   rw10,rw8
```

This example does not show that there may be a format specification without an operand size. In contrast, if an operand size is used without a format specification, AS will automatically use the shortest possible format. The allowed formats and operand sizes again depend on the machine instruction and may be looked up e.g. in [^Tosh900], [^HitH8_5], [^MitM16], resp. [^MitM16C].

The number of instruction parameters depends on the mnemonic and is limited to 476 operands per source statement; individual instructions usually accept far fewer. The separation of the parameters from each other is to be performed only by commas (exception: DSP56xxx, its parallel data transfers are separated with blanks). Commas that are included in brackets or quotes, of course, are not taken into consideration.

Instead of a comment at the end, the whole line can consist of comment if it starts in the first column with a semicolon.

To separate the individual components you may also use tabulators instead of spaces.

## Format of the Listing

The listing produced by AS using the command line options `-l` or `-L` is roughly divisible into the following parts :

1. issue of the source code assembled;

2. symbol list;

3. usage list;

4. cross reference list.

The two last ones are only generated if they have been demanded by additional command line options.

In the first part, AS lists the complete contents of all source files including the produced code. A line of this listing has the following form:

```sh
[<n>] <line>/<address> <code> <source>
```

In the field `n` , AS displays the include nesting level. The main file (the file where assembly was started) has the depth 0, an included file from there has depth 1 etc... Depth 0 is not displayed.

In the field `line` , the source line number of the referenced file is issued. The first line of a file has the number 1\. The address at which the code generated from this line is written follows after the slash in the field `address` .

The code produced is written behind `address` in the field `code` , in hexadecimal notation. Depending on the processor type and actual segment the values are formatted either as bytes or 16/32-bit-words. If more code is generated than the field can take, additional lines will be generated, in which case only this field is used.

Finally, in the field `source` , the line of the source file is issued in its original form.

The symbol table was designed in a way that it can be displayed on an 80-column display whenever possible. For symbols of "normal length", a double column output is used. If symbols exceed (with their name and value) the limit of 40 columns (characters), they will be issued in a separate line. The output is done in alphabetical order. Symbols that have been defined but were never used are marked with a star (\*) as prefix.

<!-- markdownlint-disable MD038-->

The parts mentioned so far as well as the list of all macros/functions defined can be selectively masked out from the listing. This can be done by the already mentioned command line switch `-t` . There is an internal listing mask inside AS whose bits represent which parts are to be written. The assignment of bits to parts of the listing is listed in the following table:

<!-- markdownlint-enable MD038-->

| bit | part                           |
| :-- | :----------------------------- |
| 0   | source file(s) + produced code |
| 1   | symbol table                   |
| 2   | macro list                     |
| 3   | function list                  |
| 4   | line numbering                 |
| 5   | textual DEFINE list            |
| 6   | register symbol list           |
| 7   | character set table            |
| 8   | structure list                 |

All bits are set to 1 by default, when using the switch

```dosbat
-t <mask>
```

Bits set in `<mask>` are enabled by `-t`; `+t` clears those bits and suppresses the corresponding parts. The default mask is 511 (all nine bits set). If someone wants to have, for example, only the symbol table, it is enough to write:

```dosbat
+t 511 -t 2
```

The usage list issues the occupied areas hexadecimally for every single segment. If the area has only one address, only this is written, otherwise the first and last address.

The cross reference list issues any defined symbol in alphabetical order and has the following form:

```asm
symbol <symbol name> (=<value>,<file>/<line>):
    file <file 1>:
    <n1>[(m1)]  ..... <nk>[(mk)]
    .
    .
    file <file l>:
    <n1>[(m1)]  ..... <nk>[(mk)]
```

The cross reference list lists for every symbol in which files and lines it has been used. If a symbol was used several times in the same line, this would be indicated by a number in brackets behind the line number. If a symbol was never used, it would not appear in the list; The same is true for a file that does not contain any references for the symbol in question.

**CAUTION!** AS can only print the listing correctly if it was previously informed about the output media's page length and width! This has to be done with the `PAGE` instruction (see there). The preset default is a length of 60 lines and an unlimited line width.

### Unknown values in listings

Listing code is shown in units appropriate to the target and segment: bytes, 16-bit or 32-bit words, or shorter instruction words such as the PDP-8's 12-bit words. DSP56K listings use 24-bit words. On supported targets, fields whose values are unknown in the first pass are shown as question marks. `-no-list-unknown-values` disables this marking; `-list-unknown-values` enables it. Support initially covers 65xx, 8080/8085, Z80/eZ80/Z180/Z280/Z380, MCS-48/51/251, 8086/V20–V55, AVR, TMS370, MCS-96, and 6800/6805/68HC08/6809/68HC11/68HC12, 68xxx/ColdFire, PowerPC, and DSP56000/56300.

## Symbol Conventions

Symbol names are compared in full, rather than only by a prefix, and must meet the following naming conventions:

Symbol names are allowed to consist of a random combination of letters, digits, underlines and dots, whereby the first character must not be a digit. The dot is only allowed to meet the MCS-51 notation of register bits and should - as far as possible - not be used in own symbol names. To separate symbol names in any case the underline ( `_` ) and not the dot ( `.` ) should be used .

AS is by default not case-sensitive, i.e. it does not matter whether one uses upper or lower case characters. The command line switch `U` however allows to switch AS into a mode where upper and lower case makes a difference. The predefined symbol `CASESENSITIVE` signifies whether AS has been switched to this mode: TRUE means case-sensitiveness, and FALSE its absence.

The following table shows the most important symbols which are predefined by AS.

| name                 | meaning                                                       |
| :------------------- | :------------------------------------------------------------ |
| `TRUE`               | logically "true"                                              |
| `FALSE`              | logically "false"                                             |
| `CONSTPI`            | Pi (3.1415.....)                                              |
| `VERSION`            | version of AS in BCD-coding, e.g. 1331 hex for version 1.33p1 |
| `ARCHITECTURE`       | target platform AS was compiled for, in the style system      |
| `DATE`               | date of the assembly (start)                                  |
| `TIME`               | time of the assembly (start)                                  |
| `MOMCPU`             | current target CPU (see the CPU instruction)                  |
| `MOMFILE`            | current source file                                           |
| `MOMLINE`            | line number in source file                                    |
| `MOMPASS`            | number of the currently running pass                          |
| `MOMSECTION`         | name of the current section or an empty string                |
| `*` , `$` resp. `PC` | current value of program counter                              |

**CAUTION!** While it does not matter in case-sensitive mode which combination of upper and lower case to use to reference predefined symbols, one has to use exactly the version given above (only upper case) when AS is in case-sensitive mode!

Additionally some pseudo instructions define symbols that reflect the value that has been set with these instructions. Their descriptions are explained at the individual commands belonging to them.

A hidden feature (that has to be used with care) is that symbol names may be assembled from the contents of string symbols. This can be achieved by framing the string symbol's name with curly braces and inserting it into the new symbol's name. This allows for example to define a symbol's name based on the value of another symbol:

```asm
cnt             set     cnt+1
temp            equ     "\{CNT}"
                jnz     skip{temp}
                .
                .
skip{temp}:     nop
```

**CAUTION:** The programmer has to assure that only valid symbol names are generated!

A complete list of all symbols predefined by AS can be found in [Predefined Symbols](predefined-symbols.md).

Apart from its value, every symbol also owns a marker which signifies to which _segment_ it belongs. Such a distinction is mainly needed for processors that have more than one address space. The additional information allows AS to issue a warning when a wrong instruction is used to access a symbol from a certain address space. A segment attribute is automatically added to a symbol when is gets defined via a label or a special instruction like `BIT` ; a symbol defined via the "allround instructions" `SET` resp. `EQU` is however "typeless", i.e. its usage will never trigger warnings. A symbol's segment attribute may be queried via the built-in function `SYMTYPE` , e.g.:

```asm
Label:
        .
        .
Attr    equ     symtype(Label)  ; results in 1
```

The individual segment types have the assigned numbers listed in the following table. Register symbols which do not really fit into the order of normal symbols are explained in [Register Symbols](#register-symbols). The `SYMTYPE` function delivers -1 as result when called with an undefined symbol as argument. However, if all you want to know is whether a symbol is defined or not, you may as well use the `DEFINED` function.

### Symbol existence across passes

`SYMEXIST(symbol)` and `IFSYMEXIST symbol` query whether a symbol is in the symbol table. A forward declaration is absent in the first pass and present in subsequent passes. `IFNSYMEXIST` reverses this condition. In contrast, `DEFINED(expression)` and `IFDEF expression` check definitions encountered up to the current point in the source, including every symbol in an expression. The fork also recognizes macro and function names in `IFDEF` and `IFNDEF`.

##### **Table:** return values of the `SYMTYPE` function

| segment             | return value |
| :------------------ | :----------: |
| `<none>`            |      0       |
| `CODE`              |      1       |
| `DATA`              |      2       |
| `IDATA`             |      3       |
| `XDATA`             |      4       |
| `YDATA`             |      5       |
| `BITDATA`           |      6       |
| `IO`                |      7       |
| `REG`               |      8       |
| `ROMDATA`           |      9       |
| `EEDATA`            |      10      |
| `<register symbol>` |     128      |

## Temporary Symbols

Especially when dealing with programs that contain sequences of loops of if-like statements, one is continuously faced with the problem of inventing new names for labels - labels of which you know exactly that you will never need to reference them again afterwards and you really would like to get 'rid' of them somehow. A simple solution if you don't want to swing the large hammer of sections (see [Local Symbols](pseudo-instructions.md#local-symbols)) are _temporary_ symbols which remain valid as long as a new, non-temporary symbol gets defined. Other assemblers offer a similar mechanism which is commonly referred as 'local symbols'; however, for the sake of a better distinction, I want to stay with the term 'temporary symbols'. AS knows three different types of temporary symbols, in the hope to offer everyone 'switching' to AS a solution that makes conversion as easy as possible. However, practically every assembler has its own interpretation of this feature, so there will be only few cases where a 1:1 solution for existing code:

## Named Temporary Symbols

A symbol whose name starts with two dollar signs (something that is neither allowed for non-temporary symbols nor for constants) is a named temporary symbol. AS keeps an internal counter which is reset to 0 before assembly begins and which gets incremented upon every definition of a non-temporary symbol. When a temporary symbol is defined or referenced, both leading dollar signs are discarded and the counter's current value is appended. This way, one regains the used symbol names with every definition of a non-temporary symbol - but you also cannot reach the previously symbols any more! Temporary symbols are therefore especially suited for usage in small instruction blocks, typically a dozen of machine instructions, definitely not more than one screen. Otherwise, one easily gets confused...

Here is a small example:

```asm
$$loop: nop
        dbra    d0,$$loop

split:

$$loop: nop
        dbra    d0,$$loop
```

Without the non-temporary label between the loops, of course an error message about a double-defined symbol would be the result.

### Nameless Temporary Symbols

For all those who regard named temporary symbols still as too complicated, there is an even simpler variant: If one places a single plus or minus sign as a label, this is converted to special internal symbol names. Those symbols are "unpronounceable" and unpredictable, and should not be relied on in a program; instead, they are referenced via the special names `- – —` respectively `+ ++ +++` , which refer to the three last 'minus symbols' and the next three 'plus symbols'. Therefore, the selection between these two variants depends on whether one wants to forward- or backward-reference a symbol.

Apart from plus and minus, _defining_ nameless temporary symbols also exists in a third variant, namely a slash (/). A temporary symbol defined in this way may be referenced both backward and forward, i.e. it is treated either as a plus or a minus, depending on the way it is being referenced.

Nameless temporary symbols are usually used in constructs that fit on one screen page, like skipping a few machine instructions or tight loops - things would become too puzzling otherwise (this only a good advice, however...). An example for this is the following piece of code, this time as 65xx code:

```asm
        cpu     6502

-       ldx     #00
-       dex
        bne     -           ; branch to 'dex'
        lda     RealSymbol
        beq     +           ; branch to 'bne --'
        jsr     SomeRtn
        iny
+       bne     --          ; branch to 'ldx #00'

SomeRtn:
        rts

RealSymbol:
        dfs     1

        inc ptr
        bne     +           ; branch to 'tax'
        inc     ptr+1
+       tax

        bpl     ++          ; branch to 'dex'
        beq     +           ; branch forward to 'rts'
        lda     #0
/       rts                 ; slash used as wildcard.
+       dex
        beq     -           ; branch backward to 'rts'

ptr:    dfs 2
```

### Composed Temporary Symbols

This is maybe the type of temporary symbols that is nearest to the concept of local symbols and sections. Whenever a symbol's name begins with a dot (.), the symbol is not directly stored with this name in the symbol table. Instead, the name of the most recently-defined symbol not beginning with a dot is prepended to the symbols name. This way, 'non-dotted' symbols take the role of section separators and 'dotted' symbol names may be reused after a 'non-dotted' symbol has been defined. Take a look at the following little example:

```asm
proc1:                      ; non-temporary symbol 'proc1'

.loop   moveq   #20,d0      ; actually defines 'proc1.loop'
        dbra    d0,.loop
        rts

proc2:                      ; non-temporary symbol 'proc2'

.loop   moveq   #10,d1      ; actually defines 'proc2.loop'
        jsr proc1
        dbra    d1,.loop
        rts
```

Note that it is still possible to access all temporary symbols, even without being in the same 'area', by simply using the composed name (like 'proc2.loop' in the previous example).

It is principally possible to combine composed temporary symbols with sections, which makes them also to local symbols. Take however into account that the most recent non-temporary symbol is not stored per-section, but simply globally. This may change however in a future version, so one shouldn't rely on the current behaviour.

## Formula Expressions

In most places where the assembler expects numeric inputs, it is possible to specify not only simple symbols or constants, but also complete formula expressions. The components of these formula expressions can be either single symbols and constants. Constants may be either integer, floating point, or string constants.

### Integer Constants

Integer constants describe non-fractional numbers. They are written as a sequence of digits. This may be done in different numbering systems, as shown in the following table.

##### **Table:** Defined Numbering Systems and Notations

| <!--  --> |    Intel Mode     | Motorola Mode |   C Mode    |       IBM Mode       |
| :-------- | :---------------: | :-----------: | :---------: | :------------------: |
| Decimal   |      Direct       |    Direct     |   Direct    |        Direct        |
| Hex       |    Suffix `H`     |  Prefix `$`   | Prefix `0x` | `X'...'` or `H'...'` |
| _Ident_   |      `hexh`       |    `$hex`     |   `0xhex`   | `x'hex'` or `h'hex'` |
| Binary    |    Suffix `B`     |  Prefix `%`   | Prefix `0b` |       `O'...'`       |
| _Ident_   |      `binb`       |    `%bin`     |   `0bbin`   |       `b'bin'`       |
| Octal     | Suffix `O` or `Q` |  Prefix `@`   | Prefix `0`  |       `B'...'`       |
| _Ident_   | `octo` or `octq`  |    `@oct`     |   `0oct`    |       `o'oct'`       |

In case the numbering system has not been explicitly stated by adding the special control characters listed in the table, AS assumes the base given with the `RADIX` statement (which has itself 10 as default). This statement allows to set up 'unusual' numbering systems, i.e. others than 2, 8, 10, or 16.

Valid digits are numbers from 0 to 9 and letters from A to Z (value 10 to 35) up to the numbering system's base minus one. The usage of letters in integer constants however brings along some ambiguities since symbol names also are sequences of numbers and letters: a symbol name must not start with a character from 0 to 9\. This means that an integer constant which is not clearly marked a such with a special prefix character never may begin with a letter. One has to add an additional, otherwise superfluous zero in front in such cases. The most prominent case is the writing of hexadecimal constants in Intel mode: If the leftmost digit is between A and F, the trailing H doesn't help anything, an additional 0 has to be prefixed (e.g. 0F0H instead of F0H). The Motorola and C syntaxes, which both mark the numbering system at the front of a constant, do not have this problem.

Quite tricky is furthermore that the higher the default numbering system set via `RADIX` becomes, the more letters used to denote numbering systems in Intel and C syntax become 'eaten'. For example, you cannot write binary constants anymore after a `RADIX 16` , and starting at `RADIX 18` , the Intel syntax even doesn't allow to write hexadecimal constants any more. Therefore **CAUTION!**

[Pseudo-Instructions and Integer Syntax](pseudo-instructions-and-integer-syntax.md) lists which syntax is used by which target by default. Independent of this default, there is always the option to add or delete individual syntax variants via the `INTSYNTAX` instruction (see [INTSYNTAX](pseudo-instructions.md#intsyntax)). The names listed as _Ident_, prefixed with a plus or minus sign, serve as arguments to this instruction.

The `RELAXED` instruction (see [RELAXED](pseudo-instructions.md#relaxed)) serves as a sort 'global enable switch': in relaxed mode, all notations may be used, independent of the selected target processor. The result is that an arbitrary syntax may be used (possibly loosing compatibility to standard assemblers).

Both `INTSYNTAX` and `RELAXED` specifically enable usage of the 'IBM syntax' for all targets, which is sometimes found on other assemblers:

This notation puts the actual value into apostrophes and prepends the numbering system ('x' or 'h' for hexadecimal, 'o' for octal and 'b' for binary). So, the integer constant 305419896 can be written in the following ways:

```asm
x'12345678'
h'12345678'
o'2215053170'
b'00010010001101000101011001111000'
```

Another variant of this notation for some targets is to leave away the closing apostrophe, to allow simpler porting of existing code. It is not recommended for new programs.

### Floating Point Constants

Floating point constants are to be written in the usual scientific notation, which is known in the most general form:

```asm
[-]<integer digits>[.post decimal positions][E[-]exponent]
```

**CAUTION!** The assembler first tries to interpret a constant as an integer constant and makes a floating-point format try only in case the first one failed. If someone wants to enforce the evaluation as a floating point number, this can be done by dummy post decimal positions, e.g. `2.0` instead of `2` .

### String Constants

String constants have to be enclosed in single or double quotation marks. In order to make it possible to include quotation marks or special characters in string constants, an "escape mechanism" has been implemented, which should sound familiar for C programmers:

The assembler understands a backslash ( `\` ) with a following decimal number of three digits maximum in the string as a character with the according decimal ASCII value. The numerical value may alternatively be written in hexadecimal or octal notation if it is prefixed with an x resp. a 0\. In case of hexadecimal notation, the maximum number of digits is limited to 2\. For example, `\3` inserts an ETX character and `\000` inserts NUL. Expression strings carry an explicit length, so embedded NUL bytes are preserved in string data and string operations; a filename or other operating-system string still cannot contain NUL.

Some frequently used control characters can also be reached with the following abbreviations:

```asm
\b : Backspace           \a : Bell         \e : Escape
\t : Tabulator           \n : Linefeed     \r : Carriage Return
\\ : Backslash           \' or \H : Apostrophe
\" or \I : Quotation marks
```

Both upper and lower case characters may be used for the identification letters.

By means of this escape character, you can even work formula expressions into a string, if they are enclosed by curly braces: e.g.

```asm
message "root of 81 : \{sqrt(81)}"
```

results in

```asm
root of 81 : 9
```

AS chooses with the help of the formula result type the correct output format, further string constants, however, are to be avoided in the expression. Otherwise the assembler will get mixed up at the transformation of capitals into lower case letters. Integer results will by default be written in hexadecimal notation, which may be changed via the `OUTRADIX` instruction.

Except for the insertion of formula expressions, you can use this "escape-mechanism" as well in ASCII defined integer constants, like this:

```asm
move.b   #'\n',d0
```

However, everything has its limits, because the parser with higher priority, which disassembles a line into op-code and parameters, does not know what it is actually working with, e.g. here:

```asm
move.l   #'\'abc',d0
```

After the third apostrophe, it will not find the comma any more, because it presumes that it is the start of a further character constant. An error message about a wrong parameter number is the result. A workaround would be to write e.g., `\i` instead of `\'` .

### String to Integer Conversion and Character Constants

Earlier versions of AS strictly distinguished between character strings and so-called "character constants": At first glance, a character constant looks like a string, the characters are however enclosed in single instead of double quotation marks. Such an object had the data type 'Integer', i.e. it represented a number with the value given by the (ASCII) code of the character, and it was something completely different:

```asm
move.b   #65,d0
move.b   #'A',d0      ; equal to first instruction
move.b   #"A",d0      ; not allowed in older versions of AS!
```

This strict differentiation _no longer exists_, so it is irrelevant whether single or double quotes are used. If an integer value is expected as argument, and a string is used, the conversion via the character's (ASCII) value is done "on the fly" at this place. This means that in the example given, _all three lines_ result in the same machine code.

Such an implicit conversion to integer values also take place for strings consisting of multiple constants, which are sometimes called "multi character constants":

```asm
'A'    ==$41
'AB'   ==$4142
'ABCD' ==$41424344
```

Multi character constants are the only case where using single or double quotes still makes a difference. Many targets define pseudo instructions to dispose constants in memory, and which accept different data types. In such a case, it is still necessary to use double quotes if a character string shall be placed in memory:

```asm
dc.w    "ab"  ; disposes two words (0x0041,0x0042)
dc.w    'ab'  ; disposes one word (0x4142)
```

Important: using the correct quotation is not necessary if the character string is longer than the used operand size, which is two characters or 16 bits in this example.

### Evaluation

Expressions use the configured host precision: 64-bit integers, or 128-bit integers where supported, and IEEE double or extended floating point. `INTWIDTH` reports the integer width and `HAS64` indicates that it is at least 64 bits. The approximate finite floating-point range is ±1.8 × 10^308 for double and ±1.1 × 10^4932 for extended precision. `FLOATMAX` reports the actual host limit. Range checks for the requested operand type apply to the final result.

### Operators

The assembler provides the operands listed in the following table for combination.

<!-- markdownlint-disable MD038 -->

##### **Table:** Operators Predefined by AS

| operand | function         | #operands |  integer  |   float   |  string   |   rank    |
| :-----: | :--------------- | :-------: | :-------: | :-------: | :-------: | :-------: |
|  `<>`   | inequality       |     2     |    yes    |    yes    |    yes    |    14     |
|  `!=`   | alias for `<>`   | <!--  --> | <!--  --> | <!--  --> | <!--  --> | <!--  --> |
|  `>=`   | greater or equal |     2     |    yes    |    yes    |    yes    |    14     |
|  `<=`   | less or equal    |     2     |    yes    |    yes    |    yes    |    14     |
|   `<`   | truly smaller    |     2     |    yes    |    yes    |    yes    |    14     |
|   `>`   | truly greater    |     2     |    yes    |    yes    |    yes    |    14     |
|   `=`   | equality         |     2     |    yes    |    yes    |    yes    |    14     |
|  `==`   | alias for `=`    | <!--  --> | <!--  --> | <!--  --> | <!--  --> | <!--  --> |
|  `!!`   | log. XOR         |     2     |    yes    |    no     |    no     |    13     |
| `\|\|`  | log. OR          |     2     |    yes    |    no     |    no     |    12     |
|  `&&`   | log. AND         |     2     |    yes    |    no     |    no     |    11     |
|  `~~`   | log. NOT         |     1     |    yes    |    no     |    no     |     2     |
|   `-`   | difference       |     2     |    yes    |    yes    |    no     |    10     |
|   `+`   | sum              |     2     |    yes    |    yes    |    yes    |    10     |
|   `#`   | modulo division  |     2     |    yes    |    no     |    no     |     9     |
|   `/`   | quotient         |     2     |  yes\*)   |    yes    |    no     |     9     |
|   `*`   | product          |     2     |    yes    |    yes    |    no     |     9     |
|   `^`   | power            |     2     |    yes    |    yes    |    no     |     8     |
|   `!`   | binary XOR       |     2     |    yes    |    no     |    no     |     7     |
|  `\|`   | binary OR        |     2     |    yes    |    no     |    no     |     6     |
|   `&`   | binary AND       |     2     |    yes    |    no     |    no     |     5     |
|  `><`   | mirror of bits   |     2     |    yes    |    no     |    no     |     4     |
|  `>>`   | log. shift right |     2     |    yes    |    no     |    no     |     3     |
|  `<<`   | log. shift left  |     2     |    yes    |    no     |    no     |     3     |
|   `~`   | binary NOT       |     1     |    yes    |    no     |    no     |     1     |
|   `-`   | unary negation   |     1     |    yes    |    yes    |    no     |     1     |

\*) remainder will be discarded

<!-- markdownlint-enable MD038 -->

"Rank" is the priority of an operator at the separation of expressions into subexpressions. The operator with the highest rank will be evaluated at the very end. The order of evaluation can be defined by new bracketing.

Unary negation binds more tightly than binary operators: `2*-value` means `2*(-value)`, and `-value^2` means `(-value)^2`. Chained prefix operators apply from right to left, so `-~value` means `-(~value)`. Short runs of `+` or `-` used as temporary symbols retain their existing meaning.

The compare operators deliver TRUE in case the condition fits, and FALSE in case it doesn't. For the logical operators an expression is TRUE in case it is not 0, otherwise it is FALSE.

The mirroring of bits probably needs a little bit of explanation: the operator mirrors the lowest bits in the first operand and leaves the higher priority bits unchanged. The number of bits which is to be mirrored is given by the right operand and may be between 1 and 32 .

A small pitfall is hidden in the binary complement: As computation uses the internal integer width reported by `INTWIDTH`, its application on e.g. 8-bit masks usually results in values that do not fit into 8-bit numbers any more due to the leading ones. A binary AND with a fitting mask is therefore unavoidable!

### Functions

In addition to the operators, the assembler defines another line of primarily transcendental functions with floating point arguments which are listed in this table:

##### **Table:** Functions Predefined by AS

| name        | meaning                                | argument                    | result                             |
| :---------- | :------------------------------------- | :-------------------------- | :--------------------------------- |
| SQRT        | square root                            | _arg_ ≥ 0                   | floating point                     |
| SIN         | sine                                   | floating point              | floating point                     |
| COS         | cosine                                 | floating point              | floating point                     |
| TAN         | tangent                                | _arg_ ≠ (2*n*+1) \* _π_ / 2 | floating point                     |
| COT         | cotangent                              | _arg_ ≠ _n_ \* _π_          | floating point                     |
| ASIN        | inverse sine                           | ∣ _arg_ ∣ ≤ 1               | floating point                     |
| ACOS        | inverse cosine                         | ∣ _arg_ ∣ ≤ 1               | floating point                     |
| ATAN        | inverse tangent                        | floating point              | floating point                     |
| ACOT        | inverse cotangent                      | floating point              | floating point                     |
| EXP         | exponential function                   | floating point              | floating point                     |
| ALOG        | 10 power of argument                   | floating point              | floating point                     |
| ALD         | 2 power of argument                    | floating point              | floating point                     |
| SINH        | hyp. sine                              | floating point              | floating point                     |
| COSH        | hyp. cosine                            | floating point              | floating point                     |
| TANH        | hyp. tangent                           | floating point              | floating point                     |
| COTH        | hyp. cotangent                         | _arg_ ≠ 0                   | floating point                     |
| LN          | nat. logarithm                         | _arg_ > 0                   | floating point                     |
| LOG         | dec. logarithm                         | _arg_ > 0                   | floating point                     |
| LD          | bin. logarithm                         | _arg_ > 0                   | floating point                     |
| ASINH       | inv. hyp. Sine                         | floating point              | floating point                     |
| ACOSH       | inv. hyp. Cosine                       | _arg_ ≥ 1                   | floating point                     |
| ATANH       | inv. hyp. Tangent                      | _arg_ \< 1                  | floating point                     |
| ACOTH       | inv. hyp. Cotangent                    | _arg_ > 1                   | floating point                     |
| INT         | integer part                           | floating point              | floating point                     |
| BITCNT      | number of one's                        | integer                     | integer                            |
| FIRSTBIT    | lowest 1-bit                           | integer                     | integer                            |
| LASTBIT     | highest 1-bit                          | integer                     | integer                            |
| BITPOS      | unique 1-bit                           | integer                     | integer                            |
| PHYS2CPU    | physical to CPU address mapping        | integer                     | integer                            |
| CPU2PHYS    | CPU to physical address mapping        | integer                     | integer                            |
| SGN         | sign (0/1/-1)                          | floating point or integer   | integer                            |
| ABS         | absolute value                         | integer or floating point   | integer or floating point          |
| TOUPPER     | matching capital                       | integer                     | integer                            |
| TOLOWER     | matching lower case                    | integer                     | integer                            |
| UPSTRING    | changes all characters into capitals   | string                      | string                             |
| LOWSTRING   | changes all characters into lower case | string                      | string                             |
| STRLEN      | returns the length of a string         | string                      | integer                            |
| SUBSTR      | extracts parts of a string             | string, integer, integer    | string                             |
| CHARFROMSTR | extracts a character from a string     | string, integer             | integer                            |
| STRSTR      | searches a substring in a string       | string, string              | integer                            |
| VAL         | evaluates contents as expression       | string                      | depends on argument                |
| EXPRTYPE    | delivers type of argument              | integer, float, or string   | integer = 0, float = 1, string = 2 |

The functions `FIRSTBIT` , `LASTBIT` , and `BITPOS` return -1 as result if no resp. not exactly one bit is set. `BITPOS` additionally issues an error message in such a case.

The string function `SUBSTR` expects the source string as first parameter, the start position as second and the number of characters to be extracted as third parameter (a 0 means to extract all characters up to the end). Similarly, `CHARFROMSTR` expects the source string as first argument and the character position as second argument. In case the position argument is larger or equal to the source string's length, `SUBSTR` returns an empty string while `CHARFROMSTR` returns -1\. A position argument smaller than zero is treated as zero by `SUBSTR` , while `CHARFROMSTR` will return -1 also in this case.

Here is an example how to use these both functions. The task is to put a string into memory, with the string end being signified by a set MSB in the last character:

```asm
dbstr   macro   arg
        if      strlen(arg) > 1
         db     substr(arg, 0, strlen(arg) - 1)
        endif
        if      strlen(arg) > 0
         db     charfromstr(arg, strlen(arg) - 1) | 80h
        endif
        endm
```

`STRSTR` returns the first occurrence of the second string within the first one resp. -1 if the search pattern was not found. Similarly to `SUBSTR` and `CHARFROMSTR` , the first character has the position 0.

If a function expects floating point arguments, this does not mean it is impossible to write e.g.

```asm
sqr2 equ sqrt(2)
```

In such cases an automatic type conversion is engaged. In the reverse case the `INT` -function has to be applied to convert a floating point number to an integer. When using this function, you have to pay attention that the result produced always is a signed integer and therefore has a value range of approximately +/-2.0E9.

When AS is switched to case-sensitive mode, predefined functions may be accessed with an arbitrary combination of upper and lower case (in contrast to predefined symbols). However, in the case of user-defined functions (see [FUNCTION](pseudo-instructions.md#function)), a distinction between upper and lower case is made. This has e.g. the result that if one defines a function `Sin` , one can afterwards access this function via `Sin` , but all other combinations of upper and lower case will lead to the predefined function.

## Forward References and Other Disasters

This section is the result of a significant amount of hate on the (legal) way some people program. This way can lead to trouble in conjunction with AS in some cases. The section will deal with so-called 'forward references'. What makes a forward reference different from a usual reference? To understand the difference, take a look at the following programming example (please excuse my bias for the 68000 family that is also present in the rest of this manual):

```asm
        move.l  #10,d0
loop:   move.l  (a1),d1
        beq     skip
        neg.l   d1
skip:   move.l  d1,(a1+)
        dbra    d0,loop
```

If one overlooks the loop body with its branch statement, a program remains that is extremely simple to assemble: the only reference is the branch back to the body's beginning, and as an assembler processes a program from the beginning to the end, the symbol's value is already known before it is needed the first time. If one has a program that only contains such backward references, one has the nice situation that only one pass through the source code is needed to generate a correct and optimal machine code. Some high level languages like Pascal with their strict rule that everything has to be defined before it is used exploit exactly this property to speed up the compilation.

Unfortunately, things are not that simple in the case of assembler, because one sometimes has to jump forward in the code or there are reasons why one has to move variable definitions behind the code. For our example, this is the case for the conditional branch that is used to skip over another instruction. When the assembler hits the branch instruction in the first pass, it is confronted with the situation of either leaving blank all instruction fields related to the target address or offering a value that "hurts no one" via the formula parser (which has to evaluate the address argument). In case of a "simple" assembler that supports only one target architecture with a relatively small number of instructions to treat, one will surely prefer the first solution, but the effort for AS with its dozens of target architectures would have become extremely high. Only the second way was possible: If an unknown symbol is detected in the first pass, the formula parser delivers the program counter's current value as result! This is the only value suitable to offer an address to a branch instruction with unknown distance length that will not lead to errors. This answers also a frequently asked question why a first-pass listing (it will not be erased e.g. when AS does not start a second pass due to additional errors) partially shows wrong addresses in the generated binary code - they are the result of unresolved forward references.

The example listed above however uncovers an additional difficulty of forward references: Depending on the distance of branch instruction and target in the source code, the branch may be either long or short. The decision however about the code length - and therefore about the addresses of following labels - cannot be made in the first pass due to missing knowledge about the target address. In case the programmer did not explicitly mark whether a long or short branch shall be used, genuine 2-pass assemblers like older versions of MASM from Microsoft "solve" the problem by reserving space for the longest version in the first pass (all label addresses have to be fixed after the first pass) and filling the remaining space with `NOP` s in the second pass. AS versions up to 1.37 did the same before I switched to the multipass principle that removes the strict separation into two passes and allows an arbitrary number of passes. Said in detail, the optimal code for the assumed values is generated in the first pass. In case AS detects that values of symbols changed in the second pass due to changes in code lengths, simply a third pass is done, and as the second passes' new symbol values might again shorten or lengthen the code, a further pass is not impossible. I have seen 8086 programs that needed 12 passes to get everything correct and optimal. Unfortunately, this mechanism does not allow to specify a maximum number passes; I can only advise that the number of passes goes down when one makes more use of explicit length specifications.

Especially for large programs, another situation might arise: the position of a forward directed branch has moved so much in the second pass relative to the first pass that the old label value still valid is out of the allowed branch distance. AS knows of such situations and suppresses all error messages about too long branches when it is clear that another pass is needed. This works for 99% of all cases, but there are also constructs where the first critical instruction appears so early that AS had no chance up to now to recognize that another pass is needed. The following example constructs such a situation with the help of a forward reference (and was the reason for this section's heading...):

```asm
        cpu   6811
        org     $8000
        beq     skip
        rept    60
        ldd     Var
        endm
skip:   nop

Var     equ     $10
```

Due to the address position, AS assumes long addresses in the first pass for the `LDD` instructions, what results in a code length of 180 bytes and an out of branch error message in the second pass (at the point of the `BEQ` instruction, the old value of `skip` is still valid, i.e. AS does not know at this point that the code is only 120 bytes long in reality) is the result. The error can be avoided in three different ways:

1. Explicitly tell AS to use short addressing for the `LDD` instructions (`ldd <Var`)

2. Remove this damned, rotten forward reference and place the `EQU` statement at the beginning where it has to be (all right, I'm already calming down...)

3. For real die-hards: use the `-Y` command line option. This option tells AS to forget the error message when the address change has been detected. Not pretty, but...

Another tip regarding the `EQU` instruction: AS cannot know in which context a symbol defined with `EQU` will be used, so an `EQU` containing forward references will not be done at all in the first pass. Thus, if the symbol defined with `EQU` gets forward-referenced in the second pass:

```asm
        move.l  #sym2,d0
        sym2    equ     sym1+5
        sym1    equ     0
```

one gets an error message due to an undefined symbol in the second pass...but why on earth do people do such things?

Admittedly, this was quite a lengthy excursion, but I thought it was necessary. Which is the essence you should learn from this section?

1. AS always tries to generate the shortest code possible. A finite number of passes is needed for this. If you do not tweak AS extremely, AS will know no mercy...

2. Whenever sensible and possible, explicitly specify branch and address lengths. There is a chance of significantly reducing the number of passes by this.

3. Limit forward references to what is absolutely needed. You make your and AS's live much easier this way!

## Register Symbols

_valid for: PowerPC, M-Core, XGate, 4004/4040, MCS-48/(2)51, 8086, 80C16x, AVR, XS1, Z8, KCPSM, Mico8, MSP430(X), ST9, M16, M16C, H8/300, H8/500, SH7x00, H16, i960, XA, 29K, TLCS-9000, KENBAK, SC/MP_

Sometimes it is desirable not only to assign symbolic names to memory addresses or constants, but also to a register, to emphasize its function in a certain program section. This is no problem for processors that treat registers simply as another address space, as this allows to use numeric expressions and one can use simple `EQU`s to define such symbols. (e.g. for the MCS-96 or TMS70000). However, for most processors, register identifiers are fixed literals which are separately treated by AS for speed reasons. Therefore, registers symbols (sometime also called 'register aliases') are also a separate type of symbols in the symbol table. Just like other symbols, they may be defined or re-defined with `EQU` or `SET`, and there is a specialized `REG` instruction which accepts only symbols and expressions of this type.

On the other hand, register symbols are subject of a couple of restrictions: the number of literals is limited and depends on the selected target processor, and arithmetic operations are not possible on registers A construct like this:

```asm
myreg   reg     r17         ; definition of register symbol
        addi    myreg+1,3   ; does not work!
```

is _not_ valid. Simple assignments are however possible:

```asm
myreg   reg     r17         ; definition of register symbol
myreg2  reg     myreg       ; myreg2 -> r17
```

Furthermore, forward references are even more critical than for other types of symbols. If a symbol is not (yet) defined, AS does not know which type it is going to have,a nd will decide for a plain integer number. For most target processors, a number is the equivalent of absolute memory addressing, and on most processors, usage of memory operands is more limited than of registers. Depending on situation, one will get an error message about a non-allowed addressing mode, and no second pass will be started...

Analogous to ordinary symbols, register symbols are local to sections and it is possible to access a register symbol from a specific section by appending the section's name enclosed in brackets.

## Share File

This function is a by-product from the old pure-68000 predecessors of AS, I have kept them in case someone really needs it. The basic problem is to access certain symbols produced during assembly, because possibly someone would like to access the memory of the target system via this address information. The assembler allows to export symbol values by means of `SHARED` pseudo commands (see there). For this purpose, the assembler produces a text file with the required symbols and its values in the second pass. This file may be included into a higher-level language or another assembler program. The format of the text file (C, Pascal or Assembler) can be set by the command line switches `p, c` or, `a`.

**CAUTION!** If none of the switches is given, no file will be generated and it makes no difference if `SHARED`-commands are in the source text or not!

When creating a Sharefile, AS does not check if a file with the same name already exists, such a file will be simply overwritten. In my opinion a request does not make sense, because AS would ask at each run if it should overwrite the old version of the Sharefile, and that would be really annoying...

## Processor Aliases

Common microcontroller families are like rabbits: They become more at a higher speed than you can provide support for them. Especially the development of processor cores as building blocks for ASICs and of microcontroller families with user-definable peripherals has led to a steeply rising number of controllers that only deviate from a well-known type by a slightly modified peripheral set. But the distinction among them is still important, e.g. for the design of include files that only define the appropriate subset of peripherals. I have struggled up to now to integrate the most important representatives of a processor family into AS (and I will continue to do this), but sometimes I just cannot keep pace with the development...there was an urgent need for a mechanism to extend the list of processors by the user.

The result are processor aliases: the alias command line option allows to define a new processor type, whose instruction set is equal to another processor built into AS. After switching to this processor via the `CPU` instruction, AS behaves exactly as if the original processor had been used, with a single difference: the variables `MOMCPU` resp. `MOMCPUNAME` are set to the alias name, which allows to use the new name for differentiation, e.g. in include files.

There were two reasons to realize the definition of aliases by the command line and not by pseudo instructions: first, it would anyway be difficult to put the alias definitions together with register definitions into a single include file, because a program that wants to use such a file would have to include it before and after the CPU instruction - an imagination that lies somewhere between inelegant and impossible. Second, the definition in the command line allows to put the definitions in a key file that is executed automatically at startup via the `ASCMD` variable, without a need for the program to take any further care about this.

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->


`CODEPAGE_VAL(character[, name])` returns the translated byte for a character code from 0 through 255. Omitting `name` uses the current codepage; a string name selects another codepage without changing the current one.


Register symbols support `=`, `==`, `<>`, `!=`, `<`, `>`, `<=` and `>=`. Aliases for the same physical register compare equal (for example, 68000 `SP` and `A7`). Registers from different register files or architectures have no ordering: `<`, `>`, `<=` and `>=` return false, while equality and inequality remain meaningful. `!=` is also an alias for `<>` with integers, floats and strings.

The optional `INTSYNTAX +a'asc'` format accepts `A'ABCD'` as a multi character constant equivalent to `'ABCD'`, using the current character translation table.

The `-bigendian` and `-packing` command line switches override the initial `BIGENDIAN` and `PACKING` values on targets that support them. Negating a switch selects the opposite default.

Multi character constants use big endian order by default (`'ABCD'` = 0x41424344). PDP-11 and WD16 use little endian order to match MACRO-11 (`'AB'` = 0x4241).

### Help and message levels

Run `asl -help` for command-line help, `asl -version` for version information, and `asl -cpu list` or `asl -cpu ?` for the target list. With no input filenames, ASL exits instead of asking for them interactively. `-v` enables verbose output; normal operation does not print the version banner. `-screenheight` enables paging, which is disabled by default.

`-q`/`-quiet` suppresses normal progress; repeating it also suppresses source `MESSAGE` output. Error messages remain visible.

CMake builds embed message catalogs by default, so installed executables do not require companion `.msg` files. Configure with `-DASL_EMBED_MESSAGES=OFF` to use external catalogs and their existing language/path lookup behavior.

A leading zero in `-listradix` (for example, `-listradix 08`) prints the program counter with leading zeros in the listing. The radix remains octal in this example; the supported radix range is 2 through 36.


### Underscores in macro parameters

`-underscore-macroargs` permits underscores in macro parameter names. By default, underscores delimit parameter references: a parameter `led` is expanded in both `led` and `led_shadow`. With this option, only the standalone `led` is expanded. Shipped include files may rely on the default behavior. Explicit backslash-delimited parameter references remain available in this fork.

`-plainbase` enables the 6809 indexed-address shorthand described under [PLAINBASE](pseudo-instructions.md#plainbase).


The listing learns maximum include depth in the first pass and retains it across later passes, reserving a consistent-width depth field. The selected target address space determines address-field width.


`-wrelative` enables warnings when a Z80 absolute jump can use a relative form; `-wno-relative` disables them. `-wimplicit-sign-extension` and `-wno-implicit-sign-extension` control implicit 68000 `MOVEQ` sign-extension warnings.


### Listing line prefixes

`-listline-prefix` sets the format before each source line. `%i` inserts include depth, `%n` the source line number (default width 5), and `%a` the current address (target-specific width). A width may be placed before the letter, such as `%8a`; a leading zero requests zero padding. Include depth zero may leave a blank field or omit it when no includes are used. The default is `%i%n/%a`; `%1i%5n/%8a` recreates the format from before build249. `%i` and `%n` may each appear at most three times. Generated code uses the selected listing radix too.

`DEFINED(expression)` returns true when all symbols in the expression have been defined before this point. It also accepts a single symbol name.

Binary operand evaluation order is unspecified, except for logical AND and OR: they evaluate the left operand first and skip the right operand when the result is already determined.

- `-radix <2...36>` selects the initial base for unmarked integer constants; `RADIX` can override it within the source.
- `-intsyntax <formats>` adjusts the accepted integer syntaxes from the start of assembly, using the same arguments as `INTSYNTAX`.

### File size queries

`FSIZE("filename")` returns a file's size in bytes. It searches the current working directory and the directory containing the source being assembled. The include search path is not used.

### Symbol use queries

`SYMUSED(symbol)` reports whether a symbol has been referenced up to this point. `IFUSED` and `IFNUSED` query the same information. Forward references count when both reference and definition are global, when they are in the same macro expansion, or when a reference in a section and a definition in that section, a parent section, or global scope are both outside macro expansions.

For uses outside these cases, a `FORWARD` declaration creates a placeholder that can store the usage flag before the symbol's value is defined.

### Chained forward references

`-maxsympass n` sets the last pass in which unresolved forward references are allowed; the default is 1. A larger value may be needed for chained references. `-maxinclevel n` sets the include nesting limit, whose default is 200.

The CMake option `AS_USE_INT128` enables 128-bit expressions when the compiler supports them. It defaults to `OFF` to preserve existing 64-bit expression semantics, including comparisons between signed phase addresses and unsigned hexadecimal constants. Sources can query `INTWIDTH` to adapt to either mode. Listing addresses retain their 64-bit representation.

`-listing ON|OFF|NOSKIPPED|PURECODE` sets the initial listing mode using the same values as the `LISTING` statement. NOSKIPPED omits excluded source; PURECODE also omits conditional control lines.

`-extracomments` enables the optional Motorola asterisk and end-of-line comment syntax from the start of assembly.

`-fred` retains the bit length of binary, octal, and hexadecimal integer constants, inferred from their written digits and capped at `INTWIDTH`. In this mode, the one’s-complement operator applies to that reduced width. Without it, expressions retain the full internal integer width reported by `INTWIDTH`; `FLOATMAX` reports the largest host floating-point value.
