# Badges

[![License: GPL v2][gpl-badge]][gpl-url]

[![CodeFactor Grade][codefactor-badge]][codefactor-url]
[![CodeQL][codeql-badge]][codeql-url]

[![CI Emscripten][emscripten-badge]][emscripten-url]
[![CI FreeBSD][freebsd-badge]][freebsd-url]
[![CI Mac OS X][mac-os-x-badge]][mac-os-x-url]
[![CI OmniOS][omnios-badge]][omnios-url]
[![CI Ubuntu][ubuntu-badge]][ubuntu-url]
[![CI Windows MinGW][windows-mingw-badge]][windows-mingw-url]
[![CI Windows MSVC][windows-msvc-badge]][windows-msvc-url]

[![Coverity Scan Analysis][scan-analysis-badge]][scan-analysis-url]
[![Coverity Scan][scan-badge]][scan-url]

[![Snapshot build][snapshot-build-badge]][snapshot-build-url]
[![Snapshot build][snapshot-link-badge]][snapshot-link-url]

## asl-releases

Custom version of AS, intended to track the original source and fix some issues.

## Create and install the package

You need a C development toolchain, and `cmake` at least `3.19`. You also need Git. When everything is setup, run the following commands:

```bash
cmake -S . -B build -G <generator>
cmake --build build -j
```

Here, `<generator>` stands for any backend for `cmake` to use. I personally use `Ninja`, but you can also use any others, like `Unix Makefiles`, or `MSYS Makefiles`. Check `cmake` help for generators available in your platform.

To install the program, run

```bash
cmake --install build
```

Where the later will probably require administrative privileges.

To run tests, build as above then run:

```bash
ctest --build-run-dir build --test-dir build -j
```

This build procedure has been tested on the following environments:

- FreeBSD (x86-64 build)
- Mac OS X (x86-64 build) with Ninja and XCode
- OmniOS (x86-64 build)
- Ubuntu (x86-64 build)
- Windows (32- and 64-bit builds) with MinGW64/MSYS2
- Windows (64-bit build) with Microsoft Visual Studio
- WASM/Emscripten cross compiled from Linux

I would like to know if it works on other circumstances, particularly when cross-compiling.

## Cross-compiling

To cross-compile, you need to have an appropriate toolchain file for your target, in addition to the actual toolchain. You start by doing a (potentially partial) host build:

```bash
cmake -S . -B build-host -G <host-generator>
cmake --build build-host -j --target rescomp
```

Then you do the cross-compile build itself:

```bash
cmake -S . -B build-target -G <target-generator> \
        -DIMPORT_EXECUTABLES=build-host/ImportExecutables.cmake \
        -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain/file.cmake
cmake --build build-target -j
```

If the toolchain file defines an appropriate emulator, you can even run the tests:

```bash
ctest --build-run-dir build-target --test-dir build-target -j
```

### Worked cross-compile example: WASM target, Linux host

Here is a full example of how to cross-compile to WASM/Emscripten. This is based on [this][emscripten-gist]. Start by getting the latest SDK:

```bash
cd ~
mkdir emsdk
cd emsdk
git clone https://github.com/emscripten-core/emsdk.git .
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh   # Change this to match your shell
```

Then on the `asl-releases` directory:

```bash
cd ~
mkdir asl
cd asl
git clone https://github.com/flamewing/asl-releases.git .
cmake -S . -B build-host -G <host-generator>
cmake --build build-host -j --target rescomp
emcmake cmake -S . -B build-wasm \
        -DIMPORT_EXECUTABLES=build-host/ImportExecutables.cmake
cmake --build build-wasm -j
cmake --build build-wasm -j --target test
```

## Building the documentation

The documentation is built by default if you have [Pandoc][pandoc-url] installed and available through the `PATH` environment variable; otherwise, it is skipped.

"Building", in this case, means turning a loose bunch of Markdown files into a self-contained html file, which will be installed to the `docs` directory when running `cmake --install build`.

Pandoc can be installed in a few OSes as follows (adapted from [this][pandoc-install]):

