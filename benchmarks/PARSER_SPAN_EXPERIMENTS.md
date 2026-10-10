# Borrowed parser strings

The starting production baseline is `329c60f0`; `b6ac09ea` adds its final validation report. Build, diagnostic and timing artifacts remain outside the source tree, under the existing Windows and native Linux benchmark roots. Private workload source material and manifests are not committed. Markdown paragraphs are not wrapped.

## Stages

1. Instrument expression and component copies, recording call counts and bytes without using diagnostic timings as speed estimates.
2. Introduce immutable borrowed spans and bounded slice, trim, comparison and search helpers. A span does not own storage or promise a trailing NUL; its owner must remain alive and unchanged until the consumer returns.
3. Make integer constant parsing consume a bounded span, preserving literal-format priority, character translation, overflow and symbol flags.
4. Decode quoted strings from borrowed input, keeping decoded output owned and materializing only where an existing interface requires termination.
5. Propagate spans through operator and function argument splitting and recursive expression evaluation. Keep owned adapters at mutable callback boundaries; preserve source positions and short-circuit order.
6. Inventory line components and other consumers, checking mutation and pointer retention before extending borrowing into them.
7. Independently compare streaming input, immutable buffered input and immutable mapped input. Hide separate Windows and POSIX mapping implementations behind an abstraction with a portable fallback. Continuations, transformations and persistent macro bodies still require appropriate owned storage.

Each executable change receives full tests and paired workload comparisons with Windows GCC, Windows MSVC and native Linux GCC. Meaningful improvements receive independent confirmation before committing. Infrastructure that enables later copy removal is reported separately from measured optimization wins.

## Copy inventory

The diagnostic `span-copy-inventory-20261009-r2` ran each of the eleven workloads twice. Both runs produced identical counters and object hashes matching the normal baseline. The diagnostic changes were then removed from the external source snapshot. Input and backup copies happen once per expression evaluation; the categories below describe parser sites and must not be added to overlapping generic component-copy counters.

| Workload | Evaluations | Input bytes | Backup bytes | Symbol restore bytes |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 777,785 | 7,759,490 | 7,750,310 | 1,213,490 |
| Sonic 2 | 2,110,186 | 22,318,487 | 22,325,747 | 2,849,211 |
| Sonic & Knuckles | 1,424,409 | 13,367,645 | 13,345,608 | 3,767,714 |
| Big Fishing Derby | 1,492,241 | 12,862,701 | 12,853,297 | 2,816,825 |
| Sonic Classic Heroes | 1,279,226 | 13,245,732 | 13,226,435 | 3,626,117 |
| ScalerMacros-v1.asm | 889,456 | 4,325,370 | 4,325,190 | 589,758 |
| ScalerMacros-v2.asm | 1,491,138 | 12,747,236 | 12,747,056 | 5,287,672 |
| Madness 4 release | 2,284,439 | 22,160,843 | 22,138,039 | 3,208,631 |
| Madness 4 debug | 2,357,211 | 23,370,291 | 23,343,427 | 3,338,907 |
| local-e | 2,268,755 | 20,509,895 | 20,495,982 | 4,401,292 |
| local-e-debug | 2,291,345 | 20,900,717 | 20,885,559 | 4,439,830 |

Typical inputs are short. These counts establish repeated work, not a memory-bandwidth bottleneck. Quoted-interior copies occur much less often: 182 to 53,074 calls per workload. Integer format recognition already uses pointer/length contexts internally, making it a suitable first leaf.

`tStrComp.Pos.Len` is diagnostic metadata and is not a universal storage bound: some transformations alter the string without updating it. New spans must use lengths known from their actual storage or bounded parents. Existing component tests deliberately cover this distinction. Recursive scratch ownership, mutable CPU callbacks, quote-qualification callbacks and symbol-name canonicalization are boundaries requiring explicit audit.

## Span foundation

`strref.h` defines the borrowed view and bounded helpers. Tests use nonterminated immutable arrays, empty NULL views, out-of-range slices and maximal counts. The `span-foundation-20261009` build passes all 329 tests on each of the three toolchains. Production parsing does not yet use the helpers; this is enabling infrastructure, with no speedup claimed.

