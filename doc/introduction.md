# Introduction

<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->

This instruction is meant for programmers who are already very familiar with Assembler and who like to know how to work with AS. It is rather a reference than a user's manual and so it neither tries to explain the "language assembler" nor the processors. I have listed further literature in the bibliography which was substantial in the implementation of the different code generators. There is no book I know where you can learn Assembler from the start, so I generally learned this by "trial and error".

## License Agreement

Before we can go "in medias res", first of all the inevitable prologue:

As in the present version is licensed according to the Gnu General Public License (GPL); the details of the GPL may be read in the file COPYING bundled with this distribution. If you did not get it with AS, complain to the one you got AS from!

Shortly said, the GPL covers the following points:

- Programs based upon AS must also be licensed according to the GPL;
- distribution is explicitly allowed;
- explicit disclaiming of all warranties for damages resulting from usage of this program.

...however, I really urge you to read the file COPYING for the details!

The source code is available here:

- <https://github.com/flamewing/asl-releases>

The latest version of AS (Win32, Win64, Ubuntu, Mac OS X) is available here:

- <https://github.com/flamewing/asl-releases/releases>

To accelerate the error diagnose and correction, please add the following details to the bug report:

- Operating system (Windows, Linux, Mac OS X, etc.) and its version
- Version of AS used
- If you compiled the assembler yourself, the compiler used and its version
- If possible, the source file that triggered the bug

Now, after this inevitable introduction we can turn to the actual documentation:

## General Capabilities of the Assembler

In contrast to ordinary assemblers, AS offers the possibility to generate code for totally different processors. At the moment, the following processor families have been implemented:

- Motorola 68000...68040, 683xx, and Coldfire including coprocessor and MMU
- Motorola ColdFire
- Motorola DSP5600x, DSP56300
- Motorola M-Core
- Motorola/IBM MPC601/MPC505/PPC403/MPC821
- Motorola 6800, 6801, 68(HC)11(K4) and Hitachi 6301
- Motorola/Freescale 6805, 68HC(S)08
- Motorola 6809 / Hitachi 6309
- Motorola/Freescale 68HC12(X) including XGATE
- Freescale/NXP S12Z ("MagniV")
- Motorola 68HC16
- Freescale 68RS08
- Hitachi H8/300(H)
- Hitachi H8/500
- Hitachi SH7000/7600/7700
- Hitachi HMCS400
- Hitachi H16
- Rockwell 6502, 65(S)C02, Commodore 65CE02, WDC W65C02S, Rockwell 65C19, and Hudson HuC6280
- CMD 65816
- Mitsubishi MELPS-740
- Mitsubishi MELPS-7700
- Mitsubishi MELPS-4500
- Mitsubishi M16
- Mitsubishi M16C
- Intel 4004/4040
- Intel MCS-48/41, including Siemens SAB80C382, and the OKI variants
- Intel MCS-51/251, Dallas DS80C390
- Intel MCS-96/196(Nx)/296
- Intel 8080/8085
- Intel i960
- Signetics 8X30x
- Signetics 2650
- Philips XA
- Atmel (Mega-)AVR
- AMD 29K
- Siemens 80C166/167
- Zilog Z80, Z180, Z380
- Zilog Z8, Super8, Z8 Encore
- Zilog Z8000
- Xilinx KCPSM/KCPSM3 ('PicoBlaze')
- LatticeMico8
- Toshiba TLCS-900(L)
- Toshiba TLCS-90
- Toshiba TLCS-870(/C)
- Toshiba TLCS-47
- Toshiba TLCS-9000
- Toshiba TC9331
- Microchip PIC16C54...16C57
- Microchip PIC16C84/PIC16C64
- Microchip PIC17C42
- Parallax SX20/28
- SGS-Thomson ST6
- SGS-Thomson ST7/STM8
- SGS-Thomson ST9
- SGS-Thomson 6804
- Texas Instruments TMS32010/32015
- Texas Instruments TMS3202x
- Texas Instruments TMS320C3x/TMS320C4x
- Texas Instruments TMS320C20x/TMS320C5x
- Texas Instruments TMS320C54x
- Texas Instruments TMS320C6x
- Texas Instruments TMS99xx/99xxx
- Texas Instruments TMS7000
- Texas Instruments TMS1000
- Texas Instruments TMS370xxx
- Texas Instruments MSP430(X)
- National Semiconductor SC/MP
- National Semiconductor INS807x
- National Semiconductor COP4
- National Semiconductor COP8
- National Semiconductor SC144xx
- National Semiconductor NS32xxx
- Fairchild ACE
- Fairchild F8
- NEC μPD78(C)0x/μPD 78(C)1x
- NEC μPD75xx
- NEC μPD 75xxx (alias 75K0)
- NEC 78K0
- NEC 78K2
- NEC 78K3
- NEC 78K4
- NEC μPD7720/7725
- NEC μPD77230
- Fujitsu F<sup>2</sup>MC8L
- Fujitsu F<sup>2</sup>MC16L
- OKI OLMS-40
- OKI OLMS-50
- Panafacom MN1610/MN1613
- Padauk PMS/PMC/PFSxxx
- Symbios Logic SYM53C8xx (yes, they are programmable!)
- Intersil CDP1802/1804/1805(A)
- XMOS XS1
- MIL STD 1750
- KENBAK-1
- GI CP-1600

