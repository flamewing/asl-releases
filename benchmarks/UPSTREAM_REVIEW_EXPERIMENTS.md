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

## Hash finalizer experiment, 2026-10-08

Compare a fixed-width unsigned 64-bit avalanche finalizer against committed `be04cdea`, using the frozen `review-forward-hash-ordered-20261007` binaries. Keep the existing name accumulation, scope lists, allocation behavior and visibility rules; add only the final mixing arithmetic. All shifts are within the fixed type width and there is no compiler/OS-specific path. The unchanged collision regression still checks distinct names with identical full hashes. All 327 CTest cases pass on Windows GCC, Windows MSVC and native GCC before timings. Reject this finalizer: it reduces bucket collisions but independently slows the large distinct-name input on all three toolchains, without a consistent representative-workload gain. Restore the committed hash function rather than committing the slower production change.

Use the existing alternating-pair, warmup and correctness-check protocol, with no overlap between timing runs, builds or diagnostic collection. Seven-pair synthetic screens and confirmations are independent; the eleven-variant game/macro screen uses three pairs. Positive percentages below indicate elapsed/CPU savings, so negative values are losses. Do not combine cohorts into an aggregate improvement claim.

| finalizer-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| literal | -6.81% / -12.50% | -4.73% / 0.00% | -3.52% / -3.64% |
| defined | 0.56% / 0.00% | -0.70% / 0.00% | 1.75% / 1.64% |
| duplicate-forward | 0.00% / 0.00% | -1.52% / 0.00% | 0.47% / 0.46% |
| unique-forward | -0.54% / 0.00% | 0.36% / 11.11% | -0.07% / -0.10% |
| query-128 | -1.24% / 7.69% | -5.41% / 0.00% | -1.68% / -1.65% |
| query-8192 | 3.80% / 0.00% | -2.39% / 0.00% | -2.58% / -2.60% |
| scopes-128 | -2.59% / 0.00% | -1.00% / 0.00% | -3.38% / -5.08% |
| scopes-8192 | -0.58% / 0.00% | 5.68% / 0.00% | 0.20% / 0.13% |
| unique-forward-large | -3.35% / -4.41% | -6.75% / -6.49% | -8.39% / -8.38% |

| finalizer-confirm | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| duplicate-forward | -0.73% / 0.00% | -2.89% / -2.04% | -0.23% / 0.03% |
| query-128 | 0.57% / 0.00% | -2.65% / 0.00% | 0.44% / 0.48% |
| query-8192 | -1.49% / 0.00% | -2.16% / 0.00% | -0.92% / -0.79% |
| unique-forward-large | -3.44% / -4.48% | -6.21% / -8.00% | -6.20% / -6.21% |

| workloads-screen | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | -0.43% / 0.00% | 1.03% / 0.00% | -13.19% / -13.23% |
| Sonic 2 | -3.03% / 0.00% | -0.95% / -1.22% | 8.03% / 8.03% |
| Sonic 3 & Knuckles | -0.93% / 0.00% | -0.76% / -2.08% | -1.98% / -2.22% |
| Big’s Fishing Derby | 0.12% / 0.00% | -0.99% / -2.86% | -1.02% / -1.79% |
| Sonic Classic Heroes | -0.96% / 0.00% | 2.64% / -1.82% | -4.91% / -5.67% |
| Scaler v1 | 1.51% / 3.33% | -3.21% / -6.06% | 1.93% / 1.61% |
| Scaler v2 | -0.93% / -1.08% | -0.90% / -1.89% | -2.93% / -3.03% |
| Madness 4 release | -0.89% / 0.56% | 2.37% / -2.15% | -3.00% / -0.85% |
| Madness 4 debug | -1.40% / -1.06% | -0.18% / -0.50% | -0.13% / 0.39% |
| local-e | -0.09% / -1.43% | 0.29% / 0.00% | 1.76% / 0.49% |
| local-e debug | -1.00% / -1.35% | 2.87% / 2.50% | -5.97% / -1.21% |