The Windows GCC and MSVC assembler `.text` sections match the final accepted whitespace-baseline binaries byte for byte. Foundation commit: `682e1921`.

## Bounded integer leaf

`const_int_val_ref` accepts a pointer/length pair; ASL's existing format recognizers already operate on bounded contexts. The shared internal implementation remains visible to the optimizer and to a unit test using nonterminated arrays. Tests cover signs, C/Intel/Motorola formats, explicitly enabled IBM character constants and controlled character-translation diagnostics. The existing assembler suites exercise the real translation implementation. Two initial unit-test setup mistakes (an uninitialized default radix and an omitted character-format enablement) were corrected; production regression tests passed throughout.

The leaf-only `span-integer-20261009-r3` artifact passes all 330 tests on all three toolchains. Its three-pair full-cohort screen preserves all object code and normalized listings. Geometric elapsed/CPU savings are −1.62%/−0.32% for Windows GCC, +2.00%/+0.89% for MSVC and −1.44%/−1.85% for Linux GCC. This screen does not establish a portable speed benefit, so the leaf is not committed as an optimization by itself.

The next integer experiment uses the bounded leaf to return common literals before allocating evaluator scratch space or copying input and backup strings. Local sign-only references and PC symbols retain precedence; overflowing constants and unsuccessful probes retain the legacy continuation. The complete change is compared against the original span foundation, rather than claiming the leaf-only changes as savings.

The initial `span-integer-fast-20261009` version passes all 330 tests on each toolchain and preserves all eleven workloads' code and normalized listings. Its three-pair full screen gives geometric elapsed/CPU savings of −0.92%/+0.16% on Windows GCC, +0.75%/+1.23% on MSVC and −0.76%/+1.52% on native GCC. The mixed result does not justify retaining this version. GCC inlines the integer leaf at two call sites; the evaluator's distance to the next symbol grows from 11,120 to 13,072 bytes, including alignment padding.

`span-integer-shared-20261009` routes borrowed and owned input through one probe, avoiding that duplication. All 330 tests pass on each toolchain; all workload code and normalized listings match. The evaluator's next-symbol distance decreases to 12,064 bytes. A three-pair full screen gives +0.66%/+0.47%, +0.54%/+1.28% and +1.24%/+2.09% elapsed/CPU savings respectively. These remain screening results, not an independently confirmed optimization. A further revision rejects obvious nonliteral starts before measuring and trimming a span, reducing preparatory work on symbol-heavy inputs.

## Integer stage retained

`span-integer-gate-20261009` performs that cheap first-character rejection at the existing terminated public interface. Only plausible literals construct and trim a borrowed span. Borrowed and materialized inputs share one integer probe; an unsuccessful early probe is reused after the owned adapter is prepared. Successful literals return before temporary-result initialization and scratch checkout. Bare local signs, PC symbols, overflowing values and nonstandard default radices retain the established paths. Integer format checks, translation, range handling and symbol flags retain their existing implementation.

All 330 tests pass on each toolchain. The three-pair full screen preserves every workload's code and normalized listings and gives +1.25%/+0.91%, +2.68%/+2.16% and +2.01%/+2.91% geometric elapsed/CPU savings for Windows GCC, Windows MSVC and native Linux GCC respectively. The final `span-integer-final-20261009` build includes all utilities and all 330 tests on each toolchain. Its assembler `.text` matches the screened candidate byte for byte on all three compilers, confirming that the diagnostic counters are absent and the comment cleanup has no executable effect. Native Linux's foundation `.text` also matches the original accepted baseline.

An independent seven-pair full-cohort confirmation against the foundation gives the following results. All code and normalized listings still match. Positive values indicate savings; these are geometric per-workload results, rather than a sum of all benchmark runtimes.

| Toolchain | Elapsed savings | CPU savings |
| --- | ---: | ---: |
| Windows GCC | 1.50% | 1.59% |
| Windows MSVC | 1.95% | 2.06% |
| Native Linux GCC | 1.30% | 1.38% |

