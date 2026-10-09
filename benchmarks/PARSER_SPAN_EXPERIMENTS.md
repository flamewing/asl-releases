# Borrowed parser strings

The starting production baseline is `329c60f0`; `b6ac09ea` adds its final validation report. Build, diagnostic and timing artifacts remain outside the source tree, under the existing Windows and native Linux benchmark roots. Private workload source material and manifests are not committed. Markdown paragraphs are not wrapped.

## Stages

1. Instrument expression and component copies, recording call counts and bytes without using diagnostic timings as speed estimates.
2. Introduce immutable borrowed spans and bounded slice, trim, comparison and search helpers. A span does not own storage or promise a trailing NUL; its owner must remain alive and unchanged until the consumer returns.
3. Make integer constant parsing consume a bounded span, preserving literal-format priority, character translation, overflow and symbol flags.
4. Decode quoted strings from borrowed input, keeping decoded output owned and materializing only where an existing interface requires termination.
5. Propagate spans through operator and function argument splitting and recursive expression evaluation. Keep owned adapters at mutable callback boundaries; preserve source positions and short-circuit order.
6. Inventory line components and other consumers, checking mutation and pointer retention before extending borrowing into them.
7. Independently compare streaming input, immutable buffered input and immutable mapped input. Hide separate Windows and POSIX mapping implementations behind an abstraction with a portable fallback. Continuations, transformations and persistent macro bodies still require appropriate owned storage.

Each executable change receives full tests and paired workload comparisons with Windows GCC, Windows MSVC and native Linux GCC. Meaningful improvements receive independent confirmation before committing. Infrastructure that enables later copy removal is reported separately from measured optimization wins.

## Copy inventory

The diagnostic `span-copy-inventory-20261009-r2` ran each of the eleven workloads twice. Both runs produced identical counters and object hashes matching the normal baseline. The diagnostic changes were then removed from the external source snapshot. Input and backup copies happen once per expression evaluation; the categories below describe parser sites and must not be added to overlapping generic component-copy counters.

| Workload | Evaluations | Input bytes | Backup bytes | Symbol restore bytes |
| --- | ---: | ---: | ---: | ---: |
| Sonic 1 | 777,785 | 7,759,490 | 7,750,310 | 1,213,490 |
| Sonic 2 | 2,110,186 | 22,318,487 | 22,325,747 | 2,849,211 |
| Sonic & Knuckles | 1,424,409 | 13,367,645 | 13,345,608 | 3,767,714 |
| Big Fishing Derby | 1,492,241 | 12,862,701 | 12,853,297 | 2,816,825 |
| Sonic Classic Heroes | 1,279,226 | 13,245,732 | 13,226,435 | 3,626,117 |
| ScalerMacros-v1.asm | 889,456 | 4,325,370 | 4,325,190 | 589,758 |
| ScalerMacros-v2.asm | 1,491,138 | 12,747,236 | 12,747,056 | 5,287,672 |
| Madness 4 release | 2,284,439 | 22,160,843 | 22,138,039 | 3,208,631 |
| Madness 4 debug | 2,357,211 | 23,370,291 | 23,343,427 | 3,338,907 |
| local-e | 2,268,755 | 20,509,895 | 20,495,982 | 4,401,292 |
| local-e-debug | 2,291,345 | 20,900,717 | 20,885,559 | 4,439,830 |

Typical inputs are short. These counts establish repeated work, not a memory-bandwidth bottleneck. Quoted-interior copies occur much less often: 182 to 53,074 calls per workload. Integer format recognition already uses pointer/length contexts internally, making it a suitable first leaf.

`tStrComp.Pos.Len` is diagnostic metadata and is not a universal storage bound: some transformations alter the string without updating it. New spans must use lengths known from their actual storage or bounded parents. Existing component tests deliberately cover this distinction. Recursive scratch ownership, mutable CPU callbacks, quote-qualification callbacks and symbol-name canonicalization are boundaries requiring explicit audit.

## Span foundation

`strref.h` defines the borrowed view and bounded helpers. Tests use nonterminated immutable arrays, empty NULL views, out-of-range slices and maximal counts. The `span-foundation-20261009` build passes all 329 tests on each of the three toolchains. Production parsing does not yet use the helpers; this is enabling infrastructure, with no speedup claimed.

## Floating conversion follow-up

A stack termination buffer can cover common floating literals, with reusable dynamic storage for longer inputs. Numeric precision does not bound source length: leading zeros, long exponents and rounding-sensitive tails must not be silently truncated. Preserve ASL's locale, range and accepted-syntax behavior.

Adapting a counted converter remains a later independent experiment. The [libstdc++ floating `from_chars` implementation](https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/src/c%2B%2B17/floating_from_chars.cc) uses `fast_float` for supported IEEE float/double cases, while wider `long double` configurations can still use `strtold`. Its exact source dependencies and licensing notices must be reviewed if code is imported; no external conversion code has been imported in this stage.
