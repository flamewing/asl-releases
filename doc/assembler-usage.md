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

Principally, you can obtain AS in one of two forms: as a _binary_ or a _source_ distribution. In case of a binary distribution, one gets AS, the accompanying tools and auxiliary files readily compiled, so you can immediately start to use it after unpacking the archive to the desired destination on your hard drive. Binary distributions are made for widespread platforms. A source distribution in contrast contains the complete set of C sources to generate AS; it is ultimately a snapshot of the source tree I use for development on AS. The generation of AS from the sources and their structure is described in detail in [Hints for the AS Source Code](modifying-as.md), which is why at this place, only the contents and installation of a binary distribution will be described:

The contents of the archive is separated into several subdirectories, therefore you get a directory subtree immediately after unpacking without having to sort out things manually. The individual directories contain the following groups of files:

- `BIN`: executable programs, text resources;

- `INCLUDE`: include files for assembler programs, e.g. register definitions or standard macros;

- `MAN`: quick references for the individual programs in Unix 'man' format.

A list of the files found in every binary distribution is given in the following table. In case a file listed in one of these (or the following) tables is missing, someone took a nap during copying (probably me)...

##### **Table:** Standard Contents of a Binary Distribution

| File                  | Function                                                                                 |
| :-------------------- | :--------------------------------------------------------------------------------------- |
| **Directory BIN**     | <!-- empty -->                                                                           |
| `AS.EXE`              | executable of assembler                                                                  |
| `PLIST.EXE`           | lists contents of code files                                                             |
| `PBIND.EXE`           | merges code files                                                                        |
| `P2HEX.EXE`           | converts code files to hex files                                                         |
| `P2BIN.EXE`           | converts code files to binary files                                                      |
| `AS.MSG`              | text resources for AS                                                                    |
| `PLIST.MSG`           | text resources for PLIST                                                                 |
| `PBIND.MSG`           | text resources for PBIND                                                                 |
| `P2HEX.MSG`           | text resources for P2HEX                                                                 |
| `P2BIN.MSG`           | text resources for P2BIN                                                                 |
| `TOOLS.MSG`           | common text resources for all tools                                                      |
| `CMDARG.MSG`          | common text resources for all programs                                                   |
| `IOERRS.MSG`          | common text resources for all programs                                                   |
| **Directory DOC**     | <!-- empty -->                                                                           |
| **Directory INCLUDE** | <!-- empty -->                                                                           |
| `BITFUNCS.INC`        | functions for bit manipulation                                                           |
| `CTYPE.INC`           | functions for classification of characters                                               |
| `80C50X.INC`          | register addresses SAB C50x                                                              |
| `80C552.INC`          | register addresses 80C552                                                                |
| `H8_3048.INC`         | register addresses H8/3048                                                               |
| `KENBAK.INC`          | register addressed Kenbak-1                                                              |
| `REG166.INC`          | addresses and instruction macros 80C166/167                                              |
| `REG251.INC`          | addresses and bits 80C251                                                                |
| `REG29K.INC`          | peripheral addresses AMD 2924x                                                           |
| `REG53X.INC`          | register addresses H8/53x                                                                |
| `REG6303.INC`         | register addresses 6303                                                                  |
| `REG683XX.INC`        | register addresses 68332/68340/68360                                                     |
| `REG7000.INC`         | register addresses TMS70Cxx                                                              |
| `REG78310.INC`        | register addresses & vectors 78K3                                                        |
| `REG78K0.INC`         | register addresses 78K0                                                                  |
| `REG96.INC`           | register addresses MCS-96                                                                |
| `REGACE.INC`          | register addresses ACE                                                                   |
| `REGF8.INC`           | register and memory addresses F8                                                         |
| `REGAVROLD.INC`       | register and bit addresses AVR family (old)                                              |
| `REGAVR.INC`          | register and bit addresses AVR family                                                    |
| `REGCOLD.INC`         | register and bit addresses Coldfire family                                               |
| `REGCOP8.INC`         | register addresses COP8                                                                  |
| `REGGP32.INC`         | register addresses 68HC908GP32                                                           |
| `REGH16.INC`          | register addresses H16                                                                   |
| `REGHC12.INC`         | register addresses 68HC12                                                                |
| `REGM16C.INC`         | register addresses Mitsubishi M16C                                                       |
| `REGMSP.INC`          | register addresses TI MSP430                                                             |
| `REGPDK.INC`          | register and bit addresses PMC/PMS/PFSxxx                                                |
| `REGS12Z.INC`         | register and bit addresses S12Z family                                                   |
| `REGST6.INC`          | register and macro definitions ST6                                                       |
| `REGST7.INC`          | register and macro definitions ST7                                                       |
| `REGSTM8.INC`         | register and macro definitions STM8                                                      |
| `REGST9.INC`          | register and macro definitions ST9                                                       |
| `REGZ380.INC`         | register addresses Z380                                                                  |
| `STDDEF04.INC`        | register addresses 6804                                                                  |
| `STDDEF16.INC`        | instruction macros and register addresses PIC16C5x                                       |
| `STDDEF17.INC`        | register addresses PIC17C4x                                                              |
| `STDDEF18.INC`        | register addresses PIC16C8x                                                              |
| `STDDEF2X.INC`        | register addresses TMS3202x                                                              |
| `STDDEF37.INC`        | register and bit addresses TMS370xxx                                                     |
| `STDDEF3X.INC`        | peripheral addresses TMS320C3x                                                           |
| `STDDEF4X.INC`        | peripheral addresses TMS320C4x                                                           |
| `STDDEF47.INC`        | instruction macros TLCS-47                                                               |
| `STDDEF51.INC`        | definition of SFRs and bits for 8051/8052/80515                                          |
| `STDDEF56K.INC`       | register addresses DSP56000                                                              |
| `STDDEF5X.INC`        | peripheral addresses TMS320C5x                                                           |
| `STDDEF60.INC`        | instruction macros and register addresses PowerPC                                        |
| `REGSX20.INC`         | register and bit addresses Parallax SX20/28                                              |
| `AVR/*.INC`           | register and bit addresses AVR family (do not include directly, use `REGAVR.INC` )       |
| `COLDFIRE/*.INC`      | register and bit addresses ColdFire family (do not include directly, use `REGCOLD.INC` ) |
| `PDK/*.INC`           | register and bit addresses PMC/PMS/PFSxxx (do not include directly, use `REGPDK.INC` )   |
| `S12Z/*.INC`          | register and bit addresses S12Z family (do not include directly, use `REGS12Z.INC` )     |
| `ST6/*.INC`           | register and bit addresses ST6 family (do not include directly, use `REGST6.INC` )       |
| `ST7/*.INC`           | register and bit addresses ST7 family (do not include directly, use `REGST7.INC` )       |
| `STM8/*.INC`          | register and bit addresses STM8 family (do not include directly, use `REGSTM8.INC` )     |
| `STDDEF62.INC`        | register addresses and macros ST6 (old)                                                  |
| `STDDEF75.INC`        | register addresses 75K0                                                                  |
| `STDDEF87.INC`        | register and memory addresses TLCS-870                                                   |
| `STDDEF90.INC`        | register and memory addresses TLCS-90                                                    |
| `STDDEF96.INC`        | register and memory addresses TLCS-900                                                   |
| `STDDEFXA.INC`        | SFR and bit addresses Philips XA                                                         |
| `STDDEFZ8.INC`        | register addresses Z8 family (old)                                                       |
| `REGZ8.INC`           | register addresses Z8 family (new)                                                       |
| `Z8/*.INC`            | register and bit addresses Z8 family (do not include directly, use `REGZ8.INC` )         |
| **Directory LIB**     | <!-- empty -->                                                                           |
| **Directory MAN**     | <!-- empty -->                                                                           |
| `ASL.1`               | Short Reference for `AS`                                                                 |
| `PLIST.1`             | Short Reference for `PLIST`                                                              |
| `PBIND.1`             | Short Reference for `PBIND`                                                              |
| `P2HEX.1`             | Short Reference for `P2HEX`                                                              |
| `P2BIN.1`             | Short Reference for `P2BIN`                                                              |