The large distinct-name confirmation loses 3.44% elapsed with Windows GCC, 6.21% with MSVC and 6.20% with native GCC; zero, one and one of seven pairs favor the candidate, respectively. The game screen is mixed and is not independently confirmed: native Sonic 1 loses 13.19% elapsed / 13.23% CPU while Sonic 2 improves 8.03% / 8.03%. The repeatable synthetic loss already rejects the candidate; do not credit the isolated Sonic 2 result as a proven optimization. Every timing run succeeds with deterministic objects, generated-code equality across the compared binaries and normalized-listing equality within each compiler. Both local-e variants match their prepared comparison baseline; that baseline differs from the older aggregate golden.

### Bucket collisions and scope traversal

Diagnostic counters are confined to external source copies, collected after timing runs. Calibrate them with deliberately identical complete hashes and known successful, unsuccessful and empty-table probe counts. Read every diagnostic record, including records attached to assembler messages without a preceding newline. Compare diagnostic generated code with the ordinary binaries. Windows and native Linux counters agree exactly on every workload; no complete-hash collision occurs between distinct stored names in these datasets. Repeated insertion of the same name is not a collision. Collision averages below count extra probes through occupied buckets belonging to other names per insertion/name check, including duplicate scope requests but excluding table-growth rehash work. These workloads invoke no forward-reference resolution queries.

| Workload | Current extra probes / insertion | Finalizer extra probes / insertion |
| --- | ---: | ---: |
| Sonic 1 | 1.45 | 1.13 |
| Sonic 2 | 1.96 | 1.12 |
| Sonic 3 & Knuckles | 1.45 | 1.26 |
| Big’s Fishing Derby | 3.56 | 1.23 |
| Sonic Classic Heroes | 1.23 | 1.02 |
| Scaler v1 | 1.36 | 1.27 |
| Scaler v2 | 1.36 | 1.27 |
| Madness 4 release | 1.54 | 1.24 |
| Madness 4 debug | 1.55 | 1.25 |
| local-e | 1.67 | 1.16 |
| local-e debug | 1.66 | 1.16 |

The operation-weighted cohort average falls from 1.6657 to 1.1732 extra probes. Fewer collisions are established, but assembly-time improvement is not. Extra finalization work and changed table layout may offset the probe savings; their individual contributions have not been isolated.

| Workload | Most scopes for one name | Existing scope nodes examined / insertion | Longest scope walk | Names with more than 32 scopes |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 36 | 0.4567 | 10 | 1 |
| Sonic 2 | 2319 | 0.3973 | 27 | 10 |
| Sonic 3 & Knuckles | 128 | 0.3207 | 5 | 1 |
| Big’s Fishing Derby | 444 | 0.4887 | 10 | 1 |
| Sonic Classic Heroes | 3252 | 1.3439 | 98 | 14 |
| Scaler v1 | 1 | 0.0000 | 0 | 0 |
| Scaler v2 | 1 | 0.0000 | 0 | 0 |
| Madness 4 release | 48 | 0.4181 | 47 | 3 |
| Madness 4 debug | 50 | 0.4238 | 49 | 5 |
| local-e | 69 | 0.3652 | 21 | 2 |
| local-e debug | 69 | 0.3657 | 21 | 2 |

The scope measurements describe the unchanged ordered-list algorithm; name and scope counts also match the original hash-table diagnostic. Most game workloads examine fewer than 0.5 existing scope nodes per insertion. Classic Heroes is the exception at 1.3439 nodes, 23,655 walk steps over 28,890 insertions, a longest walk of 98 and a largest list of 3,252 records. Sonic 2 has a list of 2,319 records but a longest walk of 27, demonstrating why size alone is not a good promotion trigger. Both scalers contain only single-scope names. The earlier 8,192-scope stress case also benefits from increasing handles and does not by itself justify adding a per-name index.

A secondary index is therefore a possible future independent experiment, rather than part of this finalizer change. Prefer a lazily created integer-keyed scope hash table to a balanced tree: exact handle lookup requires no ordering, and balancing would reintroduce larger nodes and insertion bookkeeping. Preserve the list for cheap head insertion/current-handle hits and global/section iteration; consider promotion after repeated expensive walks, not merely after a name acquires many scopes. Benchmark Classic Heroes and deliberately out-of-order/revisited-handle cases against the current ordered list, count promotion/allocation behavior and memory, preserve duplicate/resolution state and pass cleanup, and check that small or steadily increasing scope sets do not slow. The present averages suggest that a universal secondary tree/table would mostly add overhead; no secondary scope index is implemented or claimed as a measured gain here.

