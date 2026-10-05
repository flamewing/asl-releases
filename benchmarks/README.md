# Profiling ASL on Windows and Linux

Use separate optimized timing and profiling builds. `ASL_PROFILE=OFF` (the default) keeps normal build flags. `SAMPLE` adds symbols and frame pointers for external sampling tools; MSVC uses `/Zi /DEBUG /INCREMENTAL:NO`. `GPROF` adds GCC -pg at compile and link time, including shared object libraries. It requires GCC, including MinGW GCC on Windows. The MinGW diagnostic build disables ASLR so gprof can resolve recorded addresses. OFF and SAMPLE retain normal linker behavior.

## Windows (PowerShell)

Adjust GCC and gprof paths for your MSYS2 installation:

```powershell
cmake -S . -B build-bench-windows -G Ninja -DCMAKE_C_COMPILER="$(cygpath -m $(which gcc))" -DCMAKE_BUILD_TYPE=Release -DASL_PROFILE=OFF
cmake --build build-bench-windows --target asl p2bin test_driver -j 8
ctest --test-dir build-bench-windows -j 8 --output-on-failure
python benchmarks/run.py --asl build-bench-windows/asl.exe --repos path/to/sonic/disassemblies --out build-benchmark-results/baseline-new --prepare --runs 7

cmake -S . -B build-profile-windows -G Ninja -DCMAKE_C_COMPILER="$(cygpath -m $(which gcc))" -DCMAKE_BUILD_TYPE=Release -DASL_PROFILE=GPROF
cmake --build build-profile-windows --target asl -j 8
python benchmarks/run.py --asl build-profile-windows/asl.exe --repos build-benchmark-results/baseline-new --out build-benchmark-results/profile-new --gprof "$(cygpath -m $(which gprof))" --runs 7 --warmups 0
```

The runner requires Python 3.6+. Each output directory must be new. It copies selected repositories, excluding `.git`, and assembles from the copies. `--prepare` generates PCM/DPCM assets with the bundled Lua on Windows (or `--lua /path/to/lua` elsewhere). For Sonic 2 it also reuses the music-generation functions from `build.lua`, invoking the repository's existing tool bundle. Preparation runs outside the timed region; no dependencies are installed. Missing tools cause preparation to fail with a log. Pass a prepared output directory as `--repos` on subsequent runs.

Only the main ASL subprocess is timed with a monotonic clock. Copying, conversion, hashing, and report generation are excluded. Startup, file access, assembly passes, and object writing are included. Output is captured instead of displayed. Default arguments are `-xx -n -q -A -U -i .`; SK additionally defines `Sonic3_Complete=0`. Add `--listing` for the repository scripts' -L. These measurements cover ASL, not complete compressed-ROM builds.

`workloads.json` defines Sonic 1 (`s1disasm_as/sonic.asm`), Sonic 2 (`s2disasm/s2.asm`), and Sonic & Knuckles (`skdisasm/sonic3k.asm`). Select a subset with `--workloads s1 sk`. The runner saves logs, commands, individual wall times, median/range, executable SHA-256, and object SHA-256. It rejects failed processes, timeouts, missing output, inconsistent objects, and empty profiles. gprof aggregates measured runs into `s1-gprof.txt`, `s2-gprof.txt`, and `sk-gprof.txt`; individual raw `.gmon` files are retained.

## WSL2 Ubuntu

Use native Linux storage, such as your home directory or `/tmp`. Shared `/mnt/d` storage can dominate runtime. Example with a fresh temporary workspace:

```bash
src=path/to/asl/source
task=$(mktemp -d /tmp/asl-profile.XXXXXX)
mkdir "$task/source"
rsync -a --exclude='/build*' --exclude='/.git' --exclude='/.vs' --exclude='/.cache' --exclude='/Testing' "$src/" "$task/source/"
cmake -S "$task/source" -B "$task/baseline" -G Ninja -DCMAKE_BUILD_TYPE=Release -DASL_PROFILE=OFF
cmake --build "$task/baseline" --target asl p2bin test_driver -j 8
ctest --test-dir "$task/baseline" -j 8 --output-on-failure

# Reuse assets prepared on Windows; no Ubuntu Lua needed.
python3 "$src/benchmarks/run.py" --asl "$task/baseline/asl" --repos "$src/build-benchmark-results/baseline-new" --out "$task/linux-native" --runs 7
cmake -S "$task/source" -B "$task/gprof" -G Ninja -DCMAKE_BUILD_TYPE=Release -DASL_PROFILE=GPROF
cmake --build "$task/gprof" --target asl -j 8
python3 "$src/benchmarks/run.py" --asl "$task/gprof/asl" --repos "$task/linux-native" --out "$task/linux-profile" --gprof /usr/bin/gprof --runs 7 --warmups 0
```

To measure shared-filesystem cost, repeat with the same baseline executable and prepared inputs, setting `--out` to a fresh directory under `$src/build-benchmark-results`. Run timing builds sequentially on an idle machine. Warmups measure warm caches; this runner does not flush caches or claim cold-I/O performance. Record compiler versions and storage locations.

## Interpretation and output checks

