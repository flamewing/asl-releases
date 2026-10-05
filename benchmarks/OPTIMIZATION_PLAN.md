# ASL optimization plan

This plan follows the [initial profiling results](RESULTS.md). Each experiment must preserve assembly behavior and demonstrate an improvement in optimized, uninstrumented builds on Windows and WSL2 Ubuntu. Linux measurements use native Linux storage; `/mnt/d` overhead is measured separately.

## Macro expansion — first implementation

### Skip replacements on constant macro lines

During `MACRO_OutProcessor`, collect the return counts from every `CompressLine` call. Store one flag per body line indicating whether any ordinary or special parameter was encoded. Include attributes, argument count, all arguments, and internal labels. Always execute every compression operation; an accumulated true flag must not short-circuit later calls.

Store flags on the macro definition, aligned with `FirstLine`. Copy them for global macro copies and release them with the definition. At invocation, use `LineZ` to read the flag. Copy every line as usual, but execute the existing replacement sequence only for flagged lines. Keep source positions, local scope, recursion, and end-of-macro processing unchanged.

This preserves replacement order on expandable lines. A line with no encoded tokens cannot receive replacement text through an initial substitution, so skipping the whole replacement block does not introduce the dependency issues of skipping individual parameters.

Validation: mixed constant/parameter lines; special-parameter-only lines; empty and nested macros; recursive invocation; globally copied definitions; metadata growth beyond its initial capacity. Run the complete CTest suite and compare Sonic 1, Sonic 2, and Sonic & Knuckles objects against the baseline. Use gprof call counts to measure avoided replacements, then use alternating baseline/candidate runs for wall-time measurements.

### Further macro experiments, separately measured

1. Specialize fixed-length token searches: find record separators, then compare payload bytes, retaining existing replacement order.
2. Investigate which parameters each line references, accounting for tokens that earlier replacements may introduce. Do not assume substitutions are independent.
3. Build replacement output into a reusable buffer to avoid repeated suffix movement; consider expanding all parameters in one pass only after proving equivalence with sequential replacement.
4. Maintain an invocation-local current-line pointer to avoid repeatedly walking from the linked-list head. Treat this as a separate experiment.

### Guarded single-pass macro and IRP expansion

`SetToken` now encodes parameter number `n` as three nonzero bytes: `$1E`, `(n / 29) + 1`, `(n % 29) + 1`. With `ArgCntMax=476`, all ordinary and special tokens use control bytes; neither payload byte contains `$1E`. Ordinary printable argument text, including another parameter's textual name, cannot introduce an encoded token. A direct token lookup can therefore replace repeated whole-line searches on the common path, while appending replacement text verbatim.

The previous two-byte encoding was not self-synchronizing: adjacent tokens 16 and 17 produced bytes `[2,1,2,2]`, whose middle pair `[1,2]` matched token 1. Actual assembly confirmed the erroneous replacement. This is a correctness bug, now fixed with the uniquely marked encoding below; single-pass expansion should preserve corrected behavior. A fallback may still be needed for replacement strings containing encoded control bytes.

1. Build an invocation-local indexed replacement table, including the enabled special parameters. Keep replacement pointers stable for the invocation and distinguish inactive tokens from empty replacements.
2. Compute fallback eligibility from invocation replacements: values containing `$1E` retain ordered substitution. Validate stored token payloads while decoding, preserving inactive tokens and malformed fragments. Adjacent valid tokens and empty substitutions without `$1E` no longer create overlapping matches; do not decode every control-byte pair indiscriminately.
3. Decode eligible lines directly into the existing reusable line output buffer, copying literal spans and inserting replacement values without rescanning them. Preserve the existing constant-line shortcut. Apply the decoder to `IRP_Processor` (including IRPN); `IRP_OutProcessor` continues capturing and compressing the body. Leave other `ExpandLine` callers unchanged initially.
4. Differentially test the decoder and fallback against the sequential implementation, including concatenation, parameters 16/17, empty values, control-bearing arguments, special parameters, defaults, nesting, recursion, and global copies. Run the full suite on both OSes.
5. Measure eligibility/fallback rates in a diagnostic build. Repeat alternating uninstrumented comparisons on the public and additional local workloads, verifying objects and cross-platform code records. Evaluate this independently of the linked-list cursor experiment.

### Lambda substitution path — future evaluation

The user-defined/lambda function path in `EvalStrExpression` evaluates each argument, formats its value into a parenthesized string, and calls `ExpandLine` for each formal parameter before recursively evaluating the expanded function expression. Investigate batching these replacements after measuring this path separately. Preserve argument evaluation order, relocation rejection, symbol flags, address-space masks, data-size promotion, parenthesization, and recursive evaluation behavior. A syntax-tree approach would be a separate, larger experiment; this implementation leaves the lambda path unchanged. If replacements are batched, retain each formatted argument separately: the current `stemp` buffer is reused for every argument, so caching pointers to it would make earlier replacement values invalid or overwrite them.

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

### Invocation-local macro line cursor — experiment 5 October 2026

`MACRO_Processor` now reads the current body node from the input tag's existing `LineRun` pointer and advances it once per returned line. `ExpandMacro` initializes that pointer for each invocation. Previously, line `n` required walking `n-1` links from the body head: processing an entire `L`-line body required `L*(L-1)/2` link traversals. The cursor requires `L` advances. The separate invocation-time line-count scan remains unchanged in this experiment.

The cursor belongs to the invocation rather than the shared macro definition, so recursive and nested calls resume independently. `LineZ` still indexes expansion flags and supplies diagnostic line numbers; parameter replacement, local scope, cleanup, and end-of-body behavior are unchanged. Empty macros never enter the processor. No additional allocation or input-tag field is needed.