External timing artifacts: `review-forward-finalizer-20261007/{finalizer-screen,finalizer-confirm,workloads-screen,candidate.patch}`. Baseline counters: `review-forward-collisions-20261007/{results.json,summary.json,calibration.json}`. Candidate collision/scope diagnostics: `review-forward-finalizer-collisions-20261008/{results.json,calibration.json,fwd_refs.c,fwd_hash_profile.h}`. Build sources, workload copies and raw logs stay outside the checkout; local-e remains anonymous.

## Main symbol-name hash experiments (2026-10-08)

The ordinary symbol tables and the new forward-reference name table both accumulate djb2, starting at 5381 and computing `hash * 33 + byte`. Evaluate their hashes separately: the main symbol table also performs frequent successful/missing symbol lookups and mixes the section/local-handle attribute into the name hash, whereas the previous forward-reference diagnostics mainly measured insertion. Keep full name/attribute equality, ownership and table algorithms unchanged. Start these independent trials from `28deb260`; its production sources match the frozen `review-forward-hash-ordered-20261007` baseline. None of the rejected forward-reference finalizer is retained.

The tables select an initial bucket with `hash & (capacity - 1)`. Since 33 is congruent to 1 modulo 32, djb2's lowest five bits depend on the sum of the input bytes, independently of their order; applying the existing scope XOR does not distinguish those equal-name-hash low bits within a scope. This motivates testing a different byte recurrence, but does not establish that a more elaborate hash will assemble faster. Candidate arithmetic remains unsigned and shared across Windows/Linux.

### djb2 XOR variant: rejected

Replace only the main table's per-byte addition with XOR, retaining the 5381 seed, multiplier 33 and final attribute mixing. This is a one-operation change rather than an added finalizer, and the forward-reference hash remains unchanged. All 327 tests pass independently on Windows GCC, Windows MSVC and native WSL GCC.

The initial strict-listing screen stopped at Madness 4 release because the printed symbol table follows bucket iteration order. The generated code matched, and an independent comparison established that the symbol-table record order was the only substantive listing difference. For these main-table hash trials, retain the established timestamp normalization, compare the assembly listing exactly, and canonicalize only the symbol-table section by sorting its printed records. Preserve every name, value and unused/segment flag, and check the parsed count against the printed symbol total. Calibration rejects changed values, changed usage flags and missing entries. Within-binary determinism is still required. This is an external experimental comparator, not a production listing change. The completed three-pair screen below uses that comparator; earlier stopped-screen samples remain saved separately and are not pooled with it.

Positive percentages indicate a reduction in elapsed/CPU time. One warmup precedes alternating before/after pairs; builds, diagnostic profiles and timing batches do not overlap. The full screen covers all eleven workload variants, checks deterministic objects and identical generated code across revisions/toolchains, and compares the normalized assembly listing plus symbol records. local-e and local-e debug already differ from the older aggregate golden in the accepted baseline; both candidates must match the actual current baseline, and no older-golden agreement is claimed for them.

| Workload | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 1.69% / 0.00% | -0.47% / 2.04% | 2.65% / 2.76% |
| Sonic 2 | 0.72% / 1.37% | 0.09% / 0.00% | 3.50% / 3.48% |
| Sonic 3 & Knuckles | 1.27% / 1.14% | 0.05% / 0.00% | -2.02% / -2.08% |
| Big’s Fishing Derby | 11.52% / 1.56% | 3.52% / -1.45% | 2.38% / 2.41% |
| Sonic Classic Heroes | -0.03% / 0.00% | -0.39% / -0.93% | 1.04% / 1.04% |
| Scaler v1 | 1.06% / 3.45% | -0.16% / 0.00% | 0.25% / 0.25% |
| Scaler v2 | 1.09% / 0.00% | -1.95% / -0.97% | 0.82% / 0.82% |
| Madness 4 release | -2.26% / 1.16% | -3.31% / 0.54% | 2.20% / 1.59% |
| Madness 4 debug | 1.16% / 0.56% | 1.17% / 0.00% | 2.44% / 2.82% |
| local-e | -0.54% / -1.44% | -0.52% / 1.31% | -0.97% / 0.27% |
| local-e debug | -0.42% / 0.70% | 5.27% / 0.65% | 0.23% / -0.13% |
| Geometric cohort mean | 1.45% / 0.78% | 0.32% / 0.11% | 1.15% / 1.21% |

