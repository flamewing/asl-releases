# Independent optimization trials following the 2026-10-08 profiles

The candidates come from [MASTER_EXPERIMENTAL_PROFILES_20261008.md](MASTER_EXPERIMENTAL_PROFILES_20261008.md). The starting accepted revision is `1c9514a1`; its assembler code is identical to the freshly profiled `9abb1335`. Each trial changes one mechanism, passes full Windows GCC/MSVC and native Linux GCC suites, and is timed against its immediate accepted baseline. Promising results receive a separate confirmation batch. Rejected changes are removed before proceeding. Production changes use shared portable code.

Artifacts stay outside the checkout under `D:/tmp/asl-bench-20261006/output-experiments/` and native `/home/marzojr/asl-benchmark-20261005/output-experiments/`. Normal release flags match the profile refresh: GCC `-O3 -DNDEBUG -g`, MSVC `/O2 /Ob2 /DNDEBUG /Zi` with optimized nonincremental debug linking; no LTO or instrumentation. Windows GCC/MSVC run interleaved, Linux runs separately on ext4 with synchronization outside measured time. Builds/tests and timing batches do not overlap. Reports show positive savings, while negative percentages indicate a slowdown. local-e variants remain anonymous. Paragraphs are unwrapped.

## 1. Opening-quote guard before string-probe strlen — discarded

A three-line guard rejected expressions beginning with neither quote before ConstStringVal measured their length. Quoted strings followed the existing parser. All 327 tests passed on each toolchain, and all eleven workloads retained identical code and normalized listings against the parent.

The five-pair full screen gave geometric elapsed/CPU savings of 0.14%/0.04% on Windows GCC, 0.59%/0.47% on MSVC and 0.43%/0.64% on native GCC. The apparent MSVC scaler-v2 gain was 4.87% (5/5 pairs), but a seven-pair confirmation on Big’s Fishing Derby, Sonic Classic Heroes and both scalers reduced that to 2.10% (5/7 pairs) with no median CPU reduction. Its scaler-v1 gain changed from 2.80% to -0.25%. Windows GCC scaler-v2 remained slightly slower (-1.09%, then -0.49% elapsed; -1.09%, then -2.17% CPU), while native scaler-v2 changed from +1.41% to -0.19%. The mixed results did not establish a significant portable benefit; the guard was removed.

Artifacts: `trial-string-gate-20261008`, batches `screen` and `confirmation`, including exact patch, saved executables, tests, timing results and disassembly. Confirmation is a four-workload subset and must not be presented as a full-cohort aggregate.

## 2a. Macro and IRP expansion length returns — discarded

ExpandTokens returned its existing output-length counter; sequential replacement retained a final strlen, and constant macro lines reused as_dynstr_copy_c_str’s return value. MACRO_Processor and IRP_Processor consumed these lengths instead of rescanning. ProcessFile still performed its original scan, making this independent of the later reader trial. All 327 tests passed on each toolchain; every workload’s code and normalized listing matched its parent.

The three-pair full screen gave geometric elapsed/CPU savings of -0.29%/+0.06% on Windows GCC, +1.33%/+1.02% on MSVC and +0.63%/+0.74% on native GCC. A seven-pair confirmation covered Sonic & Knuckles, Sonic Classic Heroes and both scalers. Windows scaler elapsed gains were 1.35%/1.60% on GCC and 1.60%/1.87% on MSVC for v1/v2, but Windows GCC Sonic Classic Heroes repeated a 4.11% elapsed loss (-0.98% CPU). Native scaler v2 changed from +1.29% to -1.53%, winning only 1/7 confirmation pairs. The small gains and repeated losses did not justify retaining the change.

Artifacts: `trial-macro-length-20261008`, `screen` and four-workload `confirmation` batches. The API and callers were restored before the next trial.

## 2b. Reader lengths and ProcessFile rescan removal — discarded