## Start-Up Command, Parameters

AS is a command line driven program, i.e. all parameters and file options are to be given in the command line.

A couple of message files belongs to AS (recognizable by their suffix `MSG` ) AS accesses to dynamically load the messages appropriate for the national language. AS searches the following directories for these files:

- the current directory;

- the EXE-file's directory;

- the directory named in the `AS_MSGPATH` environment variable, or alternatively the directories listed in the `PATH` environment variable;

- the directory compiled into AS via the `LIBDIR` macro.

These files are _indispensable_ for a proper operation of AS, i.e. AS will terminate immediately if these files are not found.

The language selection (currently only German and English) is based on the `COUNTRY` setting under DOS and OS/2 respectively on the `LANG` environment variable under Unix.

The command line parameters can roughly be divided into three categories: switches, key file references (see below) and file specifications. Parameters of these two categories may be arbitrarily mixed in the command line. The assembler evaluates at first all parameters and then assembles the specified files. From this follow two things:

- the specified switches affect all specified source files. If several source files shall be assembled with different switches, this has to be done in separate runs.

- it is possible to assemble more than one file in one shot and to bring it to the top, it is allowed that the file specs contain wildcards.

Parameter switches are recognized by AS by starting with a slash (/) or hyphen (-). There are switches that are only one character long, and additionally switches composed of a whole word. Whenever AS cannot interpret a switch as a whole word, it tries to interpret every letter as an individual switch. For example, if you write

