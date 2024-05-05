# Pseudo-Instructions and Integer Syntax

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

This appendix is designed as a quick reference to look up all pseudo instructions provided by AS. The list is ordered in two parts: The first part lists the instructions that are always available, and this list is followed by lists that enumerate the instructions additionally available for a certain processor family.

#### Instructions that are always available

> `=` `:=` `ALIGN` `BINCLUDE` `CASE` `CHARSET` `CPU` `DEPHASE` `DOTTEDSTRUCTS` `ELSE` `ELSECASE` `ELSEIF` `END` `ENDCASE` `ENDIF` `ENDM` `ENDS` `ENDSECTION` `ENDSTRUCT` `ENUM` `ENUMCONF` `ERROR` `EQU` `EXITM` `FATAL` `FORWARD` `FUNCTION` `GLOBAL` `IF` `IFB` `IFDEF` `IFEXIST` `IFNB` `IFNDEF` `IFNEXIST` `IFNUSED` `IFUSED` `INCLUDE` `INTSYNTAX` `IRP` `LABEL` `LISTING` `MACEXP` `MACECP_DFT` `MACEXP_OVR` `MACRO` `MESSAGE` `NEWPAGE` `NEXTENUM` `ORG` `PAGE` `PHASE` `POPV` `PUSHV` `PRTEXIT` `PRTINIT` `PUBLIC` `READ` `RELAXED` `REPT` `RESTORE` `RORG` `SAVE` `SECTION` `SEGMENT` `SHARED` `STRUC` `STRUCT` `SWITCH` `TITLE` `UNION` `WARNING` `WHILE`

Additionally, there are:

- `SET` resp. `EVAL`, in case `SET` is already a machine instruction.
- `SHIFT` resp. `SHFT`, in case `SHIFT` is already a machine instruction.

#### Motorola 680x0/MCF5xxx

_Default Integer Syntax: Motorola_

> `DC[.<size>]` `DS[.<size>]` `FULLPMMU` `FPU` `PADDING` `PMMU` `REG` `SUPMODE`

#### Motorola 56xxx

_Default Integer Syntax: Motorola_

> `DC` `DS` `XSFR` `YSFR`

#### PowerPC

_Default Integer Syntax: C_

> `BIGENDIAN` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG` `SUPMODE`

#### Motorola M-Core

_Default Integer Syntax: Motorola_

> `DC[.<size>]` `DS[.<size>]` `REG` `SUPMODE`

#### Motorola XGATE

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DC[.<size>]` `DFS` `DS[.<size>]` `FCB` `FCC` `FDB` `PADDING` `REG` `RMB`

#### Motorola 68xx/Hitachi 63xx

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### Motorola/Freescale 6805/68HC(S)08

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### Motorola 6809/Hitachi 6309

_Default Integer Syntax: Motorola_

> `ADR` `ASSUME` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### Motorola 68HC12

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### NXP S12Z

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DB` `DC[.<size>]` `DEFBIT` `DEFBITFIELD` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### Motorola 68HC16

_Default Integer Syntax: Motorola_

> `ADR` `ASSUME` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING` `RMB`

#### Freescale 68RS08

_Default Integer Syntax: Motorola_

> `ADR` `ASSUME` `BYT` `DB` `DC[.<size>]` `DFS` `DS[.<size>]` `DW` `FCB` `FCC` `FDB` `PADDING`

#### Hitachi H8/300(L/H)

_Default Integer Syntax: Motorola_

> `BIT` `DC[.<size>]` `DS[.<size>]` `MAXMODE` `PADDING` `REG`

#### Hitachi H8/500

_Default Integer Syntax: Motorola_

> `ASSUME` `BIT` `DATA` `DC[.<size>]` `DS[.<size>]` `MAXMODE` `PADDING` `REG`

#### Hitachi SH7x00

_Default Integer Syntax: Motorola_

> `COMPLITERALS` `DC[.<size>]` `DS[.<size>]` `LTORG` `PADDING` `REG` `SUPMODE`

#### Hitachi HMCS400

_Default Integer Syntax: Motorola_

> `DATA` `RES` `SFR`

#### Hitachi H16

_Default Integer Syntax: Motorola_

> `BIT` `DC[.<size>]` `DS[.<size>]` `REG` `SUPMODE`

#### 65xx/MELPS-740

_Default Integer Syntax: Motorola_

> `ADR` `ASSUME` `BYT` `DFS` `FCB` `FCC` `FDB` `RMB`

#### 65816/MELPS-7700

_Default Integer Syntax: Motorola_

