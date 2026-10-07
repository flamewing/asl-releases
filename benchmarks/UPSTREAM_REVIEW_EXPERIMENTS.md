# Upstream performance-review experiments (2026-10-07)

Implement the reviewed plan in independent stages on `experimental`, starting from `f37884f0`. Preserve the five recent correctness fixes. The implementation order is redundant forward-reference search, lazy forward-reference allocation, current expression profiling and scanning, then current source-line profiling and parsing. Keep only individually validated performance gains, using shared Windows/Linux production code. Historical import regressions and synthetic review results motivate experiments but are not current-HEAD speedup claims.

## Method

Freeze a normal Release baseline on Windows GCC 16.2, Windows MSVC 19.44 and native WSL2 Ubuntu GCC 15.2. Use external Windows artifacts under `D:/tmp/asl-bench-20261006` and native ext4 artifacts under `/home/marzojr/asl-benchmark-20261005`; no source-checkout build or timing directories are created. Correct the external fixture-copy harness to include per-test `asflags` and `p2binflags`, and build every standalone test target. The corrected baseline passes all 326 CTest cases on all three toolchains. An earlier copy omitted flags for the restricted-width fixtures and is not used for comparisons.

Time normal uninstrumented binaries, one warmup per binary and alternating pairs, with no overlapping builds, profiles or timing batches. Native batches flush queued writes outside both elapsed and child-CPU measurement. Require successful deterministic objects and identical generated code across compared binaries; normalized listings must also agree for requested listings. Anonymous workloads keep only `local-e` and `local-e-debug` names; private inputs and adapters remain external. Report elapsed and process CPU separately. Positive reductions mean faster. Geometric cohort summaries give equal weight to per-variant median ratios; do not pool independent screens and confirmations.

## Stage 1: one forward-reference root search

Remove the declaration initializer whose result is overwritten before use. Keep the loop search, same-name iteration and resolution rules. Cast the root pointer to its leading tree member type without forming a member address from an empty root. The focused test includes the production forward-reference implementation and links the production tree traversal, counting root searches without instrumentation in the assembler binary. It covers empty/missing names, one and multiple scope attributes, section ancestry, macro isolation, resolved entries, global resolution, duplicate insertion and pass cleanup, with tree balancing both enabled and disabled. All 327 CTest cases pass on each toolchain.

Synthetic query controls retain 128 or 8,192 unresolved distinct names while repeatedly querying an unused defined symbol. Additional controls cover repeated and unique forward references, literals and already-defined symbols. Assembly timing and game inputs remain external.

The five-pair query-assembly screen gives geometric elapsed/CPU reductions of 1.13% / -3.77% (Windows GCC), 1.38% / -3.08% (MSVC), and 1.26% / 1.33% (native GCC). The Windows CPU medians are coarsely quantized; this screen does not establish a significant whole-assembly gain.

A separate uninstrumented routine benchmark makes eight million search-and-mark queries after constructing each balanced tree, covering missing names and a matching name whose entry becomes resolved. Five alternating pairs per size give the following elapsed/CPU reductions. These are routine-focused results, not game speedup estimates. The driver independently requires exactly one successful resolution.

| Tree entries | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| 128 | 45.60% / 44.19% | 47.86% / 47.06% | 48.23% / 48.24% |
| 8,192 | 48.58% / 46.67% | 48.26% / 49.06% | 48.36% / 48.37% |
| 65,536 | 44.13% / 44.79% | 45.59% / 46.58% | 46.78% / 46.85% |

| Workload cohort | Windows GCC elapsed / CPU reduction | MSVC elapsed / CPU reduction | Native GCC elapsed / CPU reduction |
| --- | ---: | ---: | ---: |
| Full eleven-variant screen, three pairs | 0.18% / -0.12% | -1.67% / -0.47% | -0.60% / -0.39% |

The screen preserves all generated code and normalized listings. Its apparent MSVC Sonic 2 loss is 4.41% elapsed / 4.94% CPU; the independent five-pair repeat instead improves 0.89% / 1.20%, so do not treat the first batch as a confirmed regression. The focused operation-count test fails against the unchanged double-search implementation, and the corrected implementation passes native address and undefined-behavior sanitizers. Keep all samples and report the separate confirmation rather than replacing the screen. External artifacts: `review-single-search-v2-20261007/{queries-screen,workloads-screen,workloads-confirm,candidate.patch}`, `review-plan-search-micro`, and `review-plan-baseline-v2-20261007`.

