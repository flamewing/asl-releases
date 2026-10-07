# Upstream-sync optimization experiments

The starting point is experimental compatibility-fix commit `4f12da8`, based on upstream build 313. The CHARSET high-byte correction and inactive ENDM/ENDR guard are part of this baseline. The anonymous local-e benchmark copy also has its invalid expression corrected on Windows and Linux; both release and debug variants are included going forward. Identifying source details and private adaptations remain external.

Each experiment is applied separately to the latest accepted baseline, built and tested with Windows GCC, Windows MSVC and native Linux GCC, then measured with alternating baseline/candidate runs after a warmup. Timings run without concurrent builds or profiling. Generated code must match exactly between the compared binaries and remain deterministic; listing workloads also check normalized listings between baseline and candidate within each compiler. GCC/MSVC floating-point built-ins can differ in listing text even when code matches. Screen results alone do not justify committing an optimization: promising gains require an independent confirmation batch. All builds, inputs and raw results remain under D:/tmp or the native WSL benchmark directory, outside the watched checkout. Markdown paragraphs are not wrapped.

## Ordered experiments

1. Known-length line substring copies.
2. Operator discovery filtering, followed independently by a conservative simple-operand shortcut.
3. Literal spans in the new format parser.
4. Listing-local append cursor, followed independently by prefix-format preparation.
5. Expression copy/trim fusion, delayed backup copying and lazy operand initialization, each independently.

## Results

Results will be recorded here as each experiment is accepted or discarded. Percentage reductions are positive for faster builds and negative for slower builds. The compatibility fixes are already committed; this report and optimization changes will be committed with accepted experiments or the final experiment summary.

### 1. Known-length line substring copies: discarded

A caller-bounded helper replaced seven validated line/attribute extraction sites without using diagnostic position length as string length. Its independent tests covered fixed-buffer truncation and guards, dynamic growth, empty spans, and reference sources; all 319 tests passed on all three toolchains. All eleven corrected workloads produced deterministic baseline-identical code and normalized listings.

The three-pair screen and independent five-pair confirmation gave Windows GCC aggregate wall reductions of 2.26% and 3.32%, with CPU reductions of 2.52% and 2.36%; Windows MSVC gave 1.51% and 2.50% wall, and 2.07% and 2.01% CPU. Scaler v2 showed repeatable Windows wall savings of approximately 4–7%. Linux results were inconsistent across workloads. Crucially, local-e-debug slowed by 8.70% in the screen and 7.46% in a fresh seven-pair targeted repeat, slower in all three and all seven pairs respectively; the intervening confirmation showed only a 1.00% slowdown. The repeated material regression rejects this implementation despite the Windows gains. Production changes and the candidate-specific unit test were removed. External artifacts: upstream-known-spans-20261007/{screen,confirm,stability}, builds/tests and preserved binaries.

The native local-e-debug CPU reductions were +0.14% in the screen and +0.48% in the targeted repeat: the rejection is based on elapsed time, not an established CPU regression. An identical-baseline-binary seven-pair control showed +1.36% wall reduction with mixed pairs, and all normalized listings matched between binaries. This control did not reproduce a consistent label bias; the cause of the candidate-specific elapsed-time behavior remains unestablished. No claim is made that the span helper increases CPU work.

### 2a. Operator discovery filtering: accepted

The evaluator builds a 256-bit first-byte presence map once after constant/callback handling. Absent first bytes skip full-string operator probes; present single-byte operators need no separate strchr. The quote-aware callback also rejects first-byte mismatches before strncmp. Built-in, target and callback-specific operator order, matching, precedence, quote handling and short-circuit behavior are retained. All 318 tests pass on Windows GCC, Windows MSVC and native Linux GCC. Every workload has deterministic identical code across compared binaries; normalized listings match baseline/candidate within each compiler.

| Batch | Variants | Windows GCC wall / CPU reduction | Windows MSVC wall / CPU reduction | Linux GCC wall / CPU reduction |
| --- | ---: | ---: | ---: | ---: |
| Screen, three alternating pairs | 11 | 4.29% / 3.99% | 3.25% / 2.77% | 4.76% / 4.62% |
| Independent confirmation, five alternating pairs | 5 | 4.32% / 4.08% | 2.66% / 3.31% | 4.57% / 3.94% |

These are geometric means within each batch; the two rows have different cohorts. Confirmation cases were Sonic 2, Sonic & Knuckles, ScalerMacros-v2, Madness 4 release and local-e-debug. Sonic 2 confirmed wall reductions of 5.71%/4.04%/6.09% for Windows GCC/MSVC/Linux GCC; scaler v2 confirmed 6.11%/3.31%/3.15%. Every confirmation median improved, with CPU-time averages corroborating the result. No material workload regression was observed in the full screen. The change is accepted. External artifacts: upstream-operator-filter-20261007/{screen,screen-tail,screen-combined.json,confirm}, builds/tests and preserved binaries. The Windows screen is split because the first cross-compiler listing check was too broad; baseline/candidate listings were identical within each compiler, and the corrected check compares them on that basis.
