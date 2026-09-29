# Change Tracking and View Polish: Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to carry out this plan task by task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Automatically remember every finished scan and show what changed since last time in a new **Changes** tab. Also make ages readable, explain columns with tooltips, and trim default column clutter.

**Architecture:**
- A fork-owned `History` module runs on the scan worker thread after a completed scan. It writes a folder-ledger snapshot per scanned location to `%LOCALAPPDATA%\altWinDirStat\History`, then diffs the scan against the previous session's snapshot using the existing `Ledger::Compare`.
- The result goes to the UI thread, where a fork-owned `CFileChangesView` tab shows only the significant rows, chosen by `Ledger::Significant`.
- Everything else is fork-owned helpers (`ForkFormat`, `ForkTooltips`, `ForkCli`) reached through one-line hooks in upstream files.

**Tech stack:**
- C++23, MSVC v143 (VS 2022), upstream's own MFC-replacement UI framework (`UiFramework.h`).
- PowerShell tests.
- GitHub Actions CI (windows-2022).

**Spec:** `docs/superpowers/specs/2026-09-29-change-tracking-design.md`

## Global Constraints

- **Small fork diff.** New logic goes in fork-owned files. Upstream files get one-line hooks, or the smallest possible edit. Every upstream touch is added to the fork-diff table in `HANDOFF.md`.
- **UI thread.** Never do snapshot I/O or diffing on the UI thread.
- **Registry.** Settings stay under `HKCU\Software\altWinDirStat`. Nothing may be written under `HKCU\Software\WinDirStat`; CI checks this.
- **History location:** `%LOCALAPPDATA%\altWinDirStat\History\<16-hex FNV-1a-64 of location key>\YYYYMMDD-HHMMSS-mmm.ledger.csv` (UTC), plus a `location.txt`. The env var `ALTWDS_HISTORY_DIR` overrides the root.
- **Retention:** keep the newest **5** snapshots per location.
- **Significance threshold:** `minOwnDelta` = **1 MiB** (1048576 bytes).
- **Strings:** every new UI string is an `IDS_*` key in `windirstat/res/fork/lang_en.txt`. Keep that file **sorted**: the pre-build `Compress Strings.ps1` re-sorts it in place, and an unsorted edit leaves the working tree dirty.
- **Fork IDs:** fork command IDs continue the fork range at **33902+**. Fork control IDs continue at **1907+**. Both live in `windirstat/ForkResource.h`.
- **Build command.** Every "Build" step means running this in PowerShell from the repo root:
  ```powershell
  & "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64 -SkipAutomaticLocation | Out-Null
  msbuild windirstat.sln /m /nologo /v:minimal /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v143
  ```
  Expected: `EXIT 0` and `build\altWinDirStat_x64.exe`. A `'vswhere.exe' is not recognized` line from the dev shell is harmless.
- **Test command.** Every "Run tests" step means:
  ```powershell
  powershell -NoProfile -ExecutionPolicy Bypass -File tests\Test-ForkChanges.ps1 -ExePath build\altWinDirStat_x64.exe
  ```
