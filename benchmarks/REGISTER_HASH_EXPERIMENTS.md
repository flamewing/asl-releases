# Register, opcode and symbol-hashing experiments — 6 October 2026

**Windows timing correction:** the initial Windows incremental-build helper could reuse object files from discarded trials after restoring older source timestamps. Initial Windows timing tables below are provisional, including the first FPU acceptance evidence. Linux copies source files with new timestamps before building and is unaffected by this specific issue. Windows comparisons below were repeated with separate pristine source/build directories under `windows-isolated`; the final decisions use the corrected evidence at the end of this record. The shared helper now refreshes all experiment-managed Windows source/header timestamps before building.

Evaluate these independently, in order, against the latest accepted code. The initial baseline is `e0a21be`.

1. Reject non-`FP` names before FPU register recognition.
2. Reject impossible integer-register spellings without scanning entire names.
3. Reduce opcode/name expansion overhead when no interpolation is present.
4. Compute a symbol-name hash once for searches across local/section handles, then apply the existing handle mix separately.
5. Cache the name-hash state of the current global-label prefix and extend it for dot-local labels, preserving the exact composed-name hash.

Use the established seven workloads and private adapters. Names of private workloads may appear in this Markdown record; their inputs and adapters remain outside the checkout. Windows artifacts are under `D:/tmp/asl-bench-20261006`; Linux inputs/builds/results use the native ext4 workspace `/home/marzojr/asl-benchmark-20261005/retests-20261006`. No benchmark directories are added to the source tree.

Each trial runs all 209 correctness tests on both OSes before timing. Five alternating screening pairs follow one warmup per executable; promising results receive a fresh nine-pair confirmation. Builds, tests and heavy inspection finish before timing; platform batches run sequentially. Verify same-platform output hashes and cross-platform absolute-code records. Keep the exact baseline/candidate executables and source snapshots. Commit only significant, repeatable improvements; restore discarded code before starting the next trial.

## FPU prefix guard — accepted

The candidate rejects names whose first two characters cannot match `FP`, then retains the existing control/data-register recognition. Short-circuit evaluation protects empty and one-character inputs. All 209 tests pass on both platforms. An extracted old/new decoder comparison agrees on 600,736 cases per platform, covering register spellings, mixed case, random ASCII strings and tightly allocated short strings; Linux runs under AddressSanitizer. Compare classification and register value on success, since output values on failure are unspecified.

Artifacts: `experiments/fp-prefix`, `baselines/fp-prefix`, `results/fp-prefix` and native `fp-prefix-differential`.

Fresh nine-pair confirmation (positive median reduction is faster):

| Workload | Windows | Faster pairs | Linux | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 2.50% | 8/9 | 0.30% | 6/9 |
| Sonic 2 | 2.60% | 9/9 | 2.96% | 8/9 |
| Sonic 3 & Knuckles | 4.16% | 8/9 | 1.59% | 5/9 |
| Big Fishing Derby | 2.99% | 7/9 | 0.93% | 5/9 |
| Sonic Classic Heroes | 0.97% | 8/9 | 1.85% | 6/9 |
| ScalerMacros-v1 | 2.72% | 8/9 | 1.17% | 8/9 |
| ScalerMacros-v2 | 0.99% | 6/9 | -1.63% | 2/9 |

Windows Sonic 2 repeats its 2.66% screening reduction at 2.60%, with non-overlapping confirmation ranges (baseline 1.083–1.135 s; candidate 1.056–1.081 s). Linux Sonic 2 repeats 1.86% at 2.96%. A further fresh nine-pair Linux ScalerMacros-v2 recheck reduces its apparent slowdown to about 0.3%, so a material regression is not established. The focused recheck uses the same exact executables; its object agrees with full confirmation. All seven objects and cross-platform code records agree in both full batches. Minimum available host memory is 10,894 MiB, maximum load 66%. Accept for the repeated Sonic 2 benefit; do not treat the smaller/noisy percentages elsewhere as established gains.

## Integer-register recognition — accepted after pristine rebuilds

