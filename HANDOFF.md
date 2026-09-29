# altWinDirStat: Handoff Notes

> Background for whoever works on this repo next (human or Claude). Check claims against the code and `git log` before relying on them.

## What this repo is now

* **Since the WinDirStat 2.x adoption, altWinDirStat is an unofficial fork of the official [WinDirStat](https://github.com/windirstat/windirstat) 2.x.**
* **Why it changed:** the legacy altWinDirStat (2014–2016 MFC fork, releases up to `v0.1.0`) scanned on the UI thread. It froze on a laptop's C: drive and couldn't handle multi-terabyte servers. Official 2.x already has what we needed:
  * multithreaded scanning and direct NTFS MFT reading
  * suspend, resume and stop
  * duplicates, cleanups and search
* **How the history was joined:** `git merge -s ours --allow-unrelated-histories` joined the two histories. The files are upstream's; the old code is still reachable at tag `v0.1.0`.

## altWinDirStat changes on top of upstream (keep this list current)

Keep this diff small, because every upstream sync has to merge through it.

| File | Change | Why |
|---|---|---|
| `windirstat/Constants.h` | `strWinDirStat` = `altWinDirStat`; `strUninstall` → `...\Uninstall\altWinDirStat` | Drives titles, the Explorer context-menu key, the exe-name match and the uninstall key |
| `windirstat/Version.h` | Product name, description, exe name, repository, company/copyright | Version resource shows altWinDirStat; the copyright still credits the WinDirStat Team |
| `windirstat/Version.h` (block at the end) | `ALT_VER_*` (default 2.0.0) replace upstream's `PRD_MAJVER/MINVER/PATCH` via `#undef` | The fork has its own version number. Kept at the end of the file so upstream version bumps merge without conflicts. **Bump the defaults after each release** |
| `windirstat/Property.cpp` | Registry root `Software\altWinDirStat\altWinDirStat\` | Settings don't collide with official WinDirStat |
| `windirstat/WinDirStat.cpp` | "Reset preferences" deletes `Software\altWinDirStat` | Matches the key above |
| `windirstat/Localization.cpp/.h` | `ApplyForkBranding()` rewrites "WinDirStat" → "altWinDirStat" in loaded strings. It keeps "WinDirStat Team" and adds a fork notice to the About text | Rebrands the UI without editing 25 `lang_*.txt` files |
| `windirstat/windirstat.rc` | `IDS_URL_REPORT_BUG` → `github.com/Demonad112/altWinDirStat/issues/new` | Fork bug reports shouldn't land in upstream's tracker. Help → manual still opens upstream's wiki (same features) |
| `windirstat/windirstat.vcxproj` | `TargetName` = `altWinDirStat_<arch>` | Output exe name; the portable INI follows it (`altWinDirStat.ini`) |
| Removed: `.github/workflows/publish-*-to-winget-pkgs.yml`, `.github/FUNDING.yml`, `setup/chocolatey/`, `setup/store/` | | These publish under the official identity. **If a sync re-adds them, delete them again.** CI's `fork-guard` job fails when any of them is present, and it also blocks releases |
| Added: `.github/workflows/build.yml`, `.github/workflows/sync-upstream.yml`, `installer/altWinDirStat.iss`, `README.md`, `HANDOFF.md`, `CLAUDE.md`, `docs/releases/` | | Our CI, sync, installer, docs and release notes |
| `windirstat/windirstat.rc` | `#include "ForkResource.h"` plus two File-menu items (`ID_FOLDER_LEDGER_EXPORT`, `ID_FOLDER_LEDGER_COMPARE`) | Folder ledger menu entries |
| `windirstat/WinDirStatModel.h` | `#include "ForkResource.h"`, three method declarations and three routes | Folder ledger commands; the bodies live in `ForkCommands.cpp` |
| `windirstat/CsvLoader.cpp` | One-line hook at the top of `SaveResults`: `*.ledger.csv` targets write a folder ledger | Save Results and `/saveto` can produce ledgers |
| `windirstat/windirstat.vcxproj` | One separate `ItemGroup` listing the fork sources below | Keeps upstream's item lists untouched |
| Added: `ForkResource.h`, `Fork.rc`, `Ledger.cpp/.h`, `ForkCommands.cpp`, `Dialogs/LedgerCompareDlg.cpp/.h`, `res/fork/lang_en.txt` | | Folder ledger export and side-by-side comparison. IDs use ranges upstream doesn't (900, 1900-1906, 33900-33901). Strings live in the fork's own `lang_en.txt`, which `Compress Strings.ps1` picks up recursively. That keeps the 25 upstream language files and their key-parity test untouched, and other languages fall back to English |
| Added: `project.early.props` | | The vcxproj already imports it if present. When MSBuild gets `/p:AltVerMajor/AltVerMinor/AltVerPatch`, it defines `ALT_VER_*` for both `cl` and `rc` (`ExternalCompilerOptions` reaches only `cl`, so it can't stamp FileVersion) |

`setup/msi` (upstream's WiX MSI) is left untouched and unused.

## Build

* **Visual Studio 2026:** build `windirstat.sln` as is.
* **Visual Studio 2022:** add `/p:PlatformToolset=v143`. It builds cleanly (verified locally and in CI):
  ```
  msbuild windirstat.sln /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
  ```
* **Output:** `build\altWinDirStat_{x64|x86|arm64}.exe`.
* **Pre-build steps:** they run PowerShell scripts (`windirstat\Build\*.ps1`) that format the sources and compress the language strings into `res\lang_combined.bin`.

## CI (`.github/workflows/build.yml`)

* **Build matrix:** x64, Win32 and ARM64 on `windows-2022`. ARM64 is build-only.
* **Tests (x64 and Win32):**
  * Smoke/GUI scan of the repo and `C:\Program Files`. It fails if the window stops responding for more than 3 seconds, or if the title isn't "altWinDirStat".
  * Upstream's `tests/Test-WinDirStat.ps1 -Only Filtering,Cli,EdgeCases,Permissions,Enumeration,Unc,Ui`. Settings is left out because it rebuilds the solution without `PlatformToolset=v143`, which the VS2022 runner needs. Mtp and Reparse are left out because they need a phone or scratch drives. It needs PowerShell 7.6+, which CI downloads if the runner is older.
  * Inno installer silent install + uninstall.
* **Large-scan stress test** (`stress` job, `.github/scripts/Stress-LargeScan.ps1`, x64):
  * **Headless export:** generates a ~700k-file tree (a folder grid, a flat folder with 100k files, and a path deeper than MAX_PATH). A headless `/saveto` export must list every file, and the root's file count and byte total must match exactly.
  * **GUI scans:** it then scans that tree (generic multithreaded finder) and all of `C:\` (MFT finder, since runners are elevated). Throughout each scan and for 15 s after it, it probes the window every 250 ms with `SendMessageTimeout(WM_NULL)`, and it resizes the window after each scan to force a treemap re-layout.
  * **Fails on:** any probe of 5 s or more, which is Windows' "Not Responding" threshold. Scan times, worst and p99 UI latency, and peak memory are written to the job summary.
  * **Releases depend on it.**
* **Signing:** optional, via repository secrets.
  * **Azure Trusted Signing:** `AZURE_TENANT_ID`, `AZURE_CLIENT_ID`, `AZURE_CLIENT_SECRET`, `SIGNING_ENDPOINT` (e.g. `https://eus.codesigning.azure.net/`), `SIGNING_ACCOUNT`, `SIGNING_PROFILE`.
  * **Or a .pfx:** `SIGNING_PFX_BASE64` + `SIGNING_PFX_PASSWORD`.
  * **Without either:** builds are unsigned and a notice appears on the run. **Smart App Control blocks unsigned builds**, which is why the official signed WinDirStat ran on the owner's laptop but our builds didn't.
* **Versioning:**
  * Plain builds report the `ALT_VER_*` default from `Version.h` and are named `<ver>-dev.<run>`.
  * Release builds stamp the tag's version into the exe.
  * The "Collect binary" step fails if the exe's FileVersion doesn't match.
  * A tag with a `-suffix` (e.g. `v2.1.0-rc.1`) is a prerelease: "Beta" stays in the title, and it isn't marked as the latest release.
* **Release**, either way:
  * **Actions → Build → Run workflow** with `release_tag` = `vX.Y.Z`. The release job creates the tag on the commit that was built, and refuses if the tag already exists.
  * Or `git tag vX.Y.Z && git push origin vX.Y.Z`.
  * The release gets the installers, the portable zips and `SHA256SUMS.txt`. Its body is `docs/releases/<tag>.md` if that file exists; otherwise GitHub generates the notes.

## Syncing with upstream (`.github/workflows/sync-upstream.yml`)

* **What it does:**
  * Runs Mondays and on demand.
  * Merges `windirstat/windirstat` master into `sync/upstream-<date>` and opens a PR.
  * Starts the Build workflow on that branch.
* **Conflicts:** the conflict markers are committed so they can be resolved on the PR branch.
* **Repo setting needed:** Settings → Actions → General → **"Allow GitHub Actions to create and approve pull requests"** must be on, or the PR step fails. The branch is still pushed, so you can open the PR by hand.
* **Manual sync:**
  ```
  git remote add wds https://github.com/windirstat/windirstat.git
  git fetch wds && git merge wds/master
  ```

## Settings carried over from legacy 1.x

Legacy 1.x and 2.x both use `HKCU\Software\altWinDirStat\altWinDirStat\<section>`. Registry key names are case-insensitive, so legacy `options` is 2.x's `Options`.

* **Values 2.x picks up:** only `Options\ListStripes` and `Options\ListFullRowSelection`. Both are bool DWORDs with the same meaning, so they carry over correctly.
* **Everything else from legacy is never read.** That includes the `persistence` section and the treemap values, which 2.x keeps in other sections. These values are harmless leftovers, and **Reset preferences** removes them.
* **No crash risk from mismatched values:** `Property.cpp` reads with strict types (`QueryDWORDValue`, and `REG_SZ` only for strings) and falls back to the default on a mismatch.

## Maintenance

* **Deleting a prerelease:** go to **Actions → Maintenance → Run workflow** with `delete_prerelease=<tag>`.
  * It deletes a prerelease and its git tag.
  * It refuses final releases.
  * It exists because cloud Claude sessions can't delete tags through their git proxy.
* **Old branches:** `Problem-commit`, `attempt-removal-of-ConcRT-deps` and `fix-blank-header-ctrl` are 2014–2016 legacy branches, and `claude/no-command-line` is a legacy-codebase feature branch. None is merged into master. They're kept for history; delete them only on purpose.

## Known gaps / next steps

1. **Code signing.** Without a certificate, SAC-enabled PCs can't run our builds.
2. **Help → manual** still opens upstream's wiki. That's accurate for the features. Help → Report Bug already opens this repo's issues.
3. **Folder ledger follow-ups:**
   * translations of the `IDS_LEDGER_*` strings
   * a CLI compare mode (e.g. `/compareledger base.ledger.csv`) for scheduled change reports
   * a UI-automation test for the compare dialog
4. Possible fork-specific addition: a simpler "beginner" mode, if there's demand. Everything else comes from upstream.
