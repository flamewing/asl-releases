# Listing and string-processing experiments (2026-10-06)

Evaluate four changes independently, in order: write each listing line through one buffered stream operation; fast-path common whitespace; append literal format text in spans; retain an append cursor during listing construction. Each accepted change becomes the next baseline. Restore rejected changes before proceeding. Initial source was `930b9b0`; pristine builds exposed the [global macro-copy initialization defect](MACRO_COPY_FIX.md), corrected separately in `653269c`. No performance timings use the initial failing builds. The first optimization baseline is `653269c`.

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

Pending the preceding decision.

### 4. Listing append cursor

Retain the constructed listing line's length locally, passing it to an internal formatted-append helper. Avoid a library-wide dynamic-string layout change. Preserve length across buffer growth, line resets, radix/padding changes and wrapped code lines.

Pending the preceding decision.
