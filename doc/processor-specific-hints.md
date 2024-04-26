# Processor-specific Hints

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->

When writing the individual code generators, I strived for a maximum amount of compatibility to the original assemblers. However, I only did this as long as it did not mean an unacceptable additional amount of work. I listed important differences, details and pitfalls in the following chapter.

## 6811

"Where can I buy such a beast, a HC11 in NMOS?", some of you might ask. Well, of course it does not exist, but an H cannot be represented in a hexadecimal number (older versions of AS would not have accepted such a name because of this), and so I decided to omit all the letters...

> _"Someone stating that something is impossible should be at least as cooperative as not to hinder the one who currently does it."_

From time to time, one is forced to revise one's opinions. Some versions earlier, I stated at his place that I couldn't use AS's parser in a way that it is also possible to to separate the arguments of `BSET/BCLR` resp. `BRSET/BRCLR` with spaces. However, it seems that it can do more than I wanted to believe...after the n+1th request, I sat down once again to work on it and things seem to work now. You may use either spaces or commas, but not in all variants, to avoid ambiguities: for every variant of an instruction, it is possible to use only commas or a mixture of spaces and commas as Motorola seems to have defined it (their data books do not always have the quality of the corresponding hardware...):

```asm
Bxxx  abs8 #mask         is equal to Bxxx  abs8,#mask
Bxxx  disp8,X #mask      is equal to Bxxx  disp8,X,#mask
BRxxx abs8 #mask addr    is equal to BRxxx abs8,#mask,addr
BRxxx disp8,X #mask addr is equal to BRxxx disp8,X,#mask,addr
```

