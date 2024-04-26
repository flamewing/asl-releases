# Pseudo Instructions

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

Not all pseudo instructions are defined for all processors. A note that shows the range of validity is therefore prepended to every individual description.

## Definitions

### SET, EQU, and CONSTANT

_valid for: all processors, `CONSTANT` only for KCPSM(3)_

`SET` and `EQU` allow the definition of typeless constants, i.e. they will not be assigned to a segment and their usage will not generate warnings because of segment mixing. `EQU` defines constants which can not be modified (by `EQU`) again, but `SET` permits the definition of variables, which can be modified during the assembly. This is useful e.g. for the allocation of resources like interrupt vectors, as shown in the following example:

```asm
VecCnt  set     0       ; somewhere at the beginning
        .
        .
        .
DefVec  macro   Name    ; allocate a new vector
Name    equ     VecCnt
VecCnt  set     VecCnt+4
        endm
        .
        .
        .
        DefVec  Vec1    ; results in Vec1=0
        DefVec  Vec2    ; results in Vec2=4
```

constants and variables are internally stored in the same way, the only difference is that they are marked as unchangeable if defined via `EQU`. Trying to change a constant with `SET` will result in an error message.

`EQU/SET` allow to define constants of all possible types, e.g.

```asm
        IntTwo   equ    2
        FloatTwo equ    2.0
```

Some processors unfortunately have already a `SET` instruction. For these targets, `EVAL` must be used instead of `SET` if no differentiation via the argument count is possible.

A simple equation sign may be used instead of `EQU`. Similarly, one may simply write `:=` instead of `SET` resp. `EVAL`. Furthermore, there is an 'alternate' syntax that does not take the symbol's name from the label field, but instead from the first argument. So for instance, it is valid to write:

```asm
        EQU   IntTwo,2
        EQU   FloatTwo,2.0
```

For compatibility reasons to the original assembler, the KCPSM target also knows the `CONSTANT` statement, which - in contrast to `EQU` - takes both name and value as argument. For example:

```asm
        CONSTANT  const1, 2
```

`CONSTANT` is however limited to integer constants.

Symbols defined with `SET` or `EQU` are typeless by default, but optionally a segment name (`CODE, DATA, IDATA, XDATA, YDATA, BITDATA, IO`, or `REG`) or `MOMSEGMENT` for the currently active segment may be given as a second or third parameter, allowing to assign the symbol to a specific address space. AS does not check at this point if the used address space exists on the currently active target processor!

A little hidden extra feature allows to set the program counter via `SET` or `EQU`, something one would ordinarily do via `ORG`. To accomplish this, use the special value as symbol name that may also be used to query the current program counter's value. Depending on the selected target architecture, this is either an asterisk, a dollar sign or `PC`.

In case the target architecture supports instruction attributes to define the operand size (e.g. on 680x0), those are also allowed for `SET` and `EQU`. The operand size will be stored along with the symbol's value in the symbol table. Its use is architecture-dependant.

### SFR and SFRB

_valid for: various, `SFRB` only MCS-51_