> `ADR` `ASSUME` `BYT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `DFS` `FCB` `FCC` `FDB` `RMB`

#### Mitsubishi MELPS-4500

_Default Integer Syntax: Motorola_

> `DATA` `RES` `SFR`

#### Mitsubishi M16

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### Mitsubishi M16C

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### Intel 4004

_Default Integer Syntax: Intel_

> `DATA` `DS` `REG`

#### Intel 8008

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT` `Z80SYNTAX`

#### Intel MCS-48

_Default Integer Syntax: Intel_

> `ASSUME` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### Intel MCS-(2)51

_Default Integer Syntax: Intel_

> `BIGENDIAN` `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT` `REG` `SFR` `SFRB` `SRCMODE`

#### Intel MCS-96

_Default Integer Syntax: Intel_

> `ASSUME` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Intel 8080/8085

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT`

#### Intel 8086/80186/NEC V30/35

_Default Integer Syntax: Intel_

> `ASSUME` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT`

#### Intel i960

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `FPU` `REG` `SPACE` `SUPMODE` `WORD`

#### Signetics 8X30x

_Default Integer Syntax: Motorola_

> `LIV` `RIV`

#### Signetics 2650

_Default Integer Syntax: Motorola_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Philips XA

_Default Integer Syntax: Intel_

> `ASSUME` `BIT` `DB` `DC[.<size>]` `DD` `DN` `DQ` `DS[.<size>]` `DT` `DW` `PADDING` `PORT` `REG` `SUPMODE`

#### Atmel AVR

_Default Integer Syntax: C_

> `BIT` `DATA` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PACKING` `PORT` `REG` `RES` `SFR`

#### AMD 29K

_Default Integer Syntax: C_

> `ASSUME` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `EMULATED` `ERG` `SUPMODE`

#### Siemens 80C166/167

_Default Integer Syntax: Intel_

> `ASSUME` `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### Zilog Zx80

_Default Integer Syntax: Intel_

> `DB` `DD` `DEFB` `DEFW` `DN` `DQ` `DS` `DT` `DW` `EXTMODE` `LWORDMODE`

#### Zilog Z8

_Default Integer Syntax: Intel_

> `DB` `DEFBIT` `DD` `DN` `DQ` `DS` `DT` `DW` `REG` `SFR`

#### Zilog Z8000

_Default Integer Syntax: Intel_

> `DB` `DD` `DEFBIT` `DEFBITB` `DN` `DQ` `DS` `DT` `DW` `PORT` `REG`

#### Xilinx KCPSM

_Default Integer Syntax: Intel_

> `CONSTANT` `NAMEREG` `REG`

#### Xilinx KCPSM3

_Default Integer Syntax: Intel_

> `CONSTANT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `NAMEREG` `PORT` `REG`

#### LatticeMico8

_Default Integer Syntax: C_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT` `REG`

#### Toshiba TLCS-900

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `MAXIMUM` `SUPMODE`

#### Toshiba TLCS-90

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Toshiba TLCS-870(/C)

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Toshiba TLCS-47(0(A))

_Default Integer Syntax: Intel_

> `ASSUME` `DB` `DN` `DD` `DQ` `DS` `DT` `DW` `PORT`

#### Toshiba TLCS-9000

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### Toshiba TC9331

_Default Integer Syntax: Intel_

#### Microchip PIC16C5x

_Default Integer Syntax: Motorola_

> `DATA` `RES` `SFR` `ZERO`

#### Microchip PIC16C8x

_Default Integer Syntax: Motorola_

> `DATA` `RES` `SFR` `ZERO`

#### Microchip PIC17C42

_Default Integer Syntax: Motorola_

> `DATA` `RES` `SFR` `ZERO`

#### Parallax SX20

_Default Integer Syntax: Motorola_

> `BIT` `DATA` `SFR` `ZERO`

#### SGS-Thomson ST6

_Default Integer Syntax: Intel_

> `ASCII` `ASCIZ` `ASSUME` `BIT` `BYTE` `BLOCK` `SFR` `WORD`

#### SGS-Thomson ST7/STM8

_Default Integer Syntax: Intel_

> `DC[.<size>]` `DS[.<size>]` `PADDING`

#### SGS-Thomson ST9

_Default Integer Syntax: Intel_