- **Commits.** Commit messages end with the line `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.

## Review Focus

These are the input classes most likely to hurt a real user that no task's happy-path test covers. Each one has a pinned test or code guard in the owning task.

1. **A stopped scan (user pressed Stop).** The worker still finalizes a partial tree. Snapshotting it would show huge false "Removed" lists next time. The hook must run only when `stopReason == Default`. Guard: Task 2, Step 6.
2. **Two app instances within the same second, or an F5 in the same second.** Snapshot names must not collide. The names carry milliseconds, and the same-session file is reused. Test: Task 2's retention run starts six processes back to back, waits for each one's new snapshot, and checks that exactly 5 files remain.
3. **A corrupt or half-written snapshot** (power loss, or a hand-edited file). The baseline search must skip it and fall back to the next newest, never crash or block a scan. Writes are atomic (`.tmp` then rename). Test: Task 2 drops a garbage `*.ledger.csv` newer than the real one and checks the next scan still writes and compares.
4. **The same folder scanned with different letter case or a trailing slash** (`C:\Data` vs `c:\data\`). Both must map to one location. Test: Task 2 scans the tree once through an upper-cased path with a trailing backslash and checks there is still exactly one location folder.
5. **Huge trees** (1M+ folders). `Significant` must be O(n) with a hash lookup, not a nested scan. Guard: the Task 1 implementation uses one `unordered_map` pass. CI's existing stress job (700k files plus all of C:) covers the UI-hang side once Task 2's hook is in.

---

## File map

| File | Status | Responsibility |
|---|---|---|
| `windirstat/Ledger.h/.cpp` | modify (fork-owned) | add `Significant`, `DefaultMinOwnDelta`, `SignedBytes`, `SignedCount`, `ChangeColor` |
| `windirstat/Dialogs/LedgerCompareDlg.h/.cpp` | modify (fork-owned) | use the shared helpers and the shared list control |
| `windirstat/Controls/LedgerListCtrl.h` | create | `CLedgerListCtrl`: a list control whose header text stays readable in dark mode (moved out of the dialog) |
| `windirstat/ForkCli.h/.cpp` | create | `/compare` headless command |
| `windirstat/ForkSettings.h` | create | `TrackChanges`, `ShowRelativeAge` settings |
| `windirstat/History.h/.cpp` | create | snapshot store, baseline and session logic, `OnScanComplete`, `PublishScan` |
| `windirstat/ForkFormat.h/.cpp` | create | `RelativeAge`, `LastChangeText`, `ShouldDimLastChange` |
| `windirstat/Views/FileChangesView.h/.cpp` | create | the Changes tab pane |
| `windirstat/ForkTooltips.h/.cpp` | create | header tooltips for list controls |
| `windirstat/ForkCommands.cpp` | modify (fork-owned) | Options-menu toggles |
| `windirstat/ForkResource.h`, `windirstat/res/fork/lang_en.txt` | modify (fork-owned) | IDs and strings |
| `windirstat/windirstat.vcxproj` | modify (fork `ItemGroup` only) | list the new sources |
| `windirstat/WinDirStat.cpp` | upstream hook | `ForkCli::RunIfRequested()` |
| `windirstat/WinDirStatModel.Actions.cpp` | upstream hook | `History::PublishScan` after a completed scan |
| `windirstat/WinDirStatModel.h` | upstream hook (extends the fork block) | toggle declarations and routes |
| `windirstat/windirstat.rc` | upstream hook | two Options-menu items |
| `windirstat/Views/FileTabbedView.h/.cpp` | upstream hooks | host the Changes pane |
| `windirstat/Item.Extended.cpp`, `windirstat/Item.h` | upstream hooks | readable Last Change text and dimmed stale cells |
| `windirstat/Controls/WdsListControl.h/.cpp` | upstream hooks | per-cell text-colour virtual |
| `windirstat/Views/FileTreeView.cpp` | upstream hooks | attach header tooltips |
| `windirstat/Options.cpp` | upstream one-value edit | Logical size hidden on fresh profiles |
| `tests/Test-ForkChanges.ps1`, `tests/fixtures/changes/*` | create | fork tests |
| `.github/workflows/build.yml` | modify | run the fork tests |
| `HANDOFF.md` | modify | fork-diff table and manual checklist |

---

### Task 1: Significant-change filter, shared helpers and `/compare` CLI

**Files:**
- Modify: `windirstat/Ledger.h`, `windirstat/Ledger.cpp`
- Create: `windirstat/Controls/LedgerListCtrl.h`
- Modify: `windirstat/Dialogs/LedgerCompareDlg.h`, `windirstat/Dialogs/LedgerCompareDlg.cpp`
- Create: `windirstat/ForkCli.h`, `windirstat/ForkCli.cpp`
- Modify: `windirstat/WinDirStat.cpp` (one include and one call)
- Modify: `windirstat/windirstat.vcxproj` (fork `ItemGroup`)
- Create: `tests/Test-ForkChanges.ps1`, `tests/fixtures/changes/base.ledger.csv`, `tests/fixtures/changes/cur.ledger.csv`, `tests/fixtures/changes/bad.ledger.csv`

**Interfaces:**
- **Consumes:** `Ledger::Load`, `Ledger::Compare`, `Ledger::SaveComparison` (existing).
- **Produces:**
  - `std::vector<size_t> Ledger::Significant(const std::vector<Ledger::DiffRow>& rows, ULONGLONG minOwnDelta, bool all)`
  - `constexpr ULONGLONG Ledger::DefaultMinOwnDelta = 1048576`
  - `std::wstring Ledger::SignedBytes(LONGLONG)`
  - `std::wstring Ledger::SignedCount(LONGLONG)`
  - `COLORREF Ledger::ChangeColor(Ledger::Change)`
  - `class CLedgerListCtrl final : public CListCtrl` (in `Controls/LedgerListCtrl.h`)
  - `void ForkCli::RunIfRequested()`
  - CLI: `altWinDirStat.exe /compare <baseline> <current> <out.csv> [/all]` → exit 0 on success, 1 on a load or save failure, 2 on bad usage.

- [ ] **Step 1: Write the fixtures**

`tests/fixtures/changes/base.ledger.csv` (the UTF-8 BOM is optional; `Ledger::Load` accepts both):
```
Path,Relative Path,Size (bytes),Files,Subfolders
"D:\T",".",100000000,10,7
"D:\T\a","a",60000000,6,2
"D:\T\a\b","a\b",50000000,5,1
"D:\T\a\b\c","a\b\c",10000000,1,0
"D:\T\edge","edge",1000000,1,0
"D:\T\gone","gone",20000000,2,1
"D:\T\gone\sub","gone\sub",5000000,1,0
"D:\T\small","small",1000000,1,0
```

`tests/fixtures/changes/cur.ledger.csv`:
```
Path,Relative Path,Size (bytes),Files,Subfolders
"D:\T",".",90485759,11,7
"D:\T\a","a",65242880,6,2
"D:\T\a\b","a\b",55242880,5,1
"D:\T\a\b\c","a\b\c",15242880,1,0
"D:\T\edge","edge",2048576,1,0
"D:\T\new","new",3145728,2,1
"D:\T\new\inner","new\inner",1048576,1,0
"D:\T\small","small",2048575,1,0
```

How this fixture works:
- `a\b\c` grows by exactly 5 MiB. Its ancestors `a\b`, `a` and `.` grow by the same amount, so their own delta is 0 and they are hidden.
- `edge` grows by exactly 1 MiB, so it is shown.
- `small` grows by 1 byte less than 1 MiB, so it is hidden.
- `new` and `new\inner` are both added; only `new` is shown.
- `gone` and `gone\sub` are both removed; only `gone` is shown.

`tests/fixtures/changes/bad.ledger.csv`:
```
Not,A,Ledger
1,2,3
```

- [ ] **Step 2: Write the failing test script**

Create `tests/Test-ForkChanges.ps1`:
```powershell
<#
    altWinDirStat fork tests: /compare significance filter and automatic scan history.
    Usage: powershell -File tests\Test-ForkChanges.ps1 -ExePath build\altWinDirStat_x64.exe [-SkipGui]
#>
param(
    [Parameter(Mandatory)] [string] $ExePath,
    [switch] $SkipGui
)
$ErrorActionPreference = 'Stop'
$ExePath = (Resolve-Path $ExePath).Path
$Fixtures = Join-Path $PSScriptRoot 'fixtures\changes'
$Work = Join-Path ([IO.Path]::GetTempPath()) ("awds-fork-" + [guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Force $Work | Out-Null
$failures = [System.Collections.Generic.List[string]]::new()
$passes = 0

function Check([bool] $condition, [string] $name, [string] $detail = '') {
    if ($condition) { $script:passes++; Write-Host "PASS  $name" }
    else { $script:failures.Add("$name $detail"); Write-Host "FAIL  $name $detail" -ForegroundColor Red }
}

function Invoke-App([string[]] $arguments, [int] $timeoutSec = 60) {
    $quoted = $arguments | ForEach-Object { if ($_ -match '\s') { "`"$_`"" } else { $_ } }
    $p = Start-Process -FilePath $ExePath -ArgumentList ($quoted -join ' ') -PassThru
    if (-not $p.WaitForExit($timeoutSec * 1000)) { Stop-Process -Id $p.Id -Force; return -999 }
    return $p.ExitCode
}

function Read-Folders([string] $csv) {
    @(Import-Csv -LiteralPath $csv -Encoding UTF8 | ForEach-Object { $_.Folder })
}

# --- /compare: significance filter -------------------------------------------------
$base = Join-Path $Fixtures 'base.ledger.csv'
$cur = Join-Path $Fixtures 'cur.ledger.csv'

$out = Join-Path $Work 'significant.csv'
$code = Invoke-App @('/compare', $base, $cur, $out)
Check ($code -eq 0) '/compare exits 0' "(exit $code)"
if (Test-Path $out) {
    $folders = Read-Folders $out
    Check (($folders -join '|') -eq 'edge|gone|new') 'significant rows are edge, gone, new' "(got '$($folders -join '|')')"
} else { Check $false '/compare wrote output' }

$outAll = Join-Path $Work 'all.csv'
$code = Invoke-App @('/compare', $base, $cur, $outAll, '/all')
Check ($code -eq 0) '/compare /all exits 0' "(exit $code)"
if (Test-Path $outAll) {
    $folders = Read-Folders $outAll
    $expected = '.|a|a\b|a\b\c|edge|gone|gone\sub|new|new\inner|small'
    Check (($folders -join '|') -eq $expected) '/all lists every changed row' "(got '$($folders -join '|')')"
} else { Check $false '/compare /all wrote output' }

$code = Invoke-App @('/compare', (Join-Path $Fixtures 'bad.ledger.csv'), $cur, (Join-Path $Work 'bad.csv'))
Check ($code -eq 1) 'malformed ledger gives exit 1' "(exit $code)"

$code = Invoke-App @('/compare', $base)
Check ($code -eq 2) 'missing arguments give exit 2' "(exit $code)"

# --- GUI scan history (added in Task 2) -----------------------------------------------
if (-not $SkipGui) {
    # GUI_TESTS_PLACEHOLDER_REPLACED_IN_TASK_2
}

Remove-Item $Work -Recurse -Force -ErrorAction SilentlyContinue
Write-Host ""
Write-Host "$passes passed, $($failures.Count) failed"
if ($failures.Count -gt 0) { exit 1 }
exit 0
```
(Task 2 replaces the comment line `# GUI_TESTS_PLACEHOLDER_REPLACED_IN_TASK_2` with real tests. Until then the block is empty on purpose.)

- [ ] **Step 3: Run the tests against the current build to confirm they fail**

Build, then run the tests. Expected result:
- `FAIL  /compare exits 0` or a timeout, because the current app treats `/compare` as a malformed command line. It exits 1, or opens a window that `Invoke-App` kills after 60 s and reports as `-999`.
- The script exits 1.

- [ ] **Step 4: Add the shared helpers and `Significant` to `Ledger`**

In `windirstat/Ledger.h`, inside `namespace Ledger`, after `ChangeName`, add:
```cpp
    // Change lists hide folders whose own change is below this many bytes (see Significant).
    constexpr ULONGLONG DefaultMinOwnDelta = 1024 * 1024;

    // Indices of rows worth showing in a change list. A folder is listed when it is the topmost
    // Added/Removed folder of a branch, or when it Grew/Shrank by at least minOwnDelta beyond what its
    // direct subfolders explain. With 'all', every row that changed at all is listed. O(n).
    std::vector<size_t> Significant(const std::vector<DiffRow>& rows, ULONGLONG minOwnDelta, bool all);

    std::wstring SignedBytes(LONGLONG delta);
    std::wstring SignedCount(LONGLONG delta);
    COLORREF ChangeColor(Change change);
```

In `windirstat/Ledger.cpp`, add to the anonymous namespace, after `IsSameOrBelow`:
```cpp
    // Lower-cased key of the parent row: "." for top-level folders, empty for the root itself.
    std::wstring ParentKey(const std::wstring& relative)
    {
        if (relative == rootRelative) return {};
        const size_t slash = relative.find_last_of(L'\\');
        return slash == std::wstring::npos ? std::wstring(rootRelative) : Key(std::wstring_view(relative).substr(0, slash));
    }
```
Append at the end of `Ledger.cpp`:
```cpp
std::vector<size_t> Ledger::Significant(const std::vector<DiffRow>& rows, const ULONGLONG minOwnDelta, const bool all)
{
    std::unordered_map<std::wstring, size_t> byKey;
    byKey.reserve(rows.size());
    for (size_t i = 0; i < rows.size(); ++i) byKey.emplace(Key(rows[i].relative), i);

    // A folder's own delta is its change minus the change of its direct child rows.
    std::vector<LONGLONG> own(rows.size());
    std::vector<size_t> parent(rows.size(), SIZE_MAX);
    for (size_t i = 0; i < rows.size(); ++i)
    {
        own[i] += rows[i].sizeDelta;
        if (const auto it = byKey.find(ParentKey(rows[i].relative)); it != byKey.end() && it->second != i)
        {
            parent[i] = it->second;
            own[it->second] -= rows[i].sizeDelta;
        }
    }

    const auto isAddedOrRemoved = [&](const size_t i)
    {
        return i != SIZE_MAX && (rows[i].change == Change::Added || rows[i].change == Change::Removed);
    };

    std::vector<size_t> result;
    for (size_t i = 0; i < rows.size(); ++i)
    {
        const Change change = rows[i].change;
        if (all)
        {
            if (change != Change::Unchanged) result.push_back(i);
            continue;
        }
        if ((change == Change::Added || change == Change::Removed) && !isAddedOrRemoved(parent[i]))
            result.push_back(i);
        else if ((change == Change::Grown || change == Change::Shrunk) &&
            static_cast<ULONGLONG>(own[i] < 0 ? -own[i] : own[i]) >= minOwnDelta)
            result.push_back(i);
    }
    return result;
}

std::wstring Ledger::SignedBytes(const LONGLONG delta)
{
    if (delta == 0) return L"0";
    const auto magnitude = static_cast<ULONGLONG>(delta < 0 ? -delta : delta);
    return (delta < 0 ? L"−" : L"+") + FormatBytes(magnitude);
}

std::wstring Ledger::SignedCount(const LONGLONG delta)
{
    if (delta == 0) return L"0";
    const auto magnitude = static_cast<ULONGLONG>(delta < 0 ? -delta : delta);
    return (delta < 0 ? L"−" : L"+") + FormatCount(magnitude);
}

COLORREF Ledger::ChangeColor(const Change change)
{
    const bool dark = DarkMode::IsDarkModeActive();
    switch (change)
    {
    case Change::Added:        return dark ? RGB(110, 220, 120) : RGB(0, 128, 0);
    case Change::Removed:      return dark ? RGB(255, 120, 120) : RGB(192, 0, 0);
    case Change::Grown:        return dark ? RGB(255, 190, 90) : RGB(176, 96, 0);
    case Change::Shrunk:       return dark ? RGB(120, 180, 255) : RGB(0, 90, 190);
    case Change::FilesChanged: return dark ? RGB(200, 160, 255) : RGB(110, 50, 160);
    default:                   return DarkMode::SystemColor(COLOR_WINDOWTEXT);
    }
}
```
The existing `Key` helper takes `std::wstring_view`. The `it->second != i` guard stops the root's empty parent key from ever matching.

- [ ] **Step 5: Move the dialog's list control and helpers to shared code**

Create `windirstat/Controls/LedgerListCtrl.h`:
```cpp
// altWinDirStat - list control used by folder-ledger views (fork-owned; not part of upstream WinDirStat)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//

#pragma once

#include "pch.h"

// Owner-data list whose native header text stays readable with the dark palette.
class CLedgerListCtrl final : public CListCtrl
{
    bool OnNotify(const WPARAM wParam, const LPARAM lParam, LRESULT* result) override
    {
        const auto header = reinterpret_cast<NMHDR*>(lParam);
        if (!header || header->code != NM_CUSTOMDRAW || header->hwndFrom != GetHeader().Handle() ||
            !DarkMode::IsDarkModeActive()) return CListCtrl::OnNotify(wParam, lParam, result);
        const auto draw = reinterpret_cast<NMCUSTOMDRAW*>(header);
        *result = draw->dwDrawStage == CDDS_PREPAINT ? CDRF_NOTIFYITEMDRAW : CDRF_DODEFAULT;
        if (draw->dwDrawStage == CDDS_ITEMPREPAINT) ::SetTextColor(draw->hdc, DarkMode::SystemColor(COLOR_BTNTEXT));
        return true;
    }
};
```
In `LedgerCompareDlg.h`:
- add `#include "LedgerListCtrl.h"`
- delete the nested `class ResultsList { ... };` block
- change the member `ResultsList m_list;` to `CLedgerListCtrl m_list;`

In `LedgerCompareDlg.cpp`:
- delete `SignedBytes`, `SignedCount` and `ChangeColor` from the anonymous namespace
- delete the `LedgerCompareDlg::ResultsList::OnNotify` definition
- replace the calls with `Ledger::SignedBytes(...)`, `Ledger::SignedCount(...)` and `Ledger::ChangeColor(...)`, including in `UpdateSummary`, `CellText` and `OnCustomDraw`

- [ ] **Step 6: Add the `/compare` CLI**

Create `windirstat/ForkCli.h`:
```cpp
// altWinDirStat - fork-only headless command lines (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header as LedgerListCtrl.h)

#pragma once

#include "pch.h"

namespace ForkCli
{
    // Handles "/compare <baseline.ledger.csv> <current.ledger.csv> <out.csv> [/all]" before upstream's
    // command-line parser sees it: writes the significant (or, with /all, every) folder change and exits
    // the process with 0 on success, 1 when a ledger cannot be read or the output written, 2 on bad usage.
    // Returns normally when the command line is not a fork command.
    void RunIfRequested();
}
```
Create `windirstat/ForkCli.cpp`:
```cpp
// altWinDirStat - fork-only headless command lines (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#include "pch.h"
#include "ForkCli.h"
#include "Ledger.h"

namespace
{
    bool IsFlag(const std::wstring& arg, const std::wstring_view name)
    {
        return arg.size() == name.size() + 1 && (arg[0] == L'/' || arg[0] == L'-') &&
            MakeLower(arg.substr(1)) == name;
    }

    bool CompareLedgers(const std::wstring& baselinePath, const std::wstring& currentPath,
        const std::wstring& outPath, const bool all)
    {
        std::wstring error;
        auto baseline = Ledger::Load(baselinePath, error);
        if (!baseline) return false;
        auto current = Ledger::Load(currentPath, error);
        if (!current) return false;

        Ledger::Summary summary;
        const auto rows = Ledger::Compare(*baseline, *current, summary);
        std::vector<Ledger::DiffRow> picked;
        for (const size_t i : Ledger::Significant(rows, Ledger::DefaultMinOwnDelta, all)) picked.push_back(rows[i]);
        return Ledger::SaveComparison(outPath, picked);
    }
}

void ForkCli::RunIfRequested()
{
    int argc = 0;
    const std::unique_ptr<wchar_t*, decltype(&LocalFree)> argv(CommandLineToArgvW(GetCommandLineW(), &argc), LocalFree);
    if (argv == nullptr || argc < 2) return;
    const std::vector<std::wstring> args(argv.get(), argv.get() + argc);
    if (!IsFlag(args[1], L"compare")) return;

    const bool all = argc == 6 && IsFlag(args[5], L"all");
    if (argc != 5 && !all) ExitProcess(2);
    ExitProcess(CompareLedgers(args[2], args[3], args[4], all) ? 0 : 1);
}
```
In `windirstat/WinDirStat.cpp`:
- add `#include "ForkCli.h"` after the existing includes
- directly after the line `Localization::LoadResource(MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL));` in `CDirStatApp::InitInstance`, add:
```cpp
    ForkCli::RunIfRequested(); // altWinDirStat: headless /compare (ForkCli.cpp)
```
In `windirstat/windirstat.vcxproj`, inside the fork `ItemGroup` (the one under the comment `altWinDirStat: fork-only sources`), add:
```xml
    <ClInclude Include="Controls\LedgerListCtrl.h" />
    <ClInclude Include="ForkCli.h" />
    <ClCompile Include="ForkCli.cpp" />
```

- [ ] **Step 7: Build and run the tests**

Build, then run the tests. Expected:
- `PASS` for all six checks
- the summary line `6 passed, 0 failed`
- exit code 0

- [ ] **Step 8: Check that the compare dialog still works**

Launch `build\altWinDirStat_x64.exe`, scan any small folder, then choose File → Compare with Folder Ledger… → `tests\fixtures\changes\base.ledger.csv`. Expected:
- the dialog opens with coloured rows
- in dark mode, the header text is readable

- [ ] **Step 9: Commit**
```bash
git add windirstat/Ledger.h windirstat/Ledger.cpp windirstat/Controls/LedgerListCtrl.h windirstat/Dialogs/LedgerCompareDlg.h windirstat/Dialogs/LedgerCompareDlg.cpp windirstat/ForkCli.h windirstat/ForkCli.cpp windirstat/WinDirStat.cpp windirstat/windirstat.vcxproj tests/Test-ForkChanges.ps1 tests/fixtures/changes
git commit -m "feat: significant-change filter and headless /compare"
```

---

### Task 2: Snapshot history on completed scans

**Files:**
- Create: `windirstat/ForkSettings.h`, `windirstat/History.h`, `windirstat/History.cpp`
- Modify: `windirstat/WinDirStatModel.Actions.cpp` (one include and one hook line)
- Modify: `windirstat/windirstat.vcxproj` (fork `ItemGroup`)
- Modify: `tests/Test-ForkChanges.ps1` (replace the GUI placeholder)

**Interfaces:**
- **Consumes:** `Ledger::FromScan`, `Ledger::Save`, `Ledger::Load`, `Ledger::Compare` (Task 1 or earlier).
- **Produces:**
  - `ForkSettings::TrackChanges`, `ForkSettings::ShowRelativeAge` (`Setting<bool>`)
  - `struct History::Result { Ledger::Snapshot baseline, current; std::vector<Ledger::DiffRow> rows; Ledger::Summary summary; FILETIME baselineTime; }`
  - `std::filesystem::path History::Root()`
  - `std::wstring History::LocationKey(const CItem* root)`
  - `std::shared_ptr<const History::Result> History::OnScanComplete(const CItem* root)`
  - `void History::PublishScan(const CItem* root)`. This is what the upstream hook calls. In this task it only calls `OnScanComplete`; Task 4 adds the UI hand-off.

- [ ] **Step 1: Write the failing GUI tests**

In `tests/Test-ForkChanges.ps1`, replace the line `    # GUI_TESTS_PLACEHOLDER_REPLACED_IN_TASK_2` with:
```powershell
    $history = Join-Path $Work 'history'
    $env:ALTWDS_HISTORY_DIR = $history
    $tree = Join-Path $Work 'tree'
    New-Item -ItemType Directory -Force (Join-Path $tree 'deep\nested') | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $tree 'other') | Out-Null
    Set-Content -LiteralPath (Join-Path $tree 'other\keep.txt') -Value 'x'

    function Get-Snapshots {
        if (-not (Test-Path $history)) { return @() }
        @(Get-ChildItem -LiteralPath $history -Recurse -Filter '*.ledger.csv' | Sort-Object Name)
    }

    function Get-NewestWrite {
        $s = Get-Snapshots | Sort-Object LastWriteTimeUtc | Select-Object -Last 1
        if ($s) { $s.LastWriteTimeUtc } else { [datetime]::MinValue }
    }

    # Scans $target in the GUI and returns once a snapshot newer than any before the scan exists (or times out).
    function Invoke-GuiScan([string] $target) {
        $since = Get-NewestWrite
        $p = Start-Process -FilePath $ExePath -ArgumentList "`"$target`"" -PassThru
        $sw = [Diagnostics.Stopwatch]::StartNew()
        while ((Get-NewestWrite) -le $since -and $sw.Elapsed.TotalSeconds -lt 60 -and -not $p.HasExited) {
            Start-Sleep -Milliseconds 200
        }
        Start-Sleep -Milliseconds 300
        Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue
        $p.WaitForExit(10000) | Out-Null
    }

    # 1. First scan writes one snapshot and a readable location.txt
    Invoke-GuiScan $tree
    $snaps = Get-Snapshots
    Check ($snaps.Count -eq 1) 'first scan writes one snapshot' "(found $($snaps.Count))"
    $locations = @(Get-ChildItem -LiteralPath $history -Directory -ErrorAction SilentlyContinue)
    Check ($locations.Count -eq 1) 'one location folder' "(found $($locations.Count))"
    if ($locations.Count -ge 1) {
        $loc = Get-Content -LiteralPath (Join-Path $locations[0].FullName 'location.txt') -Raw
        # Compare only the leaf: %TEMP% may be an 8.3 short path that the app does not expand.
        Check ($loc.Trim() -clike '*\tree') 'location.txt holds the lower-cased root' "(got '$loc')"
        Check ($locations[0].Name -match '^[0-9a-f]{16}$') 'location folder is a 16-hex hash' "(got '$($locations[0].Name)')"
    }

    # 2. Grow a nested folder by 5 MB and scan again in a new process
    $bytes = New-Object byte[] (5MB)
    [IO.File]::WriteAllBytes((Join-Path $tree 'deep\nested\big.bin'), $bytes)
    Invoke-GuiScan $tree
    $snaps = Get-Snapshots
    Check ($snaps.Count -eq 2) 'second session writes a second snapshot' "(found $($snaps.Count))"

    # 3. The two snapshots compare to exactly the nested folder
    if ($snaps.Count -eq 2) {
        $diff = Join-Path $Work 'gui-diff.csv'
        $code = Invoke-App @('/compare', $snaps[0].FullName, $snaps[1].FullName, $diff)
        $folders = if (Test-Path $diff) { Read-Folders $diff } else { @() }
        Check ($code -eq 0 -and ($folders -join '|') -eq 'deep\nested') 'snapshot diff shows only deep\nested' "(exit $code, got '$($folders -join '|')')"
    }

    # 4. Same folder with different case and a trailing slash maps to the same location
    Invoke-GuiScan ($tree.ToUpperInvariant() + '\')
    $locations = @(Get-ChildItem -LiteralPath $history -Directory)
    Check ($locations.Count -eq 1) 'case/trailing-slash variants share one location' "(found $($locations.Count))"

    # 5. A corrupt newer snapshot is skipped, not fatal
    $locDir = $locations[0].FullName
    Set-Content -LiteralPath (Join-Path $locDir '29991231-235959-999.ledger.csv') -Value 'garbage'
    $before = (Get-Snapshots).Count
    Invoke-GuiScan $tree
    Check ((Get-Snapshots).Count -eq $before + 1 -or (Get-Snapshots).Count -eq 5) 'scan still writes with a corrupt snapshot present'
    Remove-Item -LiteralPath (Join-Path $locDir '29991231-235959-999.ledger.csv') -ErrorAction SilentlyContinue

    # 6. Retention keeps the newest 5
    for ($i = 0; $i -lt 6; $i++) { Invoke-GuiScan $tree }
    $count = @(Get-ChildItem -LiteralPath $locDir -Filter '*.ledger.csv').Count
    Check ($count -eq 5) 'retention keeps 5 snapshots' "(found $count)"
    Check (@(Get-ChildItem -LiteralPath $locDir -Filter '*.tmp').Count -eq 0) 'no leftover .tmp files'

    Remove-Item Env:\ALTWDS_HISTORY_DIR
```
`Invoke-GuiScan` waits for a snapshot with a newer write time than any that existed before the scan, not for a higher file count. Once retention holds the count at 5, the count no longer changes, so waiting on it would return before the new snapshot was written.

- [ ] **Step 2: Run the tests to confirm the new checks fail**

Build, then run the tests. Expected:
- Task 1's six checks still pass.
- `FAIL  first scan writes one snapshot (found 0)`, and the GUI checks that follow also fail.
- Exit code 1.

- [ ] **Step 3: Create `ForkSettings.h`**
```cpp
// altWinDirStat - fork-only settings (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#pragma once

#include "pch.h"
#include "Options.h"

// Persisted with upstream's settings (HKCU\Software\altWinDirStat, or the portable ini) in their own section.
struct ForkSettings final
{
    inline static constexpr std::wstring_view Section = L"altWinDirStat";
    inline static Setting<bool> TrackChanges{ Section, L"TrackChanges", true };
    inline static Setting<bool> ShowRelativeAge{ Section, L"ShowRelativeAge", true };
};
```

- [ ] **Step 4: Create `History.h`**
```cpp
// altWinDirStat - automatic scan history and change detection (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#pragma once

#include "pch.h"
#include "Ledger.h"

class CItem;

// Every completed scan is saved as a folder ledger under Root()\<location hash>\. The first scan of a
// location in this process picks the newest snapshot written by an earlier process as its baseline and
// keeps it for the whole session, so refreshes keep comparing against "last time", not "a moment ago".
namespace History
{
    constexpr size_t KeepPerLocation = 5;

    struct Result
    {
        Ledger::Snapshot baseline;
        Ledger::Snapshot current;
        std::vector<Ledger::DiffRow> rows;  // Points into baseline/current: keep Result where it was built
        Ledger::Summary summary;
        FILETIME baselineTime{};            // When the baseline snapshot was written (UTC)
    };

    // %LOCALAPPDATA%\altWinDirStat\History, or ALTWDS_HISTORY_DIR when set.
    std::filesystem::path Root();

    // Lower-cased root path without trailing backslashes; for multi-drive scans the sorted drive roots
    // joined with '|'.
    std::wstring LocationKey(const CItem* root);

    // Scan-worker entry: saves this session's snapshot and compares it with the baseline. Returns nullptr
    // when tracking is off, there is no baseline yet, or anything failed (failures are traced, never thrown).
    std::shared_ptr<const Result> OnScanComplete(const CItem* root);

    // Called by the scan worker after a completed (not stopped) scan.
    void PublishScan(const CItem* root);
}
```

- [ ] **Step 5: Create `History.cpp`**
```cpp
// altWinDirStat - automatic scan history and change detection (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#include "pch.h"
#include "History.h"
#include "ForkSettings.h"
#include "Item.h"

namespace
{
    struct Session
    {
        bool resolved = false;                    // Baseline lookup done for this location
        std::optional<Ledger::Snapshot> baseline;
        FILETIME baselineTime{};
        std::filesystem::path file;               // This process's snapshot for the location
    };

    std::mutex s_lock;
    std::unordered_map<std::wstring, Session> s_sessions;   // By location key
    std::unordered_set<std::wstring> s_written;             // Lower-cased paths this process wrote

    std::wstring Lower(std::wstring text)
    {
        if (!text.empty()) CharLowerBuffW(text.data(), static_cast<DWORD>(text.size()));
        return text;
    }

    std::wstring HashKey(const std::wstring& key)
    {
        std::uint64_t hash = 14695981039346656037ull;
        for (const wchar_t c : key)
        {
            hash ^= static_cast<std::uint64_t>(c);
            hash *= 1099511628211ull;
        }
        return std::format(L"{:016x}", hash);
    }

    std::wstring NewSnapshotName()
    {
        SYSTEMTIME t{};
        GetSystemTime(&t);
        return std::format(L"{:04}{:02}{:02}-{:02}{:02}{:02}-{:03}.ledger.csv",
            t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    }

    // Snapshot files of one location, newest first (names sort chronologically).
    std::vector<std::filesystem::path> ListSnapshots(const std::filesystem::path& dir)
    {
        std::vector<std::filesystem::path> files;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(dir, ec))
            if (entry.is_regular_file(ec) && Ledger::IsLedgerPath(entry.path().wstring())) files.push_back(entry.path());
        std::ranges::sort(files, std::greater{});
        return files;
    }

    void ResolveBaseline(const std::filesystem::path& dir, Session& session)
    {
        session.resolved = true;
        for (const auto& file : ListSnapshots(dir))
        {
            if (s_written.contains(Lower(file.wstring()))) continue;
            std::wstring error;
            auto snapshot = Ledger::Load(file.wstring(), error);
            if (!snapshot)
            {
                VTRACE(L"History: skipping unreadable snapshot {}: {}", file.wstring(), error);
                continue;
            }
            WIN32_FILE_ATTRIBUTE_DATA data{};
            if (GetFileAttributesExW(file.c_str(), GetFileExInfoStandard, &data)) session.baselineTime = data.ftLastWriteTime;
            session.baseline = std::move(snapshot);
            return;
        }
    }

    bool SaveAtomically(const std::filesystem::path& file, const Ledger::Snapshot& snapshot)
    {
        std::filesystem::path temp = file;
        temp += L".tmp";
        if (!Ledger::Save(temp.wstring(), snapshot)) return false;
        return MoveFileExW(temp.c_str(), file.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    }

    void Prune(const std::filesystem::path& dir)
    {
        const auto files = ListSnapshots(dir);
        std::error_code ec;
        for (size_t i = History::KeepPerLocation; i < files.size(); ++i) std::filesystem::remove(files[i], ec);
    }

    // UTF-16 with BOM so any path round-trips; CREATE_NEW leaves an existing file alone.
    void WriteLocationFile(const std::filesystem::path& dir, const std::wstring& key)
    {
        const HANDLE file = CreateFileW((dir / L"location.txt").c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file == INVALID_HANDLE_VALUE) return;
        constexpr wchar_t bom = 0xFEFF;
        DWORD written = 0;
        WriteFile(file, &bom, sizeof(bom), &written, nullptr);
        WriteFile(file, key.data(), static_cast<DWORD>(key.size() * sizeof(wchar_t)), &written, nullptr);
        CloseHandle(file);
    }
}

std::filesystem::path History::Root()
{
    std::wstring overrideDir(MAX_PATH, L'\0');
    if (const DWORD n = GetEnvironmentVariableW(L"ALTWDS_HISTORY_DIR", overrideDir.data(), MAX_PATH); n > 0 && n < MAX_PATH)
    {
        overrideDir.resize(n);
        return overrideDir;
    }

    PWSTR local = nullptr;
    std::filesystem::path root;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local))) root = local;
    CoTaskMemFree(local);
    return root / L"altWinDirStat" / L"History";
}

std::wstring History::LocationKey(const CItem* root)
{
    std::vector<std::wstring> parts;
    if (root->GetItemType() == IT_MYCOMPUTER)
    {
        for (const CItem* child : root->GetChildren()) parts.push_back(child->GetPath());
    }
    else parts.push_back(root->GetPath());

    for (auto& part : parts)
    {
        while (!part.empty() && part.back() == L'\\') part.pop_back();
        part = Lower(std::move(part));
    }
    std::ranges::sort(parts);

    std::wstring key;
    for (const auto& part : parts)
    {
        if (!key.empty()) key += L'|';
        key += part;
    }
    return key;
}

std::shared_ptr<const History::Result> History::OnScanComplete(const CItem* root)
{
    if (root == nullptr || !ForkSettings::TrackChanges) return nullptr;
    try
    {
        const std::wstring key = LocationKey(root);
        const auto dir = Root() / HashKey(key);
        std::filesystem::create_directories(dir);
        WriteLocationFile(dir, key);

        auto result = std::make_shared<Result>();
        result->current = Ledger::FromScan(root);

        const std::lock_guard guard(s_lock);
        auto& session = s_sessions[key];
        if (!session.resolved) ResolveBaseline(dir, session);
        if (session.file.empty()) session.file = dir / NewSnapshotName();

        // Save before comparing: Compare may narrow 'current' in place.
        if (!SaveAtomically(session.file, result->current))
            VTRACE(L"History: could not write {}", session.file.wstring());
        s_written.insert(Lower(session.file.wstring()));
        Prune(dir);

        if (!session.baseline) return nullptr;
        result->baseline = *session.baseline;  // Copy: Compare may narrow it in place
        result->baselineTime = session.baselineTime;
        result->rows = Ledger::Compare(result->baseline, result->current, result->summary);
        return result;
    }
    catch (const std::exception& e)
    {
        VTRACE(L"History: {}", std::wstring(e.what(), e.what() + strlen(e.what())));
        return nullptr;
    }
}

void History::PublishScan(const CItem* root)
{
    (void)OnScanComplete(root);
}
```
If `SHGetKnownFolderPath`, `FOLDERID_LocalAppData`, `std::mutex`, `std::unordered_set` or `std::format` fail to compile, check `pch.h` and add the missing standard header (`<shlobj.h>`, `<mutex>`, `<unordered_set>`) at the top of `History.cpp`. Do not add them to `pch.h`, which is an upstream file.

- [ ] **Step 6: Hook the scan worker**

In `windirstat/WinDirStatModel.Actions.cpp`:
- add `#include "History.h"` after the file's existing includes
- change the two lines
```cpp
        CItem::ScanItemsFinalize(GetRootItem());
        Get()->RebuildExtensionData();
```
  to
```cpp
        CItem::ScanItemsFinalize(GetRootItem());
        Get()->RebuildExtensionData();
        if (stopReason == Default && CDirStatApp::Get()->GetSaveToPath().empty()) History::PublishScan(GetRootItem()); // altWinDirStat
```
The `stopReason == Default` check keeps stopped (partial) scans out of history (Review Focus 1). The `/saveto` check keeps headless exports out of history. `/savedupesto` and `/savepermsto` exit before any UI, and adding their checks too would widen this hook for no benefit: both run a full scan, so snapshotting them is harmless.

In the fork `ItemGroup` of `windirstat.vcxproj`, add:
```xml
    <ClInclude Include="ForkSettings.h" />
    <ClInclude Include="History.h" />
    <ClCompile Include="History.cpp" />
```

- [ ] **Step 7: Build and run the tests**

Build, then run the tests. Expected: all checks pass (Task 1's 6 plus Task 2's 10), and exit code 0.

- [ ] **Step 8: Check that a Stop leaves no snapshot**

Set `$env:ALTWDS_HISTORY_DIR` to a new temp folder. Launch the exe on `C:\` and press the toolbar Stop button within 2 seconds. Expected: no `*.ledger.csv` appears under that folder.

- [ ] **Step 9: Commit**
```bash
git add windirstat/ForkSettings.h windirstat/History.h windirstat/History.cpp windirstat/WinDirStatModel.Actions.cpp windirstat/windirstat.vcxproj tests/Test-ForkChanges.ps1
git commit -m "feat: save a folder snapshot after every completed scan"
```

---

### Task 3: Readable ages and dimmed stale folders

**Files:**
- Create: `windirstat/ForkFormat.h`, `windirstat/ForkFormat.cpp`
- Modify: `windirstat/Controls/WdsListControl.h`, `windirstat/Controls/WdsListControl.cpp` (a virtual and one call site)
- Modify: `windirstat/Item.h`, `windirstat/Item.Extended.cpp` (one override and one text hook)
- Modify: `windirstat/ForkResource.h`, `windirstat/windirstat.rc`, `windirstat/WinDirStatModel.h`, `windirstat/ForkCommands.cpp`
- Modify: `windirstat/res/fork/lang_en.txt`, `windirstat/windirstat.vcxproj`

**Interfaces:**
- **Consumes:** `ForkSettings::ShowRelativeAge` (Task 2).
- **Produces:**
  - `std::wstring ForkFormat::RelativeAge(const FILETIME& then, const FILETIME& now)`
  - `FILETIME ForkFormat::Now()`
  - `std::wstring ForkFormat::LastChangeText(const FILETIME&)`
  - `bool ForkFormat::ShouldDimLastChange(const FILETIME&)`
  - `ID_FORK_TRACK_CHANGES` (33902) and `ID_FORK_RELATIVE_AGES` (33903)
  - `CWinDirStatModel::OnForkTrackChanges / OnUpdateForkTrackChanges / OnForkRelativeAges / OnUpdateForkRelativeAges`

This task is display code with no headless surface, so it is checked by build and manual observation, not an automated test.

- [ ] **Step 1: Add strings and IDs**

Insert these into `windirstat/res/fork/lang_en.txt`, keeping the file sorted (ordinal sort, the same as `Sort-Object`):
```
IDS_AGE_DAYS={} days ago
IDS_AGE_HOURS={} hr ago
IDS_AGE_JUST_NOW=just now
IDS_AGE_MINUTES={} min ago
IDS_AGE_MONTHS={} mo ago
IDS_AGE_YEARS={} yrs ago
IDS_MENU_FORK_RELATIVE_AGES=Show &Relative Ages
IDS_MENU_FORK_TRACK_CHANGES=&Track Changes Between Scans
```
Append to `windirstat/ForkResource.h`:
```cpp
#define ID_FORK_TRACK_CHANGES           33902
#define ID_FORK_RELATIVE_AGES           33903
```

- [ ] **Step 2: Create `ForkFormat.h/.cpp`**

`windirstat/ForkFormat.h`:
```cpp
// altWinDirStat - human-friendly formatting helpers (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#pragma once

#include "pch.h"

namespace ForkFormat
{
    FILETIME Now();

    // "just now", "5 min ago", "7 hr ago", "12 days ago", "4 mo ago", "3 yrs ago". Future times give "".
    std::wstring RelativeAge(const FILETIME& then, const FILETIME& now);

    // Last Change cell text: "3 yrs ago (date)" when relative ages are on, otherwise upstream's date text.
    std::wstring LastChangeText(const FILETIME& t);

    // True when relative ages are on and t is more than 365 days ago.
    bool ShouldDimLastChange(const FILETIME& t);
}
```
`windirstat/ForkFormat.cpp`:
```cpp
// altWinDirStat - human-friendly formatting helpers (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#include "pch.h"
#include "ForkFormat.h"
#include "ForkSettings.h"

namespace
{
    constexpr ULONGLONG TicksPerSecond = 10'000'000ull;

    ULONGLONG Ticks(const FILETIME& t) { return (static_cast<ULONGLONG>(t.dwHighDateTime) << 32) | t.dwLowDateTime; }
}

FILETIME ForkFormat::Now()
{
    FILETIME now{};
    GetSystemTimeAsFileTime(&now);
    return now;
}

std::wstring ForkFormat::RelativeAge(const FILETIME& then, const FILETIME& now)
{
    if (Ticks(then) == 0 || Ticks(then) > Ticks(now)) return {};
    const ULONGLONG seconds = (Ticks(now) - Ticks(then)) / TicksPerSecond;
    const ULONGLONG minutes = seconds / 60, hours = minutes / 60, days = hours / 24;
    if (minutes < 1) return Localization::Lookup(IDS_AGE_JUST_NOW);
    if (hours < 1) return Localization::Format(IDS_AGE_MINUTES, minutes);
    if (hours < 48) return Localization::Format(IDS_AGE_HOURS, hours);
    if (days < 60) return Localization::Format(IDS_AGE_DAYS, days);
    if (days < 730) return Localization::Format(IDS_AGE_MONTHS, days / 30);
    return Localization::Format(IDS_AGE_YEARS, days / 365);
}

std::wstring ForkFormat::LastChangeText(const FILETIME& t)
{
    std::wstring date = FormatFileTime(t);
    if (!ForkSettings::ShowRelativeAge || date.empty()) return date;
    const std::wstring age = RelativeAge(t, Now());
    return age.empty() ? date : std::format(L"{} ({})", age, date);
}

bool ForkFormat::ShouldDimLastChange(const FILETIME& t)
{
    const ULONGLONG then = Ticks(t), now = Ticks(Now());
    return ForkSettings::ShowRelativeAge && then != 0 && then < now &&
        (now - then) / TicksPerSecond > 365ull * 24 * 60 * 60;
}
```
If `Localization::Format` doesn't accept `ULONGLONG` arguments, pass `FormatCount(n)` instead. It is already used that way in `LedgerCompareDlg`: `Localization::Format(IDS_LEDGER_SUMMARY_COUNTS, count(...))`.

- [ ] **Step 3: Hook the Last Change text and add a per-cell colour**

In `windirstat/Item.Extended.cpp`:
- add `#include "ForkFormat.h"` after the existing includes
- in the `case COL_LAST_CHANGE:` branch of `CItem::GetText`, change `return FormatFileTime(GetLastChange());` to:
```cpp
            return ForkFormat::LastChangeText(GetLastChange()); // altWinDirStat: relative age
```
In `windirstat/Controls/WdsListControl.h`, directly after the `GetItemTextColor()` virtual in `CWdsListItem`, add:
```cpp
    // altWinDirStat: per-cell text colour; defaults to the row colour.
    virtual COLORREF GetSubItemTextColor(int /*subitem*/) const { return GetItemTextColor(); }
