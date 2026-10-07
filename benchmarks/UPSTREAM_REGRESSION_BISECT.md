# Upstream performance regression bisection (2026-10-07)

Compare `master` at `3b603b887382d1964305152c295376e4de0c3127`, the pristine sync at `fb75b92ddad1d87e6003868b622971b11b9f87c4`, and the optimization baseline at `07dcf60978ae9e3571804a68d4e254bdcac6b656`. Preserve the working checkout on `experimental`. Use an external shared-object control repository and no-checkout Git bisections, with exact source archives and native ext4 builds. All build, input and result directories remain outside the checkout under the established external benchmark roots. Markdown paragraphs are unwrapped; anonymous workloads retain only `local-e` / `local-e-debug` labels.

## Search and validation

Use uninstrumented Release GCC binaries with `-O3 -DNDEBUG -g`, one warmup and alternating revision pairs. Flush native queued writes before each assembly outside elapsed and child-CPU measurement. Use scaler v2 as a successful, macro-heavy search signal, with a fixed reference per stage. Classify by median child CPU: 10% above master, then 10% above build 267, then 5% above build 306. Search samples have three pairs; samples within two percentage points of a threshold add five independent pairs. Retain original batches rather than replacing them. A threshold crossing nominates a commit; independent comparisons against its immediate parent are required before attributing a regression. Do not infer that a commit itself accounts for the entire accumulated threshold.

Require successful deterministic objects and identical generated code across each comparison. Listings must be deterministic after timestamp normalization within a revision, but historical upstream listing-format changes preclude cross-revision text equality. Corrected optimization trials require normalized listing equality within each compiler as well as code equality. The anonymous variants already differ from the older aggregate golden code in the accepted baseline and remain identical between each trial and its baseline; no older golden-hash agreement is claimed for those variants. No builds, profiles or timing batches overlap.

Search artifacts are `bisect-20261007/{macro-stage1,macro-stage2,macro-stage3}-{history.json,bisect.log}` and `output-experiments/bisect-<commit-prefix>-20261007`, with source archives, compiler logs, binary hashes, raw samples, diagnostics, objects/listings and pressure counters. Historical search builds compile the assembler; this is not a claim that their full test suites were run.

## Compatibility failure

Pristine build 287 cannot assemble Sonic 2's temporary-label indexed displacements, including `+(pc,d2.w)` and `++(pc,d2.w)`. Build 298 (`3248dfc`) already supplies the missing temporary-label check in `pc_outdisp_qualifier`. Apply only that small qualifier block to a separate build-287 archive. Sonic 2 then assembles successfully with deterministic code identical to master. Do not mark a failed assembly as a performance regression.

The five-pair pristine-versus-overlaid scaler control changes elapsed/CPU by -0.39%/-0.40% (positive means faster); three of five elapsed pairs favor the overlay. This small mixed control does not explain the large regression. Keep compatibility-overlay results separate from pristine bisections. Artifacts: `bisect-287-temp-compat-20261007/{compat-overlay.json,overlay-control,compat-workloads}`. The fix already exists in the current branch; no duplicate production fix is needed.

## Bisected commits

| Commit | Change | Native parent comparison, elapsed / CPU increase |
| --- | --- | ---: |
| `01c05c6`, build 267 | Generalized expression callbacks and operator discovery replace the fork scanner | Scaler v2 14.84% / 14.95%, seven pairs |
| `c2d41b3`, build 306 | Source-line metadata and line-parser rework | Sonic 2 15.59% / 15.67%; scaler v2 12.43% / 12.39%, seven pairs each |
| `fb75b92`, build 313 | Integer bit-width propagation and reduced-width complement support | Scaler v2 1.88% / 1.88%, seven pairs; smaller than the 5% search threshold |

Every scaler pair favors the parent for builds 267 and 306. Build 306's Madness 4 release comparison is less consistent: 5.07% elapsed / 5.37% CPU increase, with four of seven elapsed pairs favoring the newer revision. Do not present that workload as an equally strong independent signal. Direct build-313 comparisons are much smaller than the earlier build-306-to-313 endpoint estimate; retain both observations and avoid attributing the earlier approximately 7% endpoint increase entirely to build 313.

Build 267 introduces per-expression inventory probes and a generic per-character callback. Accepted earlier changes already restore immutable built-in dispatch chains and reduce callback overhead. Preserve the new target/callback operators, precedence and quote handling when evaluating additional corrections. Build 306 resets five components before parsing, even when subsequent copies immediately replace their contents and positions, and adds repeated source-length work. These are concrete sites for independent experiments, not proof that any single one explains its entire regression.

## Correction experiments

Results are recorded after each independent trial. Keep accepted upstream semantics and shared Windows/Linux source; do not add OS-specific optimization branches.

### Windows confirmation

