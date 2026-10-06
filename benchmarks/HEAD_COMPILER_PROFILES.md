# Fresh aggregate timings and compiler profiles (2026-10-06)

This comparison freezes original `0730cdf` and current HEAD `2369d51eb1f8d3f92573400f86120de0b36d7cef`. It includes all eleven registered workload variants. Only `local-e` remains anonymous; its identity and provenance are excluded from repository documentation. Raw traces stay outside the repository. Sources, prepared inputs, builds and reports are external; original workload repositories are unchanged.

## Builds and checks

Separate pristine Git archives provide the two source snapshots. Windows uses x64 MinGW GCC 16.2.0 and installed MSVC 19.44; native WSL2 Ubuntu uses GCC 15.2.0. Normal GCC builds use `-O3 -DNDEBUG -g`; MSVC uses `/O2 /Ob2 /DNDEBUG /Zi`, linking with `/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO`. Symbols support sampling; normal timed binaries contain no profiling hooks. No link-time optimization is added to either compiler. Each compiler/platform passes its complete suite: 202 tests for the original revision and 210 for HEAD.

Separate GCC diagnostic builds use `-pg -g -fno-omit-frame-pointer` and `-pg` at link time. Windows gprof binaries alone disable ASLR for gmon address resolution. Explicit diagnostic flags work at both revisions: `0730cdf` does not include the later profiling CMake module, whereas HEAD supports `ASL_PROFILE`. These runs set that option to OFF and supply explicit flags to avoid relying on differing CMake feature availability.

## Uninstrumented timing method

Existing prepared workdirs are reused without copying input trees between runs. One warmup per binary precedes nine measured rounds. Windows interleaves original/HEAD pairs for both compilers, reverses pair order and alternates compiler order. Linux alternates its original/HEAD pair. Windows and Linux batches run sequentially after every build and correctness suite finishes.

Record elapsed time and child CPU time using `GetProcessTimes` on Windows and `getrusage` on Linux. Windows monitoring records available memory; native reports preserve CPU/I/O/memory pressure and swap counters. Windows CPU-time quantization limits interpretation of small changes. Preserve actual per-case assembly flags: variants that request listings keep them; the previously established workloads keep their existing configuration. These comparisons measure ASL, excluding compression and ROM post-processing.

Every assembly must succeed and reproduce deterministic output. Compare code records across revisions, Windows compilers and OSes, allowing the platform-specific creator footer to differ.

## Profiling method

Fresh gprof collections repeat each workload three times at each revision on both GCC platforms. These diagnose call counts and instrumented self-time; their elapsed times do not estimate production speedups.

For the Windows compiler comparison, the same installed Visual Studio Diagnostics CPU agent samples the normal optimized GCC and MSVC binaries. Each compiler/revision has a separate collection containing three executions of each case. Launch each child suspended, attach the collector before execution, then resume it. Record target PIDs and execution intervals. Decode ETL samples with Windows `tracerpt` and retain only those target processes and intervals in reported results. Unrelated system events are excluded.

Map relocated instruction pointers through each process's loaded images. Resolve GCC physical function bodies from the executable's COFF symbols and MSVC physical function bodies from its matching PDB through the installed DIA SDK. This avoids comparing GCC inline-frame attribution with MSVC outer function attribution. Decode import jump stubs through PE import tables for both compilers instead of treating them as missing PDB functions. Identical-code folding and compiler inlining still limit direct function-by-function comparison. Separate executable, runtime DLL, Windows user API, kernel and unresolved samples; sampled CPU share is not a wall-time percentage. Public DLL function names are used only where an export matches a recorded unwind-function range.

