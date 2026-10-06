# Listing and string-processing experiments (2026-10-06)

Evaluate four changes independently, in order: write each listing line through one buffered stream operation; fast-path common whitespace; append literal format text in spans; retain an append cursor during listing construction. Each accepted change becomes the next baseline. Restore rejected changes before proceeding. Initial source was `930b9b0`; pristine builds exposed the [global macro-copy initialization defect](MACRO_COPY_FIX.md), corrected separately in `653269c`. No performance timings use the initial failing builds. The first optimization baseline is `653269c`.

Completed in order. Commit `3ada94e` retains listing writes; `d5407ab` retains common whitespace. Literal format spans and the listing append cursor are discarded. Winners primarily improve Windows GCC, with smaller MSVC listing-write gains and measured Linux limitations recorded below. Every trial preserves assembled code and listing contents, and rejected source changes are removed.

## Method

Use pristine source/build directories under `D:/tmp/asl-bench-20261006/output-experiments` and native WSL2 ext4 `/home/marzojr/asl-benchmark-20261005/output-experiments`. Build Windows GCC 16.2, Windows MSVC 19.44 and Linux GCC 15.2 with the same normal Release flags as the [aggregate comparison](HEAD_COMPILER_PROFILES.md). Run complete correctness suites before timings. Build and timing batches do not overlap; Windows and Linux timing batches run sequentially.

Reuse all eleven prepared variants with their established flags. Screen with one warmup per binary and five measured alternating pairs; confirm promising candidates in a separate nine-pair batch. Windows interleaves both compilers and reverses pair/compiler order. Record child CPU time, host memory and native resource counters as well as wall time. Do not accept a small isolated median difference without repeatable useful gains.

Require deterministic objects matching baseline on each platform/compiler and matching absolute code records across platforms. For variants producing listings, compare listing contents on every run after replacing only page-header timestamps, predefined `TIME`/`DATE` symbol values and the assembly-time summary with fixed placeholders. Retain raw objects, listings, logs, source snapshots and exact executable hashes externally. Only anonymized performance data for `local-e` may appear in this report; identity, provenance, source and adapters remain external. Markdown paragraphs are not hard-wrapped.

## Trials

### 1. Listing writes

Replace the three listing-body `fprintf("%s\n", ...)` paths in `WrLstLine` with a reusable buffer containing the line plus newline and a single `fwrite`. Keep text-mode streams, pagination, existing error checks and allocation-failure fallback to the original operation. Do not change other output paths or buffering settings in this trial.

Accepted against `653269c`. All 210 tests pass for all three toolchains. Independent source-linked listing fixtures pass with each compiler for exact bytes, empty/high-byte/long lines, tab wrapping, pagination, suppressed output, allocation fallback and write errors. Windows and Linux objects preserve the established code records; normalized listings match on every completed run.

Positive percentages mean less median elapsed time. Parentheses give faster-pair counts; screen uses five pairs and confirmation nine. The initial screen completed seven non-listing cases before the listing comparator encountered volatile clock values; four listing cases resumed in separate batches after correcting normalization. None of the incomplete listing timings is included. Confirmation is a fresh full eleven-case batch on each OS.