The independent four-variant, five-pair confirmation (Sonic 2, Sonic 3 & Knuckles, scaler v1 and scaler v2) has geometric elapsed/CPU reductions of -3.30% / -0.54% for Windows GCC, -0.05% / -0.42% for MSVC and 1.11% / 1.16% for native GCC. Windows GCC Sonic 3 & Knuckles loses 9.17% elapsed with unchanged median CPU and mixed pairs; native scaler v1 changes from -6.59% / -6.59% in the screen to -0.82% / -0.79% in the repeat. The short MSVC scaler v1 CPU medians differ by a single 15.625 ms timing quantum, while repeat elapsed differs by -0.32%. These observations do not establish a general assembly-speed improvement. Retain Stage 1 specifically for the independently measured 44–49% routine CPU reduction and the verified removal of one root traversal; preserve the mixed whole-workload evidence and do not advertise it as an aggregate build gain.

## Stage 2: lazy forward-reference allocation

Add a creator-based tree insertion API that searches the full name-and-attribute key once and constructs an owned entry/name only at an absent leaf. The existing insertion API retains its callback and replacement behavior through the same balancing implementation. Duplicate keys leave the existing entry, including its resolved state, untouched. Keep section/macro attributes, pass cleanup and the existing usage-tracking callers unchanged.

Extend the production-code test with 1,024 randomly ordered distinct keys and sixteen duplicate rounds under both balancing modes. Assert one entry/name allocation per unique key and zero additional allocations for duplicate rounds, with correct resolution after duplicates and cleanup. The test fails against Stage 1's eager insertion as expected; the candidate passes native address/undefined-behavior sanitizers and all 327 CTest cases on each toolchain.

The first implementation carries separate name and attribute arguments through the recursive insertion routine. Its five-pair duplicate-heavy screen improves elapsed/CPU by 14.35% / 17.02% (Windows GCC), 9.30% / 12.50% (MSVC) and 4.36% / 4.35% (native GCC). Unique-key elapsed changes are 0.56%, -1.31% and 0.01%, respectively.

The separate seven-pair confirmation preserves duplicate-heavy gains of 15.81% / 15.22%, 12.72% / 12.50% and 2.28% / 2.28%, but native unique references slow 4.89% elapsed / 4.85% CPU with only one of seven elapsed pairs favoring the trial. Reject this version despite its duplicate-heavy benefit. Keep the first screen and independent confirmation separate.

The full eleven-variant screen for this rejected version gives geometric elapsed/CPU reductions of 0.37% / 0.24% for Windows GCC, -0.96% / -0.26% for MSVC and 0.67% / 0.75% for native GCC. Preserve elapsed/CPU disagreements: Windows MSVC Sonic 3 & Knuckles slows 6.89% elapsed with unchanged CPU, and native Madness 4 release improves 4.53% elapsed while CPU worsens 2.06%. No general workload gain is credited. All generated code and normalized listings match Stage 1. Artifacts: `review-lazy-tree-20261007/{allocation-screen,allocation-confirm,workloads-screen,candidate.patch}`.

Refine the shared traversal to pass a borrowed stack key instead of separate name/attribute arguments, reducing recursive arguments from seven to five. The borrowed key is never inserted or freed; only the creator's owned result enters the tree. This is a new independent candidate against Stage 1, not a comparison against the rejected version.

The borrowed-key five-argument version passes all 327 tests and sanitizer/allocation checks. Its seven-pair duplicate-heavy screen improves elapsed/CPU by 17.21% / 17.02% (Windows GCC), 14.40% / 12.50% (MSVC) and 4.41% / 4.42% (native GCC). Unique-key elapsed changes are -0.32%, 0.65% and -0.59%, with mixed pairs and zero Windows median CPU changes. The independent seven-pair duplicate confirmation remains positive at 14.30% / 15.22%, 12.92% / 12.50% and 2.80% / 2.78%; unique-key changes are 1.34% / 0.00%, -3.09% / 0.00% and -1.26% / -1.15%, again mixed.

Its full eleven-variant screen has geometric elapsed/CPU reductions of 0.28% / 0.18% (Windows GCC), 0.74% / 0.70% (MSVC), and -0.88% / 0.04% (native GCC). Native Madness 4 debug's -12.68% elapsed / 2.00% CPU outlier changes to 1.82% / 1.47% in an independent repeat. Native Sonic 1's -3.41% / -3.40% screen result repeats at -2.53% / -2.50%, so reject this version rather than accepting its targeted gain alongside the remaining game loss. The four-variant, five-pair repeat covers Sonic 1, Sonic 2, Sonic 3 & Knuckles and Madness 4 debug: geometric elapsed/CPU reductions are 0.22% / 0.58%, 0.80% / -0.18% and -0.26% / -0.37%. Artifacts: `review-lazy-key-20261007/{allocation-screen,allocation-confirm,workloads-screen,workloads-confirm,candidate.patch}`.