> `ASSUME` `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### 6804

_Default Integer Syntax: Motorola_

> `ADR` `BYT` `DB` `DFS` `DS` `DW` `FCB` `FCC` `FDB` `RMB` `SFR`

#### Texas Instruments TMS3201x

_Default Integer Syntax: Intel_

> `DATA` `PORT` `RES`

#### Texas Instruments TMS32C02x

_Default Integer Syntax: Intel_

> `BFLOAT` `BSS` `BYTE` `DATA` `DOUBLE` `EFLOAT` `TFLOAT` `LONG` `LQxx` `PORT` `Qxx` `RES` `RSTRING` `STRING` `WORD`

#### Texas Instruments TMS320C3x/C4x

_Default Integer Syntax: Intel_

> `ASSUME` `BSS` `DATA` `EXTENDED` `SINGLE` `WORD`

#### Texas Instruments TM32C020x/TM32C05x/TM32C054x

_Default Integer Syntax: Intel_

> `BFLOAT` `BSS` `BYTE` `DATA` `DOUBLE` `EFLOAT` `TFLOAT` `LONG` `LQxx` `PORT` `Qxx` `RES` `RSTRING` `STRING` `WORD`

#### Texas Instruments TMS320C6x

_Default Integer Syntax: Intel_

> `BSS` `DATA` `DOUBLE` `SINGLE` `WORD`

#### Texas Instruments TMS99xx

_Default Integer Syntax: Intel_

> `BSS` `BYTE` `DOUBLE` `PADDING` `SINGLE` `WORD`

#### Texas Instruments Instruments TMS1000

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Texas Instruments TMS70Cxx

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Texas Instruments TMS370

_Default Integer Syntax: Intel_

> `DB` `DBIT` `DN` `DD` `DQ` `DS` `DT` `DW`

#### Texas Instruments MSP430

_Default Integer Syntax: Intel_

> `BSS` `BYTE` `PADDING` `REG` `WORD`

#### National SC/MP

_Default Integer Syntax: C_

> `BIGENDIAN` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### National INS807x

_Default Integer Syntax: C_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### National COP4

_Default Integer Syntax: C_

> `ADDR` `ADDRW` `BYTE` `DB` `DD` `DQ` `DS` `DSB` `DSW` `DT` `DW` `FB` `FW` `SFR` `WORD`

#### National COP8

_Default Integer Syntax: C_

> `ADDR` `ADDRW` `BYTE` `DB` `DD` `DQ` `DS` `DSB` `DSW` `DT` `DW` `FB` `FW` `SFR` `WORD`

#### National SC14xxx

_Default Integer Syntax: C_

> `DC` `DC8` `DS` `DS8` `DS16` `DW` `DW16`

#### National NS32xxx

> `BYTE` `CUSTOM` `DB` `DD` `DOUBLE` `DQ` `DS` `DT` `DW` `FLOAT` `FPU` `LONG` `PMMU` `REG` `SUPMODE` `WORD`

#### Fairchild ACE

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Fairchild F8

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `PORT`

#### NEC μPD78(C)1x

_Default Integer Syntax: Intel_

> `ASSUME` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC 75xx

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC 75K0

_Default Integer Syntax: Intel_

> `ASSUME` `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `SFR`

#### NEC 78K0

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC 78K2

_Default Integer Syntax: Intel_

> `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC 78K3

_Default Integer Syntax: Intel_

> `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC 78K4

_Default Integer Syntax: Intel_

> `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### NEC μPD772x

_Default Integer Syntax: Intel_

> `DATA` `RES`

#### NEC μPD77230

_Default Integer Syntax: Intel_

> `DS` `DW`

#### Symbios Logic SYM53C8xx

_Default Integer Syntax: C_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Fujitsu F<sup>2</sup>MC8L

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### Fujitsu F<sup>2</sup>MC16L

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### OKI OLMS-40

_Default Integer Syntax: Intel_

> `DATA` `RES` `SFR`

#### OKI OLMS-50

_Default Integer Syntax: Intel_

> `DATA` `RES` `SFR`

#### Panafacom MN161x

_Default Integer Syntax: IBM_

> `DC` `DS`

#### Padauk PMC/PMS/PFSxxx

_Default Integer Syntax: C_

> `BIT` `DATA` `RES` `SFR`

#### Intersil 180x

_Default Integer Syntax: Intel_

> `DB` `DD` `DN` `DQ` `DS` `DT` `DW`

#### XMOS XS1

_Default Integer Syntax: Motorola_

> `DB` `DD` `DQ` `DN` `DS` `DT` `DW` `REG`

#### ATARI Vector

_Default Integer Syntax: Motorola_

#### MIL STD 1750

_Default Integer Syntax: Intel_

> `DATA` `EXTENDED` `FLOAT`

#### KENBAK

_Default Integer Syntax: Intel_

> `BIT` `DB` `DD` `DN` `DQ` `DS` `DT` `DW` `REG`

#### CP1600

_Default Integer Syntax: IBM (hex), C (oct)_

> `BYTE` `RES` `TEXT` `WORD` `ZERO`

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->