| Workload | GCC Windows screen | GCC Windows confirm | MSVC screen | MSVC confirm | Linux screen | Linux confirm |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | -0.08% (4/5) | -0.17% (5/9) | -14.44% (1/5) | -0.16% (5/9) | -2.30% (2/5) | -1.08% (2/9) |
| Sonic 2 | +0.10% (2/5) | -0.45% (4/9) | -0.64% (1/5) | +0.61% (5/9) | +1.82% (5/5) | -1.56% (5/9) |
| Sonic & Knuckles | -1.17% (2/5) | -1.29% (3/9) | +1.75% (2/5) | +0.00% (4/9) | -3.84% (1/5) | +0.27% (4/9) |
| Big's Fishing Derby | -0.56% (2/5) | -0.50% (4/9) | -2.32% (1/5) | -0.16% (6/9) | +0.74% (2/5) | +0.61% (4/9) |
| Sonic Classic Heroes | -0.72% (2/5) | +3.72% (5/9) | -0.60% (1/5) | +2.82% (8/9) | -0.84% (1/5) | -0.70% (3/9) |
| ScalerMacros-v1.asm | -2.24% (1/5) | +0.70% (7/9) | -16.67% (2/5) | +2.59% (4/9) | -1.91% (0/5) | +0.75% (5/9) |
| ScalerMacros-v2.asm | +0.05% (2/5) | -0.13% (3/9) | -0.54% (2/5) | +0.12% (5/9) | -0.32% (2/5) | +0.61% (7/9) |
| Madness 4 release | +19.13% (5/5) | +19.04% (9/9) | +3.52% (5/5) | +3.38% (9/9) | +4.08% (3/5) | +2.99% (8/9) |
| Madness 4 debug | +20.37% (5/5) | +20.26% (9/9) | +2.99% (4/5) | +2.83% (9/9) | +2.49% (4/5) | -1.54% (4/9) |
| local-e | +26.50% (5/5) | +27.44% (9/9) | +5.74% (4/5) | +5.32% (9/9) | -4.17% (1/5) | -4.38% (2/9) |
| local-e-debug | +28.11% (5/5) | +26.56% (9/9) | +4.74% (5/5) | +5.25% (9/9) | +5.03% (4/5) | +3.28% (6/9) |

The four listing-heavy cases confirm 19.04–27.44% less Windows GCC time and 2.83–5.32% less MSVC time, with all nine pairs faster for each case/compiler. Windows GCC child CPU reductions for those cases are 18.54%, 20.09%, 27.72% and 27.17%; MSVC reductions are 2.22%, 3.21%, 5.33% and 5.26%. Non-listing changes are mostly small or inconsistent, and their apparent wall gains do not establish an output-path improvement. The large initial MSVC Sonic 1/scaler-v1 wall slowdowns do not repeat in confirmation.

Linux listing CPU reductions are 2.73%, 0.65%, 2.83% and 3.28%. Linux wall results are mixed: anonymous release is 4.17% slower in the screen and 4.38% slower in confirmation despite lower child CPU time in both batches. Retain that measured limitation; this is primarily a Windows improvement, not a universal Linux elapsed-time gain. The accepted tradeoff is the substantial repeatable Windows benefit with small Linux computation savings. Raw native resource/I/O counters accompany the timings. Windows confirmation retains at least 9,262 MiB available memory, with maximum load 71%.

External artifacts: `listing-write-fixed/{screen,screen-listings,confirmation}`, `summary.json`, `audit.json`, baseline `macro-copy-init`, initial untimed failing builds, and focused fixture programs/logs. Exact sources and binaries are retained outside the checkout.

### 2. Common whitespace

Fast-path space and tab in `as_isspace`, falling back to the existing locale-aware classifier for other bytes. Preserve the unsigned-byte conversion and evaluate the input expression once. Check the complete byte domain and locale behavior independently before measuring.

Accepted against `3ada94e`. All 210 tests pass with all three toolchains. Independent byte-domain, extended integer-input, input-evaluation and locale checks pass for baseline and candidate with each compiler. Full eleven-workload screens and fresh nine-pair confirmations preserve objects, code records and normalized listings.

| Workload | GCC Windows screen | GCC Windows confirm | MSVC screen | MSVC confirm | Linux screen | Linux confirm |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | -0.45% (1/5) | +0.86% (7/9) | +0.72% (3/5) | -1.45% (4/9) | +3.05% (4/5) | +2.12% (5/9) |
| Sonic 2 | +0.16% (2/5) | +1.99% (9/9) | +1.03% (4/5) | -0.86% (3/9) | +2.38% (2/5) | +0.39% (4/9) |
| Sonic & Knuckles | +1.02% (4/5) | +1.66% (8/9) | -0.33% (2/5) | +0.15% (4/9) | -2.97% (1/5) | -0.83% (4/9) |
| Big's Fishing Derby | +2.58% (5/5) | +1.43% (6/9) | -0.26% (3/5) | -1.88% (5/9) | +4.40% (3/5) | +0.00% (4/9) |
| Sonic Classic Heroes | +2.31% (5/5) | +0.45% (7/9) | -0.63% (0/5) | +0.18% (5/9) | +0.73% (2/5) | +0.62% (7/9) |
| ScalerMacros-v1.asm | +8.55% (5/5) | +9.77% (8/9) | +4.28% (3/5) | -0.82% (6/9) | +0.58% (3/5) | +0.16% (6/9) |
| ScalerMacros-v2.asm | +7.27% (5/5) | +10.28% (9/9) | +1.38% (4/5) | -1.80% (4/9) | -0.87% (1/5) | -1.36% (1/9) |
| Madness 4 release | +1.64% (4/5) | -0.58% (5/9) | +0.23% (3/5) | +0.71% (4/9) | -1.31% (2/5) | +0.67% (4/9) |
| Madness 4 debug | +0.91% (3/5) | +0.29% (5/9) | -0.11% (2/5) | -1.54% (3/9) | -0.45% (0/5) | -1.20% (3/9) |
| local-e | +0.57% (3/5) | +2.79% (8/9) | -0.80% (2/5) | -1.14% (1/9) | +1.98% (5/5) | +0.30% (5/9) |
| local-e-debug | +3.62% (4/5) | +1.23% (7/9) | -0.38% (2/5) | +1.16% (7/9) | -2.23% (1/5) | -3.68% (4/9) |

