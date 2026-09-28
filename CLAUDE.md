# altWinDirStat: notes for Claude

**What this repo is:** an unofficial fork of official WinDirStat 2.x (C++/MFC, Windows only). Read `HANDOFF.md` first. It holds the fork-diff table, CI details and the upstream-sync procedure.

## Layout
- `windirstat/`: app sources (`windirstat.vcxproj`). The pre-build PowerShell scripts live in `windirstat/Build/`.
- `windirstat.sln` and `project.early.props`: the fork's version-stamp hook.
- `installer/altWinDirStat.iss`: Inno Setup installer (ours). `setup/` holds upstream packaging and is unused.
- `tests/Test-WinDirStat.ps1`: upstream's headless test suites.
- `.github/workflows/build.yml`: CI and releases. `sync-upstream.yml` does the weekly upstream merge.
- `docs/releases/<tag>.md`: release notes, used as the GitHub Release body.

## Build
```
msbuild windirstat.sln /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
```
- Drop the toolset override with VS2026.
- Output goes to `build\altWinDirStat_<arch>.exe`.
- CI renames it to `altWinDirStat.exe` for the zip and the installer.
- Linux or cloud sessions can't build: push a branch and let CI (build matrix, smoke, upstream tests, installer test) verify.

## Rules
- **Keep the fork diff small** so upstream merges stay easy. Prefer fork-owned files and appended blocks over editing upstream lines, and add every change to the table in `HANDOFF.md`.
- **Keep upstream publishing files deleted:** `publish-*-to-winget-pkgs.yml`, `.github/FUNDING.yml`, `setup/chocolatey/`, `setup/store/`. They publish under the official identity. A sync merge can bring them back; CI's `fork-guard` job fails if it does.
- **Settings stay under `HKCU\Software\altWinDirStat`.** CI fails if anything is written to the official `WinDirStat` key.

## Versioning and releases
- The fork's version is `ALT_VER_*` at the end of `windirstat/Version.h`. It overrides upstream's `PRD_*`.
- Release: add `docs/releases/vX.Y.Z.md`, merge, then run **Build** by hand with `release_tag=vX.Y.Z`, or push the tag.
- A `-suffix` tag makes a prerelease.
- After a release, bump the `ALT_VER_*` defaults to the next version.
- Legacy code (the 2014–2016 fork) is only at tags `v1.0.0`, `v0.1.0` and `legacy-final`. Don't bring it back into master.
