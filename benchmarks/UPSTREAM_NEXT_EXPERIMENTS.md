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