Baseline: `e3e78e4`, which commits the FPU guard above. Classify the first character before testing the short integer-register spelling. Retain the old `SP` comparison for `S` names; reject other names outside `A`/`D`, and check termination after the digit instead of calling `strlen`. All 209 tests pass on both platforms; the same old/new decoder harness agrees on 600,736 cases per OS, including native AddressSanitizer. Output parameters on failure remain unspecified and callers use register values only after successful classification.

Artifacts: `experiments/reg-shape`, `baselines/reg-shape`, `results/reg-shape` and native `reg-shape-differential`.

Fresh nine-pair confirmation:

| Workload | Windows | Faster pairs | Linux | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 2.91% | 5/9 | 5.46% | 8/9 |
| Sonic 2 | 2.28% | 8/9 | 1.55% | 6/9 |
| Sonic 3 & Knuckles | -0.54% | 5/9 | 2.60% | 8/9 |
| Big Fishing Derby | 1.45% | 6/9 | 5.32% | 4/9 |
| Sonic Classic Heroes | -1.04% | 5/9 | -3.08% | 3/9 |
| ScalerMacros-v1 | -0.77% | 4/9 | 2.93% | 6/9 |
| ScalerMacros-v2 | -0.54% | 4/9 | 1.33% | 7/9 |

A further fresh nine-pair check gives Windows Sonic 2 a 3.11% reduction from separate medians, but only 0.43% from the median of adjacent paired speedups, with 6/9 faster pairs and overlapping ranges. Linux Sonic 1 and Sonic 3 & Knuckles change to -0.74% (6/9) and -1.06% (4/9); Sonic Classic Heroes changes to +1.31% (6/9). The candidate may have a small benefit on Windows Sonic 2, but a significant repeatable overall improvement is not established. Restore this implementation. All full-batch objects and cross-platform code records match; targeted executable/output hashes also agree. Host memory remains at least 10,796 MiB available, maximum load 66%.

## Opcode/name expansion — discarded

Baseline: `e3e78e4`, with integer-register recognition restored. Add a direct bounded-copy path to `ExpandStrSymbol` when the name has no opening brace; retain interpolation, original opcode spelling and subsequent opcode uppercasing. All 209 tests pass on both platforms. A fixture with independently specified bytes passes on old and new executables on both OSes, covering literal/interpolated symbols, instruction names and macro calls.

Artifacts: `experiments/plain-name-copy`, `baselines/plain-name-copy`, `results/plain-name-copy`.

Fresh nine-pair confirmation:

| Workload | Windows | Faster pairs | Linux | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.56% | 4/9 | -1.45% | 6/9 |
| Sonic 2 | -0.85% | 2/9 | 0.30% | 4/9 |
| Sonic 3 & Knuckles | 0.05% | 6/9 | -1.66% | 4/9 |
| Big Fishing Derby | 3.41% | 7/9 | 1.69% | 6/9 |
| Sonic Classic Heroes | 1.00% | 4/9 | 1.49% | 7/9 |
| ScalerMacros-v1 | -1.44% | 4/9 | 0.52% | 6/9 |
| ScalerMacros-v2 | 1.36% | 5/9 | 1.76% | 6/9 |

The initial Windows Sonic 1 reduction of 8.09% does not repeat. Big Fishing Derby switches from -2.99% in Windows screening to +3.41% in confirmation, while its Linux confirmation has large timing interruptions (baseline maximum 1.213 s, candidate maximum 0.850 s). The modest scaler benefit and mixed full-project results do not establish a significant repeatable improvement. Restore this version. All objects and cross-platform code records match; host memory remains at least 10,826 MiB available, maximum load 66%.

## Name hashes across scope searches — discarded

Baseline: `e3e78e4`, with the two discarded candidates restored. Separate name hashing from the existing scope-identifier mix. Normalize/expand the lookup name as before, compute its name hash once per local/global lookup, and reuse it when searching successive handles. Add `openhash_get_hashed`; the existing `openhash_get` retains its behavior and skips hashing when the table has no buckets. Full name/scope equality and collision probing remain unchanged.