These are modest, repeatable portable gains, not the larger individual elapsed outliers seen in screening. For example, MSVC scaler-v1's confirmation improves elapsed time by 4.95% in all seven pairs, whereas Windows GCC scaler-v2's earlier 7.21% elapsed screen becomes −0.23% in confirmation. The complete integer stage is retained on the independent full-cohort result and because it enables further bounded propagation; neither the leaf-only conversion nor the earlier duplicate-probe version is retained separately.

Retained integer implementation and tests: `68ca50da`. Confirmation's smallest per-workload CPU saving is 0.00% on both Windows compilers and −0.31% on native GCC. Windows process CPU quantization limits interpretation of tiny changes on the shortest workloads. The monitored Windows confirmation retains at least 9,869 MiB available memory; no memory thrashing is indicated by that record.

The diagnostic `span-integer-counts-20261009` runs all eleven workloads twice against the final mechanism. Both runs agree exactly and retain each normal object's hash. Total evaluation counts match the original inventory; borrowed successes account for 15.6–37.8% of calls. The diagnostic source is restored before the final normal build. These counts establish removed work and are not timing measurements.

| Workload | Borrowed integer evaluations | Input copy bytes avoided | Backup copy bytes avoided |
| --- | ---: | ---: | ---: |
| Sonic 1 | 262,903 | 467,442 | 465,674 |
| Sonic 2 | 677,384 | 1,111,618 | 1,103,714 |
| Sonic & Knuckles | 524,688 | 1,163,796 | 1,156,788 |
| Big Fishing Derby | 487,361 | 1,202,892 | 1,200,632 |
| Sonic Classic Heroes | 343,344 | 789,395 | 785,373 |
| ScalerMacros-v1.asm | 184,262 | 253,040 | 253,040 |
| ScalerMacros-v2.asm | 231,974 | 338,164 | 338,164 |
| Madness 4 release | 755,231 | 1,392,406 | 1,386,538 |
| Madness 4 debug | 771,023 | 1,410,066 | 1,402,898 |
| local-e | 858,575 | 1,981,576 | 1,978,672 |
| local-e-debug | 863,735 | 1,987,174 | 1,984,006 |

The recognized literals are especially short, so avoided call setup, initialization and scratch handling are relevant alongside copying. Most copied bytes belong to the remaining symbol and compound-expression paths. The standalone quoted-string decoding trial below was discarded; the subsequent recursive view propagation stage was retained with the toolchain and workload limits recorded below. Memory mapping remains a later experiment with the Windows/POSIX abstraction described above.

## Ownership and interface boundaries

| Consumer | Current copy or scan | Borrowing requirement |
| --- | --- | --- |
| Integer constants | Input strlen before a bounded format context | Pass the physical length; preserve format priority, sign flags, overflow and translation |
| Floating constants | Terminated libc conversion | Use a common stack termination buffer and a longer-input fallback; preserve syntax and range behavior |
| Quoted constants | Owned interior copy and destructive escape/interpolation splitting | Bounded source scanning and escapes; decoded output remains owned |
| Brace expressions in expanded symbol names | Copied expression before recursive evaluation | Parent storage remains alive while the child consumes a bounded slice |
| Expression evaluation | Owned input and backup, then recursive splitting | Retain terminated adapters for mutable callbacks and diagnostics; borrow the unmodified backup only with a stable owner |
| Symbol lookup | Copied canonical name with local/section processing and uppercase conversion | Separate transient lookup views from owned keys retained in the symbol table |
| Line arguments | Copies from the current source line | CPU decoders can mutate arguments; do not replace these components wholesale without auditing their consumers |
| Diagnostics | `WrStrErrorPos` passes component text to a C-string error interface | Materialize a terminated adapter when needed; diagnostic position metadata is not a storage bound |
| Source lines | stdio reads into mutable dynamic line storage | Preserve newline, continuation, text-mode and EOF behavior; persistent macro storage cannot outlive its owner |

