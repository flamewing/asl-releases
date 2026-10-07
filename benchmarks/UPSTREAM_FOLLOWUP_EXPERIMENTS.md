# Further profiling and optimization experiments

This round begins at experimental HEAD `66d5e77`. Fix SysString first, refresh profiles at that committed fix, then test temporary-label rejection, expression-specific operator scanning and the disabled-listing path independently, against the latest accepted baseline. All eleven prepared workloads include the corrected anonymous local-e release/debug inputs. Keep builds, prepared inputs, profile collections and raw results outside the checkout under D:/tmp/asl-bench-20261006 and /home/marzojr/asl-benchmark-20261005. Markdown paragraphs are not wrapped.

## SysString correctness fix

An original-code reproduction stalls on an 8-byte buffer converting 0x123456 in radix 16 with minimum width 100 and an underscore separator. Padding is clamped to digit capacity, but separators consume additional bytes. When the buffer fills with padding still outstanding and Num zero, the split loop can make no further progress. Stop conversion whenever digit storage is exhausted, retaining the stored suffix, count and terminating NUL. Reserve a sign byte before number conversion to prevent writes before tiny or truncated signed buffers, and compute negative magnitude with unsigned subtraction to avoid signed overflow for the minimum LargeInt.

Permanent formatter regressions cover separator/padding exhaustion, zero- and one-byte buffers, signed truncation, leading-zero/sign interaction, minimum signed value, and the public formatting path. The formatter test has a 15-second timeout to catch stalled regressions. All 320 tests pass on Windows GCC, Windows MSVC and native Linux GCC. A differential stress harness compares bounded conversion with suffixes of the original untruncated converter across 433,440 cases on Windows GCC, MSVC and sanitized Linux: capacities 0 through 128, radices 2 through 36, widths through 200, signed/unsigned values, padding, digit case, separators, output counts, termination and boundary guards. The original stall is reproduced with a two-second subprocess timeout. Initial fixture expectations for zero padding and the formatter's conversion capacity were corrected before the final full suites; production fix unchanged. External artifacts: upstream-sysstring-fix-v2-20261007 and upstream-sysstring-checks-20261007; the first fixture-repair build is retained separately as upstream-sysstring-fix-20261007.

All eleven workloads produce deterministic unchanged code and normalized listings against `66d5e77` on all three toolchains. The single-pair output-check batch establishes normal object references for profiling and is not used to claim performance changes. Sanitized permanent formatter regressions also pass.

## Ordered candidates

1. Reject ordinary inputs before temporary-label length/tail scans in chk_tmp_2. Preserve whitespace, temporary-label definition/reference rules, scope state and diagnostics.
2. Specialize expression operator scanning so matching can be inlined and per-character indirect callbacks reduced. Preserve quote qualification, escapes, brackets, candidate order, custom operators and temporary-label rules.
3. Reject disabled listings before granularity and body preparation in MakeList, preserving per-line state and listing modes.

Refresh diagnostic GCC profiles on both operating systems and sample optimized Windows GCC/MSVC copies before experiments. Do not use diagnostic binaries for timings or interpret instrumentation overhead as production cost. Screen all eleven variants with three alternating pairs, independently confirm useful gains, compare deterministic generated code and normalized listings, and remove discarded production changes. Native queued writes are flushed before each assembly outside elapsed/child-CPU measurement. Timing batches never overlap builds, profiling or other timing batches. Report compiler-specific gains and elapsed/CPU differences explicitly; do not add individual percentage reductions into an aggregate speedup.

## Refreshed profiles at `420f9b0`

Freeze committed source and collect three diagnostic GCC assemblies per workload on each OS, followed sequentially by three optimized sampled assemblies per workload for Windows GCC and MSVC. Every profiling output matches its corresponding normal fixed binary. Sampling resolves physical assembler bodies and imported function stubs; both traces have zero unresolved assembler-body samples, with 54,135 GCC and 55,721 MSVC target samples. MSVC sampling uses a separately built optimized symbol copy; timing continues to use the normal binaries. General runtime-private samples retain module attribution rather than guessed allocation/copy/locking labels.

| Resolved function body/category | Windows GCC sampled CPU share | Windows MSVC sampled CPU share |
| --- | ---: | ---: |
| Kernel execution | 35.01% | 32.91% |
| strlen | 5.64% | 5.47% |
| EvalStrExpressionWithCallback | 3.50% | 2.47% |
| operator_search_cb | 2.09% | 1.95% |
| as_iterate_str_quoted | 1.96% | 2.19% |
| isspace | 2.58% | 2.11% |
| vsprcatf_core | 1.44% | 2.83% |
| MakeList | 1.06% | 0.87% |

Shares pool all eleven variants by sampled CPU time and are neither elapsed fractions nor predicted speedups. Imported stub costs are separate from the resolved function-body rows. Different total costs, compiler inlining and sequential sampling prevent reading share changes as measured before/after gains.

Native diagnostic counts support the ordered candidates: Sonic 2 makes 2,176,682 chk_tmp_2 calls, 20,263,602 operator-search callbacks and 720,970 MakeList calls per assembly. Scaler v2 makes 1,704,558 chk_tmp_2 calls, 12,243,492 callbacks and 4,019,338 MakeList calls. Madness 4 release makes 2,392,033 chk_tmp_2 calls and 4,374,638 formatter calls. Counts are instrumented out-of-line calls, not production CPU costs. External artifacts: upstream-followup-head-20261007/{source,metadata.json,gprof,gprof-summary.json,sampling/analysis.json}, source-frozen builds, exact executables, collection logs and normal output references.

