# Utility Programs

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->

To simplify the work with the assembler's code format a bit, I added some tools to aid processing of code files. These programs are released under the same license terms as stated in section
[License Agreement](introduction.md#license-agreement)!

Common to all programs are the possible return codes they may deliver upon completion:

| return code | error condition                  |
| :---------: | :------------------------------- |
|      0      | no errors                        |
|      1      | error in command line parameters |
|      2      | I/O error                        |
|      3      | file format error                |

Just like AS, all programs take their input from STDIN and write messages to STDOUT (resp. error messages to STDERR). Therefore, input and output redirections should not be a problem.

In case that numeric or address specifications have to be given in the command line, they may also be written in hexadecimal notation, either by all ending in `h`, or by prepending a dollar character or a `0x` like in C. (e.g. `10h`, `$10`, or `0x10` instead of 16).

Unix shells however assign a special meaning to the dollar sign, which makes it necessary to escape a dollar sign with a backslash. The `0x` variant is definitely more comfortable in this case.

Otherwise, calling conventions and variations are equivalent to those of AS (except for PLIST and AS2MSG); i.e. it is possible to store frequently used parameters in an environment variable (whose name is constructed by appending CMD to the program's name, i.e. `BINDCMD` for BIND), to negate options, and to use all upper- resp. lower-case writing (for details on this, see [Start-Up Command, Parameters](assembler-usage.md#start-up-command-parameters)).

Address specifications always relate to the granularity of the processor currently in question; for example, on a PIC, an address difference of 1 means a word and not a byte.

## PLIST

PLIST is the simplest one of the five programs supplied: its purpose is simply to list all records that are stored in a code file. As the program does not do very much, calling is quite simple:

```dosbat
    PLIST <file name>
```

The file name will automatically be extended with the extension `P` if it doesn't already have one.

**CAUTION!** At this place, no wildcards are allowed! If there is a necessity to list several files with one command, use the following "mini batch":

```dosbat
    for %n in (*.p) do plist %n
```

PLIST prints the code file's contents in a table style, whereby exactly one line will be printed per record. The individual rows have the following meanings:

- code type: the processor family the code has been generated for.
- start address: absolute memory address that expresses the load destination for the code.
- length: length of this code chunk in bytes.
- end address: last address of this code chunk. This address is calculated as start address+length-1.

All outputs are in hexadecimal notation.

Finally, PLIST will print a copyright remark (if there is one in the file), together with a summarized code length.

Simply said, PLIST is a sort of DIR for code files. One can use it to examine a file's contents before one continues to process it.

## BIND

BIND is a program that allows to concatenate the records of several code files into a single file. A filter function is available that can be used to copy only records of certain types. Used in this way, BIND can also be used to split a code file into several files.

The general syntax of BIND is

```dosbat
    BIND <source file(s)> <target file> [options]
```

Just like AS, BIND regards all command line arguments that do not start with a `+`, `-` or `/` as file specifications, of which the last one must designate the destination file. All other file specifications name sources, which may again contain wildcards.

Currently, BIND defines only one command line option:

- `f <Header[,Header]>`: sets a list of record headers that should be copied. Records with other header IDs will not be copied. Without such an option, all records will be copied. The headers given in the list correspond to the `HeaderID` field of the record structure described in [Code Files](file-formats.md#code-files). Individual headers in this list are separated with commas.

For example, to filter all MCS-51 code out of a code file, use BIND in the following way:

```dosbat
    BIND <source name> <target name> -f $31
```

If a file name misses an extension, the extension `P` will be added automatically.

## P2HEX

P2HEX is an extension of BIND. It has all command line options of BIND and uses the same conventions for file names. In contrary to BIND, the target file is written as a Hex file, i.e. as a sequence of lines which represent the code as ASCII hex numbers.

P2HEX knows nine different target formats, which can be selected via the command line parameter `F`:

- Motorola S-Records (`-F Moto`)
- MOS Hex (`-F MOS`)
- Intel Hex (Intellec-8, `-F Intel`)
- 16-Bit Intel Hex (MCS-86, `-F Intel16`)
- 32-Bit Intel Hex (`-F Intel32`)
- Tektronix Hex (`-F Tek`)
- Texas Instruments DSK (`\-F DSK`)
- Atmel AVR Generic (`-F Atmel`, see [^AVRObj])
- Lattice Mico8 prom_init (`-F Mico8`)
- C arrays, for inclusion into C(++) source files (`-F C`)

If no target format is explicitly specified, P2HEX will automatically choose one depending in the processor type: S-Records for Motorola CPUs, Hitachi, and TLCS-900, MOS for 65xx/MELPS, DSK for the 16 bit signal processors from Texas, Atmel Generic for the AVRs, and Intel Hex for the rest. Depending on the start addresses width, the S-Record format will use Records of type 1, 2, or 3, however, records in one group will always be of the same type. This automatism can be partially suppressed via the command line option

```dosbat
    -M <1|2|3>
```

A value of 2 resp. 3 assures that that S records with a minimum type of 2 resp. 3 will be used, while a value of 1 corresponds to the full automatism.

Normally, the AVR format always uses an address length of 3 bytes. Some programs however do not like that...which is why there is a switch

```dosbat
    -avrlen <2|3>
```

that allows to reduce the address length to two bytes in case of emergency.

The Mico8 format is different from all the other formats in having no address fields - it is plain list of all instruction words in program memory. When using it, be sure that the used address range (as displayed e.g. by PLIS) starts at zero and is continuous.

The Intel, MOS and Tektronix formats are limited to 16 bit addresses, the 16-bit Intel format reaches 4 bits further. Addresses that are to long for a given format will be reported by P2HEX with a warning; afterwards, they will be truncated (!).

For the PIC microcontrollers, the switch

```dosbat
    -m <0...3>
```

allows to generate the three different variants of the Intel Hex format. Format 0 is INHX8M which contains all bytes in a Lo-Hi-Order. Addresses become double as large because the PICs have a word-oriented address space that increments addresses only by one per word. This format is also the default. With Format 1 (INHX16M), bytes are stored in their natural order. This is the format Microchip uses for its own programming devices. Format 2 (INHX8L) resp. 3 (INHX8H) split words into their lower resp. upper bytes. With these formats, P2HEX has to be called twice to get the complete information, like in the following example:

```dosbat
    p2hex test -m 2
    rename test.hex test.obl
    p2hex test -m 3
    rename test.hex test.obh
```

For the Motorola format, P2HEX additionally uses the S5 record type mentioned in [^CPM68k]. This record contains the number of data records (S1/S2/S3) to follow. As some programs might not know how to deal with this record, one can suppress it with the option

```dosbat
    +5
```

The C format is different in the sense that it always has to be selected explicitly. The output file is basically a complete piece of C or C++ code that contains the data as a list of C arrays. Additionally to the data itself, a list of descriptors is written that describes the start, length, and end address of each data block. The contents of these descriptors may be configured via the option

```dosbat
    -cformat <format>
```

Each letter in `format` defines an element of the descriptor:

- A `d` or `D` defines a pointer to the data itself. Usage of a lower or upper case letter defines whether lowercase or uppercase letters are used for hexadecimal constants.
- An `s` or `S` defines the start address of the data, either as _unsigned_ or _unsigned long_.
- An `l` or `L` defines the length of the data, either as _unsigned_ or _unsigned long_.
- An `e` or `E` defines the end address of the data, specifically the last address used by the data, either as _unsigned_ or _unsigned long_.

In case a source file contains code record for different processors, the different hex formats will also show up in the target file - it is therefore strongly advisable to use the filter function.

Apart form this filter function, P2HEX also supports an address filter, which is useful to split the code into several parts (e.g. for a set of EPROMs):

```dosbat
    -r <start address>-<end address>
```

The start address is the first address in the window, and the end address is the last address in the window, **not** the first address that is out of the window. For example, to split an 8051 program into 4 2764 EPROMs, use the following commands:

```dosbat
    p2hex <source file> eprom1 -f $31 -r $0000-$1fff
    p2hex <source file> eprom2 -f $31 -r $2000-$3fff
    p2hex <source file> eprom3 -f $31 -r $4000-$5fff
    p2hex <source file> eprom4 -f $31 -r $6000-$7fff
```

It is allowed to specify a single dollar character or '0x' as start or stop address. This means that the lowest resp. highest address found in the source file shall be taken as start resp. stop address. The default range is '0x-0x', i.e. all data from the source file is transferred.

**CAUTION!** This type of splitting does not change the absolute addresses that will be written into the files! If the addresses in the individual hex files should rather start at 0, one can force this with the additional switch

```dosbat
    -a
```

On the other hand, to move the addresses to a different location, one may use the switch

```dosbat
    -R <value>
```

The value given is an _offset_, i.e. it is added to the addresses given in the code file.

By using an offset, it is possible to move a file's contents to an arbitrary position. This offset is simply appended to a file's name, surrounded with parentheses. For example, if the code in a file starts at address 0 and you want to move it to address 1000 hex in the hex file, append `($1000)` to the file's name (without spaces!).

In case the source file(s) not only contain data for the code segment, the switch

```dosbat
    -segment <name>
```

allows to select the segment data is extracted from and converted to HEX format. The segment names are the same as for the `SEGMENT` pseudo instruction ([SEGMENT](pseudo-instructions.md#segment)). The TI DSK is a special case since it has the ability to distinguish between data and code in one file. If TI DSK is the output format, P2HEX will automatically extract data from both segments if no segment was specified explicitly.
Similar to the `-r` option, the argument

```dosbat
    -d <start>-<end>
```

allows to designate the address range that should be written as data instead of code.

The option

```dosbat
    -e <address>
```

is valid for the DSK, Intel, and Motorola formats. Its purpose is to set the entry address that will be inserted into the hex file. If such a command line parameter is missing, P2HEX will search a corresponding entry in the code file. If even this fails, no entry address will be written to the hex file (DSK/Intel) or the field reserved for the entry address will be set to 0 (Motorola).

Unfortunately, one finds different statements about the last line of an Intel-Hex file in literature. Therefore, P2HEX knows three different variants that may be selected via the command-line parameter `i` and an additional number:

```asm
    0  :00000001FF
    1  :00000001
    2  :0000000000
```

By default, variant 0 is used which seems to be the most common one.

If the target file name does not have an extension, an extension of `HEX` is supposed.

By default, P2HEX will print a maximum of 16 data bytes per line, just as most other tools that output Hex files. If you want to change this, you may use the switch

```asm
    -l <count>
```

The allowed range of values goes from 2 to 254 data bytes; odd values will implicitly be rounded down to an even count.

In most cases, the temporary code files generated by AS are not of any further need after P2HEX has been run. The command line option

```dosbat
    -k
```

allows to instruct P2HEX to erase them automatically after conversion.

In contrast to BIND, P2HEX will not produce an empty target file if only one file name (i.e. the target name) has been given. Instead, P2HEX will use the corresponding code file. Therefore, a minimal call in the style of

```dosbat
    P2HEX <name>
```

is possible, to generate `<name>.hex` out of `<name>.p`.

## P2BIN

P2BIN works similar to P2HEX and offers the same options (except for the a and i options that do not make sense for binary files), however, the result is stored as a simple binary file instead of a hex file. Such a file is for example suitable for programming an EPROM.

P2BIN knows three additional options to influence the resulting binary file:

- `l <8 bit number>`: sets the value that should be used to fill unused memory areas. By default, the value $ff is used. This value assures that every half-way intelligent EPROM burner will skip these areas. This option allows to set different values, for example if you want to generate an image for the EPROM versions of MCS-48 microcontrollers (empty cells of their EPROM array contain zeroes, so $00 would be the correct value in this case).
- `s`: commands the program to calculate a checksum of the binary file. This sum is printed as a 32-bit value, and the two's complement of the least significant bit will be stored in the file's last byte. This way, the modulus- 256-sum of the file will become zero.
- `m`: is designed for the case that a CPU with a 16- or 32-bit data bus is used and the file has to be split for several EPROMs. The argument may have the following values:

  - `ALL`: copy everything
  - `ODD`: copy all bytes with an odd address
  - `EVEN`: copy all bytes with an even address
  - `BYTE0...BYTE3`: copy only bytes with an address of `4n+0 ... 4n+3`
  - `WORD0, WORD1`: copy only the lower resp. upper 16- bit word of a 32-bit word

To avoid confusions: If you use this option, the resulting binary file will become smaller because only a part of the source will be copied. Therefore, the resulting file will be smaller by a factor of 2 or 4 compared to `ALL`. This is just natural...

In case the code file does not contain an entry address, one may set it via the `-e` command line option just like with P2HEX. Upon request, P2BIN prepends the resulting image with this address. The command line option

```dosbat
    -S
```

activates this function. It expects a numeric specification ranging from 1 to 4 as parameter which specifies the length of the address field in bytes. This number may optionally be prepended wit a `L` or `B` letter to set the endian order of the address. For example, the specification `B4` generates a 4 byte address in big endian order, while a specification of `L2` or simply `2` creates a 2 byte address in little endian order.

## AS2MSG

AS2MSG is not a tool in the real sense, it is a filter that was designed to simplify the work with the assembler for (fortunate) users of Borland Pascal 7.0. The DOS IDEs feature a 'tools' menu that can be extended with own programs like AS. The filter allows to directly display the error messages paired with a line specification delivered by AS in the editor window. A new entry has to be added to the tools menu to achieve this (Options/Tools/New). Enter the following values:

```asm
    - Title: ~m~acro assembler
    - Program path: AS
    - Command line:
        -E !1 $EDNAME $CAP MSG(AS2MSG) $NOSWAP $SAVE ALL
    - assign a hotkey if wanted (e.g. Shift-F7)
```

The -E option assures that Turbo Pascal will not become puzzled by STDIN and STDERR.

I assume that AS and AS2MSG are located in a directory listed in the `PATH` variable. After pressing the appropriate hotkey (or selecting AS from the tools menu), as will be called with the name of the file loaded in the active editor window as parameter. The error messages generated during assembly are redirected to a special window that allows to browse through the errors. `Ctrl-Enter` jumps to an erroneous line. The window additionally contains the statistics AS prints at the end of an assembly. These lines obtain the dummy line number 1.

`TURBO.EXE` (Real Mode) and `BP.EXE` (Protected Mode) may be used for this way of working with AS. I recommend however BP, as this version does not have to 'swap' half of the DOS memory before before AS is called.

<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
