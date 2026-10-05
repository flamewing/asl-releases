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
