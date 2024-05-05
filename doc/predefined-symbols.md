# Predefined Symbols

<!-- markdownlint-disable MD001 -->
<!-- markdownlint-disable MD025 -->
<!-- markdownlint-disable MD030 -->
<!-- markdownlint-disable MD036 -->

| name          | data type | definition | meaning                                                                                   |
| :------------ | :-------- | :--------- | :---------------------------------------------------------------------------------------- |
| ARCHITECTURE  | string    | predefined | target platform AS was compiled for, in the style processor-manufacturer-operating system |
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
| INSUPMODE     | boolean   | dynamic(0) | processor in supervisor mode?                                                             |

| name       | data type | definition       | meaning                                                       |
| :--------- | :-------- | :--------------- | :------------------------------------------------------------ |
| INSRCMODE  | boolean   | dynamic(0)       | processor in source mode?                                     |
| FULLPMMU   | boolean   | dynamic(0/1)     | full PMMU instruction set allowed?                            |
| LISTON     | boolean   | dynamic(1)       | listing enabled?                                              |
| MACEXP     | boolean   | dynamic(1)       | expansion of macro constructs in listing enabled?             |
| MOMCPU     | integer   | dynamic\(68008\) | number of target CPU currently set                            |
| MOMCPUNAME | string    | dynamic\(68008\) | name of target CPU currently set                              |
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
| PC        | integer   | special      | current program counter (Thomson)                                         |
| TIME      | string    | predefined   | time of begin of assembly (1. pass)                                       |
| TRUE      | integer   | predefined   | 1 = logically "true"                                                      |
| VERSION   | integer   | predefined   | version of AS in BCD coding, e.g. 1331 hex for version 1.33p1             |
| WRAPMODE  | Integer   | predefined   | shortened program counter assumed?                                        |
| \*        | integer   | special      | current program counter (Motorola, Rockwell, Microchip, Hitachi)          |
| $         | integer   | special      | current program counter (Intel, Zilog, Texas, Toshiba, NEC, Siemens, AMD) |

To be exact, boolean symbols are just ordinary integer symbols with the difference that AS will assign only two different values to them (0 or 1, corresponding to False or True). AS does not store special symbols in the symbol table. For performance reasons, they are realized with hardcoded comparisons directly in the parser. They therefore do not show up in the assembly listing's symbol table. Predefined symbols are only set once at the beginning of a pass. The values of dynamic symbols may in contrast change during assembly as they reflect settings made with related pseudo instructions. The values added in parentheses give the value present at the beginning of a pass.

The names given in this table also reflect the valid way to reference these symbols in case-sensitive mode.

The names listed here should be avoided for own symbols; either one can define but not access them (special symbols), or one will receive an error message due to a double-defined symbol. The ugliest case is when the redefinition of a symbol made by AS at the beginning of a pass leads to a phase error and an infinite loop...

<!-- markdownlint-enable MD036 -->
<!-- markdownlint-enable MD030 -->
<!-- markdownlint-enable MD025 -->
<!-- markdownlint-enable MD001 -->
