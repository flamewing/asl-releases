# Flamewing's Enhanced AS

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->

Welcome to the Flamewing's Enhanced AS documentation!

Most of this is based on the original documentation from AS, v1.42 Beta `[Bld 283]`.

# Macro Assembler AS V1.42

Alfred Arnold, Stefan Hilse, Stephan Kanthak, Oliver Sellke, Vittorio De Tomasi, flamewing

# User's Manual

Edition November 2024 Note: This is a modified version by flamewing.

_IBM, PPC403Gx, OS/2, and PowerPC_ are registered trademarks of IBM Corporation.
_Intel, MCS-48, MCS-51, MCS-251, MCS-96, MCS-196 and MCS-296_ are registered trademarks of Intel Corp.
_Motorola and ColdFire_ are registered trademarks of Motorola Inc.
_MagniV_ is a registered trademark of Freescale Semiconductor.
_PicoBlaze_ is a registered trademark of Xilinx Inc.
_UNIX_ is a registered trademark of the The Open Group.
_Linux_ is a registered trademark of Linus Torvalds.
_Microsoft, Windows, and MS-DOS_ are registered trademarks of Microsoft Corporation.
All other trademarks not explicitly mentioned in this section and used in this manual are properties of their respective owners.

Intended reading order is as follows:

1. [Introduction](introduction.md)
2. [Assembler Usage](assembler-usage.md)
3. [Pseudo instructions](pseudo-instructions.md)
4. [Processor specific Hints](processor-specific-hints.md)
5. [File Formats](file-formats.md)
6. [Utility Programs](utility-programs.md)
7. [Error Messages](error-messages.md)
8. [Pseudo Instructions and Integer Syntax](pseudo-instructions-and-integer-syntax.md)
9. [Predefined Symbols](predefined-symbols.md)
10. [Shipped Include Files](shipped-include-files.md)
11. [Programming Examples](programming-examples.md)
12. [Bibliography](bibliography.md)

For historical reasons, the following are preserved:

1. [Frequently Asked Questions (deprecated)](frequently-asked-questions.md)
2. [Modifying AS](modifying-as.md)

<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->

### Toshiba TLCS-42

Supported models are `4240P`, `4250N`, `4260P`, `4270N`, `42C00Y`,
`42C40P`, `42C50N`, `42C60P`, and `42C70N`. CMOS models add two
instructions; the models otherwise differ in I/O ports and program memory.
The default integer syntax is Intel. `ASSUME`, `PORT`, and the Intel
`DB`, `DW`, `DD`, `DQ`, `DT`, `DO`, `DN`, and `DS` directives are available.

### Symbol existence across passes

`SYMEXIST(symbol)` and `IFSYMEXIST symbol` query whether a symbol is in the
symbol table. A forward declaration is absent in the first pass and present
in subsequent passes. `IFNSYMEXIST` reverses this condition. In contrast,
`DEFINED(expression)` and `IFDEF expression` check definitions encountered
up to the current point in the source, including every symbol in an expression.
The fork also recognizes macro and function names in `IFDEF` and `IFNDEF`.

Warning 500 reports instruction generation outside the `CODE` segment when
that segment's word width is required.
