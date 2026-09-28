# altWinDirStat

> **Unofficial fork of [WinDirStat](https://github.com/windirstat/windirstat) 2.x.** It is not affiliated with or endorsed by the WinDirStat team. For the official, signed app, use the [official WinDirStat releases](https://github.com/windirstat/windirstat/releases) or [windirstat.net](https://windirstat.net/).

altWinDirStat is a disk usage analyzer and cleanup assistant for Windows, built on the official WinDirStat 2.x code. It shows what is filling a drive with a sortable folder tree, a file-type breakdown and an interactive treemap, where bigger files take up bigger areas. It stays responsive on very large volumes and servers.

It installs **side by side** with official WinDirStat. It uses its own program name, settings (`HKCU\Software\altWinDirStat`), portable INI (`altWinDirStat.ini`) and Explorer right-click entry, so the two never overwrite each other.

## Features (from WinDirStat 2.x)

* Fast scanning:
  * direct NTFS MFT reading when run as administrator
  * multithreaded folder walking otherwise
  * refresh, suspend, resume and stop
* All Files, Largest Files, Duplicate Files, Search Results, File Watcher, Extension and Treemap views
* Duplicate detection by hash, search with filters and regular expressions
* Built-in actions:
  * open, copy path, show in Explorer, Command Prompt/PowerShell here
  * move, delete, empty the Recycle Bin
* Cleanup and maintenance shortcuts:
  * Disk Cleanup, DISM, shadow copies, CHKDSK and more
  * user-defined cleanups
* Dark mode, portable mode, localization, high-DPI support

## altWinDirStat additions

### Folder ledger and baseline comparison

A small snapshot of a scan with one row per folder and no per-file rows. You can save one as a baseline and later compare the same folders to see what changed.

* **File → Export Folder Ledger...** saves the finished scan as a `.ledger.csv` file.
  * It lists every folder with its total size, file count and subfolder count.
  * The file opens in Excel.
* **File → Compare with Folder Ledger...** loads a baseline ledger and compares it with the scan that's open. If no scan is open, it asks for a second ledger instead. The side-by-side view shows:
  * the baseline and current size and file count for each folder, and the change in each
  * each folder color-coded as **Added**, **Removed**, **Grown**, **Shrunk** or **Files changed**
  * a summary line with the counts and the net size and file change
  * a text filter
  * a **Show unchanged folders** option
  * sorting by any column
  * **Export Comparison...**, which saves the rows shown to CSV
  * double-click, which opens a folder in Explorer
* **Matching:** folders are matched by their path relative to the scan root, ignoring case.
  * A baseline of `C:\Data` compared against a scan of all of `C:` is narrowed to `C:\Data` automatically.
  * A copy on another drive, e.g. `D:\Data` against `E:\Backup\Data`, is matched folder by folder.
* **Automation:** any Save Results or `/saveto` target ending in `.ledger.csv` writes a ledger. A scheduled task can therefore record baselines:

  ```
  altWinDirStat.exe /saveto "D:\Baselines\data-2026-09-28.ledger.csv" "D:\Data"
  ```

## Download

Get the latest build from [Releases](https://github.com/Demonad112/altWinDirStat/releases):

| File | Use |
|---|---|
| `altWinDirStat-<ver>-x64-Setup.exe` | Installer for most PCs and servers. No admin needed for a per-user install. It can add **"Analyze with altWinDirStat"** to the Explorer right-click menu. |
| `altWinDirStat-<ver>-x64-portable.zip` | A single `altWinDirStat.exe`. Unzip it anywhere (a USB stick works) and run it. |
| `…-ARM64-…` / `…-Win32-…` | The same builds for ARM64 and 32-bit Windows. |
| `SHA256SUMS.txt` | SHA-256 checksums of every file in the release. |

**Code signing:** release builds are signed only when the repository has a signing certificate configured.

Unsigned builds:

* show a SmartScreen warning: click **More info → Run anyway**
* are **blocked by Smart App Control** on Windows 11 PCs where it's turned on

If you need a signed build on such a PC, use the official WinDirStat.

## Building

* **Requirements:**
  * Visual Studio 2022 or later with the C++ desktop workload, ATL, and (for ARM64) the ARM64 build tools
  * PowerShell for the pre-build steps
* **With Visual Studio 2026:** open `windirstat.sln` and build.
* **With Visual Studio 2022:** override the toolset:

```
msbuild windirstat.sln /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
```

The output is `build\altWinDirStat_x64.exe` (or `_x86` / `_arm64`).

CI (`.github/workflows/build.yml`) builds all three architectures, then:

* runs the smoke tests and upstream's headless test suites
* builds the installers
* publishes a GitHub Release (with `SHA256SUMS.txt`) on `v*` tags, or when run by hand with a `release_tag` input

## Staying in sync with WinDirStat

The **Sync with official WinDirStat** workflow runs every Monday, and you can also run it on demand. It merges the latest official WinDirStat into a `sync/upstream-<date>` branch, opens a pull request and runs the build on it. Merge the pull request when it's green.

The altWinDirStat changes are kept small so these merges rarely conflict. `HANDOFF.md` lists them.

## Credits and licenses

* **Based on WinDirStat.** Copyright © WinDirStat Team ([windirstat.net](https://windirstat.net/)). The original WinDirStat was written by Bernhard Seifert and Oliver Schneider. WinDirStat 2.x is maintained by Bryan Berns and contributors; see [CONTRIBUTORS.md](CONTRIBUTORS.md).
* **License:** distributed under the terms of the [GPL v2](windirstat/res/license.txt), like WinDirStat. You may not upgrade the GPL version to anything later than v2.
* **Logo:** the logo and its derivatives are available under the Creative Commons license [CC BY 3.0](https://creativecommons.org/licenses/by/3.0/) (WinDirStat Team).
* **Legacy code:** the pre-2.x altWinDirStat codebase (ariccio's 2014–2016 fork, last release `v0.1.0`) remains in the git history. Check out tag `v0.1.0` to see it.

## Compatibility

The same as official WinDirStat 2.x. Its README lists Windows 7 through 11 and Windows Server 2008 R2 through 2025. altWinDirStat's CI tests on Windows Server 2022.
