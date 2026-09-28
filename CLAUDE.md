# altWinDirStat: notes for Claude Code

A native Windows disk-usage viewer (a fork of WinDirStat), written in C++ with MFC and WTL. It builds to a single statically linked `altWinDirStat.exe`, packaged as an Inno Setup installer and a portable zip. **For architecture, change history, known issues and the backlog, read `HANDOFF.md`.** Treat it as background and check its claims against the code.

## Build (Windows, Developer PowerShell for VS 2022)
Requires VS 2022 with the "Desktop development with C++" workload and the MFC + ATL v143 components.
```powershell
msbuild WinDirStat\windirstat\windirstat.vcxproj /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /p:RunCodeAnalysis=false
# -> WinDirStat\x64\release\windirstat.exe   (Win32: WinDirStat\windirstat\release\windirstat.exe)
```
- Installer: `ISCC installer\altWinDirStat.iss /DAppVersion=X.Y.Z /DArch=x64 /DSourceDir=<dir with altWinDirStat.exe, LICENSE.txt, gpl-2.0.txt, README.md>`
- Reset settings for a first-run test: `reg delete HKCU\Software\altWinDirStat /f`
- The Intel_* configurations in `windirstat.sln` are dead. Use Release|x64 or Release|Win32.

## Where things live
| Path | What |
|---|---|
| `WinDirStat/windirstat/windirstat.cpp` | App startup, command line, `OnIdle` (drives the scan), About |
| `WinDirStat/windirstat/dirstatdoc.cpp` | Document: tree root, selection, Copy Path, Delete, Explorer Here, Cmd Here, Refresh |
| `WinDirStat/windirstat/directory_enumeration.cpp` | Scanner (`FindFirstFileExW` + `GetCompressedFileSizeW`, ConcRT workers) |
| `WinDirStat/windirstat/ChildrenHeapManager.h` | Packed child and name storage, so the tree can't be edited in place and Delete rescans |
| `WinDirStat/windirstat/options.cpp` | Registry settings (`HKCU\Software\altWinDirStat`) |
| `WinDirStat/windirstat/windirstat.rc`, `resource.h` | Menus, accelerators, command IDs |
| `WinDirStat/common/build_version.h` | Version stamp. CI overwrites it from the tag; the committed copy says 0.0.0-dev |
| `installer/altWinDirStat.iss` | Installer |
| `.github/workflows/build.yml` | CI + releases |

## Conventions
- Match the existing style: Ratliff indentation, SAL annotations, `TRACE` for diagnostics.
- Don't add `std::terminate()` for conditions the environment can cause (files, permissions, registry, user input). Handle them.
- Don't edit `Reference code/`, `filesystem-docs-n-stuff/` or the vendored `WinDirStat/packages/wtl.*`. The one exception is `build/native/wtl.targets`, which must stay committed because the build fails without it.
- Don't commit a CI-stamped `build_version.h`.

## CI and releases
- Every PR and every push to `master` builds x64 and Win32 on `windows-2022`, then:
  - smoke-tests the exe
  - runs a GUI scan and takes a screenshot
  - runs the rescan and settings-migration test
  - checks the version stamp
  - builds the installer and tests a silent install
- If a GUI test crashes, the job uploads a WER dump and PDB as `crash-<arch>`.
- Release, option 1: `git tag vX.Y.Z && git push origin vX.Y.Z`.
- Release, option 2: GitHub → Actions → Build → **Run workflow** with `release_tag = vX.Y.Z`.
- Either way, the Release gets the installers, portable zips, screenshots and `SHA256SUMS.txt`. If `docs/releases/vX.Y.Z.md` exists, it becomes the release body.
