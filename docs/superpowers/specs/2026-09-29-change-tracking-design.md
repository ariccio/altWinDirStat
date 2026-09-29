# Automatic change tracking and view polish: design

Status: approved in conversation 2026-09-29, pending written-spec review.

## Goal

The main user is the owner, running altWinDirStat on their own PCs. Two things should get easier:

1. **See what changed.** Rescan a drive or folder and immediately see what grew, shrank, appeared or disappeared since last time. There should be no ledger files to export, pick or manage.
2. **Clearer views.** Plain-language ages and tooltips, and slightly less column clutter.

**Success looks like this:** open altWinDirStat, scan C:, click **Changes**, and read something like "Since 22 Sep 2026 (7 days ago): +14.2 GB". Below it is a short list headed by `Downloads +12 GB`, with no parent-folder noise.

## Scope

In scope:
1. The Changes tab, backed by automatic snapshots (the main feature).
2. A readable age for Last Change, with muted colouring for stale folders.
3. Plain-language tooltips on column headers and tabs.
4. Hiding the Logical size column by default, for new settings only.
5. A headless `/compare` CLI, which is also the test seam for the change logic.

Out of scope:
- a growth column in the All Files tree
- picking an arbitrary earlier snapshot to compare against
- a tree view of changes
- cleanup automation
- translations beyond English (other languages fall back to English, as the ledger strings already do)

## Constraints

- **Keep the fork diff small.** New logic goes in fork-owned files. Upstream files get one-line hooks, or appended blocks where unavoidable, and every upstream touch is added to the table in `HANDOFF.md`.
- **Never block the UI thread** with snapshot I/O or diffing on large trees (1M+ folders).
- Settings stay under `HKCU\Software\altWinDirStat`.

## 1. Changes tab

### 1.1 Snapshot store (`windirstat/History.h/.cpp`, fork-owned)