gprof instrumentation changes runtime substantially, especially for tiny, frequently called functions. Use call counts and self-time rankings to identify candidates; validate changes with the uninstrumented build. Shared-library time is incomplete, inlining affects attribution, and inclusive times are estimates. See the [GNU gprof manual](https://sourceware.org/binutils/docs/gprof.html). Windows `_mcount_private`/`__fentry__` are instrumentation overhead. Linux `_init` attribution is not evidence that startup is the bottleneck.

Objects must match byte-for-byte between baseline and profiling builds on each platform. Windows/Linux `.p` files have different creator footers. Compare their actual records with:

```bash
python3 benchmarks/compare.py path/to/windows/sonic.p path/to/linux/sonic.p
```

This parses absolute-code records and ignores only the final creator string, rejecting malformed or unsupported records. It does not verify compressed ROMs against original game hashes. Repeat CTest and object comparisons after each optimization; retain only repeatable uninstrumented improvements.

See [the initial findings](RESULTS.md) for this machine's measurements.

## Comparing an optimization

The [optimization plan](OPTIMIZATION_PLAN.md) records the experiments and their acceptance criteria. `ab.py` runs a baseline and candidate alternately on the same prepared snapshot, reversing their order each pair. Each executable gets one warmup followed by nine measured runs by default. All object hashes must match across both executables and every repetition.

```powershell
python benchmarks/ab.py --baseline build-bench-windows/asl.exe --candidate build-opt-macro-windows/asl.exe --repos build-benchmark-results/windows-final --out build-benchmark-results/macro-comparison-new --runs 9
```

On Ubuntu, use `python3`, Linux executables, and an output directory on native Linux storage. The tool records each execution order, both sample sets, executable hashes, median time reduction, and paired speedup ratios. Inputs must already contain generated assets; the output directory must be new. Run timing comparisons separately from compiler builds, tests, and instrumented profiling.

Both runners accept `--manifest /path/to/local/workloads.json` for additional local workloads, selected explicitly with `--workloads`. The manifest uses the same schema as `workloads.json`: a label maps to a staged repository directory, an assembly entry point, and an argument list. The built-in public workload definitions remain the default. Keep private manifests, input snapshots, wrappers, logs, and identity mappings under an ignored local build directory; include private workload names in documentation only with the owner's approval.

## Additional local workload results (2026-10-05)

Four private workloads were tested against the baseline and the macro line replacement optimization. A (Big Fishing Derby) and B (Sonic Classic Heroes) assemble full project entry points; C and D exercise standalone macro scaler generators through assembly wrappers.

These are uninstrumented Release builds (-O3), using MinGW GCC 16.2 on Windows and GCC 15.2 on WSL2 Ubuntu. Windows inputs use local NTFS; Linux inputs and outputs use native Linux storage. Each comparison used one warmup per executable followed by nine alternating measured pairs, reversing order each pair. Timing runs were separate from profiling and compiler builds. Times cover the ASL subprocess, without listings or ROM postprocessing.

| Workload | Windows baseline → optimized (s) | Time reduction | Linux baseline → optimized (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| A: Big Fishing Derby | 1.800 → 1.701 | 5.52% | 0.996 → 0.915 | 8.15% |
| B: Sonic Classic Heroes | 2.402 → 2.281 | 5.04% | 1.026 → 0.975 | 4.94% |
| C: ScalerMacros-v1.asm | 1.211 → 0.950 | 21.53% | 0.914 → 0.722 | 21.03% |
| D: ScalerMacros-v2.asm | 3.900 → 3.383 | 13.25% | 3.010 → 2.613 | 13.18% |

Values are medians. The optimized build won 8/9 Windows pairs for A and B, 9/9 Linux pairs for A, 8/9 Linux pairs for B, and every pair for C and D on both platforms. These measurements describe the staged inputs on this machine, rather than a guaranteed improvement for every macro workload.

Separate gprof builds ran each workload three times per binary and platform. The following call counts are per assembly, calculated from the aggregate profiles. Windows and Linux counts agree exactly. `ExpandLine` and `ReplaceLineUnchecked` each have the counts shown:

| Workload | Baseline calls | Optimized calls | Calls avoided |
| --- | ---: | ---: | ---: |
| A: Big Fishing Derby | 1,749,395 | 820,991 | 53.07% |
| B: Sonic Classic Heroes | 1,326,118 | 692,550 | 47.78% |
| C: ScalerMacros-v1.asm | 2,589,370 | 725,544 | 71.98% |
| D: ScalerMacros-v2.asm | 6,140,822 | 968,018 | 84.24% |

`MACRO_Processor`, `EvalStrExpression`, and `QuotSMultPosQualify` call counts are unchanged between the two versions on all four workloads. All warmup and measured objects match between baseline and optimized builds on each platform; the instrumented builds produce the same objects too. Actual code records also match between Windows and Linux, ignoring only the platform-specific creator footer.

The standalone adapters exercise generation in both inputs. One adapter omits a consumer example whose PC-relative references exceed the CPU's addressing range, while retaining its tables and generator example. The other explicitly invokes a generator whose supplied example is disabled. One staged Linux project needs an include-directory case alias to reproduce Windows path resolution. Original sources were not modified. These tests verify assembly output equivalence, not complete project build pipelines.

After the optimization, Linux gprof attributes 20.78% of Local D self-time to `MACRO_Processor` and 10.66% to `QuotSMultPosQualify`; Local C attributes 10.87% to each. This supports testing the planned invocation-local macro line cursor and delimiter scanning experiments next. The profile does not isolate linked-list traversal within `MACRO_Processor`, so that attribution must be checked with an isolated change and uninstrumented timings. Expression evaluation remains prominent in A and B (12.50% and 10.33% Linux self-time respectively). Instrumentation overhead and sampling uncertainty still apply; profile times are not used for the improvements reported above.

## Single-pass substitution results (2026-10-05)

Macro and IRP/IRPN expansion now decode all active parameter tokens in one pass, copying literal spans and argument values directly into the line output buffer. Replacement tables cache pointers and lengths per invocation or loop iteration; `SHIFT` invalidates the macro table. Arguments containing the record separator retain ordered substitution. IRPC and lambda substitutions keep their existing path; see the [lambda evaluation note](OPTIMIZATION_PLAN.md#lambda-substitution-path--future-evaluation).

Both baseline and candidate include the constant-line shortcut and the three-byte token correctness fix. These measurements isolate the additional single-pass change. Release -O3 builds use MinGW GCC 16.2 on Windows and GCC 15.2 on WSL2 Ubuntu, with local NTFS and native Linux storage respectively. Each executable had one warmup and nine measured runs in alternating pairs, reversing order each pair. Windows and Linux timing batches ran sequentially, separately from builds, tests, and instrumented profiling. Local adapters and measurement scope are the same as described above.

| Workload | Windows baseline → optimized (s) | Time reduction | Linux baseline → optimized (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.345 → 1.135 | 15.66% | 0.559 → 0.462 | 17.35% |
| Sonic 2 | 2.359 → 2.072 | 12.14% | 1.127 → 0.988 | 12.33% |
| Sonic & Knuckles | 2.258 → 2.252 | 0.24% | 0.903 → 0.881 | 2.42% |
| Big Fishing Derby | 1.921 → 1.848 | 3.82% | 0.941 → 0.873 | 7.18% |
| Sonic Classic Heroes | 2.570 → 2.303 | 10.41% | 1.003 → 0.941 | 6.15% |
| ScalerMacros-v1.asm | 0.956 → 0.910 | 4.74% | 0.730 → 0.661 | 9.45% |
| ScalerMacros-v2.asm | 3.349 → 3.242 | 3.20% | 2.655 → 2.573 | 3.10% |

Values are medians. In table order, the candidate won 9/9, 8/9, 3/9, 8/9, 8/9, 8/9, and 9/9 Windows pairs; Linux counts were 9/9, 9/9, 7/9, 6/9, 9/9, 9/9, and 7/9. Sonic & Knuckles is inconclusive on Windows: its negligible median change and paired results do not demonstrate a gain. The Linux change there is modest too. Other gains remain measurements on this machine, rather than guarantees for every input.

Separate candidate gprof builds collected three runs per workload and OS. These per-assembly counts agree exactly between Windows and Linux:

| Workload | `ExpandTokens` calls | Remaining `ExpandLine` / `ReplaceLineUnchecked` calls |
| --- | ---: | ---: |
| Sonic 1 | 129,136 | 36,828 |
| Sonic 2 | 237,026 | 84,448 |
| Sonic & Knuckles | 102,140 | 44,784 |
| Big Fishing Derby | 216,534 | 64,027 |
| Sonic Classic Heroes | 167,972 | 124,314 |
| ScalerMacros-v1.asm | 241,454 | 540 |
| ScalerMacros-v2.asm | 286,446 | 540 |

The Linux call graphs attribute the remaining 540 legacy replacements in each standalone generator to `EvalStrExpression`. Their sampled cost is small. The larger generator still spends 19.51% of Linux gprof self-time in `MACRO_Processor`, supporting a separate current-line cursor experiment. Windows/Linux gprof processor-wrapper totals differ in these builds and are not used for cross-platform count comparisons; decoder, legacy replacement, and expression-evaluation totals agree. Instrumented wall times are not used for the reductions above.

All 205 tests pass on both OSes, including the new behavioral regression checked against the sequential executable. Every baseline/candidate warmup and measured object matches; profiling objects also match the timing objects. All seven workloads retain their previous same-platform object hashes, and actual code records match between Windows and Linux, ignoring only creator footers. Raw reports, snapshots, and private adapters remain in ignored local directories.

## Macro line cursor results (2026-10-05)

`MACRO_Processor` now advances the invocation's existing `LineRun` pointer once per body line. Previously it walked from the shared body head to the current line on every call, requiring `L*(L-1)/2` link traversals for a complete `L`-line invocation. Each recursive or nested call owns its cursor. The invocation-time line-count scan remains unchanged.

These measurements isolate the cursor change: baseline and candidate both include single-pass substitution, the constant-line shortcut, unambiguous tokens, and the SHIFT accounting fix. Uninstrumented Release -O3 builds use MinGW GCC 16.2 on Windows and GCC 15.2 on WSL2 Ubuntu. Windows uses local NTFS; Linux inputs and outputs use native Linux storage. Each binary had one warmup followed by nine alternating measured pairs, with execution order reversed each pair. OS timing batches ran sequentially and separately from builds and tests. Local adapters and assembly scope are unchanged from the earlier comparisons.

| Workload | Windows baseline → cursor (s) | Time reduction | Linux baseline → cursor (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.180 → 1.158 | 1.80% | 0.427 → 0.425 | 0.41% |
| Sonic 2 | 1.866 → 1.834 | 1.76% | 0.928 → 0.922 | 0.60% |
| Sonic & Knuckles | 1.953 → 1.973 | -1.04% | 0.823 → 0.820 | 0.37% |
| Big Fishing Derby | 1.678 → 1.666 | 0.71% | 0.828 → 0.826 | 0.26% |
| Sonic Classic Heroes | 2.249 → 2.237 | 0.50% | 0.890 → 0.883 | 0.75% |
| ScalerMacros-v1.asm | 0.901 → 0.897 | 0.43% | 0.633 → 0.613 | 3.19% |
| ScalerMacros-v2.asm | 3.121 → 2.755 | 11.73% | 2.408 → 2.064 | 14.26% |

Values are medians. In table order, the cursor won 3/9, 8/9, 3/9, 5/9, 6/9, 5/9, and 9/9 Windows pairs; Linux counts were 5/9, 5/9, 5/9, 5/9, 5/9, 7/9, and 9/9. The clear repeatable improvement is ScalerMacros-v2: Windows baseline samples span 3.063–3.204 s versus 2.669–2.913 s with the cursor; Linux spans 2.333–2.489 s versus 2.028–2.115 s. These ranges do not overlap. The v1 Linux result is smaller, while its Windows result is inconclusive. The other small changes, including the negative Sonic & Knuckles Windows median, do not establish a broad gain or regression. Sonic 1's positive Windows median with only three faster pairs illustrates why median reduction alone is insufficient.

The cursor is retained for its consistent benefit on the larger scaler and removal of repeated traversal without additional allocation or input-tag storage. All 207 tests pass on both platforms. The new `t_macrocursor` regression covers recursive returns, repeated invocations, nested IRP input, SHIFT, early exits, and empty macros; the previous head-walking executable also matches its independently specified expected bytes. Every warmup and measured object matches between baseline and cursor on each OS, and actual code records match across platforms, ignoring only creator footers. Raw samples, executable hashes, logs, snapshots, verification results, and private adapters remain in ignored local directories. This experiment uses wall-time comparisons; no new instrumented profile is claimed.

## Further linked-list experiments (2026-10-05)

Three follow-up candidates were evaluated independently, in the requested order. Each used the committed macro-line-cursor version as its baseline because neither preceding candidate was accepted. These are uninstrumented Release -O3 builds with MinGW GCC 16.2 on Windows/local NTFS and GCC 15.2 on WSL2 Ubuntu/native Linux storage. Each screen used one warmup per binary and five alternating measured pairs, reversing order each pair. Windows and Linux batches ran sequentially, separately from builds and tests. Workloads, adapters and assembly flags retain the scope described above. Positive percentages mean less time; negative percentages mean more time. Absolute times are medians in seconds, rounded independently of the percentage calculation.

### IRP/IRPN argument cursor: discarded

The candidate advanced an invocation-local argument pointer by one element for IRP or by the tuple width for IRPN, replacing the walk from the list head at each iteration. Diagnostics retained the original list. All 208 candidate tests passed on both OSes; the extra regression covered empty arguments, tuple padding, nested loops and repeated invocations, with independently specified output also verified against the baseline.

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.174 → 1.194 | -1.71% | 0.468 → 0.451 | 3.52% |
| Sonic 2 | 2.053 → 2.023 | 1.44% | 0.972 → 0.959 | 1.34% |
| Sonic & Knuckles | 2.094 → 2.112 | -0.87% | 0.867 → 0.872 | -0.59% |
| Big Fishing Derby | 1.761 → 1.798 | -2.10% | 0.863 → 0.882 | -2.12% |
| Sonic Classic Heroes | 2.565 → 2.450 | 4.49% | 0.913 → 0.932 | -2.17% |
| ScalerMacros-v1.asm | 0.993 → 0.971 | 2.23% | 0.626 → 0.651 | -4.05% |
| ScalerMacros-v2.asm | 2.992 → 3.190 | -6.60% | 2.150 → 2.195 | -2.09% |

In table order, the candidate won 3/5, 5/5, 1/5, 2/5, 4/5, 3/5 and 1/5 Windows pairs; Linux counts were 3/5, 3/5, 3/5, 1/5, 0/5, 0/5 and 1/5. Results did not establish a consistent useful gain across the available inputs. The implementation and candidate-only test were discarded without a commit.

### Tail pointers for body capture and copying: discarded

An append-with-tail helper left the existing list API available for other callers. Macro definitions and loop capture tags retained final nodes, making body capture appends constant-time for macros, IRP/IRPC, REPT and WHILE. Generic string-list duplication and global macro-body copies became linear. All 207 tests passed on both platforms.

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.175 → 1.164 | 0.99% | 0.472 → 0.485 | -2.76% |
| Sonic 2 | 2.065 → 2.075 | -0.50% | 0.992 → 1.014 | -2.17% |
| Sonic & Knuckles | 2.131 → 2.114 | 0.83% | 0.896 → 0.897 | -0.09% |
| Big Fishing Derby | 1.742 → 1.771 | -1.64% | 0.954 → 0.999 | -4.71% |
| Sonic Classic Heroes | 2.420 → 2.456 | -1.48% | 0.919 → 0.958 | -4.28% |
| ScalerMacros-v1.asm | 0.977 → 1.063 | -8.83% | 0.629 → 0.656 | -4.21% |
| ScalerMacros-v2.asm | 3.152 → 3.241 | -2.83% | 2.155 → 2.203 | -2.24% |

The candidate won 4/5, 3/5, 2/5, 2/5, 1/5, 1/5 and 1/5 Windows pairs; Linux counts were 1/5, 0/5, 2/5, 2/5, 1/5, 0/5 and 0/5. This candidate showed no repeatable useful benefit and was discarded without a commit. Its extra tail fields changed structure layouts as well as append behavior; the timings do not isolate a cause for the negative results. Unusually long captured bodies may behave differently.

### Reuse stored macro body count: discarded after confirmation

The candidate assigned `ExpandMacro`'s line count from the existing `BodyLineCount`, removing the per-invocation counting walk and adding no state. All 207 tests passed on both platforms.

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.318 → 1.219 | 7.46% | 0.478 → 0.469 | 1.82% |
| Sonic 2 | 1.998 → 2.014 | -0.81% | 0.990 → 1.007 | -1.73% |
| Sonic & Knuckles | 2.033 → 2.171 | -6.75% | 0.900 → 0.899 | 0.15% |
| Big Fishing Derby | 1.704 → 1.708 | -0.24% | 0.893 → 0.904 | -1.26% |
| Sonic Classic Heroes | 2.290 → 2.322 | -1.43% | 1.011 → 0.974 | 3.71% |
| ScalerMacros-v1.asm | 0.945 → 0.918 | 2.90% | 0.668 → 0.671 | -0.47% |
| ScalerMacros-v2.asm | 3.072 → 3.033 | 1.26% | 2.212 → 2.251 | -1.78% |

The candidate won 4/5, 1/5, 2/5, 2/5, 1/5, 4/5 and 3/5 Windows pairs; Linux counts were 4/5, 2/5, 4/5, 2/5, 3/5, 1/5 and 1/5. Sonic 1 warranted confirmation, as did the apparent Sonic & Knuckles Windows regression. A fresh nine-pair comparison on all three public inputs gave:

| Workload | Windows baseline → candidate (s) | Time reduction | Faster pairs | Linux baseline → candidate (s) | Time reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.098 → 1.099 | -0.07% | 6/9 | 0.492 → 0.489 | 0.59% | 5/9 |
| Sonic 2 | 1.919 → 1.899 | 1.07% | 5/9 | 0.998 → 1.003 | -0.51% | 6/9 |
| Sonic & Knuckles | 2.009 → 1.981 | 1.41% | 7/9 | 0.912 → 0.903 | 0.94% | 6/9 |

The initial larger gain and regression did not repeat, and confirmation differences were small with overlapping timing ranges. The stored-count change was discarded without a commit. These experiments illustrate that removing list traversal does not guarantee a measurable improvement when other work dominates, and that a short positive median is insufficient evidence for acceptance.

All seven workloads preserved identical baseline/candidate objects through every warmup and measured run of all three screens. Actual code records matched across OSes, ignoring only creator footers. Confirmation objects also matched. Raw reports retain individual samples, timing ranges, executable hashes and execution order; binaries, logs, snapshots and private adapters remain in ignored local directories. No new instrumented profile is claimed. The committed macro-line-cursor implementation remains the final code baseline; only these experiment notes are retained from the follow-up work.

## Delimiter and scratch-buffer experiments (2026-10-05)

These experiments use the same seven workloads and adapters described above. Measurements are uninstrumented Release -O3 builds, with MinGW GCC 16.2 on Windows/local NTFS and GCC 15.2 on WSL2 Ubuntu/native Linux storage. Each binary gets one warmup; measured pairs alternate and reverse execution order each pair. OS batches run sequentially, separately from builds and tests. Positive percentages mean less time. Raw samples, binaries, logs, verification results, and private input snapshots remain in ignored local directories.

### Single-character delimiter: accepted

The baseline includes the accepted macro line cursor. The candidate detects exactly one single-character entry in the packed delimiter list and routes it through `QuotPosQualify`, avoiding general `strlen`/`strncmp` matching at each source character. Other configurations keep the general scanner. Five-pair screening results:

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.335 → 1.209 | 9.41% | 0.518 → 0.464 | 10.43% |
| Sonic 2 | 2.134 → 1.979 | 7.25% | 1.061 → 0.962 | 9.36% |
| Sonic & Knuckles | 2.235 → 2.064 | 7.67% | 0.987 → 0.863 | 12.51% |
| Big Fishing Derby | 1.703 → 1.572 | 7.68% | 1.002 → 0.902 | 9.99% |
| Sonic Classic Heroes | 2.498 → 2.381 | 4.69% | 0.990 → 0.896 | 9.42% |
| ScalerMacros-v1.asm | 1.046 → 0.873 | 16.59% | 0.683 → 0.526 | 22.93% |
| ScalerMacros-v2.asm | 3.334 → 2.403 | 27.92% | 2.186 → 1.604 | 26.63% |

The candidate won all five pairs for every workload on both platforms. A fresh nine-pair confirmation on the public inputs and the larger scaler gave:

| Workload | Windows baseline → candidate (s) | Time reduction | Faster pairs | Linux baseline → candidate (s) | Time reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.178 → 1.102 | 6.45% | 8/9 | 0.471 → 0.400 | 15.09% | 8/9 |
| Sonic 2 | 2.086 → 1.929 | 7.54% | 8/9 | 0.950 → 0.848 | 10.72% | 9/9 |
| Sonic & Knuckles | 2.274 → 2.109 | 7.26% | 8/9 | 0.880 → 0.783 | 10.99% | 9/9 |
| ScalerMacros-v2.asm | 3.150 → 2.268 | 28.01% | 9/9 | 2.174 → 1.517 | 30.25% | 9/9 |

The scaler confirmation ranges do not overlap: Windows 2.970–3.513 s versus 2.004–2.522 s, Linux 2.132–2.230 s versus 1.493–1.547 s. Sonic 1's Windows baseline includes a 3.128 s outlier; medians and paired wins are reported rather than treating that sample as typical. These are repeatable gains on this machine, not instrumented profile percentages or guarantees for every input.

All 207 tests pass on both OSes. A temporary differential harness compiled the old and candidate scanner functions extracted directly from source, comparing 600,000 results on deterministic generated inputs with quote escapes, bracket nesting, qualified/unqualified apostrophes, empty and multi-entry lists, multi-character delimiters and all three scanner entry points. All results agree. Every warmup and measured object matches the baseline; actual code records match across Windows and Linux, ignoring only creator footers, including confirmation runs. The fast path is retained and becomes the baseline for the next experiment.

### Skip matching inside quotes/brackets: discarded

This candidate moved the protected-state check before delimiter matching in the shared scanner. It retained every quote, escape, bracket and apostrophe-qualification update, including matching at eligible opening characters before updating state. Its baseline includes the accepted single-character fast path. All 207 tests pass on both platforms and the same 600,000 differential scanner comparisons agree with the baseline. Five-pair results:

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.273 → 1.209 | 5.04% | 0.457 → 0.502 | -9.94% |
| Sonic 2 | 1.993 → 1.992 | 0.06% | 0.983 → 0.995 | -1.15% |
| Sonic & Knuckles | 2.161 → 2.105 | 2.60% | 0.950 → 0.976 | -2.78% |
| Big Fishing Derby | 1.653 → 1.712 | -3.57% | 0.830 → 0.840 | -1.15% |
| Sonic Classic Heroes | 2.193 → 2.363 | -7.75% | 0.924 → 0.914 | 1.06% |
| ScalerMacros-v1.asm | 0.832 → 0.825 | 0.85% | 0.574 → 0.586 | -2.03% |
| ScalerMacros-v2.asm | 2.545 → 2.355 | 7.47% | 1.646 → 1.692 | -2.81% |

In table order, the candidate won 4/5, 2/5, 5/5, 1/5, 2/5, 2/5 and 3/5 Windows pairs; Linux counts were 1/5, 2/5, 2/5, 3/5, 3/5, 1/5 and 2/5. The code was correct but the timings did not establish a repeatable useful incremental gain. The candidate was discarded without a commit. Every object matched the baseline and actual code records matched across OSes.

### Expression scratch-buffer reuse: accepted

The baseline includes the accepted single-character delimiter fast path, without the discarded protected-region change. `EvalStrExpression` borrows a pair of scratch buffers from a free list; each active recursive evaluation keeps its pair checked out. New pairs are allocated only when all existing pairs are active, and pairs are individually allocated so recursive calls cannot move a parent's storage. Cleanup returns buffers with their updated capacity; file cleanup frees the pool. Parsing and backup-copy behavior are unchanged, and expression values are never cached. The pool retains peak pair count and enlarged capacities until file cleanup. Other parser allocations, including lambda-formatting and string-constant scratch storage, are unchanged.

Five-pair screening results:

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.273 → 1.122 | 11.89% | 0.465 → 0.413 | 11.14% |
| Sonic 2 | 2.175 → 1.741 | 19.98% | 0.963 → 0.841 | 12.60% |
| Sonic & Knuckles | 2.073 → 1.928 | 7.01% | 0.920 → 0.784 | 14.76% |
| Big Fishing Derby | 1.671 → 1.441 | 13.80% | 0.822 → 0.721 | 12.29% |
| Sonic Classic Heroes | 2.341 → 2.113 | 9.73% | 0.850 → 0.768 | 9.62% |
| ScalerMacros-v1.asm | 0.835 → 0.733 | 12.27% | 0.514 → 0.476 | 7.36% |
| ScalerMacros-v2.asm | 2.450 → 2.390 | 2.46% | 1.617 → 1.705 | -5.46% |

All five Windows pairs favored the candidate on every workload. Linux counts were 4/5, 5/5, 5/5, 5/5, 5/5, 5/5 and 3/5. The larger scaler's negative Linux median despite three faster pairs warranted confirmation. A fresh nine-pair batch on every workload gave:

| Workload | Windows baseline → candidate (s) | Time reduction | Faster pairs | Linux baseline → candidate (s) | Time reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 1.139 → 0.953 | 16.34% | 8/9 | 0.437 → 0.403 | 7.92% | 9/9 |
| Sonic 2 | 1.948 → 1.564 | 19.69% | 9/9 | 0.929 → 0.790 | 15.00% | 9/9 |
| Sonic & Knuckles | 2.079 → 1.822 | 12.35% | 9/9 | 0.840 → 0.755 | 10.15% | 7/9 |
| Big Fishing Derby | 1.687 → 1.448 | 14.18% | 9/9 | 0.764 → 0.684 | 10.49% | 9/9 |
| Sonic Classic Heroes | 2.247 → 2.017 | 10.23% | 7/9 | 0.798 → 0.728 | 8.69% | 9/9 |
| ScalerMacros-v1.asm | 0.764 → 0.622 | 18.62% | 9/9 | 0.457 → 0.416 | 9.06% | 8/9 |
| ScalerMacros-v2.asm | 2.267 → 2.056 | 9.32% | 9/9 | 1.503 → 1.466 | 2.48% | 8/9 |

The candidate improves every confirmation median. The larger scaler's apparent Linux slowdown does not repeat; its confirmed gain is modest and timing ranges overlap. Sonic 2's confirmation ranges do not overlap on either OS: Windows 1.872–2.070 s versus 1.485–1.636 s, Linux 0.885–0.966 s versus 0.778–0.834 s. These are incremental gains from scratch reuse, measured separately from the delimiter fast path; percentages from different batches are not added together.

All 208 tests pass on Windows and Linux. The new `t_exprscratch` regression covers nested functions/operators, float and stringification paths, scratch growth beyond the initial 1024 bytes followed by short expressions, forward references across passes and forty nested parenthesized calls. Its independently specified bytes also match the baseline assembler. Two-file runs produce matching output on both platforms, exercising file cleanup and subsequent initialization. Linux AddressSanitizer memory-access checks pass for the recursion/growth regression and the two-file run; leak detection was disabled for these checks.

All seven workloads retain identical baseline/candidate objects through every warmup, screening and confirmation run. Actual code records match across Windows and Linux, ignoring only creator footers. The pool change is retained. The final implementation includes the single-character delimiter fast path and scratch reuse, with the protected-region candidate discarded. No new instrumented profile is claimed for these experiments.

## Refreshed instrumented profiles (2026-10-05)

Fresh Release gprof builds of commit `7adbaef` include the accepted macro line cursor, single-character delimiter fast path and expression scratch pool. All seven workloads were profiled again, five assemblies each, with the same snapshots, adapters, flags and no listing. Windows used MinGW GCC 16.2.0 and gprof 2.47.20260726; WSL2 Ubuntu used GCC 15.2.0 and gprof 2.46, with inputs on native Linux storage. Windows profiling disables executable ASLR for gprof collection only. Builds completed before collection; the OS batches ran sequentially, with no warmups.

The user reported staying tabbed out of VSCode during these runs to reduce possible Git-extension activity. This was not a controlled editor-activity comparison, so its contribution to earlier noise remains unproven. Windows still had elapsed-time outliers. Instrumented elapsed times are not used to infer optimization gains.

Every profiled assembly produced the same object hash as the latest uninstrumented scratch-pool confirmation executable on its OS. Actual code records match across OSes, excluding creator footers. This checks all five runs of all seven workloads on each platform. No production code changed for this refresh; the existing 208-test results apply to this commit.

### Current sampled self-time

These are percentages of all gprof samples, including profiling overhead and unidentified attribution. They exclude callees from each named function's share. They are instrumented CPU-time rankings, not percentages of uninstrumented wall time or expected speedups. Linux provides the clearer function attribution:

| Workload | `EvalStrExpression` | `QuotPosQualify` | `AssembleFile` | `MACRO_Processor` |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 17.61% | 11.27% | 4.23% | 0.70% |
| Sonic 2 | 17.97% | 14.06% | 7.81% | 0.39% |
| Sonic & Knuckles | 14.69% | 10.20% | 6.53% | 0.00% |
| Big Fishing Derby | 16.07% | 11.61% | 6.70% | 0.89% |
| Sonic Classic Heroes | 17.13% | 7.57% | 4.38% | 0.80% |
| ScalerMacros-v1.asm | 7.50% | 12.50% | 15.00% | 0.62% |
| ScalerMacros-v2.asm | 8.96% | 16.42% | 13.81% | 1.87% |

Zero is a rounded sampled share, not evidence that a function does no work. `AssembleFile` includes broad assembly/line-processing work and compiler-inlined code; this table does not identify file I/O as the bottleneck.

Windows corroborates expression evaluation and scanning as remaining areas, but its instrumentation occupies much of the sample budget:

| Workload | `EvalStrExpression` | Reported `SearchSingleChar` | `_mcount_private` + `__fentry__` |
| --- | ---: | ---: | ---: |
| Sonic 1 | 8.05% | 5.75% | 51.73% |
| Sonic 2 | 11.37% | 4.37% | 54.23% |
| Sonic & Knuckles | 8.46% | 3.98% | 49.01% |
| Big Fishing Derby | 10.13% | 6.86% | 45.43% |
| Sonic Classic Heroes | 11.28% | 6.23% | 44.51% |
| ScalerMacros-v1.asm | 6.91% | 6.45% | 52.99% |
| ScalerMacros-v2.asm | 4.61% | 10.09% | 48.42% |

Disassembly of both normal and instrumented Windows builds shows direct delimiter comparison in the specialized scanner core, without a per-character call to `SearchSingleChar`. The Windows helper label therefore does not establish callback overhead; attribution around optimized clones is unreliable. Linux incorporates the specialized loop into `QuotPosQualify`. Linux also reports 10.61–22.54% under `_init`; that bucket is unresolved and must not be interpreted as startup cost or a particular library function. Shares above have not been renormalized to remove these buckets.

### Call-count confirmation

Compared with the earlier three-run single-pass profiles, before the macro cursor, delimiter and scratch-pool changes, `StrCompAlloc` calls per assembly fall as follows. Counts agree between Windows and Linux:

| Workload | Earlier → current `StrCompAlloc` calls | Reduction |
| --- | ---: | ---: |
| Sonic 1 | 1,680,828 → 23,818 | 98.58% |
| Sonic 2 | 4,454,689 → 55,471 | 98.75% |
| Sonic & Knuckles | 3,147,949 → 15,701 | 99.50% |
| Big Fishing Derby | 3,367,276 → 50,346 | 98.51% |
| Sonic Classic Heroes | 2,891,384 → 56,724 | 98.04% |
| ScalerMacros-v1.asm | 1,931,896 → 214 | 99.99% |
| ScalerMacros-v2.asm | 3,136,343 → 217 | 99.99% |

These are calls to this allocation wrapper across the assembler, not all heap allocations. The reduction is consistent with scratch reuse; it is not a wall-time saving estimate. Primary and self-recursive `EvalStrExpression` call counts are unchanged from the earlier profiles on both OSes. `ExpandTokens` and legacy `ExpandLine` counts are also unchanged for every workload.

The larger scaler's Linux `MACRO_Processor` sampled self-time share drops from 19.51% in the earlier profile to 1.87% now, consistent with removing repeated body-list traversal. Sampling and instrumentation prevent treating that ratio as a speedup. The accepted uninstrumented paired results remain the performance evidence.

The delimiter shortcut shifts work between scanner symbols: on Linux, current `QuotPosQualify` calls equal its earlier count plus the unchanged `QuotSMultPosQualify` count for every workload. For example, the larger scaler goes from 3,964,224 to 7,983,562 calls, with 4,019,338 multi-delimiter entry calls routed through it. `QuotSMultPosQualify` itself now samples at only 0–0.62%. This is redistribution of existing scanning work, not additional source scanning; the general wrapper's lower share does not mean comment scanning has disappeared.

### Next measurements

Expression evaluation remains a priority for the full projects. Measure how often simple numeric literals and symbols reach the general operator-discovery path, and the cost of conversion attempts, before proposing a conservative fast path. Preserve CPU-specific numeric syntax, temporary/internal symbols, relocation, flags and source-position behavior. The allocation profile gives little reason to prioritize another broad scratch-allocation change first.

Quote-aware scanning remains substantial on every workload. A small candidate to investigate is rejecting a single-character search with `strchr` when the character is absent, retaining the full state machine when present. First measure absence frequency and audit apostrophe-qualification callback behavior; correctness and a gain are not yet established. This is separate from the discarded protected-region matching experiment.

For the scalers, finer attribution inside `AssembleFile` would help choose the next line-processing experiment. Further macro-cursor or list-metadata changes are lower priority given the current profile and the previous inconclusive experiments. `EvalStrExpression` in lambda substitution remains a future evaluation item, rather than a demonstrated current bottleneck.

Raw Windows gmons, reports and results remain under ignored `build-benchmark-results/reprofile/windows-public` and `build-benchmark-results/local-private/reprofile-windows`; the exact Windows executable/catalogs and analysis are under `build-benchmark-results/reprofile`. Native Linux artifacts are under `/tmp/asl-token-20261005/reprofile-public`, `reprofile-local` and `reprofile-binaries`. Private sources, adapters and original paths remain outside committed documentation. Workload scope remains ASL assembly with the previously documented adapters, not the complete game build pipelines.

## Scanner and expression follow-up experiments (2026-10-05)

These experiments proceed independently in order: delimiter absence, expression backup copying, operator dispatch, then a conservative simple-symbol path. Each accepted implementation becomes the next baseline. Timing uses the same seven prepared workloads, flags and adapters, optimized uninstrumented builds, one warmup per executable and alternating/reversed measured pairs. Builds and tests precede timings; OS batches run sequentially. The user reports remaining tabbed out of VSCode. This condition does not establish the source of earlier noise. Raw inputs, private adapters, binaries and reports stay ignored.

### Single-character delimiter absence — accepted

`QuotPosQualify` first uses `strchr` to reject a missing delimiter, avoiding quote/bracket traversal. A present delimiter retains the complete original scanner. Both installed quote-qualification callbacks only inspect their inputs, so skipping them when no match can exist has no side effects. Searching for NUL retains the previous scanner result. This does add a preliminary search on strings containing the delimiter; gains must therefore be established on complete assemblies.

All 208 tests pass on Windows and Linux. The extracted-source differential scanner harness agrees on 600,000 comparisons with quotes, escapes, nesting, apostrophe qualification and packed delimiter configurations. Five-pair screening improves every median: Windows 2.85–11.78%, Linux 3.58–15.34%. Windows faster-pair counts in table order are 5, 5, 4, 5, 4, 4, 5 out of five; Linux counts are 5, 4, 4, 4, 4, 5, 5.

Fresh nine-pair confirmation:

| Workload | Windows baseline → candidate (s) | Time reduction | Faster pairs | Linux baseline → candidate (s) | Time reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.951 → 0.881 | 7.35% | 9/9 | 0.401 → 0.371 | 7.32% | 9/9 |
| Sonic 2 | 1.510 → 1.458 | 3.47% | 4/9 | 0.784 → 0.746 | 4.81% | 7/9 |
| Sonic & Knuckles | 1.832 → 1.725 | 5.84% | 9/9 | 0.761 → 0.724 | 4.85% | 7/9 |
| Big Fishing Derby | 1.426 → 1.320 | 7.45% | 8/9 | 0.703 → 0.667 | 5.11% | 8/9 |
| Sonic Classic Heroes | 2.067 → 1.997 | 3.39% | 8/9 | 0.794 → 0.742 | 6.52% | 9/9 |
| ScalerMacros-v1.asm | 0.651 → 0.623 | 4.33% | 8/9 | 0.474 → 0.447 | 5.78% | 8/9 |
| ScalerMacros-v2.asm | 2.081 → 1.771 | 14.89% | 9/9 | 1.514 → 1.291 | 14.70% | 9/9 |

Sonic 2's Windows confirmation is inconclusive in isolation despite the positive median. The other Windows workloads and every Linux workload favor the candidate in at least seven pairs. The larger scaler wins all pairs with nonoverlapping ranges: Windows 2.033–2.188 versus 1.678–1.889 s; Linux 1.497–1.531 versus 1.276–1.307 s. The overall evidence supports accepting this small change. No delimiter-absence frequency or instrumented speedup estimate is claimed.

Every warmup and measured object matches its baseline; actual code records match across OSes for screening and confirmation. Raw reports are under ignored `build-benchmark-results/delimiter-absence/windows-public-{ab,confirm}`, `build-benchmark-results/local-private/delimiter-absence-windows-{ab,confirm}` and native Linux `/tmp/asl-token-20261005/delimiter-absence/{public,local}-{ab,confirm}`. Preserved baselines, test logs and differential checks accompany the experiment artifacts.

### Delay expression backup until numeric conversion fails — discarded

This candidate moved `StrCompCopy(&STempComp, &CopyComp)` after integer and float conversion, retaining it before string conversion and target-specific symbol handling. Both numeric conversions only read the expression, so constants and the program counter no longer needed the backup. Later parsing and symbol restoration were unchanged. The baseline includes accepted delimiter absence.

All 208 tests pass on Windows and Linux; every warmup and measured object matches the baseline, and actual code records match across OSes. Five-pair screening:

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.948 → 0.959 | -1.15% | 0.380 → 0.378 | 0.47% |
| Sonic 2 | 1.559 → 1.591 | -2.00% | 0.775 → 0.761 | 1.90% |
| Sonic & Knuckles | 1.711 → 1.706 | 0.31% | 0.752 → 0.719 | 4.38% |
| Big Fishing Derby | 1.320 → 1.299 | 1.60% | 0.702 → 0.687 | 2.18% |
| Sonic Classic Heroes | 1.956 → 1.912 | 2.25% | 0.763 → 0.756 | 0.88% |
| ScalerMacros-v1.asm | 0.612 → 0.615 | -0.40% | 0.457 → 0.466 | -2.05% |
| ScalerMacros-v2.asm | 1.820 → 1.888 | -3.73% | 1.354 → 1.350 | 0.28% |

Windows faster-pair counts in table order are 2, 3, 2, 3, 3, 3, 2; Linux counts are 2, 2, 3, 3, 2, 2, 2 out of five. Every range overlaps, and positive medians lack a consistent pairwise advantage. No meaningful gain is established. The implementation was reverted without confirmation or a code commit. Raw reports and baselines use the same directory pattern as above with experiment name `expr-backup` and suffix `ab`.

### First-character expression operator dispatch — accepted

At parser initialization, operators are grouped into chains by their first byte, preserving original table order and the existing operator-count limit. During expression scanning, only that byte's chain is considered; single-byte operators need no `strncmp`. The preliminary whole-expression `strchr`/`strstr` pass for every operator is removed. Quote/bracket state, longest-match handling, precedence, associativity, unary interpretation, result conversion and symbol handling remain unchanged. The original expression backup copy is retained; the discarded backup candidate is not included.

All 209 tests pass on Windows and Linux, including `t_exprdispatch`, whose independent expected bytes cover overlapping operators, precedence, associativity, unary operations, strings and floats and also pass with the previous assembler. An extracted-source harness compares operator choice, position and quote/bracket state on 600,000 generated cases, including high bytes and three apostrophe-qualification modes; all agree. A capacity adjustment before confirmation preserves all `OPERATOR_MAXCNT` slots plus the placeholder index; final builds repeat both suites and differential checks.

The Linux screen initially ran out of staging space: Ubuntu's `/tmp` is a 7.8 GB tmpfs, not disk-backed storage. Only public timings had completed; private timings had not begun. The entire generated `/tmp/asl-token-20261005` workspace was relocated to `/home/marzojr/asl-benchmark-20261005`, retaining the former path as a symlink. Logs, objects, binaries and input snapshots were preserved. The original public batch is retained as `public-ab-tmpfs`, and incomplete private staging as `local-ab-incomplete`. The entire Linux screen was rerun with fresh inputs on native ext4; confirmation and subsequent experiments use that disk. No timings were collected during relocation. Earlier Linux results used native Linux tmpfs; percentages below compare contemporaneous pairs on the same filesystem, not absolute times across storage conditions.

Five-pair screening median reductions:

| Workload | Windows | Linux, fresh ext4 batch |
| --- | ---: | ---: |
| Sonic 1 | 8.84% | 8.30% |
| Sonic 2 | 8.52% | 15.10% |
| Sonic & Knuckles | 6.13% | 14.41% |
| Big Fishing Derby | 10.35% | 18.42% |
| Sonic Classic Heroes | 7.26% | -6.85% |
| ScalerMacros-v1.asm | 11.10% | 15.47% |
| ScalerMacros-v2.asm | 8.30% | 9.53% |

Windows faster-pair counts are 5, 5, 4, 5, 5, 5, 5; Linux counts are 4, 4, 4, 5, 2, 5, 5. Sonic Classic Heroes' Linux candidate range includes 1.926 s versus a 0.741 s median, but neither that outlier nor the other positive medians alone establishes the outcome. Fresh nine-pair confirmation of the final implementation gives:

| Workload | Windows baseline → candidate (s) | Time reduction | Faster pairs | Linux baseline → candidate (s) | Time reduction | Faster pairs |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.909 → 0.869 | 4.32% | 7/9 | 0.362 → 0.307 | 15.11% | 8/9 |
| Sonic 2 | 1.428 → 1.327 | 7.07% | 8/9 | 0.752 → 0.610 | 18.88% | 9/9 |
| Sonic & Knuckles | 1.666 → 1.547 | 7.14% | 7/9 | 0.735 → 0.622 | 15.26% | 9/9 |
| Big Fishing Derby | 1.288 → 1.155 | 10.30% | 9/9 | 0.681 → 0.547 | 19.62% | 9/9 |
| Sonic Classic Heroes | 1.976 → 1.822 | 7.81% | 8/9 | 0.719 → 0.620 | 13.81% | 9/9 |
| ScalerMacros-v1.asm | 0.607 → 0.522 | 14.04% | 8/9 | 0.449 → 0.383 | 14.87% | 9/9 |
| ScalerMacros-v2.asm | 1.723 → 1.598 | 7.30% | 8/9 | 1.382 → 1.187 | 14.09% | 9/9 |

Every confirmation median improves and every workload wins at least seven Windows pairs and eight Linux pairs. The earlier Sonic Classic Heroes slowdown does not repeat; its Linux ranges do not overlap (0.706–0.793 versus 0.605–0.662 s). Big Fishing Derby's ranges do not overlap on either platform. The dispatch change is accepted. Gains are incremental to delimiter absence and must not be added to percentages from other batches.

Every warmup and measured object matches its baseline, and actual code records match across OSes for screening and confirmation. Raw artifacts follow the earlier directory pattern with experiment name `operator-index`; Linux paths now resolve through the retained symlink to native disk. No new instrumented profile or call-count reduction is claimed.

### Conservative simple-symbol shortcut — discarded

After the existing constant conversions and target-specific `InternSymbol` attempt, this candidate recognized ASCII identifiers of the form `[A-Za-z_][A-Za-z0-9_]*` and jumped to the original symbol-restoration/lookup path. These identifiers cannot contain the current operators, quotes or brackets. Numeric syntax, program-counter handling, registers, temporary symbols, internal symbols, forward-reference flags and lookup behavior retained their existing order; other inputs retained the full scanner. No expression values were cached. The accepted operator dispatch was the baseline, so this experiment measured the shortcut's incremental value after that improvement.

All 209 tests pass on Windows and Linux. Every warmup and measured object matches the baseline, and actual code records match across OSes. Five-pair screening, with Linux inputs on native ext4:

| Workload | Windows baseline → candidate (s) | Time reduction | Linux baseline → candidate (s) | Time reduction |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 0.949 → 0.958 | -0.95% | 0.306 → 0.322 | -5.20% |
| Sonic 2 | 1.343 → 1.384 | -3.05% | 0.622 → 0.628 | -0.93% |
| Sonic & Knuckles | 1.579 → 1.533 | 2.93% | 0.647 → 0.638 | 1.28% |
| Big Fishing Derby | 1.167 → 1.245 | -6.75% | 0.585 → 0.586 | -0.24% |
| Sonic Classic Heroes | 1.818 → 1.803 | 0.83% | 0.672 → 0.682 | -1.40% |
| ScalerMacros-v1.asm | 0.517 → 0.596 | -15.22% | 0.395 → 0.416 | -5.32% |
| ScalerMacros-v2.asm | 1.671 → 1.704 | -1.95% | 1.213 → 1.239 | -2.15% |

Windows faster-pair counts in table order are 3, 3, 4, 1, 4, 2, 2; Linux counts are 1, 3, 2, 3, 3, 0, 2 out of five. No useful overall gain is established. The first scaler's Linux ranges do not overlap (0.384–0.397 versus 0.401–0.429 s), with no faster candidate pairs. The extra eligibility scan may outweigh work avoided in the already improved parser, but its precise cost was not separately profiled. This conservative variant does not determine whether a more ambitious symbol path could help.

The implementation was reverted without confirmation or a code commit, and both normal assemblers rebuilt from the accepted implementation. Raw reports and preserved baselines follow the earlier directory pattern with experiment name `symbol-fast` and suffix `ab`. The final production code includes delimiter absence and first-character operator dispatch; the original backup copy and general symbol path remain. All four requested follow-up experiments are complete.