These instructions act like `EQU`, but symbols defined with them are assigned to the directly addressable data resp. I/O segment, i.e. they are preferably used for the definition of (as the name lets guess) hardware registers mapped into the data res. I/O area. The allowed range of values is equal to the range allowed for `ORG` in the data segment (see [ORG](#org)). The difference between `SFR` and `SFRB` is that `SFRB` marks the register as bit addressable, which is why AS generates 8 additional symbols which will be assigned to the bit segment and carry the names xx.0 to xx.7, e.g.

```asm
    PSW     sfr     0d0h    ; results in PSW = D0H (data segment)

    PSW     sfrb    0d0h    ; results in extra PSW.0 = D0H (bit)
                            ;               to PSW.7 = D7H (bit)
```

The `SFRB` instruction is not any more defined for the 80C251 as it allows direct bit access to all SFRs without special bit symbols; bits like `PSW.0` to `PSW.7` are automatically present.

Whenever a bit-addressable register is defined via `SFRB`, AS checks if the memory address is bit addressable (range 20h...3fh resp. 80h, 88h, 90h, 98h...0f8h). If it is not bit-addressable, a warning is issued and the generated bit symbols are undefined.

### XSFR and YSFR

_valid for: DSP56xxx_

Also the DSP56000 has a few peripheral registers memory-mapped to the RAM, but the affair becomes complicated because there are two data areas, the X- and Y-area. This architecture allows on the one hand a higher parallelism, but forces on the other hand to divide the normal `SFR` instruction into the two above mentioned variations. They works identically to `SFR`, just that `XSFR` defines a symbol in the X- addressing space and YSFR a corresponding one in the Y-addressing space. The allowed value range is 0...$ffff.

### LABEL

_valid for: all processors_

The function of the `LABEL` instruction is identical to `EQU`, but the symbol does not become typeless, it gets the attribute "code". `LABEL` is needed exactly for one purpose: Labels are normally local in macros, that means they are not accessible outside of a macro. With an `EQU` instruction you could get out of it nicely, but the phrasing

```asm
<name>  label   $
```

generates a symbol with correct attributes.

### BIT

_valid for: MCS/(2)51, XA, 80C166, 75K0, ST9, AVR, S12Z, SX20/28, H16, H8/300, H8/500, KENBAK, Padauk_

`BIT` serves to equate a single bit of a memory cell with a symbolic name. This instruction varies from target platform to target platform due to the different ways in which processors handle bit manipulation and addressing:

The MCS/51 family has an own address space for bit operands. The function of `BIT` is therefore quite similar to `SFR`, i.e. a simple integer symbol with the specified value is generated and assigned to the `BDATA` segment. For all other processors, bit addressing is done in a two-dimensional fashion with address and bit position. In these cases, AS packs both parts into an integer symbol in a way that depends on the currently active target processor and separates both parts again when the symbol is used. The latter is is also valid for the 80C251: While an instruction like

```asm
    My_Carry bit    PSW.7
```

would assign the value 0d7h to `My_Carry` on an 8051, a value of 070000d0h would be generated on an 80C251, i.e. the address is located in bits 0...7 and the bit position in bits 24...26. This procedure is equal to the way the `DBIT` instruction handles things on a TMS370 and is also used on the 80C166, with the only difference that bit positions may range from 0...15:

```asm
    MSB     BIT     r5.15
```

On a Philips XA, the bit's address is located in bits 0...9 just with the same coding as used in machine instructions, and the 64K bank of bits in RAM memory is placed in bits 16...23.

The `BIT` instruction of the 75K0 family even goes further: As bit expressions may not only use absolute base addresses, even expressions like

```asm
    bit1    BIT     @h+5.2
```

are allowed.

The ST9 in turn allows to invert bits, what is also allowed in the `BIT` instruction:

```asm
    invbit  BIT     r6.!3
```

More about the ST9's `BIT` instruction can be found in the processor specific hints.

In case of H16, note that the address and bit position arguments are swapped. This was done to make the syntax of BIT consistent with the machine instructions that manipulate individual bits.

### DBIT

_valid for: TMS 370xxx_

Though the TMS370 series does not have an explicit bit segment, single bit symbols may be simulated with this instruction. `DBIT` requires two operands, the address of the memory cell that contains the bit and the exact position of the bit in the byte. For example,

```asm
INT3        EQU  P019
INT3_ENABLE DBIT 0,INT3
```

defines the bit that enables interrupts via the INT3 pin. Bits defined this way may be used in the instructions `SBIT0, SBIT1, CMPBIT, JBIT0`, and `JBIT`.

### DEFBIT and DEFBITB

#### S12Z

The S12Z family's processor core provides instructions to manipulate individual bits in registers or memory cells. To conveniently address bits in the CPU's I/O area (first 4 Kbytes of the address space), a bit may be given a symbolic name. The bit is defined by its memory address and the bit position:

```asm
<name>         defbit[.size]   <address>,<position>
```

The `address` must be located within the first 4 Kbytes, and the operand size may be 8, 16, or 32 bits (`size`=b/w/l). Consequently, the `position` may at most be 7, 15 or 31. If no operand size is given, byte size (.b) is assumed. A bit defined this way may be used as argument for the instructions `BCLR, BSET, BTGL, BRSET,` and `BRCLR`:

```asm
mybit   defbit.b  $200,4
        bclr.b    $200,#4
        bclr      mybit
```

Both uses of `bclr` in this example generate identical code. Since a bit defined this way "knows" its size, the size attribute may be omitted when using it.

It is also possible to define bits that are located within a structure's element:

```asm
mystruct struct    dots
reg      ds.w      1
flag     defbit    reg,4
        ends

        org       $100
data     mystruct

        bset      data.flag  ; same as bset.w $100,#4
```

#### Super8

Opposed to the 'classic' Z8, the Super8 core supports instructions to operate on bits in working or general registers. ONe however has to to regard that some of them can only operate on bits in one of the 16 working registers. The `DEFBIT` instruction allows to define bits of either type:

```asm
workbit defbit  r3,#4
slow    defbit  emt,#6
```

Bits that have been defined this way may be used just like a argument duple of register and bit position:

```asm
        ldb     r3,emt,#6
        ldb     r3,slo          ; same result

        bitc    r3,#4
        bitc    workbit         ; same result
```

#### Z8000

The Z8000 features instructions to set and clear bits, however they cannot access addresses in I/O space. For this reason, both `DEFBIT` and `DEFBITB` only allow to define bit objects in memory space. The differentiation in operand size is important because the Z8000 is a big endian processor: bit _n_ of a 16 bit word at address _m_ corresponds to bit _n_ of an 8-bit byte at address _m+1_.

### DEFBITFIELD

_valid for: S12Z_

The S12Z family's CPU core not only deals with individual bits, it is also able to extract a field of consecutive bits from an 8/16/24/32 value or to insert a bit field into such a value. Similar to `DEFBIT`, a bit field may be defined symbolically:

```asm
<Name>     defbitfield[.size] <address>,<width>:<position>
```

Opposed to individual bits, an operand size of 24 bits (.p) is also allowed. The range of `position` and `width` is accordingly 0 to 23 resp. 1 to 24. It is also allowed to define bit fields as parts of structures:

```asm
mystruct struct      dots
reg      ds.w        1
clksel   defbitfield reg,4:8
        ends

        org       $100
data     mystruct

        bfext     d2,data.clksel ; fetch $100.w bits 4...11
                                ; to D2 bits 0...7
        bfins     data.clksel,d2 ; insert D2 bits 0...7 into
                                ; $100.w bits 4...11
```

The internal representation of bits defined via `DEFBIT` is equivalent to bit fields with a width of one. Therefore, a symbolically defined bit may also be used as argument for `BFINS` and `BFEXT`.

### PORT

_valid for: 8008/8080/8085/8086, XA, Z80, Z8000, 320C2x/5x, TLCS-47, AVR, F8_

`PORT` works similar to `EQU`, just the symbol becomes assigned to the I/O-address range. Allowed values are 0...7 for the 3201x and 8008, 0...15 for the 320C2x, 0...65535 for the 8086, Z8000, and 320C5x, 0...63 for the AVR, and 0...255 for the rest.

Example : an 8255 PIO is located at address 20H:

```asm
PIO_port_A port 20h
PIO_port_B port PIO_port_A+1
PIO_port_C port PIO_port_A+2
PIO_ctrl   port PIO_port_A+3
```

### REG and NAMEREG

_valid for: 680x0, AVR, M\*Core, ST9, 80C16x, Z8000, KCPSM (`NAMEREG` valid only for KCPSM(3)), LatticeMico8, MSP430(X)_
Though it always has the same syntax, this instruction has a slightly different meaning from processor to processor: If the processor uses a separate addressing space for registers, `REG` has the same effect as a simple `EQU` for this address space (e.g. for the ST9). `REG` defines register symbols for all other processors whose function is described in [Register Symbols](assembler-usage.md#register-symbols).

`NAMEREG` exists for compatibility reasons to the original KCPSM assembler. It has an identical function, however both register and symbolic name are given as arguments, for example:

```asm
        NAMEREG  s08, treg
```

### LIV and RIV

_valid for: 8X30x_

`LIV` and `RIV` allow to define so-called "IV bus objects". These are groups of bits located in a peripheral memory cell with a length of 1 up to 8 bits, which can afterwards be referenced symbolically. The result is that one does not anymore have to specify address, position, and length separately for instructions that can refer to peripheral bit groups. As the 8X30x processors feature two peripheral address spaces (a "left" and a "right" one), there are two separate pseudo instructions. The parameters of these instructions are however equal: three parameters have to be given that specify address, start position and length. Further hints for the usage of bus objects can be found in [8X30x](processor-specific-hints.md#8x30x).

### CHARSET

_valid for: all processors_

Single board systems, especially when driving LCDs, frequently use character sets different to ASCII. So it is probably purely coincidental that the umlaut coding corresponds with the one used by the PC. To avoid error-prone manual encoding, the assembler contains a translation table for characters which assigns a target character to each source-code. To modify this table (which initial translates 1:1), one has to use the `CHARSET` instruction. `CHARSET` may be used with different numbers and types of parameters. If there is only a single parameter, it has to be a string expression which is interpreted as a file name by AS. AS reads the first 256 bytes from this table and copies them into the translation table. This allows to activate complex, externally generated tables with a single statement. For all other variants, the first parameter has to be an integer in the range of 0 to 255 which designates the start index of the entries to be modified in the translation table. One or two parameters follow, giving the type of modification:

A single additional integer modifies exactly one entry. For example,

```asm
        CHARSET 'ä',128
```

means that the target system codes the 'ä' into the number 128 (80H). If however two more integers are given, the first one describes the last entry to be modified, and the second the new value of the first table entry. All entries up to the index end are loaded sequentially. For example, in case that the target system does not support lower-case characters, a simple

```asm
        CHARSET 'a','z','A'
```

translates all lower-case characters automatically into the matching capital letters.

For the last variant, a string follows the start index and contains the characters to be placed in the table. The last example therefore may also be written as

```asm
        CHARSET 'a',"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
```

`CHARSET` may also be called without any parameters, which however has a drastic effect: the translation table is reinitialized to its initial state, i.e. all character translations are removed.

**CAUTION!** `CHARSET` not only affects string constants stored in memory, but also integer constants written as "ASCII". This means that an already modified translation table can lead to other results in the above mentioned examples!

### CODEPAGE

_valid for: all processors_

Though the `CHARSET` statement gives unlimited freedom in the character assignment between host and target platform, switching among different character _sets_ can become quite tedious if several character sets have to be supported on the target platform. The `CODEPAGE` instruction however allows to define and keep different character sets and to switch with a single statement among them. `CODEPAGE` expects one or two arguments: the name of the set to be used hereafter and optionally the name of another table that defines its initial contents (the second parameter therefore only has a meaning for the first switch to the table when AS automatically creates it). If the second parameter is missing, the initial contents of the new table are copied from the previously active set. All subsequent `CHARSET` statements _only_ modify the new set.

At the beginning of a pass, AS automatically creates a single character set with the name `STANDARD` with a one-to-one translation. If no `CODEPAGE` instructions are used, all settings made via `CHARSET` refer to this table.

### ENUM, NEXTENUM, and ENUMCONF

_valid for: all processors_

Similar to the same-named instruction known from C, `ENUM` is used to define enumeration types, i.e. a sequence of integer constants that are assigned sequential values starting at 0. The parameters are the names of the symbols, like in the following example:

```asm
        ENUM    SymA,SymB,SymC
```

This instruction will assign the values 0, 1, and 2 to the symbols `SymA, SymB,` and `SymC`.

If you want to split an enumeration over more than one line, use `NEXTENUM` instead of `ENUM` for the second and all following lines. The internal counter that assigns sequential values to alls symbols will then not be reset to zero, like in the following case:

```asm
        ENUM     January=1,February,March,April,May,June
        NEXTENUM July,August,September,October
        NEXTENUM November,December
```

This example also demonstrates that it is possible to assign explicit values to individual symbols. The internal counter will be updated accordingly if this feature is used.

A definition of a symbol with `ENUM` is equal to a definition with `EQU`, i.e. it is not possible to assign a new value to a symbol that already exists.

The `ENUMCONF` statement allows to influence the behaviour of `ENUM`. `ENUMCONF` accepts one or two arguments. The first argument is always the value the internal counter is incremented for every symbol in an enumeration. For instance, the statement

```asm
        ENUMCONF 2
```

has the effect that symbols get the values 0,2,4,6... instead of 0,1,2,3...

The second (optional) argument of `ENUMCONF` rules which address space the defined symbols are assigned to. By default, symbols defined by `ENUM` are typeless. For instance, the statement

```asm
        ENUMCONF 1,CODE
```

defines that they should be assigned to the instruction address space. The names of the address spaces are the same as for the [`SEGMENT`](#segment) instruction, with the addition of `NOTHING` to generate typeless symbols again.

### PUSHV and POPV

_valid for: all processors_

`PUSHV` and `POPV` allow to temporarily save the value of a symbol (that is not macro-local) and to restore it at a later point of time. The storage is done on stacks, i.e. Last-In-First-Out memory structures. A stack has a name that has to fulfill the general rules for symbol names and it exists as long as it contains at least one element: a stack that did not exist before is automatically created upon `PUSHV`, and a stack becoming empty upon a `POPV` is deleted automatically. The name of the stack that shall be used to save or restore symbols is the first parameter of `PUSH` resp. `POPV`, followed by a list of symbols as further parameters. All symbols referenced in the list already have to exist, it is therefore **not** possible to implicitly define symbols with a `POPV` instruction.

Stacks are a global resource, i.e. their names are not local to sections.

It is important to note that symbol lists are **always** processed from left to right. Someone who wants to pop several variables from a stack with a `POPV` therefore has to use the exact reverse order used in the corresponding `PUSHV`!

The name of the stack may be left blank, like this:

```asm
        pushv   ,var1,var2,var3
        .
        .
        popv    ,var3,var2,var1
```

AS will then use a predefined internal default stack.

AS checks at the end of a pass if there are stacks that are not empty and issues their names together with their "filling level". This allows to find out if there are any unpaired `PUSHVs` or `POPVs`. However, it is in no case possible to save values in a stack beyond the end of a pass: all stacks are cleared at the beginning of a pass!

## Code Modification

### ORG

_valid for: all processors_

`ORG` allows to load the internal address counter (of the assembler) with a new value. The value range depends on the currently selected segment and on the processor type ([Address Ranges for `ORG` table](#table-address-ranges-for-org)). The lower bound is always zero, and the upper bound is the given value minus 1.

**CAUTION**: If the `PHASE` instruction is also used, one has to keep in mind that the argument of `ORG` always is the _load address_ of the code. Expressions using the $ or symbol to refer to the current program counter however deliver the _execution address_ of the code and do not yield the desired result when used as argument for `ORG`. The `RORG` statement ([RORG](#rorg)) should be used in such cases.

##### **Table:** Address Ranges for `ORG`

<!-- markdownlint-disable MD033-->

|                    |                 |               |          |          |        |          |               |     |          |          |
| :----------------- | :-------------: | :-----------: | :------: | :------: | :----: | :------: | :-----------: | :-: | :------: | :------: |
| Target             |      CODE       |     DATA      |  I-DATA  |  X-DATA  | Y-DATA | BIT-DATA |      IO       | REG | ROM-DATA | EE-DATA  |
| 68xxx/MCFxxxx      |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| DSP56000           |       64K       |      ---      |   ---    |   64K    |  64K   |   ---    |      ---      | --- |   ---    |   ---    |
| DSP56300           |       16M       |      ---      |   ---    |   16M    |  16M   |   ---    |      ---      | --- |   ---    |   ---    |
| PowerPC            |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| M\*Core            |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6800               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6301               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6811               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6805               |       8K        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 68HC08             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6809               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6309               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 68HC12             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 68HC12X            |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| XGATE              |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| S12Z               |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 68HC16             |       1M        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 68RS08             |       16K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| H8/300             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| H8/300H            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| H8/500             |     64K/16M     |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SH7000             |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SH7600             |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SH7700             |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| HD614023           |       2K        |      160      |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| HD614043           |       4K        |      160      |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| HD614081           |       8K        |      160      |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| HD641016           |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 6502               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MELPS‑740          |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| HUC6280            |       2M        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 65816              |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MELPS‑7700         |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MELPS‑4500         |       8K        |      416      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| M16                |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| M16C               |       1M        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 4004               |       4K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 8008               |       16K       |       8       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑41             | 1/2/4/6/8K[^O6] |      ---      |   256    | 256[^O8] |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑48             | 1/2/4/6/8K[^O6] |      ---      |   256    | 256[^O8] |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑51             |       64K       |      256      | 256[^O1] |   64K    |  ---   |   256    |      ---      | --- |   ---    |   ---    |
| 80C390             |       16M       |      256      | 256[^O1] |   16M    |  ---   |   256    |      ---      | --- |   ---    |   ---    |
| MCS‑251            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      512      | --- |   ---    |   ---    |
| MCS‑96             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑196            |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑196N           |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MCS‑296            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 8080               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| 8085               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| 80x86              |       64K       |      64K      |   ---    |   64K    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| 68xx0              |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 8X30x              |       8K        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 2650               |       8K        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| XA                 |       16M       |      16M      |   ---    |   ---    |  ---   |   ---    |    2K[^O3]    | --- |   ---    |   ---    |
| AVR                |    128K[^O6]    |   32K[^O6]    |   ---    |   ---    |  ---   |   ---    |      64       | --- |   ---    | 8K[^O7]  |
| 29XXX              |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 80C166             |      256K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 80C167             |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| Z80                |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| Z180               |    512K[^O2]    |               |          |          |        |          |      256      |     |          |   ---    |
| Z380               |       4G        |               |          |          |        |          |      4G       |     |          |          |
| Z8                 |       64K       |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| eZ8                |       64K       |      256      |   ---    |   64K    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| Z8001              |       8M        |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| Z8003              |       8M        |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| Z8002              |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| Z8004              |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| KCPSM              |       256       |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| KCPSM3             |       256       |      64       |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| Mico8              |      4096       |      256      |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| TLCS‑900(L)        |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TLCS‑90            |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TLCS‑870(/C)       |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TLCS‑47            |       64K       |      1K       |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| TLCS‑9000          |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TC9331             |       320       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| PIC 16C5x          |       2K        |      32       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| PIC 16C5x          |       2K        |      32       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| PIC 16C64          |       8K        |      512      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    | 256[^O6] |
| PIC 16C86          |       8K        |      512      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    | 256[^O6] |
| PIC 17C42          |       64K       |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SX20               |       2K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| ST6                |       4K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| ST7                |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| STM8               |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| ST9                |       64K       |      64K      |   ---    |   ---    |  ---   |   ---    |      ---      | 256 |   ---    |   ---    |
| 6804               |       4K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 32010              |       4K        |      144      |   ---    |   ---    |  ---   |   ---    |       8       | --- |   ---    |   ---    |
| 32015              |       4K        |      256      |          |          |        |          |       8       |     |          |          |
| 320C2x             |       64K       |      64K      |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| 320C3x             |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 320C40             |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 320C44             |       32M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 320C5x             |       64K       |      64K      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| 320C20x            |       64K       |      64K      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| 320C54x            |       64K       |      64K      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| TMS 9900           |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TMS 70Cxx          |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 370xxx             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSP430             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TMS1000            |       1K        |      64       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TMS1200            |       1K        |      64       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TMS1100            |       2K        |      128      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| TMS1300            |       2K        |      128      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SC/MP              |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 807x               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| COP4               |       512       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| COP8               |       8K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| SC144xx            |       256       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS16008            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32008            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS08032            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS16032            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32016            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32032            |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32CG16           |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32332            |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| NS32532            |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| ACE                |     4K[^O4]     |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| F3850              |       64K       |      64       |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| F8                 |       4K        |      64       |   ---    |   ---    |  ---   |   ---    |      256      | --- |   ---    |   ---    |
| µPD78(C)xx         |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 7566               |       1K        |      64       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 7508               |       4K        |      256      |   ---    |   ---    |  ---   |   ---    |      16       | --- |   ---    |   ---    |
| 75K0               |       16K       |      4K       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 78K0               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 78K2               |       1M        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 78K3               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 78K4               |    16M[^O5]     |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 7720               |       512       |      128      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   512    |   ---    |
| 7725               |       2K        |      256      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   1024   |   ---    |
| 77230              |       8K        |      ---      |   ---    |   512    |  512   |   ---    |      ---      | --- |    1K    |   ---    |
| 53C8XX             |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| F<sup>2</sup>MC8L  |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| F<sup>2</sup>MC16L |       16M       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5840            |       2K        |      128      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5842            |       768       |      32       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM58421           |      1.5K       |      40       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM58422           |      1.5K       |      40       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5847            |      1.5K       |      96       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5054            |       1K        |      62       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5055            |      1.75K      |      96       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM5056            |      1.75K      |      90       |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MSM6051            |      2.5K       |      119      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| MN1610             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| MN1613             |      256K       |      ---      |   ---    |   ---    |  ---   |   ---    |      64K      | --- |   ---    |   ---    |
| PMCxxx             |   1...4K[^O9]   | 64...256[^O9] |   ---    |   ---    |  ---   |   ---    | 32...128[^O9] | --- |   ---    |   ---    |
| PMSxxx             |   1...4K[^O9]   | 64...256[^O9] |   ---    |   ---    |  ---   |   ---    | 32...128[^O9] | --- |   ---    |   ---    |
| PFSxxx             |   1...4K[^O9]   | 64...256[^O9] |   ---    |   ---    |  ---   |   ---    | 32...128[^O9] | --- |   ---    |   ---    |
| 180x               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |       8       | --- |   ---    |   ---    |
| XS1                |       4G        |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| 1750               |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| KENBAK             |       256       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |
| CP1600             |       64K       |      ---      |   ---    |   ---    |  ---   |   ---    |      ---      | --- |   ---    |   ---    |

[^O1]: Initial value 80h. As the 8051 does not have any RAM beyond 80h, this value has to be adapted with ORG for the 8051 as target processor!
[^O2]: As the Z180 still can address only 64K logically, the whole address space can only be reached via `PHASE` instructions!
[^O3]: initial value 400h.
[^O4]: initial value 800h resp. 0C00h
[^O5]: area for program code is limited to 1 MByte
[^O6]: size depends on target processor
[^O7]: size and availability depend on target processor
[^O8]: only on variants supporting the `MOVX` instruction
[^O9]: device dependant

<!-- markdownlint-enable MD033-->

In case that different variations in a processor family have address spaces of different size, the maximum range is listed for each.

`ORG` is mostly needed to give the code a new starting address or to put different, non-continuous code parts into one source file. In case there is no explicit other value listet in a table entry, the initial address for this segment (i.e. the start address used without `ORG`) is 0.

### RORG

_valid for: all processors_

`RORG` modifies the program counter just like `ORG`, however it does not expect an absolute address as argument. Instead, it expects a relative value (positive or negative) that is added to the current program counter. A possible application of this statement is the reservation of a certain amount of address space, or the use in code parts that are included multiple times (e.g. via macros or includes) and that shall be position-independent. Another application is the use in code that has an execution address different from the load address (i.e. the `PHASE` statement is used). There is no symbol to refer to the current _load address_, but it can be referred to indirectly via the `RORG` statement.

### CPU

_valid for: all processors_

This command rules for which processor the further code shall be generated. Instructions of other processor families are not accessible afterwards and will produce error messages!

The processors can roughly be distinguished in families, inside the families different types additionally serve for a detailed distinction:

1.  68008 → 68000 → 68010 → 68012 → MCF5202 → MCF5204 → MCF5206 → MCF5208 → MCF52274 → MCF52277 → MCF5307 → MCF5329 → MCF5373 → MCF5407 → MCF5470 → MCF5471 → MCF5472 → MCF5473 → MCF5474 → MCF5475 → MCF51QM → 68332 → 68340 → 68360 → 68020 → 68030 → 68040

    The differences in this family are additional instructions and addressing modes (starting from the 68020). A small exception is the step to the 68030 that misses two instructions: `CALLM` and `RTM`. The three representatives of the 683xx family have the same processor core (a slightly reduced 68020 CPU), however completely different peripherals. MCF5xxx represents various ColdFire variants from Motorola/Freescale/NXP, RISC processors downwardly binary compatible to the 680x0. For the 68040, additional control registers (reachable via `MOVEC`) and instructions for control of the on-chip MMU and caches were added.

2.  56000 → 56002 → 56300

    While the 56002 only adds instructions for incrementing and decrementing the accumulators, the 56300 core is almost a new processor: all address spaces are enlarged from 64K words to 16M and the number of instructions almost has been doubled.

3.  PPC403 → MPPC403 → MPC505 → MPC601 → MPC821 → RS6000

    The PPC403 is a reduced version of the PowerPC line without a floating point unit, which is why all floating point instructions are disabled for him; in turn, some microcontroller-specific instructions have been added which are unique in this family. The GC variant of the PPC403 incorporates an additional MMU and has therefore some additional instructions for its control. The MPC505 (a microcontroller variant without a FPU) only differ in its peripheral registers from the 601 as long as I do not know it better - [^Mot505] is a bit reluctant in this respect... The RS6000 line knows a few instructions more (that are emulated on many 601-based systems), IBM additionally uses different mnemonics for their pure workstation processors, as a reminiscence of 370 mainframes...

4.  MCORE

5.  XGATE

6.  6800 → 6801 → 6301 → 6811

    While the 6801 only offers a few additional instructions (and the 6301 even a few more), the 6811 provides a second index register and much more instructions.

7.  6809/6309 and 6805/68HC(S)08

    These processors are partially source-code compatible to the other 68xx processors, but they have a different binary code format and a significantly reduced (6805) resp. enhanced (6809) instruction set. The 6309 is a CMOS version of the 6809 which is officially only compatible to the 6809, but unofficially offers more registers and a lot of new instructions (see [^Kaku]).

8.  68HC12 → 68HC12X

    The 12X core offers a couple of new instructions, and existing
    instructions were were enriched with new addressing modes.

9.  S912ZVC19F0MKH, S912ZVC19F0MLF, S912ZVCA19F0MKH, S912ZVCA19F0MLF, S912ZVCA19F0WKH, S912ZVH128F2CLQ, S912ZVH128F2CLL, S912ZVH64F2CLQ, S912ZVHY64F1CLQ, S912ZVHY32F1CLQ, S912ZVHY64F1CLL, S912ZVHY32F1CLL, S912ZVHL64F1CLQ, S912ZVHL32F1CLQ, S912ZVHL64F1CLL, S912ZVHL32F1CLL, S912ZVFP64F1CLQ, S912ZVFP64F1CLL, S912ZVH128F2VLQ, S912ZVH128F2VLL, S912ZVH64F2VLQ, S912ZVHY64F1VLQ, S912ZVHY32F1VLQ, S912ZVHY64F1VL, S912ZVHY32F1VLL, S912ZVHL64F1VLQ

    All variants contain the same processor core and the same instruction
    set, only the on-chip peripherals and the amount of built-in memory
    (RAM, Flash-ROM, EEPROM) vary from device to device.

10. 68HC16

11. HD6413308 → HD6413309

    These both names represent the 300 and 300H variants of the H8 family; the H version owns a larger address space (16Mbytes instead of 64Kbytes), double-width registers (32 bits), and knows a few more instructions and addressing modes. It is still binary upward compatible.

12. HD6475328 → HD6475348 → HD6475368 → HD6475388

    These processors all share the same CPU core; the different types are only needed to include the correct subset of registers in the file `REG53X.INC`.

13. SH7000 → SH7600 → SH7700

    The processor core of the 7600 offers a few more instructions that close gaps in the 7000's instruction set (delayed conditional and relative and indirect jumps, multiplications with 32-bit operands and multiply/add instructions). The 7700 series (also known as SH3) furthermore offers a second register bank, better shift instructions, and instructions to control the cache.

14. HD614023 → HD614043 → HD614081

    These three variants of the HMCS400 series differ by the size of the internal ROM and RAM.

15. HD641016

    This is currently the only target with H16 core.

16. 6502 → 65(S)C02 → 65CE02 / W65C02S / 65C19 / MELPS740 / HUC6280 / 6502UNDOC

    The CMOS version defines some additional instructions, as well as a number of some instruction/addressing mode combinations were added which were not possible on the 6502. The W65C02S adds two opcodes to the 65C02 instruction set to give more fine-grained control over how to stop the CPU for low power modes. The 65SC02 lacks the bit manipulation instructions of the 65C02. The 65CE02 adds branch instructions with 16-bit displacement, a Z register, a 16 bit stack pointer, a programmable base page, and a couple of new instructions.

    The 65C19 is _not_ binary upward compatible to the original 6502! Some addressing modes have been replaced by others. Furthermore, this processor contains instruction set extensions that facilitate digital signal processing.

    The Mitsubishi micro controllers in opposite expand the 6502 instruction set primarily to bit operations and multiplication / division instructions. Except for the unconditional jump and instructions to increment/decrement the accumulator, the instruction extensions have nothing in common.

    For the HuC 6280, the feature that sticks out most is the larger address space of 2 MByte instead of 64 KBytes. This is achieved with a built-tin banking mechanism. Furthermore, it features some special instructions to communicate with a video processor (this chip was used in video games) and to copy memory areas.

    The 6502UNDOC processor type enables access to the "undocumented" 6502 instructions, i.e. the operations that result from the usage of bit combinations in the opcode that are not defined as instructions. The variants supported by AS are listed in the appendix containing processor-specific hints.

17. MELPS7700, 65816

    Apart from a '16-bit-version' of the 6502's instruction set, these processors both offer some instruction set extensions. These are however orthogonal as they are oriented along their 8-bit predecessors (65C02 resp. MELPS-740). Partially, different mnemonics are used for the same operations.

18. MELPS4500

19. M16

20. M16C

21. 4004 → 4040

    Opposed to its predecessor, the 4040 features about a dozen additional machine instructions.

22. 8008 → 8008

    NEW Intel redefined the mnemonics around 1975, the second variant reflects this new instruction set. A simultaneous support of both sets was not possible due to mnemonic conflicts.

23. 8021, 8022, 8401, 8411, 8421, 8461, 8039, (MSM)80C39, 8048, (MSM)80C48, 8041, 8042, 80C382

    For the ROM-less versions 8039 and 80C39, the commands which are using the BUS (port 0) are forbidden. The 8021 and 8022 are special versions with a strongly shrunk instruction set, for which the 8022 has two A/D- converters and the necessary control-commands. The instruction set of the MAB8401 to 8461 (designed by Philips) is somewhere in between the 8021/8022 and a "complete" MC-48 instruction set. On the other hand, they provide serial ports and up to 8 KBytes of program memory.

    It is possible to transfer the CMOS-versions with the `IDL` resp. `HALT` command into a stop mode with lower current consumption. The 8041 and 8042 have some additional instructions for controlling the bus interface, but in turn a few other commands were omitted. The code address space of 8041, 8042, 84x1, 8021, and 8022 is not externally extendable, and so AS limits the code segment of these processors to the size of the internal ROM. The (SAB)80C382 is a variant especially designed by Siemens for usage in telephones. It also knows a `HALT` instruction, plus ist supports indirect addressing for `DJNZ` and `DEC`. In turn, several instructions of the 'generic' 8048 were left out. The OKI variants (MSM...) also feature indirect addressing for `DJNZ` and `DEC`, plus enhanced control of power-down modes, plus the full basic MCS-48 instruction set.

24. 87C750 → 8051, 8052, 80C320, 80C501, 80C502, 80C504, 80515, and 80517 → 80C390 → 80C251

    The 87C750 can only access a maximum of 2 Kbytes program memory which is why it lacks the `LCALL` and `LJMP` instructions. AS does not make any distinction among the processors in the middle, instead it only stores the different names in the `MOMCPU` variable (see below), which allows to query the setting with `IF` instructions. An exception is the 80C504 that has a mask flaw in its current versions. This flaw shows up when an `AJMP` or `ACALL` instruction starts at the second last address of a 2K page. AS will automatically use long instructions or issues an error message in such situations. The 80C251 in contrast represents a drastic progress in the the direction 16/32 bits, larger address spaces, and a more orthogonal instruction set. One might call the 80C390 the 'small solution': Dallas Semiconductor modified instruction set and architecture only as far as it was necessary for the 16 Mbytes large address spaces.

25. 8096 → 80196 → 80196N → 80296

    Apart from a different set of SFRs (which however strongly vary from version to version), the 80196 knows several new instructions and supports a 'windowing' mechanism to access the larger internal RAM. The 80196N family extends the address space to 16 Mbytes and introduces a set of instructions to access addresses beyond 64Kbytes. The 80296 extends the CPU core by instructions for signal processing and a second windowing register, however removes the Peripheral Transaction Server (PTS) and therefore looses again two machine instructions.

26. 8080 → 8085 → 8085UNDOC

    The 8085 knows the additional commands `RIM` and `SIM` for controlling the interrupt mask and the two I/O-pins. The type `8085UNDOC` enables additional instructions that are not documented by Intel. These instructions are documented in section [8085UNDOC](processor-specific-hints.md#8085undoc).

27. 8086 → 80186 → V30 → V35

    Only new instructions are added in this family. The corresponding 8-bit versions are not mentioned due to their instruction compatibility, so one e.g. has to choose 8086 for an 8088-based system.

28. 80960

29. 8X300 → 8X305

    The 8X305 features a couple of additional registers that miss on the 8X300. Additionally, it can do new operations with these registers (like direct writing of 8 bit values to peripheral addresses).

30. XAG1, XAG2, XAG3

    These processors only differ in the size of their internal ROM which is defined in `STDDEFXA.INC`.

31. AT90S1200, AT90S2313, AT90S2323, AT90S233, AT90S2343, AT90S4414, AT90S4433, AT90S4434, AT90S8515, AT90C8534, AT90S8535, ATTINY4, ATTINY5, ATTINY9, ATTINY10, ATTINY11, ATTINY12, ATTINY13, ATTINY13A, ATTINY15, ATTINY20, ATTINY24(A), ATTINY25, ATTINY26, ATTINY28, ATTINY40, ATTINY44(A), ATTINY45, ATTINY48, ATTINY84(A), ATTINY85, ATTINY87, ATTINY88, ATTINY102, ATTINY104, ATTINY167, ATTINY261, ATTINY261A, ATTINY43U, ATTINY441, ATTINY461, ATTINY461A, ATTINY828, ATTINY841, ATTINY861, ATTINY861A, ATTINY1634, ATTINY2313, ATTINY2313A, ATTINY4313, ATMEGA48, ATMEGA8, ATMEGA8515, ATMEGA8535, ATMEGA88, ATMEGA8U2, ATMEGA16U2, ATMEGA32U2, ATMEGA16U4, ATMEGA32U4, ATMEGA32U6, AT90USB646, AT90USB647, AT90USB1286, AT90USB1287, AT43USB355, ATMEGA16, ATMEGA161, ATMEGA162, ATMEGA163, ATMEGA164, ATMEGA165, ATMEGA168, ATMEGA169, ATMEGA32, ATMEGA323, ATMEGA324, ATMEGA325, ATMEGA3250, ATMEGA328, ATMEGA329, ATMEGA3290, ATMEGA406, ATMEGA64, ATMEGA640, ATMEGA644, ATMEGA644RFR2, ATMEGA645, ATMEGA6450, ATMEGA649, ATMEGA6490, ATMEGA103, ATMEGA128, ATMEGA1280, ATMEGA1281, ATMEGA1284, ATMEGA1284RFR2, ATMEGA2560, ATMEGA2561

    The various AVR chip variants mainly differ in the amount of on-chip memory (flash, SRAM, EEPROM) an the set of built-in peripherals (GPIO, timers, UART, A/D converter...). Compared to the AT90... predecessors, the ATmega chip also provide additional instructions, while the ATtinys do not support the multiplication instructions.

32. AM29245 → AM29243 → AM29240 → AM29000

    The further one moves to the right in this list, the fewer the instructions become that have to be emulated in software. While, for example, the 29245 doesn't even have a hardware multiplier, the two representatives in the middle only lack the floating-point instructions. The 29000 serves as a 'generic' type that understands all instructions in hardware.

33. 80C166 → 80C167, 80C165, 80C163

    80C167 and 80C165/163 have an address space of 16 Mbytes instead of 256 Kbytes, and furthermore they know some additional instructions for extended addressing modes and atomic instruction sequences. They are 'second generation' processors and differ from each other only in the amount of on-chip peripherals.

34. Z80 → Z80UNDOC → Z180 → Z380

    While there are only a few additional instructions for the Z180, the Z380 owns 32-bit registers, a linear address space of 4 Gbytes, a couple of instruction set extensions that make the overall instruction set considerably more orthogonal, and new addressing modes (referring to index register halves, stack relative). These extensions partially already exist on the Z80 as undocumented extensions and may be switched on via the Z80UNDOC variant. A list with the additional instructions can be found in the chapter with processor specific hints.

35. Z8601, Z8603, z86C03, z86E03, Z86C06, Z86E06, Z86C08, Z86C21, Z86E21, Z86C30, Z86C31, Z86C32
    Z86C40 → Z88C00, Z88C01 → eZ8, Z8F0113, Z8F011A, Z8F0123, Z8F012A, Z8F0130, Z8F0131, Z8F0213, Z8F021A, Z8F0223, Z8F022A, Z8F0230, Z8F0231, Z8F0411, Z8F0412, Z8F0413, Z8F041A, Z8F0421, Z8F0422, Z8F0423, Z8F042A, Z8F0430, Z8F0431, Z8F0811, Z8F0812, Z8F0813, Z8F081A, Z8F0821, Z8F0822, Z8F0823, Z8F082A, Z8F0830, Z8F0831, Z8F0880, Z8F1232, Z8F1233, Z8F1621, Z8F1622, Z8F1680, Z8F1681, Z8F1682, Z8F2421, Z8F2422, Z8F2480, Z8F3221, Z8F3222, Z8F3281, Z8F3282, Z8F4821, Z8F4822, Z8F4823, Z8F6081, Z8F6082, Z8F6421, Z8F6422, Z8F6423, Z8F6481, Z8F6482

    The variants with Z8 core only differ in internal memory size and on-chip peripherals, i.e. the choice does not have an effect on the supported instruction set. Super8 and eZ8 are substantially different, each with an instruction set that was vastly extended (into different directions), and they are not fully upward-compatible on source code level as well.

36. Z8001, Z8002, Z8003, Z8004

    The operation mode (segmented for Z8001 and Z8003, non-segmented for Z8002 and Z8004) is selected via the processor type. There is currently no further differentiation between Z8001/8002 and Z8003/8004.

37. KCPSM

    Both processor cores are not available as standalone components, they are provided as logic cores for gate arrays made by Xilinx The -3 variant offers a larger address space and some additional instructions. Note that it is not binary upward-compatible!

38. MICO8_05, MICO8_V3, MICO8_V31

    Lattice unfortunately changed the machine instructions more than once, so different targets became necessary to provide continued support for older projects. The first variant is the one described in the 2005 manual, the two other ones represent versions 3.0 resp. 3.1.

39. 96C141, 93C141

    These two processors represent the two variations of the processor family: TLCS-900 and TLCS-900L. The differences of these two variations will be discussed in detail in [TLCS-900(L)](processor-specific-hints.md#tlcs-900l).

40. 90C141

41. 87C00, 87C20, 87C40, 87C70

    The processors of the TLCS-870 series have an identical CPU core, but different peripherals depending on the type. In part registers with the same name are located at different addresses. The file `STDDEF87.INC` uses, similar to the MCS-51-family, the distinction possible by different types to provide the correct symbol set automatically.

42. TLCS-870/C

    Currently, only the processor core of the TLCS-870/C family is implemented.

43. 47C00 → 470C00 → 470AC00

    These three variations of the TLCS-47-family have on-chip RAM and ROM of different size, which leads to several bank switching instructions being added or suppressed.

44. 97C241

45. TC9331

46. 16C54 → 16C55 → 16C56 → 16C57

    These processors differ by the available code area, i.e. by the address limit after which AS reports overruns.

47. 16C84, 16C64

    Analog to the MCS-51 family, no distinction is made in the code generator, the different numbers only serve to include the correct SFRs in `STDDEF18.INC`.

48. 17C42

49. SX20, SX28

    The SX20 uses a smaller housing and lacks port C.

50. ST6200, ST6201, ST6203, ST6208, ST6209, ST6210, ST6215, ST6218, ST6220, ST6225, ST6228, ST6230, ST6232, ST6235, ST6240, ST6242, ST6245, ST6246, ST6252, ST6253, ST6255, ST6260, ST6262, ST6263, ST6265, ST6280, ST6285

    The various ST6 derivates differ in the amount of on-chip peripherals and built-in memory.

51. ST7
    ST72251G1, ST72251G2, ST72311J2, ST72311J4, ST72321BR6, ST72321BR7, ST72321BR9, ST72325S4, ST72325S6, ST72325J7, ST72325R9, ST72324J6, ST72324K6, ST72324J4, ST72324K4, ST72324J2, ST72324JK21, ST72325S4, ST72325J7, ST72325R9, ST72521BR6, ST72521BM9, ST7232AK1, ST7232AK2, ST7232AJ1, ST7232AJ2, ST72361AR4, ST72361AR6, ST72361AR7, ST72361AR9, ST7FOXK1, ST7FOXK2, ST7LITES2Y0, ST7LITES5Y0, ST7LITE02Y0, ST7LITE05Y0, ST7LITE09Y0
    ST7LITE10F1, ST7LITE15F1, ST7LITE19F1, ST7LITE10F0, ST7LITE15F0, ST7LITE15F1, ST7LITE19F0, ST7LITE19F1, ST7LITE20F2, ST7LITE25F2, ST7LITE29F2, ST7LITE30F2, ST7LITE35F2, ST7LITE39F2, ST7LITE49K2, ST7MC1K2, ST7MC1K4, ST7MC2N6, ST7MC2S4, ST7MC2S6, ST7MC2S7, ST7MC2S9, ST7MC2R6, ST7MC2R7, ST7MC2R9, ST7MC2M9, STM8
    STM8S001J3, STM8S003F3, STM8S003K3, STM8S005C6, STM8S005K6, STM8S007C8, STM8S103F2, STM8S103F3, STM8S103K3, STM8S105C4, STM8S105C6, STM8S105K4, STM8S105K6, STM8S105S4, STM8S105S6, STM8S207MB, STM8S207M8, STM8S207RB, STM8S207R8, STM8S207R6, STM8S207CB, STM8S207C8, STM8S207C6, STM8S207SB, STM8S207S8, STM8S207S6, STM8S207K8, STM8S207K6, STM8S208MB, STM8S208RB, STM8S208R8, STM8S208R6, STM8S208CB, STM8S208C8, STM8S208C6, STM8S208SB, STM8S208S8, STM8S208S6, STM8S903K3, STM8S903F3, STM8L050J3, STM8L051F3, STM8L052C6, STM8L052R8, STM8L001J3, STM8L101F1, STM8L101F2, STM8L101G2, STM8L101F3, STM8L101G3, STM8L101K3, STM8L151C2, STM8L151K2, STM8L151G2, STM8L151F2, STM8L151C3, STM8L151K3, STM8L151G3, STM8L151F3, STM8L151C4, STM8L151C6, STM8L151K4, STM8L151K6, STM8L151G4, STM8L151G6, STM8L152C4, STM8L152C6, STM8L152K4, STM8L152K6, STM8L151R6, STM8L151C8, STM8L151M8, STM8L151R8, STM8L152R6, STM8L152C8, STM8L152K8, STM8L152M8, STM8L152R8, STM8L162M8, STM8L162R8, STM8AF6366, STM8AF6388, STM8AF6213, STM8AF6223, STM8AF6226, STM8AF6246, STM8AF6248, STM8AF6266, STM8AF6268, STM8AF6269, STM8AF6286, STM8AF6288, STM8AF6289, STM8AF628A, STM8AF62A6, STM8AF62A8, STM8AF62A9, STM8AF62AA, STM8AF5268, STM8AF5269, STM8AF5286, STM8AF5288, STM8AF5289, STM8AF528A, STM8AF52A6, STM8AF52A8, STM8AF52A9, STM8AF52AA, STM8AL3136, STM8AL3138, STM8AL3146, STM8AL3148, STM8AL3166, STM8AL3168, STM8AL3L46, STM8AL3L48, STM8AL3L66, STM8AL3L68, STM8AL3188, STM8AL3189, STM8AL318A, STM8AL3L88, STM8AL3L89, STM8AL3L8A, STM8TL52F4, STM8TL52G4, STM8TL53C4, STM8TL53F4, STM8TL53G4

    The STM8 core extends the address space to 16 Mbytes and introduces a couple of new instructions. Though many instructions have the same machine code as for ST7, it is not binary upward compatible.

52. ST9020, ST9030, ST9040, ST9050

    These 4 names represent the four "sub-families" of the ST9 family, which only differ in their on-chip peripherals. Their processor cores are identical, which is why this distinction is again only used in the include file containing the peripheral addresses.

53. 6804

54. 32010 → 32015

    The TMS32010 owns just 144 bytes of internal RAM, and so AS limits addresses in the data segment just up to this amount. This restriction does not apply for the 32015, the full range from 0...255 can be used.

55. 320C25 → 320C26 → 320C28

    These processors only differ slightly in their on-chip peripherals and in their configuration instructions.

56. 320C30, 320C31 → 320C40, 320C44

    The 320C31 is a reduced version with the same instruction set, however fewer peripherals. The distinction is exploited in `STDDEF3X.INC`. The C4x variants are source-code upward compatible, the machine codes of some instructions are however slightly different. Once again, the C44 is a stripped-down version of the C40, with less peripherals and a smaller address space.

57. 320C203 → 320C50, 320C51, 320C53

    The first one represents the C20x family of signal processors which implement a subset of the C5x instruction set. The distinction among the C5x processors is currently not used by AS.

58. 320C541

    This one at the moment represents the TMS320C54x family...

59. TI990/4, TI990/10, TI990/12
    TMS9900, TMS9940, TMS9995, TMS99105, TMS99110

    The TMS99xx/99xxx processors are basically single chip implementations of the TI990 minicomputers. Some TI990 models are even based on such a processor instead of a discrete CPU. The individual models differ in their instruction set (the TI990/12 has the largest one) and the presence of a privileged mode.

60. TMS70C00, TMS70C20, TMS70C40, TMS70CT20, TMS70CT40, TMS70C02, TMS70C42, TMS70C82, TMS70C08, TMS70C48

    All members of this family share the same CPU core, they therefore do not differ in their instruction set. The differences manifest only in the file `REG7000.INC` where address ranges and peripheral addresses are defined. Types listed in the same row have the same amount of internal RAM and the same on-chip peripherals, they differ only in the amount of integrated ROM.

61. 370C010, 370C020, 370C030, 370C040 and 370C050

    Similar to the MCS-51 family, the different types are only used to differentiate the peripheral equipment in `STDDEF37.INC`; the instruction set is always the same.

62. MSP430 → MSP430X

    The X variant of the CPU core extends the address space from 64 KiBytes to 1 MiByte and augments the instruction set, e.g. by prefixed to repeat instructions.

63. TMS1000, TMS1100, TMS1200, TMS1300

    TMS1000 and TMS1200 each provide 1 KByte of ROM and 64 nibbles of RAM, while TMS1100 and TMS1300 provide twice the amount of RAM and ROM. Furthermore, TI has defined a significantly different default instruction set fot TMS1100 and TMS1300(AS only knows the default instruction sets!)

64. SC/MP

65. 8070

    This processor represents the whole 807x family (which consists at least of the 8070, 8072, and 8073), which however shares identical CPU cores.

66. COP87L84

    This is the only member of National Semiconductor's COP8 family that is currently supported. I know that the family is substantially larger, and that there are representatives with differently sized instruction sets which will be added when a need occurs. It is a beginning, and National's documentation is quite extensive...

67. COP410 → COP420 → COP440 → COP444

    The COP42x derivates offer some additional instructions, plus other instructions have an extended operand range.

68. SC14400, SC14401, SC14402, SC14404, SC14405, SC14420, SC14421, SC14422, SC14424

    This series of DECT controllers differentiates itself by the amount of instructions, since each of them supports different B field formats and their architecture has been optimized over time.

69. NS16008, NS32008, NS08032, NS16032, NS32016, NS32032, NS32332, NS32CG16, NS32532

    National renamed the first-generation CPUs several times in the early years, NS16008/NS32008/NS08032 resp. NS16032/NS32016 are the same chips. NS32332 and NS32532 support an address space of 4 GBytes instead of 16 MBytes, and the NS32CG16 is an embedded variant with additional instructions for bit block transfers.

70. ACE1101, ACE1202

71. F3850, MK3850, MK3870, MK3870/10, MK3870/12, MK3870/20, MK3870/22, MK3870/30, MK3870/32, MK3870/40, MK3870/42, MK3872, MK3873, MK3873/10, MK3873/12, MK3873/20, MK3873/22, MK3874, MK3875, MK3875/22, MK3875/42, MK3876, MK38P70/02, MK38C70, MK38C70/10, MK38C70/20, MK97400, MK97410, MK97500, MK97501, MK97503

    This huge amount of variants partially results from the fact that Mostek renamed some variants in the early 80s. The new naming scheme allows to deduce the amount of internal ROM (0 to 4 for 0 to 4 Kbytes) and executable RAM (0 or 2 for 0 or 64 bytes) from the suffix. 3850 and MK975xx support a 64K address space, which is only 4 Kbytes for all other variants. P variants have an EEPROM piggyback socket for prototyping, C variants are fabricated in CMOS technology and feature two new machine instructions (HET and HAL). The MK3873's feature is a built-in serial port, while the MK3875 offers a second supply voltage pin to buffer the internal memory in standby mode.

72. 7800, 7801, 7802
    78C05, 78C06
    7810 → 78C10, 78C11, 78C12, 78C14, 78C17, 78C18

    μPD7800 to μPD7802 represent the "first generation" of the uCOM87 family from NEC. μPD78C05 and μPD78C06 are reduced variants that implement only a subset of the instruction set. All μPD781x variants belong to the uCOM87AD series, which - aside from an A/D converter - also supports additional registers and machine instructions.

    **NOTE:** The instruction set is only partially binary upward compatible! The NMOS version μPD7810 has no stop-mode; the respective command and the ZCM register are omitted.

    **CAUTION!** NMOS and CMOS version partially differ in the reset values of some registers!

73. 7500 ↔ 7508

    There are two different types of CPU cores in the μPD75xx family: the 7566 represents the the 'instruction set B', which provides less instructions, less registers and smaller address spaces. The 7508 represents the 'full' instruction set A.

    **CAUTION!** These instruction sets are not 100% binary compatible!

74. 75402, 75004, 75006, 75008, 75268, 75304, 75306, 75308, 75312, 75316, 75328, 75104, 75106, 75108, 75112, 75116, 75206, 75208, 75212, 75216, 75512, 75516

    This 'cornucopia' of processors differs only by the RAM size in one group; the groups themselves again differ by their on-chip peripherals on the one hand and by their instruction set's power on the other hand.

75. 78070

    This is currently the only member of NEC's 78K0 family I am familiar with. Similar remarks like for the COP8 family apply!

76. 78214

    This is currently the representor of NEC's 78K2 family.

77. 78310

    This is currently the representor of NEC's 78K3 family.

78. 784026

    This is currently the representor of NEC's 78K4 family.

79. 7720 → 7725

    The μPD7725 offers larger address spaces and som more instructions compared to his predecessor.

    **CAUTION!** The processors are not binary compatible to each other!

80. 77230

81. SYM53C810, SYM53C860, SYM53C815, SYM53C825, SYM53C875, SYM53C895

    The simpler members of this family of SCSI processors lack some instruction variants, furthermore they are different in their set of internal registers.

82. MB89190

    This processor type represents Fujitsu's F<sup>2</sup>MC8L series...

83. MB9500

    ...just like this one does it currently for the 16-bit variants from Fujitsu!

84. MSM5840, MSM5842, MSM58421, MSM58422, MSM5847

    These variants of the OLMS-40 family differ in their instruction set and in the amount of internal program and data memory.

85. MSM5054, MSM5055, MSM5056, MSM6051, MSM6052

    The as for the OLMS-40 family: differences in instruction set and the amount of internal program and data memory.

86. MN1610\[ALT\] → MN1613\[ALT\]

    In addition to its predecessor's features, the MN1613 offers a larger address space, a floating point unit and a couple of new machine instructions.

87. PMC150, PMS150, PFS154, PMC131, PMS130, PMS131
    PMS132, PMS132B, PMS152, PMS154B, PMS154C, PFS173
    PMS133, PMS134, DF69, MCS11, PMC232, PMC234, PMC251
    PMC271,PMC884, PMS232, PMS234, PMS271

    The Padauk controllers differ in the size of the internal (ROM/RAM) memory, the type of internal ROM (erasable or OTP), the built-in peripherals, and their instruction set (both extent and binary coding).

88. 1802 → 1804, 1805, 1806 → 1804A, 1805A, 1806A

    1804, 1805, and 1806 feature an instruction set that is slightly enhanced, compared to the 'original' 1802, plus on-chip RAM and an integrated timer. The A variants extend the instruction set by `DSAV`, `DBNZ`, and instructions for addition and subtraction in BCD format.

89. XS1

    This type represents the XCore-"family".

90. 1750

    MIL STD 1750 is a standard, therefore there is only one (standard) variant...

91. KENBAK

    Since there has never been a KENBAK-2, the target is simply KENBAK...

92. CP-1600

    The `CPU` instruction needs the processor type as a simple literal, a calculation like:

    ```asm
        CPU     68010+10
    ```

    is not allowed. Valid calls are e.g.

    ```asm
        CPU     8051
    ```

    or

    ```asm
        CPU     6800
    ```

Regardless of the processor type currently set, the integer variable `MOMCPU` contains the current status as a hexadecimal number. For example, `MOMCPU`=$68010 for the 68010 or `MOMCPU`=80C48H for the 80C48. As one cannot express all letters as hexadecimal digits (only A...F are possible), all other letters must must be omitted in the hex notation; for example, `MOMCPU`=80H for the Z80.

You can take advantage of this feature to generate different code depending on the processor type. For example, the 68000 does not have a machine instruction for a subroutine return with stack correction. With the variable `MOMCPU` you can define a macro that uses the machine instruction or emulates it depending on the processor type:

```asm
myrtd   macro   disp
        if      MOMCPU<$68010 ; emulate for 68008 & 68000
        move.l (sp),disp(sp)
        lea    disp(sp),sp
        rts
        elseif
        rtd    #disp         ; direct use on >=68010
        endif
        endm


        cpu     68010
        myrtd   12            ; results in RTD #12

        cpu     68000
        myrtd   12            ; results in MOVE.../LEA.../RTS
```

As not all processor names are built only out of numbers and letters from A...F, the full name is additionally stored in the string variable named `MOMCPUNAME`.

The assembler implicitly switches back to the `CODE` segment when a `CPU` instruction is executed. This is done because `CODE` is the only segment all processors support.

The default processor type is 68008, unless it has been changed via the command line option with same name.

Some targets define options or variants that are so fundamental for operation, that they have to be selected with the `CPU` instruction. Such options are appended to the argument, separated by double colons:

```asm
      CPU <CPU Name>:<var1>=<val1>:<var2>=<val2>:...
```

See the respective section with processor-specific hints to check whether a certain target supports such options.

### SUPMODE, FPU, PMMU, CUSTOM

_SUPMODE valid for: 680x0, i960, TLCS-900, SH7000, i960, 29K, XA, PowerPC, M\*Core, and TMS9900_
_FPU valid for: 680x0, NS32xxx, 80x86_
_PMMU valid for: 680x0, NS32xxx_
_CUSTOM valid for: NS32xxx_

These three switches allow to define which parts of the instruction set shall be disabled because the necessary preconditions are not valid for the following piece of code. The parameter for these instructions may be either `ON` or `OFF`, the current status can be read out of a variable which is either TRUE or FALSE.

The commands have the following meanings in detail:

- `SUPMODE`: allows or prohibits commands, for whose execution the processor has to be within the supervisor mode. The status variable is called `INSUPMODE`.

- `FPU`: allows or prohibits the commands of the numerical coprocessors 8087, NS32081/32381 resp. 68881 or 68882. The status variable is called `FPUAVAIL`. For NS32xxx as target, specifying the explicit FPU type (`NS32081`, `NS32181`, `NS32381`, or `NS32580`) is also possible, to enable or disable the additional registers and instructions.

- `PMMU`: allows or prohibits the commands of the memory management unit 68851 resp. of the built-in MMU of the 68030.

  **CAUTION!** The 68030-MMU supports only a relatively small subset of the 68851 instructions. This is controlled via the `FULLPMMU` statement. The status variable is called `PMMUAVAIL`. For NS32xxx as target, specifying the explicit MMU type as target (`NS32082`, `NS32381`, or `NS32352`) is also possible, to enable access to the MMU-type-specific register set.

- `CUSTOM`: allows or prohibits the commands reserved for custom slave processors.

The usage of of instructions prohibited in this manner will generate a warning at `SUPMODE`, at `PMMU` and `FPU` a real error message.

### FULLPMMU

_valid for: 680x0_

Motorola integrated the MMU into the processor starting with the 68030, but the built-in FPU is equipped only with a relatively small subset of the 68851 instruction set. AS will therefore disable all extended MMU instructions when the target processor is 68030 or higher. It is however possible that the internal MMU has been disabled in a 68030-based system and the processor operates with an external 68851. One can the use a `FULLPMMU ON` to tell AS that the complete MMU instruction set is allowed. Vice versa, one may use a `FULLPMMU OFF` to disable all additional instruction in spite of a 68020 target platform to assure that portable code is written. The switch between full and reduced instruction set may be done as often as needed, and the current setting may be read from a symbol with the same name.

**CAUTION!** The `CPU` instruction implicitly sets or resets this switch when its argument is a 68xxx processor! `FULLPMMU` therefore has to be written after the `CPU` instruction!

### PADDING

_valid for: 680x0, 68xx, M\*Core, XA, H8, SH7000, MSP430(X), TMS9900, ST7/STM8, AVR (only if code segment granularity is 8 bits)_

Various processor families have a requirement that objects of more than one byte length must be located on a n even address. Aside from data objects, this may also include instruction words. For instance, word accesses to an odd address result in an exception on a 68000, while other processors like the H8 force the lowest address bit to zero.

The `PADDING` instruction allows to activate a mechanism that tries to avoid such misalignments. If the situation arises that an instruction word, or a data object of 16 bits or more (created e.g. via `DC`) would be stored on an odd address, a padding byte is automatically inserted before. Such a padding byte is displayed in the listing in a separate line that contains the remark

```asm
    <padding>
```

If the source line also contained a label, the label still points to the address of the code or data object, i.e. right behind the pad byte. The same is true for a label in a source line immediately before, as long as this line only holds the label and no other instruction. So, in the following example:

```asm
        padding  on
        org      $1000

        dc.b     1
adr1:  nop

        dc.b     1
adr2:
        nop

        dc.b     1
adr3:  equ      *
        nop
```

the labels `adr1` and `adr2` hold the addresses of the respective `NOP` instructions, which were made even by inserting a pad byte. `adr3` in contrast holds the address of the pad byte preceding the third `NOP`.

Similar to the previous instructions, the argument to `PADDING` may be either `ON` or `OFF`, and the current setting may be read from a symbol with the same name. `PADDING` is by default only enabled for the 680x0 family, it has to be turned on explicitly for all other families.

### PACKING

_valid for: AVR_

In some way, `PACKING` is similar to `PADDING`, it just has a somewhat opposite effect: While `PADDING` extends the disposed data to get full words and keep a possible alignment, `PACKING` squeezes several values into a single word. This makes sense for the AVR's code segment since the CPU has a special instruction (`LPM`) to access single bytes within a 16-bit word. In case this option is turned on (argument `ON`), two byte values are packed into a single word by `DATA`, similar to the single characters of string arguments. The value range of course reduces to -128...+255. If this option is turned off (argument `OFF`), each integer argument obtains its own word and may take values from -32768...+65535.

This distinction is only made for integer arguments of `DATA`, strings will always be packed. Keep further in mind that packing of values only works within the arguments of a `DATA` statement; if one has subsequent `DATA` statements, there will still be half-filled words when the argument count is odd!

### MAXMODE

_valid for: TLCS-900, H8_

The processors of the TLCS-900-family are able to work in 2 modes, the minimum and maximum mode. Depending on the actual mode, the execution environment and the assembler are a little bit different. Along with this instruction and the parameter `ON` or `OFF`, AS is informed that the following code will run in maximum resp. minimum mode. The actual setting can be read from the variable `INMAXMODE`. Presetting is `OFF`, i.e. minimum mode.

Similarly, one uses this instruction to tell AS in H8 mode whether the address space is 64K or 16 Mbytes. This setting is always `OFF` for the 'small' 300 version and cannot be changed.

### EXTMODE and LWORDMODE

_valid for: Z380_

The Z380 may operate in altogether 4 modes, which are the result of setting two flags: The XM flag rules whether the processor shall operate wit an address space of 64 Kbytes or 4 Gbytes and it may only be set to 1 (after a reset, it is set to 0 for compatibility with the Z80). The LW flag in turn rules whether word operations shall work with a word size of 16 or 32 bits. The setting of these two flags influences range checks of constants and addresses, which is why one has to tell AS the setting of these two flags via these instructions. The default assumption is that both flags are 0, the current setting (`ON` or `OFF`) may be read from the predefined symbols `INEXTMODE` resp. `INLWORDMODE.`

### SRCMODE

_valid for: MCS-251_

Intel substantially extended the 8051 instruction set with the 80C251, but unfortunately there was only a single free opcode for all these new instructions. To avoid a processor that will be eternally crippled by a prefix, Intel provided two operating modes: the binary and the source mode. The new processor is fully binary compatible to the 8051 in binary mode, all new instructions require the free opcode as prefix. In source mode, the new instructions exchange their places in the code tables with the corresponding 8051 instructions, which in turn then need a prefix. One has to inform AS whether the processor operates in source mode (`ON`) or binary mode (`OFF`) to enable AS to add prefixes when required. The current setting may be read from the variable `INSRCMODE`. The default is `OFF`.

### BIGENDIAN

_valid for: MCS-51/251, PowerPC, SC/MP_

Intel broke with its own principles when the 8051 series was designed: in contrast to all traditions, the processor uses big-endian ordering for all multi-byte values! While this was not a big deal for MCS-51 processors (the processor could access memory only in 8-bit portions, so everyone was free to use whichever endianess one wanted), it may be a problem for the 251 as it can fetch whole (long-)words from memory and expects the MSB to be first. As this is not the way of constant disposal earlier versions of AS used, one can use this instruction to toggle between big and little endian mode for the instructions `DB, DW, DD, DQ,` and `DT`. `BIGENDIAN OFF` (the default) puts the LSB first into memory as it used to be on earlier versions of AS, `BIGENDIAN ON` engages the big-endian mode compatible to the MCS-251. One may of course change this setting as often as one wants; the current setting can be read from the symbol with the same name.

### WRAPMODE

_valid for: Atmel AVR_

After this switch has been set to `ON`, AS will assume that the processor's program counter does not have the full length of 16 bits given by the architecture, but instead a length that is exactly sufficient to address the internal ROM. For example, in case of the AT90S8515, this means 12 bits, corresponding to 4 Kwords or 8 Kbytes. This assumption allows relative branches from the ROM's beginning to the end and vice versa which would result in an out-of-branch error when using strict arithmetics. Here, they work because the carry bits resulting from the target address computation are discarded. Assure that the target processor you are using works in the outlined way before you enable this option! In case of the aforementioned AT90S8515, this option is even necessary because it is the only way to perform a direct jump through the complete address space...

This switch is set to `OFF` by default, and its current setting may be read from a symbol with same name.

### SEGMENT

_valid for: all processors_

Some microcontrollers and signal processors know various address ranges, which do not overlap with each other and require also different instructions and addressing modes for access. To manage these ones also, the assembler provides various program counters, you can switch among them to and from by the use of the `SEGMENT` instruction. For subroutines included with `INCLUDE`, this e.g. allows to define data used by the main program or subroutines near to the place they are used. In detail, the following segments with the following names are supported:

- `CODE`: program code;
- `DATA`: directly addressable data (including SFRs);
- `XDATA`: data in externally connected RAM or X-addressing space of the DSP56xxx or ROM data for the μPD772x;
- `YDATA`: Y-addressing space of the DSP56xxx;
- `IDATA`: indirectly addressable (internal) data;
- `BITDATA`: the part of the 8051-internal RAM that is bitwise addressable;
- `IO`: I/O-address range;
- `REG`: register bank of the ST9;
- `ROMDATA`: constant ROM of the NEC signal processors;
- `EEDATA`: built-in EEPROM.

See also [ORG](#org) for detailed information about address ranges and initial values of the segments. Depending on the processor family, not all segment types will be permitted.

The bit segment is managed as if it would be a byte segment, i.e. the addresses will be incremented by 1 per bit.

Labels get the same type as attribute as the segment that was active when the label was defined. So the assembler has a limited ability to check whether you access symbols of a certain segment with wrong instructions. In such cases the assembler issues a warning.

Example:

```asm
        CPU     8051    ; MCS-51-code

        segment code    ; test code

        setb    flag    ; no warning
        setb    var     ; warning : wrong segment

        segment data

var     db      ?

        segment bitdata

flag    db      ?
```

### PHASE and DEPHASE

_valid for: all processors_

For some applications (especially on Z80 systems), the code must be moved to another address range before execution. If the assembler didn't know about this, it would align all labels to the load address (not the start address). The programmer is then forced to write jumps within this area either independent of location or has to add the offset at each symbol manually. The first one is not possible for some processors, the last one is extremely error-prone. With the commands `PHASE` and `DEPHASE`, it is possible to inform the assembler at which address the code will really be executed on the target system:

```asm
        phase   <address>
```

informs the assembler that the following code shall be executed at the specified address. The assembler calculates thereupon the difference to the real program counter and adds this difference for the following operations:

- address values in the listing
- filing of label values
- program counter references in relative jumps and address expressions
- readout of the program counter via the symbols \* or $

By using the instruction

```asm
        DEPHASE
```

this "shifting" is reverted to the value previous to the most recent `PHASE` instruction. `PHASE` und `DEPHASE` may be used in a nested manner.

The assembler keeps phase values for all defined segments, although this instruction pair only makes real sense in the code segment.

### SAVE and RESTORE

_valid for: all processors_

The command `SAVE` forces the assembler to push the contents of following variables onto an internal stack:

- currently selected processor type (set by `CPU`);
- currently active memory area (set by `SEGMENT`);
- the flag whether listing is switched on or off (set by `LISTING`);
- the flags that define which part of expanded macros shall be printed in the assembly listing (set by `/MACEXP_DFT/MACEXP_OVR`).
- currently active character translation table (set by `CODEPAGE`).

The counterpart `RESTORE` pops the values saved last from this stack. These two commands were primarily designed for include files, to change the above mentioned variables in any way inside of these files, without loosing their original content. This may be helpful e.g. in include files with own, fully debugged subroutines, to switch the listing generation off:

```asm
        SAVE            ; save old status

        LISTING OFF     ; save paper

        .               ; the actual code
        .

        RESTORE         ; restore
```

In opposite to a simple `LISTING OFF ... ON`-pair, the correct status will be restored, in case the listing generation was switched off already before.

The assembler checks if the number of `SAVE`-and `RESTORE`-commands corresponds and issues error messages in the following cases:

- `RESTORE`, but the internal stack is empty;
- the stack not empty at the end of a pass.

### ASSUME

_valid for: various_

This instruction allows to tell AS the current setting of certain registers whose contents cannot be described with a simple `ON` or `OFF`. These are typically registers that influence addressing modes and whose contents are important to know for AS in order to generate correct addressing. It is important to note that `ASSUME` only informs AS about these, **no** machine code is generated that actually loads these values into the appropriate registers!

A value defined with `ASSUME` can be queried or integrated into expressions via the built-in function `ASSUMEDVAL`. This is the case for all architectures listed in the following sub-sections except for the 8086.

#### 65CE02

The 65CE02 features a a register named 'B' that is used to set the 'base page'. In comparison to the original 6502, this allows the programmer to place the memory page addressable with short (8 bit) addresses anywhere in the 64K address space. This register is set to zero after a reset, so the 65CE02 behaves like its predecessor. A base page at zero is also the default assumption of the assembler. It may be informed about its actual contents via a `ASSUME B:xx` statement. Addresses located in this page will then automatically be addressed via short addressing modes.

#### 6809

In contrast to its 'predecessors' like the 6800 and 6502, the position of the direct page, i.e. the page of memory that can be reached with single-byte addresses, can be set freely. This is done via the 'direct page register' that sets the page number. One has to assign a corresponding value to this register via `ASSUME` is the contents are different from the default of 0, otherwise wrong addresses will be generated!

#### 68HC11K4

Also for the HC11, the designers finally weren't able to avoid the major sin: using a banking scheme to address more than 64 Kbytes with only 16 address lines. The registers `MMSIZ`, `MMWBR`, `MM1CR`, and `MM2CR` control whether and how the additional 512K address ranges are mapped into the physical address space. AS initially assumes the reset state of these registers, i.e. all are set to $00 and windowing is disabled.

#### 68HC12X

Similar to its cousin without the appended 'X', the HC12X supports a short direct addressing mode. In this case however, it can be used to address more than just the first 256 bytes of the address space. The `DIRECT` register specifies which 256 byte page of the address space is addressed by this addressing mode. `ASSUME` is used to tell AS the current value of this register, so it is able to automatically select the most efficient address ing mode when absolute addresses are used. The default is 0, which corresponds to the reset state.

#### 68HC16

The 68HC16 employs a set of bank registers to address a space of 1 Mbyte with its registers that are only 16 bits wide. These registers supply the upper 4 bits. Of these, the EK register is responsible for absolute data accesses (not jumps!). AS checks for each absolute address whether the upper 4 bits of the address are equal to the value of EK specified via `ASSUME`. AS issues a warning if they differ. The default for EK is 0.

#### H8/500

In maximum mode, the extended address space of these processors is addressed via a couple of bank registers. They carry the names DP (registers from 0...3, absolute addresses), EP (register 4 and 5), and TP (stack). AS needs the current value of DP to check if absolute addresses are within the currently addressable bank; the other two registers are only used for indirect addressing and can therefore not be monitored; it is a question of personal taste whether one specifies their values or not. The BR register is in contrast important because it rules which 256-byte page may be accessed with short addresses. It is common for all registers that AS does not assume **any** default value for them as they are undefined after a CPU reset. Everyone who wants to use absolute addresses must therefore assign values to at least DR and DP!

#### MELPS740

Microcontrollers of this series know a "special page" addressing mode for the `JSR` instruction that allows a shorter coding for jumps into the last page of on-chip ROM. The size of this ROM depends of course on the exact processor type, and there are more derivatives than it would be meaningful to offer via the CPU instruction...we therefore have to rely on `ASSUME` to define the address of this page, e.g.

```asm
        ASSUME  SP:$1f
```

in case the internal ROM is 8K.

#### MELPS7700/65816

These processors contain a lot of registers whose contents AS has to know in order to generate correct machine code. These are the registers in question:

| name   | function             | value range | default |
| :----- | :------------------- | :---------- | :------ |
| DT/DBR | data bank            | 0-$ff       | 0       |
| PG/PBR | code Bank            | 0-$ff       | 0       |
| DPR    | directly addr. page  | 0-$ffff     | 0       |
| X      | index register width | 0 or 1      | 0       |
| M      | accumulator width    | 0 or 1      | 0       |

To avoid endless repetitions, see section [MELPS-7700/65816](processor-specific-hints.md#melps-770065816) for instructions how to use these registers. The handling is otherwise similar to the 8086, i.e. multiple values may be set with one instruction and no code is generated that actually loads the registers with the given values. This is again up to the programmer!

#### MCS-196/296

Starting with the 80196, all processors of the MCS-96 family have a register 'WSR' that allows to map memory areas from the extended internal RAM or the SFR range into areas of the register file which may then be accessed with short addresses. If one informs AS about the value of the WSR register, it can automatically find out whether an absolute address can be addressed with a single-byte address via windowing; consequently, long addresses will be automatically generated for registers covered by windowing. The 80296 contains an additional register WSR1 to allow simultaneous mapping of two memory areas into the register file. In case it is possible to address a memory cell via both areas, AS will always choose the way via WSR!

For indirect addressing, displacements may be either short (8 bits, -128 to +127) or long (16 bits). The assembler will automatically use the shortest possible encoding for a given displacement. It is however possible to enforce a 16-bit coding by prefixing the displacement argument with a greater than sign (`>`). Similarly, absolute addresses in the area from 0ff80h to 0ffffh may be reached via a short offset relative to the "null register".

#### 8086

The 8086 is able to address data from all segments in all instructions, but it however needs so-called "segment prefixes" if another segment register than DS shall be used. In addition it is possible that the DS register is adjusted to another segment, e.g. to address data in the code segment for longer parts of the program. As AS cannot analyze the code's meaning, it has to informed via this instruction to what segments the segment registers point at the moment, e.g.:

```asm
        ASSUME  CS:CODE, DS:DATA
```

It is possible to assign assumptions to all four segment registers in this way. This instruction produces **no** code, so the program itself has to do the actual load of the registers with the values.

The usage of this instruction has on the one hand the result that AS is able to automatically put ahead prefixes at sporadic accesses into the code segment, or on the other hand, one can inform AS that the DS-register was modified and you can save explicit `CS:`-instructions.

Valid arguments behind the colon are `CODE`, `DATA` and `NOTHING`. The latter value informs AS that a segment register contains no usable value (for AS). The following values are pre-initialized:

```asm
      CS:CODE, DS:DATA, ES:NOTHING, SS:NOTHING
```

#### XA

The XA family has a data address space of 16 Mbytes, a process however can always address within a 64K segment only that is given by the DS register. One has to inform AS about the current value of this register in order to enable it to check accesses to absolute addresses.

#### 29K

The processors of the 29K family feature a register RBP that allows to protect banks of 16 registers against access from user mode. The corresponding bit has to be set to achieve the protection. `ASSUME` allows to tell AS which value RBP currently contains. AS can warn this way in case a try to access protected registers from user mode is made.

#### 80C166/167

Though none of the 80C166/167's registers is longer than sixteen bits, this processor has 18/24 address lines and can therefore address up to 256Kbytes/16Mbytes. To resolve this contradiction, it neither uses the well-known (and ill-famed) Intel method of segmentation nor does it have inflexible bank registers...no, it uses paging! To accomplish this, the logical address space of 64 Kbytes is split into 4 pages of 16 Kbytes, and for each page there is a page register (named DPP0...DPP3) that rules which of the 16/1024 physical pages shall be mapped to this logical page. AS always tries to present the address space with a size of 256Kbytes/16MBytes in the sight of the programmer, i.e. the physical page is taken for absolute accesses and the setting of bits 14/15 of the logical address is deduced. If no page register fits, a warning is issued. AS assumes by default that the four registers linearly map the first 64 Kbytes of memory, in the following style:

```asm
        ASSUME  DPP0:0,DPP1:1,DPP2:2,DPP3:3
```

The 80C167 knows some additional instructions that can override the page registers' function. The chapter with processor-specific hints describes how these instructions influence the address generation.

Some machine instructions have a shortened form that can be used if the argument is within a certain range:

- `MOV Rn,#<0...15>`
- `ADD/ADDC/SUB/SUBC/CMP/XOR/AND/OR Rn, #<0...7>`
- `LOOP Rn,#<0...15>`

The assembler automatically uses to the shorter coding if possible. If one wants to enforce the longer coding, one may place a 'bigger' character right before the expression (behind the double cross character!). Vice versa, a 'smaller' character can be used to assure the shorter coding is used. In case the operand does not fulfill the range restrictions for the shorter coding, an error is generated. This syntax may also be used for branches and calls which may either have a short displacement or a long absolute argument.

#### TLCS-47

The direct data address space of these processors (it makes no difference whether you address directly or via the HL register) has a size of only 256 nibbles. Because the "better" family members have up to 1024 nibbles of RAM on chip, Toshiba was forced to introduce a banking mechanism via the DMB register. AS manages the data segment as a continuous addressing space and checks at any direct addressing if the address is in the currently active bank. The bank AS currently expects can be set by means of

```asm
        ASSUME  DMB:<0...3>
```

The default value is 0.

#### ST6

The microcontrollers of the ST62 family are able to map a part (64 bytes) of the code area into the data area, e.g. to load constants from the ROM. This means also that at one moment only one part of the ROM can be addressed. A special register rules which part it is. AS cannot check the contents of this register directly, but it can be informed by this instruction that a new value has been assigned to the register. AS then can test and warn if necessary, in case addresses of the code segment are accessed, which are not located in the "announced" window. If, for example, the variable `VARI` has the value 456h, so

```asm
        ASSUME  ROMBASE:VARI>>6
```

sets the AS-internal variable to 11h, and an access to `VARI` generates an access to address 56h in the data segment.

It is possible to assign a simple `NOTHING` instead of a value, e.g. if the bank register is used temporarily as a memory cell. This value is also the default.

The program counter of these controller only has a width of 12 bits. This means that some sort of banking scheme had to be introduced if a device includes more than 4 KBytes of program memory. The banking scheme splits both program space and program memory in pages of 2 KBytes. Page one of the program space always accesses page one of program memory. The `PRPR` register present on such devices selects which page of program memory is accessed via addresses 000h to 7ffh of program space. As an initial approximation, AS regards program space to be linear and of the size of program memory. If a jump or call from page one is made to code in one of the other pages, it checks whether the assumed contents of the `PRPR` register match the destination address. If a jump or call is done from one of the other pages to an address outside of page one, it checks whether the destination address is within the same page. **IMPORTANT**: The program counter itself is only 12 bits wide. It is therefore not possible to jump from one page to another one, without an intermediate step of jumping back to page one. Changing the `PRPR` register while operating outside of page one would result in "pulling out" the code from under one's feet.

#### ST9

The ST9 family uses exactly the same instructions to address code and data area. It depends on the setting of the flag register's DP flag which address space is referenced. To enable AS to check if one works with symbols from the correct address space (this of course **only** works with absolute accesses!), one has to inform AS whether the DP flag is currently 0 (code) or 1 (data). The initial value of this assumption is 0.

#### 78K2

78K2 is an 8/16 bit architecture, which has later been extended to a one-megabyte address space via banking. Banking is realized with the registers PM6 (normal case) resp. P6 (alternate case with `&` as prefix) that supply the missing upper four address bits. At least for absolute addresses, AS can check whether the current, linear 20-bit address is within the given 64K window.

#### 78K3

Processors with a 78K3 core have register banks that consist of 16 registers. These registers may be used via their numbers (`R0` to `R15`) or their symbolic names
(`X=R0`, `A=R1`, `C=R2`, `B=R3`, `VPL=R8`, `VPH=R9`, `UPL=R10`, `UPH=R11`, `E=R12`, `D=R13`, `L=R14`, `H=R15`). The processor core has a register select bit ( `RSS`) to switch the mapping of A/X and B/C from R0...R3 to R4...R7. This is mainly important for instructions that implicitly use one of these registers (i.e. instruction that do not encode the register number in the machine code). However, it is also possible to inform the assembler about the changed mapping via a

```asm
      assume rss:1
```

The assembler will then insert the alternate register numbers into machine instructions that explicitly encode the register numbers. Vice versa, `R5` will be treated like `A` instead of `R1` in the source code.

#### 78K4

78K4 was designed as an 'upgrade path' from 78K3, which is why this processor core contains the same RSS bit to control the mapping of registers AX and BC (though NEC discourages use of it in new code).

Aside from many new instructions and addressing modes, the most significant extension is the larger address space of 16 MBytes, of which only the first MByte may be used for program code. The CPU-internal RAM and all special function registers may be positioned either at the top of the first MByte or the top of the first 64 KByte page. Choice is made via the `LOCATION` machine instruction that either takes a 0 or 15 as argument. Together with remapping RAM and SFRs, the processor also switches the address ranges that may be reached with short (8 bit) addresses. Parallel to using `LOCATION`, one has to inform the assembler about this setting via a `ASSUME LOCATION:...` statement. It will then use short addressing for the proper ranges. The assembler will assume a default of 0 for LOCATION.

#### 320C3x/C4x

As all instruction words of this processor family are only 32 bits long (of which only 16 bits were reserved for absolute addresses), the missing upper 8/16 bits have to be added from the DP register. It is however still possible to specify a full 24/32-bit address when addressing, AS will check then whether the upper 8 bits are equal to the DP register's assumed values. `ASSUME` is different to the `LDP` instruction in the sense that one cannot specify an arbitrary address out of the bank in question, one has to extract the upper bits by hand:

```asm
        ldp     @addr
        assume  dp:addr>>16
        .
        .
        ldi     @addr,r2
```

#### μPD78(C)10

These processors have a register (V) that allows to move the "zero page", i.e. page of memory that is addressable by just one byte, freely in the address space, within page limits. By reasons of comforts you don't want to work with expressions such as

```asm
        inrw    Lo(counter)
```

so AS takes over this job, but only under the premise that it is informed via the `ASSUME`-command about the contents of the V register. If an instruction with short addressing is used, it will be checked if the upper half of the address expression corresponds to the expected content. A warning will be issued if both do not match.

#### 75K0

As the whole address space of 12 bits could not be addressed even by the help of register pairs (8 bits), NEC had to introduce banking (like many others too...): the upper 4 address bits are fetched from the MBS register (which can be assigned values from 0 to 15 by the `ASSUME` instruction), which however will only be regarded if the MBE flag has been set to 1. If it is 0 (default), the lowest and highest 128 nibbles of the address space can be reached without banking. The `ASSUME` instruction is undefined for the 75402 as it contains neither a MBE flag nor an MBS register; the initial values cannot be changed therefore.

#### F<sup>2</sup>MC16L

Similar to many other families of microcontrollers, this family suffers somewhat from its designers miserliness: registers of only 16 bits width are faced with an address space of 24 bits. Once again, bank registers had to fill the gap. In detail, these are PCB for the program code, DTB for all data accesses, ADB for indirect accesses via RW2/RW6, and SSB/USB for the stacks. They may all take values from 0 to 255 and are by default assumed to be 0, with the exception of 0ffh for PCB.

Furthermore, a DPR register exists that specifies which memory page within the 64K bank given by DTB may be reached with 8 bit addresses. The default for DPR is 1, resulting in a default page of 0001xxh when one takes DTB's default into account.

#### MN1613

The MN1613 is an extension of an architecture with 16 bit addresses. The address extension is done by a set of "segment registers" (CSBR, SSBR, TSR0, and TSR1), each of which is four bits wide. The contents of a segment register, left-shifted by 14 bits, is added to the 16 bit addresses. This way, a process may access a memory window of 64 KWords within the address space of 256 KWords. The assembler uses segment register values reported via `ASSUME` to warn whether an absolute address is outside the window defined by the used segment register. If the address is within the window, it will compute the correct t16-bit offset. Naturally, this cannot be done when indirect addressing is used.

### CKPT

_valid for: TI990/12_

Type 12 instructions require a _checkpoint register_ for execution. This register may either be specified explicitly as fourth argument, or a default for all following code may be given via this instruction. If neither a `CKPT` instruction nor an explicit checkpoint register was used, an error is reported. The default of no default register may be restored by using `NOTHING` as argument to `CKPT`.

### EMULATED

_valid for: 29K_

AMD defined the 29000's series exception handling for undefined instructions in a way that there is a separate exception vector for each instruction. This allows to extend the instruction set of a smaller member of this family by a software emulation. To avoid that AS quarrels about these instructions as being undefined, the `EMULATED` instruction allows to tell AS that certain instructions are allowed in this case. The check if the currently set processors knows the instruction is then skipped. For example, if one has written a module that supports 32-bit IEEE numbers and the processor does not have a FPU, one writes

```asm
        EMULATED FADD,FSUB,FMUL,FDIV
        EMULATED FEQ,FGE,FGT,SQRT,CLASS
```

### BRANCHEXT

_valid for: XA_

`BRANCHEXT` with either `ON` or `OFF` as argument tells AS whether short branches that are only available with an 8-bit displacement shall automatically be 'extended', for example by replacing a single instruction like

```asm
        bne     target
```

with a longer sequence of same functionality, in case the branch target is out of reach for the instruction's displacement. For example, the replacement sequence for `bne` would be

```asm
        beq     skip
        jmp     target
skip:
```

In case there is no fitting 'opposite' for an instruction, the sequence may become even longer, e.g. for `jbc`:

```asm
        jbc     dobr
        bra     skip
dobr:   jmp     target
skip:
```

This feature however has the side effect that there is no unambiguous assignment between machine and assembly code any more. Furthermore, additional passes may be the result if there are forward branches. One should therefore use this feature with caution!

### Z80SYNTAX

_valid for: 8008, 8080/8085_

With `ON` as argument, one can optionally write (almost) all 8008/8080 instructions in the form Zilog defined them for the Z80. For instance, you simply use `LD` with self-explaining operands instead of `MVI, LXI, MOV, STA, LDA, SHLD, LHLD, LDAX, STAX` or `SPHL`.

Since some mnemonics have a different meaning in 8008/8080 and Z80 syntax, it is not possible to program in 'Z80 style' all the time, unless the '8080 syntax' is turned off entirely by using `EXCLUSIVE` as argument. The details of this operation mode can be looked up in [8080/8085](processor-specific-hints.md#80808085).

A built-in symbol of same name allows to query the operation mode. The mapping is `0=OFF`, `1=ON`, and `2=EXCLUSIVE`.

### EXPECT and ENDEXPECT

This pair of instructions may be used to frame a piece of code that is _expected_ to trigger one or more error or warning messages. If the errors or warnings (identified by their numbers, see chapter [Error Messages](error-messages.md)) do occur, they are suppressed and assembly continues without any error (naturally, without creating code at the erroneous places). However, if warnings or errors that were expected do not occur, `ENDEXPECT` will emit errors about them. The main usage scenario of these instructions are the self tests in the tests/ subdirectory. For instance, one may check this way if range checking of operands works as expected:

```asm
        cpu      68000
        expect   1320     ; immediate shift only for 1...8
        lsl.l    #10,d0
        endexpect
```

## Data Definitions

The instructions described in this section partially overlap in their functionality, but each processor family defines other names for the same function. To stay compatible with the standard assemblers, this way of implementation was chosen.

If not explicitly mentioned otherwise, all instructions for data deposition (not those for reservation of memory!) allow an arbitrary number of parameters which are being processed from left to right.

### DC\[.Size\]

_valid for: 680x0, M\*Core, 68xx, H8, SH7x00, DSP56xxx, XA, ST7/STM8, MN161x_

This instruction places one or several constants of the type specified by the attribute into memory. The attributes are the same ones as defined in [Format of the Input Files](assembler-usage.md#format-of-the-input-files), and there is additionally the possibility for byte constants to place string constants in memory, like

```asm
String  dc.B "Hello world!\0"
```

The parameter count may be between 1 and 20. A repeat count enclosed in brackets may additionally be prefixed to each parameter; for example, one can for example fill the area up to the next page boundary with zeroes with a statement like

```asm
        dc.b    [(*+255)&$ffffff00-*]0
```

**CAUTION!** This function easily allows to reach the limit of 1 Kbyte of generated code per line!

The assembler can automatically add another byte of data in case the byte sum should become odd, to keep the word alignment. This behaviour may be turned on and off via the `PADDING` instruction.

Decimal floating point numbers stored with this instruction (`DC.P...`) can cover the whole range of extended precision, one however has to pay attention to the detail that the coprocessors currently available from Motorola (68881/68882) ignore the thousands digit of the exponent at the read of such constants!

The default attribute is `W`, that means 16-bit-integer numbers.

For the DSP56xxx, the data type is fixed to integer numbers (an attribute is therefore neither necessary nor allowed), which may be in the range of -8M up to 16M-1. String constants are also allowed, whereby three characters are packed into each word.

Opposed to the standard Motorola assembler, it is also valid to reserve memory space with this statement, by using a question mark as operand. This is an extension added by some third-party suppliers for 68K assemblers, similar to what Intel assemblers provide. However, it should be clear that usage of this feature may lead to portability problems. Furthermore, question marks as operands must not be mixed with 'normal' constants in a single statement.

### DS\[.Size\]

_valid for: 680x0, M\*Core, 68xx, H8, SH7x00, DSP56xxx, XA, ST7/STM8, MN161x_

On the one hand, this instruction enables to reserve memory space for the specified count of numbers of the type given by the attribute. Therefore,

```asm
        DS.B    20
```

for example reserves 20 bytes of memory, but

```asm
        DS.X    20
```

reserves 240 bytes!

The other purpose is the alignment of the program counter which is achieved by a count specification of 0. In this way, with a

```asm
        DS.W    0
```

the program counter will be rounded up to the next even address, with a

```asm
        DS.D    0
```

in contrast to the next double word boundary. Memory cells possibly staying unused thereby are neither zeroed nor filled with NOPs, they simply stay undefined.

The default for the operand length is - as usual - `W`, i.e. 16 bits.

For the 56xxx, the operand length is fixed to words (of 24 bit), attributes therefore do not exist just as in the case of `DC`.

### DN,DB,DW,DD,DQ, and DT

_valid for: Intel (except for 4004/4040), Zilog, Toshiba, NEC, TMS370, Siemens, AMD, MELPS7700/65816, M16(C), National, ST9, Atmel, TMS70Cxx, TMS1000, Signetics, μPD77230, Fairchild, Intersil, XS1_

These commands are - one could say - the Intel counterpart to `DS` and `DC`, and as expected, their logic is a little bit different: First, the specification of the operand length is moved into the mnemonic:

- `DN`: 4-bit integer
- `DB`: byte or ASCII string similar to `DC.B`
- `DW`: 16-bit integer or half precision
- `DD`: 32-bit integer or single precision
- `DQ`: double precision (64 bits)
- `DT`: extended precision (80 bits)

Second, the distinction between constant definition and memory
reservation is done by the operand. A reservation of memory is marked by
a `?` :

```asm
        db      ?       ; reserves a byte
        dw      ?,?     ; reserves memory for 2 words (=4 byte)
        dd      -1      ; places the constant -1 (FFFFFFFFH) !
```

Reserved memory and constant definitions **must not** be mixed within
one instruction:

```asm
        db      "hello",?       ; --> error message
```

Additionally, the `DUP` Operator permits the repeated placing of
constant sequences or the reservation of whole memory blocks:

```asm
        db      3 dup (1,2)     ; --> 1 2 1 2 1 2
        dw      20 dup (?)      ; reserves 40 bytes of memory
```

As you can see, the `DUP`-argument must be enclosed in parentheses, which is also why it may consist of several components, that may themselves be `DUP`s...the stuff therefore works recursively.

`DUP` is however also a place where one can get in touch with another limit of the assembler: a maximum of 1024 bytes of code or data may be generated in one line. This is not valid for the reservation of memory, only for the definition of constant arrays!

The `DUP` operator only gets recognized if it is itself not enclosed in parentheses, and if there is a non-empty argument to its left. This way, it is possible to use a symbol of same name as argument.

In order to be compatible to the M80, `DEFB/DEFW` may be used instead of `DB/DW` in Z80-mode.

Similarly, `BYTE/ADDR` resp. `WORD/ADDRW` in COP4/8 mode are an alias for `DB` resp. `DW`, with the pairs differing in byte order: instructions defined by National for address storage use big endian, `BYTE` resp. `WORD` in contrast use little endian.

If `DB` is used in an address space that is not byte addressable (like the Atmel AVR's `CODE` segment), bytes are packed in pairs into 16 bit words, according to the endianess given by the architecture: for little endian, the LSB is filled first. If the total number of bytes is odd, one half of the last word remains unused, just like the argument list had been padded. It will also not be used if another `DB` immediately follows in source code. The analogous is true for `DN`, just with the difference that two or four nibbles are packet into a byte or 16 bit word.

The NEC 77230 is special with its `DW` instruction: It more works like the `DATA` statement of its smaller brothers, but apart from string and integer arguments, it also accepts floating point values (and stores them in the processor's proprietary 32-bit format). There is _no_ `DUP` operator!

### DS, DS8

_valid for: Intel, Zilog, Toshiba, NEC, TMS370, Siemens, AMD, M16(C), National, ST9, TMS7000, TMS1000, Intersil_

With this instruction, you can reserve a memory area:

```asm
        DS      <count>
```

It is an abbreviation of

```asm
        DB      <count> DUP (?)
```

Although this could easily be made by a macro, some people grown up with Motorola CPUs (Hi Michael!) suggest `DS` to be a built-in instruction...I hope they are satisfied now `;-)`

`DS8` is defined as an alias for `DS` on the National SC14xxx. Beware that the code memory of these processors is organized in words of 16 bits, it is therefore impossible to reserve individual bytes. In case the argument of `DS` is odd, it will be rounded up to the next even number.

### BYT or FCB

_valid for: 6502, 68xx_

By this instruction, byte constants or ASCII strings are placed in 65xx/68xx-mode, it therefore corresponds to `DC.B` on the 68000 or `DB` on Intel. Similarly to `DC`, a repetition factor enclosed in brackets (\[...\]) may be prepended to every single parameter.

### BYTE

_valid for: ST6, 320C2(0)x, 320C5x, MSP, TMS9900, CP-1600_

Ditto. Note that when in 320C2(0)x/5x mode, the assembler assumes that a label on the left side of this instruction has no type, i.e. it belongs to no address space. This behaviour is explained in the processor-specific hints.

The `PADDING` instruction allows to set whether odd counts of bytes shall be padded with a zero byte in MSP/TMS9900 mode.

The operation of `BYTE` on CP-1600 is somewhat different: the 16-bit integer arguments are stored byte-wise in two consecutive words of memory (LSB first). If individual 8-bit values shall be stored in memory (optionally packed), use the `TEXT` instruction!

### DC8

_valid for: SC144xx_

This statement is an alias for `DB`, i.e. it may be used to dump byte constants or strings to memory.

### ADR or FDB

_valid for: 6502, 68xx_

`ADR` resp. `FDB` stores word constants when in 65xx/68xx mode. It is therefore the equivalent to `DC.W` on the 68000 or `DW` on Intel platforms. Similarly to `DC`, a repetition factor enclosed in brackets (\[...\]) may be prepended to every single parameter.

### WORD

_valid for: ST6, i960, 320C2(0)x, 320C3x/C4x/C5x, MSP, CP-1600_

If assembling for the 320C3x/C4x or i960, this command stores 32-bit words, 16-bit words for the other families. Note that when in 320C2(0)x/5x mode, the assembler assumes that a label on the left side of this instruction has no type, i.e. it belongs to no address space. This behaviour is explained at the discussion on processor-specific hints.

### DW16

_valid for: SC144xx_

This instruction is for SC144xx targets a way to dump word (16 bit) constants to memory. `CAUTION!!` It is therefore an alias for `DW`.

### LONG

_valid for: 320C2(0)x, 320C5x_

LONG stores a 32-bit integer to memory with the order LoWord-HiWord. Note that when in 320C2(0)x/5x mode, the assembler assumes that a label on the left side of this instruction has no type, i.e. it belongs to no address space. This behaviour is explained in the processor-specific hints.

### SINGLE, DOUBLE, and EXTENDED

_valid for: 320C3x/C4x (not `DOUBLE`), 320C6x (not `EXTENDED`)_

Both commands store floating-point constants to memory. In case of the 320C3x/C4x, they are **not** stored in IEEE-format. Instead the processor-specific formats with 32 and 40 bit are used. In case of `EXTENDED` the resulting constant occupies two memory words. The most significant 8 bits (the exponent) are written to the first word while the other ones (the mantissa) are copied into the second word.

### FLOAT and DOUBLE

_valid for: 320C2(0)x, 320C5x_

These two commands store floating-point constants in memory using the standard IEEE 32-bit and 64-bit IEEE formats. The least significant byte is copied to the first allocated memory location. Note that when in 320C2(0)x/5x mode the assembler assumes that all labels on the left side of an instruction have no type, i.e. they belong to no address space. This behaviour is explained in the processor-specific hints.

### SINGLE and DOUBLE

_valid for: TMS99xxx_

These two commands store floating-point constants in memory using the processor's floating point format, which is equal to the IBM/360 floating point format.

### EFLOAT, BFLOAT, and TFLOAT

_valid for: 320C2(0)x, 320C5x_

Another three floating point commands. All of them support non-IEEE formats, which should be easily applicable on signal processors:

- `EFLOAT`: mantissa with 16 bits, exponent with 16 bits
- `BFLOAT`: mantissa with 32 bits, exponent with 16 bits
- `DFLOAT`: mantissa with 64 bits, exponent with 32 bits

The three commands share a common storage strategy. In all cases the mantissa precedes the exponent in memory, both are stored as 2's complement with the least significant byte first. Note that when in 320C2(0)x/5x mode the assembler assumes that all labels on the left side of an instruction have no type, i.e. they belong to no address space. This behaviour is explained in the processor-specific hints.

### Qxx and LQxx

_valid for: 320C2(0)x, 320C5x_

`Qxx` and `LQxx` can be used to generate constants in a fixed point format. `xx` denotes a 2-digit number. The operand is first multiplied by 2<sup>_x\*\*x_</sup> before converting it to binary notation. Thus `xx` can be viewed as the number of bits which should be reserved for the fractional part of the constant in fixed point format. `Qxx` stores only one word (16 bit) while `LQxx` stores two words (low word first):

```asm
        q05     2.5     ; --> 0050h
        lq20    ConstPI ; --> 43F7h 0032h
```

Please do not flame me in case I calculated something wrong on my HP28...

### DATA

_valid for: PIC, 320xx, AVR, MELPS-4500, H8/500, HMCS400, 4004/4040, μPD772x, OLMS-40/50, Padauk_

This command stores data in the current segment. Both integer values as well as character strings are supported. On 16C5x/16C8x, 17C4x in data segment, and on the 4500, 4004, and HMCS400 in code segment, characters occupy one word. On AVR, 17C4x in code segment, μPD772x in the data segments, and on 3201x/3202x, in general two characters fit into one word (LSB first). The μPD77C25 can hold three bytes per word in the code segment. When in 320C3x/C4x, mode the assembler puts four characters into one word (MSB first). In contrast to this characters occupy two memory locations in the data segment of the 4500, similar in the 4004 and HMCS400. The range of integer values corresponds to the word width of each processor in a specific segment. This means that `DATA` has the same result than `WORD` on a 320C3x/C4x (and that of `SINGLE` if AS recognizes the operand as a floating-point constant).

### ZERO, CP-1600

_valid for: PIC_

Generates a continuous string of zero words in memory (which equals a NOP on PIC).

### FB and FW

_valid for: COP4/8_

These instruction allow to fill memory blocks with a byte or word constant. The first operand specifies the size of the memory block while the second one sets the filling constant itself.

### ASCII and ASCIZ

_valid for: ST6_

Both commands store string constants to memory. While `ASCII` writes the character information only, `ASCIZ` additionally appends a zero to the end of the string.

### STRING and RSTRING

_valid for: 320C2(0)x, 320C5x_

These commands are functionally equivalent to `DATA`, but integer values are limited to the range of byte values. This enables two characters or numbers to be packed together into one word. Both commands only differ in the order they use to write bytes: `STRING` stores the upper one first then the lower one, `RSTRING` does this vice versa. Note that when in 320C2(0)x/5x mode the assembler assumes that a label on the left side of this instruction has no type, i.e. it belongs to no address space. This behaviour is explained in the processor-specific hints.

### FCC

_valid for: 6502, 68xx_

When in 65xx/68xx mode, string constants are generated using this instruction. In contrast to the original assembler AS11 from Motorola (this is the main reason why AS understands this command, the functionality is contained within the `BYT` instruction) you must enclose the string argument by double quotation marks instead of single quotation marks or slashes. Similarly to `DC`, a repetition factor enclosed in brackets (\[...\]) may be prepended to every single parameter.

### TEXT

In CP-1600 mode, This instruction is used to store string constants in packed format, i.e. two characters per word.

### DFS or RMB

_valid for: 6502, 68xx_

Reserves a memory block when in 6502/68xx mode. It is therefore the equivalent to `DS.B` on the 68000 or `DB ?` on Intel platforms.

### BLOCK

_valid for: ST6_

Ditto.

### SPACE

_valid for: i960_

Ditto.

### RES

_valid for: PIC, MELPS-4500, HMCS400, 3201x, 320C2(0)x, 320C5x, AVR, μPD772x, OLMS-40/50, Padauk, CP-1600_

This command allocates memory. When used in code segments the argument counts words (10/12/14/16 bit). In data segments it counts bytes for PICs, nibbles for 4500 and OLMS-40/50 and words for the TI devices.

### BSS

_valid for: 320C2(0)x, 320C3x/C4x/C5x/C6x, MSP_

`BSS` works like `RES`, but when in 320C2(0)x/5x mode, the assembler assumes that a label on the left side of this instruction has no type, i.e it belongs to no address space. This behaviour is explained in the processor-specific hints.

### DSB and DSW

_valid for: COP4/8_

Both instructions allocate memory and ensure compatibility to ASMCOP from National. While `DSB` takes the argument as byte count, `DSW` uses it as word count (thus it allocates twice as much memory than `DSB`).

### DS16

_valid for: SC144xx_

This instruction reserves memory in steps of full words, i.e. 16 bits. It is an alias for `DW`.

### ALIGN

_valid for: all processors_

Takes the argument to align the program counter to a certain address boundary. AS increments the program counter to the next multiple of the argument. So, `ALIGN` corresponds to `DS.x` on 68000, but is much more flexible at the same time.

Example:

```asm
        align     2
```

aligns to an even address (PC mod 2 = 0). If Align is used in this form with only one argument, the contents of the skipped memory space is not defined. An optional second argument may be used to define the (byte) value used to fill the area.

### LTORG

_valid for: SH7x00_

Although the SH7000 processor can do an immediate register load with 8 bit only, AS shows up with no such restriction. This behaviour is instead simulated through constants in memory. Storing them in the code segment (not far away from the register load instruction) would require an additional jump. AS Therefore gathers the constants an stores them at an address specified by `LTORG`. Details are explained in the processor-specific section somewhat later.

## Macro Instructions

_valid for: all processors_

Now we finally reach the things that make a macro assembler different from an ordinary assembler: the ability to define macros (guessed it !?).

When speaking about 'macros', I generally mean a sequence of (machine or pseudo) instructions which are united to a block by special statements and can then be treated in certain ways. The assembler knows the following statements to work with such blocks:

### MACRO

is probably the most important instruction for macro programming. The instruction sequence

```asm
<name>  MACRO   [parameter list]
        <instructions>
        ENDM
```

defines the macro `<name>` to be the enclosed instruction sequence. This definition by itself does not generate any code! In turn, from now on the instruction sequence can simply be called by the name, the whole construct therefore shortens and simplifies programs. A parameter list may be added to the macro definition to make things even more useful. The parameters' names have to be separated by commas (as usual) and have to conform to the conventions for symbol names (see [Symbol Conventions](assembler-usage.md#symbol-conventions)) - like the macro name itself.

**Note:** AS defines `ENDR` as a synonym of `ENDM`, which can be used whenever `ENDM` is used.

A switch to case-sensitive mode influences both macro names and parameters.

Similar to symbols, macros are local, i.e. they are only known in a section and its subsections when the definition is done from within a section. This behaviour however can be controlled in wide limits via the options `PUBLIC` and `GLOBAL` described below.

A default value may be provided for each macro parameter (appended via an equal sign). This value is used if there is no argument for this parameter at macro call or if the positional argument (see below) for this parameter is empty.

Apart from the macro parameters themselves, the parameter list may contain control parameters which influence the processing of the macro. These parameters are distinguished from normal parameters by being enclosed in braces. The following control parameters are defined:

- `EXPAND/NOEXPAND`: rule whether the enclosed code shall be written to the listing when the macro is expanded. The default is the value set by the pseudo instruction `MACEXP_DFT`.
- `EXPIF/NOEXPIF`: rule whether instructions for conditional assembly and code excluded by it shall be written to the listing when the macro is expanded. The default is the value set by the pseudo instruction `MACEXP_DFT`.
- `EXPMACRO/NOEXPMACRO`: rule whether macros defined in the macro's body shall be written to the listing when the macro is expanded. The default is the value set by the pseudo instruction `MACEXP_DFT`.
- `EXPREST/NOEXPREST` : rule whether a macro body's lines not fitting into the first two categories shall be written to the listing when the macro is expanded. The default is the value set by the pseudo instruction `MACEXP_DFT`.
- `PUBLIC[:section name]`: assigns the macro to a parent section instead of the current section. A section can make macros accessible for the outer code this way. If the section specification is missing, the macro becomes completely global, i.e. it may be referenced from everywhere.
- `GLOBAL[:section name]`: rules that in addition to the macro itself, another macro shall be generated that has the same contents but is assigned to the specified section. Its name is constructed by concatenating the current section's name to the macro name. The section specified must be a parent section of the current section; if the specification is missing, the additional macro becomes globally visible. For example, if a macro `A` is defined in a section `B` that is a child section of section `C`, an additional global macro named `C_B_A` would be generated. In contrast, if `C` had been specified as target section, the macro would be named `B_A` and be assigned to section `C`. This option is turned off by default and it only has an effect when it is used from within a section. The macro defined locally is not influenced by this option.
- `EXPORT/NOEXPORT`: rules whether the definition of this macro shall be written to a separate file in case the `-M` command line option was given. This way, definitions of 'private' macros may be mapped out selectively. The default is FALSE, i.e. the definition will not be written to the file. The macro will be written with the concatenated name if the `GLOBAL` option was additionally present.
- `INTLABEL/NOINTLABEL` : rules whether a label defined in a line that calls this macro may be used as an additional parameter inside the label or not, instead of simply 'labeling' the line.
- `GLOBALSYMBOLS/NOGLOBALSYMBOLS` : rules whether labels defined in the macro's body shall be local to this macro or also be available outside the macro. The default is to keep them local, since using a macro multiple time would be difficult otherwise.

The control parameters described above are removed from the parameter list by AS, i.e. they do not have a further influence on processing and usage.

When a macro is called, the parameters given for the call are textually inserted into the instruction block and the resulting assembler code is assembled as usual. Zero length parameters are inserted in case too few parameters are specified. It is important to note that string constants are not protected from macro expansions. The old IBM rule:

> _It's not a bug, it's a feature!_

applies for this detail. The gap was left to allow checking of parameters via string comparisons. For example, one can analyze a macro parameter in the following way:

```asm
mul     MACRO   para,parb
        IF      UpString("PARA")<>"A"
        MOV    a,para
        ENDIF
        IF      UpString("PARB")<>"B"
        MOV    b,parb
        ENDIF
        !mul     ab
        ENDM
```

It is important for the example above that the assembler converts all parameter names to upper case when operating in case-insensitive mode, but this conversion never takes place inside of string constants. Macro parameter names therefore have to be written in upper case when they appear in string constants.

Macro arguments may be given in either of two forms: _positional_ or _keyword_ arguments.

For positional arguments, the assignment of arguments to macro parameters simply results from the position of arguments, i.e. the first argument is assigned to the first parameter, the second argument to the second parameter and so on. If the number of arguments is smaller than the number of parameters, eventually defined default values or simply an empty string are inserted. The same is valid for empty arguments in the argument list.

Keyword arguments on the other hand explicitly define which parameter they relate to, by being prefixed with the parameter's name:

```asm
        mul  para=r0,parb=r1
```

Again, non-assigned parameters will use an eventually defined default or an empty string.

As a difference to positional arguments, keyword arguments allow to assign an empty string to a parameter with a non-empty default.

Mixing of positional and keyword arguments in one macro call is possible, however it is not allowed to use positional arguments after the first keyword argument.

The same naming rules as for usual symbols also apply for macro parameters, with the exception that only letters and numbers are allowed, i.e. dots and underscores are forbidden. This constraint has its reason in a hidden feature: the underscore allows to concatenate macro parameter names to a symbol, like in the following example:

```asm
concat  macro   part1,part2
        call    part1_part2
        endm
```

The call

```asm
        concat  module,function
```

will therefore result in

```asm
        call    module_function
```

Additionally, parameter names surrounded by `\\` will be replaced as one. For example:

```asm
concat  macro   part1,part2
        call    \part1\\part2\
        endm
```

The call

```asm
        concat  module,function
```

will therefore result in

```asm
        call    modulefunction
```

Apart from the parameters explicitly declared for a macro, four more 'implicitly' declared parameters exist. Since they are always present, they cannot not be redeclared as explicit parameters:

- `ATTRIBUTE` refers to the attribute appended to the macro call, in case the currently active architecture supports attributes for machine instructions. See below for an example!
- `ALLARGS` refers to a comma-separated list of all arguments passed to a macro, usable e.g. to pass them on to a IRP statement.
- `ARGCOUNT` refers to the actual count of parameters passed to a macro. Note however that this number is never lower than the formal parameter count of the macro, since AS will fill up missing arguments with empty strings!
- `__LABEL__` refers to a label present in a line that calls the macro. This replacement only takes place if the `INTLABEL` option was set for this macro!

**IMPORTANT:** the names of these implicit parameters are also case-insensitive if AS was told to operate case-sensitive!

The purpose of being able to 'internally' use a label in a macro is surely not immediately obvious. There might be cases where moving the macro's entry point into its body may be useful. The most important application however are TI signal processors that use a double pipe symbol in the label's column to mark parallelism, like this:

```asm
        instr1
    ||  instr2
```

(since both instructions merge into a single word of machine code, you cannot branch to the second instruction - so occupying the label's position doesn't hurt). The problem is however that some 'convenience instructions' are realized as macros. A parallelization symbol written in front of a macro call normally would be assigned to the macro itself, _not to the macro body's first instruction_. However, things work with this trick:

```asm
myinstr macro {INTLABEL}
__LABEL__
        instr2
        endm

        instr1
||      myinstr
```

The result after expanding `myinstr` is identical to the previous example without macro.

Recursion of macros, i.e. the repeated call of a macro from within its own body is completely legal. However, like for any other sort of recursion, one has to assure that there is an end at someplace. For cases where one forgot this, AS keeps an internal counter for every macro that is incremented when an expansion of this macro is begun and decremented again when the expansion is completed. In case of recursive calls, this counter reaches higher and higher values, and at a limit settable via `NESTMAX`, AS will refuse to expand. Be careful when you turn off this emergency brake: the memory consumption on the heap may go beyond all limits and even shut down a Unix system...

A small example to remove all clarities ;-)

A programmer brain damaged by years of programming Intel processors wants to have the instructions `PUSH/POP` also for the 68000. He solves the 'problem' in the following way:

```asm
push    macro   op
        move.ATTRIBUTE op,-(sp)
        endm

pop     macro   op
        move.ATTRIBUTE (sp)+,op
        endm
```

If one writes

```asm
        push    d0
        pop.l   a2
```

this results in

```asm
        move.   d0,-(sp)
        move.l  (sp)+,a2
```

A macro definition must not cross include file boundaries.

Labels defined in macros always are regarded as being local, unless the `GLOBALSYMBOLS` was used in the macro's definition. If a single label shall be made public in a macro that uses local labels otherwise, it may be defined with a `LABEL` statement which always creates global symbols (similar to `BIT, SFR...`):

```asm
<Name>  label   $
```

When parsing a line, the assembler first checks the macro list afterwards looks for processor instructions, which is why macros allow to redefine processor instructions. However, the definition should appear previously to the first invocation of the instruction to avoid phase errors like in the following example:

```asm
        bsr     target

bsr     macro   targ
        jsr     targ
        endm

        bsr     target
```

In the first pass, the macro is not known when the first `BSR` instruction is assembled; an instruction with 4 bytes of length is generated. In the second pass however, the macro definition is immediately available (from the first pass), a `JSR` of 6 bytes length is therefore generated. As a result, all labels following are too low by 2 and phase errors occur for them. An additional pass is necessary to resolve this.

Because a machine or pseudo instruction becomes hidden when a macro of same name is defined, there is a backdoor to reach the original meaning: the search for macros is suppressed if the name is prefixed with an exclamation mark (!). This may come in handy if one wants to extend existing instructions in their functionality, e.g. the TLCS-90's shift instructions:

```asm
srl     macro   op,n            ; shift by n places
        rept    n               ; n simple instructions
        !srl   op
        endm
        endm
```

From now on, the `SRL` instruction has an additional parameter...

#### Macro Expansion in the Listing

If a macro is being called, the macro's body is included in the assembly listing, after arguments have been expanded. This can significantly increase the listing's size and make it hard to read. It is therefore possible to suppress this expansion totally or in parts. Fundamentally, AS divides the source lines contained in a macro's body into three classes:

- Macro definitions, i.e. the macro is used to define another macro, or it contains `REPT/IRP/IRPC/WHILE` blocks.
- Instructions for conditional assembly plus any source lines that have _not_ been assembled due to conditional assembly. Since conditional assembly may depend on macro arguments, this subset may also vary.
- All remaining source lines that do not fall under the first two categories.

Which parts occur in the listing may be defined individually for every macro. When defining a macro, the default is the set defined by the most recent `MACEXP_DFT` instruction ([MACEXP_DFT and MACEXP_OVR](#macexp_dft-and-macexp_ovr)). If one of the `EXPAND/NOEXPAND`, `EXPIF/NOEXPIF` `EXPMACRO/NOEXPMACRO`, or `EXPREST/NOEXPREST` directives is used in the macro's definition, they act _additionally_, but with higher preference. For instance, if expansion had been disabled completely (`MACEXP_DFT OFF`), adding the directive `EXPREST` has the effect that when using this macro, only lines are written to the listing that remain after conditional assembly and are no macro definitions themselves.

In consequence, changing the set via `MACEXP_DFT` has no effect on macros that have been _defined before_ this statement. The listing's section shows for defined macros the effective set of expansion directives. The list given in curly braces is shortened so that it only contains the last (and therefore valid) directive for a certain class of source lines. A `NOIF` given via `MACEXP_DFT` will therefore not show up if the directive `EXPIF` had been given specifically for this macro.

There might be cases where it is useful to override the expansion rules for a certain macro, regardless whether they were given by `MACEXP_DFT` or individual directives. The statement `MACEXP_OVR` ([MACEXP_DFT and MACEXP_OVR](#macexp_dft-and-macexp_ovr)) exists for such cases. It only has an effects on macros subsequently being _expanded_. Once again, directives given by this instruction are regarded in addition to a macro's rules, and they do with higher priority. A `MACEXP_OVR` without any arguments disables such an override.

### IRP

is a simplified macro definition for the case that an instruction sequence shall be applied to a couple of operands and the the code is not needed any more afterwards. `IRP` needs a symbol for the operand as its first parameter, and an (almost) arbitrary number of parameters that are sequentially inserted into the block of code. For example, one can write

```asm
        irp     op, acc,b,dpl,dph
        push    op
        endm
```

to push a couple of registers to the stack, what results in

```asm
        push    acc
        push    b
        push    dpl
        push    dph
```

Analog to a macro definition, the argument list may contain the control parameters `GLOBALSYMBOLS` resp. `NOGLOBALSYMBOLS` (marked as such by being enclosed in curly braces). This allows to control whether used labels are local for every pass or not.

### IRPN

is similar to `IRP`, but it uses a specifiable amount of arguments each expansion. `IRPN` needs a number of arguments per iteration as its first parameter (lets call this `count`), `count` symbol names, and an (almost) arbitrary number of parameters that are sequentially inserted in batches of `count` parameters into the block of code. For example, one can write

```asm
        irpn 2,addr,pos,label1,0,label2,16,label3,27
        dc.l addr
        dc.w $C000 + pos
        endm
```

to define the following table

```asm
        dc.l label1
        dc.w $C000
        dc.l label2
        dc.w $C010
        dc.l label3
        dc.w $C01B
```

Analog to a macro definition, the argument list may contain the control parameters `GLOBALSYMBOLS` resp. `NOGLOBALSYMBOLS` (marked as such by being enclosed in curly braces). This allows to control whether used labels are local for every pass or not.

### IRPC

`IRPC` is a variant of `IRP` where the first argument's occurrences in the lines up to `ENDM` are successively replaced by the characters of a string instead of further parameters. For example, an especially complicated way of placing a string into memory would be:

```asm
        irpc    char,"Hello World"
        db      'CHAR'
        endm
```

**CAUTION!** As the example already shows, `IRPC` only inserts the pure character; it is the programmer's task to assure that valid code results (in this example by inserting quotes, including the detail that no automatic conversion to uppercase characters is done).

### REPT

is the simplest way to employ macro constructs. The code between `REPT` and `ENDM` is assembled as often as the integer argument of `REPT` specifies. This statement is commonly used in small loops to replace a programmed loop to save the loop overhead.

An example for the sake of completeness:

```asm
        rept    3
        rr      a
        endm
```

rotates the accumulator to the right by three digits.

The optional control parameters `GLOBALSYMBOLS` resp. `NOGLOBALSYMBOLS` (marked as such by being enclosed in curly braces) may also be used here to decide whether labels are local to the individual repetitions.

In case `REPT`'s argument is equal to or smaller than 0, no expansion at all is done. This is different to older versions of AS which used to be a bit 'sloppy' in this respect and always made a single expansion.

### WHILE

`WHILE` operates similarly to `REPT`, but the fixed number of repetitions given as an argument is replaced by a boolean expression. The code framed by `WHILE` and `ENDM` is assembled until the expression becomes logically false. This may mean in the extreme case that the enclosed code is not assembled at all in case the expression was already false when the construct was found. On the other hand, it may happen that the expression stays true forever and AS will run infinitely...one should apply therefore a bit of accuracy when one uses this construct, i.e. the code must contain a statement that influences the condition, e.g. like this:

```asm
cnt     set     1
sq      set     cnt*cnt
        while   sq<=1000
        dc.l    sq
cnt     set     cnt+1
sq      set     cnt*cnt
        endm
```

This example stores all square numbers up to 1000 to memory.

Currently there exists a little ugly detail for `WHILE`: an additional empty line that was not present in the code itself is added after the last expansion. This is a 'side effect' based on a weakness of the macro processor and it is unfortunately not that easy to fix. I hope no one minds...

### EXITM

`EXITM` offers a way to terminate a macro expansion or one of the instructions `REPT, IRP,` or `WHILE` prematurely. Such an option helps for example to replace encapsulations with `IF-ENDIF`-ladders in macros by something more readable. Of course, an `EXITM` itself always has to be conditional, what leads us to an important detail: When an `EXITM` is executed, the stack of open `IF` and `SWITCH` constructs is reset to the state it had just before the macro expansion started. This is imperative for conditional `EXITM`'s as the `ENDIF` resp. `ENDCASE` that frames the `EXITM` statement will not be reached any more; AS would print an error message without this trick. Please keep also in mind that `EXITM` always only terminates the innermost construct if macro constructs are nested! If one want to completely break out of a nested construct, one has to use additional `EXITM`'s on the higher levels!

### SHIFT

`SHIFT` is a tool to construct macros with variable argument lists: it discards the first parameter, with the result that the second parameter takes its place and so on. This way one could process a variable argument list...if you do it the right way. For example, the following does not work...

```asm
pushlist  macro reg
        rept  ARGCOUNT
        push  reg
        shift
        endm
        endm
```

...because the macro gets expanded `once`, its output is captured by `REPT` and then executed n times. Therefore, the first argument is saved n times...the following approach works better:

```asm
pushlist  macro reg
        if      "REG"<>""
        push    reg
        shift
        pushlist ALLARGS
        endif
        endm
```

Effectively, this is a recursion that shortens the argument list once per step. The important trick is that a new macro expansion is started in each step... An alternative to recursion is to use `IRP` or `IRPN` instead of `REPT`:

```asm
pushlist  macro reg
        irp  reg, ALLARGS
        push  reg
        endm
        endm
```

In case `SHIFT` ist already a machine instruction for a certain target, one has to use `SHIFT` instead.

### MAXNEST

`MAXNEST` allows to adjust how often a macro may be called recursively before AS terminates with an error message. The argument may be an arbitrary positive integer value, with the special value 0 turning the this security brake completely off (be careful with that...). The default value for the maximum nesting level is 256; its current value may be read from the integer symbol of same name.

### FUNCTION

Though `FUNCTION` is not a macro statement in the inner sense, I will describe this instruction at this place because it uses similar principles like macro replacements.

This instruction is used to define new functions that may then be used in formula expressions like predefined functions. The definition must have the following form:

```asm
<name>  FUNCTION <arg>,...,<arg>,<expression>
```

The arguments are the values that are 'fed into' the function. The definition uses symbolic names for the arguments. The assembler knows by this that where to insert the actual values when the function is called. This can be seen from the following example:

```asm
isdigit FUNCTION ch,(ch>='0')&&(ch<='9')
```

This function checks whether the argument (interpreted as a character) is a number in the currently valid character set (the character set can be modified via `CHARSET`, therefore the careful wording).

The arguments' names (`CH` in this case) must conform to the stricter rules for macro parameter names, i.e. the special characters . and \_ are not allowed.

User-defined functions can be used in the same way as builtin functions, i.e. with a list of parameters, separated by commas, enclosed in parentheses:

```asm
        IF isdigit(char)
        message "\{char} is a number"
        ELSEIF
        message "\{char} is not a number"
        ENDIF
```

When the function is called, all parameters are calculated once and are then inserted into the function's formula. This is done to reduce calculation overhead and to avoid side effects. The individual arguments have to be separated by commas when a function has more than one parameter.

**CAUTION!** Similar to macros, one can use user-defined functions to override builtin functions. This is a possible source for phase errors. Such definitions therefore should be done before the first call!

The result's type may depend on the type of the input arguments as the arguments are textually inserted into the function's formula. For example, the function

```asm
double  function x,x+x
```

may have an integer, a float, or even a string as result, depending on the argument's type!

When AS operates in case-sensitive mode, the case matters when defining or referencing user-defined functions, in contrast to builtin functions!

## Structures

_valid for: all processors_

Even in assembly language programs, there is sometimes the necessity to define composed data structures, similar to high-level languages. AS supports both the definition and usage of structures with a couple of statements. These statements shall be explained in the following section.

### Definition

The definition of a structure is begun with the statement `STRUCT` and ends with `ENDSTRUCT` (lazy people may also write `STRUC` resp. `ENDSTRUC` or `ENDS` instead). A optional label preceding these instructions is taken as the name of the structure to be defined; it is optional at the end of the definition and may be used to redefine the length symbol's name (see below). The remaining procedure is simple: Together with `STRUCT`, the current program counter is saved and reset to zero. All labels defined between `STRUCT` and `ENDSTRUCT` therefore are the offsets of the structure's data fields. Reserving space is done via the same instructions that are also otherwise used for reserving space, like e.g. `DS.x` for Motorola CPUs or `DB` & co. for Intel-style processors. The rules for rounding up lengths to assure certain alignments also apply here - if one wants to define 'packed' structures, a preceding `PADDING OFF` may be necessary. Vice versa, alignments may be forced with `ALIGN` or similar instructions.

Since such a definition only represents a sort of 'prototype', only instructions that reserve memory may be used, no instructions that dispose constants or generate code.

Labels defined inside structures (i.e. the elements' names) are not stored as-is. Instead, the structure's name is prepended to them, separated with a special character. By default, this is the underline (\_). This behaviour however may be modified with two arguments passed to the `STRUCT` statement:

- `NOEXTNAMES` suppressed the prepending of the structure's name. In this case, it is the programmer's responsibility to assure that field names are not used more than once.
- `DOTS` instructs AS to use the dot as connecting character instead of the underline. It should however be pointed out that on certain target architectures, the dot has a special meaning for bit addressing, which may lead to problems!

It is futhermore possible to turn the usage of a dot on resp. off for all following structures:

```asm
dottedstructs <on|off>
```

Aside from the element names, AS also defines a further symbol with the structure's overall length when the definition has been finished. This symbol has the name `LEN`, which is being extended with the structure's name via the same rules - or alternatively with the label name given with the `ENDSTRUCT` statement.

In practice, this may things may look like in this example:

```asm
Rec     STRUCT
Ident   db      ?
Pad     db      ?
Pointer dd      ?
Rec     ENDSTRUCT
```

In this example, the symbol `REC_LEN` would be assigned the value 6.

### Usage

Once a structure has been assigned, usage is as simple as possible and similar to a macro: a simple

```asm
thisrec Rec
```

reserves as much memory as needed to hold an instance of the structure, and additionally defines a symbol for every element of the structure with its address, in this case `THISREC_IDENT, THISREC_PAD`, and `THISREC_POINTER`. A label naturally must not be omitted when calling a structure; if it is missing, an error will be emitted.

Additional arguments allow to reserve memory for a whole array of structures. The dimensions (up to three) are defined via arguments in square brackets:

```asm
thisarray Rec [10],[2]
```

In this example, space for 2 \* 10 = 20 structures is reserved. For each individual structure in the array, proper symbols are generated that have the array indices in their name.

### Nested Structures

Is is perfectly valid to call an already defined structure within the definition of another structure. The procedure that is taking place then is a combination of the definition and calling described in the previous two sections: elements of the substructure are being defined, the name of the instance is being prepended, and the name of the super-structure is once again getting prepended to this concatenated name. This may look like the following:

```asm
TreeRec struct
left    dd         ?
right   dd         ?
data    Rec
TreeRec endstruct
```

It is also allowed to define one structure inside of another structure:

```asm
TreeRec struct
left    dd         ?
right   dd         ?
TreeData struct
name    db         32 dup(?)
id      dw         ?
TreeData endstruct
TreeRec endstruct
```

### Unions

A union is a special form of a structure whose elements are not laid out sequentially in memory. Instead all elements occupy the _same_ memory and are located at offset 0 in the structure. Naturally, such a definition basically does nothing more than to assign the value of zero to a couple of symbols. It may however be useful to clarify the overlap in a program and therefore to make it more 'readable'. The size of a union is the maximum of all elements' lengths.

### Nameless Structures

The name of a structure or union is optional if it is part of another (named) structure or union. Elements of this structure will then become part of of the 'next higher' named structure. For example,

```asm
TreeRec struct
left    dd         ?
right   dd         ?
        struct
name    db         32 dup(?)
id      dw         ?
        endstruct
TreeRec endstruct
```

generates the symbols `TREEREC_NAME` and _TREEREC_ID_.

Futhermore, no symbol holding its length is generated for an unnamed structure or union.

### Structures and Sections

Symbols that are created in the course of defining or usage of structures are treated just like normal symbols, i.e. when used within a section, these symbols are local to the section. The same is however also true for the structures themselves, i.e. a structure defined within a section cannot be used outside of the section.

### Structures and Macros

If one wants to instantiate structures via macros, one has to use the `GLOBALSYMBOLS` options when defining the macro to make the defined symbols visible outside the macro. For instance, a list of structures can be defined in the following way:

```asm
        irp     name,{GLOBALSYMBOLS},rec1,rec2,rec3
name    Rec
        endm
```

## Conditional Assembly

_valid for: all processors_

The assembler supports conditional assembly with the help of statements like `IF...` or `SWITCH...`. These statements work at assembly time, allowing or disallowing the assembly of program parts based on conditions. They are therefore not to be compared with IF statements of high-level languages (though it would be tempting to extend assembly language with structurization statements of higher-level languages...).

The following constructs may be nested arbitrarily (until a memory overflow occurs).

### IF / ELSEIF / ENDIF

`IF` is the most common and most versatile construct. The general style of an `IF` statement is as follows:

```asm
        IF      <expression 1>
        .
        .
        <block 1>
        .
        .
        ELSEIF  <expression 2>
        .
        .
        <block 2>
        .
        .
        (possibly more ELSEIFs)

        .
        .
        ELSEIF
        .
        .
        <block n>
        .
        .
        ENDIF
```

`IF` serves as an entry, evaluates the first expression, and assembles block 1 if the expression is true (i.e. not 0). All further `ELSEIF`-blocks will then be skipped. However, if the expression is false, block 1 will be skipped and expression 2 is evaluated. If this expression turns out to be true, block 2 is assembled. The number of `ELSEIF` parts is variable and results in an `IF-THEN-ELSE` ladder of an arbitrary length. The block assigned to the last `ELSEIF` (without argument) only gets assembled if all previous expressions evaluated to false; it therefore forms a 'default' branch. It is important to note that only **one** of the blocks will be assembled: the first one whose `IF/ELSEIF` had a true expression as argument.

The `ELSEIF` parts are optional, i.e. `IF` may directly be followed by an `ENDIF`. An `ELSEIF` without parameters must be the last branch.

`ELSEIF` always refers to the innermost, unfinished `IF` construct in case `IF`'s are nested.

In addition to `IF`, the following further conditional statements are defined:

- `IFDEF <symbol>`: true if the given symbol has been defined. The definition has to appear before `IFDEF`.
- `IFNDEF <symbol>`: counterpart to `IFDEF`.
- `IFUSED <symbol>`: true if if the given symbol has been referenced at least once up to now.
- `IFNUSED <symbol>`: counterpart to `IFUSED`.
- `IFEXIST <name>`: true if the given file exists. The same rules for search paths and syntax apply as for the `INCLUDE` instruction (see section [INCLUDE](#include)).
- `IFNEXIST <name>`: counterpart to `IFEXIST`.
- `IFB <arg-list>`: true if all arguments of the parameter list are empty strings.
- `IFNB <arg-list>`: counterpart to `IFB`.

It is valid to write `ELSE` instead of `ELSEIF` since everybody seems to be used to it...

For every IF... statement, there has to be a corresponding `ENDIF`. 'Open' constructs will lead to an error message at the end of an assembly path. The way AS has 'paired' `ENDIF` statements with `IF`s may be deduced from the assembly listing: for `ENDIF`, the line number of the corresponding `IF...` will be shown.

### SWITCH / CASE / ELSECASE / ENDCASE

`CASE` is a special case of `IF` and is designed for situations when an expression has to be compared with a couple of values. This could of course also be done with a series of `ELSEIF`s, but the following form

```asm
        SWITCH  <expression>
        .
        .
        CASE    <value 1>
        .
        <block 1>
        .
        CASE    <value 2>
        .
        <block 2>
        .
        (further CASE blocks)
        .
        CASE    <value n-1>
        .
        <block n-1>
        .
        ELSECASE
        .
        <block n>
        .
        ENDCASE
```

has the advantage that the expression is only written once and also only gets evaluated once. It is therefore less error-prone and slightly faster than an `IF` chain, but obviously not as flexible.

It is possible to specify multiple values separated by commas to a `CASE` statement in order to assemble the following block in multiple cases. The `ELSECASE` branch again serves as a 'trap' for the case that none of the `CASE` conditions was met. AS will issue a warning in case it is missing and all comparisons fail.

Even when value lists of `CASE` branches overlap, only **one** branch is executed, which is the first one in case of ambiguities.

`SWITCH` only serves to open the whole construct; an arbitrary number of statements may be between `SWITCH` and the first `CASE` (but don't leave other `IF`s open!), for the sake of better readability this should however not be done.

In case that `SWITCH` is already a machine instruction on the selected processor target, the construct is started instead with `SELECT`.

Similarly to `IF` constructs, there must be exactly one `ENDCASE` for every `SWITCH`. Analogous to `ENDIF`, for `ENDCASE` the line number of the corresponding `SWITCH` is shown in the listing.

## Listing Control

_valid for: all processors_

### PAGE

`PAGE` is used to tell AS the dimensions of the paper that is used to print the assembly listing. The first parameter is thereby the number of lines after which AS shall automatically output a form feed. One should however take into account that this value does **not** include heading lines including an eventual line specified with `TITLE`. The minimum number of lines is 5, and the maximum value is 255. A specification of 0 has the result that AS will not do any form feeds except those triggered by a `NEWPAGE` instruction or those implicitly engaged at the end of the assembly listing (e.g. prior to the symbol table).

The specification of the listing's length in characters is an optional second parameter and serves two purposes: on the one hand, the internal line counter of AS will continue to run correctly when a source line has to be split into several listing lines, and on the other hand there are printers (like some laser printers) that do not automatically wrap into a new line at line end but instead simply discard the rest. For this reason, AS does line breaks by itself, i.e. lines that are too long are split into chunks whose lengths are equal to or smaller than the specified width. This may lead to double line feeds on printers that can do line wraps on their own if one specifies the exact line width as listing width. The solution for such a case is to reduce the assembly listing's width by 1. The specified line width may lie between 5 and 255 characters; a line width of 0 means similarly to the page length that AS shall not do any splitting of listing lines; lines that are too long of course cannot be taken into account of the form feed then any more.

The default setting for the page length is 60 lines, the default for the line width is 0; the latter value is also assumed when `PAGE` is called with only one parameter.

In case `PAGE` is already a machine instruction on the selected processor target, use instead `PAGESIZE` to define the paper size.

**CAUTION!** There is no way for AS to check whether the specified listing length and width correspond to the reality!

### NEWPAGE

`NEWPAGE` can be used to force a line feed though the current line is not full up to now. This might be useful to separate program parts in the listing that are logically different. The internal line counter is reset and the page counter is incremented by one. The optional parameter is in conjunction with a hierarchical page numbering AS supports up to a chapter depth of 4. 0 always refers to the lowest depth, and the maximum value may vary during the assembly run. This may look a bit puzzling, as the following example shows:

```asm
        page 1, instruction `NEWPAGE 0` → page 2
        page 2, instruction `NEWPAGE 1` → page 2.1
        page 2.1, instruction `NEWPAGE 1` → page 3.1
        page 3.1, instruction `NEWPAGE 0` → page 3.2
        page 3.2, instruction `NEWPAGE 2` → page 4.1.1
```

`NEWPAGE <number>` may therefore result in changes in different digits, depending on the current chapter depth. An automatic form feed due to a line counter overflow or a `NEWPAGE` without parameter is equal to `NEWPAGE 0`. Previous to the output of the symbol table, an implicit `NEWPAGE <maximum up to now>` is done to start a new 'main chapter'.

### MACEXP_DFT and MACEXP_OVR

Once a macro is tested and 'done', one might not want to see it in the listing when it is used. As described in the section about defining and using macros ([MACRO](#macro)), additional arguments to the `MACRO` statement allow to control whether a macro's body is expanded upon its usage and if yes, which parts of it. In case that several macros are defined in a row, it is not necessary to give these directives for every single macro. The pseudo instruction `MACEXP_DFT` defines for all following macros which parts shall be expanded upon invocation of the macro:

- `ON` resp. `OFF` enable or disable expansion completely.
- The arguments `IF` resp. `NOIF` enable or disable expansion of instructions for conditional assembly, plus the expansion of code parts the were excluded because of conditional assembly.
- Macro definitions (which includes `REPT`, `WHILE` and `IRP(C)`) may be excluded from or included in the expanded parts via the arguments `MACRO` resp. `NOMACRO`.
- All other lines not fitting into the first two categories may be excluded from or included in the expanded parts via the arguments `REST` resp. `NOREST`.

The default is `ON`, i.e. defined macros will be expanded completely, of course unless specific expansion arguments were given to individual macros. Furthermore, arguments given to `MACEXP_DFT` work relative to the current setting: for instance, if expansion is turned on completely initially, the statement

```asm
MACEXP_DFT  noif,nomacro
```

has the result that for macros defined in succession, only code parts that are no macro definition and that are not excluded via conditional assembly will be listed.

This instruction plus the per-macro directives provide fine-grained per-macro over the parts being expanded. However, there may be cases in practice where one wants to see the expanded code of a macro at one place and not at the other. This is possible by using the statement `MACEXP_OVR`: it accepts the same arguments like `MACEXP_DFT`, these however act as overrides for all macros being _expanded_ in the following code. This is in contrast to `MACEXP_DFT` which influences macros being _defined_ in the following code. For instance, if one defined for a macro that neither macro definitions nor conditional assembly shall be expanded in the listing, a

```asm
MACEXP_OVR  MACRO
```

re-enables expansion of macro definitions for its following usages, while a

```asm
MACEXP_OVR  ON
```

forces expansion of the complete macro body in the listing. `MACEXP_OVR` without arguments again disables all overrides, macros will again behave as individually specified upon definition.

Both statements also have an effect on other macro-like constructs (`REPT, IRP, IRPC WHILE`). However, since these are expanded only one and "in-place", the functional difference of these two statements becomes minimal. In case of differences, the override set via `MACEXP_OVR` has a higher priority.

The Setting currently made via `MACEXP_DFT` may be read from the predefined symbol `MACEXP`. For backward compatibility reasons, it is possible to use the statement `MACEXP` instead of `MACEXP_DFT`. However, one should not make use of this in new programs.

### LISTING

works like `MACEXP` and accepts the same parameters, but is much more radical: After a

```asm
        listing off
```

nothing at all will be written to the listing. This directive makes sense for tested code parts or include files to avoid a paper consumption going beyond all bounds. **CAUTION!** If one forgets to issue the counterpart somewhere later, even the symbol table will not be written any more! In addition to `ON` and `OFF`, `LISTING` also accepts `NOSKIPPED` and `PURECODE` as arguments. Program parts that were not assembled due to conditional assembly will not be written to the listing when `NOSKIPPED` is set, while `PURECODE` - as the name indicates - even suppresses the `IF` directives themselves in the listing. These options are useful if one uses macros that act differently depending on parameters and one only wants to see the used parts in the listing.

The current setting may be read from the symbol `LISTING` (0=`OFF`, 1=`ON`, 2=`NOSKIPPED`, 3=`PURECODE`).

### PRTINIT and PRTEXIT

Quite often it makes sense to switch to another printing mode (like compressed printing) when the listing is sent to a printer and to deactivate this mode again at the end of the listing. The output of the needed control sequences can be automated with these instructions if one specifies the sequence that shall be sent to the output device prior to the listing with `PRTINIT <string>` and similarly the de-initialization string with `PRTEXIT <string>`. `<string>` has to be a string expression in both cases. The syntax rules for string constants allow to insert control characters into the string without too much tweaking.

When writing the listing, the assembler does **not** differentiate where the listing actually goes, i.e. printer control characters are sent to the screen without mercy!

Example:

For Epson printers, it makes sense to switch them to compressed printing because listings are so wide. The lines

```asm
        prtinit "\15"
        prtexit "\18"
```

assure that the compressed mode is turned on at the beginning of the listing and turned off afterwards.

### TITLE

The assembler normally adds a header line to each page of the listing that contains the source file's name, date, and time. This statement allows to extend the page header by an arbitrary additional line. The string that has to be specified is an arbitrary string expression.

Example:

For the Epson printer already mentioned above, a title line shall be written in wide mode, which makes it necessary to turn off the compressed mode before:

```asm
        title   "\18\14Wide Title\15"
```

(Epson printers automatically turn off the wide mode at the end of a line.)

### RADIX

`RADIX` with a numerical argument between 2 and 36 sets the default numbering system for integer constants, i.e. the numbering system used if nothing else has been stated explicitly. The default is 10, and there are some possible pitfalls to keep in mind which are described in [Integer Constants](assembler-usage.md#integer-constants).

Independent of the current setting, the argument of `RADIX` is _always decimal_; furthermore, no symbolic or formula expressions may be used as argument. Only use simple constant numbers!

### OUTRADIX

`OUTRADIX` can in a certain way be regarded as the opposite to `RADIX`: This statement allows to configure which numbering system to use for integer results when `\{...}` constructs are used in string constants (see [String Constants](assembler-usage.md#string-constants)). Valid arguments range again from 2 to 36, while the default is 16.

## Local Symbols

_valid for: all processors_

local symbols and the section concept introduced with them are a completely new function that was introduced with version 1.39. One could say that this part is version "1.0" and therefore probably not the optimum. Ideas and (constructive) criticism are therefore especially wanted. I admittedly described the usage of sections how I imagined it. It is therefore possible that the reality is not entirely equal to the model in my head. I promise that in case of discrepancies, changes will occur that the reality gets adapted to the documentation and not vice versa (I was told that the latter sometimes takes place in larger companies...).

AS does not generate linkable code (and this will probably not change in the near future `:-(`). This fact forces one to always assemble a program in a whole. In contrast to this technique, a separation into linkable modules would have several advantages:

- shorter assembly times as only the modified modules have to be reassembled;
- the option to set up defined interfaces among modules by definition of private and public symbols;
- the smaller length of the individual modules reduces the number of symbols per module and therefore allows to use shorter symbol names that are still unique.

Especially the last item was something that always nagged me: once there was a label's name defined at the beginning of a 2000-lines program, there was no way to reuse it somehow - even not at the file's other end where routines with a completely different context were placed. I was forced to use concatenated names in the style of

```asm
<subprogram name>_<symbol name>
```

that had lengths ranging from 15 to 25 characters and made the program difficult to overlook. The concept of section described in detail in the following text was designed to cure at least the second and third item of the list above. It is completely optional: if you do not want to use sections, simply forget them and continue to work like you did with previous versions of AS.

### Basic Definition (SECTION/ENDSECTION)

A section represents a part of the assembler program enclosed by special statements and has a unique name chosen by the programmer:

```asm
        .
        .
        <other code>
        .
        .
        SECTION <section's name>
        .
        .
        <code inside of the section>
        .
        .
        ENDSECTION [section's name]
        .
        .
        <other code>
        .
        .
```

The name of a section must conform to the conventions for s symbol name; AS stores section and symbol names in separate tables which is the reason why a name may be used for a symbol and a section at the same time. Section names must be unique in a sense that there must not be more than one section on the same level with the same name (I will explain in the next part what "levels" mean). The argument of `ENDSECTION` is optional, it may also be omitted; if it is omitted, AS will show the section's name that has been closed with this `ENDSECTION`. Code inside a section will be processed by AS exactly as if it were outside, except for three decisive differences:

- Symbols defined within a section additionally get an internally generated number that corresponds to the section. These symbols are not accessible by code outside the section (this can be changed by pseudo instructions, later more about this).
- The additional attribute allows to define symbols of the same name inside and outside the section; the attribute makes it possible to use a symbol name multiple times without getting error messages from AS.
- If a symbol of a certain name has been defined inside and outside of a section, the "local" one will be preferred inside the section, i.e. AS first searches the symbol table for a symbol of the referenced name that also was assigned to the section. A search for a global symbol of this name only takes place if the first search fails.

This mechanism e.g. allows to split the code into modules as one might have done it with linkable code. A more fine-grained approach would be to pack every routine into a separate section. Depending on the individual routines' lengths, the symbols for internal use may obtain very short names.

AS will by default not differentiate between upper and lower case in section names; if one however switches to case-sensitive mode, the case will be regarded just like for symbols.

The organization described up to now roughly corresponds to what is possible in the C language that places all functions on the same level. However, as my "high-level" ideal was Pascal and not C, I went one step further:

### Nesting and Scope Rules

It is valid to define further sections within a section. This is analog to the option given in Pascal to define procedures inside a procedure or function. The following example shows this:

```asm
sym     EQU        0

        SECTION    ModuleA

        SECTION    ProcA1

sym     EQU        5

        ENDSECTION ProcA1

        SECTION    ProcA2

sym     EQU        10

        ENDSECTION ProcA2

        ENDSECTION ModuleA


        SECTION    ModuleB

sym     EQU        15

        SECTION    ProcB

        ENDSECTION ProcB

        ENDSECTION ModuleB
```

When looking up a symbol, AS first searches for a symbol assigned to the current section, and afterwards traverses the list of parent sections until the global symbols are reached. In our example, the individual sections see the values given in [Valid values for the Individual Sections table](#table-valid-values-for-the-individual-sections) for the symbol `sym`:

##### **Table:** Valid values for the Individual Sections

| section   | value | from section... |
| :-------- | :---- | :-------------- |
| Global    | 0     | Global          |
| `ModuleA` | 0     | Global          |
| `ProcA1`  | 5     | `ProcA1`        |
| `ProcA2`  | 10    | `ProcA2`        |
| `ModuleB` | 15    | `ModuleB`       |
| `ProcB`   | 15    | `ModuleB`       |

This rule can be overridden by explicitly appending a section's name to the symbol's name. The section's name has to be enclosed in brackets:

```asm
        move.l  #sym[ModulB],d0
```

Only sections that are in the parent section path of the current section may be used. The special values `PARENT0...PARENT9` are allowed to reference the n-th "parent" of the current section; `PARENT0` is therefore equivalent to the current section itself, `PARENT1` the direct parent and so on. `PARENT1` may be abbreviated as `PARENT`. If no name is given between the brackets, like in this example:

```asm
        move.l  #sym[],d0
```

one reaches the global symbol.

**CAUTION!** If one explicitly references a symbol from a certain section, AS will only seek for symbols from this section, i.e. the traversal of the parent sections path is omitted!

Similar to Pascal, it is allowed that different sections have subsections of the same name; the principle of locality avoids irritations. One should IMHO still use this feature as seldom as possible: Symbols listed in the symbol resp. cross reference list are only marked with the section they are assigned to, not with the "section hierarchy" lying above them (this really would have busted the available space); a differentiation is made very difficult this way.

As a `SECTION` instruction does not define a label by itself, the section concept has an important difference to Pascal's concept of nested procedures: a pascal procedure can automatically "see" its sub-procedures(functions), AS requires an explicit definition of an entry point. This can be done e.g. with the following macro pair:

```asm
proc    MACRO   name
        SECTION name
name    LABEL   $
        ENDM

endp    MACRO   name
        ENDSECTION name
        ENDM
```

This example also shows that the locality of labels inside macros is not influenced by sections. It makes the trick with the `LABEL` instruction necessary.

This does, of course, not solve the problem completely. The label is still local and not referenceable from the outside. Those who think that it would suffice to place the label in front of the `SECTION` statement should be quiet because they would spoil the bridge to the next theme:

### PUBLIC and GLOBAL

The `PUBLIC` statement allows to change the assignment of a symbol to a certain section. It is possible to treat multiple symbols with one statement, but I will use an example with only one symbol in the following (not hurting the generality of this discussion). In the simplest case, one declares a symbol to be global, i.e. it can be referenced from anywhere in the program:

```asm
        PUBLIC  <name>
```

As a symbol cannot be moved in the symbol table once it has been sorted in, this statement has to appear **before** the symbol itself is defined. AS stores all `PUBLICs` in a list and removes an entry from this list when the corresponding symbol is defined. AS prints errors at the end of a section in case that not all `PUBLICs` have been resolved.

Regarding the hierarchical section concept, the method of defining a symbol as purely global looks extremely brute. There is fortunately a way to do this in a bit more differentiated way: by appending a section name:

```asm
        PUBLIC  <name>:<section>
```

The symbol will be assigned to the referenced section and therefore also becomes accessible for all its subsections (except they define a symbol of the same name that hides the "more global" symbol). AS will naturally protest if several subsections try to export a symbol of same name to the same level. The special `PARENTn` values mentioned in the previous section are also valid for `<section>` to export a symbol exactly `n` levels up in the section hierarchy. Otherwise only sections that are parent sections of the current section are valid for `<section>`. Sections that are in another part of the section tree are not allowed. If several sections in the parent section path should have the same name (this is possible), the lowest level will be taken.

This tool lets the aforementioned macro become useful:

```asm
proc    MACRO   name
        SECTION name
        PUBLIC  name:PARENT
name    LABEL   $
        ENDM
```

This setting is equal to the Pascal model that also only allows the "father" to see its children, but not the "grandpa".

AS will quarrel about double-defined symbols if more than one section attempts to export a symbol of a certain name to the same upper section. This is by itself a correct reaction, and one needs to "qualify" symbols somehow to make them distinguishable if these exports were deliberate. A `GLOBAL` statement does just this. The syntax of `GLOBAL` is identical to `PUBLIC`, but the symbol stays local instead of being assigned to a higher section. Instead, an additional symbol of the same value but with the subsection's name appended to the symbol's name is created, and only this symbol is made public according to the section specification. If for example two sections `A` and `B` both define a symbol named `SYM` and export it with a `GLOBAL` statement to their parent section, the symbols are sorted in under the names `A_SYM` resp. `B_SYM` .

In case that source and target section are separated by more than one level, the complete name path is prepended to the symbol name.

### FORWARD

The model described so far may look beautiful, but there is an additional detail not present in Pascal that may spoil the happiness: Assembler allows forward references. Forward references may lead to situations where AS accesses a symbol from a higher section in the first pass. This is not a disaster by itself as long as the correct symbol is used in the second pass, but accidents of the following type may happen:

```asm
loop:   .
        <code>
        .
        .
        SECTION sub
        .               ; ***
        .
        bra.s   loop
        .
        .
loop:   .
        .
        ENDSECTION
        .
        .
        jmp     loop    ; main loop
```

AS will take the global label `loop` in the first pass and will quarrel about an out-of-branch situation if the program part at `<code>` is long enough. The second pass will not be started at all. One way to avoid the ambiguity would be to explicitly specify the symbol's section:

```asm
        bra.s   loop[sub]
```

If a local symbol is referenced several times, the brackets can be saved by using a `FORWARD` statement. The symbol is thereby explicitly announced to be local, and AS will only look in the local symbol table part when this symbol is referenced. For our example, the statement

```asm
        FORWARD loop
```

should be placed at the position marked with `**`.

`FORWARD` must not only be stated prior to a symbol's definition, but also prior to its first usage in a section to make sense. It does not make sense to define a symbol private and public; this will be regarded as an error by AS.

### Performance Aspects

The multi-stage lookup in the symbol table and the decision to which section a symbol shall be assigned of course cost a bit of time to compute. An 8086 program of 1800 lines length for example took 34.5 instead of 33 seconds after a modification to use sections (80386 SX, 16MHz, 3 passes). The overhead is therefore limited. As it has already been stated at the beginning, is is up to the programmer if (s)he wants to accept it. One can still use AS without sections.

## Miscellaneous

### SHARED

_valid for: all processors_

This statement instructs AS to write the symbols given in the parameter list (regardless if they are integer, float or string symbols) together with their values into the share file. It depends upon the command line parameters described in section [Start-Up Command, Parameters](assembler-usage.md#start-up-command-parameters) whether such a file is generated at all and in which format it is written. If AS detects this instruction and no share file is generated, a warning is the result.

**CAUTION!** A comment possibly appended to the statement itself will be copied to the first line outputted to the share file (if `SHARED`'s argument list is empty, only the comment will be written). In case a share file is written in C or Pascal format, one has to assure that the comment itself does not contain character sequences that close the comment (`*/` resp. `*)`). AS does not check for this!

### INCLUDE

_valid for: all processors_

This instruction inserts the file given as a parameter into the just as if it would have been inserted with an editor (the file name may optionally be enclosed with " characters). This instruction is useful to split source files that would otherwise not fit into the editor or to create "tool boxes".

In case that the file name does not have an extension, it will automatically be extended with `INC`.

The assembler primarily tries to open the file in the directory containing the source file with the `INCLUDE` statement. This means that a path contained in the file specification is relative to this file's directory, not to the directory the assembler was called from. Via the `-i <path list>` option, one can specify a list of directories that will automatically be searched for the file. If the file is not found, a **fatal** error occurs, i.e. assembly terminates immediately.

For compatibility reasons, it is valid to enclose the file name in `"` characters, i.e.

```asm
        include stddef51
```

and

```asm
        include "stddef51.inc"
```

are equivalent. **CAUTION!** This freedom of choice is the reason why only a string constant but no string expression is allowed!

The search list is ignored if the file name itself contains a path specification.

### BINCLUDE

_valid for: all processors_

`BINCLUDE` can be used to embed binary data generated by other programs into the code generated by AS (this might theoretically even be code created by AS itself...). `BINCLUDE` has three forms:

```asm
        BINCLUDE <file>
```

This way, the file is completely included.

```asm
        BINCLUDE <file>,<offset>
```

This way, the file's contents are included starting at `<offset>` up to the file's end.

```asm
        BINCLUDE <file>,<offset>,<length>
```

This way, `<length>` bytes are included starting at `<offset>`.

The same rules regarding search paths apply as for `INCLUDE`.

### MESSAGE, WARNING, ERROR, and FATAL

_valid for: all processors_

Though the assembler checks source files as strict as possible and delivers differentiated error messages, it might be necessary from time to time to issue additional error messages that allow an automatic check for logical error. The assembler distinguishes among three different types of error messages that are accessible to the programmer via the following three instructions:

- `WARNING`: Errors that hint at possibly wrong or inefficient code. Assembly continues and a code file is generated.
- `ERROR`: True errors in a program. Assembly continues to allow detection of possible further errors in the same pass. A code file is not generated.
- `FATAL`: Serious errors that force an immediate termination of assembly. A code file may be generated but will be incomplete.

All three instructions have the same format for the message that shall be issued: an arbitrary (possibly computed?!) string expression which may therefore be either a constant or variable.

These instructions generally only make sense in conjunction wit conditional assembly. For example, if there is only a limited address space for a program, one can test for overflow in the following way:

```asm
ROMSize equ     8000h   ; 27256 EPROM

ProgStart:
        .
        .
        <the program itself>
        .
        .
ProgEnd:

        if      ProgEnd-ProgStart>ROMSize
        error  "\athe program is too long!"
        endif
```

Apart from the instructions generating errors, there is also an instruction `MESSAGE` that simply prints a message to the assembly listing and the console (the latter only if the quiet mode is not used). Its usage is equal to the other three instructions.

### READ

_valid for: all processors_

One could say that `READ` is the counterpart to the previous instruction group: it allows to read values from the keyboard during assembly. You might ask what this is good for. I will break with the previous principles and put an example before the exact description to outline the usefulness of this instruction:

A program needs for data transfers a buffer of a size that should be set at assembly time. One could store this size in a symbol defined with `EQU`, but it can also be done interactively with `READ`:

```asm
        IF      MomPass=1
        READ    "buffer size",BufferSize
        ENDIF
```

Programs can this way configure themselves dynamically during assembly and one could hand over the source to someone who can assemble it without having to dive into the source code. The `IF` conditional shown in the example should always be used to avoid bothering the user multiple times with questions.

`READ` is quite similar to `SET` with the difference that the value is read from the keyboard instead of the instruction's arguments. This for example also implies that AS will automatically set the symbol's type (integer, float or string) or that it is valid to enter formula expressions instead of a simple constant.

`READ` may either have one or two parameters because the prompting message is optional. AS will print a message constructed from the symbol's name if it is omitted.

### INTSYNTAX

_valid for: all processors_

This instruction allows to modify the set of notations for integer constants in various number systems. - After selection of a CPU target, a certain default set is selected (see section 10(#SectPseudoInst)). This set may be augmented with other notations, or notations may be removed from it. `INTSYNTAX` takes an arbitrary list of arguments which either begin with aplus or minus character, followed by the notation's identifier. For instance, the following statement

```asm
        intsyntax    -0oct,+0hex
```

has the result that a leading zero marks a hexadecimal instead of an octal constant, a common usage on some assemblers for the SC/MP. The notations' identifiers can be found in table [Defined Numbering Systems and Notations table](assembler-usage.md#table-defined-numbering-systems-and-notations). There is no limit on combining notations, except when they contradict each other. For instance, it would not be allowed to enable `0oct` and `0hex` at the same time.

### RELAXED

_valid for: all processors_

By default, AS assigns a distinct syntax for integer constants to a processor family (which is in general equal to the manufacturer's specifications, as long as the syntax is not too bizarre...). Everyone however has his own preferences for another syntax and may well live with the fact that his programs cannot be translated any more with the standard assembler. If one places the instruction

```asm
        RELAXED ON
```

right at the program's beginning, one may further use any syntax for integer constants, even mixed in a program. AS tries to guess automatically for every expression the syntax that was used. This automatism does not always deliver the result one might have in mind, and this is also the reason why this option has to be enable explicitly: if there are no prefixes or postfixes that unambiguously identify either Intel or Motorola syntax, the C mode will be used. Leading zeroes that are superfluous in other modes have a meaning in this mode:

```asm
        move.b  #08,d0
```

This constant will be understood as an octal constant and will result in an error message as octal numbers may only contain digits from 0 to 7. One might call this a lucky case; a number like 077 would result in trouble without getting a message about this. Without the relaxed mode, both expressions unambiguously would have been identified as decimal constants.

The current setting may be read from a symbol with the same name.

### COMPMODE

_valid for: various processors_

Though the assembler strives to behave like the corresponding "original assemblers", there are cases when emulating the original assembler's behaviour would forbid code optimizations which are valid and useful in my opinion. Use the statement

```asm
        compmode on
```

to switch to a 'compatibility mode' which prioritizes 'original behaviour' to most efficient code. See the respective section with processor-specific hints whether there are any situations for the specific target.

Compatibility mode is disabled by default, unless it was activated by the command line switch of same name. The current setting may be read from a symbol with the same name.

### END

_valid for: all processors_

`END` marks the end of an assembler program. Lines that eventually follow in the source file will be ignored. **IMPORTANT:** `END` may be called from within a macro, but the `IF`-stack for conditional assembly is not cleared automatically. The following construct therefore results in an error:

```asm
        IF      DontWantAnymore
        END
        ELSEIF
```

`END` may optionally have an integer expression as argument that marks the program's entry point. AS stores this in the code file with a special record and it may be post-processed e.g. with P2HEX.

`END` has always been a valid instruction for AS, but the only reason for this in earlier releases of AS was compatibility; `END` had no effect.

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
