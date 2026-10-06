# Register, opcode and symbol-hashing experiments — 6 October 2026

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

## Integer-register recognition — pending

## Opcode/name expansion — pending

## Name hashes across scope searches — pending

## Cached local-label prefix hash — pending
