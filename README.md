# Badges

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0)

[![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/flamewing/asl-releases?label=codefactor&logo=codefactor&logoColor=white)](https://www.codefactor.io/repository/github/flamewing/asl-releases)
<!-- [![LGTM Alerts](https://img.shields.io/lgtm/alerts/github/flamewing/asl-releases?logo=LGTM)](https://lgtm.com/projects/g/flamewing/asl-releases/alerts/)
[![LGTM Grade](https://img.shields.io/lgtm/grade/cpp/github/flamewing/asl-releases?logo=LGTM)](https://lgtm.com/projects/g/flamewing/asl-releases/context:cpp) -->

[![CI Emscripten](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-emscripten.yml?label=CI%20Emscripten&logo=nodedotjs&logoColor=white)](https://github.com/flamewing/asl-releases/actions/workflows/ci-emscripten.yml?query=workflow%3Aci-emscripten)
[![CI Mac OS X](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-macos.yml?label=CI%20Mac%20OS%20X&logo=Apple&logoColor=white)](https://github.com/flamewing/asl-releases/actions/workflows/ci-macos.yml?query=workflow%3Aci-macos)
[![CI Ubuntu](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-linux.yml?label=CI%20Ubuntu&logo=Ubuntu&logoColor=white)](https://github.com/flamewing/asl-releases/actions/workflows/ci-linux.yml?query=workflow%3Aci-linux)
[![CI Windows](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/ci-windows.yml?label=CI%20Windows&logo=Windows&logoColor=white)](https://github.com/flamewing/asl-releases/actions/workflows/ci-windows.yml?query=workflow%3Aci-windows)

[![Coverity Scan Analysis](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/coverity-scan.yml?label=Coverity%20Scan%20Analysis&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://github.com/flamewing/asl-releases/actions/workflows/coverity-scan.yml?query=workflow%3Acoverity-scan)
[![Coverity Scan](https://img.shields.io/coverity/scan/flamewing-asl-releases?label=Coverity%20Scan%20Results&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://scan.coverity.com/projects/flamewing-asl-releases)

[![Snapshot build](https://img.shields.io/github/actions/workflow/status/flamewing/asl-releases/snapshots.yml?label=Snapshot%20build)](https://github.com/flamewing/asl-releases/actions/workflows/snapshots.yml?query=workflow%3Asnapshots)
[![Latest snapshot](https://img.shields.io/github/release-date/flamewing/asl-releases?label=Latest%20snapshot)](https://github.com/flamewing/asl-releases/releases/latest)

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

- Windows (32- and 64-bit builds)
- Ubuntu (64-bit build)
- Mac OS X (x86-64 build)

Note that, on Windows, I have only tested with MinGW under MSYS2. I would like to know if it works on other circumstances.

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

Here is a full example of how to cross-compile to WASM/Emscripten. This is based on [this](https://gist.github.com/ericoporto/8ce679ecb862a12795156d31f3ecac49). Start by getting the latest SDK:

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

## Preparing your disassembly

This is only relevant for Sonic disassemblies based on older versions of the Sonic Retro community disassemblies.

More recent versions of AS have made a few changes that impact older disassemblies. Here are the changes and how to deal with them:

- AS by default will look for includes in the directory of the current file being assembled. This has an effect on files being included by other files inside a sibdirectory. In the S2 Git disassembly, this issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/3357ddcc2b6f5f44fdd09b8014350b76c701ab57), which just adds the disassembly's directory to the command line.
- `moveq` no longer silently sign-extends a value; instead, it gives an error if you do use a value that cannot be represented as a byte sign-extended to 32-bits. In the S2 Git disassembly, this issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f) in the S2 Git disassembly.
- `phase`/`dephase` were changed to be a stack: you now need a matching `dephase` for every `phase`. This issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f) in the S2 Git disassembly.
- Adding a character constant to an integer results in a character constant, which can overflow the limits of a character and give an error. This issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/0c1ace5c1b6b57157836d83416615285ca95c605) in the S2 Git disassembly.
