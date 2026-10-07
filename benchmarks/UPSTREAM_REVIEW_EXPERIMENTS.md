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