A new ReadLnContWithLength entry point returned stored characters independently of physical-line counts; the old interface remained available. INCLUDE_Processor retained physical counts locally and stored the character length, allowing ProcessFile to consume every processor’s existing length. A focused test checked both APIs, CRLF, continuations, EOF/DOS EOF, empty input and 16 KiB lines. All 328 tests passed on each toolchain, and all workload code and normalized listings matched the parent.

The three-pair full screen gave geometric elapsed/CPU savings of +1.58%/+1.18% on Windows GCC, +1.15%/+0.21% on MSVC and -1.22%/-0.75% on native GCC. Large initial native losses were not repeated in a nine-pair confirmation on five selected workloads. A separate seven-pair full-cohort confirmation then gave only +0.26%/+0.61%, +0.62%/+0.86% and +0.29%/+0.10% respectively. Windows scaler-v2 elapsed savings weakened from +2.51%/+2.56% (GCC/MSVC) to +1.37%/+0.27%; native scaler v2 was -0.71% in the full confirmation. Windows scaler-v1 elapsed gains were stronger, but the overall results were too small and inconsistent to justify the extra API and bookkeeping. The implementation and focused test were removed.

Artifacts: `trial-reader-length-20261008`, batches `screen`, `native-confirmation` (native only, five cases) and `confirmation` (full cohort). Do not combine subset and full-cohort aggregates.

## 2c. SplitLine length reuse after define expansion — discarded

ExpandDefines returned whether a replacement changed the stored length; SplitLine reused the input span length when no length change occurred. Equal-length replacements remained eligible, and shrinking/growing replacements retained the final strlen. A synthetic define fixture checked growing, shrinking, equal-length and quoted cases with independent expected bytes. All 328 tests passed on each toolchain, and all eleven workload code outputs and normalized listings matched the parent.

The three-pair full screen gave geometric elapsed/CPU savings of -0.10%/+0.37% on Windows GCC, +0.32%/+0.30% on MSVC and -0.62%/-0.81% on native GCC. Individual results were mixed, with no convincing broad gain. No further timing confirmation was warranted; the implementation and fixture were removed.

Artifacts: `trial-split-length-simple-20261008`, batch `screen`. An initial wrapper version in `trial-split-length-20261008` passed tests but was simplified before timing and has no performance result.

## 3. Whitespace classification table — retained

as_isspace now reads a shared 256-byte table. Its static initialization covers the process's initial C locale, including calls before strutil_init. Explicit initialization refreshes the table from libc classification, preserving changed LC_CTYPE settings, byte masking and signed-character behavior. A regression test checks every masked byte before and after initialization and after available locale changes. All 328 tests passed with Windows GCC, MSVC and native Linux GCC.

The three-pair full screen gave geometric elapsed/CPU savings of 4.24%/3.60%, 2.19%/3.39% and 4.10%/4.31% respectively. A separate nine-pair full-cohort confirmation gave 3.54%/3.72%, 2.55%/3.04% and 4.75%/4.99%. Scaler-v2 elapsed savings repeated at 9.65% on Windows GCC, 8.30% on MSVC and 8.71% on native GCC, winning all nine pairs on each. All eleven workloads retained identical code outputs and normalized listings. These repeatable gains justify retaining the shared implementation.

Artifacts: `trial-space-table-20261008`, batches `screen` and `confirmation`. Subsequent experiments use its frozen binaries as their accepted baseline.

## 4a. Float hexadecimal exclusion without strlen — discarded

The hexadecimal exclusion checked the first two bytes directly inside the existing nonempty-input branch. Conversion, range handling and format precedence stayed unchanged. All 328 tests passed on each toolchain; all eleven code outputs and normalized listings matched the whitespace baseline.

The three-pair full screen gave geometric elapsed/CPU savings of -2.19%/-0.18% on Windows GCC, -0.38%/+0.55% on MSVC and -1.24%/-0.19% on native GCC. Individual results were mixed and did not establish a meaningful portable gain. The check was restored without a confirmation batch. The large individual elapsed fluctuations are not evidence that this tiny edit caused equivalent CPU regressions.

