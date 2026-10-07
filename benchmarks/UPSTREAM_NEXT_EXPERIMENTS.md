# Further upstream optimization experiments

This round starts at experimental HEAD `7399003`, including compatibility fixes, operator-discovery filtering and known-span line copying. All eleven prepared workload variants include the corrected anonymous local-e release/debug inputs. Private sources/adaptations and raw artifacts stay external under D:/tmp/asl-bench-20261006 or /home/marzojr/asl-benchmark-20261005; no build directories are created in the source checkout. Markdown paragraphs are not wrapped.

## Fresh baseline profiles

Freeze the exact HEAD source and collect three instrumented GCC assemblies per variant on Windows and native Linux, followed by three sampled optimized assemblies per variant with Windows GCC and MSVC. Every collection matches normal baseline objects. The MSVC sampling binary is a separately built optimized copy with /Zi and /DEBUG:FULL /OPT:REF /OPT:ICF /INCREMENTAL:NO for symbols, retaining /O2 /Ob2 /DNDEBUG; it is not substituted for the established timing binary. GCC diagnostic builds add -pg and frame pointers and are never used for performance comparisons. CPU sampling resolves physical function bodies through COFF/PDB and import stubs, retains kernel/runtime categories, and has zero unresolved assembler-body samples: 59,456 target samples for GCC and 62,380 for MSVC. Percentages below pool all eleven variants by sampled CPU time and are not wall-time shares or predicted optimization gains.

| Function/category | Windows GCC | Windows MSVC |
| --- | ---: | ---: |
| Kernel execution | 33.96% | 31.74% |
| strlen | 5.40% | 5.39% |
| EvalStrExpressionWithCallback | 5.38% | 4.42% |
| operator_search_cb | 2.24% | 2.09% |
| as_iterate_str_quoted | 1.97% | 2.28% |
| isspace | 2.52% | 2.11% |
| vsprcatf_core | 1.69% | 2.56% |

Native instrumented counts still show 20,263,602 operator-search callbacks per Sonic 2 assembly, and 20,153,482 StrCompReset / 26,235,222 LineCompReset calls for scaler v2. Madness 4 release makes 4,952,027 formatter calls; local-e-debug makes 6,478,755. These are instrumented out-of-line call counts, not production CPU costs. Different compiler inlining and unresolved runtime work prevent treating every named function boundary as directly comparable. The corrected local-e variants enlarge the cohort relative to the older upstream profiles, so pooled shares are not interpreted as before/after savings. External profile artifacts: output-experiments/upstream-next-head-20261007, including exact source, metadata, GCC gprof collections/summaries, optimized sampling sessions and resolved analysis.

## Ordered experiments and method

1. First-byte dispatch for the per-expression filtered operator list, preserving candidate order and target/callback operators.
2. Reuse operand-presence checks within operator matching, independently of dispatch.
3. Inspect optimized code and inline small component-reset operations if calls remain.
4. Specialize listing byte/word number formatting while preserving radix, padding, separators and unknown-value masks.

Each experiment starts from the latest accepted baseline, with no discarded candidate left in production. Build and run the full suite on Windows GCC, Windows MSVC and native Linux GCC before timing. Screen all eleven variants with one warmup and three alternating baseline/candidate pairs; promising gains require an independent confirmation. OS timing batches are sequential and never overlap builds or profiles. Native runs flush queued writes before each assembly outside elapsed/child-CPU measurement, retaining input caches. Require deterministic identical generated code across baseline/candidate/toolchains and normalized listing equality within each compiler. Preserve custom-prefix, continuation, invalid-prefix and disabled-listing behavior. Report elapsed and CPU reductions separately, label differing cohorts, and investigate apparent regressions rather than attributing every noisy elapsed change to the candidate.

## Results

Results are recorded as each candidate is accepted or discarded. Positive reductions mean faster; negative reductions mean slower.

### 1. First-byte operator dispatch: accepted after refinement

The initial candidate grouped the filtered inventory into per-expression first-byte chains. It retained original candidate order and checked only that byte's chain, skipping redundant single-byte strncmp calls. All 320 tests passed on each toolchain and all eleven code/listing outputs matched. The three-pair screen gave geometric wall/CPU reductions of -1.02%/-0.48% for Windows GCC, -0.52%/-1.18% for MSVC and -1.50%/-1.44% for Linux; this version was rejected because setup outweighed matching savings.

The refinement prepares the immutable built-in chains once and uses them directly when target and callback operator tables are both empty. It avoids the per-expression presence/filter/setup pass in that common path. If custom operators exist, the accepted defensive presence filter and per-expression chain construction remain, preserving fresh table contents and lifetimes. Longest matches, operator override order, quote/bracket state, temporary-label treatment, precedence, associativity and short-circuit evaluation remain unchanged. Chain indices use zero as terminator and slots 1 through OPERATOR_MAXCNT, preserving all 32 usable entries. No custom-operator inventory is cached.