## Experiment results

Results are recorded as each candidate is accepted or discarded; positive reductions mean faster.

### 1. Temporary-label prefix rejection: discarded

The candidate rejects ordinary inputs after leading-whitespace handling, before strlen and trailing-whitespace scans in chk_tmp_2. Slash is rejected on reference-only calls because it is a temporary symbol only in definition contexts. All 320 tests pass on every toolchain; 450,000 extracted-source comparisons per toolchain agree on output, temporary counters/log state, return values and diagnostics across generated and boundary inputs, definition/reference modes, aliasing and truncation. Linux comparisons run with AddressSanitizer/UndefinedBehaviorSanitizer. The stress fixture respects the existing formatter rejection of pointer-sized capacities. All eleven workload code/listing outputs match baseline `420f9b0`.

The three-pair full-suite geometric wall/CPU reductions are Windows GCC 1.50%/0.16%, MSVC 0.01%/-0.68%, Linux 1.70%/0.72%. Native elapsed improvements are not consistently corroborated by CPU: Madness 4 release improves 10.05% elapsed but only 1.50% CPU, while Sonic 2 and Big's Fishing Derby regress in CPU. The aggregate saved work is too small and mixed to justify the guard; it is removed without a performance commit. External artifacts: upstream-temp-guard-20261007/{screen-sync,candidate-sources,candidate.patch}, builds/tests and upstream-temp-guard-20261007-checks.

### 2. Specialized expression operator scanner: accepted for GCC

A direct scanner keeps quote qualification and escaping in a shared private implementation, avoids the generic per-character indirect operator callback and skips ordinary bytes using the existing first-byte operator inventory. Parentheses and braces retain explicit structural markers, including custom inventories with all 32 entries occupied. Generic iterator APIs and callback-before/after behavior remain unchanged. The initial specialization was insufficient on Windows; refining it with the ordinary-byte gate produced useful GCC savings but mixed MSVC results. Retain the generic MSVC scanner, its original quote helper declaration and source order. The final MSVC .text matches the SysString baseline byte for byte; only .rdata differs. Final Windows GCC executable sections and native GCC .text match the measured refined candidate. These checks are recorded in upstream-expression-scan-gcc-core-20261007/code-equivalence.json.

All 320 tests pass on each toolchain for every build, including the final compiler-scoped implementation. Differential checks compare 600,000 operator cases and 480,000 generic iterator cases per toolchain against the original implementation: built-in and custom inventories, duplicate/overriding operators, full-capacity structural/quote operators, high bytes, quote qualification, escaping, parentheses, temporary labels, operator ordering/position, callback-before/after and extra skips/early termination. Linux checks use AddressSanitizer/UndefinedBehaviorSanitizer. All eleven refined-screen outputs preserve code and normalized listings. Final GCC code equivalence preserves those measured implementations; MSVC returns to baseline instructions.

| Cohort | Windows GCC wall / CPU reduction | Windows MSVC wall / CPU reduction | Native GCC wall / CPU reduction |
| --- | ---: | ---: | ---: |
| Initial direct scanner, eleven variants, three pairs | 1.07% / 1.18% | -1.76% / -0.96% | 1.97% / 2.78% |
| Refined scanner, eleven variants, three pairs | 2.33% / 1.91% | -0.01% / 0.15% | 3.45% / 3.78% |
| GCC-scoped confirmation, seven variants, five pairs | 1.09% / 1.61% | 0.05% / -0.36% | 2.53% / 2.38% |

The seven-case confirmation covers Sonic 2, Sonic & Knuckles, Big's Fishing Derby, Sonic Classic Heroes, scaler v2, Madness 4 release and local-e debug. Its MSVC column measures an intermediate shared-header layout and is not credited as an improvement; the final private-header layout restores baseline instructions. Six of seven Windows GCC cases improve elapsed time, with Sonic & Knuckles -0.70% elapsed / 0.00% CPU. Six of seven native cases improve elapsed and CPU; Sonic Classic Heroes conflicts with the positive full-screen result and receives a separate seven-pair rerun alongside Big's Fishing Derby. Percentages are geometric means of per-workload median ratios within each stated cohort, not directly comparable aggregate gains across different cohorts. External artifacts: upstream-expression-scan-20261007, upstream-expression-scan-v2-20261007/{screen-sync,candidate.patch}, upstream-expression-scan-gcc-v2-20261007/confirm-sync, upstream-expression-scan-gcc-core-20261007 and the corresponding differential-check collections.

The independent native rerun confirms Sonic Classic Heroes at 2.79% elapsed / 2.28% CPU reduction (six of seven elapsed pairs faster), with Big's Fishing Derby at 5.30% / 5.33% (seven of seven). Retain the GCC scanner based on repeated saved CPU and elapsed work across the cohorts; retain baseline MSVC instructions. External rerun: upstream-expression-scan-gcc-core-20261007/heroes-recheck-sync. The interim declaration-order build is retained as upstream-expression-scan-gcc-final-20261007; its MSVC text was not baseline-equivalent and is superseded by the private-header build.