All 209 tests pass on both platforms. An independent expected-map check exercises 38,400 lookups per OS with deliberately clustered hashes, negative/positive scope identifiers, deletions, reinsertion and resizing, including AddressSanitizer on Linux. Baseline and candidate also match independently specified assembly bytes for nested section fallback, nested macro handles and forward references. The fixture was corrected to avoid duplicate definitions rejected by the baseline; it does not claim repeated-macro shadowing coverage.

Artifacts: `experiments/scope-name-hash`, `baselines/scope-name-hash`, `results/scope-name-hash` and native `scope-name-hash-check`.

The first five-pair screen is unfavorable overall: Windows reductions range from -3.14% to +3.23%, with five negative workload medians; Linux ranges from -9.79% to +9.83%, with four negative medians. All objects and cross-platform code records agree; minimum available memory is 10,800 MiB. Inspection identifies unnecessary eager name hashing before searches of tables with zero buckets, where the old getter skips hashing. Preserve this behavior in a revised independent trial, `scope-name-hash-empty`, still against `e3e78e4`. Both complete test suites and the expected-byte fixture pass for the revision. The generic hash-table API is unchanged from the checked version.

Revised five-pair screening:

| Workload | Windows | Faster pairs | Linux | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 6.56% | 4/5 | -13.81% | 1/5 |
| Sonic 2 | -1.75% | 0/5 | 0.71% | 3/5 |
| Sonic 3 & Knuckles | -0.36% | 3/5 | 5.21% | 4/5 |
| Big Fishing Derby | -0.11% | 4/5 | 11.63% | 3/5 |
| Sonic Classic Heroes | -1.05% | 2/5 | 1.69% | 1/5 |
| ScalerMacros-v1 | -2.67% | 2/5 | 7.62% | 4/5 |
| ScalerMacros-v2 | -2.55% | 1/5 | -0.42% | 3/5 |

The revised Windows Sonic 2 ranges do not overlap: baseline 1.102–1.150 s, candidate 1.151–1.215 s. Six Windows medians are negative. Large Linux percentages accompany overlapping ranges and uneven pair counts (including only 3/5 wins for Big Fishing Derby and 1/5 for Sonic Classic Heroes). This implementation has an unfavorable tradeoff; discard both variants without committing the generic API or running full confirmation. This does not prove that all possible prehashed lookup designs lack benefit. Revised objects and cross-platform code records agree; available memory stays above 10,711 MiB, maximum load 67%.

## Cached local-label prefix hash — discarded

Baseline: `e3e78e4`; the generic prehashed lookup API and scope-hash reuse are restored. Cache the current global-label prefix after the existing normalization, its length and its rolling hash state. On a matching name prefix, continue hashing only the remaining bytes; otherwise hash the whole name normally. Preserve the scope mix and full equality checks. Invalidate the cache at temporary-symbol reset and global-prefix changes; recompute when the case mode changes. Prefix matching is a byte comparison against the actual name, so extending the cached state reproduces the full composed-name hash.

All 209 tests pass on both platforms. An extracted-function differential harness agrees with an independent complete-name hash on 1,200,001 cases at each of 32 and 64 bits, per OS. It includes case-mode transitions, single-byte/UTF-8 normalization, prefix changes, short names, null/empty scope prefixes and explicitly terminated truncated names. Linux runs pass under AddressSanitizer. The assembly fixture matches independently specified bytes on baseline/candidate on both OSes in case-sensitive and case-insensitive modes, covering dot-local and explicit composed names, prefix changes, a long prefix and forward references.

Artifacts: `experiments/local-prefix-hash`, `baselines/local-prefix-hash`, `results/local-prefix-hash` and native `prefix-{32,64}-check`.

### Separate correctness finding