- Windows: there are 3 choices: either the [installer][pandoc-releases], or one of the following options:

  ```powershell
  winget install --source winget --exact --id JohnMacFarlane.Pandoc
  ```

  or

  ```powershell
  choco install pandoc
  ```

  There is, unfortunately, no package available for MSYS2 (see the [tracking issue][pandoc-msys2]). So if you are using this environment, you will have to add the installation path of Pandoc to the PATH environment on your profile's `.bashrc`.

- macOS:

  ```bash
  brew install pandoc
  ```

- Linux: use your distro's package manager.

- BSD: use your OS's package manager.

## Preparing your disassembly

This is only relevant for Sonic disassemblies based on older versions of the Sonic Retro community disassemblies.

More recent versions of AS have made a few changes that impact older disassemblies. Here are the changes and how to deal with them:

- AS by default will look for includes in the directory of the current file being assembled. This has an effect on files being included by other files inside a subdirectory. In the S2 Git disassembly, this issue was dealt with in [this commit][subdir-fix-commit], which just adds the disassembly's directory to the command line.
- `moveq` no longer silently sign-extends a value; instead, it gives an error if you do use a value that cannot be represented as a byte sign-extended to 32-bits. This issue was dealt with in [this commit][moveq-fix-commit] in the S2 Git disassembly.
- `phase`/`dephase` were changed to be a stack: you now need a matching `dephase` for every `phase`. This issue was dealt with in [this commit][phase-fix-commit] in the S2 Git disassembly.
- Adding a character constant to an integer results in a character constant, which can overflow the limits of a character and give an error. This issue was dealt with in [this commit][char-fix-commit] in the S2 Git disassembly.