```
In `windirstat/Controls/WdsListControl.cpp` `CWdsListControl::DrawItem`, change
`COLORREF textColor = item->GetItemTextColor();` to `COLORREF textColor = item->GetSubItemTextColor(subitem); // altWinDirStat`.

In `windirstat/Item.h`, after `COLORREF GetItemTextColor() const override;`, add:
```cpp
    COLORREF GetSubItemTextColor(int subitem) const override; // altWinDirStat
```
In `windirstat/Item.Extended.cpp`, after `CItem::GetItemTextColor`, add:
```cpp
// altWinDirStat: dim the Last Change of folders untouched for over a year
COLORREF CItem::GetSubItemTextColor(const int subitem) const
{
    if (subitem == COL_LAST_CHANGE && !IsTypeOrFlag(IT_FILE) && ForkFormat::ShouldDimLastChange(GetLastChange()))
        return DarkMode::SystemColor(COLOR_GRAYTEXT);
    return GetItemTextColor();
}
```

- [ ] **Step 4: Add the Options-menu toggles**

In `windirstat/windirstat.rc`, inside `POPUP "IDS_MENU_OPTIONS"`, directly before its first `MENUITEM`, add:
```
        MENUITEM "IDS_MENU_FORK_TRACK_CHANGES", ID_FORK_TRACK_CHANGES
        MENUITEM "IDS_MENU_FORK_RELATIVE_AGES", ID_FORK_RELATIVE_AGES
        MENUITEM SEPARATOR
```
In `windirstat/WinDirStatModel.h`, extend the altWinDirStat block with these declarations, after `void OnFolderLedgerCompare();`:
```cpp
    void OnForkTrackChanges();
    void OnUpdateForkTrackChanges(CCmdUI* pCmdUI);
    void OnForkRelativeAges();
    void OnUpdateForkRelativeAges(CCmdUI* pCmdUI);
```
Add these routes, after `Route::Command<&OnFolderLedgerCompare>(ID_FOLDER_LEDGER_COMPARE),`:
```cpp
        Route::Command<&OnForkTrackChanges>(ID_FORK_TRACK_CHANGES),
        Route::Update<&OnUpdateForkTrackChanges>(ID_FORK_TRACK_CHANGES),
        Route::Command<&OnForkRelativeAges>(ID_FORK_RELATIVE_AGES),
        Route::Update<&OnUpdateForkRelativeAges>(ID_FORK_RELATIVE_AGES),
```
In `windirstat/ForkCommands.cpp`, add `#include "ForkSettings.h"` and append:
```cpp
void CWinDirStatModel::OnForkTrackChanges()
{
    ForkSettings::TrackChanges = !ForkSettings::TrackChanges;
}

void CWinDirStatModel::OnUpdateForkTrackChanges(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(ForkSettings::TrackChanges ? 1 : 0);
}

void CWinDirStatModel::OnForkRelativeAges()
{
    ForkSettings::ShowRelativeAge = !ForkSettings::ShowRelativeAge;
    CFileTreeControl::Get()->Invalidate();
}

void CWinDirStatModel::OnUpdateForkRelativeAges(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(ForkSettings::ShowRelativeAge ? 1 : 0);
}
```
(Task 4 extends `OnForkTrackChanges` to hide the Changes tab.)

