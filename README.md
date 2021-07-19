# Badges

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/gpl-2.0)

[![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/flamewing/asl-releases?label=codefactor&logo=codefactor&logoColor=white)](https://www.codefactor.io/repository/github/flamewing/asl-releases)
<!-- [![LGTM Alerts](https://img.shields.io/lgtm/alerts/github/flamewing/asl-releases?logo=LGTM)](https://lgtm.com/projects/g/flamewing/asl-releases/alerts/)
[![LGTM Grade](https://img.shields.io/lgtm/grade/cpp/github/flamewing/asl-releases?logo=LGTM)](https://lgtm.com/projects/g/flamewing/asl-releases/context:cpp) -->

[![CI Mac OS Catalina 10.15](https://img.shields.io/github/workflow/status/flamewing/asl-releases/ci-macos?label=CI%20Mac%20OS%20X&logo=Apple&logoColor=white)](https://github.com/flamewing/asl-releases/actions?query=workflow%3Aci-macos)
[![CI Ubuntu 20.04](https://img.shields.io/github/workflow/status/flamewing/asl-releases/ci-linux?label=CI%20Ubuntu&logo=Ubuntu&logoColor=white)](https://github.com/flamewing/asl-releases/actions?query=workflow%3Aci-linux)
[![CI Windows Server 2019](https://img.shields.io/github/workflow/status/flamewing/asl-releases/ci-windows?label=CI%20Windows&logo=Windows&logoColor=white)](https://github.com/flamewing/asl-releases/actions?query=workflow%3Aci-windows)

[![Coverity Scan Analysis](https://img.shields.io/github/workflow/status/flamewing/asl-releases/coverity-scan?label=Coverity%20Scan%20Analysis&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://github.com/flamewing/asl-releases/actions?query=workflow%3Acoverity-scan)
[![Coverity Scan](https://img.shields.io/coverity/scan/13716?label=Coverity%20Scan%20Results&logo=data:image/svg+xml;base64,PHN2ZyB2aWV3Qm94PSIwIDAgMjU2IDI1MiIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48cGF0aCBkPSJNMjYuOTUgMTA5LjA4bC0zLjUyLTkuNDUgMzcuOTYgNzAuODloLjg1bDQ3LjMzLTExOC4xM2MuODMtMi41NiA4LjI2LTIxLjc0IDguNTEtMzAuMi42My0yMS44NC0xNC4xLTIzLjgxLTI5Ljc3LTE5LjM5QzM2Ljg3IDE5LjQ2LS4yNCA2Ny44My4wMSAxMjQuNzhjLjIgNTIuOTcgMzIuNjQgOTguMjQgNzguNjUgMTE3LjM4TDI2Ljk1IDEwOS4wOE0xNzQuMzMgNS40OGMtNi4zMiAxMi43LTEzLjEgMjYuMzctMjEuNjggNDguMDhMNzkuMjIgMjQyLjM5YzE1LjA5IDYuMiAzMS42MyA5LjYgNDguOTYgOS41MiA3MC41LS4yNyAxMjcuNDItNTcuNjcgMTI3LjEzLTEyOC4xOC0uMjItNTMuODMtMzMuNzYtOTkuNy04MC45OC0xMTguMjYiIGZpbGw9IiNmZmYiLz48L3N2Zz4=)](https://scan.coverity.com/projects/flamewing-asl-releases)

[![Snapshot build](https://img.shields.io/github/workflow/status/flamewing/asl-releases/snapshots?label=Snapshot%20build)](https://github.com/flamewing/asl-releases/actions?query=workflow%3Asnapshots)
[![Latest snapshot](https://img.shields.io/github/release-date/flamewing/asl-releases?label=Latest%20snapshot)](https://github.com/flamewing/asl-releases/releases/latest)

## asl-releases

Custom version of AS, intended to track the original source and fix some issues.

## Create and install the package

You need a C development toolchain, including `make`. You also need Git. You need to copy a file from `Makefile.def-samples` directory according to your platform to a file named `Makefile.def` on the same level as `Makefile`. You can then run the following commands:

```bash
   make binaries
   make install
```

Where the later will probably require administrative privileges. This has been tested on the following environments:

- Windows 10 Home and Windows Server 2019 (32- and 64-bit builds)
- Ubuntu 20.04 (64-bit build)
- Mac OS Catalina 10.15 (64-bit build)

## Preparing your disassembly

This is only relevant for Sonic disassemblies based on older versions of the Sonic Retro community disassemblies.

More recent versions of AS have made a few changes that impact older disassemblies. Here are the changes and how to deal with them:

- AS by default will look for includes in the directory of the current file being assembled. This has an effect on files being included by other files inside a sibdirectory. In the S2 Git disassembly, this issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/3357ddcc2b6f5f44fdd09b8014350b76c701ab57), which just adds the disassembly's directory to the command line.
- `moveq` no longer silently sign-extends a value; instead, it gives an error if you do use a value that cannot be represented as a byte sign-extended to 32-bits. In the S2 Git disassembly, this issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f) in the S2 Git disassembly.
- `phase`/`dephase` were changed to be a stack: you now need a matching `dephase` for every `phase`. This issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/dbaa1b934fe04bde42764e34e42b9a1073a4bd4f) in the S2 Git disassembly.
- Adding a character constant to an integer results in a character constant, which can overflow the limits of a character and give an error. This issue was dealt with in [this commit](https://github.com/sonicretro/s2disasm/commit/0c1ace5c1b6b57157836d83416615285ca95c605) in the S2 Git disassembly.