| Commit / workload | Windows GCC elapsed / CPU increase | Windows MSVC elapsed / CPU increase |
| --- | ---: | ---: |
| Build 267, scaler v2, seven pairs | 19.36% / 17.95% | 11.73% / 13.19% |
| Build 306, scaler v2, five pairs | 14.13% / 14.44% | 9.71% / 10.48% |
| Build 306, Sonic 2, five pairs | 3.09% / 3.66% | 1.25% / 3.45% |
| Build 313, scaler v2, seven pairs | -2.27% / -1.85% | 1.48% / 1.72% |

The large scaler regressions at builds 267 and 306 reproduce on all three toolchains, with CPU increases corroborating elapsed changes. All their Windows scaler pairs favor the immediate parent. Sonic 2's build-306 impact differs materially by toolchain. Build 313's direct comparison is mixed and small: Windows GCC improves while MSVC and native GCC slow slightly. It is a nominated threshold endpoint, not a third broadly confirmed substantial regression. Artifacts: each historical archive's `adjacent-267-windows`, `adjacent-306-windows`, or pristine sync `adjacent-313-windows`; native `adjacent-267-scaler`, `adjacent-306`, `adjacent-313`, `stage2-endpoint`, and `post306-endpoint` remain separate.

### Operator state in the existing scratch pool: discarded

Move the operator-discovery state from each evaluator stack frame into the already owned recursive scratch record. Each active child owns a separate checked-out record, preserving parent state, custom operators and cleanup. All 320 tests pass on each toolchain. Four representative workloads (Sonic 2, scaler v2, Madness 4 release and local-e debug) preserve deterministic code and normalized listings, with three alternating pairs.

| Cohort | Windows GCC wall / CPU reduction | MSVC wall / CPU reduction | Native GCC wall / CPU reduction |
| --- | ---: | ---: | ---: |
| Scratch-pool pilot, four variants | -1.25% / -1.11% | -3.17% / -1.18% | -0.21% / -0.08% |

Scaler v2 slows 5.12% elapsed / 5.81% CPU with Windows GCC and 6.55% / 2.04% with MSVC. Native scaler savings of 1.92% / 1.93% are offset by Madness 4 release's 2.12% / 3.76% loss. Moving allocation is not sufficient evidence of saved work; discard the shared candidate. No new compiler/OS fork is retained. Artifacts: `bisect-operator-scratch-20261007/{pilot-sync,candidate-sources,candidate.patch}` and builds/tests.

### Avoid clearing components immediately overwritten by copies

Build 306 clears opcode, arguments and comments before the new parser copies them. The trial clears opcode/arguments only when no parsing path has assigned them, and clears comments only when no comment was copied. Label and attribute initialization and all parsing rules remain unchanged. The first implementation uses a bookkeeping flag; all 320 tests pass on each toolchain and all eleven code/listing outputs match.

| Cohort | Windows GCC wall / CPU reduction | MSVC wall / CPU reduction | Native GCC wall / CPU reduction |
| --- | ---: | ---: | ---: |
| Initial reset screen, eleven variants, three pairs | -0.15% / 0.48% | -2.22% / -0.33% | 2.98% / 0.61% |

Do not credit native elapsed-only extremes as equivalent assembler savings: Big's Fishing Derby improves 12.11% elapsed but CPU worsens 0.26%, and Madness 4 release improves 5.71% elapsed but CPU worsens 0.10%. MSVC Sonic 1's 12.84% elapsed loss has unchanged median CPU; scaler v2's 9.39% elapsed loss includes a smaller 2.00% CPU increase. A direct control-flow refinement removes the bookkeeping flag while preserving the same component states. Both versions pass all 320 tests on each toolchain; the refinement is evaluated independently against the accepted baseline. Artifacts: `bisect-line-resets-20261007/screen-sync` and `bisect-line-resets-v2-20261007`.

The four-case, five-pair refinement confirmation covers Sonic 1, Sonic 2, scaler v2 and local-e debug. Geometric wall/CPU reductions are Windows GCC -0.80% / 0.28%, Windows MSVC -1.13% / -1.05%, native GCC 2.78% / 1.86%. This does not establish a sufficiently broad saved-CPU improvement; remove both reset variants without a production performance commit. Preserve the full initial screen and independent confirmation rather than pooling differing cohorts or discarding inconvenient samples.

### Integer-syntax rejection correctness fix

Build 313 changes the literal parser to return Boolean but retains `return -1` when no integer notation matches. That result is true, so `INTSYNTAX -dec` causes ordinary symbols to be accepted as zero-valued integer literals before symbol lookup. Reproduce the bug with untouched accepted-baseline binaries on Windows GCC, MSVC and native GCC: a symbol defined as `0x2a` produces an immediate zero, and `0x2b+value` produces `0x2b` rather than `0x55`. Return `False` on that rejected parse, allowing existing float, expression and symbol handling to continue. Extend `t_intsyntax` with the symbol and compound-expression cases and independently specified expected bytes. Artifacts: `bisect-intsyntax-regression-20261007` and `bisect-intsyntax-fix-20261007`. This is a correctness correction, not a claimed performance gain.