In the fork `ItemGroup`, add:
```xml
    <ClInclude Include="ForkFormat.h" />
    <ClCompile Include="ForkFormat.cpp" />
```

- [ ] **Step 5: Build, run the tests, and check by hand**

Build, then run the tests. Expected: everything still passes.

Manual check: launch the exe and scan `C:\Windows`. Expected:
- Last Change shows text like `3 yrs ago (2023-…)`.
- Folders older than a year show that cell in grey.
- Options → Show Relative Ages clears the check mark, and the column reverts to plain dates with no grey.
- Toggling it back restores both.
- Dark mode (Options → Settings → General → dark mode): the grey is readable.

- [ ] **Step 6: Commit**
```bash
git add windirstat/ForkFormat.h windirstat/ForkFormat.cpp windirstat/Controls/WdsListControl.h windirstat/Controls/WdsListControl.cpp windirstat/Item.h windirstat/Item.Extended.cpp windirstat/ForkResource.h windirstat/windirstat.rc windirstat/WinDirStatModel.h windirstat/ForkCommands.cpp windirstat/res/fork/lang_en.txt windirstat/windirstat.vcxproj
git commit -m "feat: readable relative ages and dimmed stale folders"
```

---

### Task 4: The Changes tab

**Files:**
- Create: `windirstat/Views/FileChangesView.h`, `windirstat/Views/FileChangesView.cpp`
- Modify: `windirstat/Views/FileTabbedView.h`, `windirstat/Views/FileTabbedView.cpp` (upstream hooks)
- Modify: `windirstat/History.cpp` (`PublishScan` hand-off), `windirstat/ForkCommands.cpp` (hide the tab when tracking is turned off)
- Modify: `windirstat/ForkResource.h`, `windirstat/res/fork/lang_en.txt`, `windirstat/windirstat.vcxproj`