All 320 tests passed on Windows GCC, Windows MSVC and native Linux GCC. An extracted-source differential harness compared the candidate with `7399003` on 600,000 generated cases per toolchain, covering the immutable-built-in and custom paths, empty/full-capacity inventories, overlapping and duplicate spellings, alphabetic/high-byte operators, three apostrophe-qualification modes, quote/bracket state, operator position and selection, and temporary-label operand boundaries. Windows GCC/MSVC and Linux with AddressSanitizer/UndefinedBehaviorSanitizer all agree. Every measured workload produces deterministic identical code and normalized listings within each compiler.

| Batch | Variants | Windows GCC wall / CPU reduction | Windows MSVC wall / CPU reduction | Linux GCC wall / CPU reduction |
| --- | ---: | ---: | ---: | ---: |
| Screen, three alternating pairs | 11 | 3.08% / 3.84% | 3.27% / 3.28% | 5.02% / 5.47% |
| Independent confirmation, five alternating pairs | 5 | 4.28% / 4.30% | 3.73% / 3.38% | 4.65% / 4.93% |

The cohorts differ: confirmation covers Sonic 2, Sonic & Knuckles, scaler v2, Madness 4 release and local-e-debug. Every confirmation median improves across toolchains. Sonic 2 confirms 5.69%/5.71%/9.77% elapsed gains for Windows GCC/MSVC/Linux; scaler v2 confirms 3.30%/4.05%/2.71%. Native listing elapsed gains are interpreted alongside CPU results: Madness 4 release confirms 1.46% wall/3.27% CPU and local-e-debug 5.07% wall/4.36% CPU. No repeated material regression was observed. The refinement is accepted. External artifacts: upstream-op-chains-v2-20261007/{screen-sync,confirm-sync,candidate-sources,candidate.patch}, builds/tests and upstream-op-chains-v2-20261007-checks; the rejected initial version remains separately preserved under upstream-op-chains-20261007 and its checks directory.

### 2. Operand-presence reuse: discarded

The candidate lazily found the expression's final non-whitespace boundary when an operator first matched and reused it for subsequent right-operand tests. It also reused the left-operand presence result among matching candidates at a position. Temporary-label qualification retained its existing order, and expressions without matching operators incurred no additional boundary scan. All 320 tests passed on all three toolchains; the 600,000-case extracted differential checks agreed on Windows GCC/MSVC and sanitized Linux. All eleven code/listing outputs remained deterministic and identical to committed dispatch baseline `4e59163`.

The three-pair full-suite screen gave geometric wall/CPU reductions of -0.20%/-0.04% for Windows GCC, 0.74%/0.83% for MSVC and 0.89%/-0.05% for Linux. Native elapsed benefits in listing cases did not establish CPU savings. The aggregate CPU changes are negligible or modest, and the candidate adds bookkeeping around checks that usually stop quickly; it was removed without a performance commit. External artifacts: upstream-op-presence-20261007/{screen-sync,candidate-sources,candidate.patch}, builds/tests and upstream-op-presence-20261007-checks.

### 3. Inline component resets: accepted for MSVC only

The first candidate moved the small LineCompReset and StrCompReset store sequences into header helpers while retaining the public functions and their addressability. A refinement also used the helpers inside component allocation, release and temporary construction. All 320 tests passed on every toolchain for both versions, and the full eleven-workload screen preserved generated code and normalized listings.

The initial three-pair screen gave geometric wall/CPU reductions of 0.15%/0.32% for Windows GCC, 2.10%/1.80% for MSVC and 1.85%/1.88% for Linux. An independent five-pair confirmation of the refined version covered Sonic 1, Sonic 2, Big's Fishing Derby, both scalers, Madness 4 release and local-e-debug: Windows GCC -0.66%/-0.07%, MSVC 2.49%/2.14%, Linux 0.79%/1.19%. A separate native full-suite screen varied more substantially, so these measurements do not establish a consistent GCC benefit.

MSVC confirmed scaler v1/v2 reductions of 5.24%/3.33% elapsed and 5.88%/3.70% CPU, with every elapsed pair faster. Madness 4 release confirmed 0.77% elapsed/2.12% CPU; local-e-debug confirmed 5.11% elapsed/3.14% CPU. The final implementation therefore enables caller inlining only for MSVC (excluding clang-cl), preserving the existing GCC call sites through aliases. Public entry points remain available and component ownership/capacity are preserved.

All 320 tests pass again on each toolchain for the final version. Its MSVC executable text is byte-identical to the independently confirmed refinement; the only differing section bytes are debug-directory timestamps, allowing those measurements to apply directly. Windows GCC executable sections are byte-identical to the accepted dispatch baseline, and native GCC text is byte-identical to that baseline. No GCC performance gain is claimed. External artifacts: upstream-inline-resets-20261007/screen-sync, upstream-inline-resets-v2-20261007/{screen-sync,confirm-sync}, and upstream-inline-resets-msvc-20261007 with final builds/tests, candidate sources and windows-code-equivalence.json.

### 4. Listing code-value conversion: accepted for GCC after refinement