- **Location:** `%LOCALAPPDATA%\altWinDirStat\History\`. The `ALTWDS_HISTORY_DIR` environment variable overrides it for tests.
- **Per-location folder:** the folder name is a hex hash (FNV-1a 64) of the *location key*. The location key is the lower-cased scan root path. For a multi-drive scan (`IT_MYCOMPUTER`), it is the sorted, `|`-joined list of drive roots. Each folder holds a `location.txt` with the readable key, for debugging.
- **Snapshot files:** named `YYYYMMDD-HHMMSS.ledger.csv` in UTC, in the existing ledger format, written through `Ledger::Save`.
- **Retention:** keep the newest **5** snapshots per location and delete older ones after each write. A full C: is roughly 10–15 MB per snapshot.
- **Atomic writes:** write to `*.tmp`, then `MoveFileEx(..., MOVEFILE_REPLACE_EXISTING)`. Load ignores `*.tmp` files.

### 1.2 Baseline and session semantics

- **Baseline:** when the scan root changes (a new scan, or a different folder or drive), the baseline is the newest snapshot for that location on disk that this process did not write. The process keeps an in-memory set of files it has written. So switching to D: and back to C: in one session still compares C: with the previous session, not with itself. The baseline is resolved once per location and cached for the session.
- **Session snapshot:** each time a scan completes, the current tree is written as this session's snapshot for the location. The file is created on the first completion and overwritten on later refreshes in the same session. A session therefore leaves one snapshot per location, its final state.
- **Refreshes:** F5 and partial refreshes compare against the *same* baseline, so a refresh never replaces "since last time" with "since 30 seconds ago".
- **First scan of a location:** there is no baseline, so the tab stays hidden and only the snapshot is written.
- **Skipped entirely:** headless `/saveto`, `/savedupesto` and `/savepermsto` (they `ExitProcess` before the hook), results loaded from a CSV file, and runs with tracking turned off.

### 1.3 Change list (`Ledger` additions, fork-owned)

- **Base diff:** `Ledger::Compare` as it exists today.
- **New function:** `Ledger::Significant(const std::vector<DiffRow>&, ULONGLONG minOwnDelta) -> std::vector<size_t>` returns the indices of rows to show:
  - **Added or Removed:** a row is listed only if its parent's row is *not* Added or Removed, i.e. the topmost new or removed folder.
  - **Grown or Shrunk:** a row is listed if |own delta| ≥ `minOwnDelta`. Own delta = the folder's `sizeDelta` minus the sum of its direct child rows' `sizeDelta`. Added and Removed children count at their full size.
  - **FilesChanged and Unchanged:** never listed in significant mode.
- **Threshold:** `minOwnDelta` defaults to 1 MiB (a fork setting, not exposed in the UI yet).
- **Show all:** a "Show all changed folders" checkbox switches to every row except Unchanged.
- **Complexity:** O(n) over rows sorted by relative path, using a parent lookup by relative-path prefix via a hash map from relative path to index.

### 1.4 Threading

- **Hook:** one line in `WinDirStatModel.Actions.cpp`, after `CItem::ScanItemsFinalize` and `RebuildExtensionData()`, on the scan worker thread and before the UI-thread block. It calls `History::OnScanComplete(GetRootItem())`.
- **What it does, all on the worker:**
  1. `Ledger::FromScan`
  2. load the baseline if not yet cached
  3. `Ledger::Compare`
  4. `Significant`
  5. write the session snapshot
- **Hand-off to the UI:** the result goes to the Changes tab through `CMainFrame::InvokeInMessageThread`, as a `std::shared_ptr` to an immutable result struct holding both snapshots, the rows, the summary and the baseline time.
- **Failures:** any exception or I/O failure is traced with `VTRACE` and swallowed. Change tracking never breaks a scan.

### 1.5 UI: Changes pane (`windirstat/Views/FileChangesView.h/.cpp` and `windirstat/Controls/FileChangesControl.h/.cpp`, fork-owned)

- **Where it sits:** a new tab in `CFileTabbedView`, added after Largest Files through appended or one-line hooks in `FileTabbedView.h/.cpp`. It is hidden until a result with a baseline arrives, and hidden again on a new root without a baseline.
- **Header line:** "Since {date} ({N days ago}): {+/-size}, {n} new, {n} removed, {n} grew, {n} shrank". A "Show all changed folders" checkbox sits on the right.
- **List:** an owner-data list control. The columns and custom-draw colours are reused from `LedgerCompareDlg`; the shared cell-text and colour helpers move into `Ledger`.
  - Columns: Folder (relative path), Change, Before, Now, Difference, Files ±.
  - Default sort is by |Difference| descending. Clicking a column header sorts by that column.
- **Double-click:** on a folder that still exists, select and reveal it in the All Files tree (find the `CItem` by path from the root). On a Removed folder, do nothing.
- **Visibility and toggle:** a View menu toggle, "Track changes between scans", backed by `COptions`-style fork setting `TrackChanges` (default on). Turning it off hides the tab and stops snapshot writes. Existing history is not deleted.

## 2. Readable age

- **New fork setting:** `ShowRelativeAge`, default on. Its toggle lives in File Tree options, as a fork-owned checkbox appended to the page.
- **Display:** when the setting is on, the Last Change cell renders as `{relative} ({date})`, for example `3 yrs ago (2023-04-01)`.
  - Relative units: minutes, hours, days, months and years.
  - The formatting lives in fork-owned `ForkFormat.h/.cpp`.
- **Muted rows:** folders whose Last Change is over 365 days old draw that cell in the list's muted or disabled text colour.
- **Upstream hooks:** one-line hooks in `Item.Extended.cpp` (text) and `FileTreeControl.cpp` (colour). This is the only feature that edits upstream drawing paths; both hooks are listed in HANDOFF.
- **Sorting is unchanged:** it stays by timestamp.

## 3. Tooltips

- **Column headers:** hovering a header in the All Files, Largest Files and Changes lists shows a one-line explanation. Examples:
  - Physical size: "Space actually used on disk (after compression, sparse files, cloud placeholders)."
  - Size (logical): "The file's real length in bytes."
  - Items, Files, Folders, Last Change, Attributes and Owner get similar lines.
- **Tabs:** tab tooltips on All Files, Largest Files, Duplicate Files, Search Results, Watcher, Permissions, Storage Analytics and Changes.
- **Strings:** all in `windirstat/res/fork/lang_en.txt` (`IDS_TIP_*`).
- **Mechanism:** a fork-owned helper `ForkTooltips::Attach(CHeaderCtrl&, columnIdLookup)` and `ForkTooltips::AttachTabs(CTabControl&)`, called from one-line hooks. The implementation plan must confirm that upstream's header and tab controls allow a standard `TOOLTIPS_CLASS` subclass without conflict. If the tab control doesn't, tab tooltips are dropped; header tooltips are the priority.

## 4. Default layout

- **Change:** in `Options.cpp`, the default `FileTreeColumnVisibility` vector sets `COL_SIZE_LOGICAL` to 0.
- **Effect:** this applies only when the setting is empty (a fresh profile). Existing users' column choices are untouched.

## 5. `/compare` CLI (test seam, also generally useful)

- **Usage:** `altWinDirStat.exe /compare <baseline.ledger.csv> <current.ledger.csv> <out.csv> [/all]`
- **Behaviour:** writes the significant change list, or every changed row with `/all`, using `Ledger::SaveComparison`, then exits 0. Exit code 1 means a load or save failure.
- **No window:** no scan and no UI; it is handled in the command-line parsing path next to `/saveto`.

## Error handling summary

- **History folder can't be created or written:** trace it and carry on. The tab shows no error; the feature simply doesn't activate.
- **Corrupt or unreadable baseline:** skip it and try the next newest. If there are none, act as a first scan.
- **Root path from a different machine** (for example, a synced `%LOCALAPPDATA%`): harmless. The keys differ, so nothing matches.

## Testing

1. **`tests/Test-ForkChanges.ps1`** (new, fork-owned), run in CI next to the upstream suites:
   - **`/compare` cases:** hand-written ledger CSV fixtures cover pass-through suppression, the topmost Added and Removed, the threshold boundary (just under and at 1 MiB), `/all`, and a malformed input giving exit code 1.
   - **Snapshot flow:** with `ALTWDS_HISTORY_DIR` set to a temp directory:
     1. GUI-scan a generated folder and close; expect one snapshot and a readable `location.txt`.
     2. Add a 5 MB file in a nested folder and scan again; expect two snapshots.
     3. `/compare` the two snapshots; the nested folder is listed and its parents aren't.
     4. Scan 6 more times; expect retention to keep 5.
2. **Existing suites:** CI's smoke, upstream suites and stress test must stay green. The stress run's UI-hang probe covers the worker-thread diff on a ~6k-folder tree and on all of C:.
3. **Manual:** the local x64 build, with a click-through checklist added to HANDOFF:
   - the tab appears on the second scan
   - the header text is right
   - sorting works
   - "show all" works
   - double-click reveals the folder
   - the toggle hides the tab
   - relative ages and muted colours show
   - tooltips show
   - dark mode looks right

## Deliverables

- A PR from `feature/change-tracking` into master, with the `HANDOFF.md` fork-diff table updated.
- A local rebuilt `altWinDirStat.exe` in the owner's `AltWin\altWinDirStat\` folder.
