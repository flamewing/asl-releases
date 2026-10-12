# Documentation audit

Audit baseline: `f28e43a7`, following the CI portability fixes in `02ed1452`. Review documentation against the current fork implementation and its regression tests; distinguish retained upstream history and benchmark reports from descriptions of current behavior. Keep the original diagnostic numbers, target-specific semantics and accepted encodings accurate. This record tracks review and validation rather than claiming that pending tranches are complete.

## Tranches

| Tranche | Scope | Status |
| --- | --- | --- |
| 1 | Single-line paragraphs in tracked Markdown files | Complete |
| 2 | Build, installation, package layout and source-maintenance guidance | Complete |
| 3 | Assembler invocation, options, environment, manpage and built-in help | Complete |
| 4 | Binary utilities, disassembler/resource compiler invocations, manpages and help | Complete |
| 5 | Source syntax, macros, conditionals and fork language extensions | Complete |
| 6 | Data directives, expressions, predefined symbols, includes and file formats | Complete |
| 7 | Processor-specific hints and programming examples, including corrected behavior | Pending |
| 8 | Diagnostics, cross-document consistency, links and final help/manual validation | Pending |

## Scope and evidence

The baseline contains 35 tracked Markdown files, six manpages and the help text for the assembler, five binary-file tools, the disassembler and the resource compiler. Review the user manual and README against `CMakeLists.txt`, command records/callbacks, parser/directive implementations, processor backends and existing tests. Review historical benchmark documents against their stated revisions and recorded experiments. The untracked printf audit documents are outside this committed-documentation audit.

## Completed work

### 1. Single-line Markdown paragraphs

Join 335 soft-wrapped lines across 15 Markdown files, including list-continuation paragraphs and benchmark prose. Keep fenced/indented code, shell commands, tables, headings, reference definitions and HTML comments intact. Validate all 35 existing Markdown files with Pandoc's GitHub-flavored Markdown parser: the syntax trees are identical after treating soft breaks as spaces. No soft or hard breaks remain inside parsed prose paragraphs or list prose. The new audit record also uses single-line paragraphs.

### 2. Build, installation and maintenance guidance

Replace obsolete AS.EXE/BIN/MAN package descriptions with the actual installed targets and GNUInstallDirs layout, and distinguish flat snapshot archives from full CMake installations. Document embedded/external catalogs, installed includes, optional Pandoc and the host-only resource compiler. Use runnable build/test commands with matching configurations and a test invocation compatible with CMake 3.19. Update source-maintenance guidance for C11, generated config.h, platform checks and .mes/.msg/.msh/.rsc resources; identify obsolete FAQ claims as historical and link current guidance.

Move COPYING installation outside the Pandoc condition so installation without the optional manual generator includes the license. Validate CMake reconfiguration, a staged MinGW installation without Pandoc, installed executable/include/manpage/license paths, and the documented cmake -E chdir CTest invocation. Existing Windows/Linux build validation supplies the compiler/configuration evidence; external-catalog installation is checked separately.

### 3. Assembler invocation, help and manpage

Audit command records and callbacks in as.c, cmdarg.c, msg_level.c, asmerr.c, asmlist.c, asmsub.c, asmpars.c, onoff_common.c and code6809.c. Align the manual option reference, man/asl.1 and both help languages with actual source/help/version/CPU-list invocations. Include omitted COMPMODE, PACKING, SUPMODE, NoICE mask, warning-range and developer-trace options; repair missing help newlines and incorrect include separators. Document optional arguments, global option application, wildcard/path quoting, presets/key files, real exit statuses, hashed symbols versus remaining macro/structure trees, and embedded-catalog/language behavior.

The audit reproduces silently ignored unknown ASCMD options (ASCMD=-z with -help exits 0). Propagate all non-success preset/key-file parser results so unknown options are rejected as documented. Add an assembler_cli regression covering English/German help, informational exit codes, source paths with spaces, explicit output, key files and ASCMD failures. Tests pass with embedded/external catalogs, together with M68K diagnostics, Coverity inputs and formatter contracts. Render the new manpage with groff without warnings and the updated manual chapter with Pandoc.

### 4. Utility and build-tool invocations

Audit the utility command records and actual help output. Add explicit -o and informational synopsis forms to the five binary-tool manpages, correct obsolete no-argument prompting/help claims, document named-file input and quoting, and add dasl/rescomp manpages. Distinguish positional alink output from converter/pbind -o output, and runtime utility presets from the host-only resource compiler. Add missing P2HEX C-format help and clarify include-only CPU header filters and extracted address ranges.

Repair broken continuation lines in the dasl help resource, which made English mode print only a German fragment; correct its indirect-entry syntax and reject unknown command options rather than ignoring them. Add rescomp -help with successful exit and accurate named/legacy output order. The utility_cli regression checks complete DAS help in both languages, C-format help, version exits, invalid DAS options and native host-resource help, with embedded/external catalogs. All eight manpages render with groff without warnings. All 428 CTest cases pass with both embedded and external catalogs.

### 5. Source syntax, macros and conditionals

Review dynamic source-line reading and continuation handling, the 476-operand parser limit, listing-mask callbacks and all nine listing bits. Correct obsolete fixed-line limits, reversed -t/+t behavior, the listing invocation and omitted DEFINE/register/structure bits. Review macro replacement, IRPN, SHIFT, recursion and conditional directives; distinguish the MAXNEST directive from the NESTMAX symbol, document occupied SHIFT aliases and correct IFDEF/IFNDEF to test a symbol, macro or function name rather than an expression. Validate the existing IRPN, SHIFT, argument-cursor and conditional regressions.

### 6. Expressions, symbols, data and file formats

Check expression strings and host arithmetic against TempResult, parser wrappers and startup symbol initialization. Document length-bearing strings with embedded NUL, configured 64/128-bit arithmetic, actual ARCHITECTURE and VERSION values, CPU identifiers and target/preset-dependent mode defaults. Replace obsolete binary-tree/parser API descriptions in maintenance guidance. Correct DS.D alignment and remove an inaccurate claim about whole zero-filled bytes in wider float formats. Review include suffix/search behavior and data directives against the implementation and regression fixtures.

Check ordinary code records against fileformat.h, headids.c and toolutils.c: separate little-endian metadata from target payload, fix Z8000 ID $34, DSP56000 storage granularity, payload/header lengths, supported sub-byte granularities and MAP ordering/escaping; identify relocation extensions outside the ordinary-record sketch. Existing expression, charset, codepage, predefined-symbol and integer-to-float tests pass. A direct assembly/conversion probe verifies embedded NUL bytes, INTWIDTH/HAS64, DS.D alignment and host/CPU/version symbols.

## Validation policy

For wording changes, verify claims against the implementation and run the documented invocation or relevant existing regression when useful. For built-in help changes, build embedded and external message catalogs and check actual output and exit status. Render the manual and manpages where tooling is available, check local links and cross-references, and keep each completed tranche in a separate local commit. Record any unresolved claim with the source evidence needed to settle it.