The candidate bypasses general format-string parsing for the listing's emitted byte/word values and guessed-value masks, calling the same SysString conversion directly. Radix, minimum zero-padded width, digit case, byte separators and mask semantics remain shared with the general formatter. Destination append behavior and listing prefixes, source padding, continuation selection and output writing are preserved; no current-line cursor experiment is bundled into this change. The numeric conversion retains the formatter's 100-byte conversion capacity, with one additional byte for the leading space.

All 320 tests passed on Windows GCC, MSVC and native Linux for each version. An extracted-source harness compared the numeric append and mask conversion with the existing formatter in 137,760 cases per toolchain: radices 2 through 36, widths 0 through 40, uppercase/lowercase digits, no separator or underscore/dot/space separators, zero and boundary values through the full LargeWord range, dynamic-string growth, return counts, termination and buffer guards. Windows GCC/MSVC and Linux with AddressSanitizer/UndefinedBehaviorSanitizer agree. Listing fixtures compare byte/word/long data, continuation and reservation lines, six radices, separators and unresolved-symbol diagnostics. Additional fixtures retain custom/repeated prefixes, malformed-prefix errors and disabled-listing behavior. All eleven benchmark code/listing outputs remain identical and deterministic.

The initial stack-buffer version passed the full three-pair screen and an independent five-pair confirmation over four listing workloads and three controls. Listing-only geometric wall/CPU reductions were Windows GCC 1.13%/1.05%, MSVC 1.05%/1.73%, Linux 1.42%/1.66% in the screen, and 1.85%/1.82%, 0.74%/0.94%, 0.85%/2.34% in confirmation. However, scaler v2 confirmed CPU regressions of 1.08% on Windows GCC and 1.96% on MSVC. Generated Windows GCC MakeList code showed that helper inlining enlarged its stack frame from 0x158 to 0x168 bytes, including the listing-disabled path. This is an observed code change, not proof that stack size caused the regression.

The refinement reuses a small buffer owned by the already shared, non-reentrant listing state, restoring that GCC stack-frame size. Its full-suite screen gave Windows GCC 1.57% wall/0.96% CPU, MSVC -0.58%/-0.23%, Linux 0.35%/0.44%; the four listing cases gave 1.49%/1.76%, -0.41%/0.27%, 1.03%/1.50%. MSVC did not establish a consistent benefit, so the final implementation retains its original formatter calls through private macros. Windows GCC executable sections are byte-identical to the tested refinement, native GCC text is byte-identical to the refinement, and MSVC text is byte-identical to accepted baseline `594a455`.

The final GCC-only conversion was independently confirmed with five alternating pairs over Sonic 2, Sonic Classic Heroes, both scalers and all four listing variants. All four Windows listing cases improved in every elapsed pair; neither scaler confirmed a CPU regression. Native confirmation CPU medians improve for every workload. Windows control CPU medians are Sonic 2 -1.32%, Sonic Classic Heroes 0.95%, scaler v1 0.00% and scaler v2 0.00%. Sonic 2 elapsed improves 1.28%; its CPU direction varied across earlier batches, so no control-workload gain is claimed.

| Final confirmation cohort | Windows GCC wall / CPU reduction | Linux GCC wall / CPU reduction |
| --- | ---: | ---: |
| All eight variants | 1.17% / 1.26% | 0.63% / 1.86% |
| Four listing variants | 2.09% / 2.60% | -0.11% / 2.38% |

Native local-e-debug initially showed a 3.75% elapsed regression despite 4.13% less CPU and CPU wins in all five pairs. One candidate run took 4.02 seconds but used only 1.33 seconds of child CPU; recorded block operations and major faults were zero. These observations show unaccounted waiting, without identifying its cause. An isolated seven-pair rerun gave 2.54% elapsed/2.48% CPU reductions, six elapsed wins and seven CPU wins. The table retains the original confirmation, including its elapsed anomaly; the rerun is reported separately rather than substituted into the aggregate. Windows listing confirmation elapsed gains are Madness 4 release 1.90%, debug 1.55%, local-e release 2.08%, debug 2.83%. MSVC has no claimed gain from this experiment.

The final source passes all 320 tests per compiler again, together with final custom/numeric listing checks. External artifacts: upstream-list-values-20261007/{screen-sync,confirm-sync}, upstream-list-values-v2-20261007/screen-sync, upstream-list-values-gcc-20261007/{confirm-sync,latency-confirm-sync,candidate-sources,candidate.patch,windows-code-equivalence.json,linux-code-equivalence.json}, associated builds/tests, listing validation and the extracted-source checks directories. All builds, inputs, outputs and timing artifacts remain outside the checkout.

A separate excessive-padding stress probe stalled in the existing SysString split-byte conversion before candidate comparison, outside the listing's normal width range. The completed differential checks cover widths 0 through 40, including listing widths. General formatter behavior with separator-induced buffer exhaustion remains a deferred robustness investigation; this optimization does not change that converter.

All four ordered experiments are complete. First-byte dispatch, MSVC component resets and GCC listing conversion are retained; operand-presence reuse is discarded. Reported reductions are against each experiment's latest accepted baseline and must not be added to estimate an aggregate speedup.