Independently repeat five alternating pairs on Sonic 1, Sonic 2, Sonic 3 & Knuckles and Madness 4 release to check the initial native gains and possible regressions:

| Workload | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 2.13% / 0.00% | 1.94% / 2.00% | 0.62% / 0.63% |
| Sonic 2 | -0.34% / 0.00% | -0.92% / 0.00% | 1.15% / 1.19% |
| Sonic 3 & Knuckles | 1.52% / -1.14% | 1.19% / 1.03% | -2.34% / -2.33% |
| Madness 4 release | -0.61% / -0.57% | -0.10% / -1.64% | 9.15% / -0.69% |
| Geometric cohort mean | 0.68% / -0.43% | 0.53% / 0.36% | 2.24% / -0.29% |

Reject this variant. Native Sonic 3 & Knuckles slows 2.02% elapsed / 2.08% CPU in the screen and 2.34% / 2.33% in confirmation, with only one of five confirmation pairs faster. Native Sonic 1/2's initial 2.65%/3.50% elapsed gains shrink to 0.62%/1.15%. The confirmation's native Madness 4 release 9.15% elapsed gain accompanies a 0.69% CPU loss, so do not treat its elapsed-time swing or the confirmation cohort's elapsed average as a hashing speedup. Main-table XOR is removed before the next independent candidate; no production change is committed for it.

External artifacts: `review-symbol-djb2xor-20261008/{candidate.patch,workloads-screen,workloads-semantic-screen,workloads-confirm,workloads-semantic-screen-summary.json,workloads-confirm-summary.json}`. The strict-listing interruption is retained rather than relabeled as a completed benchmark. External tools `review-symbol-timings.py` and `review-symbol-batch.py` implement the checked symbol-record comparator. Sources, build directories and raw logs remain outside the checkout; local-e remains anonymous.

### FNV-1a 64-bit: rejected