The source has 52 `InternSymbol` assignment sites and eleven `DECLARE_AS_EVAL_CB` occurrences, rather than a single generic callback boundary. These counts are an inventory aid, not a count of unique runtime callbacks or a performance measurement. Input-file experiments must also distinguish physical source text from generated macro text; mapping source files does not eliminate copies of generated expansions.

## Quoted-string decoding experiment: discarded

The 2026-10-09 candidate `span-quoted-20261009-r4` decoded literal runs and ordinary escapes directly from an immutable view of the quoted interior, avoiding its unconditional allocation and copy. Decoded output remained owned. Interpolation lazily allocated a terminated copy of the remaining interior for the existing delimiter scanner, recursive evaluator and diagnostics; each expression retained its original source offset. This preserved the compatibility boundary rather than introducing a new quote scanner in the same experiment. Numeric escape parsing consumed a bounded view and preserved the existing hexadecimal, decimal and octal limits, including the accepted digitless `\x` escape.

All 332 trial tests passed on Windows GCC, Windows MSVC and native Linux GCC. The trial added tests for nonterminated escape input with additional digits immediately beyond its boundary, embedded NUL and high bytes, empty literals, both quote styles, compound strings, repeated interpolation, nested quotes and parentheses. The integration test's independently specified bytes also matched the pre-experiment Windows GCC assembler. Those experimental tests and the implementation are preserved in the external candidate archive; they were removed from the source tree when the experiment was discarded.

The first three-pair Windows screening completed all eleven workloads, with geometric mean elapsed-time/CPU reductions of +1.81%/+0.30% for GCC and -1.78%/-0.05% for MSVC. Linux screening failed before completing `local-e-debug`: the native benchmark filesystem became read-only while saving its warm-up log. Subsequent Ubuntu startup attempts failed with `Wsl/Service/CreateInstance/E_FAIL`, and Windows reported zero available bytes on C:. A seven-pair Windows confirmation was stopped after this host condition was discovered. The incomplete Linux and interrupted confirmation batches are excluded from the acceptance decision.

On 2026-10-10, after the user relocated Ubuntu and Docker storage to E:, Ubuntu mounted writable with 883 GiB available and C: had free space again. The tested source snapshot was checked against the workspace, and a native write probe passed. A fresh independent `recovery-confirmation-20261010` batch completed seven interleaved pairs plus one warm-up per binary on all eleven workloads, comparing the candidate with `span-integer-final-20261009` (production code `68ca50da`). Machine-code records and normalized listings matched the baseline for every workload and toolchain. Timings use normal optimized executables, without profiling instrumentation.

Positive reductions mean faster; negative reductions mean slower. Aggregates are geometric means across all eleven workloads.

| Toolchain | Elapsed-time reduction | Process CPU reduction |
| --- | ---: | ---: |
| Windows GCC | +0.75% | -0.06% |
| Windows MSVC | +2.39% | +0.74% |
| Native Linux GCC | -0.05% | -0.05% |

| Workload | Windows GCC CPU reduction | Windows MSVC CPU reduction | Native Linux GCC CPU reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | +0.00% | +0.00% | -4.06% |
| Sonic 2 | -1.51% | +0.00% | -2.70% |
| Sonic & Knuckles | +0.00% | +0.00% | +0.84% |
| Big Fishing Derby | +0.00% | +1.59% | +1.02% |
| Sonic Classic Heroes | +0.00% | +2.97% | +0.91% |
| ScalerMacros-v1.asm | +0.00% | +0.00% | -0.85% |
| ScalerMacros-v2.asm | +1.25% | +1.06% | -1.29% |
| Madness 4 release | -0.61% | +0.00% | +1.80% |
| Madness 4 debug | -0.59% | +1.11% | +2.26% |
| local-e | +0.75% | +1.38% | +1.97% |
| local-e-debug | +0.00% | +0.00% | -0.63% |

