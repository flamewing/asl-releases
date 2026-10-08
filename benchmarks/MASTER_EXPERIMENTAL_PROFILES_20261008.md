# Master versus experimental: refreshed comparison and profiles

Date: 2026-10-08. Baseline: `master` at `3b603b887382d1964305152c295376e4de0c3127`. Candidate: `experimental` at `9abb13356cf46c5707dda40746e0009d5750d992`. These are the revisions tested; a later documentation commit does not change the measured executable.

The experimental head remains slower on this workload cohort. Every workload produces identical assembled code across both revisions and all three toolchains. The most promising next changes remove repeated string work in expression recognition and source-line processing. The opportunities below are hypotheses for independent experiments, not measured speedups.

## Comparison

Positive percentages mean experimental takes longer. Aggregate values are equal-weight geometric means of the eleven head/master ratios, calculated separately for elapsed and process CPU time.

| Toolchain | Elapsed increase | CPU increase |
| --- | ---: | ---: |
| Windows GCC | 9.21% | 8.65% |
| Windows MSVC | 7.31% | 6.89% |
| Linux GCC | 16.31% | 17.62% |

Elapsed medians in seconds, followed by the relative increase:

| Workload | Windows GCC: master → head | Δ | Windows MSVC: master → head | Δ | Linux GCC: master → head | Δ |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.7114 → 0.8256 | +16.06% | 0.7565 → 0.8394 | +10.96% | 0.2646 → 0.3056 | +15.47% |
| Sonic 2 | 1.1236 → 1.1748 | +4.55% | 1.2079 → 1.2728 | +5.37% | 0.5087 → 0.5894 | +15.87% |
| Sonic & Knuckles | 1.3219 → 1.4073 | +6.46% | 1.4477 → 1.5398 | +6.36% | 0.5198 → 0.6109 | +17.52% |
| Big’s Fishing Derby | 0.9625 → 1.0061 | +4.53% | 1.0529 → 1.1109 | +5.51% | 0.4903 → 0.5620 | +14.61% |
| Sonic Classic Heroes | 1.5531 → 1.6263 | +4.71% | 1.6631 → 1.7671 | +6.26% | 0.5628 → 0.6455 | +14.69% |
| ScalerMacros-v1.asm | 0.3862 → 0.4562 | +18.12% | 0.4700 → 0.5254 | +11.79% | 0.3291 → 0.3998 | +21.51% |
| ScalerMacros-v2.asm | 1.2144 → 1.4512 | +19.50% | 1.4893 → 1.6666 | +11.91% | 1.0407 → 1.2838 | +23.36% |
| Madness 4 release | 2.6333 → 2.7651 | +5.01% | 2.7876 → 2.9111 | +4.43% | 1.0326 → 1.2127 | +17.44% |
| Madness 4 debug | 2.6570 → 2.8194 | +6.11% | 2.8617 → 2.9962 | +4.70% | 1.3271 → 1.4501 | +9.27% |
| local-e | 2.0629 → 2.2427 | +8.71% | 2.2500 → 2.3917 | +6.30% | 1.1900 → 1.3604 | +14.32% |
| local-e-debug | 2.0625 → 2.2478 | +8.98% | 2.2678 → 2.4321 | +7.25% | 1.2042 → 1.3961 | +15.94% |

Process CPU changes, which help separate computation from elapsed-time variation:

| Workload | Windows GCC | Windows MSVC | Linux GCC |
| --- | ---: | ---: | ---: |
| Sonic 1 | +4.44% | +4.17% | +15.45% |
| Sonic 2 | +4.29% | +6.58% | +15.85% |
| Sonic & Knuckles | +7.14% | +5.49% | +17.42% |
| Big’s Fishing Derby | +5.00% | +7.58% | +15.97% |
| Sonic Classic Heroes | +5.15% | +4.81% | +14.91% |
| ScalerMacros-v1.asm | +20.83% | +13.79% | +21.51% |
| ScalerMacros-v2.asm | +21.05% | +11.70% | +23.37% |
| Madness 4 release | +6.06% | +3.98% | +17.98% |
| Madness 4 debug | +5.95% | +5.56% | +17.27% |
| local-e | +8.46% | +6.34% | +17.40% |
| local-e-debug | +8.40% | +6.25% | +16.95% |

Experimental was faster in 8 of 99 Windows GCC pairs, 11 of 99 MSVC pairs, and 2 of 99 Linux pairs. The scaler workloads have the largest consistent regressions.

### Method and correctness

