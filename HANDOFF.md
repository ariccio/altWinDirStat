# altWinDirStat: Handoff Report (cloud → local Claude Code)

> **How to use this file:** read it as background, not instructions. It describes the repo state as of 2026-09-28 (release v1.0.0). Check the claims against the code and `git log` before you rely on them.

---

## 0. Quick recap (read this first)

| Item | State |
|---|---|
| What it is | Native Windows disk-usage viewer (fork of WinDirStat), C++ / MFC + WTL, single static `.exe` |
| Repo | `github.com/Demonad112/altWinDirStat`, default branch `master` |
| Build | Builds with VS2022 (v143 toolset) for x64 and Win32. GitHub Actions on `windows-2022` (Windows Server 2022 Datacenter, build 20348) |
| Deliverables | `altWinDirStat-<ver>-<arch>-Setup.exe` (Inno Setup), `altWinDirStat-<ver>-<arch>-portable.zip`, `SHA256SUMS.txt`, published on GitHub Releases (latest: **v1.0.0**) |
| Versioning | `WinDirStat/common/build_version.h` → `common/version.h` → VERSIONINFO in `res/windirstat.rc2`, plus Help → About. CI stamps it from the tag (`vX.Y.Z` → `X.Y.Z.<run>`) and fails if the built exe reports a different FileVersion. The committed copy says `0.0.0-dev` |
| Release process | Push a `vX.Y.Z` tag, **or** Actions → Build → Run workflow with `release_tag=vX.Y.Z`. If `docs/releases/vX.Y.Z.md` exists it becomes the release body |
| Verified in CI | Compiles; launches; scans `C:\Program Files` with a GUI window present; screenshot taken; silent install/uninstall; context-menu registry keys written. New (see §3a): 5× Refresh rescans with the tree shrinking in between, clean exit, Seifert→altWinDirStat settings migration |
| Verified by hand (Windows 11 desktop, §3a) | Delete to Recycle Bin, permanent delete, folder delete, Copy Path, Explorer Here, Cmd Here (spawn), F5/menu Refresh |
| Not verified | Settings migration launched locally (Smart App Control blocked the last local build; CI covers it), very large volumes, Server 2016/2019/2025 |
| Fixed in PR #1/#2 | (1) First-run crash (`std::terminate` on missing window placement). (2) Command-line path scan reopening the picker. (3) Delete/Explorer Here/Command Prompt Here were dead menu items |
| Fixed in the local session (§3a) | Crash right after every Delete (use-after-free on rescan); treemap `terminate` during rescan; Copy Path clipboard bugs; terminate audit; settings key moved; Refresh (F5) added |

**Suggested next steps:**
1. Code-sign the exe and installer. Unsigned local builds are also what Smart App Control blocks.
2. Reselect or expand the deleted item's parent after a Delete/Refresh rescan. Right now the tree collapses back to the root.
3. Move Delete from `SHFileOperationW` to `IFileOperation` to support paths longer than MAX_PATH.
4. Graceful out-of-memory handling in `ChildrenHeapManager.h`. Every caller would need to handle a null child block.

---

## 1. Repository map

```
.github/workflows/build.yml     CI: build → smoke test → GUI scan+screenshot → installer → install test → artifacts → Release on v* tag
installer/altWinDirStat.iss     Inno Setup 6 script (per-user or all-users, Start Menu, desktop icon, Explorer context menu)
WinDirStat/windirstat.sln       VS solution (has dead Intel_* configs; CI builds the .vcxproj directly)
WinDirStat/windirstat/          ALL source code
  windirstat.vcxproj            Project. Release|x64 and Release|Win32 are the only configs that matter
  windirstat.cpp                CDirstatApp: InitInstance, command-line handling, OnIdle (drives scanning)
  dirstatdoc.cpp/.h             CDirstatDoc: the tree root, selection, Copy Path, and the new Delete/Explorer/Cmd handlers
  directory_enumeration.cpp     Scanner: FindFirstFileExW(FindExInfoBasic) + GetCompressedFileSizeW
  TreeListControl.cpp/.h        Tree list view + CTreeListItem (node type)
  ChildrenHeapManager.cpp/.h    Packed allocation: children array + name string pool in one block
  graphview.cpp / treemap.cpp   Treemap view + rendering
  typeview.cpp                  Extension list view
  options.cpp                   Registry-backed settings (CPersistence, COptions)
  windirstat.rc / resource.h    Menus, accelerators, dialogs, command IDs
WinDirStat/packages/wtl.10.0.10320/   Vendored WTL (NuGet layout). build/native/wtl.targets MUST stay committed
Reference code/, filesystem-docs-n-stuff/, developmentScreenshots/   Upstream author's notes/reference, not built
```