Artifacts: `trial-float-hex-20261008`, batch `screen`, against `329c60f0` and its frozen whitespace binaries.

## 4b. Delay integer overflow-threshold division — discarded

The existing first_digit flag gated mul_max calculation after digit validation; later digits reused the result. No extra digit prescan was introduced. All 328 tests passed on each toolchain, and all eleven code outputs and normalized listings matched the parent. Generated Windows GCC code confirmed that first-digit failures bypassed the division.

The three-pair full screen gave geometric elapsed/CPU savings of +0.80%/-0.03% on Windows GCC, -0.66%/+1.11% on MSVC and +0.22%/+0.19% on native GCC. Workload results were mixed, with little GCC CPU improvement despite the high rejection counts. The screen did not justify confirmation or retention; the original setup was restored. Artifacts: `trial-delayed-divide-20261008`, batch `screen`, against the accepted whitespace baseline.

## 5. Lazy reuse of expression left-operand checks — discarded

operator_search_cb cached whether the current position had nonblank content to its left, sharing that result between temporary-label handling and overlapping operator candidates. The cache was local to one callback position and filled only when needed; there was no additional identifier prescan. All 328 tests passed on each toolchain, and all workload code outputs and normalized listings matched the parent.

The three-pair full screen gave geometric elapsed/CPU savings of -2.82%/-0.07% on Windows GCC, -0.10%/-0.00% on MSVC and +0.54%/+0.59% on native GCC. A -33.21% Windows GCC scaler-v1 elapsed outlier had unchanged median process CPU time and is not evidence of an equivalent code regression. The remaining results were mixed and too small to justify retention. The change was removed. Artifacts: `trial-expression-reuse-20261008`, batch `screen`, against the accepted whitespace baseline.

## 6a. Lazy canonical-name hash reuse across scopes — discarded

FindNode carried a per-lookup name hash through local/global scope attempts. SearchSymbolHashTable filled it only when encountering a nonempty table, then mixed the existing attribute into that same hash. The algorithm, equality and scope order were unchanged. A prehashed lookup API shared the existing probe implementation; a regression test checked parity through deliberate collisions, growth, deleted buckets, reinsertion and clearing, without invoking the hash function again. All 329 tests passed on each toolchain, and all eleven code outputs and normalized listings matched the parent.

The three-pair full screen gave geometric elapsed/CPU savings of -1.23%/-0.27% on Windows GCC, +0.28%/+0.17% on MSVC and -0.70%/-0.74% on native GCC. Some early MSVC wall gains were not supported by comparable CPU reductions; most native cases were slower. The result did not justify retaining the added cache plumbing and API. Implementation and focused test were removed. Artifacts: `trial-name-hash-reuse-20261008`, batch `screen`, against the accepted whitespace baseline.

## 6b. Single-probe symbol insertion below the growth threshold — discarded

EnterSymbolHashTable used set directly while capacity was available, interpreting its existing-key result before calling the original update callback. At the growth threshold it retained get first, preserving existing-symbol updates without allocation and the original failure behavior. New-key callback rejection still deleted the inserted bucket. This was independent of the discarded name-hash cache. All 328 tests passed on each toolchain, and all eleven code outputs and normalized listings matched the parent.

The three-pair full screen gave geometric elapsed/CPU savings of +1.26%/-0.63% on Windows GCC, +0.96%/+0.16% on MSVC and -0.33%/+0.05% on native GCC. CPU results did not establish a significant gain, and workload results were mixed. The original get/set sequence was restored. Artifacts: `trial-single-probe-insert-20261008`, batch `screen`, against the accepted whitespace baseline.

## 7. Shared hexadecimal listing code-value formatter — discarded

Plain hexadecimal code-value fragments used a shared nibble-based formatter with a leading space and minimum zero-padded width. Other radices, byte separators and oversized widths retained the existing formatter, including its existing compiler choices. The new production path had no platform-specific branches. A focused test checked zero, narrow/wide padding, lowercase, values wider than their requested field, the largest unsigned value and buffer boundaries. All 329 tests passed on each toolchain. An additional synthetic listing matrix matched the parent for 82 pairs on each toolchain, covering radices 2–36, address padding, case, separators, byte/word listings and multi-line data. Clock-generated TIME/DATE fields were normalized, like the standard benchmark listings.

