# Initial profiling findings — 5 October 2026

Historical timing figures below are provisional: on 6 October the user identified severe VSCode file-watcher contention from the growing in-tree benchmark directories. Future artifacts live outside the checkout and all nine discarded experiments have been retested. See the [new retest results](README.md#discarded-experiment-retests-2026-10-06) and [watcher-confound procedure](README.md#watcher-contention-and-retesting-2026-10-06).

ASL now has opt-in CMake profiling modes and a repeatable real-disassembly benchmark runner. Measurements below use optimized, **uninstrumented** builds: seven measured assemblies after one warmup, without listing output. Copies, asset generation, and hashing are outside the measured interval.

## Baseline measurements

| Workload | Windows, NTFS | Ubuntu, native Linux storage | Ubuntu, `/mnt/XXX` | Shared/native ratio |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.420 s | 0.607 s | 7.926 s | 13.1× |
| Sonic 2 | 2.336 s | 1.291 s | 12.458 s | 9.7× |
| Sonic & Knuckles | 2.118 s | 0.964 s | 21.381 s | 22.2× |

Run ranges (minimum–maximum):

| Workload | Windows | Linux native | Linux `/mnt/XXX` |
| --- | --- | --- | --- |
| Sonic 1 | 1.337–1.552 s | 0.595–0.652 s | 7.669–8.244 s |
| Sonic 2 | 2.087–2.456 s | 1.150–1.338 s | 12.303–12.662 s |
| Sonic & Knuckles | 1.953–2.749 s | 0.937–1.028 s | 21.205–21.865 s |

For Linux profiling, keep source and benchmark inputs on a native Linux filesystem. The shared/native comparison uses the **same Linux executable and prepared inputs**; the output directory determines where the runner stages and assembles those inputs. This includes both reading source/assets and writing objects, so it does not isolate individual filesystem operations.

The Windows/Linux numbers are descriptive baselines, not a controlled compiler or OS comparison. Windows uses MinGW GCC 16.2.0; Ubuntu uses GCC 15.2.0. Compiler, C runtime, environment, filesystem, and background system activity differ. No cold-cache or confidence-interval claim is made.

Machine: Intel Core i9-11900, Windows build 26100, WSL2 Ubuntu, kernel `6.18.40.1-microsoft-standard-WSL2`. Both builds use Release (-O3).

## Optimization candidates

The independent Windows and Linux gprof reports identify the same main areas.
Percentages below are **instrumented self-time**, not shares of baseline wall
time. Windows reports aggregate five runs; Linux reports aggregate seven.

| Function | Windows self-time, S1 / S2 / SK | Linux self-time, S1 / S2 / SK | Calls per assembly, S1 / S2 / SK |
| --- | --- | --- | --- |
| `EvalStrExpression` | 7.95 / 11.60 / 6.70% | 12.73 / 15.93 / 15.03% | 210,950 / 351,532 / 701,615 |
| `ReplaceLineUnchecked` | 3.41 / 3.00 / 1.06% | 8.18 / 7.73 / 4.10% | 1,479,714 / 2,535,532 / 869,844 |
| `QuotSMultPosQualify` | 3.41 / 3.80 / 5.29% | 4.55 / 7.96 / 4.64% | 432,496 / 720,970 / 748,380 |

1. **Reduce expression-parser allocation and conversion work.**
   [EvalStrExpression](../asmpars.c) allocates two 1024-byte scratch strings on every call, including recursive calls, then copies/trims strings and attempts integer, float, and string conversion before general expression evaluation. [as_dynstr_ini](../dynstr.c) allocates and zeroes the entire buffer. Investigate scratch storage reused by recursion depth, lazy allocation, and cheap lexical checks before float conversion. Preserve long-expression handling, recursion, temporary/local symbols, numeric syntax, and relocation behavior. These are hypotheses; no speedup has been established yet.

2. **Reduce repeated full-line macro-token scans.**
   [ReplaceLineUnchecked and ExpandLine](../asmsub.c) scan a string for each token, performing a comparison at each position and shifting replacement text. Consider a first-byte search fast path, then a single-pass decoder for the two-byte encoded macro tokens. Preserve the current replacement order and behavior when replacement text contains other tokens. This is especially relevant to Sonic 1 and Sonic 2.

3. **Reduce quote-aware delimiter scanning.**
   [QuotPosCore and SearchMultString](../asmsub.c) repeatedly check strings while tracking quotes and brackets. A fast filter for possible delimiter-leading bytes could skip expensive checks. Preserve escaped quotes, nesting, and architecture-specific comment rules.

4. **Measure lookup and normalization separately.**
   `SearchTree`, `FindNode_FNode`, `as_strcasecmp`, string copies, and trimming also appear frequently. Symbol lookup in [asmpars.c](../asmpars.c) already uses an open-addressed hash table; `SearchTree` covers other structures such as macros. Investigate comparison/probe and repeated normalization costs before choosing a data-structure change.

Treat gprof as a guide to experiments. Windows spends about 41–45% of collected self-time in `_mcount_private` and `__fentry__`; Linux reports a substantial `_init` bucket whose attribution should not be interpreted as application startup work. Inlining and uninstrumented library code also limit attribution. The [GNU gprof manual](https://sourceware.org/binutils/docs/gprof.html) describes the profiling model and limitations. Ubuntu did not have perf or Valgrind installed; no packages were installed. `ASL_PROFILE=SAMPLE` prepares symbolized builds for later independent sampling. MSVC sampling flags were added but not exercised in this run.

## Validation and saved artifacts

All **202 existing CTest tests passed on Windows and Linux**. All three disassemblies assemble successfully; output hashes are stable across repetitions and identical between baseline and gprof builds on each platform. Parsing the `.p` files confirms that Windows/Linux code records are byte-identical; only the creator footer differs (`AMD64-Windows` versus `x86_64-Linux`). This is not
a comparison of fully compressed ROMs to retail ROM hashes.

Artifacts are under the ignored `build-benchmark-results/` directory:

- `windows-final/results.json`: final Windows timing samples and staged inputs.
- `windows-profile-fixed/`: usable Windows reports, raw `.gmon` files, and inputs.
- `linux-native/`: final native Linux timing samples, logs, build logs, and executable.
- `linux-profile/`: Linux reports, raw profiles, build logs, and exact symbolized executable.
- `linux-mnt-d/`: shared-filesystem timing samples, logs, and staged inputs.
- `verification.json`: baseline/profile and cross-platform code-record checks.

Earlier exploratory directories are retained but are not the final datasets. In particular, `windows-profile/` has empty gprof reports from before the MinGW ASLR fix; its generated assets were reused as preparation inputs. Linux-native input snapshots and build directories are also retained at `/tmp/asl-profile-20261005` in Ubuntu and may be lost when temporary storage is cleared. Windows binaries are in `build-bench-windows/` and `build-profile-windows/`; exact Linux binaries are saved with their results.

Source revisions at collection time (local working trees and generated assets may differ from their commits):

| Repository | Commit |
| --- | --- |
| ASL | `0730cdf2b6156384e8bb291420704e18814f536e` |
| s1disasm_as | `f6ece657c1cf253404312137dfcb8ec15fa42318` |
| s2disasm | `31549c1e3b1a1c4a6ca8c4d37a4d7216d7d10164` |
| skdisasm | `ea95bc8dbe3120967027c8a0565bb51377bf12b3` |

The original disassembly repositories were not modified. No assembler optimization was applied; this change establishes measurements and candidates. See [the guide](README.md) to reproduce runs, include listing cost, and validate each proposed optimization against the existing tests and identical output.
