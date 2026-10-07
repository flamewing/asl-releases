# Upstream sync: master versus experimental (2026-10-07)

Freeze `master` at `3b603b887382d1964305152c295376e4de0c3127` and `experimental` at `fb75b92ddad1d87e6003868b622971b11b9f87c4` (upstream 1.42 build 313). Compare fresh pristine builds, then profile only experimental. No bisection or optimization implementation is part of this comparison. Production sources and the selected branch are unchanged.

## Findings

All nine workloads that assemble on both revisions are slower in every measured pair on all three toolchains. Across the eight variants with identical generated code, geometric-mean elapsed-time increases are **23.55% for Windows GCC, 18.57% for Windows MSVC and 38.89% for native Linux GCC**; child CPU increases are 23.44%, 17.76% and 39.91%. Equal weighting gives each variant the same influence; these are not project-mix weighted totals. Including the changed-output case gives elapsed increases of 23.10%, 18.29% and 38.69%, respectively.

The largest Windows losses are the scalers: 39.19–39.99% with GCC and 30.22–34.11% with MSVC. Their generated code matches master. Linux regressions range from 33.11% to 49.29% across successful variants. The sync adds real CPU work, although the profiles do not attribute every part of the regression to a specific source change.

## Method and validation

Archive each exact commit into separate external source trees. Windows sources/builds/results are under `D:/tmp/asl-bench-20261006/output-experiments/upstream-{master,experimental}-20261007`; native WSL2 Ubuntu artifacts are under `/home/marzojr/asl-benchmark-20261005/output-experiments` with matching names on ext4. Reuse established prepared inputs and assembly flags, including requested listings, without modifying original workload repositories. Neither builds nor outputs are inside the source checkout.

Use x64 GCC 16.2 on Windows, MSVC 19.44 and native Linux GCC 15.2. Normal builds use the established Release settings: GCC `-O3 -DNDEBUG -g`; MSVC `/O2 /Ob2 /DNDEBUG /Zi` and `/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO`. No LTO or profiling hooks enter timed binaries. Master passes all 210 tests and experimental all 316 tests with each toolchain. All builds and suites finish before timing.

After workload preflight, use one warmup per binary and nine measured alternating pairs. Windows interleaves both compilers, reverses pair order and alternates compiler order; Linux alternates the two revisions. Windows and native timing batches run sequentially. Capture child CPU time, memory and native pressure counters. Preflight measurements are not included in the reported medians.

Every successful binary produces deterministic objects, and code records agree across GCC/MSVC and both OSes within each revision. Eight successful workload variants preserve identical code across revisions. Master objects retain the established golden code records. For listings, normalize only page timestamps, predefined TIME/DATE values and assembly-time summaries, and require determinism within each binary. Upstream changes listing layout and version information, so cross-revision text equality is not required. Raw objects, listings, diagnostic logs, executable hashes and exact snapshots remain external.

### Compatibility differences

(*) Big's Fishing Derby produces the same 688,442 payload bytes in 385 records, but 171 addressed bytes differ between master and experimental. This is a generated-byte difference rather than only footer or record layout. Each revision's result agrees across toolchains and OSes. Its timings remain visible but it is excluded from the strict eight-variant aggregate. Follow-up investigation confirmed an upstream CHARSET signedness defect, detailed below.

Both `local-e` and `local-e-debug` fail with experimental on Windows GCC, MSVC and Linux GCC: diagnostics 2310 (invalid multi-character constant length) and 1808 (macro/repetition terminator outside its block), exit status 2 and no object. Master previously assembled both variants successfully. No legacy mode was established, and their sources were not adapted for this comparison. They have no successful-build speed ratio and are excluded from timing aggregates and profile collections. Their identity, provenance, paths and identifying diagnostics are absent from repository documentation.

