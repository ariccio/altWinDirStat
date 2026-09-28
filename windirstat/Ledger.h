// altWinDirStat - folder ledger export and baseline comparison (fork-owned; not part of upstream WinDirStat)
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

class CItem;

// A folder ledger is a small CSV snapshot of a scan: one row per folder (no per-file rows) with the
// folder's total logical size, file count and subfolder count. Two ledgers, or a ledger and the
// current scan, can be compared to see which folders were added, removed, grew or shrank.
namespace Ledger
{
    struct Row
    {
        std::wstring path;          // Absolute path as scanned (display only)
        std::wstring relative;      // Path relative to the scan root; "." for the root itself
        ULONGLONG size = 0;         // Total logical size of everything below the folder
        ULONGLONG files = 0;        // Total files below the folder
        ULONGLONG folders = 0;      // Total subfolders below the folder
    };

    struct Snapshot
    {
        std::wstring source;        // Ledger file path, or empty for the live scan
        std::wstring root;          // Absolute path of the scan root
        std::vector<Row> rows;      // Sorted by relative path; rows[0] is the root when present
    };

    enum class Change : std::uint8_t { Added, Removed, Grown, Shrunk, FilesChanged, Unchanged };

    struct DiffRow
    {
        std::wstring relative;
        const Row* before = nullptr; // Baseline row, or nullptr if the folder is new
        const Row* after = nullptr;  // Current row, or nullptr if the folder was removed
        Change change = Change::Unchanged;
        LONGLONG sizeDelta = 0;
        LONGLONG filesDelta = 0;
    };

    struct Summary
    {
        std::array<size_t, 6> counts{};  // Indexed by Change
        LONGLONG sizeDelta = 0;          // Root-to-root change
        LONGLONG filesDelta = 0;
    };

    // "*.ledger.csv" paths passed to Save Results or /saveto are written as ledgers.
    bool IsLedgerPath(const std::wstring& path);

    Snapshot FromScan(const CItem* root);
    bool Save(const std::wstring& path, const Snapshot& snapshot);
    std::optional<Snapshot> Load(const std::wstring& path, std::wstring& error);

    // If one root contains the other, the broader snapshot is narrowed in place to the common root.
    // Rows point into both snapshots, which must outlive (and not move under) the result.
    std::vector<DiffRow> Compare(Snapshot& baseline, Snapshot& current, Summary& summary);
    bool SaveComparison(const std::wstring& path, const std::vector<DiffRow>& rows);
    std::wstring ChangeName(Change change);
}