---

## 2. How it works

1. **Startup** (`CDirstatApp::InitInstance`, `windirstat.cpp`):
   - Loads the options from the registry.
   - Creates the single-document template (`CDirstatDoc`, `CMainFrame`, `CGraphView`).
   - Runs `ProcessShellCommand`:
     - With no arguments, it routes to `ID_FILE_NEW`, which runs `OnFileOpenLight` and shows a folder picker (WTL `CFolderDialog`).
     - With a path argument, it routes to `OpenDocumentFile(path)`, which calls `CDirstatDoc::OnOpenDocument` and starts the scan.
2. **Scan:**
   - `CDirstatDoc::OnOpenDocument` calls `buildDriveItems`, which creates the root.
   - Scanning then happens incrementally from `CDirstatApp::OnIdle`, which calls `CDirstatDoc::Work()`.
   - Enumeration uses `FindFirstFileExW(..., FindExInfoBasic, ...)`, with `GetCompressedFileSizeW` for size on disk.
   - It needs no admin rights and does not read the MFT directly. Folders you have no access to are skipped.
3. **Data model:**
   - Each folder's children live in one `child_info` block: a `CTreeListItem[]` array plus a string pool for the names (`ChildrenHeapManager.h`).
   - Because of this packing, the tree **can't be edited in place**. Delete therefore rescans the root afterwards.
4. **Views:**
   - Tree list (`CDirstatView`/`CTreeListControl`).
   - Treemap (`CGraphView` + `CTreemap`), toggled with F9.
   - Extension list (`CTypeView`), toggled with F8.
5. **Settings:**
   - Stored under `HKCU\Software\altWinDirStat\altWinDirStat\...`: MFC `SetRegistryKey(L"altWinDirStat")` in `InitInstance`, plus the app title `altWinDirStat` (`AFX_IDS_APP_TITLE`) as the profile name.
   - Before §3a the key was `HKCU\Software\Seifert\altWinDirStat`. It was never `...\Seifert\windirstat`, as earlier versions of this file claimed. `migrate_legacy_settings_key` (`windirstat.cpp`) copies it across once.
6. **Rescans (Delete / Refresh):**
   - `CDirstatDoc::RescanRoot` calls `OpenDocumentFile` on the same root, which leads to `OnOpenDocument` and then `DeleteContents`.
   - `DeleteContents` must tell the views (`HINT_NEWROOT`) **before** it frees the tree, because the views hold raw `CTreeListItem*`.
   - Scanning runs from `OnIdle`, so an open menu or any other modal loop pauses a rescan until it closes. That's expected, not a hang.

---

## 3. Changes made this session

### Merged in PR #1
- **CI** (`.github/workflows/build.yml`):
  - Runs `nuget restore`.
  - Builds with `msbuild windirstat.vcxproj /p:Configuration=Release /p:Platform=<x64|Win32> /p:PlatformToolset=v143 /p:RunCodeAnalysis=false /p:EnablePREfast=false`.
  - Collects the exe and zips it with the license and README.
  - Builds the installer with `ISCC /DAppVersion /DArch /DSourceDir`.
  - Silent-installs with `/CURRENTUSER /TASKS=contextmenu`, checks the files and the `HKCU\Software\Classes\Directory\shell\altWinDirStat\command` key, then uninstalls.
  - Uploads the artifacts. On `v*` tags, the `release` job publishes a GitHub Release.
- **Installer** (`installer/altWinDirStat.iss`):
  - `PrivilegesRequired=lowest` with a dialog override, so it installs per-user without admin, or for all users.
  - Uses `HKA` registry roots, so the context-menu keys go under HKCU or HKLM to match the install mode.
  - The context menu covers the `Directory\shell` and `Drive\shell` keys.
