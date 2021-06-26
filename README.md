The Macroassembler AS Releases
==============================

[The Macroassembler AS][asl], also known as "ASL," is a multi-platform
cross-assembler. Build platforms include a variety of Unix systems
(including Linux and MacOS X), Windows, OS/2 and DOS (both native and
with an extender). Target platforms cover a huge variety of 8- and
16-bit CPUs and microcontrollers.

The `upstream` branch of this repository contains the source code for
every publicly available [source release][src] of the C version. The
tools to do this and this documentation are on the `master` branch.

Pull requests (to improve the import system and its documentation) are
accepted for the `master` branch. The `upstream` branch containing the
vendor sources never contains patches, but there may be patch branches
derived from imported vendor commits on `upstream`.

### Branches in this Repo

- `master`: The import script and its documentation.
- `upstream`: Imported ASL source code for each release.
- `dev/flamweing/current`: A recent version tested to build on msys2 and
  assemble [S2 Disassembly (slightly tweaked)](https://github.com/sonicretro/s2disasm), Sonic Classic Heroes, and Big's Fishing Derby.
- `dev/NAME/...`: Branches from [Macroassembler-AS/asl-releases](https://github.com/Macroassembler-AS/asl-releases)


Importing New ASL Releases
--------------------------

Copy the `download.sh` script to a location outside of the
repository in which you're going to do the import, change to the
`upstream` branch, and then run `download.sh`. This will find all new
[source releases][src] that have not yet been imported and allow you
to import them one by one, generating a new commit for every new
release.


Authors and Maintainers
-----------------------

The master site for this and related repos is the [Macroassembler-AS
organization on GitHub][ghmas]. Issues and PRs for this repo should be
filed in the [`Macroassembler-AS/asl-releases`][ghmasrel] project
there.

The original import code was written by [Kuba Ober][KubaO],
<kuba@mareimbrium.org>. [Curt J. Sampson][0cjs] contributed this
documentation and is currently doing regular imports of new ASL
versions.



<!-------------------------------------------------------------------->
[asl]: http://john.ccac.rwth-aachen.de:8000/as/
[src]: http://john.ccac.rwth-aachen.de:8000/ftp/as/source/c_version/

[ghmas]: https://github.com/Macroassembler-AS
[ghmasrel]: https://github.com/Macroassembler-AS/asl-releases
[KubaO]: https://github.com/KubaO
[0cjs]: https://github.com/0cjs
[8bitdev]: https://github.com/0cjs/8bitdev