The correctness correction is committed as `9123ee9`; all 320 CTest cases, including the extended INTSYNTAX fixture, pass on Windows GCC, Windows MSVC and native Linux GCC. The reset variants are fully removed before this build.

### Integer digit counting

Build 313 maintains a byte-sized digit counter, a first-digit flag and an Intel hexadecimal leading-zero test inside every digit conversion, even when reduced-width complements are disabled. The first trial derives the count from the selected digit span before conversion. The refinement derives it from the consumed span only after successful positive conversion, and only when reduced-width complements are enabled. Preserve byte-count wraparound, negative-result width, overflow diagnostics, character translation and the existing Intel leading-zero behavior, including the one-digit suffix boundary.

Both trials pass all 320 tests on each toolchain. An independent extracted-function harness compares the accepted parser and each trial across 720,000 conversion cases per toolchain, checking acceptance, values, widths, flags and diagnostics; Linux additionally runs with address and undefined-behavior sanitizers. Cases include signs, notation boundaries, invalid characters, range limits and long zero runs that wrap the byte counter. These comparisons supplement the assembler regression suite and workload output checks.

| Cohort | Windows GCC wall / CPU reduction | MSVC wall / CPU reduction | Native GCC wall / CPU reduction |
| --- | ---: | ---: | ---: |
| Digit count before conversion, eleven variants, three pairs | 0.56% / 0.86% | -1.42% / -0.77% | 1.24% / 1.18% |

The initial trial is mixed and is not retained. Its native Sonic 1 improvement is 16.80% elapsed / 16.80% CPU, but both Madness 4 variants slow approximately 3%, while Windows MSVC worsens overall. Preserve these results without attributing the outlier to an unverified external cause. Artifacts: `bisect-digit-count-20261007/screen-sync` and `bisect-digit-count-checks-20261007`; refinement artifacts are `bisect-digit-count-v2-20261007` and `bisect-digit-count-v2-checks-20261007`.

The refinement's full eleven-variant, three-pair screen yields geometric elapsed/CPU reductions of 0.12% / 0.33% for Windows GCC, -0.08% / -1.33% for MSVC and -0.06% / 0.83% for native GCC. Native Madness 4 debug improves 14.58% elapsed / 9.95% CPU, but Sonic 1 and Big's Fishing Derby slow about 3% by both measures and scaler v2 also worsens. The shared candidate does not establish a broad meaningful improvement. Both digit-count variants are discarded without a performance commit. All eleven variants preserve deterministic objects and baseline generated code; normalized listings also match for every requested listing.

Cohort summaries are equal-weight geometric means of per-variant median ratios. Elapsed and CPU are summarized independently; differing cohorts and historical search protocols are not pooled. All correction trials use normal uninstrumented Release builds and the same accepted baseline within each comparison.

## Starting-HEAD endpoint check

Five independent native GCC pairs compare the untouched starting HEAD `07dcf609` with pre-sync master, using the historical-build flags. Positive values mean the starting HEAD is faster. This measures the branch before the new INTSYNTAX correction and does not claim that the discarded trials improve the final branch.

| Workload | Native elapsed / CPU reduction |
| --- | ---: |
| Sonic 1 | -19.16% / -19.21% |
| Sonic 2 | -18.28% / -18.29% |
| Sonic 3 & Knuckles | -19.74% / -19.74% |
| Big's Fishing Derby | -13.84% / -12.86% |
| Sonic Classic Heroes | -15.89% / -16.68% |
| Scaler macros v1 | -24.16% / -24.17% |
| Scaler macros v2 | -25.95% / -25.95% |
| Madness 4 release | -16.32% / -18.38% |
| Madness 4 debug | -17.38% / -13.13% |
| local-e | 9.59% / -18.88% |
| local-e debug | -10.20% / -19.15% |

All eleven variants assemble successfully, produce deterministic outputs and identical generated code across revisions. The branch still has substantial regressions relative to pre-sync master for most workloads, despite earlier optimization gains. The anonymous release variant improves in elapsed time but consumes 18.88% more CPU; its debug variant worsens by both measures. Do not interpret that elapsed-only improvement as reduced assembler work. Keep the variant distinction. Artifacts: `bisect-07dcf60978ae-20261007/endpoints-native`.

## Retained changes and remaining work

The bisections and independent parent comparisons identify builds 267 and 306 as substantial regression points on the scaler workload across all three toolchains. Build 313 has a smaller, mixed direct impact and also introduced the Boolean rejection bug corrected in `9123ee9`. The historical Sonic 2 compatibility failure is repaired by the existing build-298 qualifier fix; the isolated overlay verifies that diagnosis without turning a failed build into a timing result.

None of the scratch-pool, line-reset or digit-count performance trials meets the retention criterion. Their production changes are removed, leaving the INTSYNTAX correctness correction and this report. The substantial remaining regressions are not claimed to be fixed. Further experiments should target build 267's expression-discovery work and build 306's line parsing while preserving target callbacks, quoting, source positions and upstream grammar; these measurements do not justify reverting those features wholesale.