- **vcxproj:**
  - Removed `/await` (deprecated in VS2022, and the code doesn't use coroutines).
  - Removed the `/d1reportSingleClassLayout*`, `/Qvec-report`, `/Qpar-report` noise flags.
- **Committed** `WinDirStat/packages/wtl.10.0.10320/build/native/wtl.targets`, which had been gitignored.
  - The vcxproj hard-errors without this file.
  - `nuget restore` won't recreate it, because the package folder already exists.
- **windirstat.cpp:** removed the block that reopened the folder picker after a command-line path. It overwrote the requested scan.

### PR #2
- **Delete / Explorer Here / Command Prompt Here** (`dirstatdoc.cpp`, `dirstatdoc.h`, `resource.h`, `windirstat.rc`):
  - New command IDs:

    | ID | Value |
    |---|---|
    | `ID_CLEANUP_EXPLORER_HERE` | 32774 |
    | `ID_CLEANUP_CMD_HERE` | 32808 |
    | `ID_CLEANUP_DELETE_BIN` | 32809 |
    | `ID_CLEANUP_DELETE` | 32810 |

    These numbers match the literal IDs the `.rc` already used.
  - All four actions appear in both popups (`IDR_POPUPLIST`, `IDR_POPUPGRAPH`).
  - Accelerators: Ctrl+C / Ctrl+E / Ctrl+P / Del / Shift+Del.
  - Delete uses `SHFileOperationW`:
    - Recycle Bin mode uses `FOF_ALLOWUNDO` and shows the shell's own confirmation.
    - Permanent mode shows our own `MessageBox` warning first, then passes `FOF_NOCONFIRMATION`.
    - It's enabled only when the scan is done and the selection is not the root.
    - After a successful delete it calls `GetDocTemplate()->OpenDocumentFile(rootPath)` to rescan.
  - Explorer Here:
    - Folder: `explorer.exe "<path>"`.
    - File: `explorer.exe /select,"<path>"`.
  - Cmd Here runs `%COMSPEC%` with `lpDirectory` set to the folder (or to the file's parent).
- **Crash fix** (`options.cpp`, `CPersistence::GetMainWindowPlacement`):
  - It called `std::terminate()` when the registry had no saved main-window placement.
  - Result: the **first launch with a path argument on any fresh machine crashed** 1 s after start, with `0xC0000409 FAST_FAIL_FATAL_APP_EXIT`.
  - It now falls back to the default placement.
  - Found from a WER full-dump stack: `abort ← terminate ← CPersistence::GetMainWindowPlacement (options.cpp:246) ← CMainFrame::InitialShowWindow ← CDirstatApp::InitInstance`.
  - Note: running under `cdb` did *not* reproduce the crash; the WER dump did.
- **CI additions:**
  - GUI scan test: `altWinDirStat.exe "C:\Program Files"`, 25 s alive, `MainWindowHandle != 0`, full-screen PNG uploaded as `screenshot-<arch>`.
  - On failure it enables WER LocalDumps (full dump), analyses with `cdb -z`, and uploads the dump and PDB as `crash-<arch>`.

### 3a. Local session (branch `claude/continue-local-dev`)
- **Crash after every Delete** (`0xC0000409`), found by hand-testing:
  - Stack from a WER dump: `DeleteSelectedItem → OpenDocumentFile → OnOpenDocument → SetSplitterPos → RecalcLayout → CDirstatView::OnSize → RedrawWindow → COwnerDrawnListCtrl::DrawItem` on an already-freed `CTreeListItem`, then `_purecall`, then `abort`.
  - Cause: `DeleteContents` freed the tree while the list still pointed into it.
  - Fix: `DeleteContents` detaches the tree, sends `HINT_NEWROOT`, and only then frees it. `CDirstatView::OnUpdateHINT_NEWROOT` clears the list when there's no root.
  - `DeleteContents` also now clears the stale extension records and color map.
- **Second crash in the same flow:** `CGraphView::OnDraw`, `OnMouseMove` and `OnLButtonDown` called `std::terminate()` when the root wasn't done yet. During a rescan the treemap stays visible, so the next repaint killed the app. They now return early.
- **Copy Path:**
  - `GlobalAlloc(GMEM_MOVEABLE bitand GMEM_ZEROINIT)` evaluated to `GMEM_FIXED`. Now uses `bitor`.
  - The copied text was padded with MAX_PATH NUL characters. Removed.
- **Recycle-Bin delete** adds `FOF_WANTNUKEWARNING`. Also note: on Win10/11 with default settings, the shell shows **no** confirmation for a recycle-bin delete.
- **Refresh (F5)** `ID_CLEANUP_REFRESH` = 33030: F5, the File menu, and both popups. It shares `RescanRoot` with Delete.
- **Terminate audit:** converted the sites that files or input can trigger (`mountpoints.cpp` non-letter drive, `directory_enumeration.cpp` error-message formatting and the `$MFT` unmap). Invariants and OS-resource guards were deliberately kept. See commit `b0dfaf7` for the full list and the reasons.
- **Settings key** moved to `HKCU\Software\altWinDirStat`, with a one-time migration (§2.5).
- **CI:** new "Rescan (Refresh) + settings migration test" step. It posts `WM_COMMAND(33030)` 5× and asserts no hang, no crash event, and a migrated marker value.

---

## 4. Known issues / risks

1. **Remaining `std::terminate()` calls** (~120) guard internal invariants and GDI/window/timer resource exhaustion (`datastructures.cpp`, `globalhelpers.cpp`, `hwnd_funcs.cpp`, `ownerdrawnlistcontrol.h`, `TreeListControl.cpp`). They were deliberately left in place (see §3a).
2. **Delete rescans the whole root**, which is slow on large volumes. A proper fix needs in-place tree mutation, which is hard with the packed `child_info`. After the rescan, the tree collapses back to the root.
3. **No code signing.** SmartScreen warns on first run, and **Smart App Control** (Windows 11) may block locally built, unsigned exes outright. The Code Integrity log shows events 3077/3118. Don't work around it; test in CI or sign the binary.
4. **An empty scanned folder** shows the root with date 01/01/1601 and a black treemap. This is cosmetic.
5. **Server Core** has no Explorer shell, so this GUI (like any MFC app) won't be usable there. You need a server with Desktop Experience.
6. **Hundreds of compiler warnings** under v143 (C4365, C5039, …). They're harmless for now but worth a cleanup pass.
7. **The solution file** still lists the `Intel_*` configurations (Intel XE 14 toolset, not installed anywhere). Building the `.sln` in the IDE with those selected will fail. Use Release|x64.
8. **Long paths (>MAX_PATH):**
   - Delete, Explorer Here and Cmd Here strip `\\?\`.
   - `SHFileOperation` and `cmd.exe` may fail on very long paths. You'll see an error box; the app won't crash.

---

## 5. Local build setup (Windows)

1. Install **Visual Studio 2022** (Community is fine) with:
   - the **Desktop development with C++** workload
   - **C++ MFC for latest v143 build tools (x86 & x64)**
   - **C++ ATL for latest v143 build tools (x86 & x64)**
2. Optional: **Inno Setup 6** (`winget install JRSoftware.InnoSetup`) to build the installer.
3. Clone the repo and build (from a *Developer PowerShell for VS 2022*):
   ```powershell
   git clone https://github.com/Demonad112/altWinDirStat.git
   cd altWinDirStat
   nuget restore WinDirStat\windirstat\packages.config -PackagesDirectory WinDirStat\packages   # optional; the package is vendored
   msbuild WinDirStat\windirstat\windirstat.vcxproj /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /p:RunCodeAnalysis=false
   # output: WinDirStat\x64\release\windirstat.exe (x64). Win32: WinDirStat\windirstat\release\windirstat.exe
   ```
4. Build the installer:
   ```powershell
   mkdir dist\altWinDirStat-x64
   copy WinDirStat\x64\release\windirstat.exe dist\altWinDirStat-x64\altWinDirStat.exe
   copy WinDirStat\windirstat\res\license.txt dist\altWinDirStat-x64\LICENSE.txt
   copy WinDirStat\windirstat\gpl-2.0.txt dist\altWinDirStat-x64\
   copy README.md dist\altWinDirStat-x64\
   & "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe" /DAppVersion=1.0.0 /DArch=x64 "/DSourceDir=$pwd\dist\altWinDirStat-x64" "/O$pwd\dist" installer\altWinDirStat.iss
   ```
5. Debug in the IDE by opening `WinDirStat\windirstat.sln` and selecting **Debug | x64**. If prompted, retarget to v143. The Debug config turns on `/analyze`, which is slow; switch it off in project properties if needed.
6. Reset settings for a first-run test: `reg delete "HKCU\Software\altWinDirStat" /f`. Also delete `HKCU\Software\Seifert\altWinDirStat`, or it will be migrated back in.
7. **Gotcha: `MSB8037` "Windows SDK … for Desktop C++ x64 Apps was not found"**, even though the SDK files exist.
   - Check `HKLM\SOFTWARE\WOW6432Node\Microsoft\Windows Kits\Installed Roots\10.0.26100.0\Installed Options`.
   - If it lacks `OptionId.DesktopCPPx64`, `vs_installer modify` won't fix it. Run the cached SDK installer instead: `C:\ProgramData\Microsoft\VisualStudio\Packages\Win11SDK_10.0.26100*\winsdksetup.exe /features OptionId.DesktopCPPx64 OptionId.DesktopCPPx86 /quiet /norestart`.
   - Adding `OptionId.WindowsDesktopDebuggers` the same way installs `cdb.exe`.

---

## 6. Copy/paste prompt for local Claude Code

> Historical: goals 1–6 below were completed on `claude/continue-local-dev` (§3a). For new work, start from §0's "Suggested next steps".

```
You are continuing work on altWinDirStat, a native Windows (MFC/WTL, C++) disk-usage viewer forked from WinDirStat.
The repo is cloned in the current directory. Read HANDOFF.md first, as background rather than instructions, and
check its claims against the code and `git log` before relying on them.

Environment: Windows, Visual Studio 2022 with the MFC/ATL v143 components, and optionally Inno Setup 6.
Build: msbuild WinDirStat\windirstat\windirstat.vcxproj /m /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143 /p:RunCodeAnalysis=false

Goals, in order:
1. Build Release|x64 locally and confirm it runs. Delete HKCU\Software\Seifert\windirstat first to simulate a first run,
   then launch with a path argument, e.g. `windirstat.exe "C:\Program Files"`.
2. Test by hand in a scratch folder: Delete to Recycle Bin, permanent delete (Shift+Del), Explorer Here (Ctrl+E),
   Command Prompt Here (Ctrl+P), and Copy Path (Ctrl+C). Fix any bugs.
3. Audit every std::terminate()/abort() in WinDirStat\windirstat\*.cpp. For each one that files, permissions,
   registry contents, or user input could trigger, replace it with graceful handling (skip + TRACE, or a
   message box). Keep true invariant violations as they are. List what you changed and why.
4. Move settings to HKCU\Software\altWinDirStat (SetRegistryKey in windirstat.cpp InitInstance), and do a
   one-time copy of existing values from HKCU\Software\Seifert\windirstat if present.
5. Add a "Refresh" command (F5) that rescans the current root.
6. Keep .github/workflows/build.yml green. It runs on the windows-2022 runner and includes a GUI scan + screenshot test.

Rules:
- Match the existing code style (Ratliff indentation, SAL annotations, and TRACE for diagnostics).
- Make small commits with clear messages.
- Run the Release x64 build after each change. Don't claim something works until it has built and you've exercised it.
- Don't touch Reference code/, filesystem-docs-n-stuff/, or the vendored WTL package, except build/native/wtl.targets, which must stay committed.
```

---

## 7. Deeper reference

- **Releases:**
  - Two ways to trigger one: push a `vX.Y.Z` tag, or use `workflow_dispatch` with `release_tag`. The dispatch creates the tag at the selected commit if it doesn't exist.
  - The `release` job attaches every build artifact plus `SHA256SUMS.txt`.
  - Write the release notes first as `docs/releases/vX.Y.Z.md`.
- **Claude Code context:** the root `CLAUDE.md` loads automatically in local sessions. It holds the build commands, the file map, the conventions, and the release steps.

- **CI artifacts per run:**
  - `altWinDirStat-<ver>-<arch>` (installer + portable zip)
  - `screenshot-<arch>`
  - `crash-<arch>`, only when the scan test fails
- **Release:** `git tag vX.Y.Z && git push origin vX.Y.Z` runs the `release` job, which attaches all the zips and installers.
- **Crash triage recipe** (reusable locally):
  ```powershell
  # The key name must match the exe's basename: altWinDirStat.exe for CI/installed builds, windirstat.exe for a local build.
  $k='HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\altWinDirStat.exe'
  New-Item $k -Force; Set-ItemProperty $k DumpFolder C:\dumps -Type ExpandString; Set-ItemProperty $k DumpType 2 -Type DWord
  # reproduce the crash, then:
  cdb -z C:\dumps\<file>.dmp -y "<dir with windirstat.pdb>;srv*C:\sym*https://msdl.microsoft.com/download/symbols" -lines -c ".ecxr; kP 60; !analyze -v; q"
  ```
- **Menu/command routing:** popup menus are tracked with `AfxGetMainWnd()` as owner. Commands route frame → active view → document, so the document-level `ON_COMMAND` handlers receive both popups and the accelerators.