Minimum available Windows host memory was 14,616 MiB. Native memory-pressure counters remained zero, with no swap-ins or swap-outs during the batch. The native sum of workload median CPU times fell by only 0.37%, a differently weighted measure from the geometric mean above. Improvements near 2% on some workloads were accompanied by regressions elsewhere, leaving Windows GCC and Linux GCC effectively unchanged overall and MSVC only 0.74% better in CPU time. Windows elapsed-time gains were larger than the corresponding CPU gains. These results do not establish a clear overall benefit sufficient to retain the change, so the implementation was discarded and no optimization commit was made.

Complete candidate sources, experimental tests, frozen binaries, build/test logs, and results remain outside the source tree under `D:/tmp/asl-bench-20261006/output-experiments/span-quoted-20261009-r4`; native results are under the corresponding `output-experiments` directory in `/home/marzojr/asl-benchmark-20261005`. This rejects the standalone decoder change, rather than the broader borrowed-string plan: propagating views through the higher-frequency expression-copy paths remains a separate experiment.

## Recursive operator and function views: retained

This independent stage starts from report head `afa0296f`, whose production code is `68ca50da`. It retains the accepted integer optimization and does not include the discarded quoted-string decoder. The public expression interface still accepts terminated components; a private recursive interface takes an immutable positioned view with separately tracked physical bounds. Operator operands, parenthesized subexpressions, user-function arguments and definitions, and built-in function arguments propagate views without inserting terminators into their parent's storage. Every descriptor and its source remain live throughout the synchronous child evaluation.

Ordinary expressions borrow the original input as their backup instead of unconditionally copying it into a second scratch buffer. Expanded temporary labels still receive an owned backup, preserving their generated spelling before callbacks run. Noninteger parsing and mutable target/caller callbacks keep an owned terminated working buffer; symbol-related built-ins and diagnostics receive terminated adapters when required. Function names also retain a writable buffer for legacy lookup and uppercase conversion. This removes the common backup copy and repeated length discovery at recursive integer leaves; it does not remove all expression input copies or change the existing floating and quoted-string leaves.

The bounded comma scanner matches the unqualified `QuotPos` rules for quotes, escapes, parentheses and square brackets. Its unit test compares 66,430 generated short inputs against the existing quoted iterator and delimiter callback, with delimiter-like bytes immediately beyond the tested view. Positioned-view tests deliberately give diagnostic metadata a different length from the physical text. The integration test covers nested and five-argument user functions, quoted commas/brackets, precedence, parenthesized expressions, short-circuit suppression of undefined symbols, forward references and excessive built-in argument counts. The built-in argument guard now runs before storing a fourth operand, preserving error 1490 without accessing outside the three-element array.

The first by-value descriptor variant, `span-recursive-20261010-r2`, passed all 332 tests. Its three-pair CPU screening reductions were +1.95% Windows GCC, +1.97% MSVC and -0.18% native Linux GCC, with substantial variation in early Linux cases. A portable follow-up passed the positioned descriptor by pointer at recursive call boundaries. `span-recursive-20261010-r3` passed all 332 tests and screened at +1.59%/+1.16%/+1.82% CPU reductions for those three toolchains. These are separate baseline comparisons, not a direct causal measurement of descriptor calling overhead.

The final snapshot `span-recursive-final-20261010` builds all utilities and passes all 332 tests on each toolchain. Its normal executable `.text` sections match the screened pointer version on all three toolchains. The new integration fixture also passes with the baseline Windows GCC assembler. A separate negative-input probe produces exactly matching baseline/candidate diagnostics and exit status for operator type errors, function type errors, unknown functions, excessive function arguments and mismatched parentheses.

Six focused native tests passed with AddressSanitizer and recoverable UndefinedBehaviorSanitizer checks, with leak detection disabled. No address errors were reported. This is not a clean UBSan result: the checker reports null-pointer field-address expressions in unchanged `asmmac.c:355` and `asmstructs.c:494`, and a null argument to a zero-length copy in unchanged `nonzstring.c:187`. These remain separate correctness follow-ups. The sanitizer build is never used for performance measurements.

### Independent seven-pair confirmation

