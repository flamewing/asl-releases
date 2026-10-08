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

## Locale clarification

Inspection for the whitespace candidate found nls_init setting LC_TIME and LC_MONETARY. No LC_CTYPE-changing call was found. Thus the assembler’s character classification remains in the C locale; the initial profile report’s broader statement that no locale-changing calls existed was too broad. Any caching experiment must still support helper use before initialization and preserve byte masking.
