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
| `windirstat/Property.cpp` | Registry root `Software\altWinDirStat\altWinDirStat\` | Settings don't collide with official WinDirStat |
| `windirstat/WinDirStat.cpp` | "Reset preferences" deletes `Software\altWinDirStat` | Matches the key above |
| `windirstat/Localization.cpp/.h` | `ApplyForkBranding()` rewrites "WinDirStat" → "altWinDirStat" in loaded strings. It keeps "WinDirStat Team" and adds a fork notice to the About text | Rebrands the UI without editing 25 `lang_*.txt` files |
| `windirstat/windirstat.vcxproj` | `TargetName` = `altWinDirStat_<arch>` | Output exe name; the portable INI follows it (`altWinDirStat.ini`) |
| Removed: `.github/workflows/publish-*-to-winget-pkgs.yml`, `.github/FUNDING.yml`, `setup/chocolatey/`, `setup/store/` | | These publish under the official identity. **If a sync re-adds them, delete them again** |
| Added: `.github/workflows/build.yml`, `.github/workflows/sync-upstream.yml`, `installer/altWinDirStat.iss`, `README.md`, `HANDOFF.md` | | Our CI, sync and installer; fork README |

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
  * Upstream's `tests/Test-WinDirStat.ps1 -Only Filtering,Cli,EdgeCases`. It needs PowerShell 7.6+, which CI downloads if the runner is older.
  * Inno installer silent install + uninstall.
* **Signing:** optional, via repository secrets.
  * **Azure Trusted Signing:** `AZURE_TENANT_ID`, `AZURE_CLIENT_ID`, `AZURE_CLIENT_SECRET`, `SIGNING_ENDPOINT` (e.g. `https://eus.codesigning.azure.net/`), `SIGNING_ACCOUNT`, `SIGNING_PROFILE`.
  * **Or a .pfx:** `SIGNING_PFX_BASE64` + `SIGNING_PFX_PASSWORD`.
  * **Without either:** builds are unsigned and a notice appears on the run. **Smart App Control blocks unsigned builds**, which is why the official signed WinDirStat ran on the owner's laptop but our builds didn't.
* **Release:** `git tag vX.Y.Z && git push origin vX.Y.Z` publishes the installers and portable zips.

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

## Known gaps / next steps

1. **Code signing.** Without a certificate, SAC-enabled PCs can't run our builds.
2. The **Help → manual** and website links still point to windirstat.net (upstream docs). That's accurate for the features, but it's upstream's site.
3. Possible fork-specific addition: a simpler "beginner" mode, if there's demand. Everything else comes from upstream.