The `confirmation` batch compares the final snapshot with `span-integer-final-20261009`, using normal optimized Windows GCC/MSVC and native Linux GCC binaries. All eleven workloads complete seven interleaved pairs after one warm-up per binary. Machine-code records and normalized listings match the baseline for every workload and toolchain. Native input, binaries and outputs remain on native ext4; all build and benchmark directories remain outside the source tree. Minimum available Windows host memory was 12,134 MiB; native swap-in/out deltas and memory-pressure counters were zero.

Positive reductions mean faster, negative reductions mean slower. Aggregates are geometric means across the eleven workloads.

| Toolchain | Elapsed-time reduction | Process CPU reduction |
| --- | ---: | ---: |
| Windows GCC | +2.38% | +0.68% |
| Windows MSVC | +2.36% | +2.52% |
| Native Linux GCC | +0.62% | +0.57% |

| Workload | Windows GCC CPU reduction | Windows MSVC CPU reduction | Native Linux GCC CPU reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | +0.00% | +2.17% | -4.50% |
| Sonic 2 | +1.56% | +5.56% | +2.83% |
| Sonic & Knuckles | -1.30% | +2.30% | +1.92% |
| Big Fishing Derby | +0.00% | +3.28% | -0.54% |
| Sonic Classic Heroes | -1.12% | +2.08% | +0.77% |
| ScalerMacros-v1.asm | +0.00% | +3.45% | +1.47% |
| ScalerMacros-v2.asm | -2.60% | +1.11% | +0.57% |
| Madness 4 release | +2.52% | +1.21% | +0.45% |
| Madness 4 debug | +2.98% | +2.26% | +0.13% |
| local-e | +3.76% | +1.45% | +1.58% |
| local-e-debug | +1.53% | +2.80% | +1.39% |

Retain the portable pointer version: MSVC improves on every workload and confirms +2.52% mean CPU savings; Windows GCC and Linux GCC have smaller positive aggregates. Gains are not universal. Linux Sonic 1 is 4.50% slower in CPU time in confirmation, and Windows GCC has several small regressions. Windows process CPU times have 15.625 ms granularity, so small individual differences should not be treated as precise speed estimates. The report records these limits rather than claiming a uniform improvement or importing the larger screening percentages into the confirmed result.

Complete source snapshots, normal and sanitizer binaries, test logs, diagnostic probes and raw timing results remain under the external `output-experiments` roots. The retained artifacts are `span-recursive-final-20261010` and the screened pointer version `span-recursive-20261010-r3`; the by-value trial is preserved separately. No private workload source, original source paths or manifests are committed. Profiles and future counted floating/quoted leaves can evaluate the remaining materialization boundaries independently.

## Linux Sonic 1 regression investigation: no retained code change

The reported 4.50% slowdown compares `span-recursive-final-20261010` (production `8d2493e4`) with `span-integer-final-20261009` (production `68ca50da`). Fresh measurements reuse those frozen optimized binaries, with no rebuilding or instrumentation during timing. All runs use native ext4 and alternate binary order; objects and normalized listings match. Positive reductions mean faster.

| Linux Sonic 1 comparison | Measured pairs | Warm-ups per binary | Elapsed reduction | CPU reduction |
| --- | ---: | ---: | ---: | ---: |
| Original confirmation | 7 | 1 | -4.47% | -4.51% |
| Fresh focused repeat | 21 | 1 | +0.18% | +0.92% |
| Second focused repeat | 21 | 1 | +1.35% | +1.36% |
| Fresh eleven-workload sweep | 7 | 1 | -6.07% | -6.12% |
| Focused repeat on virtual CPU 2 | 63 | 10 | +1.95% | +1.84% |

The short seven-pair samples remain unstable, whereas the longer focused checks do not reproduce a persistent regression. In original confirmation, baseline CPU times range from 0.285 to 0.560 seconds and candidate times from 0.275 to 0.405 seconds. That sample's ratio of medians suggests a slowdown, but its geometric mean of paired CPU ratios suggests a 4.86% improvement. An exploratory paired bootstrap spans -4.82% to +14.69%, including no change. This is evidence of inadequate precision, not a reason to replace the preselected median statistic with a more favorable statistic.