[emscripten-gist]: https://gist.github.com/ericoporto/8ce679ecb862a12795156d31f3ecac49
[subdir-fix-commit]: https://github.com/sonicretro/s2disasm/commit/3357ddcc2b6f5f44fdd09b8014350b76c701ab57
[moveq-fix-commit]: https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f
[phase-fix-commit]: https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f
[char-fix-commit]: https://github.com/sonicretro/s2disasm/commit/0c1ace5c1b6b57157836d83416615285ca95c605
[gpl-badge]: https://img.shields.io/badge/license-GPL--2.0--or--later-blue
[codefactor-badge]: https://img.shields.io/codefactor/grade/github/flamewing/asl-releases?label=CodeFactor%20Grade&logoColor=white&logo=codefactor
[codeql-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/codeql.yml?label=CodeQL&logoColor=white&logo=github
[emscripten-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-emscripten.yml?label=CI%20Emscripten&logoColor=white&logo=nodedotjs
[freebsd-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-freebsd.yml?label=CI%20FreeBSD&logoColor=white&logo=freebsd
[mac-os-x-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-macos.yml?label=CI%20Mac%20OS%20X&logoColor=white&logo=Apple
[omnios-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-omnios.yml?label=CI%20OmniOS&logoColor=white&logo=data:image/svg%2bxml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSI1MTIiIGhlaWdodD0iNTEyIj48ZyBzdHlsZT0iZmlsbDojZmZmIj48cGF0aCBkPSJNMzE2LjcwOCA1MDcuOTIxYy0zNC40ODItNi4yODQtNTQuMzM1LTEzLjg0Ni03OC40NDMtMjkuODc4LTE0LjU1LTkuNjc2LTMyLjc0LTI2LjkxNy00Mi44NjYtNDAuNjMzLTIyLjI1My0zMC4xNC0zMy41MjQtNjcuODQxLTMxLjg1Ni0xMDYuNTU3IDEuOS00NC4xMDQgMTcuNzY3LTc5Ljk1MyA0OS4xODEtMTExLjEyIDI0LjktMjQuNzA0IDUzLjAyLTQwLjA0IDg4LjUzLTQ4LjI4MiAxNC44OC0zLjQ1NCA0Ny43OTctNC4zIDYzLjc5NS0xLjY0IDcwLjY2NyAxMS43NTIgMTI2LjI1OCA2MS42NSAxNDIuMzYgMTI3Ljc4IDUuOTUgMjQuNDM3IDYuMzUgNTEuMjggMS4xNDIgNzYuNTk4LTEzLjIwNiA2NC4xOTctNjUuMzU4IDExNS4yNjUtMTMzLjg4NCAxMzEuMTAxLTcuNzIzIDEuNzg1LTE1LjQ5IDIuNDQyLTMyLjU3NCAyLjc1Ni0xMi4zMzguMjI3LTIzLjc2Mi4xNzEtMjUuMzg1LS4xMjV6bTQ2LjYzNy03NS4wMTFjMTYuNTkyLTQuNTMzIDMyLjk2My0xNS4yODMgNDQuMzItMjkuMTAzIDYuOTM2LTguNDQyIDE1LjUzNC0yNS4yNyAxOC4xNTItMzUuNTMgNi42MzctMjYuMDEgNC4yNC01NC41MDEtNi40MTktNzYuMjgyLTE3LjkzLTM2LjYzOC01My4xMjgtNTUuOTktOTMuMDUzLTUxLjE2My00MS42NDggNS4wMzYtNzMuNTc5IDM3Ljg5My03OS4zOSA4MS42OTItMS43NzIgMTMuMzU1LS43MzggMzIuMjc2IDIuNDM3IDQ0LjYyIDguOTQzIDM0Ljc2NiAzNC42MjMgNTkuODg3IDY4LjQ5NyA2Ny4wMDIgMTAuMDQ3IDIuMTEgMzUuNzY1IDEuNDEgNDUuNDU2LTEuMjM2eiIgc3R5bGU9ImRpc3BsYXk6aW5saW5lO2ZpbGw6I2ZmZjtzdHJva2Utd2lkdGg6MS4xNzc0MyIgdHJhbnNmb3JtPSJtYXRyaXgoMS4wMDQgMCAwIDEuMDE1IC0yLjE0NCAtMy45MSkiLz48cGF0aCBkPSJNMTQzLjE0NyAzOTYuMzk5Yy0yNi4zNzEtOC43MDgtNTQuNTUtMjQuODIyLTc1Ljg2NC00My4zODZDMzYuMDQ4IDMyNS44MSAxNC4wOTkgMjg3Ljk4OCA0Ljc4NiAyNDUuMzJjLTIuMy0xMC41NC0yLjYzLTE1LjUzLTIuNjUtNDAuMjE0LS4wMi0yNS4wMTIuMjgtMjkuNTY3IDIuNjcyLTQwLjUxIDcuMTg1LTMyLjg3NiAxOS43MzUtNTkuMzIgMzkuOTUyLTg0LjE4NCAzMy4wODMtNDAuNjg2IDgwLjQ5Ny02Ny4xOCAxMzQuMzEzLTc1LjA0OCAxNy45NzItMi42MjggNTIuOTk3LTEuNzEgNjkuNDY3IDEuODIzIDI3Ljg3NiA1Ljk3NyA1OS41NiAxOS42NTQgODEuMDYxIDM0Ljk5MiAzMS4yNTQgMjIuMjk1IDU2Ljg2IDU0LjM2NiA3MC4yMjcgODcuOTU2IDQuMjQ2IDEwLjY2OSAxMC45NzMgMzQuMDYgMTAuMDQ3IDM0LjkzNS0uMjY2LjI1LTUuNzk2LTEuMzAyLTEyLjI5LTMuNDUtMjEuMzYyLTcuMDY5LTQ2LjYxMi0xMC43MjMtNjYuMjcyLTkuNTkxbC0xMC43NzkuNjItNC44ODQtOS44MDJjLTYuMzEyLTEyLjY2Ny0xNi4zODktMjYuMDUzLTI3LjM4Mi0zNi4zNzMtMjEuNTU1LTIwLjIzNS00NC4yOTEtMjkuNjUtNzQuNDYtMzAuODMzLTM4LjEyNS0xLjQ5Ni02Ni43NSA5LjYwNC05MS44NjggMzUuNjI0LTE1LjYwMiAxNi4xNjItMjUuNjg0IDMzLjc0NC0zMS4yODUgNTQuNTU2LTkuMzc0IDM0LjgzOC01LjgwMyA3My44MzggOS41MDYgMTAzLjgwNyA5LjAwNiAxNy42MyAyNC41NDggMzUuMjU0IDM5Ljc4MiA0NS4xMDhsNy45MjcgNS4xMjl2MTguNTMyYzAgMTkuNzEyIDIuNDUgMzguMDY3IDcuMDQ4IDUyLjgxNyAyLjY0MSA4LjQ3MiAyLjc5NSA5LjQyIDEuNTEyIDkuMzM2LS40ODctLjAzMi02LjQ2NC0xLjktMTMuMjgzLTQuMTUxem0xMzMuMTc0LTcuMjIzYy00LjY0MS02Ljk3NS04Ljc0MS0xNS41MDQtMTAuODczLTIyLjYyLTIuNjc0LTguOTIxLTQuMjkzLTI1LjkyNS0zLjQzMy0zNi4wNWwuNzE4LTguNDU1IDcuNzM1LTQuNDZjMTYuNjM2LTkuNTkzIDM3LjM2MS0zMy4wMjEgNDYuNjk3LTUyLjc4NyAzLjk2OS04LjQwMyA0LjUzMi04LjYzIDIxLjIzOC04LjYwNCAxMi43NjguMDIgMjMuMDIgMi4yMSAzMi4zOTkgNi45MTkgOC4xODUgNC4xMDkgMTguNTUgMTIuMjk3IDIzLjEzNyAxOC4yNzdsMy4zMjYgNC4zMzctMi4xODMgNC44OTJjLTE3LjU1IDM5LjM0LTU0LjQ2MiA3NS41MjgtOTcuOTI5IDk2LjAxLTE2LjkyNyA3Ljk3Ni0xNy4xOTQgOC4wMDktMjAuODMyIDIuNTQxeiIgc3R5bGU9ImRpc3BsYXk6aW5saW5lO2ZpbGw6I2ZmZjtzdHJva2Utd2lkdGg6MS4xNzc0MyIgdHJhbnNmb3JtPSJtYXRyaXgoMS4wMDQgMCAwIDEuMDE1IC0yLjE0NCAtMy45MSkiLz48L2c+PC9zdmc+
[ubuntu-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-linux.yml?label=CI%20Ubuntu&logoColor=white&logo=Ubuntu
[windows-mingw-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-windows-mingw.yml?label=CI%20Windows%20MinGW&logoColor=white&logo=Windows
[windows-msvc-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-windows-msvc.yml?label=CI%20Windows%20MSVC&logoColor=white&logo=Windows
[scan-analysis-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/coverity-scan.yml?label=Coverity%20Scan%20Analysis&logoColor=white&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=
[scan-badge]: https://img.shields.io/coverity/scan/flamewing-asl-releases?label=Coverity%20Scan%20Results&logoColor=white&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=
[snapshot-build-badge]: https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/snapshots.yml?label=Snapshot%20build
[snapshot-link-badge]: https://img.shields.io/github/release-date/flamewing/asl-releases?label=Latest%20snapshot
[gpl-url]: https://www.gnu.org/licenses/gpl-2.0
[codefactor-url]: https://www.codefactor.io/repository/github/flamewing/asl-releases
[codeql-url]: https://github.com/flamewing/asl-releases/actions/workflows/codeql.yml
[emscripten-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-emscripten.yml
[freebsd-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-freebsd.yml
[mac-os-x-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-macos.yml
[omnios-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-omnios.yml
[ubuntu-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-linux.yml
[windows-mingw-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-windows-mingw.yml
[windows-msvc-url]: https://github.com/flamewing/asl-releases/actions/workflows/ci-windows-msvc.yml
[scan-analysis-url]: https://github.com/flamewing/asl-releases/actions/workflows/coverity-scan.yml
[scan-url]: https://scan.coverity.com/projects/flamewing-asl-releases
[snapshot-build-url]: https://github.com/flamewing/asl-releases/actions/workflows/snapshots.yml
[snapshot-link-url]: https://github.com/flamewing/asl-releases/releases/latest
[pandoc-url]: https://pandoc.org
[pandoc-releases]: https://github.com/jgm/pandoc/releases/tag/3.1.13
[pandoc-install]: https://pandoc.org/installing.html
[pandoc-msys2]: https://github.com/msys2/MINGW-packages/issues/12485
