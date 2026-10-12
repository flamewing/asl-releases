# Sonic 1 GitHub Madness 4 and lazy hashing (2026-10-06)

## Workload and comparison boundaries

[Sonic 1 GitHub Madness 4](https://github.com/DeltaWooloo/Sonic-1-GitHub-Madness-4) is pinned to `0c23972ab787b0a657b69e15f6a38e021aee3c9f`. A pristine clone and disposable prepared input copies live outside the ASL checkout. The new [manifest](madness4-workloads.json) defines release and debug variants. Both generate listings, matching the upstream assembly flags. Default benchmark flags remain `-xx -n -q -A -U -i .`.

This measures ASL assembly only. The upstream Lua build subsequently performs ROM conversion, compression, symbol insertion and checksum/header work; those steps are excluded. Upstream bundled executables are not executed. The existing tracked assets suffice for these ASL assemblies.

For native Linux, change backslash path separators in the `incbin` file names in `ATOGKSpongeBob/MAIN.asm` and `conimodes/multiFC/GM_MultiFC.asm` in the prepared copy. Preserve macro delimiters such as `\name\` in `LiquidSplashes/Splashes.asm`: these are syntax, not paths. No filename case aliases are needed for this pinned revision. The original clone is unchanged. Windows and Linux objects have matching code records; platform-specific object footers make complete cross-platform file hashes differ.

Run one variant per output directory with `ab.py` or `run.py`: both manifest entries share a repository, and the runners otherwise try to copy into an already existing destination. Keep builds, inputs and reports outside the checkout, and finish preparation/builds before timing.

For example, after preparing the pinned clone as `<inputs>/madness4`:

```text
python benchmarks/ab.py --baseline <original-asl> --candidate <accepted-asl> --repos <inputs> --out <external-release-results> --manifest benchmarks/madness4-workloads.json --workloads madness4-release --runs 5
python benchmarks/ab.py --baseline <original-asl> --candidate <accepted-asl> --repos <inputs> --out <external-debug-results> --manifest benchmarks/madness4-workloads.json --workloads madness4-debug --runs 5
```

The existing runner's top-level `listing: false` field describes its default; this manifest explicitly supplies `-L`. Inspect each workload's recorded arguments for the actual listing configuration.

## Aggregate accepted optimizations

Compare `0730cdf`, before the optimization series, against accepted `a93a2f4`, including the bounded-prepend correctness fix. Fresh Git-archive builds use Release `-O3 -DNDEBUG`: MinGW GCC 16.2.0 on Windows NTFS, GCC 15.2.0 on WSL2 Ubuntu native ext4. The original revision passes all 202 tests and the accepted revision all 210 tests on each OS.

Each row has one warmup per binary and five alternating measured pairs. Positive reduction means faster. These are uninstrumented median assembly wall times.

| Platform / variant | Original | Accepted | Reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: |
| Windows release | 4.514 s | 3.170 s | 29.77% | 5/5 |
| Windows debug | 4.650 s | 3.297 s | 29.10% | 5/5 |
| Linux release | 2.179 s | 1.238 s | 43.20% | 5/5 |
| Linux debug | 2.016 s | 1.171 s | 41.91% | 4/5 |

Every same-platform original/accepted object hash agrees, and both variants' cross-platform code records agree. Minimum observed available Windows memory is 8,965 MiB. Do not interpret sums of variant medians as a measured complete ROM-build duration.

## Instrumented before/after profiles

Separate diagnostic binaries explicitly use `-O3 -DNDEBUG -g -fno-omit-frame-pointer -pg`, with `-pg` at link time. Windows diagnostics additionally disable dynamic-base and high-entropy VA relocation for gprof address resolution. Normal timed binaries retain normal linker behavior. Setting `ASL_PROFILE` alone does not instrument the original `0730cdf` snapshot, which lacks the later profiling CMake module; the accepted snapshot supports that option. An initial noninstrumented original-build attempt produced no `gmon.out` and was replaced by explicit-flag builds; no such attempt is used as a profile.

One assembly of each variant/revision/OS yields eight profiles. All eight objects match their corresponding normal-build objects. Stable calls per assembly agree on both OSes:

| Variant / function | Original calls | Accepted calls |
| --- | ---: | ---: |
| Release `EvalStrExpression` | 533,489 | 533,489 |
| Release `StrCompAlloc` | 4,669,503 | 43,873 |
| Release `ExpandLine` | 3,624,712 | 18,240 |
| Release `MakeList` | 1,063,720 | 1,063,720 |
| Debug `EvalStrExpression` | 550,017 | 550,017 |
| Debug `StrCompAlloc` | 4,833,419 | 53,445 |
| Debug `ExpandLine` | 3,745,524 | 18,244 |
| Debug `MakeList` | 1,139,224 | 1,139,224 |

The accepted batched `ExpandTokens` decoder has 310,060 release calls and 322,088 debug calls; its calls are not equivalent to old per-parameter scans. Native `MACRO_Processor` counts remain 548,836 and 600,096 respectively. Windows attribution of this label differs after optimization, so its counts are not used as evidence of equivalent logical work. Expression counts and identical output support that comparison.

In the accepted Linux release profile, sampled self-time includes formatting in `vsprcatf_core` (9.38%), `SearchTree` (6.25%), `EvalStrExpression` (4.69%) and `MakeList` (3.12%). Debug gives `EvalStrExpression` 12.12% and `MakeList` 3.03%. These are diagnostic sample shares, not production speedup estimates: sampling is coarse, inlining redistributes attribution, and runtime hooks are expensive. Exclude unresolved Linux `_init` and Windows `_mcount_private` / `__fentry__` from production bottleneck attribution. Remaining expression, symbol-search and listing/formatting work is a direction for investigation, not evidence that a particular hash cache will improve wall time.

## Lazy-cache trials

Both candidates start independently from `a93a2f4`, use separate pristine build directories, retain the existing generic hash getter, and add a separate prehashed getter. Full name equality and scope equality still resolve collisions. Inserts and resizing retain the existing full hash function.

### Lazy global-prefix hash: discarded

Cache the normalized global-label prefix, its byte length and rolling hash only when an actual dot-local lookup first needs it. Continue hashing the suffix from that state if the final normalized name has the exact cached prefix; otherwise hash the complete name. Ordinary and explicitly composed names use the existing lookup. Invalidate on prefix/reset and scope/handle changes, and guard case mode/codepage changes. This version conservatively invalidates on handle changes even when prefix bytes remain equal.

All 210 tests pass on both systems. An extracted-function differential harness passes 1,200,001 cases at each of 32 and 64 bits on each OS, including normalization and terminated truncation cases; native runs use AddressSanitizer. A generic prehashed-API expected-map test passes 38,400 cases per OS through collisions, resizing and deletion/reinsertion. Three independent-byte assembly fixtures pass on baseline/candidate on both systems in both case modes, covering dot/explicit names, nested sections, macro handles, prefix changes and forward references.

Five-pair screening covers the seven established workloads plus Madness 4 release and debug. Positive median reduction means faster.

| Workload | Windows reduction (wins) | Linux reduction (wins) |
| --- | ---: | ---: |
| Sonic 1 | 0.12% (2/5) | -1.16% (2/5) |
| Sonic 2 | -0.13% (2/5) | 0.93% (2/5) |
| Sonic & Knuckles | -0.12% (3/5) | -1.18% (0/5) |
| Big Fishing Derby | -0.57% (1/5) | -13.69% (1/5) |
| Sonic Classic Heroes | 0.11% (1/5) | 0.97% (3/5) |
| ScalerMacros-v1 | -1.09% (1/5) | 0.05% (3/5) |
| ScalerMacros-v2 | -0.24% (3/5) | -1.01% (1/5) |
| Madness 4 release | -0.12% (1/5) | 6.51% (5/5) |
| Madness 4 debug | -0.11% (3/5) | -2.81% (1/5) |

Windows changes are mostly neutral or slower. Linux release Madness 4 initially appears faster, but its ranges span 1.407–13.782 s for baseline and 1.350–5.463 s for candidate, making the percentage unreliable. Native counters show no swap traffic and abundant memory, but elevated I/O pressure; its cause is not established. An identical-binary release A/A batch also produces an apparent 1.9% median difference.

Focused nine-pair repeats reuse prepared workdirs without copying large asset trees and record child CPU time alongside wall time:

| Linux workload | Wall median reduction | CPU median reduction | Faster wall / CPU pairs |
| --- | ---: | ---: | ---: |
| Madness 4 release | -0.63% | 1.77% | 7/9; 6/9 |
| Madness 4 debug | -8.00% | -0.90% | 2/9; 2/9 |
| Big Fishing Derby | 13.50% | 2.28% | 6/9; 7/9 |

Median and paired estimates disagree under I/O variation; the apparent release win does not repeat as a useful overall benefit, and debug remains slower. Discard this version. This decision does not establish that all possible prefix-cache designs are ineffective.

### Lazy name-hash reuse across scope probes: discarded

Cache only the final normalized name's rolling hash inside one lookup. Compute it on the first nonempty table probe, then reuse it for parent section/local handles, recomputing the cheap handle mix for each probe. Each different name gets fresh state, so name and scope transitions cannot reuse stale full hashes. This is an independent candidate; no global-prefix cache is included.

The fresh candidate passes all 210 tests on each platform, the 38,400-case prehashed-API test on each platform (AddressSanitizer on Linux), and the three independent-byte fixtures on both revisions/OSes in both case modes. Actual name-hash and full-hash functions also agree on 1,500,005 name/handle checks at each of 32 and 64 bits, per OS, including high-byte names, empty/long names and signed handles. Native width checks use AddressSanitizer and UBSan. The old scope-hash trial's unexplained recursive-macro failure remains a separate unresolved observation; passing this candidate's suite does not explain it.

Timing reuses the existing prepared snapshots rather than copying large trees before each batch. Alternating runs record elapsed time and child CPU time (`GetProcessTimes` on Windows; `getrusage` on Linux). Native reports also record block I/O and major faults. Windows CPU-time quantization limits interpretation of small differences, especially on the shorter scaler workloads. Every run must succeed and reproduce the accepted object hash.

Five-pair screening:

| Workload | Windows reduction (wins) | Linux reduction (wins) |
| --- | ---: | ---: |
| Sonic 1 | -0.66% (3/5) | 1.81% (3/5) |
| Sonic 2 | -3.22% (1/5) | 1.11% (4/5) |
| Sonic & Knuckles | 0.70% (5/5) | 1.30% (5/5) |
| Big Fishing Derby | 0.27% (4/5) | 1.61% (3/5) |
| Sonic Classic Heroes | -0.36% (1/5) | 0.48% (4/5) |
| ScalerMacros-v1 | -0.76% (0/5) | 1.04% (4/5) |
| ScalerMacros-v2 | 1.41% (4/5) | -1.80% (1/5) |
| Madness 4 release | -0.87% (2/5) | 0.98% (3/5) |
| Madness 4 debug | 0.66% (3/5) | 2.38% (3/5) |

Nine-pair confirmation:

| Workload | Windows wall reduction (wins) | Linux wall reduction (wins) | Linux CPU reduction |
| --- | ---: | ---: | ---: |
| Sonic 1 | -0.70% (3/9) | 2.09% (6/9) | 2.13% |
| Sonic 2 | 0.49% (6/9) | -0.57% (5/9) | -0.56% |
| Sonic & Knuckles | -1.04% (5/9) | -0.40% (3/9) | -0.49% |
| Big Fishing Derby | -2.40% (4/9) | 0.86% (7/9) | 0.89% |
| Sonic Classic Heroes | -0.65% (4/9) | -1.23% (2/9) | -1.00% |
| ScalerMacros-v1 | 0.50% (3/9) | -0.21% (6/9) | -0.22% |
| ScalerMacros-v2 | -1.00% (1/9) | -0.34% (3/9) | -0.34% |
| Madness 4 release | 0.31% (5/9) | 6.04% (7/9) | -0.27% |
| Madness 4 debug | 0.65% (5/9) | -0.69% (4/9) | 1.29% |

Discard this version. Sonic & Knuckles changes from positive screening to negative confirmation on both OSes. Windows v2 is -1.00% with only 1/9 faster pairs; native v2 is slower in both batches. A possible modest Linux Sonic 1 benefit repeats, but Windows Sonic 1 is slower in both batches. This does not establish a useful overall tradeoff.

The apparent Linux Madness 4 release wall gain is not corroborated by CPU (-0.27% median reduction), and ranges overlap: 1.093–1.478 s versus 1.078–1.368 s. Debug wall time is slightly slower with 4/9 faster pairs. These observations do not establish a hash-cache speedup on this workload. No lazy hashing code or generic prehashed API is retained. Experimental source, binaries, tests and reports remain outside the checkout for future analysis. The accepted bounded-prepend fix remains in production.

The final audit verifies 14 paired report sets containing 29 matching cross-platform workload records and all eight profiling outputs. Executable hashes match the saved measured binaries; the production sources (allowing LF/CRLF conversion only) and ordinary external binaries match accepted `a93a2f4`. Final accepted CTest runs pass all 210 tests on both platforms. Audit and final test logs are preserved alongside the external results.

## External artifacts

Windows artifacts are under `D:/tmp/asl-bench-20261006/lazy-hash`; native ones under `/home/marzojr/asl-benchmark-20261005/lazy-hash`. The pinned original clone is `D:/tmp/asl-bench-20261006/madness4-upstream`. Build snapshots/logs are in `original`, `baseline`, `lazy-prefix`, `lazy-scope`, and the corresponding `*-profile` directories. Aggregate timings and hash verification are in `aggregate-madness4`; profiles are in `profiles-explicit`, with selected counts in the Windows `profile-summary.json`. Lazy trial reports are under `results/<candidate>`, with screening, focused repeats and executable hashes. External tools preserve transformations, adapter records and test harnesses. Private inputs, manifests, adapters and original paths are kept outside the checkout; only workload names are recorded in Markdown.