The final refinement gives lazy insertion a separate four-argument traversal and a compact read-only borrowed key. Ordinary insertion retains its four-argument interface without a creator mode. Both paths share standard inline AVL balancing helpers, preserving one balancing implementation. This removes additional recursive argument/mode work from existing callers, without compiler/OS-specific source. All 327 tests pass on each toolchain before timing.

The separate-traversal refinement's seven-pair synthetic screen improves duplicate-reference elapsed by 16.29% (Windows GCC), 10.86% (MSVC) and 5.96% (native GCC). Unique-reference elapsed changes are -2.64%, -0.21% and -0.81%; literal changes are -7.33%, -10.04% and -0.30%, and defined-symbol changes are -1.37%, -0.60% and 2.91%. This trial was superseded by the requested hash-table experiment before independent confirmation or the full game cohort, so it is neither accepted nor a confirmed rejection. Its tree API and AVL helper changes are removed from the working candidate. Artifact: `review-lazy-isolated-key-20261007/allocation-screen`.

### Name-indexed hash-table experiment

Replace only the private forward-reference tree with the existing portable open-addressed hash-table implementation. The table owns one string per distinct expanded name and a linked list of records for its scope attributes. Insertions allocate a scope record only when the full name-and-attribute key is absent; duplicate insertion preserves the resolved flag. Name resolution visits that name's records and retains the exact global, macro-local and section/subsection visibility rules. No source ordering is required. Cleanup frees every scope, owned name and table array at pass end, avoiding storage retention between passes. Ordinary symbol tables and the generic tree implementation remain unchanged. The hash-table comparison baseline is committed Stage 1, not any uncommitted lazy-tree refinement.

The focused production-code test retains empty/missing, visibility, resolved-state and pass-reset checks, and adds 1,024 shuffled names, sixteen duplicate rounds, complete hash collisions across resizing, borrowed input-buffer reuse and 128 macro scopes sharing one owned name. Duplicate insertions must allocate neither records nor names. Both tree-balancing option states produce the same results. All 327 CTest cases pass on Windows GCC, Windows MSVC and native GCC; the focused test also passes address and undefined-behavior sanitizers. Scope-list insertion and resolution remain linear in the number of scopes attached to a name; measure scope-heavy behavior before treating the table as a universal improvement.

The first hash-table version is rejected. The repeated-reference benefit confirms independently, but the 8,192-scope native input loses 23.25% elapsed / 23.27% CPU in all five pairs, the larger distinct-name input loses 3.61% / 4.23% with MSVC, and native Sonic 1 and Classic Heroes regress again in independent workload repeats. All outputs and normalized listings match the comparison baseline. Positive numbers below indicate savings; batches are independent and are not pooled.

| hash-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| literal | -10.61% / 0.00% | 1.55% / -12.50% | -4.94% / -4.97% |
| defined | 7.21% / 0.00% | 6.54% / 0.00% | 1.96% / 1.97% |
| duplicate-forward | 16.38% / 15.56% | 12.73% / 11.32% | 3.11% / 3.13% |
| unique-forward | -0.78% / 0.00% | -4.49% / 0.00% | 9.39% / 9.27% |
| query-128 | 0.10% / 0.00% | 2.58% / 6.67% | 1.71% / 1.68% |
| query-8192 | 6.36% / 0.00% | 0.14% / 5.88% | 2.78% / 2.94% |

| hash-confirm | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| literal | 0.48% / 0.00% | -3.28% / -12.50% | 0.11% / 0.07% |
| defined | -1.14% / 0.00% | 1.75% / 4.35% | -0.52% / -0.50% |
| duplicate-forward | 14.59% / 13.95% | 12.51% / 11.32% | 2.70% / 2.61% |
| unique-forward | 0.35% / 0.00% | -1.36% / 0.00% | -0.65% / -0.75% |

| hash-scaling | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| scopes-128 | -0.90% / 0.00% | 2.36% / 0.00% | 5.32% / 3.75% |
| scopes-8192 | 13.45% / 17.65% | 8.63% / 12.50% | -23.25% / -23.27% |
| unique-forward-large | -0.30% / 1.54% | -3.61% / -4.23% | 4.36% / 4.34% |