Follow-up compatibility investigation: these changes require no enabling switch. The official build-313 source archive contains the same relevant implementations as experimental; there is no evidence that these findings were introduced by the sync. The source was checked against [the upstream build-313 archive](http://john.ccac.rwth-aachen.de:8000/ftp/as/source/c_version/asl-current-142-bld313.tar.bz2), SHA-256 `04257c1f9b221cb9629ecee1197468a5deb7108e34be383f9a097e9d09b4558f`.

- Big's Fishing Derby: the string form of CHARSET passes a plain char to an unsigned argument in `as_chartrans_table_set`. On signed-char builds, a replacement byte of 0x86 becomes an out-of-range unsigned value; the setter rejects it and the caller ignores the error. A synthetic mapping of `[` to 0x86 emits 0x86 under master but the original 0x5B under experimental. An external diagnostic build that casts the replacement byte to unsigned char restores complete master code-record equality for this workload, including all 171 changed bytes. This is a confirmed inherited upstream bug.
- Anonymous workload, diagnostic 2310: mixed string/integer addition attempts to interpret the string as a multi-character integer constant, whose valid length is one to four bytes. The observed string has nine bytes. Master silently failed the expression and could retain a previously assigned value; experimental reports the invalid length. The upstream changelog explicitly records the missing-diagnostic fix on 2024-03-25, build 266. A synthetic reassignment from 17 to a nine-character string plus 2 succeeds under master and emits the old value 17, while experimental reports 2310. This is default stricter error reporting for an invalid expression, rather than an optional feature.
- Anonymous workload, diagnostic 1808: this is independent of 2310. Suppressing only the newly reported string-conversion errors in an external diagnostic build leaves 1808. The terminator handler runs while conditional assembly is inactive, without checking `IfAsm`. A synthetic ENDM inside IF 0 succeeds under master but fails with 1808 under experimental. The upstream source contains the same unconditional handler. This appears to be an inherited upstream compatibility bug in handling skipped code, rather than an enabled feature; the intended upstream policy has not been independently established.

An external diagnostic build combining the CHARSET unsigned-byte correction, the old silent mixed-addition behavior, and an inactive-conditional guard on the terminator handler successfully assembles both anonymous variants. Both resulting code records exactly match master. This isolates the observed compatibility failures; preserving silent expression failure is a diagnostic technique, not a proposed production fix.

All diagnostic builds and synthetic fixtures remain outside the repository. Production sources and the frozen timing/profile binaries were not modified during the investigation.

Subsequent compatibility fixes preserve replacement bytes with an unsigned-char conversion in the string CHARSET path and guard the misplaced ENDM/ENDR diagnostic with `IfAsm`. New independent fixtures `t_charsetbytes` and `t_skippedendm` cover low/high replacement bytes, character constants, reset behavior, nested inactive conditional branches, skipped definitions/repetitions, and continued errors for active misplaced terminators. Both fixtures fail against the frozen experimental binary and pass with the fixes. All 318 tests pass on Windows GCC, Windows MSVC, and Linux GCC. Fixed Windows GCC, Windows MSVC, and Linux GCC builds reproduce master code exactly for Big's Fishing Derby. Both anonymous variants now report only error 2310 under Windows GCC and MSVC. Anonymous expression details and source locations are not recorded here. Builds, test outputs and workload diagnostics remain external; the comparison tables above continue to describe the frozen revisions.


### Timings

Median seconds; positive percentages mean experimental is slower than master. Every row has zero experimental wins out of nine pairs. CPU percentages use median child CPU time; short Windows CPU intervals are quantized.

#### Windows GCC

| Workload | Master | Experimental | Wall increase | CPU increase |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.670 | 0.793 | +18.37% | +16.28% |
| Sonic 2 | 1.049 | 1.322 | +26.01% | +27.27% |
| Sonic & Knuckles | 1.282 | 1.551 | +20.94% | +18.52% |
| Big's Fishing Derby (*) | 0.925 | 1.106 | +19.50% | +18.64% |
| Sonic Classic Heroes | 1.502 | 1.735 | +15.53% | +14.74% |
| ScalerMacros-v1.asm | 0.367 | 0.513 | +39.99% | +43.48% |
| ScalerMacros-v2.asm | 1.162 | 1.617 | +39.19% | +39.73% |
| Madness 4 release | 2.562 | 2.951 | +15.19% | +15.53% |
| Madness 4 debug | 2.619 | 3.040 | +16.10% | +15.66% |

#### Windows MSVC

| Workload | Master | Experimental | Wall increase | CPU increase |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.737 | 0.838 | +13.71% | +10.87% |
| Sonic 2 | 1.143 | 1.393 | +21.87% | +22.22% |
| Sonic & Knuckles | 1.423 | 1.600 | +12.47% | +12.22% |
| Big's Fishing Derby (*) | 1.023 | 1.187 | +16.10% | +19.05% |
| Sonic Classic Heroes | 1.594 | 1.797 | +12.72% | +11.88% |
| ScalerMacros-v1.asm | 0.449 | 0.602 | +34.11% | +31.03% |
| ScalerMacros-v2.asm | 1.388 | 1.808 | +30.22% | +30.68% |
| Madness 4 release | 2.701 | 3.050 | +12.91% | +13.45% |
| Madness 4 debug | 2.789 | 3.145 | +12.75% | +11.86% |

#### Native WSL2 Linux GCC

| Workload | Master | Experimental | Wall increase | CPU increase |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.247 | 0.347 | +40.33% | +40.63% |
| Sonic 2 | 0.489 | 0.729 | +49.29% | +49.33% |
| Sonic & Knuckles | 0.504 | 0.694 | +37.72% | +37.63% |
| Big's Fishing Derby (*) | 0.461 | 0.632 | +37.06% | +37.04% |
| Sonic Classic Heroes | 0.535 | 0.716 | +33.70% | +33.53% |
| ScalerMacros-v1.asm | 0.321 | 0.452 | +40.89% | +40.86% |
| ScalerMacros-v2.asm | 1.006 | 1.415 | +40.67% | +40.68% |
| Madness 4 release | 1.002 | 1.334 | +33.11% | +38.20% |
| Madness 4 debug | 1.055 | 1.436 | +36.10% | +38.94% |

Windows timing retains at least 9,034 MiB available memory, with maximum memory load 72%. Native counters show zero swap-in, swap-out or OOM events and zero additional memory-pressure time. Native I/O pressure increases by 5.02 seconds and CPU pressure by 0.35 seconds. Listing wall-time ratios can include storage variation, but measured child CPU increases support the regression conclusion.

## Fresh experimental profiles

Separate GCC diagnostic builds use `-pg -g -fno-omit-frame-pointer`; Windows alone disables ASLR for gmon resolution. Collect three assemblies per successful variant on each GCC platform. Preserve gmon files and call graphs, and require objects to match the corresponding normal experimental binary. Instrumented elapsed times are not production timings. Windows profiling-hook overhead dominates some gprof samples; native `_init` attribution includes unresolved runtime work and must not be interpreted as expensive initialization.

Separately sample the normal optimized Windows GCC and MSVC binaries with the same installed Visual Studio Diagnostics CPU agent. Each collection contains three executions of each of the nine successful variants, with each target launched suspended and attached before resumption. Filter events to recorded target PIDs and execution intervals. Resolve physical function bodies through GCC COFF symbols or the matching MSVC PDB/DIA, including import jump stubs. Both traces have zero lost events, a 1 ms interval and zero unresolved assembler-body samples: 45,065 total target samples for GCC and 46,789 for MSVC. Kernel and DLL-private samples retain category/module attribution rather than guessed function names. Inlining, compiler optimization and sequential collections limit exact cross-compiler comparisons.

### Windows sampled hotspots

CPU sample shares pool all nine variants, weighting by sampled CPU time. They are not wall-time percentages or estimates of speedup achievable by removing a function.

| Function/category | Windows GCC | MSVC |
| --- | ---: | ---: |
| Kernel execution | 33.74% | 31.69% |
| strlen | 5.96% | 5.52% |
| EvalStrExpressionWithCallback | 3.85% | 2.72% |
| operator_search_cb | 2.19% | 2.04% |
| as_iterate_str_quoted | 2.16% | 2.37% |
| isspace | 2.55% | 2.18% |
| vsprcatf_core | 1.05% | 1.73% |

Scaler v2 highlights scanning and copying: strlen is 11.43%/10.14% of GCC/MSVC CPU samples, isspace 5.80%/4.52%, and quoted iteration 2.96%/3.19%. GCC's inlined line-processing work appears in AssembleFile (14.05%); MSVC exposes SplitLine (6.35%), so those names are not directly comparable. Substantial runtime samples remain unresolved by public DLL exports; do not label the entire runtime module as memcpy, allocation or locking.

Sonic 2 exposes expression/operator work: EvalStrExpressionWithCallback is 6.35%/4.34%, operator_search_cb 4.03%/3.71%, and quoted iteration 2.98%/3.09%. Madness 4 release instead gives vsprcatf_core 1.99%/3.43%, alongside strlen 4.94%/4.90%. Kernel shares are large in listing-heavy collections; this does not by itself identify a locking or I/O implementation defect.

### Native instrumented evidence

Counts below are calls per assembly from the three-run Linux GCC collection. They describe instrumented out-of-line calls, not every inlined operation or production CPU share.

| Routine | Sonic 2 | Scaler v2 | Madness 4 release |
| --- | ---: | ---: | ---: |
| `EvalStrExpressionWithCallback` | 352,559 | 628,890 | 533,919 |
| `operator_search_cb` | 20,263,602 | 12,243,492 | 19,875,869 |
| `as_iterate_str_quoted` | 1,726,859 | 2,966,456 | 2,010,493 |
| `StrCompCopySub` | 1,798,229 | 8,937,828 | 2,650,350 |
| `StrCompReset` | 3,718,792 | 20,153,482 | 5,436,208 |
| `LineCompReset` | 5,468,477 | 26,235,222 | 8,053,260 |
| `as_dynstr_copy_c_str` | 5,762,138 | 12,259,984 | 6,928,584 |
| `as_tempres_ini` | 6,768,059 | 5,684,588 | 7,500,374 |
| `vsprcatf_core` | 221,091 | 1,246,376 | 4,952,027 |
| `as_format_context_consume` | 925,034 | 4,333,868 | 25,445,042 |

Native flat self-time on Sonic 2 attributes 16.79% to expression evaluation, 9.16% to operator_search_cb and 6.87% to quoted iteration. On Madness 4 release it attributes 4.98% to vsprcatf_core, 3.98% to as_format_context_consume and 2.49% to append. These are diagnostic sample shares; profiling overhead and unresolved runtime attribution prevent treating them as production fractions. Function boundaries and helpers changed substantially in the sync, so historical call-count changes are not direct measures of added operations.

## Recommended experiments, in order

1. **Known-length line substring copies.** SplitLine now calls StrCompCopySub repeatedly for labels, mnemonics and other fields. That helper rescans the complete source with strlen even when the caller has already bounded the span using p_line_end/p_comment_start. Scaler v2 makes 8,937,828 such calls per assembly. Add a narrow known-length/bounded-span helper for those validated callers, preserving source positions, capacity growth, empty spans and alias semantics. Do not assume Pos.Len always equals the physical string length: preprocessing and reference components can invalidate that assumption. Test this independently before broader copy/trim or reset changes.

2. **Expression operator discovery and a conservative simple-operand path.** Sonic 2 makes 20,263,602 operator-search callbacks for 352,559 expression evaluations. Each evaluation prefilters built-in, target and callback-specific operators with repeated strchr/strstr scans, then scans again through the quote-aware iterator. First measure a first-character dispatch/presence filter reducing full-string operator probes; separately reevaluate a simple-symbol shortcut. Preserve callback precedence, target-specific operators, quoted regions, temporary +/- labels, operator override order, diagnostics and short-circuit evaluation. An unconditional old shortcut is unsafe with the expanded callback semantics.

3. **Retry literal format spans against the new parser.** Formatting now calls as_format_context_consume for each character and also parses configurable listing prefixes. Madness 4 release makes 25,445,042 consume calls and 8,588,545 append calls per assembly. Retry grouping ordinary literal text and bypassing consume while outside a format specifier. Check all new format modifiers, percent escapes, output counts, truncation and growth; preserve handling of invalid custom listing prefixes. The prior discarded trial measured an older formatter and therefore does not settle this candidate now.

4. **Retry a listing-local append cursor, then prefix-format preparation as a separate experiment.** The listing generator was substantially rewritten. It still repeatedly appends through a formatter that discovers destination length with strlen. Keep a local actual length through prefix/body construction and resets, with a narrow helper that avoids perturbing ordinary formatter callers. The prior generic-helper trial caused repeatable MSVC scaler regressions, so recheck non-listing cases and both Windows compilers explicitly. Separately consider preparing the configured prefix into operations when its setting changes, while retaining per-line values, alignment and diagnostics.

5. **Lower-priority expression preparation retests.** Copy/trim fusion, delayed backup copying and lazy operand initialization remain plausible around EvalStrExpressionWithCallback, but do not rerun all discarded experiments indiscriminately. Native temporary-result initialization still occurs millions of times, yet its historical count did not grow, and current profiles prioritize scanning/copying. Reuse ownership/early-error fixtures, extend them for callbacks and short-circuit operations, and test each idea separately. Scope hash caching is lower priority than these measured hotspots.

The accepted float-prefix guard, common whitespace fast path, single-pass macro substitution and batched listing writes are still present in experimental. In particular, ConstFloatVal contains its prefix check; an unconditional call at the expression layer does not mean strtof executes for every failed integer parse. Retrying those exact changes as if absent would be redundant. Previously discarded experiments need adaptations to the new source and fresh measurements rather than automatic acceptance.

Each candidate should use this frozen experimental revision as its first baseline, then the latest accepted candidate. Build/test all three toolchains before timings, screen all comparable variants, confirm meaningful gains in a fresh batch, and remove rejected changes. Address compatibility findings separately; never include failed assemblies in a speed comparison or silently modify the anonymous workload to make it comparable.

External artifacts include comparison/results.json and pressure counters on both platforms; compatibility-failures.json and identifying logs kept external; pristine source archives, build/test logs and snapshot metadata; GCC gprof files/results/summary; Windows sampling sessions, jobs, PDB/COFF resolution and analysis. No profiling artifacts or private adapters are committed into the checkout.

Final audit passes: both pristine source snapshots agree between Windows and native builds for checked source/test files; all six complete baseline/candidate suites pass; all 54 saved timing objects and twelve normalized listings validate; eighteen instrumented case/platform profiles and eighteen sampled case/compiler profiles match their normal experimental objects. Every workload/revision code hash agrees across toolchains and OSes.
