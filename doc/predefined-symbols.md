# Predefined Symbols

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

| name          | data type | definition | meaning                                                                                   |
| :------------ | :-------- | :--------- | :---------------------------------------------------------------------------------------- |
| ARCHITECTURE  | string    | predefined | target platform AS was compiled for, in the form host-processor-host-operating-system (not the selected assembly target) |
| BIGENDIAN     | boolean   | dynamic(0) | storage of constants MSB first?                                                           |
| CASESENSITIVE | boolean   | normal     | case sensitivity in symbol names?                                                         |
| CONSTPI       | float     | normal     | constant Pi (3.1415.....)                                                                 |
| DATE          | string    | predefined | date of begin of assembly                                                                 |
| FALSE         | boolean   | predefined | 0 = logically "false"                                                                     |
| HASFPU        | boolean   | dynamic(0) | coprocessor instructions enabled?                                                         |
| HASPMMU       | boolean   | dynamic(0) | MMU instructions enabled?                                                                 |
| INEXTMODE     | boolean   | dynamic(0) | XM flag set for 4 Gbyte address space?                                                    |
| INLWORDMODE   | boolean   | dynamic(0) | LW flag set for 32 bit instructions?                                                      |
| INMAXMODE     | boolean   | dynamic(0) | processor in maximum mode?                                                                |
| INPANEL | Boolean | dyn.(0) | IM61x0 Control Panel Flip-Flop state. |
| INSUPMODE     | boolean   | dynamic(0) | processor in supervisor mode?                                                             |

| name       | data type | definition       | meaning                                                       |
| :--------- | :-------- | :--------------- | :------------------------------------------------------------ |
| INSRCMODE  | boolean   | dynamic(0)       | processor in source mode?                                     |
| FULLPMMU   | boolean   | dynamic(0/1)     | full PMMU instruction set allowed?                            |
| LISTON     | boolean   | dynamic(1)       | listing enabled?                                              |
| MACEXP     | boolean   | dynamic(1)       | expansion of macro constructs in listing enabled?             |
| MOMCPU     | integer   | dynamic | internal identifier of the current target CPU (not its model number)                            |
| MOMCPUNAME | string    | dynamic | name of target CPU currently set                              |
| MOMFILE    | string    | special          | current source file (including include files)                 |
| MOMLINE    | integer   | special          | current line number in source file                            |
| MOMPASS    | integer   | special          | number of current pass                                        |
| MOMSECTION | string    | special          | name of current section or empty string if out of any section |
| MOMSEGMENT | string    | special          | name of address space currently selected with `SEGMENT`       |
|            |           |                  |                                                               |

| name      | data type | definition   | meaning                                                                   |
| :-------- | :-------- | :----------- | :------------------------------------------------------------------------ |
| `NESTMAX` | Integer   | dynamic(256) | maximum nesting level of macro expansions                                 |
| PADDING   | boolean   | dynamic(1)   | pad byte field to even count?                                             |
| RELAXED   | boolean   | dynamic(0)   | any syntax allowed integer constants?                                     |
| . | Integer | special | Current program counter (IM61x0). |
| PC        | integer   | special      | current program counter (ST7/ST9 and related targets)                                         |
| TIME      | string    | predefined   | time of begin of assembly (1. pass)                                       |
| TRUE      | integer   | predefined   | 1 = logically "true"                                                      |
| VERSION   | integer   | predefined   | legacy numeric version identifier; this fork uses $142F, separately from the full -version release string             |
| WRAPMODE  | Integer   | predefined   | shortened program counter assumed?                                        |
| \*        | integer   | special      | current program counter (Motorola, Rockwell, Microchip, Hitachi)          |
| $         | integer   | special      | current program counter (Intel, Zilog, Texas, Toshiba, NEC, Siemens, AMD) |

To be exact, boolean symbols are just ordinary integer symbols with the difference that AS will assign only two different values to them (0 or 1, corresponding to False or True). AS does not store special symbols in the symbol table. For performance reasons, they are realized with hardcoded comparisons directly in the parser. They therefore do not show up in the assembly listing's symbol table. Predefined symbols are only set once at the beginning of a pass. The values of dynamic symbols may in contrast change during assembly as they reflect settings made with related pseudo instructions. Values in parentheses describe typical initial settings; CPU selection and command-line presets may select different defaults. Target-specific mode symbols exist only where the selected backend registers them.

The names given in this table also reflect the valid way to reference these symbols in case-sensitive mode.

The names listed here should be avoided for own symbols; either one can define but not access them (special symbols), or one will receive an error message due to a double-defined symbol. The ugliest case is when the redefinition of a symbol made by AS at the beginning of a pass leads to a phase error and an infinite loop...

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->

`FLOATMAX` is the largest finite floating-point value representable on the host.

On hosts that parse infinity, `INF` is reserved as a floating-point infinity and cannot be a user-defined symbol.

`INTWIDTH` gives the bit width of internal integer arithmetic. `HAS64` is true when that arithmetic has at least 64 bits; supported hosts may use 128 bits.

`COMPMODE`, `PACKING` and `EXTRACOMMENTS` reflect the corresponding on/off settings on targets that register these directives. `MOMCPU` is an internal CPU identifier; use `MOMCPUNAME` when testing a target name. `DATE` and `TIME` retain the values captured at the start of the first pass rather than being refreshed on every pass.