The initial sanitizer harness exposes a pre-existing `strmaxprep2` truncation issue: it copies `dest_len + 1` bytes after shortening `dest_len`, but that last copied byte need not be the source's NUL. For an eight-byte destination containing `.abc`, prepending `LONGNAME` produces eight nonzero bytes (`LONGNAM.`). A subsequent string scan can run beyond the buffer. The initial sanitizer run reports a stack-buffer-overflow during preparation/normalization, before cached hashing. No production fix is included in this performance series. Hash-equivalence checks explicitly terminate constructed truncated names so they test the hash helper's valid-string contract. This finding needs a separate bounded-prepend fix and regression test.

## Pristine Windows verification and revised register decision

Restored source timestamps allowed stale Windows objects to survive between trials. Separate Git-archive source/build directories fix this isolation problem; every fresh accepted/candidate build is tested before timing. Initial Windows tables above remain historical, provisional observations. Linux copies all managed sources with fresh timestamps and its initial records remain usable.

Fresh FPU confirmation retains the commit: Sonic 2 improves 1.82% (9/9 faster pairs, paired reduction 1.81%, non-overlapping ranges 1.080–1.094 versus 1.064–1.079 s); ScalerMacros-v1 improves 2.52% (8/9). All seven clean Windows confirmation medians are positive, ranging from 0.67% to 2.52%.

The clean integer-register screen is mixed. Nine-pair confirmation gives sub-1% full-game reductions and a stronger ScalerMacros-v1 result, 1.91% with 9/9 faster pairs. A focused scaler repeat gives Windows v1 only 0.33% (6/9; paired 1.00%) but Linux v1 3.99% (8/9; paired 2.68%). A further focused v1 repeat gives Windows 2.38% (8/9; paired 1.23%) and Linux 2.05% (7/9; paired 2.29%). Linux v2 improves 0.96% (7/9) in the focused repeat; Windows v2 improves 1.10% (6/9; paired only 0.05%). Accept the small repeated v1 benefit, strongest on Linux, without claiming significant full-game or v2 gains. The original large public-workload gains do not establish a repeatable improvement.

Clean register artifacts are `windows-isolated/reg-shape` and `results/windows-isolated/reg-shape/{screen,confirm,targeted,targeted-v1}`; native focused records are `results/reg-shape/{targeted-scalers,targeted-v1}`. All 209 fresh Windows tests pass. The earlier decoder differential and native sanitizer checks apply to the exact same register function. Subsequent candidates must be rebuilt against this newly accepted code.

## Name-copy and scope-hash retests against `f46eb3c`

The accepted register change requires new independent baselines. Rebase only that accepted decoder into each pristine Windows candidate directory; refresh all 15 managed native source/header files before Linux builds. Every rebased candidate passes all 209 tests on each OS. The saved Linux executables preserve executable permissions. An initial Linux launch with missing execute permission performed no timing; its artifact is retained separately.

Positive median reduction is faster; counts are adjacent faster pairs.

### plain-name-copy

| Workload | Windows reduction (wins) | Linux reduction (wins) |
| --- | ---: | ---: |
| Sonic 1 | -1.27% (3/9) | 1.42% (5/9) |
| Sonic 2 | 2.18% (6/9) | 1.75% (7/9) |
| Sonic 3 & Knuckles | 0.33% (5/9) | 1.65% (7/9) |
| Big Fishing Derby | 1.70% (8/9) | 0.76% (7/9) |
| Sonic Classic Heroes | 1.07% (6/9) | 1.82% (6/9) |
| ScalerMacros-v1 | 0.65% (7/9) | -0.15% (4/9) |
| ScalerMacros-v2 | 2.32% (7/9) | 2.12% (8/9) |

### scope-name-hash-empty

| Workload | Windows reduction (wins) | Linux reduction (wins) |
| --- | ---: | ---: |
| Sonic 1 | 0.65% (4/5) | 7.49% (3/5) |
| Sonic 2 | 2.60% (4/5) | 0.34% (3/5) |
| Sonic 3 & Knuckles | -0.27% (2/5) | 1.62% (4/5) |
| Big Fishing Derby | -0.14% (2/5) | -1.63% (0/5) |
| Sonic Classic Heroes | -0.22% (1/5) | -1.32% (1/5) |
| ScalerMacros-v1 | -2.32% (0/5) | -2.88% (1/5) |
| ScalerMacros-v2 | -1.78% (1/5) | 1.64% (4/5) |