| workloads-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 2.74% / 2.17% | -8.99% / -4.17% | -3.45% / -3.47% |
| Sonic 2 | -3.71% / -1.41% | 1.10% / -1.30% | 2.40% / 2.23% |
| Sonic 3 & Knuckles | 5.01% / -1.16% | -5.19% / -3.19% | 3.56% / 3.56% |
| Big’s Fishing Derby | 3.80% / -1.64% | -9.43% / 0.00% | 0.52% / -0.37% |
| Sonic Classic Heroes | 0.25% / 0.99% | 0.04% / 0.00% | -2.88% / -2.88% |
| Scaler v1 | 0.54% / 0.00% | 3.53% / 0.00% | -0.00% / 0.00% |
| Scaler v2 | -0.81% / -1.16% | 5.64% / 1.01% | 0.43% / 0.39% |
| Madness 4 release | -2.25% / -0.60% | 0.60% / 0.56% | -0.77% / -2.29% |
| Madness 4 debug | -0.94% / -0.57% | -0.45% / -1.09% | -1.36% / -0.30% |
| local-e | 1.66% / 1.30% | -1.27% / -1.85% | -0.82% / 0.83% |
| local-e debug | -0.51% / -0.68% | -1.99% / 0.00% | 1.49% / 0.82% |

| workloads-confirm | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 0.40% / 0.00% | 1.56% / 0.00% | -5.75% / -5.35% |
| Sonic 2 | -5.49% / -1.41% | -1.41% / 0.00% | -4.84% / -4.87% |
| Sonic 3 & Knuckles | 2.08% / -1.15% | 1.00% / -1.08% | 8.15% / 7.96% |
| Big’s Fishing Derby | -2.12% / 0.00% | -2.96% / -2.99% | -0.88% / -0.81% |
| Sonic Classic Heroes | -1.35% / -1.02% | 0.52% / 0.00% | -3.74% / -4.49% |
| local-e debug | 0.53% / 1.44% | -0.07% / 0.67% | 0.01% / 1.09% |

Refine the scope lists into descending attribute order. Fresh monotonically increasing macro handles insert at the head, and macro-local resolution finds only its own handle rather than traversing every scope. Arbitrarily ordered handles and sections remain supported; global and section definitions retain the original visibility rules. Regression tests insert the 128 scopes in a nonmonotonic permutation and resolve them in a different order. This is a new candidate against committed Stage 1, not against the rejected table. Artifacts for the first version: `review-forward-hash-20261007/{hash-screen,hash-confirm,hash-scaling,workloads-screen,workloads-confirm,candidate.patch}`.

The ordered-scope candidate passes all 327 tests on each toolchain and the focused address/undefined-behavior sanitizer test. Accept it for the independently confirmed scope-heavy improvement and allocation avoidance, without claiming an overall game-build gain. Compared with committed Stage 1, the seven-pair confirmation saves 79.82% elapsed / 81.25% CPU on Windows GCC, 76.05% / 75.00% on MSVC and 87.39% / 87.48% on native GCC for 8,192 macro scopes. All seven pairs favor the candidate on each toolchain. Repeated single-name forward references save 11.33% / 13.95%, 8.62% / 9.43% and 1.07% / 1.06%, respectively. The scope-heavy case is a synthetic stress input, not a proxy for all macros or games.

| hash-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| literal | 2.79% / -14.29% | 3.02% / 0.00% | -4.35% / -4.38% |
| defined | -2.25% / 0.00% | -3.28% / -4.55% | 0.76% / 0.73% |
| duplicate-forward | 12.91% / 13.64% | 10.18% / 11.32% | 0.53% / 0.53% |
| unique-forward | 0.12% / 0.00% | -1.52% / 0.00% | 1.65% / 1.64% |
| query-128 | 0.36% / 0.00% | 3.89% / 6.67% | 2.76% / 2.76% |
| query-8192 | 2.31% / 0.00% | 0.07% / 5.88% | 2.86% / 2.88% |
| scopes-128 | -0.77% / 0.00% | 0.55% / 0.00% | 1.77% / 1.85% |
| scopes-8192 | 79.42% / 81.25% | 76.78% / 81.25% | 87.71% / 87.79% |
| unique-forward-large | 2.64% / 3.03% | 0.54% / 1.39% | 1.14% / 1.21% |

