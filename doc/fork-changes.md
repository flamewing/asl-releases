# Changes from original AS

This document summarizes the assembler, language and utility changes made by the flamewing/asl-releases fork, grouped by snapshot tag. The fork starts after original AS 1.42 build 204 at [`9f6b3bdb`](https://github.com/flamewing/asl-releases/commit/9f6b3bdb9311527a29b9013ded1f9db29536da2f). The latest original AS source import is build 313 at [`fb75b92d`](https://github.com/flamewing/asl-releases/commit/fb75b92ddad1d87e6003868b622971b11b9f87c4). Features inherited from original AS are described in the upstream [changelog](../changelog).

Coverage ends at [`7822fe98`](https://github.com/flamewing/asl-releases/commit/7822fe983c017dc09529823f9af40a644bbcadac). Each batch covers revisions after the preceding tag through the named tag, following main-branch ancestry. Changes after the last tag appear in Pending changes. The comparison links provide the detailed history; the summaries concentrate on code behavior, compatibility, performance and bug fixes. Historical changes may have been refined or superseded in later batches.

## Main differences

- Assembly syntax and compatibility: more complete M68K effective-address parsing, whitespace support, macro/repetition and conditional improvements, directive aliases, temporary-symbol and PHASE fixes, and register aliases.
- Correctness and robustness: instruction-encoding and addressing fixes, safer expression and string handling, malformed binary/catalog rejection, and more reliable file I/O and resource cleanup.
- Diagnostics: operand positions, actual values and limits, supported alternatives and bracket details, with English/German messages and stable numeric IDs.
- Performance: hashed symbol and forward-reference lookup, faster macro expansion and expression parsing, fewer temporary string copies and more efficient listing output.
- Utility behavior: corrected command-line validation, binary conversion, disassembly and message-catalog handling.

## Tagged batches

### snapshot-snapshot-v1.4.2.20210627

Boundary: [`9f6b3bdb`](https://github.com/flamewing/asl-releases/commit/9f6b3bdb9311527a29b9013ded1f9db29536da2f) to [`snapshot-snapshot-v1.4.2.20210627`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210627) ([`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/9f6b3bdb9311527a29b9013ded1f9db29536da2f...3e31e255c1f4ed080442899608614c81d115c938).

Keep MESSAGE output visible in quiet mode and accept whitespace in M68K effective-address expressions.

### snapshot-snapshot-v1.4.2.20210628

Boundary: [`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938) to [`snapshot-snapshot-v1.4.2.20210628`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210628) ([`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/3e31e255c1f4ed080442899608614c81d115c938...3e31e255c1f4ed080442899608614c81d115c938).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-snapshot-v1.4.2.20210629

Boundary: [`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938) to [`snapshot-snapshot-v1.4.2.20210629`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210629) ([`9ba75c50`](https://github.com/flamewing/asl-releases/commit/9ba75c5025181bb9c84dec0c05c786460de3d56f)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/3e31e255c1f4ed080442899608614c81d115c938...9ba75c5025181bb9c84dec0c05c786460de3d56f).

Make IFDEF and IFNDEF recognize macros and user-defined functions as well as ordinary symbols.

### snapshot-snapshot-v1.4.2.20210701

Boundary: [`9ba75c50`](https://github.com/flamewing/asl-releases/commit/9ba75c5025181bb9c84dec0c05c786460de3d56f) to [`snapshot-snapshot-v1.4.2.20210701`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210701) ([`d4336629`](https://github.com/flamewing/asl-releases/commit/d43366291b9b550fda08e8ea67cd564f3176df4c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/9ba75c5025181bb9c84dec0c05c786460de3d56f...d43366291b9b550fda08e8ea67cd564f3176df4c).

Replace the M68K effective-address parser, complete its remaining addressing forms and handle function expressions consistently with other operands.

### snapshot-snapshot-v1.4.2.20210702

Boundary: [`d4336629`](https://github.com/flamewing/asl-releases/commit/d43366291b9b550fda08e8ea67cd564f3176df4c) to [`snapshot-snapshot-v1.4.2.20210702`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210702) ([`c600e1a5`](https://github.com/flamewing/asl-releases/commit/c600e1a5cd984afe441f173287dd7e602d44395c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/d43366291b9b550fda08e8ea67cd564f3176df4c...c600e1a5cd984afe441f173287dd7e602d44395c).

Accept SLL and SL1 as aliases of Z80 SLIA. Add automatic integer-to-string conversion for symbol construction. Correct skipped repetition-block termination and recognize ELSEC/ENDC conditional aliases.

### snapshot-snapshot-v1.4.2.20210703

Boundary: [`c600e1a5`](https://github.com/flamewing/asl-releases/commit/c600e1a5cd984afe441f173287dd7e602d44395c) to [`snapshot-snapshot-v1.4.2.20210703`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210703) ([`b2d5ed87`](https://github.com/flamewing/asl-releases/commit/b2d5ed87c2cff8e4f0cacf757752be8cb368e123)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/c600e1a5cd984afe441f173287dd7e602d44395c...b2d5ed87c2cff8e4f0cacf757752be8cb368e123).

Fix PHASE program-counter handling, including signed M68K addresses, and correct the include-nesting limit. Support alternate REG and LABEL directive syntax.

### snapshot-snapshot-v1.4.2.20210705

Boundary: [`b2d5ed87`](https://github.com/flamewing/asl-releases/commit/b2d5ed87c2cff8e4f0cacf757752be8cb368e123) to [`snapshot-snapshot-v1.4.2.20210705`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210705) ([`4ec2f115`](https://github.com/flamewing/asl-releases/commit/4ec2f115bffeac7b9ca1def1f263d5a606831e9b)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/b2d5ed87c2cff8e4f0cacf757752be8cb368e123...4ec2f115bffeac7b9ca1def1f263d5a606831e9b).

Fix SWITCH handling inside inactive conditional branches and correct temporary-symbol behavior.

### snapshot-snapshot-v1.4.2.20210706

Boundary: [`4ec2f115`](https://github.com/flamewing/asl-releases/commit/4ec2f115bffeac7b9ca1def1f263d5a606831e9b) to [`snapshot-snapshot-v1.4.2.20210706`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210706) ([`68d40439`](https://github.com/flamewing/asl-releases/commit/68d40439679ac9af1f24a783188cd4df900b6d0d)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4ec2f115bffeac7b9ca1def1f263d5a606831e9b...68d40439679ac9af1f24a783188cd4df900b6d0d).

Correct LABEL interactions and make repeated -q options control quietness consistently with repeated -x options. MESSAGE output is repaired in a later batch.

### snapshot-snapshot-v1.4.2.20210707

Boundary: [`68d40439`](https://github.com/flamewing/asl-releases/commit/68d40439679ac9af1f24a783188cd4df900b6d0d) to [`snapshot-snapshot-v1.4.2.20210707`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210707) ([`cafa1f10`](https://github.com/flamewing/asl-releases/commit/cafa1f10a92673216fa80a9553b32963739a7af2)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/68d40439679ac9af1f24a783188cd4df900b6d0d...cafa1f10a92673216fa80a9553b32963739a7af2).

Add IRPN for expanding repeated groups of arguments, fix its initial crash and preserve backslashes during IRPC substitution.

### snapshot-snapshot-v1.4.2.20210708

Boundary: [`cafa1f10`](https://github.com/flamewing/asl-releases/commit/cafa1f10a92673216fa80a9553b32963739a7af2) to [`snapshot-snapshot-v1.4.2.20210708`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210708) ([`44d4037f`](https://github.com/flamewing/asl-releases/commit/44d4037fc60339fd7c26a1ffa0218fe2aef55886)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/cafa1f10a92673216fa80a9553b32963739a7af2...44d4037fc60339fd7c26a1ffa0218fe2aef55886).

Synchronize with original AS build 205. This batch adds no fork-specific assembler behavior.

### snapshot-snapshot-v1.4.2.20210720

Boundary: [`44d4037f`](https://github.com/flamewing/asl-releases/commit/44d4037fc60339fd7c26a1ffa0218fe2aef55886) to [`snapshot-snapshot-v1.4.2.20210720`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210720) ([`4a82beaa`](https://github.com/flamewing/asl-releases/commit/4a82beaabe94a78990ce26f5e9608b656e8e9e4d)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/44d4037fc60339fd7c26a1ffa0218fe2aef55886...4a82beaabe94a78990ce26f5e9608b656e8e9e4d).

Clarify that nested INCLUDE paths are resolved relative to the source file containing the directive.

### snapshot-snapshot-v1.4.2.20210727

Boundary: [`4a82beaa`](https://github.com/flamewing/asl-releases/commit/4a82beaabe94a78990ce26f5e9608b656e8e9e4d) to [`snapshot-snapshot-v1.4.2.20210727`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210727) ([`a8bfa90c`](https://github.com/flamewing/asl-releases/commit/a8bfa90c0f9b000eef9cc3002aa94ced83174c36)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4a82beaabe94a78990ce26f5e9608b656e8e9e4d...a8bfa90c0f9b000eef9cc3002aa94ced83174c36).

Restore MESSAGE output when -q is used. Synchronize with original AS builds 206 and 207.

### snapshot-snapshot-v1.4.2.20210906

Boundary: [`a8bfa90c`](https://github.com/flamewing/asl-releases/commit/a8bfa90c0f9b000eef9cc3002aa94ced83174c36) to [`snapshot-snapshot-v1.4.2.20210906`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210906) ([`5975e073`](https://github.com/flamewing/asl-releases/commit/5975e0730a0bd7c12dadbc111a91d489c6690461)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/a8bfa90c0f9b000eef9cc3002aa94ced83174c36...5975e0730a0bd7c12dadbc111a91d489c6690461).

Make ENDR a full synonym of ENDM for macro and repetition blocks.

### snapshot-snapshot-v1.4.2.20210908

Boundary: [`5975e073`](https://github.com/flamewing/asl-releases/commit/5975e0730a0bd7c12dadbc111a91d489c6690461) to [`snapshot-snapshot-v1.4.2.20210908`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210908) ([`f58a703e`](https://github.com/flamewing/asl-releases/commit/f58a703edceca4256e0d6bed14e9dc8ce022cee2)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/5975e0730a0bd7c12dadbc111a91d489c6690461...f58a703edceca4256e0d6bed14e9dc8ce022cee2).

No assembler behavior changes in this batch.

### snapshot-snapshot-v1.4.2.20211010

Boundary: [`f58a703e`](https://github.com/flamewing/asl-releases/commit/f58a703edceca4256e0d6bed14e9dc8ce022cee2) to [`snapshot-snapshot-v1.4.2.20211010`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20211010) ([`088b12ca`](https://github.com/flamewing/asl-releases/commit/088b12cac86396aaab9708b98ce52d3c67f6049c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/f58a703edceca4256e0d6bed14e9dc8ce022cee2...088b12cac86396aaab9708b98ce52d3c67f6049c).

Synchronize with original AS builds 208 through 211. This batch adds no fork-specific assembler behavior.

### snapshot-snapshot-v1.4.2.20211121

Boundary: [`088b12ca`](https://github.com/flamewing/asl-releases/commit/088b12cac86396aaab9708b98ce52d3c67f6049c) to [`snapshot-snapshot-v1.4.2.20211121`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20211121) ([`34045459`](https://github.com/flamewing/asl-releases/commit/34045459296251eec004b89db0be5b101040775c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/088b12cac86396aaab9708b98ce52d3c67f6049c...34045459296251eec004b89db0be5b101040775c).

Support macro parameter substitution using backslash-delimited names and correct a diagnostic-text typo. Synchronize with original AS build 212.

### snapshot-snapshot-v1.4.2.20240412

Boundary: [`34045459`](https://github.com/flamewing/asl-releases/commit/34045459296251eec004b89db0be5b101040775c) to [`snapshot-snapshot-v1.4.2.20240412`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240412) ([`e4dd4301`](https://github.com/flamewing/asl-releases/commit/e4dd43016e0d31986fd3ddf7bd36f828630a1065)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/34045459296251eec004b89db0be5b101040775c...e4dd43016e0d31986fd3ddf7bd36f828630a1065).

Fix incorrect M68K "short address not allowed" errors.

### snapshot-snapshot-v1.4.2.20240413

Boundary: [`e4dd4301`](https://github.com/flamewing/asl-releases/commit/e4dd43016e0d31986fd3ddf7bd36f828630a1065) to [`snapshot-snapshot-v1.4.2.20240413`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240413) ([`0b07a579`](https://github.com/flamewing/asl-releases/commit/0b07a579e25be25bf3e54c3794c8c75419702c27)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/e4dd43016e0d31986fd3ddf7bd36f828630a1065...0b07a579e25be25bf3e54c3794c8c75419702c27).

No assembler behavior changes in this batch.

### snapshot-snapshot-v1.4.2.20240416

Boundary: [`0b07a579`](https://github.com/flamewing/asl-releases/commit/0b07a579e25be25bf3e54c3794c8c75419702c27) to [`snapshot-snapshot-v1.4.2.20240416`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240416) ([`f3e164f1`](https://github.com/flamewing/asl-releases/commit/f3e164f1af17e7596433052d9aa9903c2ef0611e)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/0b07a579e25be25bf3e54c3794c8c75419702c27...f3e164f1af17e7596433052d9aa9903c2ef0611e).

No assembler behavior changes in this batch.

### snapshot-snapshot-v1.4.2.20240821

Boundary: [`f3e164f1`](https://github.com/flamewing/asl-releases/commit/f3e164f1af17e7596433052d9aa9903c2ef0611e) to [`snapshot-snapshot-v1.4.2.20240821`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240821) ([`1df09354`](https://github.com/flamewing/asl-releases/commit/1df0935489ac8ba99ea11bdffd5eb65406375d5b)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/f3e164f1af17e7596433052d9aa9903c2ef0611e...1df0935489ac8ba99ea11bdffd5eb65406375d5b).

Fix stale forward-reference handling for expressions required on the first pass and increase the source input-line capacity. Remove the copyright-string anti-tamper trap.

Repair bounded string concatenation, formatted output, endian handling and integer-size mismatches. Correct an accidental bitwise operator.

### snapshot-snapshot-v1.4.2.20240912

Boundary: [`1df09354`](https://github.com/flamewing/asl-releases/commit/1df0935489ac8ba99ea11bdffd5eb65406375d5b) to [`snapshot-snapshot-v1.4.2.20240912`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240912) ([`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1df0935489ac8ba99ea11bdffd5eb65406375d5b...077572d160aa03919af3736e5fb0ec4831979045).

Avoid premature errors for unresolved M68K displacements and report the appropriate out-of-range PC-relative diagnostic.

### snapshot-snapshot-v1.4.2.20240913

Boundary: [`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045) to [`snapshot-snapshot-v1.4.2.20240913`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240913) ([`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/077572d160aa03919af3736e5fb0ec4831979045...077572d160aa03919af3736e5fb0ec4831979045).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-snapshot-v1.4.2.20241029

Boundary: [`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045) to [`snapshot-snapshot-v1.4.2.20241029`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20241029) ([`b5b3ebc9`](https://github.com/flamewing/asl-releases/commit/b5b3ebc975f8383cd6374b37dacf743f70c11285)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/077572d160aa03919af3736e5fb0ec4831979045...b5b3ebc975f8383cd6374b37dacf743f70c11285).

Work around a Windows GCC miscompilation and accept BYTE wherever BYT is supported.

### snapshot-snapshot-v1.4.2.20250716

Boundary: [`b5b3ebc9`](https://github.com/flamewing/asl-releases/commit/b5b3ebc975f8383cd6374b37dacf743f70c11285) to [`snapshot-snapshot-v1.4.2.20250716`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20250716) ([`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/b5b3ebc975f8383cd6374b37dacf743f70c11285...1b65169ab5de6e0a3a61cdaff7818bb515171837).

Repair path construction on Windows.

### snapshot-snapshot-v1.4.2.20250717

Boundary: [`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837) to [`snapshot-snapshot-v1.4.2.20250717`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20250717) ([`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1b65169ab5de6e0a3a61cdaff7818bb515171837...1b65169ab5de6e0a3a61cdaff7818bb515171837).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-v1.4.2.20261010-run38090250819

Boundary: [`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837) to [`snapshot-v1.4.2.20261010-run38090250819`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261010-run38090250819) ([`4d342b19`](https://github.com/flamewing/asl-releases/commit/4d342b193bd116785dce6fdd730860c563bc57bb)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1b65169ab5de6e0a3a61cdaff7818bb515171837...4d342b193bd116785dce6fdd730860c563bc57bb).

Synchronize with original AS builds 213 through 313 while retaining the fork’s syntax and compatibility changes. Targets and instructions introduced by those upstream versions are described in [the original AS changelog](../changelog).

Correct macro argument substitution, skipped terminators, high argument counts and SHIFT bookkeeping. Improve conditional and first-pass expression handling, temporary labels, signed M68K PHASE addresses and Z80 register aliases. Warn when a newly created CODEPAGE is read before its mapping is modified.

Speed up symbol and forward-reference lookup, macro expansion, expression parsing and listing output. Preserve existing semantics while reducing repeated scans and temporary string copies.

Fix string truncation, zero-valued and high-byte CHARSET mappings, integer-syntax fallback, restricted-width complements and floating-point subnormal rounding. Correct TMS340xx FIELD bounds, single-address mapping overlaps and register/operand validation; improve memory cleanup.

### snapshot-v1.4.2.20261011-run38119234017

Boundary: [`4d342b19`](https://github.com/flamewing/asl-releases/commit/4d342b193bd116785dce6fdd730860c563bc57bb) to [`snapshot-v1.4.2.20261011-run38119234017`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261011-run38119234017) ([`a31c0b24`](https://github.com/flamewing/asl-releases/commit/a31c0b24de653f968f303dba0a5abeeee416c733)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4d342b193bd116785dce6fdd730860c563bc57bb...a31c0b24de653f968f303dba0a5abeeee416c733).

Support embedded message catalogs and improve locale compatibility on hosts without nl_langinfo.

### snapshot-v1.4.2.20261012-run38215198548

Boundary: [`a31c0b24`](https://github.com/flamewing/asl-releases/commit/a31c0b24de653f968f303dba0a5abeeee416c733) to [`snapshot-v1.4.2.20261012-run38215198548`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261012-run38215198548) ([`4730f629`](https://github.com/flamewing/asl-releases/commit/4730f6299d743703b7b677c6a9d09eddae5e92b7)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/a31c0b24de653f968f303dba0a5abeeee416c733...4730f6299d743703b7b677c6a9d09eddae5e92b7).

Harden malformed binary-record and message-catalog handling, length/allocation checks, file I/O and resource cleanup across the assembler and its utilities.

Fix addressing, decoder-result and register handling in several backends. Correct MSP430 emulated PC-relative adjustment and RX compact MOV displacement validation; repair unsafe shifts, divisors and bit mirroring. Fix integer-to-float conversion semantics and diagnose unsupported immediate sizes.

## Pending changes

Boundary: [`snapshot-v1.4.2.20261012-run38215198548`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261012-run38215198548) to [`7822fe98`](https://github.com/flamewing/asl-releases/commit/7822fe983c017dc09529823f9af40a644bbcadac). These changes are committed on the local main branch but are not included in the last recorded snapshot tag. [Compare this batch](https://github.com/flamewing/asl-releases/compare/4730f6299d743703b7b677c6a9d09eddae5e92b7...7822fe983c017dc09529823f9af40a644bbcadac).

Improve diagnostics with the failing operand, actual value, permitted range and supported alternatives. Branch errors include the displacement and PC base; bracket errors identify the mismatch and expected delimiter. English and German messages retain the established numeric IDs. The broader diagnostic inventory review remains in progress.

Reject invalid operands without emitting partial instructions or data, preserve the primary expression error and defer checks that depend on unresolved forward references. Correct instruction encoding and branch behavior in several backends, including AVR byte-mode wrapping, HC12X BTAS, HuC6280 TST, 65CE02 base-page addressing and MCore JSRI.

Fix ST7/STM8 packed BTJF/BTJT target selection, preventing an access violation. Compute STM8 branch distances at the full CPU address width before checking the signed-byte displacement. Repair register aliases, malformed address expressions, negative fixed/floating-format conversions and external message catalogs larger than 64 KiB.

Reject previously ignored unknown options in preset/key files and disassembler invocations. Repair incomplete disassembler help and add resource-compiler help.

Preserve M68K index-register classification in optimized MSVC builds by completing register lookup before restoring the size/scale suffix. This avoids an MSVC Release optimization that restores the suffix before lookup and incorrectly treats registers such as D2.W as displacements. Windows MSVC CI now tests both Debug and Release configurations.

Fix unary expression parsing within binary operands and chained prefixes. Give unary minus independent precedence above binary operators, preserving existing binary precedence, spellings and temporary-symbol syntax.

## Updating this document

Keep the snapshot tag boundaries and comparison links. Summarize changes to assembler or utility behavior, compatibility, performance and bug fixes, distinguishing upstream additions from fork changes. Group related fixes rather than listing individual commits. Keep paragraphs and list items on one physical line and rely on editor or previewer word wrapping.