**Interfaces:**
- **Consumes:**
  - `History::Result`, `History::OnScanComplete` (Task 2)
  - `Ledger::Significant`, `Ledger::SignedBytes`, `Ledger::SignedCount`, `Ledger::ChangeColor`, `CLedgerListCtrl` (Task 1)
  - `ForkFormat::RelativeAge`, `ForkFormat::Now` (Task 3)
- **Produces:**
  - `class CFileChangesView` with `void SetResult(std::shared_ptr<const History::Result>)`
  - `CFileTabbedView::SetChangesTabVisibility(bool)`
  - `CFileTabbedView::GetFileChangesView()`
  - `CFileTabbedView::SetActiveChangesView()`

This task is UI with no headless surface. It is verified by build, the existing and new automated suites staying green, and the manual checklist in Step 7.

- [ ] **Step 1: Add strings and control IDs**

Insert into `lang_en.txt`, keeping it sorted:
```
IDS_CHANGES_SHOW_ALL=Show &all changed folders
IDS_CHANGES_SUMMARY=Since {} ({}): net {}, {} new, {} removed, {} grew, {} shrank
IDS_CHANGES_TAB=Changes
IDS_CHANGES_COL_BEFORE=Before
IDS_CHANGES_COL_NOW=Now
IDS_CHANGES_COL_DIFF=Difference
IDS_CHANGES_COL_FILES=Files ±
```
Append to `ForkResource.h`:
```cpp
#define IDC_CHANGES_SUMMARY             1907
#define IDC_CHANGES_SHOW_ALL            1908
#define IDC_CHANGES_LIST                1909
```

- [ ] **Step 2: Create `FileChangesView.h`**
```cpp
// altWinDirStat - "Changes" tab: folders that changed since the previous session's scan
// (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#pragma once

#include "pch.h"
#include "WinDirStatPane.h"
#include "History.h"
#include "LedgerListCtrl.h"
#include "ForkResource.h"

class CFileChangesView final : public MessageTarget<CFileChangesView, CWinDirStatPane>
{
public:
    // Shows a new comparison (nullptr clears the view).
    void SetResult(std::shared_ptr<const History::Result> result);
    bool HasResult() const { return m_result != nullptr; }

    void OnDraw(CDC* pDC) override;
    void OnFontSizeChanged(int, int) override;
    static std::span<const RouteEntry> Routes();

protected:
    int OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnSize(UINT nType, int cx, int cy);
    bool OnEraseBkgnd(CDC*) { return true; }
    HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    void OnShowAll();
    void OnDisplayInfo(NMHDR* header, LRESULT* result);
    void OnColumnClick(NMHDR* header, LRESULT* result);
    void OnCustomDraw(NMHDR* header, LRESULT* result);
    void OnDoubleClick(NMHDR* header, LRESULT* result);

private:
    void UpdateList();
    void UpdateSummary();
    std::wstring CellText(const Ledger::DiffRow& row, int column) const;

    CStatic m_summary;
    CButton m_showAll;
    CLedgerListCtrl m_list;
    CFont m_font;
    HBRUSH m_background = nullptr;
    std::shared_ptr<const History::Result> m_result;
    std::vector<size_t> m_visible;
    std::wstring m_cell;
    int m_sortColumn = 4;       // Difference
    bool m_descending = true;
};

inline std::span<const RouteEntry> CFileChangesView::Routes()
{
    static constexpr std::array entries
    {
        Route::Window<&OnCreate>(WM_CREATE),
        Route::Window<&OnSize>(WM_SIZE),
        Route::Window<&OnEraseBkgnd>(WM_ERASEBKGND),
        Route::Window<&OnCtlColor>(WM_CTLCOLOR),
        Route::Control<&OnShowAll>(BN_CLICKED, IDC_CHANGES_SHOW_ALL),
        Route::Notify<&OnDisplayInfo>(LVN_GETDISPINFO, IDC_CHANGES_LIST),
        Route::Notify<&OnColumnClick>(LVN_COLUMNCLICK, IDC_CHANGES_LIST),
        Route::Notify<&OnCustomDraw>(NM_CUSTOMDRAW, IDC_CHANGES_LIST),
        Route::Notify<&OnDoubleClick>(NM_DBLCLK, IDC_CHANGES_LIST),
    };
    return entries;
}
```