In this list, `xxx` is a synonym either for `SET` or `CLR`; `#mask` is the bit mask to be applied (the \# sign is optional). Of course, the same statements are also valid for Y-indexed expression (not listed here).

With the K4 version of the HC11, Motorola has introduced a banking scheme, which one one hand easily allows to once again extend an architecture that has become 'too small', but on the other hand not really makes programmers' and tool developers' lives simpler...how does one sensibly map something like this on a model for a programmer?

The K4 architecture _extends_ the HC11 address space by 2x512 Kbytes, which means that we now have a total address space of 64+1024=1088 Kbytes. AS acts like this were one large unified address space, with the following layout:

- `$000000...$00ffff`: the old HC11 address space
- `$010000...$08ffff`: Window 1
- `$090000...$10ffff`: Window 2

Via the `ASSUME` statement, one tells AS how the banking registers are set up, which in turn describes which extended areas are mapped to which physical addresses. For absolute addresses modes with addresses beyond $10000, AS automatically computes the address within the first 64K that is to be used. Of course this only works for direct addressing modes, it is the programmer's responsibility to keep the overview for indirect or indexed addressing modes!

In case one is not really sure if the current mapping is really the desired one, the pseudo instruction `PRWINS` may be used, which prints the assumes MMxxx register contents plus the current mapping(s), like this:

```asm
MMSIZ $e1 MMWBR $84 MM1CR $00 MM2CR $80
Window 1: 10000...12000 --> 4000...6000
Window 1: 90000...94000 --> 8000...c000
```

An instruction

```asm
        jmp     *+3
```

located at $10000 would effectively result in a jump to address $4003.

## PowerPC

Of course, it is a bit crazy idea to add support in AS for a processor that was mostly designed for usage in work stations. Remember that AS mainly is targeted at programmers of single board computers. But things that today represent the absolute high end in computing will be average tomorrow and maybe obsolete the next day, and in the meantime, the Z80 as the 8088 have been retired as CPUs for personal computers and been moved to the embedded market; modified versions are marketed as microcontrollers. With the appearance of the MPC505 and PPC403, my suspicion has proven to be true that IBM and Motorola try to promote this architecture in as many fields as possible.

However, the current support is a bit incomplete: Temporarily, the Intel-style mnemonics are used to allow storage of data and the more uncommon RS/6000 machine instructions mentioned in [^Mot601] are missing (hopefully no one misses them!). I will finish this as soon as information about them is available!

## DSP56xxx

Motorola, which devil rode you! Which person in your company had the "brilliant" idea to separate the parallel data transfers with spaces! In result, everyone who wants to make his code a bit more readable, e.g. like this:

```asm
        move    x:var9 ,r0
        move    y:var10,r3
```

is p\*\*\*\*ed because the space gets recognized as a separator for parallel data transfers!

Well...Motorola defined it that way, and I cannot change it. Using tabs instead of spaces to separate the parallel operations is also allowed, and the individual operations' parts are again separated with commas, as one would expect it.

[^Mot56] states that instead of using `MOVEC`, `MOVEM`, `ANDI` or `ORI`, it is also valid to use the more general Mnemonics `MODE`, `AND` or `OR`. AS (currently) does not support this.

## H8/300

Regarding the assembler syntax of these processors, Hitachi generously copied from Motorola (that wasn't by far the worst choice...), unfortunately the company wanted to introduce its own format for hexadecimal numbers. To make it even worse, it is a format that uses unbalanced single quotes, just like Microchip does:

```asm
       mov.w #h'ff,r0
```

This format is not supported by default. Instead, one has to write hexadecimal numbers in the well-known Motorola syntax: with a leading dollar sign. If you really need the 'Hitachi Syntax', e.g. to assemble existing code, enable the RELAXED mode. Bear in mind that this syntax has received few testing so far. I can therefore not guarantee that it will work in all cases!

## H8/500

The H8/500's `MOV` instruction features an interesting and uncommon optimization: If the target operand has a size of 16 bits, it is still possible to use an 8-bit (immediate) source operand. For example, for an instruction like this:

```asm
       mov.w #$ffff,@$1234
```

it is possible to encode the source just as a single $ff and to save one byte in code size. The processor automatically performs a sign extension, which turns $ff into the desired value $ffff. AS is aware of this optimization and will use it, unless it was explicitly forbidden via a `:16` suffix at the immediate operand.

Unfortunately, the original Hitachi assembler seems to implement this optimization in another way: it assumes a zero instead of a sign extension. This means that values from 0 to 255 ($0000 to $00ff) and not from -128 to +127 ($ff80 to $007f) are encoded as one byte. I do not know whether the Hitachi assembler or the H8/500 Programmers Manual is right, and I have no way to verify this. Anyway, if one wants the "Hitachi Assembler Behaviour" for existing source code, one may enable the compatibility mode, either by the statement

```asm
      compmode on
```

or by the respective command line switch.

Aside from this, the same remarks regarding hexadecimal number syntax apply as for H8/500.

## SH7000/7600/7700

Unfortunately, Hitachi once again used their own format for hexadecimal numbers, and once again I was not able to reproduce this with AS...please use Motorola syntax!

When using literals and the `LTORG` instruction, a few things have to be kept in mind if you do not want to suddenly get confronted with strange error messages:

Literals exist due to the fact that the processor is unable to load constants out of a range of -128 to 127 with immediate addressing. AS (and the Hitachi assembler) hide this inability by the automatic placement of constants in memory which are then referenced via PC-relative addressing. The question that now arises is where to locate these constants in memory. AS does not automatically place a constant in memory when it is needed; instead, they are collected until an LTORG instruction occurs. The collected constants are then dumped en bloc, and their addresses are stored in ordinary labels which are also visible in the symbol table. Such a label's name is of the form

```asm
LITERAL_s_xxxx_n  .
```

In this name, `s` represents the literal's type. Possible values are `W` for 16-bit constants, `L` for 32-bit constants and `F` for forward references where AS cannot decide in anticipation which size is needed. In case of `s=W` or `L`, `xxxx` denotes the constant's value in a hexadecimal notation, whereas `xxxx` is a simple running number for forward references (in a forward reference, one does not know the value of a constant when it is referenced, so one obviously cannot incorporate its value into the name). `n` is a counter that signifies how often a literal of this value previously occurred in the current section. Literals follow the standard rules for localization by sections. It is therefore absolutely necessary to place literals that were generated in a certain section before the section is terminated!

The numbering with `n` is necessary because a literal may occur multiple times in a section. One reason for this situation is that PC-relative addressing only allows positive offsets; Literals that have once been placed with an `LTORG` can therefore not be referenced in the code that follows. The other reason is that the displacement is generally limited in length (512 resp. 1024 bytes).

An automatic `LTORG` at the end of a program or previously to switching to a different target CPU does not occur; if AS detects unplaced literals in such a situation, an error message is printed.

As the PC-relative addressing mode uses the address of the current instruction plus 4, it is not possible to access a literal that is stored directly after the instruction, like in the following example:

```asm
        mov     #$1234,r6
        ltorg
```

This is a minor item since the CPU anyway would try to execute the following data as code. Such a situation should not occur in a real program...another pitfall is far more real: if PC-relative addressing occurs just behind a delayed branch, the program counter is already set to the destination address, and the displacement is computed relative to the branch target plus 2. Following is an example where this detail leads to a literal that cannot be addressed:

```asm
        bra     Target
        mov     #$12345678,r4        ; is executed
        .
        .
        ltorg                        ; here is the literal
        .
        .
Target: mov     r4,r7                ; execution continues here
```

As `Target`+2 is on an address behind the literal, a negative displacement would result. Things become especially hairy when one of the branch instructions `JMP, JSR, BRAF, or BSRF` is used: as AS cannot calculate the target address (it is generated at runtime from a register's contents), a PC value is assumed that should never fit, effectively disabling any PC-relative addressing at this point.

It is not possible to deduce the memory usage from the count and size of literals. AS might need to insert a padding word to align a long word to an address that is evenly divisible by 4; on the other hand, AS might reuse parts of a 32-bit literal for other 16-bit literals. Of course multiple use of a literal with a certain value will create only one entry. However, such optimizations are completely suppressed for forward references as AS does not know anything about their value.

As literals use the PC-relative addressing which is only allowed for the `MOV` instruction, the usage of literals is also limited to `MOV` instructions. The way AS uses the operand size is a bit tricky: A specification of a byte or word move means to generate the shortest possible instruction that results in the desired value placed in the register's lowest 8 resp. 16 bits. The upper 24 resp. 16 bits are treated as "don't care". However, if one specifies a long-word move or omits the size specification completely, this means that the complete 32-bit register should contain the desired value. For example, in the following sequence

```asm
        mov.b   #$c0,r0
        mov.w   #$c0,r0
        mov.l   #$c0,r0
```

the first instruction will result in true immediate addressing, the second and third instruction will use a word literal: As bit 7 in the number is set, the byte instruction will effectively create the value $FFFFFFC0 in the register. According to the convention, this wouldn't be the desired value in the second and third example. However, a word literal is also sufficient for the third case because the processor will copy a cleared bit 15 of the operand to bits 16...31.

As one can see, the whole literal stuff is rather complex; I'm sorry but there was no chance of making things simpler. It is unfortunately a part of its nature that one sometimes gets error messages about literals that were not found, which logically should not occur because AS does the literal processing completely on his own. However, if other errors occur in the second pass, all following labels will move because AS does not generate any code any more for statements that have been identified as erroneous. As literal names are partially built from other symbols' values, other errors might follow because literal names searched in the second pass differ from the names stored in the first pass and AS quarrels about undefined symbols...if such errors should occur, please correct all other errors first before you start cursing on me and literals...

People who come out of the Motorola scene and want to use PC-relative addressing explicitly (e.g. to address variables in a position-independent way) should know that if this addressing mode is written like in the programmer's manual:

```asm
        mov.l   @(Var,PC),r8
```

**no** implicit conversion of the address to a displacement will occur, i.e. the operand is inserted as-is into the machine code (this will probably generate a value range error...). If you want to use PC-relative addressing on the SH7x00, simply use "absolute" addressing (which does not exist on machine level):

```asm
        mov.l   Var,r8
```

In this example, the displacement will be calculated correctly (of course, the same limitations apply for the displacement as it was the case for literals).

## HMCS400

The instruction set of these 4 bit processors spontaneously reminded me of the 8080/8085 - many mnemonics, the addressing mode (e.g. direct or indirect) is coded into the instruction, and the instructions are sometimes hard to memorize. AS or course supports this syntax as Hitachi defined it. I however implemented another variant for most instructions that is - in my opinion - more beautiful and better to read. The approach is similar to what Zilog did back then for the Z80. For instance, all machine instructions that transfer data in some form, may the operands be constants, registers, or memory cells, may be used via the `LD` instruction. Similar 'meta instructions' exist for arithmetic and logical instructions. A complete list of all meta instructions and their operands can be found in the tables [Meta Instructions HMCS400](#table-meta-instructions-hmcs400) and [Operand Types for HMCS400 Meta Instructions](#table-operand-types-for-hmcs400-meta-instructions), their practical use can be seen in the file `t_hmcs4x.asm`.

##### **Table:** Meta Instructions HMCS400

| Meta Instruction     | Replaces                                                    |
| :------------------- | :---------------------------------------------------------- |
| `LD src, dest`       | `LAI`, `LBI`, `LMID`, `LMIIY`, `LAB`, `LBA`, `LAY`          |
| `LD src, dest`       | `LASPX`, `LASPY`, `LAMR`, `LWI`, `LXI`, `LYI`, `LXA`, `LYA` |
| `LD src, dest`       | `LAM`, `LAMD` `LBM`, `LMA`, `LMAD`, `LMAIY`, `LMADY`        |
| `XCH src, dest`      | `XMRA`, `XSPX`, `XSPY`, `XMA`, `XMAD`, `XMB`                |
| `ADD src, dest`      | `AYY`, `AI`, `AM`, `AMD`                                    |
| `ADC src, dest`      | `AMC`, `AMCD`                                               |
| `SUB src, dest`      | `SYY`                                                       |
| `SBC src, dest`      | `SMC`, `SMCD`                                               |
| `OR src, dest`       | `OR`, `ORM`, `ORMD`                                         |
| `AND src, dest`      | `ANM`, `ANMD`                                               |
| `EOR src, dest`      | `EORM`, `EORMD`                                             |
| `CP cond, src, dest` | `INEM`, `INEMD`, `ANEM`, `ANEMD`, `BNEM`, `YNEI`            |
| `CP cond, src, dest` | `ILEM`, `ILEMD`, `ALEM`, `ALEMD`, `BLEM`, `ALEI`            |
| `BSET bit`           | `SEC`, `SEM`, `SEMD`                                        |
| `BCLR bit`           | `REC`, `REM`, `REMD`                                        |
| `BTST bit`           | `TC`, `TM`, `TMD`                                           |

##### **Table:** Operand Types for HMCS400 Meta Instructions

| Operand       | Types                                              |
| :------------ | :------------------------------------------------- |
| `src`, `dest` | `A`, `B`, `X`, `Y`, `W`, `SPX`, `SPY` `(register)` |
| `src`, `dest` | `M` (memory addressed by `X`/`Y`/`W`)              |
| `src`, `dest` | `M+` (ditto, with auto increment)                  |
| `src`, `dest` | `M-` (ditto, with auto decrement)                  |
| `src`, `dest` | `#val` (2/4 bits immediate)                        |
| `src`, `dest` | `addr10` (memory direct)                           |
| `src`, `dest` | `MRn` (memory register 0...15)                     |
| `cond`        | `NE` (unequal)                                     |
| `cond`        | `LE` (less or equal)                               |
| `bit`         | `CA` (carry)                                       |
| `bit`         | `bitpos`,`M`                                       |
| `bit`         | `bitpos`,`addr10`                                  |
| `bitpos`      | `0...3`                                            |

## H16

The instruction set of the H16's core well deserves the label "CISC": complex addressing modes, instructions of extremely variable length, and there are many short forms for instructions with common operands. For instance, several instructions know different "formats", depending on the type of source and destination operand. The general rule is that AS will always use the shortest possible format, unless it was specified explicitly: angegeben:

```asm
        mov.l     r4,r7     ; uses R format
        mov.l     #4,r7     ; uses RQ format
        mov.l     #4,@r7    ; uses Q format
        mov.l     @r4,@r7   ; uses G format
        mov:q.l   #4,r7     ; forces Q instead of RQ format
        mov:g.l   #4,r7     ; forces G instead of RQ format
```

For immediate arguments, the "natural" argument length is used, e.g. 2 bytes for 16 bits. Shorter or longer arguments may be forced by an appended operand size (.b, .w, .l or :8, :16, :32). However, the rule for displacements and absolute addresses is that the shortest form will be used if no explicit size is given. This includes exploiting that the processor does not output the uppermost eight bits of an address. Therefore, an absolute address of $ffff80 can be coded as a single byte ($80).

Furthermore, AS knows the "accumulator bit", i.e. the second operand of a two-operand instruction my be left away if the destination is register zero. There is currently no override this behaviour.

Additionally, the following optimizations are performed:

- `MOV R0,<ea>` gets optimized to `MOVF <ea>`, unless `<ea>` is a PC-relative expression and the size of the displacement would change. This optimization may be disabled by specifying an explicit format.
- `SUB` does not support the Q format, however it may be replaced by ADD:Q` with a negated immediate argument, given the argument is in the range -127...+128. This optimization may as well be disabled by specifying an explicit format.

## OLMS-40

Similar to the HMCS400, addressing modes are largely encoded (or rather encrypted...) into into the mnemonics, and also here I decided to provide an alternate notation that is more modern and better to read. A complete list of all meta instructions and their operands can be found in the tables [Meta Instructions OLMS-40](#table-meta-instructions-olms-40) and [Operand Types for OLMS-40 Meta Instructions](#table-operand-types-for-olms-40-meta-instructions), their practical use can be seen in the file `t_olms4.asm`.

##### **Table:** Meta Instructions OLMS-40

| Meta Instruction | Replaces                                                                                                            |
| :--------------- | :------------------------------------------------------------------------------------------------------------------ |
| `LD dest, src`   | `LAI`, `LLI`, `LHI`, `L`, `LAL`, `LLA`, `LAW`, `LAX`, `LAY`, `LAZ`, `LWA`, `LXA`, `LYA`, `LPA`, `LTI`, `RTH`, `RTL` |
| `DEC dest`       | `DCA`, `DCL`, `DCM`, `DCW`, `DCX`, `DCY`, `DCZ`, `DCH`                                                              |
| `INC dest`       | `INA`, `INL`, `INM`, `INW`, `INX`, `INY`, `INZ`                                                                     |
| `BSET bit`       | `SPB`, `SMB`, `SC`                                                                                                  |
| `BCLR bit`       | `RPB`, `RMB`, `RC`                                                                                                  |
| `BTST bit`       | `TAB`, `TMB`, `Tc`                                                                                                  |

##### **Table:** Operand Types for OLMS-40 Meta Instructions

| Operand       | Types                                            |
| :------------ | :----------------------------------------------- |
| `src`, `dest` | `A`, `W`, `X`, `Y`, `Z`, `DPL`, `DPH` (Register) |
| `src`, `dest` | `T`, `TL`, `TH` (Timer, upper/lower half)        |
| `src`, `dest` | `(DP)`, `M` (Memory addressed by DPH/DPL)        |
| `src`, `dest` | `#val` (4/8 bit immediate)                       |
| `src`, `dest` | `PP` (Port-Pointer)                              |
| `bit`         | `C` (Carry)                                      |
| `bit`         | `(PP)`, `bitpos`                                 |
| `bit`         | `(DP)`, `bitpos`                                 |
| `bit`         | `(A)`, `bitpos`                                  |
| `bitpos`      | `0...3`                                          |

## OLMS-50

The data memory of these 4 bit controllers consists of up to 128 nibbles. However, only a very small subset of the machine instructions have enough space to accommodate seven address bits, which menas that - once again - banking must help out. The majority of instructions that address memory only contain the lower four bits of the RAM address, and unless the lowest 16 nibbles of the memory shall be addressed, the P register delivers the necessary upper address bits. The assembler is told about its current value via an

```asm
       assume  p:<value>
```

statement, e.g. directly after a `PAGE` instruction.

Speaking of `PAGE`: both `PAGE` and `SWITCH` are machine instructions on these controllers, i.e. the do not have the function known from other targets. The pseudo instruction to start a `SWITCH`/`CASE` construct is `SELECT` in OLMS-50 mode, and the listing's page size is set via `PAGESIZE`.

## MELPS-4500

The program memory of these microcontrollers is organized in pages of 128 words. Honestly said, this organization only exists because there are on the one hand branch instructions with a target that must lie within the same page, and on the other hand "long" branches that can reach the whole address space. The standard syntax defined by Mitsubishi demands that page number and offset have to be written as two distinct arguments for the latter instructions. As this is quite inconvenient (except for indirect jumps, a programmer has no other reason to deal with pages), AS also allows to write the target address in a "linear" style, for example

```asm
        bl      $1234
```

instead of

```asm
        bl      $24,$34
```

## 6502UNDOC

Since the 6502's undocumented instructions naturally aren't listed in any data book, they shall be listed shortly at this place. Of course, you are using them on your own risk. There is no guarantee that all mask revisions will support all variants! They anyhow do not work for the CMOS successors of the 6502, since they allocated the corresponding bit combinations with "official" instructions...

The following symbols are used:

| Symbol  | Meaning                       |
| :------ | :---------------------------- |
| `&`     | binary AND                    |
| `\|`    | binary OR                     |
| `^`     | binary XOR                    |
|  `<<`   | logical shift left            |
|  `>>`   | logical shift right           |
|  `<<<`  | rotate left                   |
|  `>>>`  | rotate right                  |
| `←`     | assignment                    |
| `(...)` | contents of ...               |
| `...`   | bits ...                      |
| `A`     | accumulator                   |
| `X`,`Y` | index registers X,Y           |
| `S`     | stack pointer                 |
| `An`    | accumulator bit n             |
| `M`     | operand                       |
| `C`     | carry                         |
| `PCH`   | upper half of program counter |

| Instruction      | `JAM` or `KIL` or `CRS`   |
| :--------------- | :------------------------ |
| Function         | none, processor is halted |
| Addressing Modes | implicit                  |

| Instruction      | `SLO`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← ((M) << 1) \| (A)`                                                 |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

| Instruction      | `ANC`                   |
| :--------------- | :---------------------- |
| Function         | `A ← (A) & (M), C ← A7` |
| Addressing Modes | immediate               |

| Instruction      | `RLA`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← ((M) << 1) & (A)`                                                  |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

| Instruction      | `SRE`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← ((M) >> 1) ^ (A)`                                                  |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

| Instruction      | `ASR`                  |
| :--------------- | :--------------------- |
| Function         | `A ← ((A) & (M)) >> 1` |
| Addressing Modes | immediate              |

| Instruction      | `RRA`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← ((M) >>> 1)  +  (A) + (C)`                                         |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

| Instruction      | `ARR`                   |
| :--------------- | :---------------------- |
| Function         | `A ← ((A) & (M)) >>> 1` |
| Addressing Modes | immediate               |

| Instruction      | `SAX`                                            |
| :--------------- | :----------------------------------------------- |
| Function         | `M ← (A) & (X)`                                  |
| Addressing Modes | absolute long/short, Y-indexed short, Y-indirect |

| Instruction      | `ANE`                            |
| :--------------- | :------------------------------- |
| Function         | `M ← ((A) & $ee) \| ((X) & (M))` |
| Addressing Modes | immediate                        |

| Instruction      | `SHA`                     |
| :--------------- | :------------------------ |
| Function         | `M ← (A) & (X) & (PCH+1)` |
| Addressing Modes | X/Y-indexed long          |

| Instruction      | `SHS`                                         |
| :--------------- | :-------------------------------------------- |
| Function         | `X ← (A) & (X),  S ← (X),  M ← (X) & (PCH+1)` |
| Addressing Modes | Y-indexed long                                |

| Instruction      | `SHY`               |
| :--------------- | :------------------ |
| Function         | `M ← (Y) & (PCH+1)` |
| Addressing Modes | Y-indexed long      |

| Instruction      | `SHX`               |
| :--------------- | :------------------ |
| Function         | `M ← (X) & (PCH+1)` |
| Addressing Modes | X-indexed long      |

| Instruction      | `LAX`                                                   |
| :--------------- | :------------------------------------------------------ |
| Function         | `A,  X ← (M)`                                           |
| Addressing Modes | absolute long/short, Y-indexed long/short, X/Y-indirect |

| Instruction      | `LXA`                                      |
| :--------------- | :----------------------------------------- |
| Function         | `X04 ← (X)04 & (M)04, A04 ← (A)04 & (M)04` |
| Addressing Modes | immediate                                  |

| Instruction      | `LAE`                     |
| :--------------- | :------------------------ |
| Function         | `X,  S,  A ← ((S) & (M))` |
| Addressing Modes | Y-indexed long            |

| Instruction      | `DCP`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← (M) − 1, Flags ← ((A) − (M))`                                      |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

| Instruction      | `SBX`                   |
| :--------------- | :---------------------- |
| Function         | `X ← ((X) & (A)) − (M)` |
| Addressing Modes | immediate               |

| Instruction      | `ISB`                                                                   |
| :--------------- | :---------------------------------------------------------------------- |
| Function         | `M ← (M) + 1,  A ← (A) − (M) − (C)`                                     |
| Addressing Modes | absolute long/short, X-indexed long/short, Y-indexed long, X/Y-indirect |

## MELPS-740

Microcontrollers of this family have a quite nice, however well-hidden feature: If one sets bit 5 of the status register with the `SET` instruction, the accumulator will be replaced with the memory cell addressed by the X register for all load/store and arithmetic instructions. An attempt to integrate this feature cleanly into the assembly syntax has not been made so far, so the only way to use it is currently the "hard" way (`SET`...instructions with accumulator addressing...`CLT`).

Not all MELPS-740 processors implement all instructions. This is a place where the programmer has to watch out for himself that no instructions are used that are unavailable for the targeted processor; AS does not differentiate among the individual processors of this family. For a description of the details regarding special page addressing, see the discussion of the `ASSUME` instruction.

## MELPS-7700/65816

As it seems, these two processor families took disjointed development paths, starting from the 6502 via their 8 bit predecessors. Shortly listed, the following differences are present:

- The 65816 does not have a B accumulator.
- The 65816 does not have instructions to multiply or divide.
- The 65816 misses the instructions `SEB`, `CLB`, `BBC`, `BBS`, `CLM`, `SEM`, `PSH`, `PUL` and `LDM`. Instead, the instructions `TSB`, `TRB`, `BIT`, `CLD`, `SED`, `XBA`, `XCE` and `STZ` take their places in the opcode table.

The following instructions have identical function, yet different names:

| 65816 | MELPS-7700 | 65816 | MELPS-7700 |
| :---: | :--------: | :---: | :--------: |
| `REP` |   `CLP`    | `PHK` |   `PHG`    |
| `TCS` |   `TAS`    | `TSC` |   `TSA`    |
| `TCD` |   `TAD`    | `TDC` |   `TDA`    |
| `PHB` |   `PHT`    | `PLB` |   `PLT`    |
| `WAI` |   `WIT`    |       |            |

Especially tricky are the instructions `PHB`, `PLB` and `TSB`: these instructions have a totally different encoding and meaning on both processors!

Unfortunately, these processors address their memory in a way that is IMHO even one level higher on the open-ended chart of perversity than the Intel-like segmentation: They do banking! Well, this seems to be the price for the 6502 upward-compatibility; before one can use AS to write code for these processors, one has to inform AS about the contents of several registers (using the `ASSUME` instruction):

The M flag rules whether the accumulators A and B should be used with 8 bits (1) or 16 bits (0) width. Analogously, the X flag decides the width of the X and Y index registers. AS needs this information for the decision about the argument's width when immediate addressing (`#<constant>`) occurs.

The memory is organized in 256 banks of 64 KBytes. As all registers in the CPU core have a maximum width of 16 bits, the upper 8 bits have to be fetched from 2 special bank registers: DT delivers the upper 8 bits for data accesses, and PG extends the 16-bit program counter to 24 bits. A 16 bits wide register DPR allows to move the zero page known from the 6502 to an arbitrary location in the first bank. If AS encounters an address (it is irrelevant if this address is part of an absolute, indexed, or indirect expression), the following addressing modes will be tested:

1.  Is the address in the range of `DPR...DPR+$ff`? If yes, use direct addressing with an 8-bit address.
2.  Is the address contained in the page addressable via DT (resp. PG for branch instructions)? If yes, use absolute addressing with a 16-bit address.
3.  If nothing else helps, use long addressing with a 24-bit address.

As one can see from this enumeration, the knowledge about the current values of DT, PG and DPR is essential for a correct operation of AS; if the specifications are incorrect, the program will probably do wrong addressing at runtime. This enumeration also implied that all three address lengths are available; if this is not the case, the decision chain will become shorter.

The automatic determination of the address length described above may be overridden by the usage of prefixes. If one prefixes the address by a `<`, `>`, or  `>>` without a separating space, an address with 1, 2, or 3 bytes of length will be used, regardless if this is the optimal length. If one uses an address length that is either not allowed for the current instruction or too short for the address, an error message is the result.

To simplify porting of 6502 programs, AS uses the Motorola syntax for hexadecimal constants instead of the Intel/IEEE syntax that is the format preferred by Mitsubishi for their 740xxx series. I still think that this is the better format, and it looks as if the designers of the 65816 were of the same opinion (as the `RELAXED` instruction allows the alternative use of Intel notation, this decision should not hurt anything). Another important detail for the porting of programs is that it is valid to omit the accumulator A as target for operations. For example, it is possible to simply write `LDA #0` instead of `LDA A,#0`.

A real goodie in the instruction set are the instructions `MVN` resp. `MVP` to do block transfers. However, their address specification rules are a bit strange: bits 0-15 are stored in index registers, bits 16-23 are part of the instruction. When one uses AS, one simply specifies the full destination and source addresses. AS will then automatically grab the correct bits. This is a fine yet important difference Mitsubishi's assembler where you have to extract the upper 8 bits on your own. Things become really convenient when a macro like the following is used:

```asm
mvpos   macro   src,dest,len
        if      MomCPU=$7700
        lda    #len
        elseif
        lda    #(len-1)
        endif
        ldx     #(src&$ffff)
        ldy     #(dest&$ffff)
        mvp     dest,src
        endm
```

Caution, possible pitfall: if the accumulator contains the value n, the Mitsubishi chip will transfer n bytes, but the 65816 will transfer n+1 bytes!

The `PSH` and `PUL` instructions are also very handy because they allow to save a user-defined set to be saved to the stack resp. to be restored from the stack. According to the Mitsubishi data book [^Mit16], the bit mask has to be specified as an immediate operand, so the programmer either has to keep all bit↔register assignments in mind or he has to define some appropriate symbols. To make things simpler, I decided to extend the syntax at this point: It is valid to use a list as argument which may contain an arbitrary sequence of register names or immediate expressions. Therefore, the following instructions

```asm
        psh     #$0f
        psh     a,b,#$0c
        psh     a,b,x,y
```

are equivalent. As immediate expressions are still valid, AS stays upward compatible to the Mitsubishi assemblers.

One thing I did not fully understand while studying the Mitsubishi assembler is the treatment of the `PER` instruction: this instruction allows to push a 16-bit variable onto the stack whose address is specified relative to the program counter. Therefore, it is an absolute addressing mode from the programmer's point of view. Nevertheless, the Mitsubishi assembler requests immediate addressing, and the instructions argument is placed into the code just as-is. One has to calculate the address in his own, which is something symbolic assemblers were designed for to avoid...as I wanted to stay compatible, AS contains a compromise: If one chooses immediate addressing (with a leading `#` sign), AS will behave like the original from Mitsubishi. But if the `#` sign is omitted, as will calculate the difference between the argument's value and the current program counter and insert this difference instead.

A similar situation exists for the `PEI` instruction that pushes the contents of a 16-bit variable located in the zero page: Though the operand represents an address, once again immediate addressing is required. In this case, AS will simply allow both variants (i.e. with or without a `#` sign).

## M16

The M16 family is a family of highly complex CISC processors with an equally complicated instruction set. One of the instruction set's properties is the detail that in an instruction with two operands, both operands may be of different sizes. The method of appending the operand size as an attribute of the instruction (known from Motorola and adopted from Mitsubishi) therefore had to be extended: it is valid to append attributes to the operands themselves. For example, the following instruction

```asm
        mov     r0.b,r6.w
```

reads the lowest 8 bits of register 0, sign-extends them to 32 bits and stores the result into register 6. However, as one does not need this feature in 9 out of 10 cases, it is still valid to append the operand size to the instruction itself, e.g.

```asm
        mov.w   r0,r6
```

Both variants may be mixed; in such a case, an operand size appended to an operand overrules the "default". An exception are instructions with two operands. For these instructions, the default for the source operand is the destination operand's size. For example, in the following example

```asm
        mov.h   r0,r6.w
```

register 0 is accessed with 32 bits, the size specification appended to the instruction is not used at all. If an instruction does not contain any size specifications, word size (`w`) will be used. Remember: in contrast to the 68000 family, this means 32 bits instead of 16 bits!

The chained addressing modes are also rather complex; the ability of AS to automatically assign address components to parts of the chain keeps things at least halfway manageable. The only way of influencing AS allows (the original assembler from Mitsubishi/Green Hills allows a bit more in this respect) is the explicit setting of displacement lengths by appending `:4`, `:16` and `:32`.

## 4004/4040

Thanks to John Weinrich, I now have the official Intel data sheets describing these 'grandfathers' of all microprocessors, and the questions about the syntax of register pairs (for 8-bit operations) have been weeded out for the moment: It is `RnRm` with `n` resp. `m` being even integers in the range from `0` to `E` resp. `1` to `F`. The equation `m = n + 1` must be fulfilled.

## MCS-48

The maximum address space of these processors is 4 Kbytes, resp. up to 8 Kbytes on some Philips variants. This address space is not organized in a linear way (how could this be on an Intel CPU...). Instead, it is split into 2 banks of 2 Kbytes. The only way to change the program counter from one bank to the other are the instructions `CALL` and `JMP`, by setting the most significant bit of the address with the instructions `SEL MB0` to `SEL MB3`.

The assembler may be informed about the bank currently being selected for jumps and calls, via an `ASSUME` statement:

```asm
        ASSUME MB:<0...3>
```

If one tries to jump to an address in a different bank, a warning is issued.

If the special value `NOTHING` is used (this is by the way the default), an automatism built into `JMP` and `CALL` is activated. It will insert a `SEL MBx` instruction if the current program counter and the target address are located in different banks. Explicit usage of `SEL MBx` instructions is no longer necessary (though it remains possible), and it might interfere with this mechanism, like in the following example:

```asm
000:    SEL      MB1
        JMP      200h
```

AS assumes that the MB flag is 0 and therefore does not insert a `SEL MB0` instruction, with the result that the CPU jumps to address `A00h`.

Furthermore, one should keep in mind that a jump instruction might become longer (3 instead of 2 bytes).

## MCS-51

The assembler is accompanied by the files `STDDEF51.INC` resp. `80C50X.INC` that define all bits and SFRs of the processors 8051, 8052, and 80515 resp. 80C501, 502, and 504. Depending on the target processor setting (made with the `CPU` statement), the correct subset will be included. Therefore, the correct order for the instructions at the beginning of a program is

```asm
        CPU     <processor type>
        INCLUDE stddef51.inc
```

Otherwise, the MCS-51 pseudo instructions will lead to error messages.

As the 8051 does not have instructions to to push the registers 0...7 onto the stack, one has to work with absolute addresses. However, these addresses depend on which register bank is currently active. To make this situation a little bit better, the include files define the macro `USING` that accepts the symbols `Bank0...Bank3` as arguments. In response, the macro will assign the registers' correct absolute addresses to the symbols `AR0...AR7`. This macro should be used after every change of the register banks. The macro itself does **not** generate any code to switch to the bank!

The macro also makes bookkeeping about which banks have been used. The result is stored in the integer variable `RegUsage`: bit 0 corresponds to bank 0, bit 1 corresponds to bank 1. and so on. To output its contents after the source has been assembled, use something like the following piece of code:

```asm
        irp       BANK,Bank0,Bank1,Bank2,Bank3
        if        (RegUsage&(2^BANK))<>0
        message   "bank \{BANK} has been used"
        endif
        endm
```

The multipass feature introduced with version 1.38 allowed to introduce the additional instructions `JMP` and `CALL`. If branches are coded using these instructions, AS will automatically use the variant that is optimal for the given target address. The options are `SJMP`, `AJMP`, or `LJMP` for `JMP` resp. `ACALL` or `LCALL` for `CALL`. Of course it is still possible to use these variants directly, in case one wants to force a certain coding.

## MCS-251

When designing the 80C251, Intel really tried to make the move to the new family as smooth as possible for programmers. This culminated in the fact that old applications can run on the new processor without having to recompile them. However, as soon as one wants to use the new features, some details have to be regarded which may turn into hidden pitfalls.

The most important thing is the absence of a distinct address space for bits on the 80C251. All SFRs can now be addressed bitwise, regardless of their address. Furthermore, the first 128 bytes of the internal RAM are also bit addressable. This has become possible because bits are not any more handled by a separate address space that overlaps other address spaces. Instead, similar to other processors, bits are addressed with a two-dimensional address that consists of the memory location containing the bit and the bit's location in the byte. One result is that in an expression like `PSW.7`, AS will do the separation of address and bit position itself. Unlike to the 8051, it is not any more necessary to explicitly generate 8 bit symbols. This has the other result that the `SFRB` instruction does not exist any more. If it is used in a program that shall be ported, it may be replaced with a simple `SFR` instruction.

Furthermore, Intel cleaned up the cornucopia of different address spaces on the 8051: the internal RAM (`DATA` resp. `IDATA`), the `XDATA` space and the former `CODE` space were unified to a single `CODE` space that is now 16 Mbytes large. The internal RAM starts at address 0, the internal ROM starts at address ff0000h, which is the address code has to be relocated to. In contrast, the SFRs were moved to a separate address space (which AS refers to as the `IO` segment). However, they have the same addresses in this new address space as they used to have on the 8051. The `SFR` instructions knows of this difference and automatically assigns symbols to either the `DATA` or `IO` segment, depending on the target processor. As there is no `BIT` segment any more, the `BIT` instruction operates completely different: Instead of a linear address ranging from 0...255, a bit symbol now contains the byte's address in bit 0...7, and the bit position in bits 24...26. Unfortunately, creating arrays of flags with a symbolic address is not that simple any more: On an 8051, one simply wrote:

```asm
        segment bitdata

bit1    db      ?
bit2    db      ?

or

defbit  macro   name
name    bit     cnt
cnt     set     cnt+1
        endm
```

On a 251, only the second way still works, like this:

```asm
adr     set     20h     ; start address of flags
bpos    set     0       ; in the internal RAM

defbit  macro   name
name    bit     adr.bpos
bpos    set     bpos+1
        if      bpos=8
bpos    set     0
adr     set     adr+1
        endif
        endm
```

Another small detail: Intel now prefers `CY` instead of `C` as a symbolic name for the carry, so you might have to rename an already existing variable of the same name in your program. However, AS will continue to understand also the old variant when using the instructions `CLR`, `CPL`, `SETB`, `MOV`, `ANL`, or `ORL`. The same is conceptually true for the additional registers `R8...R15`, `WR0...WR30`, `DR0...DR28`, `DR56`, `DR60`, `DPX`, and `SPX`.

Intel would like everyone to write absolute addresses in a syntax of `XX:YYYY`, where `XX` is a 64K bank in the address space resp. signifies addresses in the I/O space with an `S`. As one might guess, I am not amused about this, which is why it is legal to alternatively use linear addresses in all places. Only the `S` for I/O addresses is inescapable, as in this case:

```asm
Carry   bit     s:0d0h.7
```

Without the prefix, AS would assume an address in the `CODE` segment, and only the first 128 bits in this space are bit-addressable...

Like for the 8051, the generic branch instructions `CALL` and `JMP` exist that automatically choose the shortest machine code depending on the address layout. However, while `JMP` also may use the variant with a 24-bit address, `CALL` will not do this for a good reason: In contrast to `ACALL` and `LCALL`, `ECALL` places an additional byte onto the stack. A `CALL` instruction would result where you would not know what it will do. This problem does not exist for the `JMP` instructions.

There is one thing I did not understand: The 80251 is also able to push immediate operands onto the stack, and it may push either single bytes or complete words. However, the same mnemonic (`PUSH`) is assigned to both variants - how on earth should an assembler know if an instruction like

```asm
        push    #10
```

shall push a byte or a word containing the value 10? So the current rule is that `PUSH` always pushes a byte; if one wants to push a word, simply use `PUSHW` instead of `PUSH`.

Another well-meant advise: If you use the extended instruction set, be sure to operate the processor in source mode; otherwise, all instructions will become one byte longer! The old 8051 instructions that will in turn become one byte longer are not a big matter: AS will either replace them automatically with new, more general instructions or they deal with obsolete addressing modes (indirect addressing via 8 bit registers).

## 8080/8085

As mentioned before, the statement

```asm
        Z80SYNTAX <ON|OFF|EXCLUSIVE>
```

makes it possible to write the vast majority of 8080/8085 instructions in 'Z80 style', i.e. with less mnemonics but with operands that are easier to understand. In non-exclusive mode, the Z80 syntax is not allowed for the following instructions, because they conflict with existing 8080 mnemonics:

- `CP` in 'Intel syntax' means 'Call on Positive', in Zilog syntax however it means 'Compare'. If you use `CP` with a numeric value, it is not possible for the assembler to recognize whether a jump to an absolute address or a compare with an immediate value is meant. The assembler will generate a jump in this case, since the Intel syntax has precedence in case of ambiguities. If one wants the comparison, one may explicitly write down the accumulator as destination operand, e.g. `CP A,12h` instead of `CP 12h`.
- `JP` in Intel syntax means 'Jump on Positive', in Zilog syntax however, this is the jump instruction in general. Conditional jumps in Zilog syntax (`JP cond,addr`) are unambiguous because of the two arguments. With only one argument, the assembler will however always generate the conditional jump. If you want an unconditional jump to an absolute address, you still have to use the Intel syntax (`JMP addr`).

The 8085 supports the instructions `RIM` and `SIM` that are not part of the Z80 instruction set. They may be written in 'Z80 style' as `LD A,IM` resp. `LD IM,A`.

## 8085UNDOC

Similarly to the Z80 or 6502, Intel did not further specify the undocumented 8085 instructions. This however means that other assemblers might use different mnemonics for the same function. Therefore, I will list the instructions in the following. Once again, usage of these instructions is at one's own risk - even the Z80 which is principally upward compatible to the 8085 uses the opcodes for entirely different functions...

| Instruction | `DSUB [reg]`                                       |
| :---------- | :------------------------------------------------- |
| Z80 Syntax  | `SUB HL,reg`                                       |
| Function    | `HL ← HL - reg`                                    |
| Flags       | `CY`, `S`, `X5`, `AC`, `Z`, `V`, `P`               |
| Arguments   | `reg` = `B` for `BC` (optional for non-Z80 syntax) |

| Instruction | `ARHL`                          |
| :---------- | :------------------------------ |
| Z80 Syntax  | `SRA HL`                        |
| Function    | `HL, CY ← HL >> 1` (arithmetic) |
| Flags       | `CY`                            |
| Arguments   | none resp. fixed for Z80 syntax |

| Instruction | `RDEL`                          |
| :---------- | :------------------------------ |
| Z80 Syntax  | `RLC DE`                        |
| Function    | `CY,DE ← DE << 1`               |
| Flags       | `CY`, `V`                       |
| Arguments   | none resp. fixed for Z80 syntax |

| Instruction | `LDHI d8`                                             |
| :---------- | :---------------------------------------------------- |
| Z80 Syntax  | `ADD DE,HL,d8`                                        |
| Function    | `DE ← HL + d8`                                        |
| Flags       | none                                                  |
| Arguments   | `d8` = 8-bit constant, registers fixed for Z80 syntax |

| Instruction | `LDSI d8`                                             |
| :---------- | :---------------------------------------------------- |
| Z80 Syntax  | `ADD DE,SP,d8`                                        |
| Function    | `DE ← SP + d8`                                        |
| Flags       | none                                                  |
| Arguments   | `d8` = 8-bit constant, registers fixed for Z80 syntax |

| Instruction | `RSTflag`                     |
| :---------- | :---------------------------- |
| Z80 Syntax  | `RST flag`                    |
| Function    | restart to `40h` if `flag=1`  |
| Flags       | none                          |
| Arguments   | `flag` = `V` for overflow bit |

| Instruction | `SHLX [reg]`                                            |
| :---------- | :------------------------------------------------------ |
| Z80 Syntax  | `LD (reg),HL`                                           |
| Function    | `[reg] ← HL`                                            |
| Flags       | none                                                    |
| Arguments   | `reg` = `D`/`DE` for `DE` (optional for non-Z80 syntax) |

| Instruction | `LHLX [reg]`                                            |
| :---------- | :------------------------------------------------------ |
| Z80 Syntax  | `LD HL,(reg)`                                           |
| Function    | `HL ← [reg]`                                            |
| Flags       | none                                                    |
| Arguments   | `reg` = `D`/`DE` for `DE` (optional for non-Z80 syntax) |

| Instruction | `JNX5 addr`                      |
| :---------- | :------------------------------- |
| Z80 Syntax  | `JP NX5, addr`                   |
| Function    | jump to `addr` if `X5=0`         |
| Flags       | none                             |
| Arguments   | `addr` = absolute 16-bit address |

| Instruction | `JX5 addr`                       |
| :---------- | :------------------------------- |
| Z80 Syntax  | `JP X5,addr`                     |
| Function    | jump to `addr` if `X5=1`         |
| Flags       | none                             |
| Arguments   | `addr` = absolute 16-bit address |

X5 refers to the otherwise unused bit 5 in the processor status word (PSW).

## 8086...V35

Actually, I had sworn myself to keep the segment disease of Intel's 8086 out of the assembler. However, as there was a request and as students are more flexible than the developers of this processor obviously were, there is now a rudimentary support of these processors in AS. When saying, 'rudimentary', it does not mean that the instruction set is not fully covered. It means that the whole pseudo instruction stuff that is available when using MASM, TASM, or something equivalent does not exist. To put it in clear words, AS was not primarily designed to write assembler programs for PC's (heaven forbid, this really would have meant reinventing the wheel!); instead, the development of programs for single-board computers was the main goal (which may also be equipped with an 8086 CPU).

For die-hards who still want to write DOS programs with AS, here is a small list of things to keep in mind:

- Only `COM` files may be created.
- Only use the `CODE` segment, and place also all variables in this segment.
- DOS initializes all segment registers to the code segment. An `ASSUME DS:DATA, SS:DATA` right at the program's beginning is therefore necessary.
- DOS loads the code to a start address of 100h. An `ORG` to this address is absolutely necessary.
- The conversion to a binary file is done with `P2BIN` (see later in this document), with an address filter of `$-$`.

For these processors, AS only supports a small programming model, i.e. there is **one** code segment with a maximum of 64 Kbytes and a data segment of equal size for data (which cannot be set to initial values for `COM` files). The `SEGMENT` instruction allows to switch between these two segments. From this facts results that branches are always intra-segment branches if they refer to targets in this single code segment. In case that far jumps should be necessary, they are possible via `CALLF` or `JMPF` with a memory address or a `Segment:Offset` value as argument.

Another big problem of these processors is their assembler syntax, which is sometimes ambiguous and whose exact meaning can then only be deduced by looking at the current context. In the following example, either absolute or immediate addressing may be meant, depending on the symbol's type:

```asm
        mov     ax,value
```

When using AS, an expression without brackets always is interpreted as immediate addressing. For example, when either a variable's address or its contents shall be loaded, the differences between MASM and AS are as follows:

- To load address of variable `vari` into `ax`:

  - MASM: either of the following 3 variants have the same effect:

    ```asm
    mov ax,offset vari
    lea ax,vari
    lea ax,[vari]
    ```

  - AS: either of the following 2 variants have the same effect:

    ```asm
    mov ax,vari
    lea ax,[vari]
    ```

- To load value of variable `vari` into `ax`:

  - MASM: either of the following 2 variants have the same effect:

    ```asm
    mov ax,vari
    mov ax,[vari]
    ```

  - AS:

    ```asm
    mov ax,[vari]
    ```

When addressing via a symbol, the assembler checks whether they are assigned to the data segment and tries to automatically insert an appropriate segment prefix. This happens for example when symbols from the code segment are accessed without specifying a `CS` segment prefix. However, this mechanism can only work if the `ASSUME` instruction (see there) has previously been applied correctly.

The Intel syntax also requires to store whether bytes or words were stored at a symbol's address. AS will do this only when the `DB` resp. `DW` instruction is in the same source line as the label. For any other case, the operand size has to be specified explicitly with the `BYTE PTR`, `WORD PTR`, ... operators. As long as a register is the other operator, this may be omitted, as the operand size is then clearly given by the register's name.

In an 8086-based system, the coprocessor is usually synchronized via via the processor's TEST input line which is connected to toe coprocessor's BUSY output line. AS supports this type of handshaking by automatically inserting a `WAIT` instruction prior to every 8087 instruction. If this is undesired for any reason, an `N` has to be inserted after the `F` in the mnemonic; for example,

```asm
        FINIT
        FSTSW   [vari]
```

becomes

```asm
        FNINIT
        FNSTSW  [vari]
```

This variant is valid for **all** coprocessor instructions.

## 8X30x

The processors of this family have been optimized for an easy manipulation of bit groups at peripheral addresses. The instructions `LIV` and `RIV` were introduced to deal with such objects in a symbolic fashion. They work similar to `EQU`, however they need three parameters:

1.  the address of the peripheral memory cell that contains the bit group (0...255);
2.  the number of the group's first bit (0...7);
3.  the length of the group, expressed in bits (1...8).

**CAUTION!** The 8X30x does not support bit groups that span over more than one memory address. Therefore, the valid value range for the length can be stricter limited, depending on the start position. AS does **not** perform any checks at this point, you simply get strange results at runtime!

Regarding the machine code, length and position are expressed vis a 3 bit field in the instruction word and a proper register number (`LIVx` resp. `RIVx`). If one uses a symbolic object, AS will automatically assign correct values to this field, but it is also allowed to specify the length explicitly as a third operand if one does not work with symbolic objects. If AS finds such a length specification in spite of a symbolic operand, it will compare both lengths and issue an error if they do not match (the same will happen for the MOVE instruction if two symbolic operands with different lengths are used - the instruction simply only has a single length field...).

Apart from the real machine instructions, AS defines similarly to its "idol" MCCAP some pseudo instructions that are implemented as builtin macros:

- `NOP` is a short form for `MOVE AUX,AUX`
- `HALT` is a short form for `JMP *`
- `XML ii` is a short form for `XMIT ii,R12` (only 8X305)
- `XMR ii` is a short form for `XMIT ii,R13` (only 8X305)
- `SEL <busobj>` is a short form for `XMIT <adr>,IVL/IVR`, i.e. it performs the necessary preselection to access `<busobj>`.

The `CALL` and `RTN` instructions MCCAP also implements are currently missing due to sufficient documentation. The same is true for a set of pseudo instructions to store constants to memory. Time may change this...

## XA

Similar to its predecessor MCS/51, but in contrast to its 'competitor' MCS/251, the Philips XA has a separate address space for bits, i.e. all bits that are accessible via bit instructions have a certain, one-dimensional address which is stored as-is in the machine code. However, I could not take the obvious opportunity to offer this third address space (code and data are the other two) as a separate segment. The reason is that - in contrast to the MCS/51 - some bit addresses are ambiguous: bits with an address from 256 to 511 refer to the bits of memory cells 20h...3fh in the current data segment. This means that these addresses may correspond to different physical bits, depending on the current state. Defining bits with the help of `DC` instructions - something that would be possible with a separate segment - would not make too much sense. However, the `BIT` instruction still exists to define individual bits (regardless if they are located in a register, the RAM or SFR space) that can then be referenced symbolically. If the bit is located in RAM, the address of the 64K-bank is also stored. This way, AS can check whether the DS register has previously be assigned a correct value with an `ASSUME` instruction.

In contrast, nothing can stop AS's efforts to align potential branch targets to even addresses. Like other XA assemblers, AS does this by inserting `NOP`s right before the instruction in question.

## AVR

In contrast to the AVR assembler, AS by default uses the Intel format to write hexadecimal constants instead of the C syntax. All right, I did not look into the (free) AVR assembler before, but when I started with the AVR part, there was hardly mor einformation about the AVR than a preliminary manual describing processor types that were never sold...this problem can be solved with a simple RELAXED ON.

Optionally, AS can generate so-called "object files" for the AVRs (it also works for other CPUs, but it does not make any sense for them...). These are files containing code and source line info what e.g. allows a step-by-step execution on source level with the WAVRSIM simulator delivered by Atmel. Unfortunately, the simulator seems to have trouble with source file names longer than approx. 20 characters: Names are truncated and/or extended by strange special characters when the maximum length is exceeded. AS therefore stores file name specifications in object files without a path specification. Therefore, problems may arise when files like includes are not in the current directory.

A small specialty are machine instructions that have already been defined by Atmel as part of the architecture, but up to now haven't been implemented in any of the family's members. The instructions in question are `MUL`, `JMP,` and `CALL`. Considering the latter ones, one may ask himself how to reach the 4 Kwords large address space of the AT90S8515 when the 'next best' instructions `RJMP` and `RCALL` can only branch up to 2 Kwords forward or backward. The trick is named 'discarding the upper address bits' and described in detail with the `WRAPMODE` statement.

All AVR targets support the optional CPU argument `CODESEGSIZE`. Like in this example,

```asm
       cpu atmega8:codesegsize=0
```

it may be used to instruct the assembler to treat the code segment (i.e. the internal flash ROM) as being organized in bytes instead of 16 bit words. This is the view when the `LPM` instruction is used, and which some other (non Atmel) assemblers use in general. It has the advantage that addresses in the `CODE` segment need not be multiplied by two if used for data accesses. On the other hand, care has to be taken that instructions do not start on an odd address - this would be the equivalent of an instruction occupying fractions of flash words. The `PADDING` option is therefore enabled by default, while it remains possible to define arrays of bytes via multiple uses of `DB` or `DATA` without the risk of padding bytes inserted in between. Target addresses for relative and absolute branches automatically get divided by two in this "byte mode". The default is the organization in 16 bit word as used by the original Atmel assembler. This may explicitly be selected by using the argument `codesegsize=1`.

## Z80UNDOC

As one might guess, Zilog did not make any syntax definitions for the undocumented instructions; furthermore, not everyone might know the full set. It might therefore make sense to list all instructions at this place:

Similar to a Z380, it is possible to access the byte halves of `IX` and `IY` separately. In detail, these are the instructions that allow this:

```asm
INC Rx              LD R,Rx             LD  Rx,n
DEC Rx              LD Rx,R             LD  Rx,Ry
ADD/ADC/SUB/SBC/AND/XOR/OR/CP A,Rx
```

`Rx` and `Ry` are synonyms for `IXL`, `IXU`, `IYL` or `IYU`. Keep however in mind that in the case of `LD Rx,Ry`, both registers must be part of the same index register.

The coding of shift instructions leaves an undefined bit combination which is now accessible as the `SLIA` or `SLS` instruction. `SLIA/SLS` works like `SLA` with the difference of entering a 1 into bit position 0. Like all other shift instructions, `SLIA` also allows another undocumented variant:

```asm
        SLIA    R,(XY+d)
```

In this case, `R` is an arbitrary 8-bit register (excluding index register halves...), and `(XY+d)` is a normal indexed address. This operation has the additional effect of copying the result into the register. This also works for the `RES` and `SET` instructions:

```asm
        SET/RES R,n,(XY+d)
```

Furthermore, two hidden I/O instructions exist:

```asm
        IN      (C) resp. TSTI
        OUT     (C),0
```

Their operation should be clear.

**CAUTION!** No one can guarantee that all mask revisions of the Z80 execute these instructions, and the Z80's successors will react with traps if they find one of these instructions. Use them on your own risk...

## Z380

As this processor was designed as a grandchild of the still most popular 8-bit microprocessor, it was a sine-qua-non design target to execute existing Z80 programs without modification (of course, they execute a bit faster, roughly by a factor of 10...). Therefore, all extended features can be enabled after a reset by setting two bits which are named XM (eXtended Mode, i.e. a 32-bit instead of a 16-bit address space) respectively LW (long word mode, i.e. 32-bit instead of 16-bit operands). One has to inform AS about their current setting with the instructions `EXTMODE` resp. `LWORDMODE`, to enable AS to check addresses and constants against the correct upper limits. The toggle between 32- and 16-bit instruction of course only influences instructions that are available in a 32-bit variant. Unfortunately, the Z380 currently offers such variants only for load and store instructions; arithmetic can only be done in 16 bits. Zilog really should do something about this, otherwise the most positive description for the Z380 would be "16-bit processor with 32-bit extensions"...

The whole thing becomes complicated by the ability to override the operand size set by LW with the instruction prefixes `DDIR W` resp. `DDIR LW`. AS will note the occurrence of such instructions and will toggle setting for the instruction following directly. By the way, one should never explicitly use other `DDIR` variants than `W` resp. `LW`, as AS will introduce them automatically when an operand is discovered that is too long. Explicit usage might puzzle AS. The automatism is so powerful that in a case like this:

```asm
        DDIR    LW
        LD      BC,12345678h
```

the necessary `IW` prefix will automatically be merged into the previous instruction, resulting in

```asm
        DDIR    LW,IW
        LD      BC,12345668h
```

The machine code that was first created for `DDIR LW` is retracted and replaced, which is signified with an `R` in the listing.

## Z8, Super8, and eZ8

The CPU core contained in the Z8 microcontrollers does not contain any specific registers. Instead, a block of 16 consecutive cells of the internal address space (contains RAM and I/O registers) may be used as 'work registers' and be addressed with 4-bit addresses. The RP registers define which memory block is used as work registers: on a classic Z8, bits 4 to 7 of RP define the 'offset' that is added to a 4-bit work register address to get a complete 8-bit address. The Super8 core features two register pointers (`RP0` and `RP1`), which allow mapping the lower and upper half of work registers to separate places.

Usually, one refers to work registers as R0...R15 in assembly statements. It is however also possible to regard work registers as an efficient way to address a block of memory addresses in internal RAM.

The `ASSUME` statement is used to inform AS about the current value of `RP`. AS is then capable to automatically decide whether an address in internal RAM may be reached with a 4-bit or 8-bit address. This may be used to assign symbolic names to work registers:

```asm
op1     equ     040h
op2     equ     041h

        srp     #040h
        assume  rp:040h

        ld      op1,op2         ; equal to ld r0,r1
```

Note that though the Super8 does not have an RP register (only RP0 and RP1), RP as argument to `ASSUME` is still allowed - it will set the assumed values of RP0 and RP1 to `value` resp. `value + 8`, as the `SRP` machine instruction does on the Super 8 core.

Opposed to the original Zilog assembler, it is not necessary to explicitly specify 'work register addressing' with a prefixed exclamation mark. AS however also understands this syntax - a prefixed exclamation mark enforces 4-bit addressing, even when the address does not lie within the 16-address block defined by RP (AS will issue a warning in that case). Vice versa, a prefixed `>` character enforces 8-bit addressing even when the address is within the current 16-address block.

The eZ8 takes this 'game' to the next level: the internal address space now has 12 instead of 8 bits. To assure compatibility with the old Z8 core, Zilog placed the additional 4 bits in the _lower_ four bits of RP. For instance, an RP value of 12h defines an address window from 210h to 21fh.

At the same time, the lower four bits of RP define a window of 256 addresses that can be addressed with 8-bit addresses. The mechanism to automatically select between 8- and 12-bit addresses is analogous. 'Long' 12-bit addresses may be enforced by prefixing two `>` characters.

## Z8000

A Z8001/8003 may be operated in one of two modes:

- _Non-Segmented_: The memory address space is limited to 64 KBytes, and all addresses are 'simple' linear 16 bit addresses. Address registers are single 16 bit registers (Rn), and absolute addresses within instructions are one byte long.
- _Segmented_: Memory is structured into up to 128 segments of up to 64 KBytes size. Addresses consist of a 7 bit segment number and a 16 bit offset. Address registers are register pairs (RRn). Absolute addresses in instructions occupy two 16 bit words, unless the offset is smaller than 256.

The operation mode (segmented or non-segmented) therefore has an influence on the generated code and is selected implicitly via the selected processor type. For instance, if the target is a Z8001 in non-segmented mode, use Z8002 as target.

However, similar to the 8086, there is no 'real' support for a segmented memory model in AS. In segmented mode, the segment number is simply interpreted as the upper seven bits of a virtually linear address space. Though this is not what Zilog intended, it is the way the segment number was used on the Z8001 if the system had no MMU.

AS in general implements the Z8000 machine instruction syntax as it is specified by Zilog in its manuals. However, there are assemblers that support extensions or variations of the syntax. AS implements a few of them as well:

### Conditions

In addition to the conditions defined by Zilog, the following alternative names are defined:

| Alternate | Zilog | Meaning                     |
| :-------- | :---- | :-------------------------- |
| `ZR`      | `Z`   | `Z = 1`                     |
| `CY`      | `C`   | `C = 1`                     |
| `LLE`     | `ULE` | `(C OR Z) = 1`              |
| `LGE`     | `UGE` | `C = 0`                     |
| `LGT`     | `UGT` | `((C = 0) AND (Z = 0)) = 1` |
| `LLT`     | `ULT` | `C = 1`                     |

### Flags

`SETFLG`, `COMFLG` und `RESFLG` accept the following alternate names as
arguments:

| Alternate | Zilog | Meaning    |
| :-------- | :---- | :--------- |
| `ZR`      | `Z`   | Zero Flag  |
| `CY`      | `C`   | Carry Flag |

### Indirect Addressing

It is valid to write `Rn^` instead of `@Rn`, if the option `AMDSyntax=1` was given to the `CPU` statement. If an I/O address is addressed indirectly, this option even allows to write just `Rn`.

### Direct versus Immediate Addressing

The Zilog syntax mandates that immediate addressing has to be done by prefixing the argument with a hash character. However, if the `AMDSyntax=1` option was given to the `CPU` statement, the type of argument (label or constant) decides whether immediate or direct addressing is to be used. Immediate addressing may be forced by prefixing the argument with a circumflex, i.e. to load the address of a label into a register.

## TLCS-900(L)

These processors may run in two operating modes: on the one hand, in minimum mode, which offers almost complete source code compatibility to the Z80 and TLCS-90, and on the other hand in maximum mode, which is necessary to make full use of the processor's capabilities. The main differences between these two modes are:

- width of the registers WA, BC, DE, and HL: 16 or 32 bits;
- number of register banks: 8 or 4;
- code address space: 64 Kbytes or 16 Mbytes;
- length of return addresses: 16 or 32 bits.

To allow AS to check against the correct limits, one has to inform him about the current execution mode via the `MAXMODE` instruction (see there). The default is the minimum mode.

From this follows that, depending on the operating mode, the 16-bit resp. 32-bit versions of the bank registers have to be used for addressing, i.e. WA, BC, DE and HL for the minimum mode resp. XWA, XBC, XDE and XHL for the maximum mode. The registers XIX...XIZ and XSP are **always** 32 bits wide and therefore always have to to be used in this form for addressing; in this detail, existing Z80 code definitely has to be adapted (not including that there is no I/O space and all I/O registers are memory-mapped...).

Absolute addresses and displacements may be coded in different lengths. Without an explicit specification, AS will always use the shortest possible coding. This includes eliminating a zero displacement, i.e. `(XIX+0)` becomes `(XIX)`. If a certain length is needed, it may be forced by appending a suffix (:8, :16, :24) to the displacement resp. the address.

The syntax chosen by Toshiba is a bit unfortunate in the respect of choosing an single quote (`'`) to reference the previous register bank. The processor independent parts of AS already use this character to mark character constants. In an instruction like

```asm
        ld      wa',wa   ,
```

AS will not recognize the comma for parameter separation. This problem can be circumvented by usage of an inverse single quote ('), for example

```asm
        ld      wa`,wa
```

Toshiba delivers an own assembler for the TLCS-900 series (TAS900), which is different from AS in the following points:

#### Symbol Conventions

- TAS900 differentiates symbol names only on the first 32 characters. In contrast, AS always stores symbol names with the full length (up to 255 characters) and uses them all for differentiation.
- TAS900 allows to write integer constants either in Intel or C notation (with a 0 prefix for octal or a 0x prefix for hexadecimal constants). By default, AS only supports the Intel notation. With the help of the `RELAXED` instruction, one also gets the C notation among others.
- AS does not distinguish between upper and lower case. In contrast, TAS900 differentiates between upper- and lowercase letters in symbol names. One needs to engage the `-u` command line option to force AS to do this.

#### Syntax

For many instructions, the syntax checking of AS is less strict than the checking of TAS900. In some (rare) cases, the syntax is slightly different. These extensions and changes are on the one hand for the sake of a better portability of existing Z80 codes, on the other hand they provide a simplification and better orthogonality of the assembly syntax:

- In the case of `LDA`, `JP`, and `CALL`, TAS requires that address expressions like `XIX+5` must not be placed in parentheses, as it is usually the case. For the sake of better orthogonality, AS requires parentheses for `LDA`. They are optional if `JP` resp. `CALL` are used with a simple, absolute address.
- In the case of `JP`, `CALL`, `JR`, and `SCC`, AS leaves the choice to the programmer whether to explicitly write out the default condition `T` (= true) as first parameter or not. TAS900 in contrast only allows to use the default condition implicitly (e.g. `jp (xix+5)` instead of `jp t,(xix+5)`).
- For the `EX` instruction, AS allows operand combinations which are not listed in [^Tosh900] but can be reduced to a standard combination by swapping the operands. Combinations like `EX f',f` or `EX wa,(xhl)` become possible. In contrast, TAS900 limits to the 'pure' combinations.
- AS allows to omit an increment resp. decrement of `1` when using the instructions `INC` and `DEC`. TAS900 instead forces the programmer to explicit usage of `1`.
- The similar is true for the shift instructions: If the operand is a register, TAS900 requires that even a shift count of 1 has to be written explicitly; however, when the operand is in memory, the hardware limits the shift count to 1 which must not be written in this case. With AS, a shift count of 1 is always optional and valid for all types of operands.

#### Macro Processor

The macro processor of TAS900 is an external program that operates like a preprocessor. It consists of two components: The first one is a C-like preprocessor, and the second one is a special macro language (MPL) that reminds of high level languages. The macro processor of AS instead is oriented towards "classic" macro assemblers like MASM or M80 (both programs from Microsoft). It is a fixed component of AS.

#### Output Format

TAS900 generates relocatable code that allows to link separately compiled programs to a single application. AS instead generates absolute machine code that is not linkable. There are currently no plans to extend AS in this respect.

#### Pseudo Instructions

Due to the missing linker, AS lacks a couple of pseudo instructions needed for relocatable code TAS900 implements. The following instructions are available with equal meaning:

> `EQU`, `DB`, `DW`, `ORG`, `ALIGN`, `END`, `TITLE`, `SAVE`, `RESTORE`

The latter two have an extended functionality for AS. Some TAS900 pseudo instructions can be replaced with equivalent AS instructions (see table [equivalent instructions TAS900 to AS](#table-equivalent-instructions-tas900-to-as)).

##### **Table:** equivalent instructions TAS900 to AS

| TAS900         | AS                    | meaning/function                    |
| :------------- | :-------------------- | :---------------------------------- |
| `DL <Data>`    | `DD <Data>`           | define long word constants          |
| `DSB <number>` | `DB <number> DUP (?)` | reserve bytes of memory             |
| `DSW <number>` | `DW <number> DUP (?)` | reserve words of memory             |
| `DSD <number>` | `DD <number> DUP (?)` | reserve long words of memory        |
| `$MIN[IMUM]`   | `MAXMODE OFF`         | following code runs in minimum mode |
| `$MAX[IMUM]`   | `MAXMODE ON`          | following code runs in maximum mode |
| `$SYS[TEM]`    | `SUPMODE ON`          | following code runs in system mode  |
| `$NOR[MAL]`    | `SUPMODE OFF`         | following code runs in user mode    |
| `$NOLIST`      | `LISTING OFF`         | turn off assembly listing           |
| `$LIST`        | `LISTING ON`          | turn on assembly listing            |
| `$EJECT`       | `NEWPAGE`             | start new page in listing           |

Toshiba manufactures two versions of the processor core, with the L version being an "economy version". AS will make the following differences between TLCS-900 and TLCS-900L:

- The instructions `MAX` and `NORMAL` are not allowed for the L version; the `MIN` instruction is disabled for the full version.
- The L version does not know the normal stack pointer `XNSP`/`NSP`, but instead has the interrupt nesting register `INTNEST`.

The instructions `SUPMODE` and `MAXMODE` are not influenced, just as their initial setting `OFF`. The programmer has to take care of the fact that the L version starts in maximum mode and does not have a normal mode. However, AS shows a bit of mercy against the L variant by suppressing warnings for privileged instructions.

## TLCS-90

Maybe some people might ask themselves if I mixed up the order a little bit, as Toshiba first released the TLCS-90 as an extended Z80 and afterwards the 16-bit version TLCS-900. Well, I discovered the '90 via the '900 (thank you Oliver!). The two families are quite similar, not only regarding their syntax but also in their architecture. The hints for the '90 are therefore a subset of of the chapter for the '900: As the '90 only allows shifts, increments, and decrements by one, the count need not and must not be written as the first argument. Once again, Toshiba wants to omit parentheses for memory operands of `LDA`, `JP`, and `CALL`, and once again AS requires them for the sake of orthogonality (the exact reason is of course that this way, I saved an extra in the address parser, but one does not say such a thing aloud).

Principally, the TLCS-90 series already has an address space of 1 Mbyte which is however only accessible as data space via the index registers. AS therefore does not regard the bank registers and limits the address space to 64 Kbytes. This should not limit too much as this area above is anyway only reachable via indirect addressing.

## TLCS-870

Once again Toshiba...a company quite productive at the moment! Especially this branch of the family (all Toshiba microcontrollers are quite similar in their binary coding and programming model) seems to be targeted towards the 8051 market: the method of separating the bit position from the address expression with a dot had its root in the 8051. However, it creates now exactly the sort of problems I anticipated when working on the 8051 part: On the one hand, the dot is a legal part of symbol names, but on the other hand, it is part of the address syntax. This means that AS has to separate address and bit position and must process them independently. Currently, I solved this conflict by seeking the dot starting at the **end** of the expression. This way, the last dot is regarded as the separator, and further dots stay parts of the address. I continue to urge everyone to omit dots in symbol names, they will lead to ambiguities:

```asm
        LD      CF,A.7  ; accumulator bit 7 to carry
        LD      C,A.7   ; constant 'A.7' to accumulator
```

## TLCS-47

This family of 4-bit microcontrollers should mark the low end of what is supportable by AS. Apart from the `ASSUME` instruction for the data bank register (see there), there is only one thing that is worth mentioning: In the data and I/O segment, nibbles are reserved instead of byte (it's a 4-bitter...). The situation is similar to the bit data segment of the 8051, where a `DB` reserves a single bit, with the difference that we are dealing with nibbles.

Toshiba defined an "extended instruction set" for this processor family to facilitate the work with their limited instruction set. In the case of AS, it is defined in the include file `STDDEF47.INC`. However, some instructions that could not be realized as macros are "built-ins" and are therefore also available without the include file:

- the `B` instruction that automatically chooses the optimal version of the jump instruction (`BSS; BS`, or `BSL`);
- `LD` in the variant of `HL` with an immediate operand;
- `ROLC` and `RORC` with a shift amplitude higher than one.

## TLCS-9000

This was the first time that I implemented a processor for AS which was not yet available at that point of time. And unfortunately, I received back then information that Toshiba had decided no to market this processor at all. This of course had the result that the TLCS-9000 part of the assembler

1.  was a "paper design", i.e. there was so far no chance to test it on real hardware and
2.  the documentation for the '9000 I could get hold [^Tosh9000] of was preliminary and was unclear in a couple of detail issues.

So in effect, this target went into 'dormant mode'...

...Fast forward 20: all of a sudden, people are contacting me and tell me that Toshiba actually did sell TLCS-9000 chips to customers, and they ask for documentation to do reverse engineering. Maybe this will shed some light on the remaining uncertainties. Nevertheless, errors in this code generator are quite possible (and will of course be fixed!). At least the few examples listed in [^Tosh9000] are assembled correctly.

Displacements included in machine instructions may only have a certain maximum length (e.g. 9 or 13 bits). In case the displacement is longer, a prefix containing the 'upper bits' must be prepended to the instruction. AS will automatically insert such prefixes when necessary, however it is also possible to force usage of a prefix by adding a leading `>`. An example for this:

```asm
      ld:g.b  (0h),0       ; no prefix
      ld:g.b  (400000h),0  ; prefix added automatically
      ld:g.b  (>0h),0      ; forced prefix
```

## TC9331

Toshiba supplied a (DOS-based) assembler for this processor which was named ASM31T. This assembler supports a number of syntax elements which could not be mapped on the capabilities of AS without risking incompatibilities for existing source files for other targets. The following issues might require changes on programs written for ASM31T:

- ASM31T supports C-like comments (`/* ... */`) which may also span multiple lines. Such comments are not supported by AS and have to be replaced by comments beginning with a semicolon.
- Similar to ASM31T, AS supports comments with round parentheses (`( ... )`), however only within a single command argument. Should such a comment contain a comma, this comma will be treated like an argument separator and the comment will not be skipped when parsing the arguments.
- ASM31T allows symbol and label names containing a dash. AS does not allow this, because the dash is regarded to be the subtraction operator. It would be unclear whether an expression like `end-start` represents a single symbol or the difference of two symbols.
- ASM31T requires an `END` statement as the last statement of the program; this is optional for AS.

Furthermore, AS currently lacks the capabilities to detect conflicting uses of functional units in a machine instructions. Toshiba's documentation is a bit difficult to understand in this respect...

## 29xxx

As it was already described in the discussion of the `ASSUME` instruction, AS can use the information about the current setting of the RBP register to detect accesses to privileged registers in user mode. This ability is of course limited to direct accesses (i.e. without using the registers IPA...IPC), and there is one more pitfall: as local registers (registers with a number `> 127`) are addressed relative to the stack pointer, but the bits in RBP always refer to absolute numbers, the check is NOT done for local registers. An extension would require AS to know always the absolute value of SP, which would at least fail for recursive subroutines...

## 80C16x

As it was already explained in the discussion of the `ASSUME` instruction, AS tries to hide the fact that the processor has more physical than logical RAM as far as possible. Please keep in mind that the DPP registers are valid only for data accesses and only have an influence on absolute addressing, neither on indirect nor on indexed addresses. AS cannot know which value the computed address may take at runtime... The paging unit unfortunately does not operate for code accesses so one has to work with explicit long or short `CALL`s, `JMP`s, or `RET`s. At least for the "universal" instructions `CALL` and `JMP`, AS will automatically use the shortest variant, but at least for the RET one should know where the call came from. `JMPS` and `CALLS` principally require to write segment and address separately, but AS is written in a way that it can split an address on its own, e.g. one can write

```asm
        jmps    12345h
```

instead of

```asm
        jmps    1,2345h
```

Unfortunately, not all details of the chip's internal instruction pipeline are hidden: if CP (register bank address), SP (stack), or one of the paging registers are modified, their value is not available for the instruction immediately following. AS tries to detect such situations and will issue a warning in such cases. Once again, this mechanism only works for direct accesses.

Bits defined with the `BIT` instruction are internally stored as a 12-bit word, containing the address in bits 4...11 and the bit position in the four LSBs. This order allows to refer the next resp. previous bit by incrementing or decrementing the address. This will however not work for explicit bit specifications when a word boundary is crossed. For example, the following expression will result in a range check error:

```asm
        bclr    r5.15+1
```

We need a `BIT` in this situation:

```asm
msb     bit     r5.15
        .
        .
        bclr    msb+1
```

The SFR area was doubled for the 80C167/165/163: bit 12 flags that a bit lies in the second part. Siemens unfortunately did not foresee that 256 SFRs (128 of them bit addressable) would not suffice for successors of the 80C166. As a result, it would be impossible to reach the second SFR area from F000H...F1DFH with short addresses or bit instructions if the developers had not included a toggle instruction:

```asm
        EXTR    #n
```

This instruction has the effect that for the next `n` instructions (`0 < n < 5`), it is possible to address the alternate SFR space instead of the normal one. AS does not only generate the appropriate machine code when it encounters this instruction. It also sets an internal flag that will only allow accesses to the alternate SFR space for the next `n` instructions. Of course, they may not contain jumps... Of course, it is always possible to define bits from either area at any place, and it is always possible to reach all registers with absolute addresses. In contrast, short and bit addressing only works for one area at a time, attempts contradicting to this will result in an error message.

The situation is similar for prefix instructions and absolute resp. indirect addressing: as the prefix argument and the address expression cannot always be evaluated at assembly time, chances for checking are limited and AS will limit itself to warnings...in detail, the situation is as follows:

- fixed specification of a 64K bank with `EXTS` or `EXTSR`: the address expression directly contains the lower 16 bits of the target address. If the prefix and the following instruction have a constant operand, AS will check if the the prefix argument and bits 16...23 of the target address are equal.
- fixed specification of a 16K page with `EXTP` or `EXTPR`: the address expression directly contains the lower 14 bits of the target address. Bits 14 and 15 are fixed to 0, as the processor ignores them in this mode. If the prefix and the following instruction have a constant operand, AS will check if the the prefix argument and bits 14...23 of the target address are equal.

An example to clarify things a bit (the DPP registers have their reset values):

```asm
        extp    #7,#1      ; range from 112K...128K
        mov     r0,1cdefh  ; results in address 0defh in code
        mov     r0,1cdefh  ; -->warning
        exts    #1,#1      ; range from 64K...128K
        mov     r0,1cdefh  ; results in address 0cdefh in code
        mov     r0,1cdefh  ; -->warning
```

## PIC16C5x/16C8x

Similar to the MCS-48 family, the PICs split their program memory into several banks because the opcode does not offer enough space for a complete address. AS uses the same automatism for the instructions `CALL` and `GOTO`, i.e. the PA bits in the status word are set according to the start and target address. However, this procedure is far more problematic compared to the 48's:

1.  The instructions are not any more one word long (up to three words). Therefore, it is not guaranteed that they can be skipped with a conditional branch.
2.  It is possible that the program counter crosses a page boundary while the program sequence is executed. The setting of PA bits AS assumes may be different from reality.

The instructions that operate on register W and another register normally require a second parameter that specifies whether the result shall be stored in W or the register. Under AS, it is valid to omit the second parameter. The assumed target then depends upon the operation's type: For unary operations, the result is by default stored back into the register. These instructions are:

> `COMF`, `DECF`, `DECFSZ`, `INCF`, `INCFSZ`, `RLF`, `RRF`, and `SWAPF`

The other operations by default regard W as an accumulator:

> `ADDWF`, `ANDWF`, `IORWF`, `MOVF`, `SUBWF`, and `XORWF`

The syntax defined by Microchip to write literals is quite obscure and reminds of the syntax used on IBM 360/370 systems (greetings from the stone-age...). To avoid introducing another branch into the parser, with AS one has to write constants in the Motorola syntax (optionally Intel or C in `RELAXED` mode).

## PIC 17C4x

With two exceptions, the same hints are valid as for its two smaller brothers: the corresponding include file only contains register definitions, and the problems concerning jump instructions are much smaller. The only exception is the `LCALL` instruction, which allows a jump with a 16-bit address. It is translated with the following "macro":

```asm
        MOVLW   <addr15...8>
        MOWF    3
        LCALL   <addr0...7>
```

## SX20/28

The limited length of the instruction word does not permit specifying a complete program memory address (11 bits) or data memory address (8 bits). The CPU core augments the truncated address from the instruction word with the PA bits from the STATUS registers, respectively with the upper bits of the FSR register. It is possible to inform the assembler via `ASSUME` instructions about the contents of these two registers. In case that addresses are used that are inaccessible with th current values, a warning is issued.

## ST6

These processors have the ability to map their code ROM page-wise into the data area. I am not keen on repeating the whole discussion of the `ASSUME` instruction at this place, so I refer to the corresponding section ([ST6](pseudo-instructions.md#st6)) for an explanation how to read constants out of the code ROM without too much headache.

Some builtin "macros" show up when one analyzes the instruction set a bit more in detail. The instructions I found are listed in table [Hidden Macros in the ST62's Instruction Set](#table-hidden-macros-in-the-st62s-instruction-set) (there are probably even more...):

##### **Table:** Hidden Macros in the ST62's Instruction Set

| instruction | in reality   |
| :---------- | :----------- |
| `CLR A`     | `SUB A,A`    |
| `SLA A`     | `ADD A,A`    |
| `CLR addr`  | `LDI addr,0` |
| `NOP`       | `JRZ PC+1`   |

Especially the last case is a bit astonishing... unfortunately, some instructions are really missing. For example, there is an `AND` instruction but no `OR`...not to speak of an `XOR`. For this reason, the include file `STDDEF62.INC` contains also some helping macros (additionally to register definitions).

The original assembler AST6 delivered by SGS-Thomson partially uses different pseudo instructions than AS. Apart from the fact that AS does not mark pseudo instructions with a leading dot, the following instructions are identical:

```asm
      ASCII, ASCIZ, BLOCK, BYTE, END, ENDM, EQU, ERROR, MACRO,
      ORG, TITLE, WARNING
```

Table [Equivalent Instructions AST6 ↔ AS](#table-equivalent-instructions-ast6--as) shows the instructions which have AS counterparts with similar function.

##### **Table:** Equivalent Instructions AST6 ↔ AS

| AST6       | AS                    | meaning/function             |
| :--------- | :-------------------- | :--------------------------- |
| `.DISPLAY` | `MESSAGE`             | output message               |
| `.EJECT`   | `NEWPAGE`             | new page in assembly listing |
| `.ELSE`    | `ELSEIF`              | conditional assembly         |
| `.ENDC`    | `ENDIF`               | conditional assembly         |
| `.IFC`     | `IF...`               | conditional assembly         |
| `.INPUT`   | `INCLUDE`             | insert include file          |
| `.LIST`    | `LISTING, MACEXP_DFT` | settings for listing         |
| `.PL`      | `PAGE`                | page length of listing       |
| `.ROMSIZE` | `CPU`                 | set target processor         |
| `.VERS`    | `VERSION` (symbol)    | query version                |
| `.SET`     | `EVAL`                | redefine variables           |

## ST7

In [^ST7Man], the `.w` postfix to signify 16-bit addresses is only defined for memory indirect operands. It is used to mark that a 16-bit address is stored at a zero page address. AS additionally allows this postfix for absolute addresses or displacements of indirect address expressions to force 16-bit displacements in spite of an 8-bit value (0...255).

## ST9

The ST9's bit addressing capabilities are quite limited: except for the `BTSET` instruction, only bits within the current set of working registers are accessible. A bit address is therefore of the following style:

```asm
        rn.[!]b
```

whereby `!` means an optional complement of a source operand. If a bit is defined symbolically, the bit's register number is stored in bits 7...4, the bit's position is stored in bits 3...1 and the optional complement is kept in bit 0. AS distinguishes explicit and symbolic bit addresses by the missing dot. A bit's symbolic name therefore must not contain a dot, thought it would be legal in respect to the general symbol name conventions. It is also valid to invert a symbolically referred bit:

```asm
bit2    bit     r5.3
        .
        .
        bld     r0.0,!bit2
```

This opportunity also allows to undo an inversion that was done at definition of the symbol.

The include file `REGST9.INC` defines the symbolic names of all on-chip registers and their associated bits. Keep however in mind that the bit definitions only work after previously setting the working register bank to the address of these peripheral registers!

In contrast to the definition file delivered with the AST9 assembler from SGS-Thomson, the names of peripheral register names are only defined as general registers (`R...`), not also as working registers (`r...`). The reason for this is that AS does not support register aliases; a tribute to assembly speed.

## 6804

To be honest: I only implemented this processor in AS to quarrel about SGS-Thomson's peculiar behaviour. When I first read the 6804's data book, the "incomplete" instruction set and the built-in macros immediately reminded me of the ST62 series manufactured by the same company. A more thorough comparison of the opcodes gave surprising insights: A 6804 opcode can be generated by taking the equivalent ST62 opcode and mirroring all the bits! So Thomson obviously did a bit of processor core recycling...which would be all right if they would not try to hide this: different peripherals, motorola instead of Zilog-style syntax, and the awful detail of **not** mirroring operand fields in the opcode (e.g. bit fields containing displacements). The last item is also the reason that finally convinced me to support the 6804 in AS. I personally can only guess which department at Thomson did the copy...

In contrast to its ST62 counterpart, the include file for the 6804 does not contain instruction macros that help a bit to deal with the limited machine instruction set. This is left as an exercise to the reader!

## TMS3201x

It seems that every semiconductor's ambition is to invent an own notation for hexadecimal numbers. Texas Instrument took an especially eccentric approach for these processors: a \> sign as prefix! The support of such a format in AS would have lead to extreme conflicts with AS's compare and shift operators. I therefore decided to use the Intel notation, which is what TI also uses for the 340x0 series and the 3201x's successors...

The instruction word of these processors unfortunately does not have enough bits to store all 8 bits for direct addressing. This is why the data address space is split into two banks of 128 words. AS principally regards the data address space as a linear segment of 256 words and automatically clears bit 7 on direct accesses (an exception is the `SST` instruction that can only write to the upper bank). The programmer has to take care that the bank flag always has the correct value!

Another hint that is well hidden in the data book: The `SUBC` instruction internally needs more than one clock for completion, but the control unit already continues to execute the next instruction. An instruction following `SUBC` therefore may not access the accumulator. AS does not check for such conditions!

## TMS320C2x

As I did not write this code generator myself (that does not lower its quality by any standard), I can only roughly line out why there are some instructions that force a prefixed label to be untyped, i.e. not assigned to any specific address space: The 2x series of TMS signal processors has a code and a data segment which are both 64 Kbytes large. Depending on external circuitry, code and data space may overlap, e.g. to allow storage of constants in the code area and access them as data. Data storage in the code segment may be necessary because older versions of AS assume that the data segment only consists of RAM that cannot have a defined power-on state in a single board system. They therefore reject storage of contents in other segments than `CODE`. Without the feature of making symbols untyped, AS would punish every access to a constant in code space with a warning ("symbol out of wrong segment"). To say it in detail, the following instructions make labels untyped:

> `BSS`, `STRING`, `RSTRING`, `BYTE`, `WORD`, `LONG`, `FLOAT`, `DOUBLE`, `EFLOAT`, `BFLOAT` and `TFLOAT`

If one needs a typed label in front of one of these instructions, one can work around this by placing the label in a separate line just before the pseudo instruction itself. On the other hand, it is possible to place an untyped label in front of another pseudo instruction by defining the label with `EQU`, e.g.

```asm
<name>  EQU     $
```

## TMS320C3x/C4x

The syntax detail that created the biggest amount of headache for me while implementing this processor family is the splitting of parallel instructions into two separate source code lines. Fortunately, both instructions of such a construct are also valid single instructions. AS therefore first generates the code for the first instruction and replaces it by the parallel machine code when a parallel construct is encountered in the second line. This operation can be noticed in the assembly listing by the machine code address that does not advance and the double dot replaced with a `R`.

Compared to the TI assembler, AS is not as flexible regarding the position of the double lines that signify a parallel operation (`||`): One either has to place them like a label (starting in the first column) or to prepend them to the second mnemonic. The line parser of AS will run into trouble if you do something else...

## TMS9900

Similar to most older TI microprocessor families, TI used an own format for hexadecimal and binary constants. AS instead favours the Intel syntax which is also common for newer processor designs from TI.

The TI syntax for registers allows to use a simple integer number between 0 and 15 instead of a real name (`Rx` or `WRx`). This has two consequences:

- `R0...R15` resp. `WR0...WR15` are simple predefined integer symbols with values from 0 to 15, and the definition of register aliases is a simple matter of `EQU`.
- In contrast to several other processors, I cannot offer the additional AS feature that allows to omit the character signifying absolute addressing (a sign in this case). As a missing character would mean register numbers (from 0 to 15) in this case, it was not possible to offer the optional omission.

Furthermore, TI sometimes uses `Rx` to name registers and `WRx` at other places...currently both variants are recognized by AS.

## TMS70Cxx

This processor family belongs to the older families developed by TI and therefore TI's assemblers use their proprietary syntax for hexadecimal resp. binary constants (a prefixed `<` resp. `?` character). As this format could not be realized for AS, the Intel syntax is used by default. This is the format TI to which also switched over when introducing the successors, of this family, the 370 series of microcontrollers. Upon a closer inspection of both's machine instruction set, one discovers that about 80% of all instruction are binary upward compatible, and that also the assembly syntax is almost identical - but unfortunately only almost. TI also took the chance to make the syntax more orthogonal and simple. I tried to introduce the majority of these changes also into the 7000's instruction set:

- It is valid to use the more common `#` sign for immediate addressing instead of the percent sign.
- If a port address (`P...`) is used as source or destination in a `AND`, `BTJO`, `BTJZ`, `MOV`, `OR`, or `XOR` instruction, it is not necessary to use the mnemonic variant with an appended `P` - the general form is sufficient.
- The prefixed `@` sign for absolute or B-relative addressing may be omitted.
- Instead of `CMPA, CMP` with `A` as target may be written.
- Instead of `LDA` resp. `STA`, one can simply use the `MOV` instruction with `A` as source resp. destination.
- One can write `MOVW` instead of `MOVD`.
- It is valid to abbreviate `RETS` resp. `RETI` as `RTS` resp. `RTI`.
- `TSTA` resp. `TSTB` may be written as `TST A` resp. `TST B`.
- `XCHB B` is an alias for `TSTB`.

An important note: these variants are only allowed for the TMS70Cxx - the corresponding 7000 variants are not allowed for the 370 series!

## TMS370xxx

Though these processors do not have specialized instructions for bit manipulation, the assembler creates (with the help of the `DBIT` instruction - see there) the illusion as if single bits were addressable. To achieve this, the `DBIT` instructions stores an address along with a bit position into an integer symbol which may then be used as an argument to the pseudo instructions `SBIT0, SBIT1, CMPBIT, JBIT0`, and `JBIT1`. These are translated into the instructions `OR, AND, XOR, BTJZ`, and `BTJO` with an appropriate bit mask.

There is nothing magic about these bit symbols, they are simple integer values that contain the address in their lower and the bit position in their upper half. One could construct bit symbols without the `DBIT` instruction, like this:

```asm
defbit  macro   name,bit,addr
name    equ     addr+(bit<<16)
        endm
```

but this technique would not lead to the `EQU`-style syntax defined by TI (the symbol to be defined replaces the label field in a line). **CAUTION!** Though `DBIT` allows an arbitrary address, the pseudo instructions can only operate with addresses either in the range from 0...255 or 1000h...10ffh. The processor does not have an absolute addressing mode for other memory ranges...

## MSP430(X)

The MSP was designed to be a RISC processor with a minimal power consumption. The set of machine instructions was therefore reduced to the absolute minimum (RISC processors do not have a microcode ROM so every additional instruction has to be implemented with additional silicon that increases power consumption). A number of instructions that are hardwired for other processors are therefore emulated with other instructions. Older versions of AS implemented these instructions via macros in the file `REGMSP.INC`. If one did not include this file, you got error messages for more than half of the instructions defined by TI. This has been changed in recent versions: as part of adding the 430X instruction set, implementation of these instructions was moved into the assembler's core. `REGMSP.INC` now only contains addresses of I/O registers. If you need the old macros for some reason, they have been moved to the file `EMULMSP.INC`.

Instruction emulation also covers some special cases not handled by the original TI assembler. For instance,

```asm
        rlc  @r6+
```

is automatically assembled as

```asm
        addc @r6+,-2(r6)
```

## TMS1000

At last, world's first microcontroller finally also supported in AS - it took long to fill this gap, but now it is done. This target has some pitfalls that will be discussed shortly in this section.

First, the instruction set of these controllers is partially defined via the ROM mask, i.e. the function of some opcodes may be freely defined to some degree. AS only knows the instructions and codings that are described as default codings in [^TMS1000PGMRef]. If you have a special application with an instruction set deviating from this, you may define and modify instructions via macros and the `DB` instruction.

Furthermore, keep in mind that branches and subroutine calls only contain the lower 6 bits of the target address. The upper 4 resp. 5 bits are fetched from page and chapter registers that have to be set beforehand. AS cannot check whether these registers have been set correctly by the programmer! At least for the cas of staying in the same chapter, there are the assembler pseudo instructions `CALLL` resp. `BL` that combine an `LDP` and `CALL/BR` instruction. Regarding the limited amount of program memory, this is a convenient yet inefficient variant.

## COP8 & SC/MP

National unfortunately also decided to use the syntax well known from IBM mainframes (and much hated by me...) to write non-decimal integer constants. Just like with other processors, this does not work with AS's parser. ASMCOP however fortunately also seems to allow the C syntax, which is why this became the default for the COP series and the SC/MP...

## SC144xxx

Originally, National offered a relatively simple assembler for this series of DECT controllers. An much more powerful assembler has been announced by IAR, but it is not available up to now. However, since the development tools made by IAR are as much target-independent as possible, one can roughly estimate the pseudo instructions it will support by looking at other available target platforms. With this in mind, the (few) SC144xx-specific instructions `DC`, `DC8`, `DW16`, `DS`, `DS8`, `DS16`, `DW` were designed. Of course, I didn't want to reinvent the wheel for pseudo instructions whose functionality is already part of the AS core. Therefore, here is a little table with equivalences. The statements `ALIGN`, `END`, `ENDM`, `EXITM`, `MACRO`, `ORG`, `RADIX`, `SET`, and `REPT` both exist for the IAR assembler and AS and have same functionality. Changes are needed for the following instructions:

| IAR                       | AS               | Funktion                              |
| :------------------------ | :--------------- | :------------------------------------ |
| `#include`                | `include`        | include file                          |
| `#define`                 | `SET, EQU`       | define symbol                         |
| `#elif`, `ELIF`, `ELSEIF` | `ELSEIF`         | start another IF branch               |
| `#else`, `ELSE`           | `ELSE`           | last branch of an IF construct        |
| `#endif`, `ENDIF`         | `ENDIF`          | ends an IF construct                  |
| `#error`                  | `ERROR`, `FATAL` | create error message                  |
| `#if, IF`                 | `IF`             | start an IF construct                 |
| `#ifdef`                  | `IFDEF`          | symbol defined ?                      |
| `#ifndef`                 | `IFNDEF`         | symbol not defined ?                  |
| `#message`                | `MESSAGE`        | output message                        |
| `=`, `DEFINE`, `EQU`      | `=`, `EQU`       | fixed value assignment                |
| `EVEN`                    | `ALIGN 2`        | force PC to be equal                  |
| `COL`, `PAGSIZ`           | `PAGE`           | set page size for listing             |
| `ENDR`                    | `ENDM`           | end REPT construct                    |
| `LSTCND`, `LSTOUT`        | `LISTING`        | control amount of listing             |
| `LSTEXP`, `LSTREP`        | `MACEXP`         | list expanded macros?                 |
| `LSTXRF`                  | `<command line>` | generate cross reference              |
| `PAGE`                    | `NEWPAGE`        | new page in listing                   |
| `REPTC`                   | `IRPC`           | repetition with character replacement |

There is no direct equivalent for `CASEON`, `CASEOFF`, `LOCAL`, `LSTPAG`, `#undef`, and `REPTI`.

A 100% equivalent is of course impossible as long as there is no C-like preprocessor in AS. C-like comments unfortunately are also impossible at the moment. Caution: When modifying IAR codes for AS, do not forget to move converted preprocessor statements out of column 1 as AS reserves this column exclusively for labels!

## NS32xxx

As one might expect from a CISC processor, the NS32xxx series provides sophisticated and complex addressing modes. National defied the assembly syntax for each of them in its manuals, and this is also the syntax AS implements. However, as for every architecture that was supported by third-party tools, there are deviations and extensions, and I added a few of them to AS:

The syntax to use PC-relative addressing, as defined by National, is:

```asm
     movb r0,*+disp
```

This of course quite clearly expresses what is happening at runtime, one however has to compute the distance himself if a certain memory location is to be addressed:

```asm
     movb r0,*+(addr-*)
```

The first simplification is that under certain conditions, it is sufficient to just write:

```asm
     movb r0,addr
```

since absolute addressing is marked by a prefix. This is allowed under the following conditions:

- Immediate addressing is not allowed, e.g. because the operand is the destination and there is no risk os ambiguities.
- An index extension is used (appended in square brackets), which must not be combined with immediate addressing.

As an alternative, AS also supports the following way to use PC-relative addressing:

```asm
     movb r0,addr(pc)
```

Analog to the 68000, the distance is computed automatically.

The external mode, which is written this way in National syntax:

```asm
     movb r0,ext(disp1)+disp2
```

there is another supported syntax variant:

```asm
     movb r0,disp2(disp1(ext))
```

which used to be common in UNIX environments.

## μPD78(C)1x

For relative, unconditional instructions, there is the `JR` instruction (branch distance -32...+31, one byte), and the `JRE` instruction (branch distance -256...+255, two bytes). AS furthermore knows the `J` pseudo instruction, which automatically selects the shortest possible variant.

Architecture and instruction set of these processors are coarsely related to the Intel 8080/8085 - thi is also true for the mnemonics. The addressing mode (direct, indirect, immediate) is packed into the mnemonic, and 16 bit registers (BC, DE, HL) are written with just one letter. However, since NEC itself also uses at some places written-out register names and parentheses to signify indirect addressing, I decided to support some alternative notations next to the 'official' ones. Some non-NEC tools like disassemblers seem to use these notations either:

- It is allowed to use `BC`, `(B)`, or `(BC)` instead of `B`.
- It is allowed to use `DE`, `(D)`, or `(DE)` instead of `D`.
- It is allowed to use `HL`, `(H)`, or `(HL)` instead of `H`.
- It is allowed to use `DE+`, `(D+)`, `(DE+)`, or `(DE)+` instead of `D+`.
- It is allowed to use `HL+`, `(H+)`, `(HL+)`, or `(HL)+` instead of `H+`.
- It is allowed to use `DE-`, `(D-)`, `(DE-)`, or `(DE)-` instead of `D-`.
- It is allowed to use `HL-`, `(H-)`, `(HL-)`, or `(HL)-` instead of `H-`.
- It is allowed to use `DE++`, `(D++)`, `(DE++)`, or `(DE)++` instead of `D++`.
- It is allowed to use `HL++`, `(H++)`, `(HL++)`, or `(HL)++` instead of `H++`.
- It is allowed to use `DE–`, `(D–)`, `(DE–)`, or `(DE)–` instead of `D–`.
- It is allowed to use `HL–`, `(H–)`, `(HL–)`, or `(HL)–` instead of `H–`.
- It is allowed to use `HL+A`, `A+H`, `A+HL`, `(H+A)`, `(HL+A)`, `(A+H)`, or `(A+HL)` instead of `H+A`.
- It is allowed to use `HL+B`, `B+H`, `B+HL`, `(H+B)`, `(HL+B)`, `(B+H)`, or `(B+HL)` instead of `H+B`.
- It is allowed to use `HL+EA`, `EA+H`, `EA+HL`, `(H+EA)`, `(HL+EA)`, `(EA+H)`, or `(EA+HL)` instead of `H+EA`.

## 75K0

Similar to other processors, the assembly language of the 75 series also knows pseudo bit operands, i.e. it is possible to assign a combination of address and bit number to a symbol that can then be used as an argument for bit oriented instructions just like explicit expressions. The following three instructions for example generate the same code:

```asm
ADM     sfr     0fd8h
SOC     bit     ADM.3

        skt     0fd8h.3
        skt     ADM.3
        skt     SOC
```

AS distinguishes direct and symbolic bit accesses by the missing dot in symbolic names; it is therefore forbidden to use dots in symbol names to avoid misunderstandings in the parser.

The storage format of bit symbols mostly accepts the binary coding in the machine instructions themselves: 16 bits are used, and there is a "long" and a "short" format. The short format can store the following variants:

- direct accesses to the address range from `0FBxH` to `0FFxH`
- indirect accesses in the style of `Addr.@L` (`0FC0H ≤ Addr ≤ 0FFFH`)
- indirect accesses in the style of `@H+d4.bit`

The upper byte is set to 0, the lower byte contains the bit expression coded according to [^NEC75]. The long format in contrast only knows direct addressing, but it can cover the whole address space (given a correct setting of MBS and MBE). A long expression stores bits 0...7 of the address in the lower byte, the bit position in bits 8 and 9, and a constant value of 01 in bits 10 and 11. The highest bits allow to distinguish easily between long and short addresses via a check if the upper byte is 0. Bits 12...15 contain bits 8...11 of the address; they are not needed to generate the code, but they have to be stored somewhere as the check for correct banking can only take place when the symbol is actually used.

## 78K0

NEC uses different ways to mark absolute addressing in its data books:

- absolute short: no prefix
- absolute long: prefix of `!`
- PC relative: prefix of `$`

Under AS, these prefixes are only necessary if one wants to force a certain addressing mode and the instruction allows different variants. Without a prefix, AS will automatically select the shortest variant. It should therefore rarely be necessary to use a prefix in practice.

## 78K2/78K3/78K4

Analogous to the 78K0, NEC here also uses dollar signs and exclamation marks to specify different lengths of address expressions. The selection between long and short addresses is done automatically (both in RAM and SFR areas), only relative addressing has to be selected explicitly, if an instruction supports both variants (like `BR`).

An additional remark (which is also true for the 78K0): Those who want to use Motorola syntax via `RELAXED`, might have to put hexadecimal constants in parentheses, since the leading dollar sign might be misunderstood as relative addressing...

## μPD772x

Both the 7720 and 7725 are provided by the same code generator and are extremely similar in their instruction set. One should however not believe that they are binary compatible: To get space for the longer address fields and additional instructions, the bit positions of some fields in the instruction word have changed, and the instruction length has changed from 23 to 24 bits. The code format therefore uses different header ids for both CPUs.

They both have in common that in addition to the code and data segment, there is also a ROM for storage of constants. In the case of AS, it is mapped onto the `ROMDATA` segment!

## F<sup>2</sup>MC16L

Along with the discussion of the `ASSUME` statement, it has already been mentioned that it is important to inform AS about the correct current values of all bank registers - if your program uses more than 64K RAM or 64K ROM. With these assumptions in mind, AS checks every direct memory access for attempts to access a memory location that is currently not in reach. Of course, standard situations only require knowledge of DTB and DPR for this purpose, since ADB resp. SSB/USB are only used for indirect accesses via RW2/RW6 resp. RW3/RW7 and this mechanism anyway doesn't work for indirect accesses. However, similar to the 8086, it is possible to place a prefix in front of an instruction to replace DTB by a different register. AS therefore keeps track of used segment prefixes and toggles appropriately for the next _machine instruction_. A pseudo instruction placed between the prefix and the machine instruction does _not_ reset the toggle. This is also true for pseudo instructions that store data or modify the program counter. Which doesn't make much sense anyway...

## MN161x

This target is special because there are two different code generators one may choose from. The first one was kindly provided by Haruo Asano and that may be reached via the CPU names `MN1610` resp.`MN1613`. The other one was written by me and is activated via the CPU names `MN1610ALT` resp. `MN1613ALT`. If you want to use the MN1613's extended address space of 256 KWords, or if you want to experiment with the MN1613's floating point formant, you have to use the `ALT` target.

## CDP180x

This family of processors supports both long and short branches: a short branch is only possible within the same 256 byte memory page, and a long branch is possible to any target in the 64K address space. The assembly syntax provides different mnemonics for both variants (the long variant with a leading 'L'), but there is no variant that would let the assembler decide itself between long or short. AS supports such 'pseudo instructions' as an extension:

- `JMP` becomes `BR` oder `LBR`.
- `JZ` becomes `BZ` oder `LBZ`.
- `JNZ` becomes `BNZ` oder `LBNZ`.
- `JDF` becomes `BDF` oder `LBDF`.
- `JPZ` becomes `BPZ` oder `LBPZ`.
- `JGE` becomes `BGE` oder `LBGE`.
- `JNF` becomes `BNF` oder `LBNF`.
- `JM` becomes `BM` oder `LBM`.
- `JL` becomes `BL` oder `LBL`.
- `JQ` becomes `BQ` oder `LBQ`.
- `JNQ` becomes `BNQ` oder `LBNQ`.

## KENBAK

The KENBAK-1 was developed in 1970, at a time when the first microprocessor was still three years away. One may assume that for the few hobbyists that could afford the kit back then, this was their first and only computer. As a consequence, they had nothing they could run an assembler on, the KENBAK-1 itself with its 256 bytes of memory was way too small for such a task. The preferred method was to use pre-printed tables, which had fields to fill in instructions and machine codes. Once this "programming job" was done, one would enter the machine code manually via the computer's switch row.

The effect of this is that though the KENBAK's assembly language is described in the manual, there is no real formal definition of it. When Grant Stockly released new KENBAK kits a few years ago, he did a first implementation of the KENBAK on my assembler. Unfortunately, this never went upstream. I tried to take up his ideas in my implementation, but on the other hand I also tried to offer a syntax that should be familiar to programmers of 6502, Z80 or similar processors. The following table lists the syntax differences:

<!-- markdownlint-disable MD056 -->

| Stock                                                 | Alternative                      | Remark                 |
| :---------------------------------------------------- | :------------------------------- | :--------------------- |
| **Arithmetic/Logic (ADD/SUB/LOAD/STORE/AND/OR/LNEG)** |
| _instr_ `Constant`, _Reg_, _Wert_,                    | _instr_ _Reg_, _\#Wert_          | immediate              |
| _instr_ `Memory`, _Reg_, _Addr_,                      | _instr_ _Reg_, _Addr_            | direct                 |
| _instr_ `Indirect`, _Reg_, _Addr_,                    | _instr_ _Reg_, _(Addr)_          | direct                 |
| _instr_ `Indexed`, _Reg_, _Addr_,                     | _instr_ _Reg_, _Addr_,X          | indexed                |
| _instr_ `Indirect-Indexed`, _Reg_, _Addr_,            | _instr_ _Reg_, _(Addr)_,X        | indirect-indexed       |
| **Jumps**                                             |                                  |                        |
| `JPD` _Reg_, _Cond_, _Addr_                           | `JP` _Reg_, _Cond_, _Addr_       | conditional-direct     |
| `JPI` _Reg_, _Cond_, _Addr_                           | `JP` _Reg_, _Cond_, _(Addr)_     | conditional-indirect   |
| `JMD` _Reg_, _Cond_, _Addr_                           | `JM` _Reg_, _Cond_, _Addr_       | conditional-direct     |
| `JMI` _Reg_, _Cond_, _Addr_                           | `JM` _Reg_, _Cond_, _(Addr)_     | conditional-indirect   |
| `JPD` `Unconditional`, _Cond_, _Addr_                 | `JP` _Addr_                      | unconditional-direct   |
| `JPI` `Unconditional`, _Cond_, _Addr_                 | `JP` _(Addr)_                    | unconditional-indirect |
| `JMD` `Unconditional`, _Cond_, _Addr_                 | `JM` _Addr_                      | unconditional-direct   |
| `JMI` `Unconditional`, _Cond_, _Addr_                 | `JM` _(Addr)_                    | unconditional-indirect |
| **Jump Conditions**                                   |
| `Non-zero`                                            | `NZ`                             |  `≠ 0`                 |
| `Zero`                                                | `Z`                              |  `= 0`                 |
| `Negative`                                            | `N`                              |  `< 0`                 |
| `Positive`                                            | `P`                              |  `≥ 0`                 |
| `Positive-Non-zero`                                   | `PNZ`                            |  `> 0`                 |
| **Skips**                                             |
| `SKP 0`, _bit_, _Addr_                                | `SKP0` _bit_, _Addr_ _\[,Dest\]_ |                        |
| `SKP 1`, _bit_, _Addr_                                | `SKP1` _bit_, _Addr_ _\[,Dest\]_ |                        |
| **Bit Manipulation**                                  |
| `SET 0`, _bit_, _Addr_                                | `SET0` _bit_, _Addr_             |                        |
| `SET 1`, _bit_, _Addr_                                | `SET1` _bit_, _Addr_             |                        |
| **Shifts/Rotates**                                    |
| `SHIFT LEFT`, _cnt_, _Reg_                            | `SFTL` _\[cnt,\]_ _Reg_          |                        |
| `SHIFT RIGHT`, _cnt_, _Reg_                           | `SFTR` _\[cnt,\]_ _Reg_          | arithmetic Shift       |
| `ROTATE LEFT`, _cnt_, _Reg_                           | `ROTL` _\[cnt,\]_ _Reg_          |                        |
| `ROTATE RIGHT`, _cnt_, _Reg_                          | `ROTR` _\[cnt,\]_ _Reg_          |                        |
|                                                       |                                  |                        |

<!-- markdownlint-enable MD056 -->

There is no pseudo instruction to switch between these syntax variants. They may both be used anytime and in an arbitrary mix.

The target address _\[Dest\]_ that may optionally be added to skip instructions will not become part of the machine code. The assembler only checks whether the processor wil actually skip to the given address. This allows for instance to check whether one actually tries to skip a one-byte instruction. If the shift count argument _\[cnt\]_ is omitted, a one-bit shift/rotate is coded.

<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-disable MD001 -->