With ten warm-ups and both binaries restricted to the same virtual CPU, median CPU time falls from 0.280405 to 0.275248 seconds. The geometric paired CPU reduction is +2.52%, with an exploratory bootstrap interval of +1.39% to +3.81%. These intervals resample pairs and do not account for temporal correlation. Guest CPU affinity also does not control the host's scheduling or clock frequency. The measurements do not establish the exact source of the earlier variation. Memory-pressure counters remain zero and there are no swap-ins or swap-outs in either pinned comparison.

For completeness, the fresh unrestricted seven-pair sweep has -0.60% mean CPU reduction across the eleven workloads, compared with +0.57% in original confirmation. The small Linux aggregate is therefore also sensitive to measurement variation.

| Workload | Fresh native Linux GCC CPU reduction |
| --- | ---: |
| Sonic 1 | -6.12% |
| Sonic 2 | -1.44% |
| Sonic & Knuckles | +0.62% |
| Big Fishing Derby | +0.34% |
| Sonic Classic Heroes | -2.85% |
| ScalerMacros-v1.asm | +1.53% |
| ScalerMacros-v2.asm | +0.28% |
| Madness 4 release | +0.49% |
| Madness 4 debug | -0.70% |
| local-e | +0.04% |
| local-e-debug | +0.92% |

### Public evaluator wrapper experiment: discarded

Disassembly reveals an additional wrapper cost in the retained implementation: the public entry point constructs a positioned view on its stack, calls the private evaluator, and returns through a stack-canary check. This is a concrete overhead, but its contribution to the reported regression was not established. One portable experiment passes the text, physical length and caller's live position directly to the core evaluator, with an inline adapter for recursive views. It leaves parsing and ownership rules unchanged. Linux GCC emits a tail jump from the public wrapper and removes its temporary view and canary check; no platform guards or compiler attributes are introduced.

The `span-recursive-tail-20261010` snapshot builds all utilities and passes all 332 tests on Windows GCC, Windows MSVC and native Linux GCC. Three-pair screening against the retained recursive implementation preserves objects and normalized listings on all eleven workloads.

| Toolchain | Elapsed reduction | CPU reduction |
| --- | ---: | ---: |
| Windows GCC | +1.92% | +1.19% |
| Windows MSVC | -0.88% | -1.66% |
| Native Linux GCC | +0.69% | +0.83% |

A separate 63-pair, ten-warm-up comparison on virtual CPU 2 measures +1.29% elapsed and +1.16% CPU reduction for the wrapper experiment on Sonic 1. The extra overhead can be reduced, but the experiment fails the requirement to preserve the other workloads' gains: MSVC screening loses performance overall. Discard the candidate and retain `8d2493e4` unchanged. At the user's request, stop investigating the vanished regression; do not chase short-sample fluctuations or add further speculative changes.

Timing results are preserved under the native `span-recursive-final-20261010` artifact in batches `regression-s1-20261010`, `regression-s1-repeat-20261010`, `regression-all-20261010` and `regression-s1-pinned-20261010`. The discarded candidate's sources, patch, binaries, test logs and screening summaries remain in `span-recursive-tail-20261010` under the external Windows and native artifact roots; its focused repeat is also named `regression-s1-pinned-20261010`. No private source or identifying information is added to the report.

## Floating conversion follow-up

A stack termination buffer can cover common floating literals, with reusable dynamic storage for longer inputs. Numeric precision does not bound source length: leading zeros, long exponents and rounding-sensitive tails must not be silently truncated. Preserve ASL's locale, range and accepted-syntax behavior.

Adapting a counted converter remains a later independent experiment. The [libstdc++ floating `from_chars` implementation](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/floating_from_chars.cc) uses `fast_float` for supported IEEE float/double cases, while wider `long double` configurations can still use `strtold`. Its exact source dependencies and licensing notices must be reviewed if code is imported; no external conversion code has been imported in this stage.