The three-pair full screen gave geometric elapsed/CPU savings of -1.77%/-0.16% on Windows GCC, +0.95%/-0.24% on MSVC and +0.70%/+0.81% on native GCC. The four Windows listing workloads had small positive elapsed gains, and all four MSVC listing CPU medians improved, warranting a separate confirmation despite the mixed full screen.

The seven-pair confirmation covered the four listing workloads plus Sonic 1, Sonic 2, Sonic & Knuckles and scaler v2 as controls. Its eight-workload elapsed/CPU aggregate was +0.58%/+0.23%, -0.72%/+0.58% and +0.96%/+0.67% respectively; this is a subset, not a full-cohort result. For the four listing workloads alone, elapsed/CPU savings were only +0.25%/+0.77%, +0.77%/+1.40% and +0.53%/-0.04%. Native Madness 4 debug's +3.41% elapsed result had -1.48% CPU savings, illustrating why elapsed gains alone were insufficient. Windows scaler-v2 CPU medians were slower by 1.22% on GCC and 2.06% on MSVC. The small, inconsistent portable benefit did not justify the additional formatter. Implementation and focused test were removed.

Artifacts: `trial-list-hex-20261008`, batches `screen`, `confirmation` and `listing-matrix`, against the accepted whitespace baseline. All normal workload code outputs and normalized listings matched the parent in both timing batches.

## Final decision

All proposed mechanisms were tested independently, including the three line-length stages and both ordinary-symbol lookup alternatives. Only whitespace classification caching demonstrated a repeatable meaningful gain across all three toolchains; it was committed as `329c60f0`. Its nine-pair full-cohort confirmation is the performance comparison for the final accepted assembler code: elapsed savings of 3.54% on Windows GCC, 2.55% on MSVC and 4.75% on native Linux GCC, with CPU savings of 3.72%, 3.04% and 4.99%. Rejected code and trial-only tests were removed. Subsequent report-only changes do not alter the assembler.

Final accepted-state validation built every utility and passed all 328 tests on Windows GCC, MSVC and native Linux GCC. Each final assembler's complete .text section matched its corresponding frozen executable from the confirmed whitespace trial, verifying that discarded experiments left no machine-code changes. Artifacts: `final-accepted-check-20261009`, with build/test logs and saved executables.

Windows memory monitors recorded at least 9,900 MiB of available memory across the completed timing batches; these records do not indicate the earlier memory-thrashing condition. This does not eliminate ordinary scheduling or I/O noise, so decisions relied on paired elapsed/CPU evidence and independent confirmation of promising changes.

## Diagnostic eligibility counts

A diagnostic-only native pass against the accepted whitespace baseline ran each of the eleven workloads twice, with deterministic counts and identical code outputs. First-digit rejections accounted for 58.53–83.26% of integer-probe division setups. Windows GCC disassembly independently showed an actual unsigned division before digit validation. This supports testing delayed calculation without a complete digit prescan.

The same pass recorded repeated scope lookup attempts and name hashing; empty tables skip actual hashing, so lookup totals alone overstate the opportunity. Listing workloads emitted 525,672–563,650 code-value fragments per assembly, all eligible for ordinary hexadecimal conversion without byte separators. Diagnostic code is absent from normal binaries and was restored in the external mutable source before the next build. Artifacts: `eligibility-space-head-20261008-r2`; the earlier attempt stopped before compilation because its source-pattern assertion did not normalize CRLF.

## Locale clarification

Inspection for the whitespace candidate found nls_init setting LC_TIME and LC_MONETARY. No LC_CTYPE-changing call was found. Thus the assembler’s character classification remains in the C locale; the initial profile report’s broader statement that no locale-changing calls existed was too broad. Any caching experiment must still support helper use before initialization and preserve byte masking.