- [ ] **Step 3: Create `FileChangesView.cpp`**
```cpp
// altWinDirStat - "Changes" tab (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#include "pch.h"
#include "FileChangesView.h"
#include "FileTabbedView.h"
#include "ForkFormat.h"

namespace
{
    enum Column : int { ColFolder, ColChange, ColBefore, ColNow, ColDelta, ColFiles, ColCount };

    LONGLONG Magnitude(const LONGLONG v) { return v < 0 ? -v : v; }

    LONGLONG SortValue(const Ledger::DiffRow& row, const int column)
    {
        switch (column)
        {
        case ColChange: return std::to_underlying(row.change);
        case ColBefore: return row.before ? static_cast<LONGLONG>(row.before->size) : -1;
        case ColNow:    return row.after ? static_cast<LONGLONG>(row.after->size) : -1;
        case ColDelta:  return Magnitude(row.sizeDelta);   // Biggest change first, grown or shrunk
        case ColFiles:  return row.filesDelta;
        default:        return 0;
        }
    }

    // Finds the live item for a folder row by walking relative-path components from the scan root.
    CItem* FindLiveFolder(const Ledger::DiffRow& row)
    {
        CItem* item = CWinDirStatModel::Get()->GetRootItem();
        if (item == nullptr || row.after == nullptr) return nullptr;
        if (row.relative == L".") return item;
        for (const auto& part : SplitString(row.relative, wds::chrBackslash))
        {
            if (item->IsLeaf()) return nullptr;
            const auto& children = item->GetChildren();
            const auto it = std::ranges::find_if(children, [&](const CItem* child)
            {
                return _wcsicmp(std::wstring(child->GetNameView()).c_str(), part.c_str()) == 0;
            });
            if (it == children.end()) return nullptr;
            item = *it;
        }
        return item;
    }
}

int CFileChangesView::OnCreate(const LPCREATESTRUCT lpCreateStruct)
{
    if (CWinDirStatPane::OnCreate(lpCreateStruct) == -1) return -1;

    const CRect rect(0, 0, 0, 0);
    m_summary.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_ENDELLIPSIS, rect, this, IDC_CHANGES_SUMMARY);
    m_showAll.Create(Localization::Lookup(IDS_CHANGES_SHOW_ALL).c_str(),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, rect, this, IDC_CHANGES_SHOW_ALL);
    m_list.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | LVS_REPORT | LVS_OWNERDATA | LVS_SHOWSELALWAYS | LVS_SINGLESEL,
        rect, this, IDC_CHANGES_LIST);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_GRIDLINES);

    const std::array columns{ IDS_LEDGER_COL_FOLDER, IDS_LEDGER_COL_CHANGE, IDS_CHANGES_COL_BEFORE,
        IDS_CHANGES_COL_NOW, IDS_CHANGES_COL_DIFF, IDS_CHANGES_COL_FILES };
    const std::array widths{ 360, 90, 90, 90, 100, 80 };
    for (int i = 0; i < ColCount; ++i)
        m_list.InsertColumn(i, Localization::Lookup(columns[i]), i <= ColChange ? LVCFMT_LEFT : LVCFMT_RIGHT, ScaleForDpi(widths[i]));
    m_list.SetBkColor(DarkMode::SystemColor(COLOR_WINDOW));
    m_list.SetTextBkColor(DarkMode::SystemColor(COLOR_WINDOW));
    m_list.SetTextColor(DarkMode::SystemColor(COLOR_WINDOWTEXT));

    DarkMode::AdjustControls(m_hWnd);
    OnFontSizeChanged(0, 0);
    return 0;
}

void CFileChangesView::OnFontSizeChanged(int, int)
{
    m_font.Create(-ScaleForDpi(12), FW_NORMAL, wds::strFontSegoeUI);
    m_summary.SetFont(m_font);
    m_showAll.SetFont(m_font);
    const CRect rc = GetClientRect();
    OnSize(SIZE_RESTORED, rc.Width(), rc.Height());
}

void CFileChangesView::OnSize(UINT, const int cx, const int cy)
{
    if (m_list.Handle() == nullptr) return;
    const int margin = ScaleForDpi(6), rowH = ScaleForDpi(22), checkW = ScaleForDpi(200);
    m_summary.MoveWindow(margin, margin, std::max(0, cx - checkW - 3 * margin), rowH);
    m_showAll.MoveWindow(std::max(margin, cx - checkW - margin), margin, checkW, rowH);
    const int top = rowH + 2 * margin;
    m_list.MoveWindow(0, top, cx, std::max(0, cy - top));
    Invalidate();
}

void CFileChangesView::OnDraw(CDC* pDC)
{
    pDC->FillSolidRect(GetClientRect(), DarkMode::SystemColor(COLOR_3DFACE));
}

HBRUSH CFileChangesView::OnCtlColor(CDC* pDC, CWnd*, const UINT nCtlColor)
{
    if (nCtlColor != CTLCOLOR_STATIC && nCtlColor != CTLCOLOR_BTN) return nullptr;
    pDC->SetTextColor(DarkMode::SystemColor(COLOR_WINDOWTEXT));
    pDC->SetBkColor(DarkMode::SystemColor(COLOR_3DFACE));
    if (m_background != nullptr) DeleteObject(m_background);
    m_background = CreateSolidBrush(DarkMode::SystemColor(COLOR_3DFACE));
    return m_background;
}

void CFileChangesView::SetResult(std::shared_ptr<const History::Result> result)
{
    m_result = std::move(result);
    UpdateSummary();
    UpdateList();
}

void CFileChangesView::UpdateSummary()
{
    if (!m_result)
    {
        m_summary.SetText(L"");
        return;
    }
    std::array<size_t, 6> counts{};
    for (const size_t i : Ledger::Significant(m_result->rows, Ledger::DefaultMinOwnDelta, false))
        ++counts[std::to_underlying(m_result->rows[i].change)];
    const auto count = [&](const Ledger::Change c) { return FormatCount(counts[std::to_underlying(c)]); };

    m_summary.SetText(Localization::Format(IDS_CHANGES_SUMMARY,
        FormatFileTime(m_result->baselineTime), ForkFormat::RelativeAge(m_result->baselineTime, ForkFormat::Now()),
        Ledger::SignedBytes(m_result->summary.sizeDelta),
        count(Ledger::Change::Added), count(Ledger::Change::Removed),
        count(Ledger::Change::Grown), count(Ledger::Change::Shrunk)));
}

void CFileChangesView::UpdateList()
{
    m_visible.clear();
    if (m_result)
    {
        const bool all = m_showAll.GetCheck() == BST_CHECKED;
        m_visible = Ledger::Significant(m_result->rows, Ledger::DefaultMinOwnDelta, all);
        std::ranges::stable_sort(m_visible, [this](const size_t a, const size_t b)
        {
            const auto& left = m_result->rows[m_descending ? b : a];
            const auto& right = m_result->rows[m_descending ? a : b];
            if (m_sortColumn == ColFolder) return _wcsicmp(left.relative.c_str(), right.relative.c_str()) < 0;
            return SortValue(left, m_sortColumn) < SortValue(right, m_sortColumn);
        });
    }
    m_list.SetItemCountEx(static_cast<int>(m_visible.size()), LVSICF_NOSCROLL);
    m_list.Invalidate();
}

std::wstring CFileChangesView::CellText(const Ledger::DiffRow& row, const int column) const
{
    switch (column)
    {
    case ColFolder: return row.relative;
    case ColChange: return Ledger::ChangeName(row.change);
    case ColBefore: return row.before ? FormatBytes(row.before->size) : std::wstring();
    case ColNow:    return row.after ? FormatBytes(row.after->size) : std::wstring();
    case ColDelta:  return Ledger::SignedBytes(row.sizeDelta);
    case ColFiles:  return Ledger::SignedCount(row.filesDelta);
    default:        return {};
    }
}

void CFileChangesView::OnShowAll()
{
    UpdateList();
}

void CFileChangesView::OnDisplayInfo(NMHDR* header, LRESULT* result)
{
    *result = 0;
    auto& item = reinterpret_cast<NMLVDISPINFO*>(header)->item;
    if (!m_result || !(item.mask & LVIF_TEXT) || item.iItem < 0 || static_cast<size_t>(item.iItem) >= m_visible.size()) return;
    m_cell = CellText(m_result->rows[m_visible[item.iItem]], item.iSubItem);
    item.pszText = m_cell.data();
}

void CFileChangesView::OnColumnClick(NMHDR* header, LRESULT* result)
{
    *result = 0;
    const int column = reinterpret_cast<NMLISTVIEW*>(header)->iSubItem;
    if (column < 0 || column >= ColCount) return;
    m_descending = column == m_sortColumn ? !m_descending : column != ColFolder;
    m_sortColumn = column;
    UpdateList();
}

void CFileChangesView::OnCustomDraw(NMHDR* header, LRESULT* result)
{
    *result = CDRF_DODEFAULT;
    auto* draw = reinterpret_cast<NMLVCUSTOMDRAW*>(header);
    if (draw->nmcd.dwDrawStage == CDDS_PREPAINT) *result = CDRF_NOTIFYITEMDRAW;
    else if (draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT && m_result && draw->nmcd.dwItemSpec < m_visible.size())
    {
        draw->clrText = Ledger::ChangeColor(m_result->rows[m_visible[draw->nmcd.dwItemSpec]].change);
        if (DarkMode::IsDarkModeActive()) draw->clrTextBk = DarkMode::SystemColor(COLOR_WINDOW);
    }
}

void CFileChangesView::OnDoubleClick(NMHDR* header, LRESULT* result)
{
    *result = 0;
    const int index = reinterpret_cast<NMITEMACTIVATE*>(header)->iItem;
    if (!m_result || index < 0 || static_cast<size_t>(index) >= m_visible.size()) return;

    // Reveal a folder that still exists in the All Files tree; removed folders have nothing to show.
    if (CItem* item = FindLiveFolder(m_result->rows[m_visible[index]]); item != nullptr)
    {
        CMainFrame::Get()->GetFileTabbedView()->SetActiveFileTreeView();
        CWinDirStatModel::Get()->NotifyPanes(MODEL_CHANGE_SELECTION_ACTION, item);
    }
}
```
**Compile-fix notes.** These are API details the implementer should confirm against `UiFramework.h` while building. If one differs, change only the line that calls it.
- **Button check state:** `CButton::GetCheck()`. If it's missing, use `IsDlgButtonChecked(m_hWnd, IDC_CHANGES_SHOW_ALL) == BST_CHECKED`.
- **Control constructors:** `CButton::Create(LPCWSTR, …)` and `CStatic::Create(LPCWSTR, …)` exist (UiFramework.h lines 1783 and 1793).
- **Item name comparison:** `CItem::GetNameView()` may already return a `std::wstring_view`. If so, compare with `_wcsnicmp` plus a length check instead of building a `std::wstring`.
- **`WM_CTLCOLOR` route:** `CStorageAnalyticsView` uses the same route name.

- [ ] **Step 4: Host the pane in `CFileTabbedView` (upstream hooks)**