`t_macrocursor` checks recursive returns, repeated invocations, nested IRP input, SHIFT, early EXITM, and empty bodies against independently specified bytes also produced by the head-walking baseline. All 207 tests pass on Windows and WSL2 Ubuntu. Every baseline/candidate object from the seven workloads matches, including warmups and all nine measured pairs; actual code records match across OSes.

The change is retained: ScalerMacros-v2 improves by 11.73% on Windows and 14.26% on Linux, winning all nine pairs on each OS with non-overlapping timing ranges. Other workloads show much smaller or inconclusive changes; no broad speedup is claimed. See the [full cursor results](README.md#macro-line-cursor-results-2026-10-05). These measurements isolate the cursor change from the already-applied single-pass and SHIFT fixes.

### SHIFT argument accounting — fixed 5 October 2026

The negative count was reachable through ordinary variadic calls. `ParCnt`
started at the formal parameter count, whereas the initial `ARGCOUNT` string
used the supplied count. After the first `SHIFT`, the string was rebuilt from
`ParCnt`, so a one-formal/six-argument call reported zero instead of five.
Subsequent shifts could make the count negative and stop expanding a formal
name even while excess arguments remained.

The input tag now stores the supplied argument count separately. The
replacement-list count includes excess positional arguments and decreases
with each removal; the supplied count decreases independently and saturates
at zero. Cached ordinary replacements are limited to declared formal slots,
so excess arguments do not enlarge the table unnecessarily. `ALLARGS` now
preserves leading empty arguments when rebuilding the remaining list.
Initial `ARGCOUNT` behavior for defaults and named calls is retained.
The manual now reflects that behavior instead of claiming defaults increase
the initial count. `t_shiftargs` covers excess and zero-formal calls, default
and named arguments, empty-list shifts, nested-loop shifts, and recursive
forwarding, including empty argument positions. The old executable fails the ordinary excess-argument cases;
the formerly negative-count case in `t_singlepass` now expects 2, 1, 0.

All 206 tests pass on Windows and WSL2 Ubuntu. The three public disassemblies
and four additional local workloads retain their previous full object hashes
on each OS, and their code records still match across platforms.

### Single-pass macro and IRP/IRPN substitution — implemented 5 October 2026

`ExpandTokens` scans record separators and decodes ordinary and enabled special tokens into the existing dynamic line output buffer. It copies literal spans and replacement values directly, with geometric buffer growth, avoiding repeated whole-line searches and suffix movement. Empty values remove tokens; inactive tokens and malformed fragments remain intact. Arguments containing `$1E` use the existing ordered replacement path.

Macro invocations lazily cache replacement pointers, lengths, and fallback eligibility. `SHIFT`, including one issued from an enclosing macro's nested loop, invalidates that cache. The initial version preserved legacy negative argument counts; those are corrected in the SHIFT accounting fix above. IRP/IRPN refresh the table once per iteration. Cleanup releases tables on normal completion and early exits. The existing constant macro line flags and macro linked-list traversal remain unchanged. `IRP_OutProcessor` continues compressing captured bodies; the shared decoder runs when `IRP_Processor` expands them. IRPC and lambda substitutions retain their existing `ExpandLine` path.

`t_singlepass` has independently specified expected output, also verified with the previous sequential executable. It covers literal argument names, named/default/empty arguments, special-token adjacency, `SHIFT`, nested IRP, IRPN tuple padding, buffer growth/reuse, inactive tokens, early macro exits, and separator-bearing arguments that introduce later tokens, including iteration-local fallback changes. The existing full-range token, recursion, global-copy, and lambda tests remain part of the complete suite.

All 205 tests pass on Windows and WSL2 Ubuntu. All seven benchmark workloads preserve their previous same-platform object hashes, including profiling builds, and code records agree across platforms. Nine alternating measured pairs show the clearest incremental gains on Sonic 1 (15.66% / 17.35% Windows/Linux) and Sonic 2 (12.14% / 12.33%). Additional local workload improvements range from 3.20–10.41% on Windows and 3.10–9.45% on Linux. Sonic & Knuckles is inconclusive on Windows (0.24%, only 3/9 faster pairs). See the [complete timing and candidate profile results](README.md#single-pass-substitution-results-2026-10-05).

### Unambiguous macro tokens — implemented 5 October 2026

An actual 17-parameter macro concatenating parameters 16 and 17 reproduced the overlapping-token bug: arguments `A,B` emitted bytes `$02,$58,$02` instead of `$41,$42`, because parameter 1's argument `X` replaced the middle pair. This was erroneous behavior, not a substitution dependency to preserve.

Tokens now contain ASCII record separator `$1E`, followed by two nonzero base-29 digits. Payload bytes are in 1..29 for every supported parameter, so `$1E` appears only at token starts. Both callers use four-byte buffers including the terminating NUL. A compile-time range check guards the encoding if the argument limit changes. The historical limit stays 476.

Full-range testing also exposed `MacroRec.ParamCount` truncating counts above 255 because it was stored in a byte. It now uses `int`, matching the count collected while reading the definition.

The new `t_mactokens` regression has independently specified expected bytes for the 16/17 overlap, all 476 ordinary parameter numbers, radix transitions, empty replacements, attributes, argument count, all arguments, and internal labels. The old executable emits the wrong bytes for the minimal example and fails the full-range regression. All 204 tests pass on Windows and WSL2 Ubuntu with the fix. Sonic 1, Sonic 2, Sonic & Knuckles, and all four additional local workloads retain their previous full object hashes on each OS; their code records match across platforms. These runs checked correctness and are not new performance measurements. This correctness fix is included in the baseline for the subsequent single-pass measurements.

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