All profiling outputs must match the corresponding normal binaries' objects. The raw diagnostic sessions, gmon files, exact binaries, PDBs and analysis tools stay external. The collection procedure follows Microsoft's [command-line profiling documentation](https://learn.microsoft.com/en-us/visualstudio/profiling/profile-apps-from-command-line?view=vs-2022).

## Results

The geometric mean of HEAD/original median elapsed-time ratios across all eleven variants gives **42.02% less time for Windows GCC, 41.30% for Windows MSVC, and 48.06% for Linux GCC**. Each variant has equal weight; this is not a weighted estimate for a particular project mix. Corresponding child CPU-time reductions are 42.47%, 41.28% and 50.93%.

All Windows cases improve in all nine paired rounds for both compilers. Linux improves in all nine rounds for ten variants, and eight of nine for `local-e-debug`. Both anonymous variants were measured in the same batch as the other workloads and were already included in the aggregate results. Their anonymized measurements are reported below; identifying information and raw artifacts remain external.

### Windows GCC

Median elapsed seconds; reduction is `1 - HEAD/original`.

| Workload | Original | HEAD | Reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | 1.158 | 0.681 | 41.23% |
| Sonic 2 | 2.041 | 1.080 | 47.12% |
| Sonic & Knuckles | 1.932 | 1.304 | 32.50% |
| Big's Fishing Derby | 1.713 | 0.949 | 44.61% |
| Sonic Classic Heroes | 2.233 | 1.540 | 31.04% |
| ScalerMacros-v1.asm | 1.164 | 0.430 | 63.04% |
| ScalerMacros-v2.asm | 3.749 | 1.323 | 64.72% |
| Madness 4 release | 4.559 | 3.267 | 28.34% |
| Madness 4 debug | 4.696 | 3.350 | 28.66% |
| local-e | 4.247 | 2.880 | 32.20% |
| local-e-debug | 4.292 | 2.952 | 31.22% |

### Windows MSVC

| Workload | Original | HEAD | Reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | 1.164 | 0.727 | 37.52% |
| Sonic 2 | 2.148 | 1.163 | 45.83% |
| Sonic & Knuckles | 2.041 | 1.388 | 31.99% |
| Big's Fishing Derby | 1.810 | 1.035 | 42.81% |
| Sonic Classic Heroes | 2.365 | 1.621 | 31.45% |
| ScalerMacros-v1.asm | 1.212 | 0.471 | 61.15% |
| ScalerMacros-v2.asm | 3.765 | 1.461 | 61.21% |
| Madness 4 release | 4.142 | 2.897 | 30.06% |
| Madness 4 debug | 4.275 | 2.959 | 30.79% |
| local-e | 3.592 | 2.377 | 33.81% |
| local-e-debug | 3.659 | 2.396 | 34.53% |

### Native WSL2 Linux GCC

| Workload | Original | HEAD | Reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | 0.564 | 0.259 | 54.03% |
| Sonic 2 | 1.148 | 0.519 | 54.78% |
| Sonic & Knuckles | 0.913 | 0.525 | 42.54% |
| Big's Fishing Derby | 0.979 | 0.472 | 51.77% |
| Sonic Classic Heroes | 1.005 | 0.555 | 44.76% |
| ScalerMacros-v1.asm | 0.895 | 0.334 | 62.65% |
| ScalerMacros-v2.asm | 2.959 | 1.040 | 64.84% |
| Madness 4 release | 2.014 | 1.293 | 35.82% |
| Madness 4 debug | 2.130 | 1.345 | 36.87% |
| local-e | 2.152 | 1.424 | 33.81% |
| local-e-debug | 2.166 | 1.434 | 33.79% |

Windows monitoring retains at least 13,982 MiB available memory, with maximum load 57%. Linux has no memory-pressure accumulation, swap traffic or OOM events. However, Linux I/O-pressure totals increase by about 25 seconds during the batch, and recent I/O pressure remains measurable. CPU-time reductions support the performance conclusion, while individual wall measurements still include storage variation. These fresh isolated-build timings supersede older aggregate estimates affected by source-tree watchers or uncertain incremental objects.

### MSVC versus GCC on Windows

For HEAD, MSVC is 5.3–10.4% slower across the seven established cases, but 11.3% faster on Madness 4 release and 11.7% faster on its debug variant. MSVC is also 17.44% faster on `local-e` and 18.85% faster on `local-e-debug`. Across all eleven variants the geometric mean gives MSVC only 1.02% less elapsed time than GCC (0.50% less CPU time). That small aggregate compiler difference is not evidence of a universal winner; the workload-specific differences are considerably larger. The comparisons include each compiler's normal C runtime as well as its generated code.

### Windows CPU sampling findings

All four traces report zero lost events and a 1 ms sampling interval. All sampled executable addresses resolve after identifying import jump stubs consistently for both compilers. DLL-private and kernel samples retain module/category attribution rather than guessed function names.

The table pools three executions of all eleven cases, weighting cases by sampled CPU time. Percentages describe running CPU, including kernel execution on target threads; they do not describe wall time or blocked I/O. Compiler collections are sequential, so modest cross-collection differences should be treated cautiously.

| Sampled function/group | GCC original | GCC HEAD | MSVC original | MSVC HEAD |
| --- | ---: | ---: | ---: | ---: |
| `EvalStrExpression` | 3.41% | 4.44% | 3.29% | 4.56% |
| `strlen`, including import dispatch | 8.58% | 4.87% | 8.61% | 5.09% |
| `strncmp`, including import dispatch | 9.28% | 0.28% | 6.41% | 0.28% |
| Delimiter functions (`Quot*`, `SearchMultString`) | 4.61% | 0.85% | 7.14% | 1.33% |
| Macro processor/replacement bodies | 4.26% | 0.58% | 4.28% | 0.67% |
| `vsprcatf_core` | 1.07% | 1.68% | 2.26% | 3.41% |
| `RtlEnter/LeaveCriticalSection` | 5.07% | 8.06% | 0.51% | 0.83% |
| Kernel execution | 20.62% | 32.34% | 21.57% | 33.94% |

Macro and delimiter costs collapse in both compilers. The expression evaluator takes a larger fraction of the smaller remaining total; this is not evidence that expression evaluation became slower. Sonic 2 HEAD attributes 9.1%/8.4% to `EvalStrExpression` for GCC/MSVC. `ScalerMacros-v2.asm` still attributes 15.8%/10.9% to its runtime's `isspace`, making whitespace processing a useful target to inspect independently of macro substitution.

There **is a meaningful compiler/runtime profile difference**, mainly around listing-heavy workloads. GCC uses `msvcrt.dll`; MSVC uses `ucrtbase.dll` and `vcruntime140.dll`. Madness 4 release HEAD attributes 10.5% to the named critical-section functions for GCC versus 1.0% for MSVC; its formatting body takes 2.0% versus 4.7%. Across three pooled HEAD repeats, critical-section shares stay at 7.83–8.25% for GCC and 0.81–0.85% for MSVC; formatting stays at 1.64–1.70% versus 3.30–3.47%. These are repeatable attribution differences. They suggest inspecting listing/output and runtime synchronization, but flat samples alone do not establish the caller or explain the whole timing gap.

Inlining changes function boundaries: GCC often absorbs line-processing work into `AssembleFile`, while MSVC exposes more `SplitLine`, `ProcessFile` and `openhash_get_symbolhash` samples. Do not mistake those labels for different algorithms. Both compilers retain the same broad remaining targets: expression evaluation, string/whitespace processing, symbols, formatting and output.

### Anonymous-case Windows sampling

The anonymous cases show the same broad Windows compiler/runtime differences. These shares use three sampled executions per compiler/revision and the attribution rules above; different inlining and runtime implementations limit direct function comparisons.

| Case | Sampled function/group | GCC original | GCC HEAD | MSVC original | MSVC HEAD |
| --- | --- | ---: | ---: | ---: | ---: |
| local-e | `EvalStrExpression` | 3.12% | 4.03% | 3.33% | 4.24% |
| local-e | `strncmp`, including import dispatch | 7.39% | 0.17% | 5.90% | 0.22% |
| local-e | `vsprcatf_core` | 1.98% | 2.72% | 4.52% | 6.62% |
| local-e | `RtlEnter/LeaveCriticalSection` | 10.33% | 14.53% | 0.86% | 1.27% |
| local-e-debug | `EvalStrExpression` | 3.21% | 3.54% | 3.57% | 4.04% |
| local-e-debug | `strncmp`, including import dispatch | 7.49% | 0.14% | 5.98% | 0.15% |
| local-e-debug | `vsprcatf_core` | 2.10% | 2.97% | 4.59% | 6.49% |
| local-e-debug | `RtlEnter/LeaveCriticalSection` | 9.87% | 14.42% | 0.74% | 1.15% |

The large drop in string comparisons agrees with the reduced replacement work. The GCC/MSVC critical-section attribution difference also persists in these cases; flat samples do not establish how much of the timing gap it explains. Linux child CPU time falls 42.26% for `local-e` and 43.22% for `local-e-debug`, compared with roughly 34% less wall time, consistent with the recorded I/O variation affecting elapsed measurements.

### Instrumented call counts and verification

Counts below are normalized per assembly from three runs, and agree exactly between Windows GCC and Linux GCC. `ExpandTokens` is the new single-pass path; its calls must be considered alongside the remaining repeated-replacement calls.

| Workload | `StrCompAlloc` original → HEAD | `ReplaceLineUnchecked` original → HEAD | HEAD `ExpandTokens` |
| --- | ---: | ---: | ---: |
| Sonic 1 | 1,680,828 → 23,818 | 1,479,714 → 36,828 | 129,136 |
| Sonic 2 | 4,454,689 → 55,471 | 2,535,532 → 84,448 | 237,026 |
| Sonic & Knuckles | 3,147,949 → 15,701 | 869,844 → 44,784 | 102,140 |
| Big's Fishing Derby | 3,367,276 → 50,346 | 1,749,395 → 64,027 | 216,534 |
| Sonic Classic Heroes | 2,891,384 → 56,724 | 1,326,118 → 124,314 | 167,972 |
| ScalerMacros-v1.asm | 1,931,896 → 214 | 2,589,370 → 540 | 241,454 |
| ScalerMacros-v2.asm | 3,136,343 → 217 | 6,140,822 → 540 | 286,446 |
| Madness 4 release | 4,669,503 → 43,873 | 3,624,712 → 18,240 | 310,060 |
| Madness 4 debug | 4,833,419 → 53,445 | 3,745,524 → 18,244 | 322,088 |
| local-e | 4,883,151 → 41,527 | 3,399,477 → 19,068 | 379,455 |
| local-e-debug | 4,932,207 → 43,891 | 3,423,516 → 19,074 | 381,525 |

`EvalStrExpression` counts are unchanged at both revisions and agree across both GCC platforms for every variant. For example, Sonic 2 remains at 351,532 calls per assembly and Madness 4 release at 533,489. The anonymous variants remain at 937,055 and 941,153 calls per assembly, respectively. Savings come from reducing work around evaluation and replacement, rather than skipping required expressions.

Native Linux gprof also shifts attention from `MACRO_Processor`, `ReplaceLineUnchecked` and `QuotSMultPosQualify` toward the expression evaluator, line processing, symbols and formatting. Treat its coarse self-time as a guide: profiling changes execution costs, Windows profiles spend substantial time in `_mcount_private`/`__fentry__`, and Linux has a large `_init` attribution bucket that should not be interpreted as actual initialization work. Use the normal binary timings and Windows CPU sampling for speed and compiler conclusions.

The final external audit verifies all 66 normal workload/binary combinations, 132 instrumented assemblies, 132 sampled assemblies and six complete CTest suites. Normal comparisons contain 594 measured assemblies plus 66 warmups. Objects are deterministic within each platform/compiler; all eleven absolute code-record sets match across revisions, compilers and OSes. Exact executable hashes and selected production sources from both pristine snapshots are checked against the frozen revisions. These checks validate assembly, not complete compressed ROM pipelines. No production source changes are part of this report.

## Artifact locations

Windows: `D:/tmp/asl-bench-20261006/aggregate-head-20261006`.

Linux: `/home/marzojr/asl-benchmark-20261005/aggregate-head-20261006`.

Each has `original`, `head`, `timings` and `gprof` directories; Windows also has `sampling`. External helpers named `aggregate-head-*` reproduce build, timing, capture and analysis steps using external case definitions.