Fresh pristine Git archives were built for each revision. Windows GCC 16.2 and native Ubuntu/WSL2 GCC 15.2 used `-O3 -DNDEBUG -g`; MSVC 19.44 used `/O2 /Ob2 /DNDEBUG /Zi`, with `/DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO` at link time. LTO and diagnostic instrumentation were disabled for timing. Each toolchain passed its full suite: 210 master tests and 327 experimental tests, six successful suites and 1,611 tests in total.

Each workload/binary had one warmup and nine alternating measured runs. Windows GCC/MSVC runs were interleaved; native runs followed separately. This gives 594 measured assemblies and 66 warmups. Builds, timing batches, normal sampling and diagnostic collections did not overlap. Native sources, inputs, executables and outputs were on ext4; `os.sync()` was outside the measured interval. Windows process CPU uses GetProcessTimes and native CPU uses getrusage.

All object outputs were deterministic within each binary. Comparison of code records, including addresses and record layout but excluding the platform creator footer, passed across all six binaries for all eleven cases. Timestamp-normalized listings were deterministic within each binary; listing equality between revisions was not required because upstream formatting and symbol order changed. Prepared inputs and compatibility adaptations were identical for both revisions. local-e and local-e-debug are intentionally anonymous.

Windows had at least 12,616 MiB available memory and at most 61% memory load. The native batch recorded no memory-pressure stalls, swap-in/swap-out changes or OOM events. This refresh uses external build/output directories throughout. Historical reports used different cohorts and conditions, so their percentages should not be subtracted from this report to estimate recovered performance. These results do not assign the residual regression to any particular commit.

## Fresh profiles

Normal optimized Windows GCC and MSVC binaries were sampled independently, with three executions of every workload per compiler: 66 sampled assemblies. Analysis selects the target process/thread and execution interval. There were 56,413 GCC and 59,843 MSVC samples, zero unresolved executable samples and zero lost ETW events. GCC physical symbols and matching MSVC PDBs were used; runtime functions were named only where an export matched its actual function range.

Selected physical-function shares of all target CPU samples are below. A compiler can inline a helper into a different parent, so percentages for different labels cannot be compared as though they measured identical semantic stages. Runtime strlen figures exclude separately labelled import thunks. These are CPU sample shares, not elapsed-time savings.

| Workload | Compiler | strlen | isspace | EvalStrExpressionWithCallback | operator_search_cb | as_iterate_str_quoted | SysString + vsprcatf_core |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 2 | GCC | 5.66% | 3.14% | 9.51% | inlined | 0.72% | 0.49% |
| Sonic 2 | MSVC | 5.04% | 1.95% | 3.75% | 3.70% | 3.65% | 0.71% |
| ScalerMacros-v2.asm | GCC | 10.45% | 5.77% | 5.39% | inlined | 1.87% | 1.40% |
| ScalerMacros-v2.asm | MSVC | 8.33% | 5.24% | 2.59% | 1.71% | 2.73% | 2.29% |
| Madness 4 release | GCC | 4.84% | 1.48% | 4.85% | inlined | 0.26% | 3.34% |
| Madness 4 release | MSVC | 4.38% | 1.75% | 1.89% | 1.54% | 1.61% | 4.72% |
| local-e-debug | GCC | 6.86% | 2.30% | 5.15% | inlined | 0.67% | 5.13% |
| local-e-debug | MSVC | 6.03% | 1.83% | 2.50% | 1.76% | 1.79% | 7.71% |

Kernel samples are kept separate: they reach approximately 44–47% for Madness 4 release and 27–30% for local-e-debug. These observations do not identify a specific I/O, allocator or locking defect. Unresolved runtime-module samples are likewise not assigned to a named helper.

GCC diagnostic builds on both Windows and native Linux used `-pg -fno-omit-frame-pointer` with distinct non-inlined stage wrappers and explicit aggregate counters. Three repetitions per case gave identical counts; Windows and Linux agreed exactly. All diagnostic objects matched their corresponding normal GCC objects. A separate literal-versus-addition calibration verified the expected recursive evaluator, backup, numeric-probe and operator-scan deltas on both platforms.

Explicit counters are authoritative for calls: gprof flat evaluator counts omit recursion, and Windows gprof does not reliably count all leaf wrappers. Initial diagnostic collections are retained separately and are not used for the following counts. Instrumented timings and gprof self-time are not production speed estimates; Windows profiling hooks consume a large share, and native gprof omits dynamic-runtime self-time. Linux stage counts/call graphs were collected, but no equivalent normal native sampling was obtained, so native CPU attribution is less precise than Windows attribution.

