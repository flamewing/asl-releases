# Expression and line-processing experiments — 6 October 2026

These experiments follow the [watcher-controlled retests](README.md#discarded-experiment-retests-2026-10-06). Test each change independently against the accepted executable, confirm promising results in a fresh batch, and commit only a favorable, meaningful improvement. Windows artifacts stay under `D:/tmp/asl-bench-20261006`; Linux uses native ext4 under `/home/marzojr/asl-benchmark-20261005/retests-20261006`.

## Queue

1. Initialize `EvalStrExpression` operand values only when their parsing branch needs them; clean up only initialized slots. Preserve recursive evaluation order, string and relocation ownership, and early-error cleanup.
2. Combine expression copying and whitespace trimming. Preserve bytes and diagnostic positions, including empty inputs, long inputs and fixed-buffer truncation semantics. Keep the existing expression backup copy.
3. Profile line input, splitting, code generation and listing handling separately in a diagnostic source snapshot. Use the result to choose one line-processing optimization, then evaluate it with normal uninstrumented builds.

Every trial uses Release `-O3`, `ASL_PROFILE=OFF`, all seven established workloads and the existing private adapters. One warmup per executable precedes five alternating screening pairs and nine fresh confirmation pairs. Builds, correctness tests and profiling finish before timing. Objects must match on each OS, and absolute-code records must agree across OSes. Profiling hook overhead and instrumented elapsed times are not speedup evidence.

## Lazy operand initialization — discarded

Baseline: `d383f77` (accepted production code includes `776e39b`). The trial replaced unconditional initialization and cleanup of three operand values with branch-local initialization and a count of initialized slots. Operators use two slots, user functions reuse one, and built-in functions initialize each argument slot before evaluating it. Literals and symbols use none.

All 209 tests pass on Windows and Linux. An independently specified fixture covers nested/repeated string functions, one/two/three operand paths, unary integer/float operations and early errors after string allocation. Baseline and candidate match its expected bytes on both OSes, and native AddressSanitizer passes with leak checking disabled for the full assembler fixture. All seven benchmark objects and cross-platform code records match.

The screen and full confirmation showed possible gains but noisy tradeoffs. A focused identical-executable control returned apparent reductions of +0.7% on Windows ScalerMacros-v1, -1.5% on Linux Sonic 2 and -1.0% on Linux Sonic Classic Heroes. A further fresh nine-pair targeted comparison gave Windows ScalerMacros-v1 about +2.0% (7/9 wins), ScalerMacros-v2 +1.1% (5/9); Linux Sonic 1 -2.4% (3/9), Sonic 2 +3.5% (5/9), and Sonic Classic Heroes +0.9% (5/9). Values are ratios of separate medians; the raw reports also retain paired ratios and ranges. Absolute times drifted upward during the full confirmation before returning toward the earlier range, so its large aggregate median differences were not treated as decisive.

Linux Sonic 2 repeatedly had a faster median, but the latest paired result was inconclusive, Sonic 1 was slower, and the larger Windows scaler's gain shrank. The implementation was restored rather than committed. This does not establish that every possible operand-lifetime optimization lacks benefit.

Artifacts: `experiments/operand-lazy`, `baselines/operand-lazy` and `results/operand-lazy/{screen,confirm,control,targeted}` in the external workspaces. Native sanitizer build: `operand-asan`. Exact baseline/candidate executables, source snapshots, fixtures, individual timings and output hashes are retained.

## Copy-and-trim fusion — discarded

Baseline: `d383f77`, with lazy operand initialization restored. The trial adds `StrCompCopyTrimmed` and uses it only for initial expression preparation. Dynamic buffers copy the trimmed span directly; fixed buffers retain the previous truncate-before-trimming behavior. The backup copy and expression parsing remain unchanged.

All 209 tests pass on both OSes. A source-linked differential harness passes 600,000 comparisons per OS for copied bytes, starting columns and lengths, including empty/all-whitespace inputs, long strings, fixed capacities 1/2/7/31 and dynamic self-copying. Native comparisons also pass under AddressSanitizer, including leak checking. All benchmark objects and cross-platform code records match.

Fresh nine-pair confirmation gave these median time reductions (positive is faster):

| Workload | Windows | Linux |
| --- | ---: | ---: |
| Sonic 1 | 0.96% | 1.09% |
| Sonic 2 | 1.35% | 1.50% |
| Sonic 3 & Knuckles | 0.89% | 1.60% |
| Big Fishing Derby | 0.98% | 0.65% |
| Sonic Classic Heroes | 1.41% | 1.50% |
| ScalerMacros-v1 | 2.41% | 0.87% |
| ScalerMacros-v2 | 0.90% | 1.60% |

The consistent positive direction suggests a possible small benefit, but ranges overlap throughout and most reductions are close to the observed identical-executable variation. Windows ScalerMacros-v1 won six of nine pairs; the initial Linux Sonic 1 screening gain of 9.3% did not repeat. This version was restored rather than committed because a meaningful gain was not established. Confirmation retained at least 11,310 MiB of available host memory (maximum load 65%).

Artifacts: `experiments/copy-trim`, `baselines/copy-trim` and `results/copy-trim/{screen,confirm}`; native sanitizer differential binary `copytrim-differential`.

## Line-stage attribution and known-length trimming — accepted

Fresh diagnostic snapshots of accepted `d383f77` ran each workload three times on both OSes. GCC out-of-line wrappers around input, splitting, code generation and listing handling, with sibling-call optimization disabled, preserve stage boundaries despite inlining. All 42 objects match accepted same-platform hashes, and all cross-platform code records agree.

Linux gprof attributed these inclusive sample shares to the stages. These are diagnostic attribution estimates, not production wall-time fractions: instrumentation, recursive call-graph allocation and unresolved `_init` attribution remain limitations. Windows profiling hooks dominate samples and some stage counts are unresolved, so its percentages are less useful.

| Workload | Input | Splitting | Code generation | Listing |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 2.2% | 11.6% | 68.9% | 0.0% |
| Sonic 2 | 4.3% | 10.9% | 67.4% | 1.7% |
| Sonic 3 & Knuckles | 2.5% | 9.4% | 71.4% | 0.3% |
| Big Fishing Derby | 0.1% | 11.8% | 61.9% | 3.6% |
| Sonic Classic Heroes | 3.4% | 7.5% | 66.1% | 0.0% |
| ScalerMacros-v1 | 4.4% | 19.5% | 55.9% | 1.2% |
| ScalerMacros-v2 | 2.1% | 26.0% | 52.3% | 1.2% |

For ScalerMacros-v2, splitting calls the trailing-whitespace helper 23,730,840 times across three assemblies. Argument copies already record their exact lengths. The third trial therefore trims those freshly copied components using the recorded lengths and reuses the trimmed argument-part length as its parsing end pointer. The general string-component trimming API remains unchanged; these local lengths are established by the immediately preceding copies.

All 209 tests pass on each OS. Both accepted and candidate executables match independently specified bytes for blank/comment lines, trailing spaces/tabs, quoted delimiters, empty/trailing macro arguments and a long dynamically copied argument. The full assembler also passes this focused fixture under native AddressSanitizer, with leak checking disabled. All screening and confirmation objects match, and all cross-platform code records agree.

Fresh nine-pair confirmation (median time reductions; faster pairs out of nine):

| Workload | Windows | Faster pairs | Linux | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.73% | 9 | 3.54% | 7 |
| Sonic 2 | 2.19% | 8 | 1.31% | 5 |
| Sonic 3 & Knuckles | 1.05% | 6 | -1.10% | 4 |
| Big Fishing Derby | 4.76% | 9 | 8.46% | 7 |
| Sonic Classic Heroes | 1.65% | 8 | -3.74% | 2 |
| ScalerMacros-v1 | 4.38% | 8 | -2.54% | 3 |
| ScalerMacros-v2 | 5.66% | 9 | 5.48% | 9 |

Windows Big Fishing Derby has non-overlapping confirmation ranges: baseline 0.982–1.090 s, candidate 0.940–0.961 s. ScalerMacros-v2 improves from 1.402 to 1.322 s on Windows and 1.112 to 1.051 s on Linux, winning every confirmation pair. Its screening reductions were 7.04% and 3.20%, respectively. Windows ScalerMacros-v1 repeats its 5.79% screening gain at 4.38%.

A further fresh nine-pair Linux recheck resolves the concerning confirmation slowdowns without changing either executable:

| Workload | Median reduction | Faster pairs |
| --- | ---: | ---: |
| Sonic 3 & Knuckles | -0.27% | 6/9 |
| Big Fishing Derby | 6.52% | 8/9 |
| Sonic Classic Heroes | 2.71% | 7/9 |
| ScalerMacros-v1 | 2.24% | 7/9 |
| ScalerMacros-v2 | 3.07% | 7/9 |

The Sonic Classic Heroes and ScalerMacros-v1 regressions did not repeat; Sonic 3 & Knuckles is inconclusive. Linux Big Fishing Derby's variable baseline (including a 1.083 s confirmation outlier) prevents treating its full-confirmation percentage as a precise speedup. The repeated large-scaler benefit on both OSes and clear Windows Big Fishing Derby improvement justify acceptance; small gains elsewhere remain uncertain. Screening and confirmation retained at least 11,274 MiB of host memory (maximum load 65%). The focused recheck also preserves accepted object hashes and the exact full-confirmation executable hashes.

Artifacts: diagnostic source/builds under `experiments/line-profile` (Windows) and `line-profile` (Linux), reports under `results/line-profile`; third-trial sources, executables and fixture under `experiments/line-trim-length`, `baselines/line-trim-length` and `results/line-trim-length/{screen,confirm,targeted}`. Targeted Linux reports reside in the native workspace.