Use the standard 64-bit [FNV-1a recurrence and constants](https://www.isthe.com/chongo/tech/comp/fnv/index.html): start at `UINT64_C(14695981039346656037)` and compute `(hash ^ byte) * UINT64_C(1099511628211)` per unsigned byte. Use a `uint64_t` accumulator, convert to `oh_size_t` on return, and retain exactly the existing attribute mixing and full name/attribute equality. This changes only the main symbol table; the forward-reference hash remains djb2. There are no CPU/OS/compiler-specific branches, unaligned loads, reads past the terminating NUL or additional dependencies. All 327 tests pass on each of Windows GCC, MSVC and native WSL GCC. The tested executables are 64-bit; no 32-bit timing claim is made.

The independent three-pair eleven-workload screen uses the same baseline, serialized execution and symbol-record-aware listing comparator as the XOR trial. All generated code and normalized assembly listings/symbol records match their baseline. Do not pool this candidate with XOR or the previous finalizer:

| Workload | Windows GCC elapsed / CPU | MSVC elapsed / CPU | Native GCC elapsed / CPU |
| --- | ---: | ---: | ---: |
| Sonic 1 | 2.26% / 4.35% | -2.88% / -2.08% | -7.36% / -7.36% |
| Sonic 2 | 0.78% / 1.39% | -1.37% / 0.00% | 1.28% / 1.28% |
| Sonic 3 & Knuckles | 1.09% / 0.00% | 0.34% / -1.06% | 3.06% / 3.17% |
| Big’s Fishing Derby | -0.68% / -1.67% | -4.85% / 0.00% | -0.08% / 0.07% |
| Sonic Classic Heroes | 0.99% / -1.04% | 2.63% / 1.89% | 0.25% / 1.00% |
| Scaler v1 | -2.45% / -3.70% | 0.61% / 3.23% | 5.40% / 5.37% |
| Scaler v2 | -1.24% / -2.35% | 0.68% / 0.00% | -3.13% / -3.08% |
| Madness 4 release | 0.65% / 0.60% | 2.16% / 2.23% | 0.68% / 1.36% |
| Madness 4 debug | 1.64% / 2.84% | -0.15% / 0.54% | -2.57% / -5.45% |
| local-e | -0.19% / 0.74% | -0.49% / 0.00% | -2.53% / -1.18% |
| local-e debug | -2.24% / 0.00% | 2.83% / 0.00% | -3.49% / -3.38% |
| Geometric cohort mean | 0.07% / 0.13% | -0.02% / 0.44% | -0.72% / -0.68% |

Windows elapsed changes are essentially neutral in the cohort, with CPU reductions below half a percent. The native screen contains stronger gains and losses, so repeat five alternating pairs independently on Sonic 1, Sonic 3 & Knuckles, both scalers and local-e debug before attributing them to the hash:

| Workload | Native GCC elapsed reduction | Native GCC CPU reduction | Faster pairs |
| --- | ---: | ---: | ---: |
| Sonic 1 | 0.04% | 0.04% | 2/5 |
| Sonic 3 & Knuckles | -0.66% | -0.66% | 2/5 |
| Scaler v1 | 1.37% | 1.23% | 2/5 |
| Scaler v2 | 1.95% | 1.97% | 3/5 |
| local-e debug | 2.06% | 1.11% | 3/5 |
| Geometric cohort mean | 0.96% | 0.74% | — |

Reject this candidate for lack of significant, dependable evidence of improvement, rather than claiming all screen regressions are established. Sonic 1's initial 7.36% elapsed / 7.36% CPU loss disappears in confirmation (0.04% / 0.04% reduction). Sonic 3 & Knuckles' initial 3.06% / 3.17% gain changes to a 0.66% / 0.66% loss; scaler v1's 5.40% / 5.37% gain shrinks to 1.37% / 1.23% with only two of five pairs faster. Scaler v2 and local-e debug switch from screen losses to modest confirmation gains, favoring only three of five pairs. This screen/repeat instability and the neutral Windows cohort do not establish a better replacement. Restore the original main-table djb2 implementation; no production hash change is committed.

Both hash candidates received 327 successful tests on all three toolchains. Saved Windows memory monitoring reports minimum available memory of 9,928 MiB for the completed XOR screen, 7,255 MiB for its confirmation and 9,289 MiB for the FNV screen. Each native batch records zero increases in memory-pressure stall totals, zero swap-in/out pages and zero OOM kills. Nonzero I/O pressure totals remain, including activity from synchronization outside the timed assemblies; these are whole-batch counters, not a measurement of stalls inside each assembly. Keep CPU and elapsed results separate and retain every sample rather than selecting favorable wall-time swings. These checks show no recurrence of the previously observed memory thrashing, but do not remove all timing variability.

These trials establish neither that djb2 is optimal nor that a hash with better dispersion would necessarily be faster. The previous collision measurements were for forward-reference names, not the main table; no main-table collision reduction or hashing-cost attribution is claimed here. Before heavier/block-oriented hash experiments, measure actual main-table lookup/insertion probe counts, key-length distribution and hashing work, keeping diagnostic overhead out of normal timing binaries. Any later recurrence/finalization change should again be isolated from caching, scope mixing and table load-factor changes. The forward-reference table is unchanged by these main-table experiments.

External artifacts: `review-symbol-fnv1a64-20261008/{candidate.patch,workloads-screen,workloads-screen-summary.json,native-confirm-summary.json}` and native `review-symbol-fnv1a64-20261008/native-confirm`. The saved candidate binaries, sources, build/test logs, raw samples, objects/listings and pressure counters remain available externally. Only this report is retained in the repository; the untracked review files and all private workload inputs are untouched.