Windows GCC scaler gains repeat: 8.55%/7.27% in screening and 9.77%/10.28% in confirmation, with 8/9 and 9/9 faster confirmation pairs. Confirmed child CPU reductions are 10.71% and 8.14%. Other GCC Windows gains are smaller; MSVC has no consistent useful gain. Its anonymous release wall slowdown repeats at 0.80% and 1.14%, although CPU changes reverse sign.

Linux is not uniformly faster: scaler v2 repeats a 0.87%/1.36% wall and CPU slowdown; anonymous debug repeats a 2.23%/3.68% wall slowdown and 2.55%/1.10% CPU slowdown. Scaler v1 is effectively unchanged. Retain the simple generic fast path for the substantial repeated Windows GCC scaler improvement, explicitly accepting these small measured Linux losses.

External artifacts: `whitespace/{screen,confirmation}`, `summary.json`, `audit.json`, byte/locale fixture logs and pristine source/build snapshots; baseline build `listing-write-fixed`.

### 3. Literal format spans

In `vsprcatf_core`, append each ordinary format-text span in one operation instead of one character at a time. Preserve format parsing, output counts, truncation, dynamic growth and percent escapes. Check fixed-buffer boundaries and dynamic output independently.

Discarded against `d5407ab`. All three 210-test suites and 2,740 independent formatting checks per compiler pass; native AddressSanitizer/UndefinedBehaviorSanitizer checks also pass. Objects and normalized listings match in all eleven screened cases on each platform/compiler.

| Workload | GCC Windows screen | MSVC screen | Linux screen |
| --- | ---: | ---: | ---: |
| Sonic 1 | -0.78% (1/5) | +0.45% (2/5) | -0.66% (3/5) |
| Sonic 2 | +0.88% (4/5) | +0.37% (2/5) | +0.64% (2/5) |
| Sonic & Knuckles | -0.25% (3/5) | +1.28% (5/5) | -0.01% (3/5) |
| Big's Fishing Derby | +0.42% (4/5) | +0.56% (3/5) | +0.48% (2/5) |
| Sonic Classic Heroes | +0.80% (3/5) | -0.07% (3/5) | -0.78% (1/5) |
| ScalerMacros-v1.asm | -0.50% (2/5) | +0.62% (3/5) | -0.99% (3/5) |
| ScalerMacros-v2.asm | -0.07% (2/5) | +0.55% (3/5) | +0.40% (3/5) |
| Madness 4 release | +0.66% (3/5) | +0.46% (3/5) | -2.84% (0/5) |
| Madness 4 debug | -0.61% (2/5) | +1.38% (4/5) | -1.49% (2/5) |
| local-e | +0.03% (3/5) | -0.37% (1/5) | +3.78% (2/5) |
| local-e-debug | -3.24% (1/5) | +0.34% (3/5) | +2.45% (3/5) |

Windows gains are small and inconsistent. Linux public listing release/debug CPU time is 4.31%/3.31% worse; the anonymous release apparent 3.78% wall gain wins only 2/5 pairs and accompanies 2.13% more CPU time. These results do not justify retaining the change or another full confirmation batch. Restore character-at-a-time literal handling before the next experiment.

