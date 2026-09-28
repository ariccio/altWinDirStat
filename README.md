altWinDirStat

## Download & install

**Latest release: [v1.0.0](https://github.com/Demonad112/altWinDirStat/releases/latest).** Every push to `master` and every pull request is also built on Windows by GitHub Actions (**Actions → Build → latest run → Artifacts**).

| File | Use |
|---|---|
| `altWinDirStat-<ver>-x64-Setup.exe` | Installer (recommended). Adds a Start Menu shortcut and uninstaller. Optionally adds a desktop icon and an Explorer right-click **"Analyze with altWinDirStat"** entry on folders and drives. No admin needed for a per-user install. |
| `altWinDirStat-<ver>-x64-portable.zip` | Single standalone `.exe` (static MFC/CRT). Unzip and run. |
| `...-Win32-...` | Same builds for 32-bit Windows. |

**Usage**
* Launch it with no arguments to get the drive/folder picker.
* `altWinDirStat.exe "D:\"` or `altWinDirStat.exe "C:\Users"` scans that path directly. This is what the context-menu entry calls.
* Right-click any item in the tree or treemap for these actions:
  * **Copy Path** (Ctrl+C)
  * **Explorer Here** (Ctrl+E): opens a folder, or opens a file's parent folder with the file selected.
  * **Command Prompt Here** (Ctrl+P)
  * **Delete to Recycle Bin** (Del)
  * **Permanent delete** (Shift+Del), after an extra confirmation.
  * **Refresh** (F5, also in the File menu): rescans the current folder or drive.

  Delete is available only after the scan finishes. It then rescans the same folder so the sizes stay accurate.
  If an item can't go to the Recycle Bin (too large, or no Bin on that drive), Windows asks before deleting it permanently.
* Settings persist in `HKCU\Software\altWinDirStat\altWinDirStat`. Settings from older versions (`HKCU\Software\Seifert\altWinDirStat`) are copied over once on first launch; the old key is left in place.

**Build locally**
1. Install Visual Studio 2022 with the *Desktop development with C++* workload and the *C++ MFC for latest v143 build tools* component.
2. `nuget restore WinDirStat\windirstat\packages.config -PackagesDirectory WinDirStat\packages`
3. `msbuild WinDirStat\windirstat\windirstat.vcxproj /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143`
4. Optional: `ISCC installer\altWinDirStat.iss /DSourceDir=<folder containing altWinDirStat.exe, LICENSE.txt, gpl-2.0.txt, README.md>`

To cut a release, either:
* run `git tag vX.Y.Z && git push origin vX.Y.Z`, or
* go to **Actions → Build → Run workflow** and set `release_tag` to `vX.Y.Z`.

Put release notes in `docs/releases/vX.Y.Z.md`. Help → About and the exe's file properties show the stamped version.

=============

An unofficial modification of WinDirStat. Tremendous performance improvements.

This repository used to be an ugly, hacky, bundle of crap - but now it's just a bundle of crap. 

**I've been working on some interesting static analysis stuff lately, so development has slowed down :(**

In the mean time, I'm planning to cut out some of the MFC code (replaced with the raw Windows API that MFC poorly wraps)
  ...and then switch over to VS 2015.

A quick guide to the structure of this repository:

* Reference Code
  * Code that I referenced/studied early in development
  * None of it compiles as part of altWinDirStat
* WinDirStat
  * My branch, this contains the Visual Studio 2013 `.sln` file
  * *This is where the source code is!*
  * Has it's own, more detailed `README.md`
* Development Screenshots
  * Interesting things I saw while working on altWinDirStat
* filesystem-docs-n-stuff
  * All sorts of information on NTFS and NTFS internals
  * LOADS of good stuff in here!
  * Also has mirrors of any documentation that I mention in the source code
* stress-progs
  * A native application that I've built to stress test WinDirStat by creating an arbitrary number of randomly named files
  * Has it's own `.sln` file, and is developed concurrently (albeit sporadically)
* stress-scripts
  * A naive version of the aforementioned stress testing utility, written in Python
  * Turned out to be extremely slow, caused by a massive text-encoding bottleneck in Python
* *(many other files, not yet sorted)*