`windirstat/Views/FileTabbedView.h`:
- after `class CStorageAnalyticsView;`, add `class CFileChangesView; // altWinDirStat`
- in the public section, after `SetStorageAnalyticsTabVisibility(bool show = true);`, add:
```cpp
    void SetChangesTabVisibility(bool show = true); // altWinDirStat
    CFileChangesView* GetFileChangesView() const { return m_fileChangesView; } // altWinDirStat
    void SetActiveChangesView() { SetActiveView(m_fileChangesViewIndex); } // altWinDirStat
```
- after `CStorageAnalyticsView* m_storageAnalyticsView = nullptr;`, add:
```cpp
    int m_fileChangesViewIndex = -1; // altWinDirStat
    CFileChangesView* m_fileChangesView = nullptr; // altWinDirStat
```
`windirstat/Views/FileTabbedView.cpp`:
- after `#include "StorageAnalyticsView.h"`, add `#include "FileChangesView.h" // altWinDirStat`
- in `OnCreate`, directly before `OnInitialUpdate();`, add:
```cpp
    m_fileChangesView = AddPane<CFileChangesView>(m_fileChangesViewIndex, IDS_CHANGES_TAB); // altWinDirStat
    if (m_fileChangesView == nullptr) return -1; // altWinDirStat
```
  The tab is added last, so no upstream tab index shifts.
- in `ResetOptionalTabVisibility()`, append `SetChangesTabVisibility(false); // altWinDirStat`
- add the method after `SetStorageAnalyticsTabVisibility`:
```cpp
// altWinDirStat: the Changes tab is shown only while a comparison is loaded
void CFileTabbedView::SetChangesTabVisibility(const bool show)
{
    GetTabControl().SetTabVisible(m_fileChangesViewIndex, show);
    if (!show && m_fileChangesView != nullptr) m_fileChangesView->SetResult(nullptr);
}
```
- in `OnUpdate`, add `static_cast<CWinDirStatPane*>(m_fileChangesView),` as the last element of the initializer list, after `m_storageAnalyticsView`
- in `CycleTab`, add `, m_fileChangesViewIndex` at the end of the `allTabs` list

- [ ] **Step 5: Hand results to the UI and hide the tab when tracking is turned off**

In `History.cpp`, replace `PublishScan`:
```cpp
void History::PublishScan(const CItem* root)
{
    auto result = OnScanComplete(root);
    CMainFrame::Get()->InvokeInMessageThread([result = std::move(result)]() mutable
    {
        auto* tabs = CMainFrame::Get()->GetFileTabbedView();
        if (tabs == nullptr) return;
        if (!result)
        {
            tabs->SetChangesTabVisibility(false);
            return;
        }
        tabs->GetFileChangesView()->SetResult(std::move(result));
        tabs->SetChangesTabVisibility(true);
    });
}
```
Also add `#include "FileTabbedView.h"` and `#include "FileChangesView.h"` to `History.cpp`.

Note the order in `SetChangesTabVisibility(true)` on the success path: `SetResult` runs first, then the tab is made visible. `SetChangesTabVisibility(false)` clears the result, so it must not run after `SetResult`.

In `ForkCommands.cpp` `OnForkTrackChanges`, add after the toggle:
```cpp
    if (!ForkSettings::TrackChanges) CMainFrame::Get()->GetFileTabbedView()->SetChangesTabVisibility(false);
```
Add `#include "FileTabbedView.h"` to `ForkCommands.cpp` if it isn't reachable through `pch.h`.

In the fork `ItemGroup`, add:
```xml
    <ClInclude Include="Views\FileChangesView.h" />
    <ClCompile Include="Views\FileChangesView.cpp" />
```

- [ ] **Step 6: Build and run the tests**

Build, then run the tests. Expected: all checks pass, and exit code 0. The tests don't drive the tab, but they prove that `PublishScan`'s UI hand-off doesn't crash or hang a scan.

- [ ] **Step 7: Manual checklist (x64 build, History at its default location)**

1. Scan a scratch folder, `%TEMP%\awds-manual`, containing `a\b` and `c`. Close the app.
2. Add a 20 MB file to `a\b`, delete `c`, and create `d\e`. Launch and scan the same folder. Expected:
   - a **Changes** tab appears
   - the summary reads `Since <date> (just now|N min ago): net …, 1 new, 1 removed, 1 grew, 0 shrank`
   - the rows are `a\b` (Grown +20 MB, orange), `c` (Removed, red) and `d` (Added, green)
   - `a` and `.` are not listed
3. Tick **Show all changed folders**: `a` and `.` appear. Untick it: they disappear.
4. Click the column headers: each sorts; clicking again reverses the order.
5. Double-click `a\b`: the All Files tab activates with `a\b` selected. Double-click `c` (Removed): nothing happens.
6. Press F5: the tab stays and still compares against the first session.
7. Options → Track Changes Between Scans (uncheck): the tab disappears. Rescan: no tab, and no new snapshot file. Re-check it for the next step.
8. Scan a folder never scanned before: no Changes tab.
9. Switch to dark mode and repeat step 2's view: the text and colours are readable, and the header text is readable.
10. Resize the window narrow and wide: the summary truncates with an ellipsis and the checkbox stays right-aligned.

- [ ] **Step 8: Commit**
```bash
git add windirstat/Views/FileChangesView.h windirstat/Views/FileChangesView.cpp windirstat/Views/FileTabbedView.h windirstat/Views/FileTabbedView.cpp windirstat/History.cpp windirstat/ForkCommands.cpp windirstat/ForkResource.h windirstat/res/fork/lang_en.txt windirstat/windirstat.vcxproj
git commit -m "feat: Changes tab showing folder changes since the last session"
```

---

### Task 5: Column header tooltips

**Files:**
- Create: `windirstat/ForkTooltips.h`, `windirstat/ForkTooltips.cpp`
- Modify: `windirstat/Views/FileTreeView.cpp` (two hook lines), `windirstat/Views/FileChangesView.cpp` (one line)
- Modify: `windirstat/res/fork/lang_en.txt`, `windirstat/windirstat.vcxproj`

**Interfaces:**
- **Consumes:** `COL_*` and `COL_ITEMTOP_*` enums (upstream), `CListCtrl::GetHeader()`.
- **Produces:**
  - `using ForkTooltips::Lookup = std::wstring_view (*)(int subitem)`
  - `void ForkTooltips::AttachHeader(CListCtrl& list, Lookup lookup)`
  - `std::wstring_view ForkTooltips::FileTreeTip(int)`, `TopListTip(int)`, `ChangesTip(int)`

- [ ] **Step 1: Add strings (sorted)**
```
IDS_TIP_CHG_BEFORE=Folder size at the earlier scan.
IDS_TIP_CHG_CHANGE=Added, Removed, Grown or Shrunk since the earlier scan.
IDS_TIP_CHG_DIFF=How much the folder grew (+) or shrank (−) on its own.
IDS_TIP_CHG_FILES=Change in the number of files inside.
IDS_TIP_CHG_FOLDER=Folder path relative to the scanned location.
IDS_TIP_CHG_NOW=Folder size now.
IDS_TIP_COL_ATTRIBUTES=File attributes such as read-only, hidden, system, compressed or encrypted.
IDS_TIP_COL_FILES=Number of files inside, counted all the way down.
IDS_TIP_COL_FOLDERS=Number of subfolders inside, counted all the way down.
IDS_TIP_COL_ITEMS=Files plus folders inside, counted all the way down.
IDS_TIP_COL_LAST_CHANGE=When anything inside was last modified. Dimmed after a year: likely forgotten data.
IDS_TIP_COL_NAME=Name of the file or folder.
IDS_TIP_COL_OWNER=The account that owns the item.
IDS_TIP_COL_PERCENTAGE=Share of the parent folder's size.
IDS_TIP_COL_SIZE_LOGICAL=The file's real length in bytes, however it is stored.
IDS_TIP_COL_SIZE_PHYSICAL=Space actually used on disk, after compression, sparse files and cloud-only placeholders.
IDS_TIP_COL_SIZE_PROPORTION=Bar showing how much of its parent folder this item takes up.
```

- [ ] **Step 2: Create `ForkTooltips.h/.cpp`**