under work / planned / in consideration :

- Analog Devices ADSP21xx
- SGS-Thomson ST20
- Texas Instruments TMS320C8x

Unloved, but now, however, present :

- Intel 80x86, 80186, Nec V30 & V35 incl. coprocessor 8087

The switch to a different code generator is allowed even within one file, and as often as one wants!

The reason for this flexibility is that AS has a history, which may also be recognized by looking at the version number. AS was created as an extension of a macro assembler for the 68000 family. On special request, I extended the original assembler so that it was able to translate 8051 mnemonics. On this way (decline ?!) from the 68000 to 8051, some other processors were created as by-products. All others were added over time due to user requests. So At least for the processor-independent core of AS, one may assume that it is well-tested and free of obvious bugs. However, I often do not have the chance to test a new code generator in practice (due to lack of appropriate hardware), so surprises are not impossible when working with new features. You see, the things stated in the [License Agreement](#license-agreement) section have a reason...

This flexibility implies a somewhat exotic code format, therefore I added some tools to work with it. Their description can be found in [Utility Programs](utility-programs.md).

AS is a macro assembler, which means that the programmer has the possibility to define new "commands" by means of macros. Additionally it masters conditional assembling. Labels inside macros are automatically processed as being local.

For the assembler, symbols may have either integer, string or floating point values. These will be stored - like interim values in formulas - with a width of 32 bits for integer values, 80 or 64 bits for floating point values, and 255 characters for strings. For a couple of micro controllers, there is the possibility to classify symbols by segmentation. So the assembler has a (limited) possibility to recognize accesses to wrong address spaces.

The assembler does not know explicit limits in the nesting depth of include files or macros; a limit is only given by the program stack restricting the recursion depth. Nor is there a limit for the symbol length, which is only restricted by the maximum line length.

From version 1.38 on, AS is a multipass-assembler. This pompous term means no more than the fact that the number of passes through the source code need not be exactly two. If the source code does not contain any forward references, AS needs only one pass. In case AS recognizes in the second pass that it must use a shorter or longer instruction coding, it needs a third (fourth, fifth...) pass to process all symbol references correctly. There is nothing more behind the term "multipass", so it will not be used further more in this documentation.

After so much praise a bitter pill: AS cannot generate linkable code. An extension with a linker needs considerable effort and is not planned at the moment.

Those who want to take a look at the sources of AS can simply get the Unix version of AS, which comes as source for self-compiling. The sources are definitely not in a format that is targeted at easy understanding - the original Pascal version still raises its head at a couple of places, and I do not share a couple of common opinions about 'good' C coding.

## Supported Platforms

In theory, any platform with a reasonably modern C compiler (must support C11) should support AS. This includes modern versions of GCC, clang, Windows with Microsoft's compiler, and AS even compiles with TinyC Compiler.

<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