External artifacts: `literal-spans/screen`, `summary.json`, `audit.json`, independent formatting fixtures and sanitizer logs; baseline build `whitespace`.

### 4. Listing append cursor

Retain the constructed listing line's length locally, passing it to an internal formatted-append helper. Avoid a library-wide dynamic-string layout change. Preserve length across buffer growth, line resets, radix/padding changes and wrapped code lines.

Discarded against `d5407ab`; the preceding literal-span candidate was removed before building this trial. Add a formatted-append helper accepting the known current length, and keep that length local to `MakeList`, updating it from actual appended counts and resetting it for each new or wrapped line. Ordinary callers still discover their destination length normally.

All three 210-test suites and 2,740 independently specified formatting checks per compiler pass, including the new known-length path and native AddressSanitizer/UndefinedBehaviorSanitizer. Screens cover all eleven workloads with all three toolchains; a fresh nine-pair Windows batch confirms benefits and penalties. Code records, deterministic objects and normalized listings match.

| Workload | GCC Windows screen | GCC Windows confirm | MSVC screen | MSVC confirm | Linux screen |
| --- | ---: | ---: | ---: | ---: | ---: |
| Sonic 1 | +0.46% (2/5) | -1.95% (3/9) | -4.21% (2/5) | -1.32% (3/9) | +0.53% (4/5) |
| Sonic 2 | +0.35% (3/5) | -0.31% (5/9) | +1.94% (3/5) | +0.04% (6/9) | -0.18% (1/5) |
| Sonic & Knuckles | -0.36% (2/5) | -0.66% (3/9) | -0.22% (2/5) | +0.00% (6/9) | -0.37% (2/5) |
| Big's Fishing Derby | +4.27% (4/5) | -0.52% (3/9) | -2.34% (0/5) | -1.79% (0/9) | -0.39% (2/5) |
| Sonic Classic Heroes | +0.37% (3/5) | +0.47% (3/9) | +0.77% (3/5) | -2.02% (3/9) | -1.04% (2/5) |
| ScalerMacros-v1.asm | +0.02% (3/5) | -1.64% (1/9) | -3.53% (0/5) | -3.59% (1/9) | -0.93% (1/5) |
| ScalerMacros-v2.asm | +0.64% (3/5) | -0.61% (3/9) | -4.76% (0/5) | -5.01% (0/9) | -0.39% (3/5) |
| Madness 4 release | +2.87% (5/5) | +1.98% (8/9) | -0.59% (1/5) | -0.44% (3/9) | +0.05% (2/5) |
| Madness 4 debug | +0.37% (3/5) | +0.29% (5/9) | +0.07% (2/5) | -0.11% (3/9) | -4.39% (3/5) |
| local-e | +1.16% (5/5) | +0.62% (5/9) | -0.47% (3/5) | +0.04% (4/9) | -5.07% (2/5) |
| local-e-debug | +1.29% (5/5) | +1.04% (7/9) | -0.01% (2/5) | -0.33% (5/9) | +3.07% (3/5) |

Windows GCC Madness 4 release improves 2.87% in screening and 1.98% in confirmation, with 1.80%/1.81% lower CPU time and 5/5 then 8/9 faster pairs. Other listing gains are small: confirmed debug 0.29%, anonymous release 0.62% and anonymous debug 1.04%. MSVC scaler slowdowns repeat: v1 3.53%/3.59%, v2 4.76%/5.01%, with confirmed CPU slowdowns of 3.33%/4.30%. MSVC Big's Fishing Derby is slower in every screen and confirmation pair. These losses outweigh the small GCC benefit.

Linux has no clear general gain in screening: the four listing CPU changes are -0.95%, -0.35%, +0.73% and +1.02%, with mixed wall results. A further native confirmation was omitted after the Windows repeat established rejection. All three experimental source files are restored; only the accepted listing-write and common-whitespace changes remain. Windows confirmation has at least 10,460 MiB available memory and maximum load 67%.

External artifacts: `listing-cursor/{screen,confirmation}` (confirmation is Windows only), `summary.json`, `audit.json`, pristine snapshots and formatting/sanitizer fixture logs; baseline build `whitespace`. Audit verifies 110 completed object combinations, 40 saved normalized listings, eleven matching code cases and six complete baseline/candidate correctness suites.
