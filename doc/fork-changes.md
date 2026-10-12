# Changes from original AS

This document records the modifications made by the flamewing/asl-releases fork, grouped by the existing snapshot tags. The fork starts after original AS 1.42 build 204 at [`9f6b3bdb`](https://github.com/flamewing/asl-releases/commit/9f6b3bdb9311527a29b9013ded1f9db29536da2f). The latest original AS source import is build 313 at [`fb75b92d`](https://github.com/flamewing/asl-releases/commit/fb75b92ddad1d87e6003868b622971b11b9f87c4). The upstream [changelog](../changelog) describes original AS additions; those imports are listed separately here so they are not attributed to the fork.

Coverage ends at [`1275d803`](https://github.com/flamewing/asl-releases/commit/1275d803cf8f6d1c4395c9a3a3fce47183f4345b). Each tag section covers commits after the preceding tag through the named tag, in main-branch ancestry order. Tag names are preserved exactly, including the historical snapshot-snapshot prefix. A tag date can differ from its target commit date; boundaries use commit ancestry, not dates. Tags pointing to the same commit have an explicit empty batch. Changes after the last tag appear in Pending changes.

The summaries describe the changes in each batch; the linked commit lists provide the complete record, including tests, documentation, build/CI maintenance, experiments and reversions. Historical entries can describe changes later superseded or reverted, so they are not a promise that every historical experiment remains enabled. Upstream merges are represented by their integration commit; their upstream-only side-branch commits are not fork modifications. Untracked files and local experimental artifacts are outside this committed-history inventory.

## Main differences

- Assembly syntax and compatibility: revised M68K effective-address parsing, whitespace support, macro/repetition and conditional improvements, directive aliases, temporary-symbol and PHASE fixes, and Z80 register aliases.
- Correctness and robustness: decoder/encoding fixes, safer expression and string handling, malformed binary/catalog rejection, resource/I/O checks, and regression tests for failures and forward references.
- Diagnostics: contextual values, limits, supported alternatives and source positions, with English/German detail, documented numeric IDs and tests; the full inventory audit remains in progress.
- Performance and measurement: hashed symbol/forward-reference lookup, macro cursors and expansion, expression/string scan optimizations, listing improvements, and reproducible profiling/benchmark tooling.
- Build and distribution: CMake/CTest, cross-compilation and host tools, MSVC and WebAssembly support, platform CI including Android, embedded message catalogs, static analysis and validated snapshot releases.
- Documentation and maintenance: Markdown/Pandoc manual generation, portability cleanup, modern compiler/editor settings, license packaging and text normalization.

## Tagged batches

### snapshot-snapshot-v1.4.2.20210627

Boundary: [`9f6b3bdb`](https://github.com/flamewing/asl-releases/commit/9f6b3bdb9311527a29b9013ded1f9db29536da2f) to [`snapshot-snapshot-v1.4.2.20210627`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210627) ([`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/9f6b3bdb9311527a29b9013ded1f9db29536da2f...3e31e255c1f4ed080442899608614c81d115c938).

Establish Windows/MSYS2 builds and installation, GitHub Actions CI, test execution, snapshot packaging and releases. Add generated-file exclusions and Git text attributes, adjust platform definitions and compiler/linker settings, and improve failure logging. The 32-bit macOS build experiment is reverted within this batch.

Keep MESSAGE output visible in quiet mode and accept whitespace in M68K effective-address expressions, including improved trimming behavior. Clean up misleading indentation. Documentation building is temporarily disabled at this stage; later batches replace the build system and restore documentation generation.

Fork changes (36 commits):

- [`dfa0ae68`](https://github.com/flamewing/asl-releases/commit/dfa0ae68d98e69b14e35767cf448a0e78b422680) Adding support for building on msys2 on Windows
- [`534a981d`](https://github.com/flamewing/asl-releases/commit/534a981da473e7c10d57ff08bb9138fad4f98354) Adding .gitignore for generated files
- [`c03d4a5f`](https://github.com/flamewing/asl-releases/commit/c03d4a5fb5cddd6ca66e9c5c64e1e054f62f67d7) Letting MESSAGE print even in quiet mode
- [`cf0948d3`](https://github.com/flamewing/asl-releases/commit/cf0948d36e0140d952ce27590f4bc9a274e128f9) Making AS ignore whitespace on 68k EA decode
- [`c3ff6bbb`](https://github.com/flamewing/asl-releases/commit/c3ff6bbb342210e2718fd506e00f8004752aa5b3) Adding more build stuff to .gitignore
- [`b6fa9dd1`](https://github.com/flamewing/asl-releases/commit/b6fa9dd10d9a76afb528ef8aa1df532ae8c52852) Install asl.exe on msys2
- [`b7e8aca9`](https://github.com/flamewing/asl-releases/commit/b7e8aca9671ed9bf5c758877c0cf8a9cb8008578) Temporarily disabling docs building/install
- [`55145d5c`](https://github.com/flamewing/asl-releases/commit/55145d5cda30558c03dfd017d06b9cbb90dcae26) Adding in CI
- [`9af122d7`](https://github.com/flamewing/asl-releases/commit/9af122d762a4017895187cc3e5466ba4f8ea2bec) Also running tests
- [`6cbb4ac3`](https://github.com/flamewing/asl-releases/commit/6cbb4ac37b0451e822957c18f42cd1eb5a15f6c8) Some minor def file cleanup
- [`b625a1bb`](https://github.com/flamewing/asl-releases/commit/b625a1bb09ca939358de042df197f86e332331a4) Adding snapshot action
- [`59f9bd0b`](https://github.com/flamewing/asl-releases/commit/59f9bd0b9ab179be34f34cc7bbd870a8cecc12d8) Fixing grammar error
- [`d7bb327f`](https://github.com/flamewing/asl-releases/commit/d7bb327f8fcf2e77cae4976d9e2fd3453c127ea8) Correct quoting in the snapshot workflow.
- [`d39689b8`](https://github.com/flamewing/asl-releases/commit/d39689b838fe24b6ac314b6792cd74d523a25ef4) Continue repairs to the initial snapshot workflow.
- [`0c6ac47b`](https://github.com/flamewing/asl-releases/commit/0c6ac47bd371663d19436cddec94ed7b78d60ad2) Correct initial snapshot workflow configuration mistakes.
- [`9f68da62`](https://github.com/flamewing/asl-releases/commit/9f68da62c06c1c31609ec5f88975b94b9a7a2d87) Correct further initial snapshot workflow configuration mistakes.
- [`271813eb`](https://github.com/flamewing/asl-releases/commit/271813ebcaf5155907ffc77b4f6dbcc4e32dd379) Repair the initial snapshot workflow invocation.
- [`87dc18fa`](https://github.com/flamewing/asl-releases/commit/87dc18faeed71b9212a9e4135d9a9256674e9ba2) Disabling 32-bit builds on Linux and OSX
- [`2c84ddf1`](https://github.com/flamewing/asl-releases/commit/2c84ddf1833410ee2edcf652bf49684e5c9b9abb) Remove an extraneous dot in the snapshot workflow.
- [`07790b4d`](https://github.com/flamewing/asl-releases/commit/07790b4dbb012ec66e905afeca3552d1cbb0f813) asw is only on Windows
- [`5051ff92`](https://github.com/flamewing/asl-releases/commit/5051ff92c4b18a0d9d4fd3facf02bb2a3bf8df81) Forgot to remove VirusTotal scan
- [`2481a5d8`](https://github.com/flamewing/asl-releases/commit/2481a5d8e5b7b980ee41ced84349ceb71f7b4983) Repair snapshot release creation.
- [`c8bce3ba`](https://github.com/flamewing/asl-releases/commit/c8bce3ba576c4eb8b63b722960c60e4edf8bc807) Correct snapshot release creation.
- [`e9bf9d7e`](https://github.com/flamewing/asl-releases/commit/e9bf9d7e6530f4bdda84b3ad3d44054ce6aca6c6) Repair Windows snapshot packaging.
- [`417fa82b`](https://github.com/flamewing/asl-releases/commit/417fa82b9370caafe0cc5f0e6c9360d0882e8fe0) Disable fail-fast on Windows
- [`fb5668b1`](https://github.com/flamewing/asl-releases/commit/fb5668b17b36a01d6fa400ee2e61822aa1d3dd1f) Reverting OSX def file changes
- [`bd523f3f`](https://github.com/flamewing/asl-releases/commit/bd523f3f1e4e2b0edf367ad80a35b776e69aff76) Trying 32-bit Ubuntu snapshot again
- [`7705654e`](https://github.com/flamewing/asl-releases/commit/7705654e033eb043a0d56a36395d5c7cdfead622) Forgot LDFLAGS for Linux
- [`bcc6211f`](https://github.com/flamewing/asl-releases/commit/bcc6211f0fa5b77fa4888c037a73db34dc68df01) Some info logging on test fail
- [`f5d385fb`](https://github.com/flamewing/asl-releases/commit/f5d385fbcc7a3da637cc9e443e38e784fe1b1253) Adding snapshots badges to readme
- [`2df4f906`](https://github.com/flamewing/asl-releases/commit/2df4f90656a2f2e62dd81da86c904eebf18935ec) Adding .gitattributes
- [`ca82a7eb`](https://github.com/flamewing/asl-releases/commit/ca82a7ebc9b946966db37031f9afa8cd58429965) Adding more stuff to .gitignire
- [`3c31e5e0`](https://github.com/flamewing/asl-releases/commit/3c31e5e063902975377175175f498979db878f97) Fixing some cases of misleading indentation
- [`935b53ac`](https://github.com/flamewing/asl-releases/commit/935b53ac0927c147248b97c22243488ce19f2cba) Improve whitespace trimming used by M68K effective-address parsing.
- [`38eccef4`](https://github.com/flamewing/asl-releases/commit/38eccef4d55e41ff7dccb6989894731d9c350190) Trying out MacOSX 32-bit builds
- [`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938) Revert the unsupported 32-bit macOS build experiment.

### snapshot-snapshot-v1.4.2.20210628

Boundary: [`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938) to [`snapshot-snapshot-v1.4.2.20210628`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210628) ([`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/3e31e255c1f4ed080442899608614c81d115c938...3e31e255c1f4ed080442899608614c81d115c938).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-snapshot-v1.4.2.20210629

Boundary: [`3e31e255`](https://github.com/flamewing/asl-releases/commit/3e31e255c1f4ed080442899608614c81d115c938) to [`snapshot-snapshot-v1.4.2.20210629`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210629) ([`9ba75c50`](https://github.com/flamewing/asl-releases/commit/9ba75c5025181bb9c84dec0c05c786460de3d56f)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/3e31e255c1f4ed080442899608614c81d115c938...9ba75c5025181bb9c84dec0c05c786460de3d56f).

Make IFDEF and IFNDEF recognize macros and functions as well as ordinary symbols.

Fork changes (1 commit):

- [`9ba75c50`](https://github.com/flamewing/asl-releases/commit/9ba75c5025181bb9c84dec0c05c786460de3d56f) Making ifdef/ifndef work with macros and functions

### snapshot-snapshot-v1.4.2.20210701

Boundary: [`9ba75c50`](https://github.com/flamewing/asl-releases/commit/9ba75c5025181bb9c84dec0c05c786460de3d56f) to [`snapshot-snapshot-v1.4.2.20210701`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210701) ([`d4336629`](https://github.com/flamewing/asl-releases/commit/d43366291b9b550fda08e8ea67cd564f3176df4c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/9ba75c5025181bb9c84dec0c05c786460de3d56f...d43366291b9b550fda08e8ea67cd564f3176df4c).

Replace the M68K effective-address parser, complete its remaining addressing forms, and remove special treatment of function expressions. Extend ignored test artifacts.

Fork changes (4 commits):

- [`2b0662ea`](https://github.com/flamewing/asl-releases/commit/2b0662eac4a86880193f9f885b4f6ef9e4d1df0b) Ignoring temp test files as well
- [`fb7feb37`](https://github.com/flamewing/asl-releases/commit/fb7feb374dab43cedcf889966c8e17fa2ebe6bc3) Implementing a proper effective address parser
- [`59b7c4ee`](https://github.com/flamewing/asl-releases/commit/59b7c4ee82450a3bf3130c43c39bc68e6d250bf9) Getting all other addressing modes working
- [`d4336629`](https://github.com/flamewing/asl-releases/commit/d43366291b9b550fda08e8ea67cd564f3176df4c) Removed function special casing

### snapshot-snapshot-v1.4.2.20210702

Boundary: [`d4336629`](https://github.com/flamewing/asl-releases/commit/d43366291b9b550fda08e8ea67cd564f3176df4c) to [`snapshot-snapshot-v1.4.2.20210702`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210702) ([`c600e1a5`](https://github.com/flamewing/asl-releases/commit/c600e1a5cd984afe441f173287dd7e602d44395c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/d43366291b9b550fda08e8ea67cd564f3176df4c...c600e1a5cd984afe441f173287dd7e602d44395c).

Accept SLL and SL1 as aliases of Z80 SLIA. Add automatic integer-to-string conversion for symbol construction. Correct skipped repetition-block termination and recognize ELSEC/ENDC conditional aliases.

Fork changes (3 commits):

- [`78d8f22f`](https://github.com/flamewing/asl-releases/commit/78d8f22f40836720e7ed41e183d72292613606ea) Making z80 accept sll/sl1 as aliases to slia
- [`d514941f`](https://github.com/flamewing/asl-releases/commit/d514941f96c0fc192d3daad3c68709e8d93ccef2) Added auto-stringification of ints for symbols
- [`c600e1a5`](https://github.com/flamewing/asl-releases/commit/c600e1a5cd984afe441f173287dd7e602d44395c) Fix skipped repetition-block terminators and recognize ELSEC/ENDC conditional aliases.

### snapshot-snapshot-v1.4.2.20210703

Boundary: [`c600e1a5`](https://github.com/flamewing/asl-releases/commit/c600e1a5cd984afe441f173287dd7e602d44395c) to [`snapshot-snapshot-v1.4.2.20210703`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210703) ([`b2d5ed87`](https://github.com/flamewing/asl-releases/commit/b2d5ed87c2cff8e4f0cacf757752be8cb368e123)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/c600e1a5cd984afe441f173287dd7e602d44395c...b2d5ed87c2cff8e4f0cacf757752be8cb368e123).

Fix PHASE program-counter handling, including signed M68K addresses. Correct -maxinclevel handling. Support alternate REG and LABEL directive syntax.

Fork changes (3 commits):

- [`39d71712`](https://github.com/flamewing/asl-releases/commit/39d71712467df6e73aba9ce72d332db69c166dfb) Fixing phase
- [`0cf1752b`](https://github.com/flamewing/asl-releases/commit/0cf1752b18b82cd08b7a0234679a10deaed74181) A couple fixes for -maxinclevel
- [`b2d5ed87`](https://github.com/flamewing/asl-releases/commit/b2d5ed87c2cff8e4f0cacf757752be8cb368e123) Making reg and label support alternate syntax

### snapshot-snapshot-v1.4.2.20210705

Boundary: [`b2d5ed87`](https://github.com/flamewing/asl-releases/commit/b2d5ed87c2cff8e4f0cacf757752be8cb368e123) to [`snapshot-snapshot-v1.4.2.20210705`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210705) ([`4ec2f115`](https://github.com/flamewing/asl-releases/commit/4ec2f115bffeac7b9ca1def1f263d5a606831e9b)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/b2d5ed87c2cff8e4f0cacf757752be8cb368e123...4ec2f115bffeac7b9ca1def1f263d5a606831e9b).

Fix SWITCH handling in unevaluated conditional contexts and temporary-symbol behavior.

Fork changes (2 commits):

- [`4318148c`](https://github.com/flamewing/asl-releases/commit/4318148c4d8a4c9bed27767c3cf794863b1d9046) Fixing switch in unevaluated contexts
- [`4ec2f115`](https://github.com/flamewing/asl-releases/commit/4ec2f115bffeac7b9ca1def1f263d5a606831e9b) Fixing temporary symbols

### snapshot-snapshot-v1.4.2.20210706

Boundary: [`4ec2f115`](https://github.com/flamewing/asl-releases/commit/4ec2f115bffeac7b9ca1def1f263d5a606831e9b) to [`snapshot-snapshot-v1.4.2.20210706`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210706) ([`68d40439`](https://github.com/flamewing/asl-releases/commit/68d40439679ac9af1f24a783188cd4df900b6d0d)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4ec2f115bffeac7b9ca1def1f263d5a606831e9b...68d40439679ac9af1f24a783188cd4df900b6d0d).

Correct LABEL interactions, preserve ASFLAGS in tests, clean up the test runner, and change -q command-line handling to match -x conventions. A later batch repairs MESSAGE output affected by this quiet-mode change.

Fork changes (4 commits):

- [`9ef0e570`](https://github.com/flamewing/asl-releases/commit/9ef0e5704981fce17df3eecef49162fe5ea945c3) Cleaning up testall script
- [`98f4eb01`](https://github.com/flamewing/asl-releases/commit/98f4eb0185fba52069e67f78864efcef4e72f67b) Missing asflags in tests
- [`c8d6b3e4`](https://github.com/flamewing/asl-releases/commit/c8d6b3e44369b7d5878f5852fbfd5c1742606eb3) Fixing label interactions
- [`68d40439`](https://github.com/flamewing/asl-releases/commit/68d40439679ac9af1f24a783188cd4df900b6d0d) Making -q work like -x

### snapshot-snapshot-v1.4.2.20210707

Boundary: [`68d40439`](https://github.com/flamewing/asl-releases/commit/68d40439679ac9af1f24a783188cd4df900b6d0d) to [`snapshot-snapshot-v1.4.2.20210707`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210707) ([`cafa1f10`](https://github.com/flamewing/asl-releases/commit/cafa1f10a92673216fa80a9553b32963739a7af2)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/68d40439679ac9af1f24a783188cd4df900b6d0d...cafa1f10a92673216fa80a9553b32963739a7af2).

Add IRPN, repair its initial crash, and escape backslashes during IRPC substitution. Make headers usable independently and add missing test documentation. Improve Windows test execution and integrate Coverity scanning.

Fork changes (8 commits):

- [`bc631a6d`](https://github.com/flamewing/asl-releases/commit/bc631a6d8b9faf96cef718f546c8edaed51808bd) Fixing tests on Windows
- [`468d9c13`](https://github.com/flamewing/asl-releases/commit/468d9c1320732c0e78a06090bfeee2c9a80ec5d2) Removing useless cat
- [`abd4bea2`](https://github.com/flamewing/asl-releases/commit/abd4bea28a4354912601ce0c41ef046d6019ddd0) Making headers work in isolation
- [`62f3de07`](https://github.com/flamewing/asl-releases/commit/62f3de0748da545145015990db5bd3bb4635a09b) Adding some missing test docs
- [`ef57e2b1`](https://github.com/flamewing/asl-releases/commit/ef57e2b1d0d9505ac5890ce26a72bbde7434b5e3) Adding IRPN command
- [`a472935a`](https://github.com/flamewing/asl-releases/commit/a472935ab8efd3e98bf7f74d344e64e2817e1d9a) Hooking up Coverity
- [`6647afe4`](https://github.com/flamewing/asl-releases/commit/6647afe4d2b0e704207019150c787840b05d15c5) Fixing segfault on irpn
- [`cafa1f10`](https://github.com/flamewing/asl-releases/commit/cafa1f10a92673216fa80a9553b32963739a7af2) Making IRPC escape backslash on substitution

### snapshot-snapshot-v1.4.2.20210708

Boundary: [`cafa1f10`](https://github.com/flamewing/asl-releases/commit/cafa1f10a92673216fa80a9553b32963739a7af2) to [`snapshot-snapshot-v1.4.2.20210708`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210708) ([`44d4037f`](https://github.com/flamewing/asl-releases/commit/44d4037fc60339fd7c26a1ffa0218fe2aef55886)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/cafa1f10a92673216fa80a9553b32963739a7af2...44d4037fc60339fd7c26a1ffa0218fe2aef55886).

Document migration considerations for existing disassemblies while merging original AS build 205. The upstream import is context for the fork changes, not a fork feature.

Fork changes (1 commit):

- [`44d4037f`](https://github.com/flamewing/asl-releases/commit/44d4037fc60339fd7c26a1ffa0218fe2aef55886) Updating readme with upgrading instructions

Upstream synchronization:

- [`a6a001f2`](https://github.com/flamewing/asl-releases/commit/a6a001f23c528df8029b255b4cd523029d90d912) Merge upstream AS history; follow the linked merge for its imported revisions and conflict resolutions.

### snapshot-snapshot-v1.4.2.20210720

Boundary: [`44d4037f`](https://github.com/flamewing/asl-releases/commit/44d4037fc60339fd7c26a1ffa0218fe2aef55886) to [`snapshot-snapshot-v1.4.2.20210720`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210720) ([`4a82beaa`](https://github.com/flamewing/asl-releases/commit/4a82beaabe94a78990ce26f5e9608b656e8e9e4d)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/44d4037fc60339fd7c26a1ffa0218fe2aef55886...4a82beaabe94a78990ce26f5e9608b656e8e9e4d).

Document the include-search behavior of nested includes.

Fork changes (1 commit):

- [`4a82beaa`](https://github.com/flamewing/asl-releases/commit/4a82beaabe94a78990ce26f5e9608b656e8e9e4d) Adding remark about nested includes

### snapshot-snapshot-v1.4.2.20210727

Boundary: [`4a82beaa`](https://github.com/flamewing/asl-releases/commit/4a82beaabe94a78990ce26f5e9608b656e8e9e4d) to [`snapshot-snapshot-v1.4.2.20210727`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210727) ([`a8bfa90c`](https://github.com/flamewing/asl-releases/commit/a8bfa90c0f9b000eef9cc3002aa94ced83174c36)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4a82beaabe94a78990ce26f5e9608b656e8e9e4d...a8bfa90c0f9b000eef9cc3002aa94ced83174c36).

Restore MESSAGE output with -q after the earlier command-line change and remove an unused parameter. Import original AS builds 206 and 207.

Fork changes (2 commits):

- [`5ff5d91d`](https://github.com/flamewing/asl-releases/commit/5ff5d91dd737fef66c942c9e2ff9db458b5bf6c6) Fixing message with -q (was broken by 68d40439)
- [`65c3ebf8`](https://github.com/flamewing/asl-releases/commit/65c3ebf812e18422a0cb85091cec6bba9f305cc4) Removing unused parameter

Upstream synchronization:

- [`39ccdaab`](https://github.com/flamewing/asl-releases/commit/39ccdaab4932159c4f653fa6d9be409060a1e240) Import original AS build 206.
- [`a8bfa90c`](https://github.com/flamewing/asl-releases/commit/a8bfa90c0f9b000eef9cc3002aa94ced83174c36) Import original AS build 207.

### snapshot-snapshot-v1.4.2.20210906

Boundary: [`a8bfa90c`](https://github.com/flamewing/asl-releases/commit/a8bfa90c0f9b000eef9cc3002aa94ced83174c36) to [`snapshot-snapshot-v1.4.2.20210906`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210906) ([`5975e073`](https://github.com/flamewing/asl-releases/commit/5975e0730a0bd7c12dadbc111a91d489c6690461)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/a8bfa90c0f9b000eef9cc3002aa94ced83174c36...5975e0730a0bd7c12dadbc111a91d489c6690461).

Make ENDR a full synonym of ENDM.

Fork changes (1 commit):

- [`5975e073`](https://github.com/flamewing/asl-releases/commit/5975e0730a0bd7c12dadbc111a91d489c6690461) Making endr a full synonym to endm

### snapshot-snapshot-v1.4.2.20210908

Boundary: [`5975e073`](https://github.com/flamewing/asl-releases/commit/5975e0730a0bd7c12dadbc111a91d489c6690461) to [`snapshot-snapshot-v1.4.2.20210908`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20210908) ([`f58a703e`](https://github.com/flamewing/asl-releases/commit/f58a703edceca4256e0d6bed14e9dc8ce022cee2)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/5975e0730a0bd7c12dadbc111a91d489c6690461...f58a703edceca4256e0d6bed14e9dc8ce022cee2).

Prevent snapshot release drafts from accumulating unnecessarily.

Fork changes (1 commit):

- [`f58a703e`](https://github.com/flamewing/asl-releases/commit/f58a703edceca4256e0d6bed14e9dc8ce022cee2) Fixing release draft spam

### snapshot-snapshot-v1.4.2.20211010

Boundary: [`f58a703e`](https://github.com/flamewing/asl-releases/commit/f58a703edceca4256e0d6bed14e9dc8ce022cee2) to [`snapshot-snapshot-v1.4.2.20211010`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20211010) ([`088b12ca`](https://github.com/flamewing/asl-releases/commit/088b12cac86396aaab9708b98ce52d3c67f6049c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/f58a703edceca4256e0d6bed14e9dc8ce022cee2...088b12cac86396aaab9708b98ce52d3c67f6049c).

Import original AS builds 208 through 211. This batch contains no additional fork-specific commit.

Upstream synchronization:

- [`4f6af62e`](https://github.com/flamewing/asl-releases/commit/4f6af62eadbd42fb3d4367716769b3333a7f4fd4) Import original AS build 208.
- [`4e83a0fb`](https://github.com/flamewing/asl-releases/commit/4e83a0fb06e631657f4334606382c8649956a1da) Import original AS build 209.
- [`9d555b5e`](https://github.com/flamewing/asl-releases/commit/9d555b5ee11b2ea2506d3cba1c164815ee092f58) Import original AS build 210.
- [`088b12ca`](https://github.com/flamewing/asl-releases/commit/088b12cac86396aaab9708b98ce52d3c67f6049c) Import original AS build 211.

### snapshot-snapshot-v1.4.2.20211121

Boundary: [`088b12ca`](https://github.com/flamewing/asl-releases/commit/088b12cac86396aaab9708b98ce52d3c67f6049c) to [`snapshot-snapshot-v1.4.2.20211121`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20211121) ([`34045459`](https://github.com/flamewing/asl-releases/commit/34045459296251eec004b89db0be5b101040775c)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/088b12cac86396aaab9708b98ce52d3c67f6049c...34045459296251eec004b89db0be5b101040775c).

Fix a diagnostic-text typo and support macro parameter substitution using backslash-delimited parameter names. Import original AS build 212.

Fork changes (2 commits):

- [`d22d757d`](https://github.com/flamewing/asl-releases/commit/d22d757d0c5f692aae7ae51f511c302e689ada3c) Fixing typo in messages.
- [`e5ea4f00`](https://github.com/flamewing/asl-releases/commit/e5ea4f007f9c01a16bdf063079b5d46e5ab11ba6) Making AS support \\param\\ macro replacements

Upstream synchronization:

- [`34045459`](https://github.com/flamewing/asl-releases/commit/34045459296251eec004b89db0be5b101040775c) Import original AS build 212.

### snapshot-snapshot-v1.4.2.20240412

Boundary: [`34045459`](https://github.com/flamewing/asl-releases/commit/34045459296251eec004b89db0be5b101040775c) to [`snapshot-snapshot-v1.4.2.20240412`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240412) ([`e4dd4301`](https://github.com/flamewing/asl-releases/commit/e4dd43016e0d31986fd3ddf7bd36f828630a1065)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/34045459296251eec004b89db0be5b101040775c...e4dd43016e0d31986fd3ddf7bd36f828630a1065).

Fix bogus M68K "short address not allowed" errors. Add and refine the Emscripten/WebAssembly target and cross-compilation script, allow tests when cross-compiling, repair SHA-1 compilation, and update CI badges.

Fork changes (7 commits):

- [`f5109919`](https://github.com/flamewing/asl-releases/commit/f5109919334525b7acc0aafcc4c4d7c96ae631ab) 68k: Fix bogus 'short address not allowed' errors
- [`a739088c`](https://github.com/flamewing/asl-releases/commit/a739088c65406dc74648d0b9b22e796be5e0d938) Add WASM compile target
- [`b9c2eea7`](https://github.com/flamewing/asl-releases/commit/b9c2eea79eba036a07d8bd5d1fede2c0304db237) Refine the Emscripten code a bit.
- [`a0d85a80`](https://github.com/flamewing/asl-releases/commit/a0d85a8058fb4a6d455a7b277b3716ace410a1a7) Improve Emscripten build script.
- [`faf22ba0`](https://github.com/flamewing/asl-releases/commit/faf22ba0ded91e4d72a27b31b1da7eb287c40c81) Fix sha1.c failing to compile.
- [`8ea857bb`](https://github.com/flamewing/asl-releases/commit/8ea857bbed6de73c9260daef6f408a9eb2855a8c) Get `make test` to work with cross-compilation.
- [`e4dd4301`](https://github.com/flamewing/asl-releases/commit/e4dd43016e0d31986fd3ddf7bd36f828630a1065) Fixing badges

### snapshot-snapshot-v1.4.2.20240413

Boundary: [`e4dd4301`](https://github.com/flamewing/asl-releases/commit/e4dd43016e0d31986fd3ddf7bd36f828630a1065) to [`snapshot-snapshot-v1.4.2.20240413`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240413) ([`0b07a579`](https://github.com/flamewing/asl-releases/commit/0b07a579e25be25bf3e54c3794c8c75419702c27)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/e4dd43016e0d31986fd3ddf7bd36f828630a1065...0b07a579e25be25bf3e54c3794c8c75419702c27).

Add the Coverity Scan workflow badge.

Fork changes (1 commit):

- [`0b07a579`](https://github.com/flamewing/asl-releases/commit/0b07a579e25be25bf3e54c3794c8c75419702c27) Forgot Coverity Scan badge

### snapshot-snapshot-v1.4.2.20240416

Boundary: [`0b07a579`](https://github.com/flamewing/asl-releases/commit/0b07a579e25be25bf3e54c3794c8c75419702c27) to [`snapshot-snapshot-v1.4.2.20240416`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240416) ([`f3e164f1`](https://github.com/flamewing/asl-releases/commit/f3e164f1af17e7596433052d9aa9903c2ef0611e)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/0b07a579e25be25bf3e54c3794c8c75419702c27...f3e164f1af17e7596433052d9aa9903c2ef0611e).

Add the CodeQL workflow. The CMake migration belongs to the following tag interval because this tag points to the earlier workflow commit, regardless of author dates.

Fork changes (1 commit):

- [`f3e164f1`](https://github.com/flamewing/asl-releases/commit/f3e164f1af17e7596433052d9aa9903c2ef0611e) Create codeql.yml

### snapshot-snapshot-v1.4.2.20240821

Boundary: [`f3e164f1`](https://github.com/flamewing/asl-releases/commit/f3e164f1af17e7596433052d9aa9903c2ef0611e) to [`snapshot-snapshot-v1.4.2.20240821`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240821) ([`1df09354`](https://github.com/flamewing/asl-releases/commit/1df0935489ac8ba99ea11bdffd5eb65406375d5b)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/f3e164f1af17e7596433052d9aa9903c2ef0611e...1df0935489ac8ba99ea11bdffd5eb65406375d5b).

Replace the legacy build configuration with CMake, add parallel CTest integration, host-tool support for cross-compilation, Emscripten/WebAssembly builds, MSVC support, platform/architecture detection and locale/header checks. Extend CI to FreeBSD and OmniOS, adjust macOS/Xcode experiments, build warnings, compiler compatibility and release packaging, and install the license.

Remove obsolete DOS, MUNIX and old Solaris support and the copyright-string anti-tamper trap. Rename endian support to as_endian, rework endian detection, repair bounded string concatenation and printf formats, fix an accidental bitwise operator and integer-size mismatch, and modernize headers, reserved names, formatting and editor/static-analysis configuration.

Convert the manual from TeX to Markdown, correct spelling, translations and table formatting, and generate HTML/ODT documentation with Pandoc. Repair documentation dependencies and generation races. Fix stale forward-reference handling for expressions required on the first pass, increase the input-line limit from 256 to 1024, remove mkdepend from packages, update CI dependencies and repair snapshot asset paths. Merge upstream history without treating its features as fork additions.

Fork changes (96 commits):

- [`1f75be13`](https://github.com/flamewing/asl-releases/commit/1f75be1388bd78f6c261cd38afbaa606159d735a) Change build system to cmake
- [`600f72b1`](https://github.com/flamewing/asl-releases/commit/600f72b19149c73066a5e74a7fefba73c559629a) Making Emscripten/Wasm cross compile work
- [`10e2eca3`](https://github.com/flamewing/asl-releases/commit/10e2eca3f6cac85a9c86c97e0dc66cbab184d2f7) Adding CodeQL file
- [`3c1f0a08`](https://github.com/flamewing/asl-releases/commit/3c1f0a08d3d5785fc47a71250b34b9710df3151c) Fixing a few uses of sprintf to snprintf
- [`970e2334`](https://github.com/flamewing/asl-releases/commit/970e233454923b24073671acf84df7d049b306d7) Fixing non-Emsctipten cross builds
- [`0478db4d`](https://github.com/flamewing/asl-releases/commit/0478db4de0cbd8e3552401288fe3d945d24f8563) Missed a few legacy files
- [`c3465d37`](https://github.com/flamewing/asl-releases/commit/c3465d37ef162aaa8e5c13be6f1d6f508ad72daa) Using ctest for running tests in parallel
- [`a1a474fc`](https://github.com/flamewing/asl-releases/commit/a1a474fcd56be5b1a5101fc6678f65a980b658cc) Adding MSVC compilation support
- [`32c5cb6d`](https://github.com/flamewing/asl-releases/commit/32c5cb6dfca6b0414571425c5c6e42e60b8b079b) Updating codeql workflow
- [`47ea94d8`](https://github.com/flamewing/asl-releases/commit/47ea94d8bb5bb3026aba250eadc82dc60ea13443) Fixing badges
- [`6685aab1`](https://github.com/flamewing/asl-releases/commit/6685aab1de16c38a23f120d749db458996ed8209) Trying out FreeBSD and OmniOS CI
- [`e9b58537`](https://github.com/flamewing/asl-releases/commit/e9b58537ac9c289d6a18e4c4f2119088b598c403) Grouping of CI for cancellation
- [`7b65b995`](https://github.com/flamewing/asl-releases/commit/7b65b995e8a849433f5af4c2649ba7f8cab34628) Verbose output for tests on CI
- [`a1ad04d0`](https://github.com/flamewing/asl-releases/commit/a1ad04d0dfa15aded3807977b50d4c1d42a70894) Update readme and add XCode generator test
- [`816c7612`](https://github.com/flamewing/asl-releases/commit/816c7612d034b06f9fccb831189c52993601d303) Make CI do debug builds
- [`ba8df3f6`](https://github.com/flamewing/asl-releases/commit/ba8df3f67269c04f88f0e0a743323d9b785df09a) Remove anti-tamper trap on copyright string
- [`d873c9d6`](https://github.com/flamewing/asl-releases/commit/d873c9d6ab6cbe9813fe6cdc823901b62ab20347) Marking text files as such
- [`cb585614`](https://github.com/flamewing/asl-releases/commit/cb585614993eb35c38c8346015e585c8b7129b59) Getting rid of DOS stuff
- [`badad464`](https://github.com/flamewing/asl-releases/commit/badad4648e7eee5e13b3c428849d18a94859ad59) Getting rid of stuff related to MUNIX and old Solaris
- [`6ab7a813`](https://github.com/flamewing/asl-releases/commit/6ab7a8132007881b19feb8ffaffde47e4e76799f) fixup! Getting rid of DOS stuff
- [`a760f3cc`](https://github.com/flamewing/asl-releases/commit/a760f3cc0acf3b314392e6a3afe3013a2cc7da31) Using CMake to detect and include unistd.h
- [`a9fd02cf`](https://github.com/flamewing/asl-releases/commit/a9fd02cf03bc5401f8e16a40e405d38abdff8402) Using cmake for processor and architecture names
- [`c18763b1`](https://github.com/flamewing/asl-releases/commit/c18763b13ca844c0fbd10aa7ff10c3218f183ac7) Using CMake for locale stuff
- [`1dbad86b`](https://github.com/flamewing/asl-releases/commit/1dbad86b95540e36d1a1472389a59e7ad987201e) Simplifying sysdefs.h
- [`d15b0b0c`](https://github.com/flamewing/asl-releases/commit/d15b0b0c4ef071a0f7cc77c9657660745af271d1) Adding some custom targets for dependencies
- [`2a1efa03`](https://github.com/flamewing/asl-releases/commit/2a1efa0384e2c01662d6cff21d13fa519d1bd443) Fixing some issues in readme
- [`cfc406e4`](https://github.com/flamewing/asl-releases/commit/cfc406e40f4caf5591e3dbe3bc9a66e728db9a39) Fixing OmniOS build
- [`294a202b`](https://github.com/flamewing/asl-releases/commit/294a202baa1a4eac01f035d5eb8ab6a392948c22) Cleaning up reserved names
- [`da4293f5`](https://github.com/flamewing/asl-releases/commit/da4293f5a0a8da9f7726bd785940bfa382cbe9ed) Newer TCC does not, in fact, require this
- [`e21ed45d`](https://github.com/flamewing/asl-releases/commit/e21ed45d8c1a3a78d0289cefe687858bb93b150e) Disabling a few of the more spammy warnings
- [`2eb4d842`](https://github.com/flamewing/asl-releases/commit/2eb4d842b0b144a2f821bd381717f0239eb8a49c) Enable printf format checking
- [`1312e521`](https://github.com/flamewing/asl-releases/commit/1312e52116708fc443b5801c6b03d84b489d7c93) Renaming endian.{c,h} to as_endian.{c,h}
- [`8bdcda0d`](https://github.com/flamewing/asl-releases/commit/8bdcda0d56d52b314b4b90056e4d6e40c1683919) Some IWYU pragmas
- [`996b0e75`](https://github.com/flamewing/asl-releases/commit/996b0e7566871bcd9c387872c449df026e5f3d67) Reworking endian stuff
- [`7e24d981`](https://github.com/flamewing/asl-releases/commit/7e24d9814608885f442f19f489fb7b3b14174f5f) Fixing CodeQL alert
- [`f09dfad3`](https://github.com/flamewing/asl-releases/commit/f09dfad31694ca0ac64c67898098d4a2ca3f8289) Fixing length truncation in strmaxcat
- [`b7b6be73`](https://github.com/flamewing/asl-releases/commit/b7b6be731f62d67fd4fd9a0cb5d976208cf37ec5) Cleaning up most printf format string issues
- [`d954b7e9`](https://github.com/flamewing/asl-releases/commit/d954b7e9fb3a4c183cac62366049d8202630cfa9) Experimental cleanup of non-standard printf parameter
- [`f3ef5105`](https://github.com/flamewing/asl-releases/commit/f3ef5105eace5b9d7b3957d3f53f8344946f46ee) Compiling speed improvements
- [`2d398136`](https://github.com/flamewing/asl-releases/commit/2d398136b08743bc5483dd9542a1e9d880497bf5) Fixing OSX XCode build
- [`fdb06bc8`](https://github.com/flamewing/asl-releases/commit/fdb06bc86ea77bd4a909f035f26ead60aeb5b238) Disabling XCode for now
- [`625aba2d`](https://github.com/flamewing/asl-releases/commit/625aba2de52dfe2341469af4dbdab1cd594d4db7) Fixing mistaken use of & in place of &&
- [`ba2afd39`](https://github.com/flamewing/asl-releases/commit/ba2afd39449c390e2c28dafcdf6487e80954c0a0) Trying to disabling XCode again
- [`92583a9a`](https://github.com/flamewing/asl-releases/commit/92583a9a9367797d853ffadad540ff2a55b120b5) Cleaning up readme with URL references
- [`dfffaf37`](https://github.com/flamewing/asl-releases/commit/dfffaf372a9ff4132f8ff510a9dc7d6eae19acc8) Applying formatter to cmake stuff
- [`bb4e5ff0`](https://github.com/flamewing/asl-releases/commit/bb4e5ff09bc70a8e477425e8fde0ed686df4b392) Fix one c++ compatibility issue
- [`eab97238`](https://github.com/flamewing/asl-releases/commit/eab9723813a6c051d0321a0809254cccfcd1df52) Yeah, no, I won't support K&K compatibility
- [`e103084e`](https://github.com/flamewing/asl-releases/commit/e103084e02e0b6eebdfaa1d0c1cab146c15785ca) Updating .clangd
- [`cfe47672`](https://github.com/flamewing/asl-releases/commit/cfe476720324abd7642d35b6c7d9f1bd9f69ad71) Removing old build stuff from documentation
- [`363289a4`](https://github.com/flamewing/asl-releases/commit/363289a4c9e68739123dc080a4733c599329fead) Making headers work with clangd
- [`20f69521`](https://github.com/flamewing/asl-releases/commit/20f695218bf07120097fe7abbdebc388ecc1d55b) Cleanup and spellcheck of TeX
- [`99f7bd93`](https://github.com/flamewing/asl-releases/commit/99f7bd933de625f1da304902eb36856b20525770) Fixing libdir
- [`358ed03d`](https://github.com/flamewing/asl-releases/commit/358ed03d133cb4fd2ba7288ad385adb7a62ad9be) Converting documentation to markdown
- [`584cc19b`](https://github.com/flamewing/asl-releases/commit/584cc19b728ecd1ab3de008c720137d090988e0b) Spellchecking man pages
- [`04a9ceb9`](https://github.com/flamewing/asl-releases/commit/04a9ceb9887a20b71f6c1f6a7bffeb3d883b68bb) Fixing German and Spanish characters
- [`2ff949a7`](https://github.com/flamewing/asl-releases/commit/2ff949a730e03b887c1c32eb66bd2ca5c16f196f) Consistent table headings in docs
- [`23f0a9c6`](https://github.com/flamewing/asl-releases/commit/23f0a9c6a4ad9854f07ee8903a793b164e2f49e7) Improving table layouts
- [`3d21544c`](https://github.com/flamewing/asl-releases/commit/3d21544cd27a97ee5493c44f787396b769c4a10c) Consistent styling for µ
- [`82660664`](https://github.com/flamewing/asl-releases/commit/82660664bd87c0e6ed5ee75f3f84fd639f370545) Consistent styling for F²MC16LX
- [`f170c27f`](https://github.com/flamewing/asl-releases/commit/f170c27f170c59b0558ac03716789d842a8396d0) Fixing a few u -> µ
- [`82804263`](https://github.com/flamewing/asl-releases/commit/828042637a4ae70ea6982edd906a6d0b1483429f) White-space changes
- [`15459329`](https://github.com/flamewing/asl-releases/commit/154593290041d9ffe295a2c497e7fb61c9eb521b) More spelling
- [`e9cb7fb9`](https://github.com/flamewing/asl-releases/commit/e9cb7fb9ffb7852da5709900554c482a3f1291e4) More leftover German
- [`bab55595`](https://github.com/flamewing/asl-releases/commit/bab555958fb1a9ad68c6ff361572f8bc1e3cf034) Fixing quotes
- [`94ec19e4`](https://github.com/flamewing/asl-releases/commit/94ec19e4f32460c8f4fe4406bbc4b39dfe3497c8) Formatting
- [`b6d683d2`](https://github.com/flamewing/asl-releases/commit/b6d683d2ef2ff8342eeb7c3e8db7ccf4285b5eca) Highlighting fixes
- [`d1842f35`](https://github.com/flamewing/asl-releases/commit/d1842f3525396d2e9c4ba262a0daa19246d2354c) Fixing debug build
- [`d61d3a67`](https://github.com/flamewing/asl-releases/commit/d61d3a673e95f614460780e7fd01877a76fe14c6) Fixing doc generation stuff
- [`1c2ae213`](https://github.com/flamewing/asl-releases/commit/1c2ae213e0ad4febfd19784f2897a17f26fef79f) Add self-contained flag to comment
- [`6446da83`](https://github.com/flamewing/asl-releases/commit/6446da8300bdcc8f66ee621b4d15c678d9c73876) Re-adding and fixing add_compile_options_safe
- [`7226b332`](https://github.com/flamewing/asl-releases/commit/7226b33237b4e801ef21f7f225ffce650c76d13c) Removing trailing whitespace from includes
- [`4ab419c7`](https://github.com/flamewing/asl-releases/commit/4ab419c729069105c7751e78fdf3d69230506ce8) Generating html and odt documentation if Pandoc is available
- [`913ded57`](https://github.com/flamewing/asl-releases/commit/913ded57b36762721de8e8c76707359f6badeed3) Inherit environment variables from Windows
- [`34aeb5d9`](https://github.com/flamewing/asl-releases/commit/34aeb5d91b31588d37917add79f40e01b0ddd00e) Giving name to Pandoc install action
- [`20746c63`](https://github.com/flamewing/asl-releases/commit/20746c63b16492d442d79f91a68969fc0abb09b1) Format tweaks
- [`67c221f7`](https://github.com/flamewing/asl-releases/commit/67c221f74032958e7bcd202ce4a79776c4da30e5) Minor changes to docs
- [`50197d25`](https://github.com/flamewing/asl-releases/commit/50197d25b9ed205e3d4c8e78ee79b81d35704b33) More trailing whitespace cleanup
- [`64f79388`](https://github.com/flamewing/asl-releases/commit/64f79388595054eb585a3218489662acea9b80e4) Removing empty CSS rules
- [`88d3acbe`](https://github.com/flamewing/asl-releases/commit/88d3acbe5fa1ef367b52aa708f46994278c1dd95) Fixing asl-tidy and asl-format targets
- [`9834c78c`](https://github.com/flamewing/asl-releases/commit/9834c78cb01547d80ef3eb6e451c97555b82c8be) IWYU cleanup
- [`37de3167`](https://github.com/flamewing/asl-releases/commit/37de3167efc61f6e5ce33308bc12d795b9e75d46) Making macro require semicolon
- [`889aa581`](https://github.com/flamewing/asl-releases/commit/889aa581ba13cf37fbc36ce401b10058a0b57152) Reformatting everything with .clang-format v18.1.4
- [`d4141fcd`](https://github.com/flamewing/asl-releases/commit/d4141fcd80577ce14d2bba3a49c0e96680b25887) Mismatched integer sizes in loop check
- [`4d7cd6a4`](https://github.com/flamewing/asl-releases/commit/4d7cd6a4db44dadba68beea9134378b0c4808f65) Install COPYING as well
- [`e9a452cc`](https://github.com/flamewing/asl-releases/commit/e9a452cc3a6c9d11ffe4bdd7045958284c29a569) Removed stray line
- [`99bf08b4`](https://github.com/flamewing/asl-releases/commit/99bf08b4125b689c1cade39a8384db7e36d6363a) Removing some spammy and unfixable warnings
- [`2866b9f0`](https://github.com/flamewing/asl-releases/commit/2866b9f0abf434e7f6713716195faf3e067d9efe) Fixing race condition on doc generation
- [`697611fd`](https://github.com/flamewing/asl-releases/commit/697611fdf50d5e2cd4db7f905459b89d275a75ae) Setting git attributes for files
- [`fcce628f`](https://github.com/flamewing/asl-releases/commit/fcce628f37768ecb692cc8dba9a09f8b33605c0b) cninja package was renamed to ninja
- [`721a327a`](https://github.com/flamewing/asl-releases/commit/721a327a803585e2d1f59acac93cbd0768213d06) Correctly signal 'must be evaluable on first pass' error when using stale forward references
- [`e76f0af9`](https://github.com/flamewing/asl-releases/commit/e76f0af98ca3d13a1fe7c190498eacedc75b6ff5) cninja package was renamed to ninja
- [`1e8d4171`](https://github.com/flamewing/asl-releases/commit/1e8d417185c43995a6a719925723f9e234a36381) Removing mkdepend from package
- [`99fdda7e`](https://github.com/flamewing/asl-releases/commit/99fdda7e109b9c2933e74f0170ef901b2cd40b19) Raise line length limit from 256 to 1024.
- [`2eb4f34d`](https://github.com/flamewing/asl-releases/commit/2eb4f34d7de305b24cabcac8501abf9dadf4f371) Add a missing path separator in snapshots.yml
- [`506bb960`](https://github.com/flamewing/asl-releases/commit/506bb960a69043cd738b6f3abb01ccdb9a253f03) Correct the asset_path in snapshots.yml.
- [`1df09354`](https://github.com/flamewing/asl-releases/commit/1df0935489ac8ba99ea11bdffd5eb65406375d5b) Remove redundant copy from snapshots.yml.

Upstream synchronization:

- [`049b7fdc`](https://github.com/flamewing/asl-releases/commit/049b7fdc7f1a415f116d508c4eb098fe48b5f22b) Merge upstream AS history; follow the linked merge for its imported revisions and conflict resolutions.

### snapshot-snapshot-v1.4.2.20240912

Boundary: [`1df09354`](https://github.com/flamewing/asl-releases/commit/1df0935489ac8ba99ea11bdffd5eb65406375d5b) to [`snapshot-snapshot-v1.4.2.20240912`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240912) ([`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1df0935489ac8ba99ea11bdffd5eb65406375d5b...077572d160aa03919af3736e5fb0ec4831979045).

Avoid premature errors for unresolved M68K displacements and report the appropriate out-of-range PC-relative diagnostic.

Fork changes (2 commits):

- [`87a38616`](https://github.com/flamewing/asl-releases/commit/87a38616236283f7c4fbfe03f0332a216e74f795) Fix forward references causing errors in displacements.
- [`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045) Make out-of-range PC-relative operands print the correct error.

### snapshot-snapshot-v1.4.2.20240913

Boundary: [`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045) to [`snapshot-snapshot-v1.4.2.20240913`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20240913) ([`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/077572d160aa03919af3736e5fb0ec4831979045...077572d160aa03919af3736e5fb0ec4831979045).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-snapshot-v1.4.2.20241029

Boundary: [`077572d1`](https://github.com/flamewing/asl-releases/commit/077572d160aa03919af3736e5fb0ec4831979045) to [`snapshot-snapshot-v1.4.2.20241029`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20241029) ([`b5b3ebc9`](https://github.com/flamewing/asl-releases/commit/b5b3ebc975f8383cd6374b37dacf743f70c11285)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/077572d160aa03919af3736e5fb0ec4831979045...b5b3ebc975f8383cd6374b37dacf743f70c11285).

Work around a Windows GCC miscompilation and accept BYTE wherever BYT is supported.

Fork changes (2 commits):

- [`c2f9f0c4`](https://github.com/flamewing/asl-releases/commit/c2f9f0c4628fa461d249513baa55f969f80c92c9) Working around miscompilation in GCC on Windows
- [`b5b3ebc9`](https://github.com/flamewing/asl-releases/commit/b5b3ebc975f8383cd6374b37dacf743f70c11285) Adding "byte" as a synonym for "byt" whenever the latter is available.

### snapshot-snapshot-v1.4.2.20250716

Boundary: [`b5b3ebc9`](https://github.com/flamewing/asl-releases/commit/b5b3ebc975f8383cd6374b37dacf743f70c11285) to [`snapshot-snapshot-v1.4.2.20250716`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20250716) ([`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/b5b3ebc975f8383cd6374b37dacf743f70c11285...1b65169ab5de6e0a3a61cdaff7818bb515171837).

Repair MSVC path construction, refresh CI, fix Emscripten and FreeBSD workflows, and remove the snapshot workflow dependency on missing apt-fast.

Fork changes (5 commits):

- [`7aa10706`](https://github.com/flamewing/asl-releases/commit/7aa107060c2cc42fffc70db875545fcb582981e7) Fix MSVC builds not forming valid paths.
- [`1a80ef84`](https://github.com/flamewing/asl-releases/commit/1a80ef841832e7dd7729db4ff98f9344383428a3) Cleaning up CI
- [`ec708835`](https://github.com/flamewing/asl-releases/commit/ec708835322c2b8193b9710893339bc8e60bdeed) Fixing emscripten CI
- [`eb6e2c3f`](https://github.com/flamewing/asl-releases/commit/eb6e2c3fb925e14f1bfe2efcf5518db2d56e0790) Hopefully fixing FreeBSD CI
- [`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837) Fixing snapshots due to missing apt-fast.

### snapshot-snapshot-v1.4.2.20250717

Boundary: [`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837) to [`snapshot-snapshot-v1.4.2.20250717`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-snapshot-v1.4.2.20250717) ([`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1b65169ab5de6e0a3a61cdaff7818bb515171837...1b65169ab5de6e0a3a61cdaff7818bb515171837).

No new commits: this tag points to the same revision as the preceding tag.

### snapshot-v1.4.2.20261010-run38090250819

Boundary: [`1b65169a`](https://github.com/flamewing/asl-releases/commit/1b65169ab5de6e0a3a61cdaff7818bb515171837) to [`snapshot-v1.4.2.20261010-run38090250819`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261010-run38090250819) ([`4d342b19`](https://github.com/flamewing/asl-releases/commit/4d342b193bd116785dce6fdd730860c563bc57bb)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/1b65169ab5de6e0a3a61cdaff7818bb515171837...4d342b193bd116785dce6fdd730860c563bc57bb).

Synchronize with original AS builds 213 through 313, retaining and repairing fork behavior during the imports. New targets and instructions supplied by those upstream builds remain upstream changes; see [the original AS changelog](../changelog) for that history.

Correct unbalanced-parenthesis handling, add a warning for a newly created CODEPAGE read before modification, repair stale first-pass checks, and support REG on the Z80 family. Replace symbol-table lookup with hashing and add macro line cursors, single-pass MACRO/IRP replacement, correct high-count and escaped macro parameters, initialize copied macro state, and prevent SHIFT from making the argument count negative.

Reduce repeated expression-operator and delimiter scans, reject impossible integer/FPU register and floating-literal spellings early, reuse known string spans and lengths, batch listing writes, and cache locale-aware whitespace classification. Propagate borrowed views through recursive expression parsing with bounded string helpers. Keep compiler-specific optimizations and profiling/benchmark tools documented in [the benchmark guide](../benchmarks/README.md); experimental measurements and discarded variants are recorded without implying that every experiment became an active optimization.

Repair truncated prepends and bounded SysString formatting, zero-valued and high-byte CHARSET mappings, skipped macro terminators, signed M68K PHASE addresses, temporary labels in compound expressions, symbol fallback when integer syntax does not match, restricted-width complements, Intel hex-zero width, target-precision subnormal floating-point rounding, TMS340xx FIELD bounds and single-address mapping overlaps. Index forward references by name and scope, initialize V60/ColdFire operand flags, fix 3203x register validation and empty code-chunk allocation, and address sanitizer-reported symbol leaks and empty-pointer operations.

Remove the remaining legacy build system, add profiling configurations and Android CI, configure Emscripten stack size, and improve documentation dependencies. Gate native snapshots on tests, retain failure logs, pin actions/cross-compilers, restrict workflow credentials, skip duplicate scheduled work only after successful runs, identify Coverity submissions by SHA, and give snapshot releases unique run identifiers with retry/ownership checks. Refresh copyright years, formatting, tracked-file types and LF normalization.

Fork changes (88 commits):

- [`989a07bd`](https://github.com/flamewing/asl-releases/commit/989a07bd524ce3898df62ddb79c79ba705ac7b9c) Fixing unbalanced parenthesis checks
- [`94eb6f1f`](https://github.com/flamewing/asl-releases/commit/94eb6f1f392ca21c8f82913dffe2a6e36f5f5553) Formatting
- [`3ed26556`](https://github.com/flamewing/asl-releases/commit/3ed26556da1bec6f0a83d1d341a3c68db2acd023) Added warning for single-argument `codepage` instruction if the created codepage is read before being modified. This likely signals a missing codepage definition.
- [`18499be2`](https://github.com/flamewing/asl-releases/commit/18499be27bed41f68a3d01515b7bb858a72fdcfb) Fixing "Correctly signal 'must be evaluable on first pass' error when using stale forward references"
- [`8dd65110`](https://github.com/flamewing/asl-releases/commit/8dd651105449cbefe3510fe5c561d1535065327d) Hash tables for symbol tables giving ~10% speed boost
- [`df4dd0b4`](https://github.com/flamewing/asl-releases/commit/df4dd0b49712ba34e82ff6df234734186a330638) Trying to fix MSVC CI
- [`9db55e27`](https://github.com/flamewing/asl-releases/commit/9db55e27b851a8b87a6c60ca4ed78016e874f403) Fixing build for newer versions of EMScripten
- [`c4fad804`](https://github.com/flamewing/asl-releases/commit/c4fad804ca7f1dadcbd992aea2549da9437a2d3f) Formatting
- [`0846c780`](https://github.com/flamewing/asl-releases/commit/0846c780f8fc2ec0114c1520f0f8aad3aa8f638b) Cleaning up GCC-specific extensions
- [`0730cdf2`](https://github.com/flamewing/asl-releases/commit/0730cdf2b6156384e8bb291420704e18814f536e) Implementing `reg` on z80 family
- [`c1e08525`](https://github.com/flamewing/asl-releases/commit/c1e085257307575e3a7ab75512b0334546d199ca) Implement macro line management and profiling enhancements
- [`7c7381f9`](https://github.com/flamewing/asl-releases/commit/7c7381f94d9973a6c1a2e740f796411fdfebc917) Fixing macro parameter replacement bug with high parameter counts and escaped parameters
- [`aba7c6d7`](https://github.com/flamewing/asl-releases/commit/aba7c6d7df6062928c2fa8618314ce031734346a) Implement single-pass macro and IRP expansion with enhanced token replacement
- [`35cca6bf`](https://github.com/flamewing/asl-releases/commit/35cca6bffab8138024a9161b146d6d767ec5a0e9) Fixing SHIFT edge case which could render argument count negative
- [`ad1a1728`](https://github.com/flamewing/asl-releases/commit/ad1a1728edb1c7f11945c313134af21e8eb2f6e1) Implementing line cursor in MACRO_Processor to avoid repeatedly traversing a linked list
- [`37407332`](https://github.com/flamewing/asl-releases/commit/374073321901ad6155c46b4de04ee9452ff14df7) Optimization experiments
- [`076ba10b`](https://github.com/flamewing/asl-releases/commit/076ba10bc190221f21def8bc97a8d2aca4e260d5) Optimize single-character comment delimiter scanning
- [`e3719dbd`](https://github.com/flamewing/asl-releases/commit/e3719dbd314d8ca9d602c2e0104874c340396019) Optimization experiments
- [`590abc57`](https://github.com/flamewing/asl-releases/commit/590abc5789f7ee8b084db07c1e932e293881fe8a) Skip quote-aware scanning when a single delimiter is absent
- [`0a79aa87`](https://github.com/flamewing/asl-releases/commit/0a79aa871228ec549e7371dfb5d5d8d81c85edbf) Index expression operators by their first character
- [`495bd2f0`](https://github.com/flamewing/asl-releases/commit/495bd2f02bc5c75ae4aa06cf66689c3f9e8c8026) Optimization experiments
- [`e70c6901`](https://github.com/flamewing/asl-releases/commit/e70c690110df21a3d638bc9637d87c0ed61fdd3e) Planning next steps
- [`776e39b9`](https://github.com/flamewing/asl-releases/commit/776e39b9c006d08ab959b9fe78cbc025d4d0a85b) Reject impossible floating-point prefixes before strtod
- [`d383f773`](https://github.com/flamewing/asl-releases/commit/d383f773c2179f066db507adb6942e8a12180c77) docs: update RESULTS.md with provisional historical timing figures and retest information
- [`e0a21bed`](https://github.com/flamewing/asl-releases/commit/e0a21bed669447ac1a88a89df503feec4f5db388) perf: reuse copied argument lengths during line splitting
- [`e3e78e4e`](https://github.com/flamewing/asl-releases/commit/e3e78e4ed8ccddf5a5b74f389ad8c67fa6ef182d) perf: reject non-FP names before FPU register recognition
- [`f46eb3cb`](https://github.com/flamewing/asl-releases/commit/f46eb3cb1661929913873e5cdda6f1cdfb5f2f7f) perf: reject impossible integer-register spellings early
- [`49fccd44`](https://github.com/flamewing/asl-releases/commit/49fccd44e9a36f35fec9fd646fa5fc8b05b7ea39) docs: record corrected register and hashing benchmarks
- [`a93a2f46`](https://github.com/flamewing/asl-releases/commit/a93a2f46cb7058ca0693d078272e85c4646ab3a1) fix: terminate truncated bounded prepends
- [`2369d51e`](https://github.com/flamewing/asl-releases/commit/2369d51eb1f8d3f92573400f86120de0b36d7cef) docs: benchmark Madness 4 and record lazy hash trials
- [`930b9b0a`](https://github.com/flamewing/asl-releases/commit/930b9b0a06b318b1bbc7e89b5ae4b0371b31a4c5) docs: refresh aggregate benchmarks and compare Windows compilers
- [`653269cb`](https://github.com/flamewing/asl-releases/commit/653269cb1467d9dc04e0eb1a011609cdc2d4a670) fix: initialize globally copied macro definition state
- [`3ada94e6`](https://github.com/flamewing/asl-releases/commit/3ada94e6b90a96fe450d8434f6af68e1de46796e) perf: batch listing line writes
- [`d5407ab4`](https://github.com/flamewing/asl-releases/commit/d5407ab4f8edfd2a4a6374685d35f6eec52028f7) perf: fast-path space and tab classification
- [`a1f3af8e`](https://github.com/flamewing/asl-releases/commit/a1f3af8e950caadf139d37be794c4c3a488f6657) docs: record completed listing and formatting experiments
- [`3b603b88`](https://github.com/flamewing/asl-releases/commit/3b603b887382d1964305152c295376e4de0c3127) build: remove legacy build system
- [`14f2d1a0`](https://github.com/flamewing/asl-releases/commit/14f2d1a0ee8c09974ece194c8df5d3d7b27fcf1e) fix: preserve zero-valued character translations
- [`b0ed3463`](https://github.com/flamewing/asl-releases/commit/b0ed346352897b3f34c8f5d18f699f1a1e46ac82) fix: preserve signed 68000 PHASE addresses
- [`10523c20`](https://github.com/flamewing/asl-releases/commit/10523c208b5cb92afa5ce80b7b87d66d4a868793) fix: keep temporary labels intact in compound expressions
- [`4f12da89`](https://github.com/flamewing/asl-releases/commit/4f12da89d21fd7effcef0043db6997a3278eb3dc) Fix high-byte CHARSET mappings and skipped macro terminators
- [`a9c38a8f`](https://github.com/flamewing/asl-releases/commit/a9c38a8f10f956fa7fdc3609a7d7374015444a1b) Reduce expression operator discovery scans
- [`7399003c`](https://github.com/flamewing/asl-releases/commit/7399003c6872ec5dd210b1fa3a2e721af507a4c8) Use known spans when copying parsed line components
- [`4e59163a`](https://github.com/flamewing/asl-releases/commit/4e59163aa1448413af44b058729a480665e0c455) Dispatch expression operators through ordered first-byte chains
- [`594a4557`](https://github.com/flamewing/asl-releases/commit/594a4557e0161a4ec21a660f8b7448912f762abd) Inline component resets on MSVC
- [`66d5e774`](https://github.com/flamewing/asl-releases/commit/66d5e774502b126113aa77220f6108859b1378b3) Bypass general listing value formatting on GCC
- [`420f9b09`](https://github.com/flamewing/asl-releases/commit/420f9b091e9bd44d20443918dbacb769f5805f33) Bound SysString conversion when padding and separators exhaust space
- [`9f47deb3`](https://github.com/flamewing/asl-releases/commit/9f47deb3f31720826003f76415835e48fc3ae8dc) Specialize GCC expression operator scanning with shared quote handling
- [`07dcf609`](https://github.com/flamewing/asl-releases/commit/07dcf60978ae9e3571804a68d4e254bdcac6b656) Record shared listing experiments and discard inconclusive variants
- [`9123ee90`](https://github.com/flamewing/asl-releases/commit/9123ee906b081753905edafbfaa371d56c6cfdd2) Keep symbol lookup when integer syntax does not match
- [`ca009610`](https://github.com/flamewing/asl-releases/commit/ca009610a7e4fbc6e18e1b7bcc253cb8c25b2268) Record upstream regression bisections and correction trials
- [`858be093`](https://github.com/flamewing/asl-releases/commit/858be093a1da04b7205103bbd5ad7fe89627df7d) Fix full-width complements in restricted-width mode
- [`eff2a749`](https://github.com/flamewing/asl-releases/commit/eff2a7493329557984d394a7e64bda2a9d69dd87) Preserve the width of single-digit Intel hex zero
- [`4ea9fe85`](https://github.com/flamewing/asl-releases/commit/4ea9fe85ee66e46b9e5140bec9d370a87c1c050d) Round subnormal floating-point data at target precision
- [`86f22b8e`](https://github.com/flamewing/asl-releases/commit/86f22b8e9946a6aad7027ae7002cfbc71e0d9f54) Correct TMS340xx FIELD bounds across hosts
- [`f37884f0`](https://github.com/flamewing/asl-releases/commit/f37884f050215efd91d2ef784be5e64f8d354ae0) Resolve single-address mapping overlaps
- [`18c3b545`](https://github.com/flamewing/asl-releases/commit/18c3b5457461e32391c60b0a449799e951138541) Avoid the duplicate forward-reference root search
- [`be04cdea`](https://github.com/flamewing/asl-releases/commit/be04cdeaa6b99c1b41369d2d7ceaf7176313e327) Index forward references by name and ordered scope
- [`564eff4d`](https://github.com/flamewing/asl-releases/commit/564eff4d45a3920a9785fe9f05289247b7c1e84d) Update copyright years
- [`28deb260`](https://github.com/flamewing/asl-releases/commit/28deb260160ae8642534135906891fa1ddfa7756) Record finalizer timings and scope traversal profiles
- [`9abb1335`](https://github.com/flamewing/asl-releases/commit/9abb13356cf46c5707dda40746e0009d5750d992) Record independent symbol hash experiments
- [`1c9514a1`](https://github.com/flamewing/asl-releases/commit/1c9514a179512e84d32d7845f2ffcb44b11c2e7b) docs: refresh master versus experimental benchmarks and profiles
- [`329c60f0`](https://github.com/flamewing/asl-releases/commit/329c60f0abe16a2601985bd3d1d35e2ff51de9ab) Cache byte whitespace classification with explicit locale refresh
- [`b6ac09ea`](https://github.com/flamewing/asl-releases/commit/b6ac09ea0a3ee6c4dace939b110ba88c1d6d6a19) Record independent optimization trials and confirmed whitespace gains
- [`682e1921`](https://github.com/flamewing/asl-releases/commit/682e19214a9f715dbc3706cce6a5a7fefec2b804) Add bounded borrowed string helpers and parser copy inventory
- [`68ca50da`](https://github.com/flamewing/asl-releases/commit/68ca50da1de6c9963ff8bc2aee87a226604ed55a) Evaluate integer literals through borrowed spans before scratch copies
- [`cc68b318`](https://github.com/flamewing/asl-releases/commit/cc68b318519b44029a987af44683fbe91c2c33b8) Record retained parser span revision and confirmation limits
- [`afa0296f`](https://github.com/flamewing/asl-releases/commit/afa0296ff2c20a4b4589eee9467eee0b392f772b) Record recovered-host quoted string decoding experiment
- [`8d2493e4`](https://github.com/flamewing/asl-releases/commit/8d2493e48d7acd4e8e6eae8fa95daf4eebd003f2) Propagate borrowed views through recursive expression arguments
- [`b3989e12`](https://github.com/flamewing/asl-releases/commit/b3989e12c89fb359808b6fb427132c95e11d6968) Document Sonic 1 regression investigation and wrapper trial
- [`e131a740`](https://github.com/flamewing/asl-releases/commit/e131a740413c65ed6af5922fb126dddf119e5056) Fixing file types
- [`fee54a8e`](https://github.com/flamewing/asl-releases/commit/fee54a8eced72cc868f156119cacc2be55788bc7) Add CI workflows for Android, update existing workflows for consistency and improvements
- [`e0230271`](https://github.com/flamewing/asl-releases/commit/e0230271358bfa6dbdcacd589069a11474c869f7) Initialize V60 and ColdFire operand flags
- [`f16afb9a`](https://github.com/flamewing/asl-releases/commit/f16afb9a8b41e4994912c2fa66c84f002ffe8b4e) Add .cache directory to .gitignore
- [`45f328e4`](https://github.com/flamewing/asl-releases/commit/45f328e42f45670e277d554961a51fb5273b1287) Fix CI configuration, coverage, and workflow concurrency
- [`62e7b54d`](https://github.com/flamewing/asl-releases/commit/62e7b54dba2fa1e1156ee266118900616cc5033a) Gate native snapshots on tests and retain workflow failure logs
- [`af8e3898`](https://github.com/flamewing/asl-releases/commit/af8e3898e5a54ff1ddbc1eb3c0b6623da0e67bf9) Pin cross-compilers and trim workflow dependency installation
- [`df0cd61b`](https://github.com/flamewing/asl-releases/commit/df0cd61b7e19fe4ef75c526fe85a6780ad6613d5) Restrict workflow credentials and pin GitHub action dependencies
- [`c886ed56`](https://github.com/flamewing/asl-releases/commit/c886ed561b5378838b183a4cfd0e55026de8d817) Skip scheduled processing only after successful runs of the same SHA
- [`d1725dfe`](https://github.com/flamewing/asl-releases/commit/d1725dfe9373e909179695f37874d7ca2daa4800) Require confirmed Coverity submission acceptance and identify builds by SHA
- [`a76d2919`](https://github.com/flamewing/asl-releases/commit/a76d2919a168eb291de02a97008f709497ac5b5a) Give snapshot runs unique releases and reuse validated drafts on retry
- [`61d11fd0`](https://github.com/flamewing/asl-releases/commit/61d11fd0c05065181566066cc9469b70c87bfdad) Verify snapshot ownership before publishing or cleaning up drafts
- [`4d00b439`](https://github.com/flamewing/asl-releases/commit/4d00b4396715c1f926f988b8a5ae8495d98bd6ba) Update .clang-tidy to remove identifier naming options
- [`cb69dfd9`](https://github.com/flamewing/asl-releases/commit/cb69dfd9bef065df72fe24476d34a4da5f64f9c0) Fix register validation in DecodeGen, simplify DecodeIntName logic, enhance SetCodeChunkListAllocLen for zero allocation, update cpu_2_phys_area_dump format specifier, and add new tests for 3203x register functionality.
- [`81468438`](https://github.com/flamewing/asl-releases/commit/81468438b3e553f130ac631ecee7d46cb532f9a7) Fix sanitizer-reported symbol leaks and empty-pointer operations
- [`4dc2fd28`](https://github.com/flamewing/asl-releases/commit/4dc2fd28e7568476b377a43fa60d6188541cd992) Line endings
- [`d96d1f9d`](https://github.com/flamewing/asl-releases/commit/d96d1f9d6487dd3c9dcccddf45fd2cb454362c18) Normalize tracked text and enforce LF line endings
- [`38a5ef6d`](https://github.com/flamewing/asl-releases/commit/38a5ef6ddfd5c70c0991e7b47d5ce00991500c9d) Bump the github-actions group with 5 updates
- [`4d342b19`](https://github.com/flamewing/asl-releases/commit/4d342b193bd116785dce6fdd730860c563bc57bb) Add Emscripten stack size configuration and improve document build dependencies

Upstream synchronization:

- [`f4222cc7`](https://github.com/flamewing/asl-releases/commit/f4222cc73ec25575a2d23331878f965dd9aed81f) Import original AS build 213.
- [`25c26e65`](https://github.com/flamewing/asl-releases/commit/25c26e658f1b7184f2bf697f0b9cb954161b4e1e) Import original AS build 214.
- [`3623dcff`](https://github.com/flamewing/asl-releases/commit/3623dcff48356dde2c2ad801181f9a4394244104) Import original AS build 215.
- [`9fcaff36`](https://github.com/flamewing/asl-releases/commit/9fcaff36bbcd19f57138f63b8b0387090f0342be) Import original AS build 216.
- [`2bb6aca3`](https://github.com/flamewing/asl-releases/commit/2bb6aca340eeff13f020409c2f7ab1876aea9193) Import original AS build 217.
- [`e24cbb55`](https://github.com/flamewing/asl-releases/commit/e24cbb5567d61a871c94d33e2b9436caec31e555) Import original AS build 218.
- [`2ea06387`](https://github.com/flamewing/asl-releases/commit/2ea0638770aa1a3a0be764b4d5e622a875407f22) Import original AS build 219.
- [`838efcf3`](https://github.com/flamewing/asl-releases/commit/838efcf34870ca34280ac2982a4adb8306094c73) Import original AS build 220.
- [`e29818ad`](https://github.com/flamewing/asl-releases/commit/e29818add301e5d11ae447abd47bd27c4564af9e) Import original AS build 221.
- [`84f2315b`](https://github.com/flamewing/asl-releases/commit/84f2315b07c37e6bfcd6e7c7764e99f7d0500d9f) Import original AS build 222.
- [`a030a7c3`](https://github.com/flamewing/asl-releases/commit/a030a7c349dbee8be66fe1169f24e7ebe4e8931a) Import original AS build 223.
- [`73dfe466`](https://github.com/flamewing/asl-releases/commit/73dfe466f60eefaa3e0d4acc07f7adb7a9007d94) Import original AS build 224.
- [`f95e248e`](https://github.com/flamewing/asl-releases/commit/f95e248e3832395258861ba7656cad6b3c482d2a) Import original AS build 225.
- [`26888ddc`](https://github.com/flamewing/asl-releases/commit/26888ddc81e2473112127c2c6bb821b52fc56dde) Import original AS build 226.
- [`6fd0c0e0`](https://github.com/flamewing/asl-releases/commit/6fd0c0e0c7756d47d81f2a916a30987446b0fefe) Import original AS build 227.
- [`46b2260e`](https://github.com/flamewing/asl-releases/commit/46b2260e00c23573c73029d1c7d0183d741430cf) Import original AS build 228.
- [`6a05640d`](https://github.com/flamewing/asl-releases/commit/6a05640d24758e08fc5dc33b6e68d8a3dcd4cab7) Import original AS build 229.
- [`75d83cfc`](https://github.com/flamewing/asl-releases/commit/75d83cfc3d99ad4256e2f116956191db827b2b12) Import original AS build 230.
- [`812f5e55`](https://github.com/flamewing/asl-releases/commit/812f5e557e60384000a6cfa7852d414715ca5557) Import original AS build 231.
- [`9337e73c`](https://github.com/flamewing/asl-releases/commit/9337e73ced77e7d18961912835d86dcc04389aef) Import original AS build 232.
- [`70c0d7c3`](https://github.com/flamewing/asl-releases/commit/70c0d7c37343cfcf968023dfdc43da5f21a985fe) Import original AS build 233.
- [`e4bd535e`](https://github.com/flamewing/asl-releases/commit/e4bd535eef274dc534b8d915dd61859de8c8908e) Import original AS build 234.
- [`83eed73c`](https://github.com/flamewing/asl-releases/commit/83eed73c938f6fb579f62b4decb9816bcd3c67fe) Import original AS build 235.
- [`2873a128`](https://github.com/flamewing/asl-releases/commit/2873a128a3830efb306aaf88a4d5552988f4c429) Import original AS build 236.
- [`40667e1f`](https://github.com/flamewing/asl-releases/commit/40667e1fa50d26f90d62e5c78be080a9e909eeb2) Import original AS build 237.
- [`fd104e6f`](https://github.com/flamewing/asl-releases/commit/fd104e6ffdbe7a172972534b3ffc3765b82add7f) Import original AS build 238.
- [`b8631524`](https://github.com/flamewing/asl-releases/commit/b86315247f4a4a86b10b87e1df7503a5705f6a5c) Import original AS build 239.
- [`99134b07`](https://github.com/flamewing/asl-releases/commit/99134b07029f6472c63432b3a373fc899bc3daae) Import original AS build 240.
- [`19934bc6`](https://github.com/flamewing/asl-releases/commit/19934bc6aeb541da94348235387a4d316c2c4db1) Import original AS build 241.
- [`e5d35278`](https://github.com/flamewing/asl-releases/commit/e5d35278dbc063d64ad295eeb1adb8752757d783) Import original AS build 242.
- [`e4f65664`](https://github.com/flamewing/asl-releases/commit/e4f6566448f481c581fbab4744b1c2de01882f43) Import original AS build 243.
- [`b2c39ad6`](https://github.com/flamewing/asl-releases/commit/b2c39ad6c62acdcfeec076fa9ef00d64201bd3f8) Import original AS build 244.
- [`87c1739f`](https://github.com/flamewing/asl-releases/commit/87c1739f6794430a6c5a53ae3912d606cb4c698a) Import original AS build 245.
- [`2887ec40`](https://github.com/flamewing/asl-releases/commit/2887ec402aa5b8eca5de291c7e2f50dfbf1ddf8e) Import original AS build 246.
- [`b45689ad`](https://github.com/flamewing/asl-releases/commit/b45689adee712e6b918ca148609e425c9df76e3e) Import original AS build 247.
- [`9321de89`](https://github.com/flamewing/asl-releases/commit/9321de896e002e020ac3e6b58aea27fef3cc184e) Import original AS build 248.
- [`94f2b8b2`](https://github.com/flamewing/asl-releases/commit/94f2b8b2361a73b0f5f85e03d322cef03ebdfa61) Import original AS build 249.
- [`2244fa04`](https://github.com/flamewing/asl-releases/commit/2244fa04f3cabe090a17f55ab7a72b948f50a68f) Import original AS build 250.
- [`126d8f55`](https://github.com/flamewing/asl-releases/commit/126d8f55235a03b3b83f59872887c846251d8573) Import original AS build 251.
- [`9570fa55`](https://github.com/flamewing/asl-releases/commit/9570fa55367ae0dc961382e57c45ecff9500f36f) Import original AS build 252.
- [`f4afbc00`](https://github.com/flamewing/asl-releases/commit/f4afbc000aa60417e3c1153dcda353e7d2569eaf) Import original AS build 253.
- [`9f95e341`](https://github.com/flamewing/asl-releases/commit/9f95e3419f6c557b5305d47f738b5beaee2b4668) Import original AS build 254.
- [`4899ce8a`](https://github.com/flamewing/asl-releases/commit/4899ce8a8836acf24aa3cd9b8c2114f35a376612) Import original AS build 255.
- [`f7a1d9fc`](https://github.com/flamewing/asl-releases/commit/f7a1d9fc71bfa4be76a3d78a770287bb3713a90e) Import original AS build 256.
- [`0aad417e`](https://github.com/flamewing/asl-releases/commit/0aad417e266a8eea6c0d6c45370d3ae933130199) Import original AS build 257.
- [`df55de46`](https://github.com/flamewing/asl-releases/commit/df55de464384d20f51784026954031ae4804c9bf) Import original AS build 258.
- [`a10d3780`](https://github.com/flamewing/asl-releases/commit/a10d3780eb5072ac24754afe226ce9c6e62270cc) Import original AS build 259.
- [`7e811128`](https://github.com/flamewing/asl-releases/commit/7e81112876d1605c8fd7906e9c7af5e85f9e38c1) Import original AS build 260.
- [`1b36ae07`](https://github.com/flamewing/asl-releases/commit/1b36ae07c3f6aed4ce76ac5e8a72ec14e0e1f2bf) Import original AS build 261.
- [`292c0da5`](https://github.com/flamewing/asl-releases/commit/292c0da52c7f351e4756def2abe05618a08cf61a) Import original AS build 262.
- [`9c3ea4ca`](https://github.com/flamewing/asl-releases/commit/9c3ea4ca4b8c075a63afe1a6c19c9235cd3d4f20) Import original AS build 263.
- [`03328cf8`](https://github.com/flamewing/asl-releases/commit/03328cf8adae505a4e216c42e2ccbe0f9a1ffd2e) Import original AS build 264.
- [`c3358cdc`](https://github.com/flamewing/asl-releases/commit/c3358cdc9d65d6c28a46ac6a48b4d3a570266d4b) Import original AS build 265.
- [`d896055d`](https://github.com/flamewing/asl-releases/commit/d896055dffa5e46dec0c8d722d8a7aa9f3173a68) Import original AS build 266.
- [`01c05c63`](https://github.com/flamewing/asl-releases/commit/01c05c63902f957d2485bd5d98639f6a8aff1d9b) Import original AS build 267.
- [`c5a4ee62`](https://github.com/flamewing/asl-releases/commit/c5a4ee6255319e3f09cde8ad5488a1fa20ff23ae) Import original AS build 268.
- [`4955f8bf`](https://github.com/flamewing/asl-releases/commit/4955f8bffcf1f0a315657c94664af7799a6ccb61) Import original AS build 269.
- [`cf60da1c`](https://github.com/flamewing/asl-releases/commit/cf60da1c1db75c26123db6a55042f525cbb0a608) Import original AS build 270.
- [`88f6e972`](https://github.com/flamewing/asl-releases/commit/88f6e9723ff8d8891ffca42f38dbb9e6b267e942) Import original AS build 271.
- [`a29c4e77`](https://github.com/flamewing/asl-releases/commit/a29c4e779b00dda5349dd6b23e0c1181a21d671d) Import original AS build 272.
- [`0ef8240d`](https://github.com/flamewing/asl-releases/commit/0ef8240d0bf8af51ddb9d729bbac9e7f96ec738f) Import original AS build 273.
- [`8cee5407`](https://github.com/flamewing/asl-releases/commit/8cee5407990d7fdc39ed05080148a58c9f8e05fa) Import original AS build 274.
- [`d0ec3940`](https://github.com/flamewing/asl-releases/commit/d0ec3940d6f06cf643a161079dfd508fffe05828) Import original AS build 275.
- [`9a2f4a27`](https://github.com/flamewing/asl-releases/commit/9a2f4a2797fd397e2901bec3ee34160695f260cb) Import original AS build 276.
- [`5950d73a`](https://github.com/flamewing/asl-releases/commit/5950d73ac9497f4be3934759f084583d96f659a9) Import original AS build 277.
- [`e57b9886`](https://github.com/flamewing/asl-releases/commit/e57b9886e61b79e319a23ca4527bfc404ab0003a) Import original AS build 278.
- [`97d5c0e4`](https://github.com/flamewing/asl-releases/commit/97d5c0e47220deddf978ab761a6f0d301adebd2e) Import original AS build 279.
- [`74bc5726`](https://github.com/flamewing/asl-releases/commit/74bc572634ff28dc6f36c69c7a6354147821ff66) Import original AS build 280.
- [`9bba53f1`](https://github.com/flamewing/asl-releases/commit/9bba53f15fe4d7e02dffd80d99a29442042055f4) Import original AS build 281.
- [`3bab0654`](https://github.com/flamewing/asl-releases/commit/3bab06544efb599e0ee9f187f043acd325fb4c48) Import original AS build 282.
- [`c07c4f97`](https://github.com/flamewing/asl-releases/commit/c07c4f97b0ad103e3ec8a2f63f58396a9d0f3f95) Import original AS build 283.
- [`6715fa1b`](https://github.com/flamewing/asl-releases/commit/6715fa1b305f27205e8cb0f0fd298b807a099ff9) Import original AS build 284.
- [`e4b68282`](https://github.com/flamewing/asl-releases/commit/e4b6828266c36c65ff5474c01e2c90ebcea12144) Import original AS build 285.
- [`cf192e0e`](https://github.com/flamewing/asl-releases/commit/cf192e0ecb5ea8154f35028a435ccfab008d2565) Import original AS build 286.
- [`fbd8162a`](https://github.com/flamewing/asl-releases/commit/fbd8162a778f367c0a6728992244f319c1563aad) Import original AS build 287.
- [`8cceb0c2`](https://github.com/flamewing/asl-releases/commit/8cceb0c203dcdeb119b335cb53f252f7e13e3571) Import original AS build 288.
- [`2609a053`](https://github.com/flamewing/asl-releases/commit/2609a053e792b7f05d72e169f1b5bb971ee1e3ea) Import original AS build 289.
- [`2a25241f`](https://github.com/flamewing/asl-releases/commit/2a25241f1aaa4c44e02f23a2416fa784a61e53dc) Import original AS build 290.
- [`35133cf8`](https://github.com/flamewing/asl-releases/commit/35133cf88e89b032e90482d9a53e89ad56f0f685) Import original AS build 291.
- [`e42ad1a9`](https://github.com/flamewing/asl-releases/commit/e42ad1a992a04f75ca49c919eb649d35d7c75c47) Import original AS build 292.
- [`11892701`](https://github.com/flamewing/asl-releases/commit/11892701af680d2a45dc1d4a42aeaa0f5e8f29b3) Import original AS build 293.
- [`0491e6fa`](https://github.com/flamewing/asl-releases/commit/0491e6fa9f8be6c836ae2b0a4ad1f3ba7a9342ca) Import original AS build 294.
- [`39ea8cbe`](https://github.com/flamewing/asl-releases/commit/39ea8cbea12c194b20f175eff60dfcbac8b2a5cb) Import original AS build 295.
- [`7ceaa13c`](https://github.com/flamewing/asl-releases/commit/7ceaa13c490bc433f6a7f134eb1e7eb0ead1ae4f) Import original AS build 296.
- [`c14b983e`](https://github.com/flamewing/asl-releases/commit/c14b983e9f8612ec99b8ebb7ff89c82a6a0ee4ca) Import original AS build 297.
- [`3248dfce`](https://github.com/flamewing/asl-releases/commit/3248dfce3dc29f50d8be479dac6885d37bd8ce08) Import original AS build 298.
- [`4084261a`](https://github.com/flamewing/asl-releases/commit/4084261ab1985d04bf85ce4edf7fc42625ef7af3) Import original AS build 299.
- [`4fddddfb`](https://github.com/flamewing/asl-releases/commit/4fddddfb1f36ccf2d4ae3c05154ff3c82b28b700) Import original AS build 300.
- [`4b20bee4`](https://github.com/flamewing/asl-releases/commit/4b20bee4dc7809cb836e2c556f7038221a030311) Import original AS build 301.
- [`126188d7`](https://github.com/flamewing/asl-releases/commit/126188d7276151b29eaca7f1be1eb5f0f276dd10) Import original AS build 302.
- [`429a675d`](https://github.com/flamewing/asl-releases/commit/429a675daf7dd0f4d1c39a4e5a45215abca8abcc) Import original AS build 303.
- [`036bc56d`](https://github.com/flamewing/asl-releases/commit/036bc56db7a6c9aed65206b4fab97a86a4269ae0) Import original AS build 304.
- [`35f7fa34`](https://github.com/flamewing/asl-releases/commit/35f7fa3455d3d05c3a5462a58b76b98e00853794) Import original AS build 305.
- [`c2d41b38`](https://github.com/flamewing/asl-releases/commit/c2d41b382054c55b9d4c3db24bd34ea1d5c675f0) Import original AS build 306.
- [`1c1fa75c`](https://github.com/flamewing/asl-releases/commit/1c1fa75cc6dfcf8847b7ee6abee4a4bce960720c) Import original AS build 307.
- [`43838777`](https://github.com/flamewing/asl-releases/commit/43838777ca0f7f584201428a29ebf4739b86f616) Import original AS build 308.
- [`7e2abc23`](https://github.com/flamewing/asl-releases/commit/7e2abc23fd0145e43bee17777ca6e2ee7a912988) Import original AS build 309.
- [`cf7582c5`](https://github.com/flamewing/asl-releases/commit/cf7582c5f31abcc41fcecb0e0e15a72b8fd6bb60) Import original AS build 310.
- [`315b91e9`](https://github.com/flamewing/asl-releases/commit/315b91e9a857510731fd1f6498323d7059a10393) Import original AS build 311.
- [`915cc046`](https://github.com/flamewing/asl-releases/commit/915cc046bf7cf5541256525c47cb719350243d9a) Import original AS build 312.
- [`fb75b92d`](https://github.com/flamewing/asl-releases/commit/fb75b92ddad1d87e6003868b622971b11b9f87c4) Import original AS build 313.

### snapshot-v1.4.2.20261011-run38119234017

Boundary: [`4d342b19`](https://github.com/flamewing/asl-releases/commit/4d342b193bd116785dce6fdd730860c563bc57bb) to [`snapshot-v1.4.2.20261011-run38119234017`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261011-run38119234017) ([`a31c0b24`](https://github.com/flamewing/asl-releases/commit/a31c0b24de653f968f303dba0a5abeeee416c733)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/4d342b193bd116785dce6fdd730860c563bc57bb...a31c0b24de653f968f303dba0a5abeeee416c733).

Embed message catalogs in CI builds and adjust packaging to omit unnecessary files. Remove the experimental Android continue-on-error setting, suppress the relevant MinGW toolchain deprecation warnings, and detect nl_langinfo for platform compatibility.

Fork changes (2 commits):

- [`1cbc0f58`](https://github.com/flamewing/asl-releases/commit/1cbc0f58fbc95994d25fdbd49f9fc80bd197f3ce) Remove experimental continue-on-error option from ci-android workflow, add suppress-deprecation-warnings for MINGW32 toolchain in ci-windows-mingw and snapshots workflows, and check for nl_langinfo symbol in CMakeLists.txt for better compatibility.
- [`a31c0b24`](https://github.com/flamewing/asl-releases/commit/a31c0b24de653f968f303dba0a5abeeee416c733) Enable ASL_EMBED_MESSAGES during CMake configuration and adjust packaging commands to exclude unnecessary files

### snapshot-v1.4.2.20261012-run38215198548

Boundary: [`a31c0b24`](https://github.com/flamewing/asl-releases/commit/a31c0b24de653f968f303dba0a5abeeee416c733) to [`snapshot-v1.4.2.20261012-run38215198548`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261012-run38215198548) ([`4730f629`](https://github.com/flamewing/asl-releases/commit/4730f6299d743703b7b677c6a9d09eddae5e92b7)). [Compare this batch](https://github.com/flamewing/asl-releases/compare/a31c0b24de653f968f303dba0a5abeeee416c733...4730f6299d743703b7b677c6a9d09eddae5e92b7).

Repair apparent Coverity submission failures: accept a confirmed successful response with HTTP 200 and tolerate whitespace in its success text. Log the actual downloaded capture-tool version and track findings that require modeling rather than claiming that a model has already resolved them.

Harden binary-record parsing, malformed catalog handling, allocation/length validation, file open/read/write/seek/close failures and resource cleanup across the assembler, linker, disassembler, converters and test driver. Check p2bin measurement seeks, make register-copy bounds explicit, clarify fatal-I/O contracts, permit legitimate H16 zero-count writes, and document bounded formatter/catalog-string contracts.

Fix decoder-result, addressing and cleanup defects in M16C, Padauk, Z80, MCS-96, M16, MCore, NS32000 and ST9; repair bit mirroring, unsafe shifts/divisors, locale initialization and listing diagnostics. Correct MSP430 emulated PC-relative adjustment and RX compact MOV displacement validation, avoid overlapping union assignment in TempResultToFloat, and diagnose unsupported immediate sizes in 75K0/M16. Document the intentionally ignored macro replacement count.

Add Coverity regression inputs and I/O/locale tests, validate the POSIX locale test under WSL, repair Android compatibility of the invalid-format fixture, and clean up generated test catalogs. Detailed finding-by-finding notes are in [the Coverity fix record](coverity-fixes.txt).

Fork changes (22 commits):

- [`7d7b4afc`](https://github.com/flamewing/asl-releases/commit/7d7b4afc11e374f6d26d769b78e486ac38f48551) Accept confirmed Coverity submissions returned with HTTP 200
- [`280d3971`](https://github.com/flamewing/asl-releases/commit/280d39719c470759fa1211f0b6d5575478127614) Validate binary input and fix major Coverity findings
- [`f9e55339`](https://github.com/flamewing/asl-releases/commit/f9e553393de763003cdad5614a908024a2dd0a10) Handle file failures and resources identified by Coverity
- [`af59df2f`](https://github.com/flamewing/asl-releases/commit/af59df2f07b5dc4f2181b58b4d73ac6b03245c3e) Fix decoder results and bit mirroring flagged by Coverity
- [`c372681a`](https://github.com/flamewing/asl-releases/commit/c372681a2ee1506fbc557d96b845b1504a215e2a) Fix remaining decoder addressing and cleanup defects
- [`fdffa051`](https://github.com/flamewing/asl-releases/commit/fdffa051e714f9a08db892bb908304d45f6afe46) Fix locale initialization, file checks and listing diagnostics
- [`7b834245`](https://github.com/flamewing/asl-releases/commit/7b8342453eeeda5777f27769bd8a8ec9e16778c7) Avoid overlapping union assignment in TempResultToFloat
- [`dcc5d233`](https://github.com/flamewing/asl-releases/commit/dcc5d23309a2b5dc14abe7ea6aab8dfc5c55a323) Guard pending Coverity shift and divisor paths
- [`257cb001`](https://github.com/flamewing/asl-releases/commit/257cb00193a1090237de40dd213990016f0cffa8) Fix MSP430 emulated PC-relative displacement adjustment
- [`7687176d`](https://github.com/flamewing/asl-releases/commit/7687176db779a3f7fa82e762023030c93a07702d) Validate RX compact immediate MOV displacements
- [`3561bfbe`](https://github.com/flamewing/asl-releases/commit/3561bfbe971c1c488773877ba688c13769f98751) Document intentional macro replacement count discard
- [`9b4f4d96`](https://github.com/flamewing/asl-releases/commit/9b4f4d965c2c4ef8ab98c7b920c878302adf83c3) Fix POSIX locale test linkage and record WSL validation
- [`775aaebf`](https://github.com/flamewing/asl-releases/commit/775aaebfd48f8bf1170e8f04deb5561e8e97a6e6) Clarify fatal I/O contracts and portable H16 zero-count writes
- [`c08e73a3`](https://github.com/flamewing/asl-releases/commit/c08e73a3755df5c0ad87a0caabb02caeeb35dcd2) Establish bounded formatter and catalog string contracts
- [`d8d89832`](https://github.com/flamewing/asl-releases/commit/d8d89832b6a4876da88ebffd8786816b3ad99fc1) Track modeling CIDs and log the Coverity capture version
- [`975d5f8e`](https://github.com/flamewing/asl-releases/commit/975d5f8ecc769c8705094c92445f147fece37020) Harden binary tool callers after Coverity trace review
- [`9732bf21`](https://github.com/flamewing/asl-releases/commit/9732bf21c3cd49e9dea028bf260ac54b5abdd641) Diagnose unsupported immediate sizes in 75K0 and M16
- [`ff6adacc`](https://github.com/flamewing/asl-releases/commit/ff6adacc2163a9d514c09a83da51df4dbb9314f1) Accept whitespace in Coverity upload success responses
- [`fce128d5`](https://github.com/flamewing/asl-releases/commit/fce128d5b108900ac05d2042347092f56c2a0feb) Keep invalid-format test compatible with Android diagnostics
- [`223a8016`](https://github.com/flamewing/asl-releases/commit/223a8016c03d52a2334515762bbe50c7a39c5068) Clean up Coverity test catalogs and clarify fixture bounds
- [`765e6213`](https://github.com/flamewing/asl-releases/commit/765e62136e095bca8b2c9f790f9de222dc10014f) Check p2bin measurement seeks
- [`4730f629`](https://github.com/flamewing/asl-releases/commit/4730f6299d743703b7b677c6a9d09eddae5e92b7) Make register copy bounds explicit and record rescan triage

## Pending changes

Boundary: [`snapshot-v1.4.2.20261012-run38215198548`](https://github.com/flamewing/asl-releases/releases/tag/snapshot-v1.4.2.20261012-run38215198548) to [`1275d803`](https://github.com/flamewing/asl-releases/commit/1275d803cf8f6d1c4395c9a3a3fce47183f4345b). These changes are committed on the local main branch but are not included in the last recorded snapshot tag. [Compare this batch](https://github.com/flamewing/asl-releases/compare/4730f6299d743703b7b677c6a9d09eddae5e92b7...1275d803cf8f6d1c4395c9a3a3fce47183f4345b).

The issue #13 diagnostic audit adds operand positions and contextual details under -x: actual values and bounds, branch displacement and PC base, allowed sizes/addressing forms/registers/conditions/CPUs, page/bank assumptions, function and operand counts, and the offending versus expected bracket. English/German catalogs, the numeric error documentation and regression fixtures are updated together. Existing error numbers and severities are generally retained, with explicit corrections where a prior category was wrong.

The original 17 tranches and the subsequent inventory review cover shared parsing/data/function/structure diagnostics and many processor backends. Each implementation commit below identifies its affected family. The inventory review is still in progress; this section does not claim that every candidate has been completed. [Diagnostic batches](diagnostic-batches.txt) record the constraints and validation, and [the inventory review](diagnostic-inventory-review.json) records completed, partial and pending files.

The audit also fixes defects discovered while exercising failures: stop code emission after invalid operands or failed expressions, retain the primary evaluator error, discard partial instructions/data, defer provisional forward-reference checks, validate widths before narrowing or shifting, and reset per-instruction parser state. Specific encoding corrections include MELPS740 NOP insertion, RS08 bit-branch operand selection, KCPSM CALL immediate masking, XCore automatic long-branch prefixes and endpoints, AVR byte-mode branch wrapping, HC12X BTAS masks/opcode length, HuC6280 TST address bytes, 65CE02 widened base-page bytes, and MCore JSRI indirect-call encoding.

Additional fixes cover register aliases, reserved/native register indices, empty TLCS-90 operands, ST9 update/address expressions and R255 byte descriptors, TC9331 final-field comments, MN1610 failed PC-relative emission, and complete external message-catalog reads beyond 64 KiB. ST7/STM8 packed BTJF/BTJT now evaluate the actual final target operand instead of an absent or stale third operand, preventing the Windows access violation. STM8 branch distances retain the full 24-bit address width until signed-byte validation; valid short branches and address-space wrapping remain supported.

Fork changes (271 commits):

- [`9a836646`](https://github.com/flamewing/asl-releases/commit/9a836646ee814e478b15d5f6151e564b5dfb776a) Record diagnostic specificity audit for issue #13
- [`7e62a4c7`](https://github.com/flamewing/asl-releases/commit/7e62a4c7880ebd3b055539a5c9abfbd45613e906) Explain M68K effective-address displacement range errors
- [`ee379a97`](https://github.com/flamewing/asl-releases/commit/ee379a9777d8b31a2360cb7cf2e39fead5d89eb0) Improve M68K branch, alignment and shift diagnostics
- [`a50b83eb`](https://github.com/flamewing/asl-releases/commit/a50b83ebaf30c858c55517571b0a5cd469a82649) Improve M68K address syntax and register-pair diagnostics
- [`bf8ee2d7`](https://github.com/flamewing/asl-releases/commit/bf8ee2d71bfdd0f8268802b48cb42c1335bdbfac) Explain numeric, page and placement diagnostic constraints
- [`7387ee52`](https://github.com/flamewing/asl-releases/commit/7387ee52c7df2fe92f6424e8d101827abeab8a4a) Report data directive values and failed range bounds
- [`44698bd1`](https://github.com/flamewing/asl-releases/commit/44698bd1454a8a86a7ea2cae1aaa54894cede5df) Explain function argument-count diagnostic contracts
- [`88b7da71`](https://github.com/flamewing/asl-releases/commit/88b7da71fe13685db650e2a38143fe1251991562) Plan bracket mismatch diagnostic batch
- [`2bd35ad4`](https://github.com/flamewing/asl-releases/commit/2bd35ad43fc329017f371fb1a84ca6daa34b2edc) Explain CPU addressing requirements and AVR program pointers
- [`f19323de`](https://github.com/flamewing/asl-releases/commit/f19323de950a4aa3353d5ecdc0cedf74908559f2) Improve page mapping and MSP430X displacement diagnostics
- [`c7a85243`](https://github.com/flamewing/asl-releases/commit/c7a85243b94ee4951815757575c5231212c9fbd9) Identify unbalanced and mismatched delimiters in diagnostics
- [`7393aba6`](https://github.com/flamewing/asl-releases/commit/7393aba62d95a5f2bb362f9dc97caea38b0d1b6f) Complete backend delimiter diagnostics and stop include error cascades
- [`09d20eb1`](https://github.com/flamewing/asl-releases/commit/09d20eb174f079be9d8cb70820214fd98d0e3e60) Report 65xx branch displacement ranges and PC bases
- [`d785bd5b`](https://github.com/flamewing/asl-releases/commit/d785bd5b2b0c05cc0a0f61638f7a776aab010a4e) Fix MELPS740 automatic NOP insertion
- [`51ff9266`](https://github.com/flamewing/asl-releases/commit/51ff9266af345bcc6936377dfd53a42ab40531e1) Report HC12 branch displacement ranges and PC bases
- [`d1f195d6`](https://github.com/flamewing/asl-releases/commit/d1f195d61615428685bee5ecbaa1ae4ee27dad77) Report 6800-family branch displacement ranges and PC bases
- [`b499b057`](https://github.com/flamewing/asl-releases/commit/b499b0574b81e7602942937007c887b2d567f49f) Report 6805-family branch displacement ranges and PC bases
- [`a5e5b802`](https://github.com/flamewing/asl-releases/commit/a5e5b8025f868a1ec85613b9a9efcd2a9ec11e6f) Report 6809-family branch displacement ranges and PC bases
- [`e7cde3fd`](https://github.com/flamewing/asl-releases/commit/e7cde3fdc1360a8eaa8c6277e10f4bad39483f6f) Report RS08 branch displacement ranges at target operands
- [`946742df`](https://github.com/flamewing/asl-releases/commit/946742df1abe78a362dbd510454ebb5479690965) Explain 6804 relative and bit-test branch ranges
- [`0130152d`](https://github.com/flamewing/asl-releases/commit/0130152d9a92420b06bfd371dfed40943fe65d2e) Explain HC16 branch ranges and alignment without error cascades
- [`fa1dfdd0`](https://github.com/flamewing/asl-releases/commit/fa1dfdd0a8146e5c72ff06b3f896e1a245a20ca1) Identify required and forbidden immediate operands on 6804
- [`3a883b9a`](https://github.com/flamewing/asl-releases/commit/3a883b9adc1a0d47aa8148de9fdc7de2acf6c4d8) Explain RS08 forced short addresses and indexed operand contracts
- [`46bcf590`](https://github.com/flamewing/asl-releases/commit/46bcf590a81e6de9dbd527abb088aef19fd239a3) Read the address operand in RS08 suffix bit branches
- [`4881f330`](https://github.com/flamewing/asl-releases/commit/4881f330005085860641e6d384fe2415035d5302) Explain character table, ASSUME and escape failures
- [`82601295`](https://github.com/flamewing/asl-releases/commit/82601295e5db66e937fdc9a4b3c7b4d5eb1531ff) Report values and limits for function and bit-mirror arguments
- [`87646f7b`](https://github.com/flamewing/asl-releases/commit/87646f7be0ee51ae40899f4168b1ff0e114a2fa8) Show full domains for four-bit DATA and custom equates
- [`63e65f62`](https://github.com/flamewing/asl-releases/commit/63e65f62509520c452d21bd5c3bb988f624c0dbe) Explain PIC target limits and SVEC operand constraints
- [`26329331`](https://github.com/flamewing/asl-releases/commit/263293314577be9ea3f2c06ae6857a7df10eef2a) Explain interrupt keywords and excluded register and device values
- [`85795265`](https://github.com/flamewing/asl-releases/commit/85795265bde50c11b8829223d9d65b42d959f334) Report SC61860 branch direction ranges without error cascades
- [`4f04bf24`](https://github.com/flamewing/asl-releases/commit/4f04bf245e7d460b37e29325193f88a7cde21d3f) Distinguish reserved TBIL branch zero and literal byte overflow
- [`2c728b04`](https://github.com/flamewing/asl-releases/commit/2c728b04b4001a3020efd28fbce394461816d5ca) Translate TBIL literal input as unsigned bytes
- [`68444240`](https://github.com/flamewing/asl-releases/commit/6844424018d875b7a000704c42cbc246b5a469b4) Identify 4004 accumulator contracts and branch page mismatches
- [`3d31d77a`](https://github.com/flamewing/asl-releases/commit/3d31d77a4b57be6a1284bf1278b152d21e895ae7) Describe TLCS-42 operand forms and CPU port limits
- [`febba09d`](https://github.com/flamewing/asl-releases/commit/febba09d7882d3b04a531f6b64e30628572e033d) Describe uCOM instruction family restrictions and ROM bounds
- [`da76b543`](https://github.com/flamewing/asl-releases/commit/da76b5434ae3f96e71b03cc2ca8cf521102ca308) Identify accepted KCPSM operand forms at the rejected operand
- [`6ca860ad`](https://github.com/flamewing/asl-releases/commit/6ca860adf2375a356f6e96e59668a8be1fd5ab89) Fix KCPSM CALL immediate operand mask
- [`5d309791`](https://github.com/flamewing/asl-releases/commit/5d30979178857de539b8bdcde1a152ddc8afd62b) Describe CP-3F operand restrictions and allowed shift counts
- [`bbec4ad7`](https://github.com/flamewing/asl-releases/commit/bbec4ad7c484b9397eb226da58599af152849e2d) Reset HMCS400 operand mode before decoding
- [`73916e77`](https://github.com/flamewing/asl-releases/commit/73916e77c38c26122d851d0daabcb534561c0cca) Describe HMCS400 operand forms and RAM address bounds
- [`ef6198e0`](https://github.com/flamewing/asl-releases/commit/ef6198e05249af173e5dc253bc41298e1cf3a3fa) Describe AM29xxx jump encoding constraints and target syntax
- [`84ce2cfa`](https://github.com/flamewing/asl-releases/commit/84ce2cfa354598b97911e7efd8a5cc6c4bea6a31) Explain COP4 nonzero operands and JSRP address restrictions
- [`7cd05feb`](https://github.com/flamewing/asl-releases/commit/7cd05feb79b6953a8c2b2e73326387bbcddb8e6a) Fix Mico8 ALU optional register probe for immediate sources
- [`19524400`](https://github.com/flamewing/asl-releases/commit/19524400993b2abeb05ece4b81efedbf5fc53c05) Describe Mico8 rotate operands and branch displacement limits
- [`bf713fdc`](https://github.com/flamewing/asl-releases/commit/bf713fdc0fca64ebe300b6a60717ed29bf154cbf) Guard TMS3201x shift mask against negative counts
- [`1267fead`](https://github.com/flamewing/asl-releases/commit/1267fead0b8fd17ef587daaa2412883f0d97c984) Describe TMS3201x immediate ranges and allowed shift counts
- [`837e1c33`](https://github.com/flamewing/asl-releases/commit/837e1c333195f6d07880eea15657628f83b6b5f0) Describe TMS320C2x operand forms and correct SST overflow category
- [`60acc1e7`](https://github.com/flamewing/asl-releases/commit/60acc1e7d9b8be3307a5bb73a3e67f67ff07cf61) Describe NEC 75xx pointer modes and compact target intervals
- [`d0d54c43`](https://github.com/flamewing/asl-releases/commit/d0d54c43335967b12b7cd19578a110a724a553ec) Keep NEC 7508 CALT forward-target guesses within range
- [`3c11c654`](https://github.com/flamewing/asl-releases/commit/3c11c654d0538737ca34811809aa324804c495dd) Explain XGATE operand constraints and distinguish branch overflow
- [`a3640995`](https://github.com/flamewing/asl-releases/commit/a3640995abce0de9edb7a4ce9d893d4141af6fc9) Describe COP8 operand forms, register interval and JP range
- [`89001829`](https://github.com/flamewing/asl-releases/commit/8900182958246de626e97548da8b1deabf695cd1) Describe F8 CPU, register pair, shift and branch constraints
- [`f116269d`](https://github.com/flamewing/asl-releases/commit/f116269d1258c57e89ab51e448cf98775bc326fa) Describe ACE indexed operands and branch constraints
- [`772c1931`](https://github.com/flamewing/asl-releases/commit/772c1931461832c46f9b3154ab4ed1a63d375c5c) Reset OLMS-40 operand mode before evaluation
- [`a678a0a0`](https://github.com/flamewing/asl-releases/commit/a678a0a0f61a729b22c6c725272bbe42645ad552) Describe OLMS-40 operand forms and zero-only values
- [`2483ad42`](https://github.com/flamewing/asl-releases/commit/2483ad4271b550d394c8ba3f94e045733d0298dd) Describe IM6100 address options and zero-page bounds
- [`dadc5577`](https://github.com/flamewing/asl-releases/commit/dadc5577f7a435be09eda3db3928ee27b5b344fa) Describe IM6100 target field and page constraints
- [`ab7ac622`](https://github.com/flamewing/asl-releases/commit/ab7ac6222b3ba31560486924af79a32fe4be9e04) Describe PALM operand modifiers and branch constraints
- [`d6c273eb`](https://github.com/flamewing/asl-releases/commit/d6c273eb60b71b4eca12242ae9413cf445b465ec) Stop XCore branch decoding after target evaluation errors
- [`279c8112`](https://github.com/flamewing/asl-releases/commit/279c8112eac57bea1c67bd179b3ffa6c5179dd80) Avoid signed overflow in XCore long branch correction
- [`30e59ac3`](https://github.com/flamewing/asl-releases/commit/30e59ac391b2c6df198f1a0028db1a6c93a15e0a) Describe XCore immediate and branch constraints
- [`250871f6`](https://github.com/flamewing/asl-releases/commit/250871f68e403db1a029c6a8f9113827a153009a) Correct XCore automatic long branch prefix
- [`18346a99`](https://github.com/flamewing/asl-releases/commit/18346a99389a91cad849184be5545b5001a8659e) Accept XCore maximum positive long branch displacement
- [`dbe3f6a8`](https://github.com/flamewing/asl-releases/commit/dbe3f6a8618461c98dec01bb3174e7caaf76d3c7) Describe MIL-STD 1750 registers and branch constraints
- [`60e0b3dc`](https://github.com/flamewing/asl-releases/commit/60e0b3dcae0cb5f573568033e1694918f72bb096) Describe NEC 772x register choices and parallel conflicts
- [`dd3c3ecf`](https://github.com/flamewing/asl-releases/commit/dd3c3ecfde42e945fd0d65d0b1d4478e61b12c2f) Normalize bare NEC 772x OP components
- [`484062ed`](https://github.com/flamewing/asl-releases/commit/484062eda4262357da265ec4ca99f1bfe095535e) Validate OLMS-50 JMPIO operands before decoding
- [`c6dd228f`](https://github.com/flamewing/asl-releases/commit/c6dd228fb9a3634e7ab609a3951b6510880f9823) Stop OLMS-50 branches after target evaluation errors
- [`b0701681`](https://github.com/flamewing/asl-releases/commit/b0701681064b549f341bc6d5ebbcf1a7a63af591) Allow OLMS-50 forward branch targets to resolve
- [`bad21a1b`](https://github.com/flamewing/asl-releases/commit/bad21a1b0913d4469eb8e8c92ed08438548ffd8c) Describe OLMS-50 operands pages and branch constraints
- [`c3db365e`](https://github.com/flamewing/asl-releases/commit/c3db365eb2f20133279f8b6bd9f54f3c02810c64) Defer TMS7000 provisional branch range checks
- [`a2c4b40d`](https://github.com/flamewing/asl-releases/commit/a2c4b40d44bd963a55df1b59533012082f5cbbbe) Improve TMS7000 operand and branch diagnostics
- [`6365ebd5`](https://github.com/flamewing/asl-releases/commit/6365ebd5e1c82fbf81193dbb456b1e8ae41a18ae) Improve TMS320C5x operand and range diagnostics
- [`e7fb28d1`](https://github.com/flamewing/asl-releases/commit/e7fb28d11009a9883ad0d61c060bc80512ed4af6) Stop encoding TMS320C5x invalid conditions
- [`dd4eddd5`](https://github.com/flamewing/asl-releases/commit/dd4eddd57893a09ccddc6b531215ab78e9f1c790) Preserve NEC 77230 LDI destination errors
- [`5756273b`](https://github.com/flamewing/asl-releases/commit/5756273b8756798512697488b87292e9973facc1) Diagnose NEC 77230 excessive shift values
- [`449506e0`](https://github.com/flamewing/asl-releases/commit/449506e0174c67fa6a0b3099ef927f487dd846b9) Improve NEC 77230 register and data diagnostics
- [`ab15a499`](https://github.com/flamewing/asl-releases/commit/ab15a499f6a803dff87613b9a764b80538f3c87f) Avoid duplicate SCMP pointer diagnostics
- [`6592342e`](https://github.com/flamewing/asl-releases/commit/6592342e0540a5b9dddca94634318aef65c01b8f) Defer SCMP provisional displacement rejection
- [`e20b35cc`](https://github.com/flamewing/asl-releases/commit/e20b35cc0a8c882347a590398916051552e2883d) Improve SCMP pointer and address diagnostics
- [`31b80794`](https://github.com/flamewing/asl-releases/commit/31b80794068a5181b03becdd6b76c55ef46408c5) Reject Padauk operand pairs without accumulator
- [`951ed23b`](https://github.com/flamewing/asl-releases/commit/951ed23b3f4c7d544a8278033d2739086a1a283d) Improve Padauk address and bit diagnostics
- [`24da4152`](https://github.com/flamewing/asl-releases/commit/24da4152286bed1d02af8b315741ce8cee73eddc) Reject negative Padauk typed addresses
- [`04d36bc5`](https://github.com/flamewing/asl-releases/commit/04d36bc5100c42b6ee460db0eb32426f6a3d69d9) Keep ST6 evaluated addresses unsigned
- [`553800db`](https://github.com/flamewing/asl-releases/commit/553800db28cd6fcbfc75f9ecb466c59803f38ed4) Pack ST6 backward branches with unsigned shifts
- [`1b7cb083`](https://github.com/flamewing/asl-releases/commit/1b7cb0838550c3ee76f6cb75a26b80d82f1678bc) Defer ST6 provisional branch range checks
- [`79b4558f`](https://github.com/flamewing/asl-releases/commit/79b4558fa4cfa609726a3df719722ea46ea810b7) Improve ST6 page and operand diagnostics
- [`d4ce1460`](https://github.com/flamewing/asl-releases/commit/d4ce14609853b094c31d7e97363e6c6c2f3583e3) Preserve F2MC8L bit expression diagnostics
- [`fbbe2133`](https://github.com/flamewing/asl-releases/commit/fbbe21339cc277f9576549464ddf0ec57fe0521b) Defer F2MC8L provisional branch range checks
- [`383261ab`](https://github.com/flamewing/asl-releases/commit/383261ab1f77651407e878cbad99e7e45a5973e8) Reset F2MC8L immediate width for each instruction
- [`a955c114`](https://github.com/flamewing/asl-releases/commit/a955c114887ca3c6ae7b015ce629bd1b534d5347) Improve F2MC8L address and branch diagnostics
- [`64084505`](https://github.com/flamewing/asl-releases/commit/640845058f49ed1055391d01e9df3227f34df3bb) Respect quotes in F2MC8L bit address separators
- [`324b93ec`](https://github.com/flamewing/asl-releases/commit/324b93ecef6bf0bf3d3ace89bcdc65f646a3eb4a) Check KENBAK register values before mask shifts
- [`121f9c24`](https://github.com/flamewing/asl-releases/commit/121f9c2409bc8832c4e4f0db5266940a330c3c6a) Diagnose KENBAK disallowed addressing modes
- [`16e07c4a`](https://github.com/flamewing/asl-releases/commit/16e07c4a17f587780737c91462267f7c51bb940b) Improve KENBAK register and shift diagnostics
- [`002e06c6`](https://github.com/flamewing/asl-releases/commit/002e06c6cda32dd1613c91694ab6e9b99d164be5) Stop SC62015 branches after failed target evaluation
- [`c774a24d`](https://github.com/flamewing/asl-releases/commit/c774a24d049cdac2117f4144d1fd7282187c652d) Improve SC62015 displacement and register diagnostics
- [`4e6c02e4`](https://github.com/flamewing/asl-releases/commit/4e6c02e4ff9f588ff29521a6ba52622230b3a122) Report SC62015 malformed address brackets once
- [`bca8d7ce`](https://github.com/flamewing/asl-releases/commit/bca8d7ced19d581420ce2144440dd2e9e82e2122) Stop i960 memory encoding after invalid addresses
- [`b20d3382`](https://github.com/flamewing/asl-releases/commit/b20d3382b123b11b77bf4aa209ad260a49080488) Defer i960 branch ranges for unresolved forward expressions
- [`1c03490a`](https://github.com/flamewing/asl-releases/commit/1c03490af5519dc697b9208ffa907c9c7c18c79c) Improve i960 register, address and branch diagnostics
- [`2f0fc688`](https://github.com/flamewing/asl-releases/commit/2f0fc6882b453524a7cc164abc762a9ac64f55ff) Reject reserved and overflowing i960 index scales
- [`81a07aec`](https://github.com/flamewing/asl-releases/commit/81a07aecfa50ad597e2559e7cd8c4f6c4be82a63) Accept i960 COBR maximum positive displacement
- [`019fc353`](https://github.com/flamewing/asl-releases/commit/019fc3537983d1baee27d97545a356fd6e955ea1) Mask TI packed bytes before left shifts
- [`6f410539`](https://github.com/flamewing/asl-releases/commit/6f410539641ecacecdd692656531f3ad0723092d) Report values and bounds for TI data range errors
- [`14818ad7`](https://github.com/flamewing/asl-releases/commit/14818ad795a3b355b082a9833364f56e731bbfe5) Defer TI fixed-point ranges for unresolved expressions
- [`aecc8e59`](https://github.com/flamewing/asl-releases/commit/aecc8e599bbd1b2ed7030242af47050731bfe91f) Defer TMS370 branch ranges for forward expressions
- [`f08dd665`](https://github.com/flamewing/asl-releases/commit/f08dd665b358beac8d2c29e607b9a9ac22b9f1f2) Improve TMS370 address, bit and branch diagnostics
- [`0c86c702`](https://github.com/flamewing/asl-releases/commit/0c86c7021959544aa0fb5c21d18c8f2c203d36e1) Mask SX20 signed RETW and MODE immediate fields
- [`8b0dcaa8`](https://github.com/flamewing/asl-releases/commit/8b0dcaa8a4333317048c311bebe9fd5041c580b4) Preserve primary SX20 MOVSZ expression errors
- [`38135e28`](https://github.com/flamewing/asl-releases/commit/38135e28618a59ea266ebbfa2ad26f1b5ba4c8a6) Explain SX20 page assumptions and operand constraints
- [`c6091733`](https://github.com/flamewing/asl-releases/commit/c609173384160e84b13bab19a7a86419256b2269) Respect quoted periods in SX20 bit addresses
- [`b3351efe`](https://github.com/flamewing/asl-releases/commit/b3351efe9f88e742ec3691d8b6dd378792063c4c) Defer 8x30x IV length checks for unresolved symbols
- [`5ddc916f`](https://github.com/flamewing/asl-releases/commit/5ddc916f903cfe37528094388e724fb5641929d2) Explain 8x30x IV length and register constraints
- [`40f7b83d`](https://github.com/flamewing/asl-releases/commit/40f7b83d9e9224aa0d488a01ae003cbb4fa09d40) Preserve 9900 register errors and require indirect operands
- [`6ce93c3d`](https://github.com/flamewing/asl-releases/commit/6ce93c3d4e430823bb513c14576356d1f791f783) Discard partial 9900 BYTE data after range errors
- [`ec6e1442`](https://github.com/flamewing/asl-releases/commit/ec6e1442699e98cd6d0d1318376f48639b528c15) Defer 9900 branch constraints for forward expressions
- [`ce908e70`](https://github.com/flamewing/asl-releases/commit/ce908e7005061d5b1a1836646a7ce0b5e1c1864d) Enforce the TI990/12 restriction for SRJ and ARJ
- [`f92b39b0`](https://github.com/flamewing/asl-releases/commit/f92b39b0df544eb5c56880ca009dec17b5d09f77) Explain 9900 operand, branch and CPU constraints
- [`e91fc303`](https://github.com/flamewing/asl-releases/commit/e91fc3031308f294b49f87d97e5e6af8382c7d99) Preserve CP-1600 primary errors and reject failed BEXT conditions
- [`fda4289f`](https://github.com/flamewing/asl-releases/commit/fda4289f91d9298f6030e4fb674e9fee0ccd6299) Defer CP-1600 constraints for forward expressions
- [`96c4e7fc`](https://github.com/flamewing/asl-releases/commit/96c4e7fc856e601de12dedc952a115d99f98cd87) Discard failed CP-1600 data directives and diagnose strings
- [`8c9f6e36`](https://github.com/flamewing/asl-releases/commit/8c9f6e3639be6fa3753c27e81566d7aac43f4b85) Explain CP-1600 register, word-width and branch constraints
- [`bbf6df68`](https://github.com/flamewing/asl-releases/commit/bbf6df686614e7517b80ab3b3d8935d7daaafef3) Preserve SOB target errors and point at the target operand
- [`2517787b`](https://github.com/flamewing/asl-releases/commit/2517787bb7bccaa82bde83ff34568c9582f4750c) Explain PDP-11 and WD16 addressing and CPU constraints
- [`a877c71d`](https://github.com/flamewing/asl-releases/commit/a877c71d2d39d9ff6deec015c7fd6e19cb8bb76d) Defer 2650 page checks for forward expressions
- [`1e2c13ac`](https://github.com/flamewing/asl-releases/commit/1e2c13ace1814eccc32ca17a1224a6a129f2f1d4) Explain 2650 register, index and page constraints
- [`16903e5e`](https://github.com/flamewing/asl-releases/commit/16903e5e0c9de5aa36cfbe7c812a19f02bb16ca0) Explain MN1610 alternate addressing and segment constraints
- [`73bc5857`](https://github.com/flamewing/asl-releases/commit/73bc585745f1c3e316741a2ae23ebad8e2c94ee4) Diagnose invalid 8008 conditional instructions
- [`d3b1fcb7`](https://github.com/flamewing/asl-releases/commit/d3b1fcb7e83e13ef458ef37d33216705c45d5267) Reject unsupported 8008 byte-register loads from register pairs
- [`58577050`](https://github.com/flamewing/asl-releases/commit/58577050b5893340001f901282215a1f3e865eab) Explain 8008 register, addressing and restart constraints
- [`0470456b`](https://github.com/flamewing/asl-releases/commit/0470456b03864ce201b37999d81a9bcb1fd747ac) Explain C54x assumed DP and SP address windows
- [`1dc49d7e`](https://github.com/flamewing/asl-releases/commit/1dc49d7e1adb5b4bbe7b85fad26f097fd14935c2) Explain C54x addressing and accumulator contracts
- [`71d7a5c2`](https://github.com/flamewing/asl-releases/commit/71d7a5c239c6635862c1da17f8af2589d9669992) Fix NS32000 scaled-index evaluation failure emission
- [`15a883a3`](https://github.com/flamewing/asl-releases/commit/15a883a3d865eadf4fba16f1e92e59bd8998a516) Preserve provisional size of NS32000 forward register aliases
- [`c48d38e5`](https://github.com/flamewing/asl-releases/commit/c48d38e591c19e2b4ee3bb9d65685df6818eb329) Abort failed NS32000 explicit PC-relative encoding
- [`3d8200c4`](https://github.com/flamewing/asl-releases/commit/3d8200c40a4747fef7ea17fe2f5d66318a75d2c1) Stop NS32000 control instructions after failed displacement evaluation
- [`8ddee102`](https://github.com/flamewing/asl-releases/commit/8ddee102efb0c61b8340a450aabbfa0510ac0aa4) Explain NS32000 register types lists and pair requirements
- [`d2cf02de`](https://github.com/flamewing/asl-releases/commit/d2cf02dee6ace573a802ebe7a89dd40f368938eb) Explain NS32000 displacement bounds address forms and CPU requirements
- [`cc864665`](https://github.com/flamewing/asl-releases/commit/cc8646657aaa3ca2c239ef6873cf8359a4d6f6d8) Map NS32000 PMMU register errors and list valid choices
- [`865f4815`](https://github.com/flamewing/asl-releases/commit/865f481533a4554baf74024ff8b4b698bb862859) Retain VAX primary register errors and guard failed descriptor reads
- [`fc474e0f`](https://github.com/flamewing/asl-releases/commit/fc474e0fc66f387c2fb10b9e0ab095073e2d519f) Reject VAX PC and R15 index registers with targeted diagnostics
- [`387c7d61`](https://github.com/flamewing/asl-releases/commit/387c7d61f5605e9485ec474c01ae6a00e8aa74b1) Resolve VAX forward index aliases with mandatory register parsing
- [`9687e4b2`](https://github.com/flamewing/asl-releases/commit/9687e4b289eb8b776d4df90e3695c88e4f881198) Explain VAX addressing restrictions index hazards branch bounds and counts
- [`2cca969a`](https://github.com/flamewing/asl-releases/commit/2cca969a3d4db3910b08d320b6cb4a941c776e55) Name eligible VAX CPUs for instruction capability failures
- [`5346b5d0`](https://github.com/flamewing/asl-releases/commit/5346b5d0693e1aec8aa7a9c54c7f203f9f263b28) Defer MELPS7700 forward branch distance checks until resolution
- [`b3051e9a`](https://github.com/flamewing/asl-releases/commit/b3051e9a25a5e3b3137160b032559972992e1258) Discard MELPS7700 accumulator-B prefix after rejected operands
- [`7561afc9`](https://github.com/flamewing/asl-releases/commit/7561afc95461e839e2b89b990e9f7adc20b97e10) Gate MELPS7700 accumulator-B RMW forms before emission
- [`84fdff45`](https://github.com/flamewing/asl-releases/commit/84fdff4525a6ffa5fc33b7b4e3e76b96a1a2be0a) Explain MELPS7700 displacement ranges and assumed address windows
- [`0afd4768`](https://github.com/flamewing/asl-releases/commit/0afd4768ae270b3698376eaa375d163817d231c0) Select available MELPS7700 provisional address widths for forward operands
- [`5498a0be`](https://github.com/flamewing/asl-releases/commit/5498a0be3c8827a4a42525a88ba93425012110b1) Explain MELPS7700 addressing contracts and unavailable address forms
- [`cdc452f3`](https://github.com/flamewing/asl-releases/commit/cdc452f3fc0425113a6d97bb87f60354d6a63e5c) Reject ignored INS8070 trailing scalar operands and missing sources
- [`d8efdd11`](https://github.com/flamewing/asl-releases/commit/d8efdd114de722746dffe05158a303dc89b72c84) Explain INS8070 size conflicts address windows and register contracts
- [`076f6959`](https://github.com/flamewing/asl-releases/commit/076f695914cdc67dd3034ce447e423d70fe8844b) Defer TLCS-870/C forward branch range checks
- [`338a2ec7`](https://github.com/flamewing/asl-releases/commit/338a2ec7cddd4f1835c6686cf045b02b17bd7d59) Avoid emitting invalid TLCS-870/C SP displacements
- [`fb974ca5`](https://github.com/flamewing/asl-releases/commit/fb974ca5589c83f2463ad03e6a146cab4944647b) Clarify TLCS-870/C operand and displacement diagnostics
- [`6a090412`](https://github.com/flamewing/asl-releases/commit/6a090412bb46762db3e59c4044c6796c47012fd1) Stop CR16 load/store encoding after invalid register operands
- [`1421b4b6`](https://github.com/flamewing/asl-releases/commit/1421b4b6ff85bdc8c4c4ff532cdfe077f4685305) Resolve forward CR16 register aliases before checking pairs
- [`de4ab7b8`](https://github.com/flamewing/asl-releases/commit/de4ab7b822eb1f5ebec0137ece7d289c79d20e7d) Clarify CR16 register, memory and alignment diagnostics
- [`f15aad2e`](https://github.com/flamewing/asl-releases/commit/f15aad2e34e00a4f2c4d63a2e68ffdeb54a54d51) Defer CR16 register-only operand classification for forward aliases
- [`4e6655a0`](https://github.com/flamewing/asl-releases/commit/4e6655a0b8c99f9dee3d4719626087cfe84e2339) Defer MCS-96 forward byte branch range checks
- [`365741de`](https://github.com/flamewing/asl-releases/commit/365741de59408e3efffa1836576c243190afcf01) Defer MCS-96 forced-short operand checks until resolution
- [`30f91550`](https://github.com/flamewing/asl-releases/commit/30f91550fb34b51a346078958bf413981b2d6f16) Defer MCS-96 count operand checks for forward expressions
- [`8272bf49`](https://github.com/flamewing/asl-releases/commit/8272bf49e4047b1806b6849bc891fd1d65495e4d) Explain MCS-96 branch, displacement and count bounds
- [`73e1eed1`](https://github.com/flamewing/asl-releases/commit/73e1eed18775e3a44a27d58b8e5981ba8f7ac5c9) Explain MCS-96 register windows and address syntax
- [`a94a7aa4`](https://github.com/flamewing/asl-releases/commit/a94a7aa4663ca1336ae2a666db121267774c02f2) Defer 6800-family forward branch range checks
- [`d751bc8a`](https://github.com/flamewing/asl-releases/commit/d751bc8a0a5d3aa8b19769a5808f90f8d6a50153) Stop 6800-family JMP and JSR encoding after operand failure
- [`0eb30924`](https://github.com/flamewing/asl-releases/commit/0eb30924680e07ebaaa2b0bee57f6bf5585c4e48) Explain HC11K4 physical mapping warnings after resolution
- [`48b3d639`](https://github.com/flamewing/asl-releases/commit/48b3d6397f1aaeed899525d7bd477de7138a10a2) Clarify 6800-family operand and direct-address diagnostics
- [`9579d29f`](https://github.com/flamewing/asl-releases/commit/9579d29f8904f3908d45ecfd02295d5560a88cfc) Guard PowerPC register and condition encoding after evaluation failure
- [`6eead8cd`](https://github.com/flamewing/asl-releases/commit/6eead8cda4ff96b1ae5a0fdaa948047f5068e2db) Defer PowerPC forward branch ranges and alignment
- [`9e4998fd`](https://github.com/flamewing/asl-releases/commit/9e4998fd22c8a459a1bad6cf01f8ee2f8b28196b) Clarify PowerPC memory operand and bracket failures
- [`beb10136`](https://github.com/flamewing/asl-releases/commit/beb10136adcdc8ab9dae0267a1aed689d7616415) Clarify 6805-family index and addressing requirements
- [`9e01d4dd`](https://github.com/flamewing/asl-releases/commit/9e01d4dd3a10f5bf4149ce82430b457b049a9fc6) Defer CPU16 forward branch range and alignment checks
- [`7c539434`](https://github.com/flamewing/asl-releases/commit/7c5394341d5e3458925e72dda31d5fe8e6a4fbb9) Explain CPU16 bank warnings and defer unresolved addresses
- [`b8b3209d`](https://github.com/flamewing/asl-releases/commit/b8b3209df0f01809afc9f6e5a43a2b8cc08e6747) Clarify CPU16 register and memory operand contracts
- [`93826c40`](https://github.com/flamewing/asl-releases/commit/93826c403eecec75ffef6806088923b6cc2d5a21) Use decoded operand count when widening 65xx addresses
- [`898ff22f`](https://github.com/flamewing/asl-releases/commit/898ff22f780032b36a3b7d750e597205f70968d6) Explain HuC6280 mapping and 65CE02 base-page failures
- [`715c229e`](https://github.com/flamewing/asl-releases/commit/715c229e61aa595ab92ddcc9e80893ef819c371f) Validate 65xx address brackets and guard short suffix reads
- [`a770e835`](https://github.com/flamewing/asl-releases/commit/a770e83533f314e910ad9726106a86558a6cf3bb) Explain 65xx special-page and indirect-pointer restrictions
- [`6643549f`](https://github.com/flamewing/asl-releases/commit/6643549fc715fea85d3d01128f14f853d5be9a81) Preserve HuC6280 TST address bytes after mask insertion
- [`5f464450`](https://github.com/flamewing/asl-releases/commit/5f46445089c472ed24b455082133eb49a17db1ce) Restore 65CE02 base-page byte when widening addresses
- [`f0ddf855`](https://github.com/flamewing/asl-releases/commit/f0ddf855e2e7b3f22f7b64b004e4c014965519ed) Clarify 65xx instruction-specific addressing alternatives
- [`295b1581`](https://github.com/flamewing/asl-releases/commit/295b1581c7065fd5c7c0d8dad8114f1945ac2f61) Defer HC12 forward branch range failures
- [`28c65cdd`](https://github.com/flamewing/asl-releases/commit/28c65cddce5fddf3b74254cd577f6bae8a29c7f6) Discard partial HC12 bit branches after range rejection
- [`9c666e4b`](https://github.com/flamewing/asl-releases/commit/9c666e4bac82eec5a1c064134c9e612ffe2e5d7b) Explain HC12 memory forms and locate indirect operand failures
- [`d1454b0e`](https://github.com/flamewing/asl-releases/commit/d1454b0e9f5e17fecff564f3f7191d8be491be86) Describe HC12 TRAP intervals and defer provisional numeric constraints
- [`85f89f24`](https://github.com/flamewing/asl-releases/commit/85f89f246a8df04463be1dccf8254038784f536e) Correct HC12X BTAS memory addressing mask
- [`65749aea`](https://github.com/flamewing/asl-releases/commit/65749aea02aaf14dea87ee49b4fc62e0c066ac50) Count both BTAS opcode bytes in HC12X PC-relative addressing
- [`3983591a`](https://github.com/flamewing/asl-releases/commit/3983591a472ce5fa3339c0af797ec81a5cf4ea6e) List CPU-specific HC12 register choices in diagnostics
- [`32f44aff`](https://github.com/flamewing/asl-releases/commit/32f44affda5572396d19163eb5c189a72db5d2da) Explain HC12 register-pair constraints at the destination
- [`a4919b67`](https://github.com/flamewing/asl-releases/commit/a4919b67f1dbb03b17a01ac99e076396b6b95c84) Defer provisional HC12 table displacements and explain range failures
- [`fe4237c7`](https://github.com/flamewing/asl-releases/commit/fe4237c7c3b2936cff33da4ddb12955f404ea292) Respect quoted HC12 operand spaces without scanning before strings
- [`e6c3c6e8`](https://github.com/flamewing/asl-releases/commit/e6c3c6e826ad352e4f43f7dd50e9f9989b12a960) Use an escaped NUL in CPU16 address-form source
- [`8134d4b7`](https://github.com/flamewing/asl-releases/commit/8134d4b7fb693338ccaf981c116277dc21127841) Explain 6809 forced displacement and direct-page failures
- [`9d1b0c66`](https://github.com/flamewing/asl-releases/commit/9d1b0c660bd5c065f2cea08e792ec77ba6bb09d8) Stop 6309 W-indexed emission after expression errors
- [`4827c43a`](https://github.com/flamewing/asl-releases/commit/4827c43a53ae9ea1fdefc8dde9b14177926b5a03) Describe 6809 register widths and transfer update constraints
- [`f8411280`](https://github.com/flamewing/asl-releases/commit/f8411280c1c4660d5f7ca5c71e5cc0336e9a2544) Explain 6809 address components and CPU-specific forms
- [`cebe70d6`](https://github.com/flamewing/asl-releases/commit/cebe70d66705c6bc8761dad3b37370b27ab2c256) Preserve 6809 LDQ and HCF primary failures without emitting bytes
- [`92bcecc8`](https://github.com/flamewing/asl-releases/commit/92bcecc8a416bb8454089c10ea8d9f845d164af0) Report one located 6809 register diagnostic with supported choices
- [`d2f636a5`](https://github.com/flamewing/asl-releases/commit/d2f636a538317918ea6e5ef43089ded60e203044) Locate 6809 stack-list failures and preserve primary errors
- [`e6273d7c`](https://github.com/flamewing/asl-releases/commit/e6273d7c0d86a310cec93a9bb0b4a16c6cb1b86a) Detail M68K MAC register constraints and preserve evaluator failures
- [`f1f9a7de`](https://github.com/flamewing/asl-releases/commit/f1f9a7de61ad1fd32c4a671e3ee823d6d4590b04) Name overlapping M68K long-multiply result registers
- [`0249d102`](https://github.com/flamewing/asl-releases/commit/0249d102465a71382506147a94eea0c4061b54a1) List supported M68K processors and ColdFire ISA variants
- [`b5bc54e2`](https://github.com/flamewing/asl-releases/commit/b5bc54e277754980f26d181f7b7ac616a580ccd5) Stop CCR logical emission after rejected sizes or CPU capabilities
- [`0460d805`](https://github.com/flamewing/asl-releases/commit/0460d8050b18da591836d230bae6d35f70961709) List supported processors for M68K feature diagnostics
- [`b337f39d`](https://github.com/flamewing/asl-releases/commit/b337f39dae1e13da23613761d33b9696f8d23cc9) Explain M68K displacement and absolute address size suffixes
- [`d9736c2b`](https://github.com/flamewing/asl-releases/commit/d9736c2bdc5407c9556602627645d640063eb45a) List CPU-specific M68K integer and floating operand sizes
- [`8e8a74ff`](https://github.com/flamewing/asl-releases/commit/8e8a74ff5166de628997fee329d4e0c4079269f9) Explain M68K arithmetic sizes and stop emission after rejection
- [`d889a4f1`](https://github.com/flamewing/asl-releases/commit/d889a4f1bc4dd052d3d8dcae634b4fbcb18bbbe8) Explain M68K MOVE special and address register sizes
- [`dcacad57`](https://github.com/flamewing/asl-releases/commit/dcacad57f9ca66f26ccb19fe70607b9a537d9789) Explain M68K integer instruction width requirements
- [`48da092a`](https://github.com/flamewing/asl-releases/commit/48da092a15f8a53e1dd355c55fd63aafc476a61c) Explain M68K extended integer operand sizes
- [`4525dacf`](https://github.com/flamewing/asl-releases/commit/4525dacf78ea69e19bf417ed0093f7cfc47e21f0) Explain M68K conditional sizes and correct Scc count errors
- [`4d85ace3`](https://github.com/flamewing/asl-releases/commit/4d85ace3354f5dfed0bca1f7e15816b19b04a5f3) Explain forbidden M68K operand size suffixes
- [`13fe244c`](https://github.com/flamewing/asl-releases/commit/13fe244c01cfcaa3f606513df0807f9887e800e4) Explain operand-specific M68K bit and FPU sizes
- [`6e1c7d58`](https://github.com/flamewing/asl-releases/commit/6e1c7d585dc0e67796ee44eef3e54e08e4bc1303) Explain PMMU operand sizes and enforce PFLUSHR availability
- [`b337a498`](https://github.com/flamewing/asl-releases/commit/b337a498f096b03d4c5d98080a9a9b1a612a7ae9) Explain remaining ColdFire instruction operand sizes
- [`2b1e012b`](https://github.com/flamewing/asl-releases/commit/2b1e012b3c3eecd493b4bbc97dade400898db348) List allowed M68K addressing forms at rejected operands
- [`a6c5c4c1`](https://github.com/flamewing/asl-releases/commit/a6c5c4c1a32a7b6929dfedcadab0a3b37fdc92bf) Explain MAC scales and preserve primary evaluation errors
- [`45ba2629`](https://github.com/flamewing/asl-releases/commit/45ba2629faa049e12e910031b95c2af15c7160ae) Explain ColdFire MOVE destination combinations
- [`11721534`](https://github.com/flamewing/asl-releases/commit/11721534501f99539951ce0ad8aeebc3df3861ea) Explain M68K operand-pair and ColdFire FP list restrictions
- [`074dfb2f`](https://github.com/flamewing/asl-releases/commit/074dfb2fdd80705cadbb234943f36b7190605f27) Explain MOVEM list syntax and preserve primary lookup errors
- [`250d18f8`](https://github.com/flamewing/asl-releases/commit/250d18f8da90f2cda23070ec4c762b3742141e9e) Explain FMOVEM lists and recognize FP register aliases
- [`fdba036e`](https://github.com/flamewing/asl-releases/commit/fdba036e329fd94c605e4302f3ce55cfd9bc1f82) Explain FSINCOS destination syntax and MAC remaining counts
- [`f9ea0f19`](https://github.com/flamewing/asl-releases/commit/f9ea0f196ef2008a4f66a8d99ccadedb9844dd5b) Resolve forward M68K register lists before operand classification
- [`b6725a0f`](https://github.com/flamewing/asl-releases/commit/b6725a0f00f68860e39490471d3590da0bbda89e) Improve AVR register subset diagnostics and forward aliases
- [`c92da6dd`](https://github.com/flamewing/asl-releases/commit/c92da6dd78314efb2b007a47d05da511224713b3) Describe AVR pointer restrictions and register overlaps
- [`a39151aa`](https://github.com/flamewing/asl-releases/commit/a39151aaf715eb9d8f1c2c83c85f21968e0ae6db) Explain AVR core capability and bit operand counts
- [`99936ffb`](https://github.com/flamewing/asl-releases/commit/99936ffb1ce342bef471485fb1bc3916ce363ee9) Report AVR word branch ranges and alignment addresses
- [`88169b23`](https://github.com/flamewing/asl-releases/commit/88169b23b018e6bc8e283271b219d8826ae991ad) Use word units for AVR byte-mode relative branch wrapping
- [`5b4a2666`](https://github.com/flamewing/asl-releases/commit/5b4a266633aa73567dd812171de3785bc79715a8) Diagnose IMP-16 accumulator restrictions and preserve register errors
- [`99541bfa`](https://github.com/flamewing/asl-releases/commit/99541bfa255dd2b64293980ee7bc9e4b2176f72f) Explain IMP-16 addressing and control-transfer constraints
- [`0daa4487`](https://github.com/flamewing/asl-releases/commit/0daa4487ae87ae3b066daf409b2607b1447dc591) Report unknown required IMP-16 conditions and PACE status flags
- [`8ae1671c`](https://github.com/flamewing/asl-releases/commit/8ae1671c0c35cfbaf665e32892e05f618a61f63b) Explain Konami register and branch constraints and defer unknown targets
- [`49f919b0`](https://github.com/flamewing/asl-releases/commit/49f919b0af5c879efb7e8f5333754d3d04c7c13a) Explain Konami addressing forms and reject ignored components
- [`1364c5e4`](https://github.com/flamewing/asl-releases/commit/1364c5e446dd9c20ba58202dc7af07d64708a940) Report Konami short displacement and direct-page constraints
- [`5e847e0f`](https://github.com/flamewing/asl-releases/commit/5e847e0ffa61cc96d06771d7cd6cacf7dcdadb7c) Report Z8 branch displacements and defer unresolved target arithmetic
- [`2913294b`](https://github.com/flamewing/asl-releases/commit/2913294b4a828ab75d404323541bbeb6a9be32ba) Describe Z8 core availability and bit operand counts
- [`aed7fc0b`](https://github.com/flamewing/asl-releases/commit/aed7fc0bc048e81c79373992bc907a6e56eca85e) Explain Z8 working-register windows and initialize forced indices
- [`0b49419b`](https://github.com/flamewing/asl-releases/commit/0b49419bfb86623ce5ee2988511046d4752f897b) List Z8 addressing alternatives and conditional jump restrictions
- [`dd4e0097`](https://github.com/flamewing/asl-releases/commit/dd4e0097d6902e4dd7fc9c52f706ee83ada02220) Diagnose Z8 bit operand windows and preserve primary failures
- [`4bd2496b`](https://github.com/flamewing/asl-releases/commit/4bd2496be66ee8254783c178a50eb46f88d1fb88) Validate Z8 native register indices before narrowing
- [`189da7b3`](https://github.com/flamewing/asl-releases/commit/189da7b32e886a99b9266d33e8a7e0711ff05600) Explain MCore branch and literal-table constraints
- [`a695e659`](https://github.com/flamewing/asl-releases/commit/a695e659795f221260bdf4908cf74902cd264d5a) Explain MCore register constraints and validate native indices
- [`1964cf84`](https://github.com/flamewing/asl-releases/commit/1964cf84035201b4e82d78813f5c5610a4bb6238) Explain MCore address, size and immediate failures
- [`1d375252`](https://github.com/flamewing/asl-releases/commit/1d3752528b4860f3a1df541c3a9f33bee55451d0) Prevent overflow in MCore aggregate address displacement
- [`4394eb15`](https://github.com/flamewing/asl-releases/commit/4394eb15e2067be0daae7a19f3d9f758edbd1405) Encode MCore JSRI as an indirect subroutine call
- [`4196918d`](https://github.com/flamewing/asl-releases/commit/4196918dbed1f939ed8dd342b1a70823796ed397) Explain TLCS-90 short branches and defer unknown targets
- [`8de2d643`](https://github.com/flamewing/asl-releases/commit/8de2d64314cfe5a36a7c2c1269d10cd3e0946dec) Identify TLCS-90 fixed register and address requirements
- [`f629f7eb`](https://github.com/flamewing/asl-releases/commit/f629f7eb89deea4a251001fbf050b4823200fb04) Read complete external message catalogs beyond 64 KiB
- [`0fc9ce09`](https://github.com/flamewing/asl-releases/commit/0fc9ce09d6de4f3d2941e8fcb834dbc68a76c204) Explain TLCS-90 address classes and operand widths
- [`b3d171d5`](https://github.com/flamewing/asl-releases/commit/b3d171d549bfba9e782bb1a23ed916c49a19161c) Reject empty TLCS-90 operands before register decoding
- [`e3e9b8fb`](https://github.com/flamewing/asl-releases/commit/e3e9b8fb01b3334897328d809ecbfc64f99bd858) Validate full TLCS-90 absolute address values before narrowing
- [`0b295d3a`](https://github.com/flamewing/asl-releases/commit/0b295d3abf4db1d95560f3c31e52fcf8a1f7349d) Explain TC9331 shifts and preserve primary expression failures
- [`d6e6638c`](https://github.com/flamewing/asl-releases/commit/d6e6638c874c10f6bc9f40cf997b56f1be5dcc24) List TC9331 instruction-specific field and flag choices
- [`cfd91e06`](https://github.com/flamewing/asl-releases/commit/cfd91e06c219cb3b0ef0bf52c1c445021491068d) Identify TC9331 reserved first instruction fields
- [`7283d727`](https://github.com/flamewing/asl-releases/commit/7283d727ef98b6ee37075ee8e23ee4a2fe1eb62a) Fix TC9331 comments in final instruction fields
- [`4a107e03`](https://github.com/flamewing/asl-releases/commit/4a107e03b0887cb702ea0d1b9d58e57d66b242f4) Explain ST9 relative branch displacement failures
- [`307e4545`](https://github.com/flamewing/asl-releases/commit/307e4545ed42868124fcfbbe3573f5e6e1163648) Fix ST9 working-register update diagnostics and failure handling
- [`5b2e9920`](https://github.com/flamewing/asl-releases/commit/5b2e992090d91dc092de0f49df8dca8ea7673b5f) Explain ST9 bit complement and immediate prefix errors
- [`821f9910`](https://github.com/flamewing/asl-releases/commit/821f9910f84dd33fbc1db04c4e7a1bdd2364dd0e) Explain ST9 shared address constraints
- [`4d33b3ea`](https://github.com/flamewing/asl-releases/commit/4d33b3ea8b80c252b7415eba64dfd3cf69626d6c) Fix ST9 failed and unresolved address expressions
- [`a90e0277`](https://github.com/flamewing/asl-releases/commit/a90e0277fd8375ed5c03c856e7afe6caf247047f) Fix ST9 byte register descriptor upper boundary
- [`49e4e21c`](https://github.com/flamewing/asl-releases/commit/49e4e21cead57f082a72a6f1570c40b69e5b229a) Explain MN1610 effective address ranges and stop invalid emission
- [`be1c8bb4`](https://github.com/flamewing/asl-releases/commit/be1c8bb4cc8057fdb778c0ed529e8b581ce36725) Explain MN1610 shared operand and immediate constraints
- [`b6271ca8`](https://github.com/flamewing/asl-releases/commit/b6271ca8e388ffab147e7137004ea8940c232d6d) Explain MN1613 fixed register and block pointer constraints
- [`f8b768ba`](https://github.com/flamewing/asl-releases/commit/f8b768ba3c2998d54ca99e3d8523f13951decccd) List MN1610 shared operand names and optional alternatives
- [`3e9a2234`](https://github.com/flamewing/asl-releases/commit/3e9a2234d3ef749c6b12d32c9c75ccfb85f791fd) Fix ST7 packed bit branch target crash
- [`58e670d9`](https://github.com/flamewing/asl-releases/commit/58e670d9467e368e00d43cb559720a0bd754890b) Explain ST7 and STM8 branch displacement limits
- [`7535ff75`](https://github.com/flamewing/asl-releases/commit/7535ff75da49702f89559664b80fbdeb7afe1220) Validate STM8 branch distances at full address width
- [`1275d803`](https://github.com/flamewing/asl-releases/commit/1275d803cf8f6d1c4395c9a3a3fce47183f4345b) Clarify ST7 diagnostics for STM8-only instructions

## Updating this document

When a new snapshot tag is created, move the applicable pending entries into a section bounded by the previous tag and the new tag, retaining ancestry order and the exact tag name. Add later commits to Pending changes, update its ending revision and comparison link, and keep upstream imports separate from fork changes. Summarize substantive behavior and keep the commit list complete, including tests and build/documentation changes. Keep each paragraph and list item on one physical line so editors and previewers handle wrapping.