```sh
-queit
```

instead of

```sh
-quiet
```

AS will take the letters `q, u, e, i` , and `t` as individual switches. Multiple-letter switches additionally have the difference to single-letter switches that AS will accept an arbitrary mixture of upper and lower casing, whereas single-letter switches may have a different meaning depending on whether upper or lower case is used.

At the moment, the following switches are defined:

- `l`: sends assembler listing to console terminal (mostly screen). In case several passes have to be done, the listing of all passes will be send to the console (in opposite to the next option).

- `L`: writes assembler listing into a file. The list file will get the same name as the source file, only the extension is replaced by `LST`. Except one uses...

- `OLIST`: with a fiel name as argument allows to redirect the listing to a different file or a different path. This option may be used multiple times in case multiple files are assembled with one execution.

- `LISTRADIX`: By default, all numeric output in the listing (addresses, generated code, symbol values) is written in hexadecimal notation. This switch requests usage of a different number system in the range of 2 to 36\. For instance, '-listradix 8' requests octal output.

- `SPLITBYTE [character]`: Display numbers in the listing in byte groups, separated by the given character. A period is used as separator if no explicit character is given. This option is usually used in conjunction with the `LISTRADIX` option. For instance, list radix 8 with a period as character results in the so-called 'split octal' notation.

- `o`: Sets the new name of the code file generated by AS. If this option is used multiple times, the names will be assigned, one after the other, to the source files which have to be assembled. A negation (see below) of this option in connection with a name erases this name from the list. A negation without a name erases the whole list.

- `SHAREOUT`:ditto for a SHARE file eventually to be created.

- `c`: SHARED-variables will be written in a format which permits an easy integration into a C-source file. The extension of the file is `H`.

- `p`: SHARED-variables will be written in a format which permits easy integration into the CONST-block of a Pascal program. The extension of the file is `INC`.

- `a`: SHARED-variables will be written in a format which permits easy integration into an assembler source file. The extension of the file is `INC`.