| Workload | Evaluations | Line splits | String probes | String matches | Operator scans | Scans without operator |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 777,785 | 432,496 | 506,175 | 14,707 | 477,978 | 56.75% |
| Sonic 2 | 2,110,186 | 720,970 | 1,414,289 | 35,053 | 1,329,190 | 53.38% |
| Sonic & Knuckles | 1,424,409 | 748,380 | 887,449 | 6,598 | 819,191 | 69.53% |
| Big’s Fishing Derby | 1,492,241 | 712,910 | 999,845 | 32,318 | 898,962 | 62.15% |
| Sonic Classic Heroes | 1,279,226 | 814,924 | 925,154 | 44,398 | 817,674 | 65.79% |
| ScalerMacros-v1.asm | 889,456 | 969,714 | 698,100 | 182 | 697,378 | 58.76% |
| ScalerMacros-v2.asm | 1,491,138 | 4,019,338 | 1,253,116 | 182 | 1,252,394 | 70.53% |
| Madness 4 release | 2,284,439 | 1,063,720 | 1,507,201 | 42,666 | 1,436,271 | 56.83% |
| Madness 4 debug | 2,357,211 | 1,139,224 | 1,564,177 | 51,930 | 1,483,975 | 57.25% |
| local-e | 2,268,755 | 1,338,543 | 1,374,798 | 20,621 | 1,320,442 | 61.14% |
| local-e-debug | 2,291,345 | 1,355,724 | 1,392,207 | 22,943 | 1,335,517 | 61.22% |

A string match means the string parser recognized the expression; it does not count every expression starting with a quote. Likewise, a scan finding no operator does not establish that it was a plain identifier or eligible for a safe shortcut.

## High-value opportunities

### 1. Reject unquoted string probes before measuring their length

In `ConstStringVal` (`asmpars.c:1330`), `strlen` currently precedes the opening-quote check. Move the opening-byte check ahead of it, retaining `*pResult = False` and all quoted-string handling. Scaler v2 makes 1,253,116 string probes per assembly but recognizes only 182 strings; the other workloads also have low recognition rates. This is a small portable experiment with a direct source-level explanation for avoided work. It is distinct from the previously rejected general expression/symbol shortcut.

Validate empty input, a lone quote, unmatched quotes, escaped quotes, single-quoted flags, character constants, interpolation, non-ASCII data and normal diagnostic locations. Run the full suites and all three timing toolchains. Count first-byte eligibility if estimating avoided scans; do not treat all failed string probes as eligible or all sampled strlen time as recoverable.

### 2. Carry physical character lengths through the line pipeline

Macro output records its length with `strlen` (`as.c:603`), ProcessFile scans the resulting line again (`as.c:3072`), and SplitLine scans again after ExpandDefines (`as.c:2873`). ExpandTokens and the physical-line reader already track useful lengths internally. Scaler v2 passes through 4,019,338 line splits per assembly, while strlen occupies 10.45% of GCC and 8.33% of MSVC sampled CPU across the whole workload.

First return or carry the actual output character length from producers; then consume it in ProcessFile; finally avoid SplitLine’s rescan when define expansion leaves the string unchanged. Keep these as independent stages. ReadLnCont currently returns a physical-line count, so that return value must not be reinterpreted as character length. Pos.Len has source-position uses and is not automatically a trustworthy storage-length cache.

Validate long dynamically grown lines, empty lines, EOF/DOS EOF, CRLF, continuation counts, nested macros, IRP/IRPC/IRPN, SHIFT, changed-length defines and error source positions. Retest listings and all code outputs. This has greater implementation risk than the first experiment; the sampled strlen total spans many other callers and is only an upper bound.

### 3. Remove repeated runtime whitespace classification where the contract permits

The shared as_isspace already fast-paths space and tab (`strutil.h:130`). Non-space bytes still reach the runtime classifier; isspace alone occupies 5.77% of GCC and 5.24% of MSVC scaler-v2 CPU samples. Investigate one portable byte-classification table covering every possible input byte, initialized from the existing classification contract, rather than repeating the previously retained space/tab shortcut.

Before implementation, establish the locale and initialization contract for the assembler and other utilities. No LC_CTYPE-changing calls were found in the current C/H sources; nls_init changes LC_TIME and LC_MONETARY. That alone is not sufficient to silently weaken helper semantics. Preserve chartouint’s masking, all 256 classifications and signed-character behavior, including how the current helper treats EOF. If locale changes must remain supported, provide a defined refresh mechanism or discard this approach. Verify initialization before standalone utility use. Linux libc may already implement classification efficiently, so native gains need independent confirmation; use the same production code on every platform.

## Medium-value opportunities

### 4. Avoid unnecessary numeric-probe setup

In ConstFloatVal (`asmpars.c:1265`), the hexadecimal exclusion computes strlen before checking whether the first byte is zero. A first-byte/next-byte check can remove that length scan without changing conversion handling. The broad float prefix guard is already present and should not be reintroduced as a new experiment.

