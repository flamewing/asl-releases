# ASL optimization plan

This plan follows the [initial profiling results](RESULTS.md). Each experiment must preserve assembly behavior and demonstrate an improvement in optimized, uninstrumented builds on Windows and WSL2 Ubuntu. Linux measurements use native Linux storage; `/mnt/d` overhead is measured separately.

## Macro expansion — first implementation

### Skip replacements on constant macro lines

During `MACRO_OutProcessor`, collect the return counts from every `CompressLine` call. Store one flag per body line indicating whether any ordinary or special parameter was encoded. Include attributes, argument count, all arguments, and internal labels. Always execute every compression operation; an accumulated true flag must not short-circuit later calls.

Store flags on the macro definition, aligned with `FirstLine`. Copy them for global macro copies and release them with the definition. At invocation, use `LineZ` to read the flag. Copy every line as usual, but execute the existing replacement sequence only for flagged lines. Keep source positions, local scope, recursion, and end-of-macro processing unchanged.

This preserves replacement order on expandable lines. A line with no encoded tokens cannot receive replacement text through an initial substitution, so skipping the whole replacement block does not introduce the dependency issues of skipping individual parameters.

Validation: mixed constant/parameter lines; special-parameter-only lines; empty and nested macros; recursive invocation; globally copied definitions; metadata growth beyond its initial capacity. Run the complete CTest suite and compare Sonic 1, Sonic 2, and Sonic & Knuckles objects against the baseline. Use gprof call counts to measure avoided replacements, then use alternating baseline/candidate runs for wall-time measurements.

### Further macro experiments, separately measured

1. Specialize two-byte token searches: find candidate first bytes, then compare second bytes, retaining existing replacement order.
2. Investigate which parameters each line references, accounting for tokens that earlier replacements may introduce. Do not assume substitutions are independent.
3. Build replacement output into a reusable buffer to avoid repeated suffix movement; consider expanding all parameters in one pass only after proving equivalence with sequential replacement.
4. Maintain an invocation-local current-line pointer to avoid repeatedly walking from the linked-list head. Treat this as a separate experiment.

## Expression evaluation

`EvalStrExpression` currently allocates two 1024-byte scratch strings per call, including recursive calls, and attempts multiple interpretations of the input.

1. Measure allocation counts, expression lengths, recursion depths, and return paths in a diagnostic build.
2. Audit mutations and uses of `CopyComp` and `STempComp`; remove or delay unnecessary backup allocation/copying while preserving symbol spelling and source-position information.
3. Reuse scratch buffers by recursion depth. Active parent buffers and references into them must remain stable when child evaluations run or the scratch pool grows. Preserve dynamic growth for long inputs and clean up retained storage at the proper assembler lifecycle boundary.
4. Independently investigate conservative lexical checks before floating-point and string conversion. Keep ambiguous syntax on the original parsing path.
5. Re-profile before changing operator discovery. Avoid caching expression values across passes because symbols and the program counter can change.

Validation covers numeric formats, floats, strings, nested/user functions, temporary and forward symbols, relocations, long/deep expressions, and matching diagnostic positions. Count reduced work, but decide on uninstrumented timings.

## Quote-aware delimiter scanning

`QuotPosCore` checks for a delimiter at each character before rejecting matches inside quotes or brackets. `SearchMultString` repeatedly walks delimiter strings and calculates lengths. The common comment configuration is a single semicolon.

1. Avoid delimiter matching at protected positions while continuing every quote, escape, and bracket-state update. Preserve the existing order of matching and state transitions at eligible positions.
2. Specialize the single-character comment case; retain the general scanner for other CPU configurations.
3. If still useful, precompute delimiter lengths and candidate first bytes when configuration changes. Never retain stale metadata across CPU switches.
4. Keep the state machine unchanged initially; measure candidate matching separately from any later scanner redesign.

Validation covers escaped quotes, target-specific apostrophes, nested brackets, delimiters within strings, multi-character comments, and CPU switches.

## Acceptance and measurement

- Keep each optimization independent so improvements and regressions can be attributed to one change.
- Run all existing tests plus focused behavioral regression cases on Windows and Linux.
- Require identical objects to the same-platform baseline and identical code records across platforms, excluding only creator footers.
- Alternate baseline and candidate executions with the same inputs and flags; retain individual samples, medians, and ranges. Keep profiling runs separate.
- Use gprof counts/rankings to explain changes, not its instrumented wall times as speedup estimates. Treat small noisy differences as inconclusive.
- Record results below before moving to the next optimization.

## Implementation log

### Per-line macro replacement metadata — implemented 5 October 2026

The macro definition now owns a byte flag per stored body line. Capture uses the existing compression return counts, with amortized array growth. Global copies duplicate the flags; macro cleanup releases them. Invocation checks the flag using its existing line position and skips only the replacement block. The linked-list traversal and substitutions on flagged lines remain unchanged.

All 203 tests pass on Windows and Linux, including a new behavioral regression with mixed lines, special parameters, empty/recursive/nested macros, global copies, and metadata growth. Its manually specified expected bytes also pass with the original Windows baseline executable.

Nine alternating measured runs per executable, after one warmup each:

| Workload | Windows baseline → candidate | Time reduction | Linux native baseline → candidate | Time reduction |
| --- | --- | ---: | --- | ---: |
| Sonic 1 | 1.244 → 1.188 s | 4.5% | 0.542 → 0.505 s | 6.9% |
| Sonic 2 | 2.171 → 2.120 s | 2.4% | 1.077 → 1.037 s | 3.7% |
| Sonic & Knuckles | 2.002 → 1.948 s | 2.7% | 0.872 → 0.842 s | 3.5% |

Candidate executions are faster in 7/9, 6/9, and 6/9 Windows pairs and 9/9, 9/9, and 7/9 Linux pairs, respectively. These are observed medians rather than guaranteed speedups; Windows differences are modest with overlapping ranges. The paired comparisons are more relevant than comparing with timings from a different session. All baseline and candidate object hashes match across every warmup and measured execution.

Windows and Linux gprof agree on the reduction in replacement work. Counts below are normalized per assembly from Linux (baseline: seven collected runs; candidate: three); Windows used five baseline and three candidate runs:

| Workload | `ExpandLine` / `ReplaceLineUnchecked` before → after | Calls avoided |
| --- | --- | ---: |
| Sonic 1 | 1,479,714 → 938,122 | 541,592 (36.6%) |
| Sonic 2 | 2,535,532 → 1,697,698 | 837,834 (33.0%) |
| Sonic & Knuckles | 869,844 → 434,868 | 434,976 (50.0%) |

`MACRO_Processor` call counts are unchanged: 257,418, 427,026, and 232,456 per assembly. Every macro line is still processed; only unnecessary substitutions are removed. Instrumented timing is excluded from the speedup table.

Saved artifacts are under ignored `build-benchmark-results/`: `macro-windows-ab/`, `macro-linux-ab/`, `macro-windows-profile/`, and `macro-linux-profile/`, plus `macro-verification.json` for object/code-record and call-count checks. The Linux directories preserve exact executables and logs. Windows candidate executables are in `build-opt-macro-windows/` and `build-opt-macro-profile-windows/`. Native Linux build/input snapshots are at `/tmp/asl-macro-20261005` in Ubuntu. See `benchmarks/ab.py` and the benchmark guide for reproduction. Further macro changes, expression evaluation, and delimiter scanning remain planned, with no implementation in this experiment.