Concerning effect and function of the SHARED-symbols please see [Share File](#share-file) resp. [SHARED](pseudo-instructions.md#shared).

- `g [format]`: This switch instructs AS to create an additional file that contains debug information for the program. Allowed formats are the AS-specific MAP format (`format=MAP`), a NoICE-compatible command file (`format=NOICE`), and the Atmel format used by the AVR tools (`format=ATMEL`). The information stored in the MAP format is comprised of a symbol table and a table describing the assignment of source lines to machine addresses. A more detailed description of the MAP format can be found in [Debug Files](file-formats.md#debug-files) The file's extension is `MAP`, `NOI`, resp. `OBJ`, depending on the chosen format. If no explicit format specification is done, the MAP format is chosen.

- `noicemask [value]`: By default, AS lists only symbols from the CODE segment in NoICE debug info files. With this option and an integer value interpreted as a bit mask, symbols fom other segments may be added. The assignment of segments to bit positions may be taken from [Codings of the `Segment` Field table](file-formats.md#table-codings-of-the-segment-field).

- `w`: suppress issue of warnings;

- `E [file]`: error messages and warnings produced by AS will be redirected to a file. Instead of a file, the 5 standard handles (STDIN...STDPRN) can also be specified as `!0, !1, !2, !3, or !4` . Default is `STDERR == !2`. If the file option is left out, the name of the error file is the same as of the source file, but with the extension `LOG`.

- `q`: This switch suppresses all messages of AS, the exceptions are error messages and outputs which are are forced from the source file. The time needed for assembly is slightly reduced hereby and if you call AS from a shell there is no redirection required. The disadvantage is that you may "stay in the dark" for several minutes ... It is valid to write `quiet` instead of `q`.

- `h`: write hexadecimal numbers in lowercase instead of capital letters. This option is primarily a question of personal taste.

- `i <path list>`: issues a list of directories where the assembler shall automatically search for include files, in case it didn't find a file in the current directory. The different directories have to be separated by semicolons.

- `u`: calculate a list of areas which are occupied in the segments. This option is effective only in case a listing is produced. This option requires considerable additional memory and computing performance. In normal operation it should be switched off.

- `C`: generates a list of cross references. It lists which (global) symbols are used in files and lines. This list will also be generated only in case a listing is produced. This option occupies, too, additional memory capacity during assembly.

- `s`: issues a list of all sections (see [Local Symbols](pseudo-instructions.md#local-symbols)). The nesting is indicated by indentations (Pascal like).

- `t`: by means of this switch it is possible to separate single components of the standard issued assembler-listing. The assignment of bits to parts can be found in the next section, where the exact format of the assembly listing is explained.

- `D`: defines symbols. The symbols which are specified behind this option and separated by commas are written to the global symbol table before starting the assembly. As default these symbols are written as integer numbers with the value TRUE, by means of an appended equal sign, however, you can select other values. The expression following the equals sign may include operators or internal functions, but **not** any further symbols, even if these should have been defined before in the list! Together with the commands for conditional assembly (see there) you may produce different program versions out of one source file by command line inputs. **CAUTION!** If the case-sensitive mode is used, this has to be specified in the command line _before_ any symbol definitions, otherwise symbol names will be converted to upper case at this place!

- `A`: stores the list of global symbols in another, more compact form. Use this option if the assembler crashes with a stack overflow because of too long symbol tables. Sometimes this option can increase the processing speed of the assembler, but this depends on the sources.

- `x`: Sets the level of detail for error messages. The level is increased resp. decreased by one each time this option is given. While on level 0 (default) only the error message itself is printed, an extended message is added beginning at level 1 that should simplify the identification of the error's cause. [Error messages](error-messages.md) lists which error messages carry which extended messages. At level 2 (maximum), the source line containing the error is additionally printed.

- `n`: If this option is set, the error messages will be issued additionally with their error number (see [Error messages](error-messages.md)). This is primarily intended for use with shells or IDE's to make the identification of errors easier by those numbers.

- `U`: This option switches AS to the case-sensitive mode, i.e. upper and lower case in the names of symbols, sections, macros, character sets, and user-defined functions will be distinguished. This is not the case by default.

- `P`: Instructs AS to write the source text processed by macro processor and conditional assembly into a file. Additional blank and pure comment lines are missing in this file. The extension of this file is `I`.

- `M`: If this switch is given, AS generates a file, that contains definitions of macros defined in the source file that did not use the `NOEXPORT` option. This new file has the same name as the source file, only the extension is modified into `MAC`.

- `G`: this switch defines whether AS should produce code or not. If switched off, the processing will be stopped after the macro processor. This switch is activated by default (logically, otherwise you would not get a code file). This switch can be used in conjunction with the `P` switch, if only the macro processor of AS shall be used.

- `r [n]`: issue warnings if situations occur that force a further pass. This information can be used to reduce the number of passes. You may optionally specify the number of the first pass where issuing of such messages shall start. Without this argument, warnings will come starting with the first pass. Be prepared for a bunch of messages!!

- `relaxed`: this switch enables the RELAXED mode right from the beginning of the program, which otherwise has to be enabled by the pseudo instruction of sane name (see [RELAXED](pseudo-instructions.md#relaxed)).

- `supmode`: this switch enables right from the beginning of the program usage of machine instructions that may only be used in the processor's supervisor mode (see [SUPMODE, FPU, PMMU, CUSTOM](pseudo-instructions.md#supmode-fpu-pmmu-custom)).

- `Y`: This switch instructs AS to to suppress all messages about out-of-branch conditions, once the necessity for another pass is given. See [Forward References and Other Disasters](#forward-references-and-other-disasters) for the (rare) situations that might make use of this switch necessary.

- `cpu <name>`: this switch allows to set the target processor AS shall generate code for, in case the source file does not contain a `CPU` instruction and is not 68008 code. If the selected target supports CPU arguments (see [CPU](pseudo-instructions.md#cpu)), they may be used on the command line as well.

- `alias <new>=<old>`: defines the processor type `<new>` to be an alias for the type `<old>`. See [Processor Aliases](#processor-aliases) for the sense of processor aliases.

- `gnuerrors`: display messages about errors resp. warnings not in the AS standard format, but instead in a format similar to the GNU C compiler. This simplifies the integration of AS into environments tuned for this format, however also suppresses the display of precise error positions in macro bodies!

- `maxerrors [n]`: instructs the assembler to terminate assembly after the given number of errors.

- `maxinclevel [n]`: instructs the assembler to terminate assembly if the include nesting level exceeds the given limit (default is 200).

- `Werror`: instructs the assembler to treat warnings as errors.

- `compmode`: This switch instructs the assembler to operate by default in compatibility mode. See [COMPMODE](pseudo-instructions.md#compmode) for more information about this mode.

As long as switches require no arguments and their concatenation does not result in a multi-letter switch, it is possible to specify several switches at one time, as in the following example :

```dosbat
as test*.asm firstprog -cl /i c:\as\8051\include
```

All files `TEST*.ASM` as well as the file `FIRSTPROG.ASM` will be assembled, whereby listings of all files are displayed on the console terminal. Additional sharefiles will be generated in the C- format. The assembler should search for additional include files in the directory `C:\AS\8051\INCLUDE` .

This example shows that the assembler assumes `ASM` as the default extension for source files.

A bit of caution should be applied when using switches that have optional arguments: if a file specification immediately follows such a switch without the optional argument, AS will try to interpret the file specification as argument - what of course fails:

```dosbat
as -g test.asm
```

The solution in this case would either be to move the -g option the end or to specify an explicit MAP argument.

Beside from specifying options in the command line, permanently needed options may be placed in the environment variable `ASCMD` . For example, if someone always wants to have assembly listings and has a fixed directory for include files, he can save a lot of typing with the following command:

```dosbat
set ascmd=-L -i c:\as\8051\include
```

The environment options are processed before the command line, so options in the command line can override contradicting ones in the environment variable.

In the case of very long path names, space in the `ASCMD` variable may become a problem. For such cases a key file may be the alternative, in which the options can be written in the same way as in the command line or the `ASCMD` -variable. But this file may contain several lines each with a maximum length of 255 characters. In a key file it is important, that for options which require an argument, switches and argument have to be written in the **same** line. AS gets informed of the name of the key file by a `@` ahead in the `ASCMD` variable, e.g.

```dosbat
set ASCMD=@c:\as\as.key
```

In order to neutralize options in the `ASCMD` variable (or in the key file), prefix the option with a plus sign. For example, if you do not want to generate an assembly listing in an individual case, the option can be retracted in this way:

```dosbat
as +L <file>
```

Naturally it is not consequently logical to deny an option by a plus sign.... UNIX soit qui mal y pense.

References to key files may not only come from the `ASCMD` variable, but also directly from the command line. Similarly to the `ASCMD` variable, prepend the file's name with a character:

```dosbat
as @<file> ....
```

The options read from a key file in this situation are processed as if they had been written out in the command line in place of the reference, _not_ like the key file referenced by the `ASCMD` variable that is processed prior to the command line options.

Referencing a key file from a key file itself is not allowed and will be answered wit an error message by AS.

In case that you like to start AS from another program or a shell and this shell hands over only lower-case or capital letters in the command line, the following workaround exists: if a tilde ( `~` ) is put in front of an option letter, the following letter is always interpreted as a lower-case letter. Similarly a `#` demands the interpretation as a capital letter. For example, the following transformations result for:

```dosbat
/~I ---> /i
-#u ---> -U
```

In dependence of the assembly's outcome, the assembler ends with the following return codes:

0 error free run, at maximum warnings occurred

1 The assembler displayed only its command-line parameters and terminated immediately afterwards.

2 Errors occurred during assembly, no code file has been produced.

3 A fatal error occurred what led to immediate termination of the run.

4 An error occurred already while starting the assembler. This may be a parameter error or a faulty overlay file.

255 An internal error occurred during initialization that should not occur in any case...reboot, try again, and contact me if the problem is reproducible!

Similar to UNIX, OS/2 extends an application's data segment on demand when the application really needs the memory. Therefore, an output like

```sh
511 KByte available memory
```

does not indicate a shortly to come system crash due to memory lack, it simply shows the distance to the limit when OS/2 will push up the data segment's size again...

As there is no compatible way in C under different operating systems to find out the amount of available memory resp. stack, both lines are missing completely from the statistics the C version prints.

## Format of the Input Files

Like most assemblers, AS expects exactly one instruction per line (blank lines are naturally allowed as well). The lines must not be longer than 255 characters, additional characters are discarded.

A single line has following format:

```sh
[label[:]] <mnemonic>[.attr] [param[,param...]] [;comment]
```

A line may also be split over several lines in the source file, continuation characters chain these parts together to a single line. One must however consider that, due to the internal buffer structure, the total line must not be longer than 256 characters. Line references in error messages always relate to the last line of such a composed source line.

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

The number of instruction parameters depends on the mnemonic and is principally located between 0 and 20\. The separation of the parameters from each other is to be performed only by commas (exception: DSP56xxx, its parallel data transfers are separated with blanks). Commas that are included in brackets or quotes, of course, are not taken into consideration.

Instead of a comment at the end, the whole line can consist of comment if it starts in the first column with a semicolon.

To separate the individual components you may also use tabulators instead of spaces.

## Format of the Listing

The listing produced by AS using the command line options i or I is roughly divisible into the following parts :

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

The parts mentioned so far as well as the list of all macros/functions defined can be selectively masked out from the listing. This can be done by the already mentioned command line switch `-t` . There is an internal byte inside AS whose bits represent which parts are to be written. The assignment of bits to parts of the listing is listed in the following table:

<!-- markdownlint-enable MD038-->

| bit | part                           |
| :-- | :----------------------------- |
| 0   | source file(s) + produced code |
| 1   | symbol table                   |
| 2   | macro list                     |
| 3   | function list                  |
| 4   | line numbering                 |
| 5   | register symbol list           |
| 7   | character set table            |

All bits are set to 1 by default, when using the switch

```dosbat
-t <mask>
```

Bits set in `<mask>` are cleared, so that the respective listing parts are suppressed. Accordingly it is possible to switch on single parts again with a plus sign, in case you had switched off too much with the `ASCMD` variable... If someone wants to have, for example, only the symbol table, it is enough to write:

```dosbat
-t 2
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

## Symbol Conventions

Symbols are allowed to be up to 255 characters long (as hinted already in the introduction) and are being distinguished on the whole length, but the symbol names have to meet some conventions:

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

A hidden feature (that has to be used with care) is that symbol names may be assembled from the contents of string symbols. This can be achieved by framing the string symbol's name with braces and inserting it into the new symbol's name. This allows for example to define a symbol's name based on the value of another symbol:

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

The assembler understands a backslash ( `\` ) with a following decimal number of three digits maximum in the string as a character with the according decimal ASCII value. The numerical value may alternatively be written in hexadecimal or octal notation if it is prefixed with an x resp. a 0\. In case of hexadecimal notation, the maximum number of digits is limited to 2\. For example, it is possible to include an ETC character by writing `\3` . But be careful with the definition of NUL characters! The C version currently uses C strings to store strings internally. As C strings use a NUL character for termination, the usage of NUL characters in strings is currently not portable!

Some frequently used control characters can also be reached with the following abbreviations:

```asm
\b : Backspace           \a : Bell         \e : Escape
\t : Tabulator           \n : Linefeed     \r : Carriage Return
\\ : Backslash           \' or \H : Apostrophe
\" or \I : Quotation marks
```

Both upper and lower case characters may be used for the identification letters.

By means of this escape character, you can even work formula expressions into a string, if they are enclosed by braces: e.g.

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

The calculation of intermediary results within formula expressions is always done with the highest available resolution, i.e. 32 or 64 bits for integer numbers, 80 bit for floating point numbers and 255 characters for strings. An possible test of value range overflows is done only on the final result.

The portable C version only supports floating point values up to 64 bits (resulting in a maximum value of roughly 10^308), but in turn features integer lengths of 64 bits on some platforms.

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

\*) remainder will be discarded

<!-- markdownlint-enable MD038 -->

"Rank" is the priority of an operator at the separation of expressions into subexpressions. The operator with the highest rank will be evaluated at the very end. The order of evaluation can be defined by new bracketing.

The compare operators deliver TRUE in case the condition fits, and FALSE in case it doesn't. For the logical operators an expression is TRUE in case it is not 0, otherwise it is FALSE.

The mirroring of bits probably needs a little bit of explanation: the operator mirrors the lowest bits in the first operand and leaves the higher priority bits unchanged. The number of bits which is to be mirrored is given by the right operand and may be between 1 and 32 .

A small pitfall is hidden in the binary complement: As the computation is always done with 32 resp. 64 bits, its application on e.g. 8-bit masks usually results in values that do not fit into 8-bit numbers any more due to the leading ones. A binary AND with a fitting mask is therefore unavoidable!

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

_valid for: PowerPC, M-Core, XGate, 4004/4040, MCS-48/(2)51, 80C16x, AVR, XS1, Z8, KCPSM, Mico8, MSP430(X), ST9, M16, M16C, H8/300, H8/500, SH7x00, H16, i960, XA, 29K, TLCS-9000, KENBAK_

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

There were two reasons to realize the definition of aliases by the command line and not by pseudo instructions: first, it would anyway be difficult to put the alias definitions together with register
definitions into a single include file, because a program that wants to use such a file would have to include it before and after the CPU instruction - an imagination that lies somewhere between inelegant and impossible. Second, the definition in the command line allows to put the definitions in a key file that is executed automatically at startup via the `ASCMD` variable, without a need for the program to take any further care about this.

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->