Separately, const_int_val (`asmpars.c:1149`) divides by the radix to compute mul_max before rejecting the first invalid digit. Count first-digit failures and inspect generated code before experimenting with delayed calculation that reuses the first digit. This is different from the rejected complete digit-count prescan. Preserve format precedence, arbitrary radix, base-256 character translation and warnings, signed limits, overflow, digit-extension rules and malformed-input results. Scaler v2 has 1,491,048 integer probes but only 237,932 matches; those totals do not prove how many divisions can be avoided. Benchmark the two changes independently.

### 5. Reuse existing expression-scan information

Operator scanning and quote traversal remain visible, especially in MSVC: Sonic 2 spends 3.70% in operator_search_cb and 3.65% in as_iterate_str_quoted. Between 53% and 71% of explicit operator scans find no operator. Investigate carrying lexical facts from an already-required walk or simplifying work within that walk, without adding another identifier prescan.

The prior plain-ASCII symbol shortcut failed to give consistent improvements. A retry needs a different mechanism and new evidence. Preserve longest-match/operator precedence, quotes, custom operators, temporary plus/minus symbols, parentheses/functions, register callbacks, bracket errors and short-circuit evaluation. Compiler inlining puts much of GCC scanning inside EvalStrExpressionWithCallback; avoid adding a platform-specific implementation.

### 6. Reduce repeated ordinary-symbol lookup work

Measure ordinary-symbol get/set calls, insertion misses, hashes and probes, then investigate reusing a name hash across scope attempts or performing insertion with one probe sequence. MSVC openhash_get occupies roughly 1–2% of samples in representative projects. Preserve scoped precedence, duplicate definitions, callbacks that reject insertion, key ownership, symbol replacement and forward-reference diagnostics.

SearchSymbolHashTable and EnterSymbolHashTable initialize temporary keys in the source (`asmpars.c:324`, `asmpars.c:343`), but inspected optimized GCC lookup code already eliminates the unused key memset. Removing that source initialization alone is therefore not a strong portable candidate. Main-hash function replacements and the forward-table finalizer recently failed independent timings; the new profiles do not justify repeating them without new evidence. The forward-reference table is not the principal remaining real-workload hotspot.

### 7. Specialize measured common listing fragments

SysString plus vsprcatf_core accounts for approximately 5.13% GCC and 7.71% MSVC local-e-debug CPU samples, and 3.34%/4.72% for Madness 4 release. Instrument the actual fragment mix, then replace a narrowly selected common numeric fragment with shared typed appends. Keep generic formatting for other cases.

Generic literal spans, listing append cursors and prefix preparation were previously rejected. A useful new trial needs a concrete common-format specialization. Preserve radix 2–36, width/padding, case, prefixes, address granularity, endianness, retractions, multi-line layouts and suppressed listings. Compare listing contents against the same parent revision. Large kernel shares do not justify assigning all remaining time to buffered writes.

## Experiment order and acceptance

Recommended order: string-probe gate; independent line-length stages; whitespace classification if its contract is established; float exclusion check; delayed integer division if eligibility and generated-code evidence support it; expression scan reuse; ordinary-symbol lookup reuse; common listing specialization. Start each from the current accepted head, make one production change at a time, and compare with its direct parent across Windows GCC, Windows MSVC and native Linux GCC.

Use paired repetitions across every workload, inspect elapsed and CPU times, verify code and normalized listings, and run correctness tests appropriate to the changed contract. Confirm promising results in a separate batch before committing. Do not accept a large workload-specific or platform regression because the aggregate improves; document tradeoffs explicitly. No candidate in this report has yet been implemented or timed.

## Artifacts

Windows source archives, normal/diagnostic builds, full test logs, nine-pair results, objects/listings, memory observations, ETL samples, symbol maps and audits are retained under `D:/tmp/asl-bench-20261006/output-experiments/refresh-master-20261008` and `refresh-experimental-20261008`. Native equivalents are under `/home/marzojr/asl-benchmark-20261005/output-experiments/` with the same directory names.

Head summaries: `comparison-summary.json`, `sampling/analysis.json`, `stage-counters-summary.json`, `gprof-summary.json` and `audit.json`. Final diagnostic collections are in `gprof/`; earlier diagnostic artifacts are preserved in `gprof-initial/`. The exact diagnostic-only modifications are saved in `diagnostic.patch`, and independent counter checks in `counter-calibration/results.json`. Diagnostic wrappers/counters are absent from the pristine source and normal binaries. Private source paths, logs and identifiers remain external; only anonymous local-e aggregate data appears here.