| hash-confirm | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| literal | -1.62% / 0.00% | -8.40% / 0.00% | -1.05% / -1.14% |
| defined | -1.28% / 0.00% | 0.96% / 0.00% | -0.26% / -0.30% |
| duplicate-forward | 11.33% / 13.95% | 8.62% / 9.43% | 1.07% / 1.06% |
| scopes-8192 | 79.82% / 81.25% | 76.05% / 75.00% | 87.39% / 87.48% |
| unique-forward-large | 2.35% / 3.03% | -2.05% / -2.82% | 2.32% / 2.33% |

| unique-check | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| 327,680 distinct names | -0.44% / 1.43% | -0.03% / 1.28% | -0.73% / -0.93% |

The larger distinct-name MSVC control is 0.54% elapsed / 1.39% CPU faster in the first seven-pair screen, then 2.05% / 2.82% slower in the independent confirmation. A focused third seven-pair check is -0.03% elapsed / 0.00% CPU, with four of seven elapsed pairs favoring the candidate, so the earlier loss is not reproduced. The third check gives -0.44% / 0.00% on Windows GCC and -0.73% / -0.73% on native GCC, both with mixed pairs. Do not credit a distinct-name speedup. Short Windows CPU medians can differ by a single 15.625 ms quantum; retain elapsed and CPU separately.

| workloads-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 0.73% / -2.22% | -2.45% / 0.00% | 2.07% / 2.06% |
| Sonic 2 | 1.67% / 0.00% | 3.88% / -1.30% | -1.78% / -1.78% |
| Sonic 3 & Knuckles | -0.59% / 0.00% | 1.28% / 2.15% | 0.07% / 0.15% |
| Big’s Fishing Derby | 6.97% / 1.64% | -0.65% / -1.52% | 1.45% / 2.53% |
| Sonic Classic Heroes | 2.64% / 1.02% | 0.27% / 0.00% | 1.51% / 0.84% |
| Scaler v1 | 0.40% / 0.00% | -8.21% / -3.23% | -0.39% / -0.39% |
| Scaler v2 | -1.28% / -2.38% | 0.51% / 0.00% | 0.86% / 0.92% |
| Madness 4 release | -0.22% / 0.00% | -0.83% / 1.67% | 5.70% / 2.98% |
| Madness 4 debug | -1.16% / 0.00% | 1.06% / -0.54% | 0.13% / -1.89% |
| local-e | 1.66% / 1.46% | -1.59% / 0.68% | 2.80% / 1.73% |
| local-e debug | 1.56% / 1.45% | 1.07% / 0.67% | 0.31% / 0.97% |

| workloads-confirm | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 1.78% / 0.00% | -1.31% / 0.00% | 1.75% / 1.73% |
| Sonic 2 | 0.60% / 0.00% | 0.06% / 0.00% | 5.15% / 5.42% |
| Sonic Classic Heroes | -3.08% / -0.96% | 1.98% / 0.90% | 0.55% / 1.12% |
| Scaler v1 | 0.56% / 0.00% | -1.54% / -3.03% | -0.83% / -0.90% |
| Scaler v2 | 2.56% / -1.09% | 0.06% / 0.00% | 0.10% / 0.10% |
| Madness 4 release | -0.35% / 1.13% | 0.20% / 0.54% | -3.49% / -3.12% |

The full eleven-variant screen has geometric elapsed/CPU savings of 1.15% / 0.10%, -0.47% / -0.12%, 1.18% / 0.75% (Windows GCC, MSVC, native GCC). The independent six-variant confirmation is 0.36% / -0.15%, -0.09% / -0.26%, 0.57% / 0.76% in the same order. Keep the cohorts and batches separate. Native Madness 4 release changes from 5.70% elapsed / 2.98% CPU faster in the screen to 3.49% / 3.12% slower in confirmation, with mixed pairs; Windows GCC Classic Heroes changes from 2.64% / 1.02% faster to 3.08% / 0.96% slower. The short MSVC scaler v1 screen loss of 8.21% elapsed becomes 1.54% in the repeat; its CPU difference is one timing quantum. These results do not establish an overall throughput improvement or a consistent large workload regression.

Every timed run succeeds, generated code is deterministic per binary and equivalent across the compared revisions/toolchains, and normalized listings match within each compiler. Both local-e variants match the prepared comparison baseline on all three toolchains; that baseline itself differs from the older aggregate golden, so no agreement with the older golden is claimed. All timing runs and builds are separate, with artifacts outside the checkout. Final artifacts: `review-forward-hash-ordered-20261007/{hash-screen,hash-confirm,unique-check,workloads-screen,workloads-confirm,candidate.patch}`. The generic tree implementation and ordinary symbol tables are unchanged. Expression-scanning and source-line profiling remain later stages of the review plan.