`windirstat/ForkTooltips.h`:
```cpp
// altWinDirStat - plain-language tooltips on list column headers (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#pragma once

#include "pch.h"

namespace ForkTooltips
{
    // Maps a column's subitem id to a string key (IDS_TIP_*), or an empty view for no tooltip.
    using Lookup = std::wstring_view (*)(int subitem);

    // Adds a hover tooltip to each header column of 'list'. Safe to call once per list; cleans up with the header.
    void AttachHeader(CListCtrl& list, Lookup lookup);

    std::wstring_view FileTreeTip(int subitem);
    std::wstring_view TopListTip(int subitem);
    std::wstring_view ChangesTip(int subitem);
}
```
`windirstat/ForkTooltips.cpp`:
```cpp
// altWinDirStat - plain-language tooltips on list column headers (fork-owned; not part of upstream WinDirStat)
//
// (same GPL header)

#include "pch.h"
#include "ForkTooltips.h"
#include "Item.h"
#include "ItemTop.h"

namespace
{
    constexpr UINT_PTR SubclassId = 0x41575453; // 'AWTS'

    struct HeaderTip
    {
        HWND list = nullptr;
        HWND tip = nullptr;
        ForkTooltips::Lookup lookup = nullptr;
        int column = -2;
        std::wstring text;
    };

    void UpdateTip(const HWND header, HeaderTip& state, const int column)
    {
        state.column = column;
        LVCOLUMNW col{ .mask = LVCF_SUBITEM };
        const bool known = column >= 0 && ::SendMessageW(state.list, LVM_GETCOLUMNW, column, reinterpret_cast<LPARAM>(&col));
        const std::wstring_view key = known ? state.lookup(col.iSubItem) : std::wstring_view{};
        state.text = key.empty() ? std::wstring() : Localization::Lookup(key);

        RECT rc{};
        if (column >= 0) ::SendMessageW(header, HDM_GETITEMRECT, column, reinterpret_cast<LPARAM>(&rc));
        TTTOOLINFOW ti{ .cbSize = sizeof(ti), .hwnd = header, .uId = 1, .rect = rc, .lpszText = state.text.data() };
        ::SendMessageW(state.tip, TTM_POP, 0, 0);
        ::SendMessageW(state.tip, TTM_NEWTOOLRECTW, 0, reinterpret_cast<LPARAM>(&ti));
        ::SendMessageW(state.tip, TTM_UPDATETIPTEXTW, 0, reinterpret_cast<LPARAM>(&ti));
        ::SendMessageW(state.tip, TTM_ACTIVATE, !state.text.empty(), 0);
    }

    LRESULT CALLBACK HeaderProc(const HWND hwnd, const UINT msg, const WPARAM wp, const LPARAM lp,
        const UINT_PTR id, const DWORD_PTR data)
    {
        auto* state = reinterpret_cast<HeaderTip*>(data);
        if (msg == WM_MOUSEMOVE)
        {
            HDHITTESTINFO hit{ .pt = { GET_X_LPARAM(lp), GET_Y_LPARAM(lp) } };
            const int column = static_cast<int>(::SendMessageW(hwnd, HDM_HITTEST, 0, reinterpret_cast<LPARAM>(&hit)));
            if (column != state->column) UpdateTip(hwnd, *state, column);
        }
        else if (msg == WM_NCDESTROY)
        {
            ::RemoveWindowSubclass(hwnd, HeaderProc, id);
            ::DestroyWindow(state->tip);
            delete state;
        }
        return ::DefSubclassProc(hwnd, msg, wp, lp);
    }
}

void ForkTooltips::AttachHeader(CListCtrl& list, const Lookup lookup)
{
    const HWND header = list.GetHeader().Handle();
    if (header == nullptr || lookup == nullptr) return;
    DWORD_PTR existing = 0;
    if (::GetWindowSubclass(header, HeaderProc, SubclassId, &existing)) return;

    const HWND tip = ::CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, header, nullptr, nullptr, nullptr);
    if (tip == nullptr) return;
    ::SendMessageW(tip, TTM_SETMAXTIPWIDTH, 0, ScaleForDpi(400));

    TTTOOLINFOW ti{ .cbSize = sizeof(ti), .uFlags = TTF_SUBCLASS, .hwnd = header, .uId = 1, .lpszText = const_cast<LPWSTR>(L"") };
    ::SendMessageW(tip, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&ti));
    ::SendMessageW(tip, TTM_ACTIVATE, FALSE, 0);

    auto* state = new HeaderTip{ .list = list.Handle(), .tip = tip, .lookup = lookup };
    if (!::SetWindowSubclass(header, HeaderProc, SubclassId, reinterpret_cast<DWORD_PTR>(state)))
    {
        ::DestroyWindow(tip);
        delete state;
    }
}

std::wstring_view ForkTooltips::FileTreeTip(const int subitem)
{
    switch (subitem)
    {
    case COL_NAME:            return IDS_TIP_COL_NAME;
    case COL_SIZE_PROPORTION: return IDS_TIP_COL_SIZE_PROPORTION;
    case COL_PERCENTAGE:      return IDS_TIP_COL_PERCENTAGE;
    case COL_SIZE_PHYSICAL:   return IDS_TIP_COL_SIZE_PHYSICAL;
    case COL_SIZE_LOGICAL:    return IDS_TIP_COL_SIZE_LOGICAL;
    case COL_ITEMS:           return IDS_TIP_COL_ITEMS;
    case COL_FILES:           return IDS_TIP_COL_FILES;
    case COL_FOLDERS:         return IDS_TIP_COL_FOLDERS;
    case COL_LAST_CHANGE:     return IDS_TIP_COL_LAST_CHANGE;
    case COL_ATTRIBUTES:      return IDS_TIP_COL_ATTRIBUTES;
    case COL_OWNER:           return IDS_TIP_COL_OWNER;
    default:                  return {};
    }
}

std::wstring_view ForkTooltips::TopListTip(const int subitem)
{
    switch (subitem)
    {
    case COL_ITEMTOP_NAME:          return IDS_TIP_COL_NAME;
    case COL_ITEMTOP_SIZE_PHYSICAL: return IDS_TIP_COL_SIZE_PHYSICAL;
    case COL_ITEMTOP_SIZE_LOGICAL:  return IDS_TIP_COL_SIZE_LOGICAL;
    case COL_ITEMTOP_LAST_CHANGE:   return IDS_TIP_COL_LAST_CHANGE;
    default:                        return {};
    }
}

std::wstring_view ForkTooltips::ChangesTip(const int subitem)
{
    // Column order matches CFileChangesView's Column enum.
    static constexpr std::array tips{ IDS_TIP_CHG_FOLDER, IDS_TIP_CHG_CHANGE, IDS_TIP_CHG_BEFORE,
        IDS_TIP_CHG_NOW, IDS_TIP_CHG_DIFF, IDS_TIP_CHG_FILES };
    return subitem >= 0 && subitem < static_cast<int>(tips.size()) ? tips[subitem] : std::wstring_view{};
}
```
If `COL_ITEMTOP_*` isn't declared in `ItemTop.h`, grep for `COL_ITEMTOP_NAME` and include the header that defines it. `GET_X_LPARAM` comes from `<windowsx.h>`; include it in this file if `pch.h` doesn't.

- [ ] **Step 3: Attach the tooltips (hooks)**

In `windirstat/Views/FileTreeView.cpp`:
- add `#include "ForkTooltips.h"`
- as the last line of `CFileTreeView::InitializeColumns()`, add:
```cpp
    ForkTooltips::AttachHeader(control, ForkTooltips::FileTreeTip); // altWinDirStat
```
- as the last line of `CFileTopView::InitializeColumns()`, add:
```cpp
    ForkTooltips::AttachHeader(control, ForkTooltips::TopListTip); // altWinDirStat
```
In `FileChangesView.cpp`:
- add `#include "ForkTooltips.h"`
- directly after the column-insert loop in `OnCreate`, add:
```cpp
    ForkTooltips::AttachHeader(m_list, ForkTooltips::ChangesTip);
```
In the fork `ItemGroup`, add:
```xml
    <ClInclude Include="ForkTooltips.h" />
    <ClCompile Include="ForkTooltips.cpp" />
```

- [ ] **Step 4: Build, run the tests, and check by hand**

Build, then run the tests. Expected: all checks pass.

Manual check: hover over each All Files column header, the Largest Files headers and the Changes headers. Expected:
- a one-line explanation appears after the usual tooltip delay
- moving between columns changes the text
- dragging a column to reorder it still shows the right text for the column under the mouse
- a header's right-click column menu still works

- [ ] **Step 5: Commit**
```bash
git add windirstat/ForkTooltips.h windirstat/ForkTooltips.cpp windirstat/Views/FileTreeView.cpp windirstat/Views/FileChangesView.cpp windirstat/res/fork/lang_en.txt windirstat/windirstat.vcxproj
git commit -m "feat: plain-language tooltips on column headers"
```

---

### Task 6: Default layout, CI, docs and delivery

**Files:**
- Modify: `windirstat/Options.cpp` (one value)
- Modify: `.github/workflows/build.yml`
- Modify: `HANDOFF.md`

**Interfaces:**
- **Consumes:** everything above.
- **Produces:** a pushed branch, a PR, and a rebuilt local exe.

- [ ] **Step 1: Hide Logical size on fresh profiles**

In `windirstat/Options.cpp`, change
```cpp
        visibility = { 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 0 };
```
to
```cpp
        visibility = { 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0 }; // altWinDirStat: Logical size hidden by default
```
Index 4 is `COL_SIZE_LOGICAL`, as defined in the `Item.h` column enum. This applies only when the stored setting is empty, so existing layouts are untouched.

- [ ] **Step 2: Check it on a clean profile**

Build. Then create an empty `altWinDirStat.ini` next to a copy of the exe; `SetPortableMode` then uses the ini instead of the registry, which gives a clean profile. Launch the copy and scan any folder. Expected:
- no Logical size column
- right-clicking the header shows Logical size unticked, and ticking it brings the column back

Delete the ini and the copy afterwards.

If an empty ini does not switch to portable mode, find the portable-mode file name in `SetPortableMode` (`WinDirStat.cpp`) and use that.

- [ ] **Step 3: Run the fork tests in CI**

In `.github/workflows/build.yml`, directly after the `Upstream test suites` step, add:
```yaml
      - name: Fork tests (change tracking)
        shell: pwsh
        run: |
          & powershell -NoProfile -ExecutionPolicy Bypass -File tests\Test-ForkChanges.ps1 -ExePath "$env:DIST\altWinDirStat.exe"
          if ($LASTEXITCODE -ne 0) { throw "Fork tests failed (exit $LASTEXITCODE)" }
```
The script works on the x64, Win32 and ARM64 runners' exe alike, because it only launches the exe and reads files.

- [ ] **Step 4: Update `HANDOFF.md`**

Add these rows to the fork-diff table:

| File | Change | Why |
|---|---|---|
| `windirstat/WinDirStat.cpp` | `#include "ForkCli.h"` and `ForkCli::RunIfRequested()` after the bootstrap language load | Headless `/compare`, handled before upstream's strict parser |
| `windirstat/WinDirStatModel.Actions.cpp` | `#include "History.h"` and one line after `RebuildExtensionData()`: `History::PublishScan` when `stopReason == Default` and not `/saveto` | Automatic snapshots and the Changes tab; stopped scans are excluded |
| `windirstat/WinDirStatModel.h` | four more declarations and routes in the fork block | Options-menu toggles |
| `windirstat/windirstat.rc` | two Options-menu items plus a separator | Track Changes and Show Relative Ages |
| `windirstat/Views/FileTabbedView.h/.cpp` | forward declaration, include, two members, three inline methods, `SetChangesTabVisibility`, `AddPane` (added last), and the Changes view added to `ResetOptionalTabVisibility`, `OnUpdate` and `CycleTab` | Hosts the Changes tab without shifting upstream tab indices |
| `windirstat/Controls/WdsListControl.h/.cpp` | `CWdsListItem::GetSubItemTextColor` virtual (defaults to the row colour); `DrawItem` calls it | Per-cell colour for dimmed ages |
| `windirstat/Item.h`, `windirstat/Item.Extended.cpp` | `GetSubItemTextColor` override; Last Change text via `ForkFormat::LastChangeText` | Readable ages; stale folders dimmed |
| `windirstat/Views/FileTreeView.cpp` | `ForkTooltips::AttachHeader` at the end of the All Files and Largest Files `InitializeColumns` | Header tooltips |
| `windirstat/Options.cpp` | default `FileTreeColumnVisibility`: Logical size off | Less clutter on fresh profiles |
| Added: `ForkCli`, `ForkSettings.h`, `History`, `ForkFormat`, `ForkTooltips`, `Views/FileChangesView`, `Controls/LedgerListCtrl.h`, `tests/Test-ForkChanges.ps1`, `tests/fixtures/changes/` | | Change tracking and view polish (design in `docs/superpowers/specs/2026-09-29-change-tracking-design.md`) |

Also update the existing `windirstat.vcxproj` and `ForkResource.h` rows to mention the new files and IDs (commands 33902–33903, controls 1907–1909).

Then append a section called `## Manual checklist: Changes tab, ages and tooltips` that copies Task 4's Step 7 list and Task 5's Step 4 list, and add one line to the CI section: "`Fork tests (change tracking)` runs `tests/Test-ForkChanges.ps1`".

- [ ] **Step 5: Full local verification**

1. Build.
2. Run the fork tests. Expected: all pass.
3. Run the upstream suites that CI runs (needs PowerShell 7.6+; skip with a note if `pwsh` isn't installed):
   ```powershell
   pwsh -File tests\Test-WinDirStat.ps1 -ExePath build\altWinDirStat_x64.exe -Only Filtering,Cli,EdgeCases,Permissions,Enumeration,Unc,Ui
   ```
   Expected: no FAIL lines.
4. Run the stress test, without the drive scan to keep it short:
   ```powershell
   pwsh -File .github\scripts\Stress-LargeScan.ps1 -ExePath build\altWinDirStat_x64.exe -SkipDriveScan
   ```
   Expected: no hang failures. This now also exercises `History::PublishScan` on a 6,324-folder tree.

- [ ] **Step 6: Commit and push**
```bash
git add windirstat/Options.cpp .github/workflows/build.yml HANDOFF.md
git commit -m "chore: hide Logical size by default, run fork tests in CI, document fork diff"
git push -u origin feature/change-tracking
```

- [ ] **Step 7: Open the PR and deliver the local build**

Open the PR:
```bash
gh pr create --repo Demonad112/altWinDirStat --base master --head feature/change-tracking --title "Automatic change tracking, readable ages, header tooltips" --body-file <scratch file containing a summary, the manual checklist, and the line "🤖 Generated with [Claude Code](https://claude.com/claude-code)">
```
Then copy `build\altWinDirStat_x64.exe` over `C:\Users\Addy7\Projects\Work\AltWin\altWinDirStat\altWinDirStat.exe`, and confirm its FileVersion and that it launches.