The name-copy screen has four negative Windows medians, including v1 at -2.57% (1/5); confirmation changes most signs, but Sonic 1 is -1.27% (3/9). Linux v2 repeats a modest benefit, +2.37% (4/5) then +2.12% (8/9), while v1 becomes -0.15% (4/9). Discard this version because the overall platform/workload tradeoff is inconclusive, despite a possible Linux v2 benefit. All objects and cross-platform code records match in both full batches; minimum available memory is 9,254 MiB in confirmation.

Scope-hash reuse is slower on every private Windows workload. Linux is slower on three of the four private workloads. In particular v1 is -2.32% (0/5) on Windows and -2.88% (1/5) on Linux. The +7.49% Linux Sonic 1 median has only 3/5 faster pairs and overlapping ranges. Discard without full confirmation. All objects/code records match; minimum available memory is 9,213 MiB.

The first pristine scope-hash build against `e3e78e4` fails `t_maclines` with a recursive-macro-depth error at the nested macro invocation; rerunning alone fails, while the accepted executable passes the same source path. Rebuilding against `f46eb3c` passes the full Windows/native suites. The earlier failure is unresolved and is additional reason to keep this candidate out of production, rather than claiming a proven hash-mathematics defect. Its logs remain in `windows-isolated/scope-name-hash-empty/tests.log`; the later passing logs have the `-reg` suffix.

## Prefix-cache retest and final decisions

The independent prefix-cache rebuild against `f46eb3c` passes all 209 tests on both systems. Five-pair screening is slower on six of seven Windows workloads and all seven Linux workloads.

| Workload | Windows reduction (wins) | Linux reduction (wins) |
| --- | ---: | ---: |
| Sonic 1 | -1.08% (1/5) | -5.21% (2/5) |
| Sonic 2 | -1.39% (1/5) | -2.60% (1/5) |
| Sonic 3 & Knuckles | -1.87% (2/5) | -2.50% (0/5) |
| Big Fishing Derby | -0.17% (3/5) | -1.83% (0/5) |
| Sonic Classic Heroes | -0.73% (2/5) | -0.80% (1/5) |
| ScalerMacros-v1 | -1.88% (0/5) | -3.80% (0/5) |
| ScalerMacros-v2 | 0.61% (3/5) | -2.74% (1/5) |

Linux v1 has non-overlapping ranges, 0.334–0.343 versus 0.343–0.356 s, with no faster pairs. The cache preserves exact hash values, as the differential checks establish, but its management and prefix comparisons add an unfavorable cost in these workloads. Discard without further confirmation. All objects and cross-platform code records agree; minimum available memory is 9,233 MiB.

Retained code commits are `e3e78e4` (FPU prefix guard) and `f46eb3c` (integer-register spelling check). The latter has a modest repeated v1 scaler benefit, strongest on Linux; significant whole-game improvement is not established. The other three versions are removed from production. Separate pristine Windows directories correct the stale-object issue for these decisions. Existing earlier Windows incremental-build timing claims should also be treated as provisional until their build provenance is checked; this series does not retroactively validate them.

Exact accepted Windows/native binaries are restored to the external ordinary build locations. Source snapshots and final CTest/output/executable-hash audits accompany the corrected results. The shared Windows build helper refreshes managed source/header timestamps; future trials should prefer separate build directories or an explicit clean build rather than rely solely on source timestamp restoration. Private inputs, adapters and identities remain external; only workload names occur here.

Final audit: 18 corrected report pairs cover 59 matching cross-platform workload records, including the focused scaler repeats. All same-platform object hashes remain unchanged across the corrected series. Baseline executable hashes for the final three trials match the newly accepted register executable on each OS; restored ordinary-build executable hashes match those exact accepted binaries. All 15 managed source/header files agree between the checkout, native source snapshot and pristine accepted Windows snapshot. Final CTest runs pass all 209 cases on both systems. The machine-readable audit is `results/windows-isolated/final-five-audit.json` under the external Windows workspace.
