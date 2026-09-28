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

#include "pch.h"
#include "Ledger.h"
#include "Item.h"

namespace
{
    // Fixed English header: ledgers stay comparable whatever UI language wrote them.
    constexpr std::wstring_view ledgerHeader = L"Path,Relative Path,Size (bytes),Files,Subfolders";
    constexpr std::wstring_view ledgerSuffix = L".ledger.csv";
    constexpr std::wstring_view rootRelative = L".";

    std::wstring Key(std::wstring_view text)
    {
        std::wstring key(text);
        if (!key.empty()) CharLowerBuffW(key.data(), static_cast<DWORD>(key.size()));
        return key;
    }

    std::wstring TrimSlashes(std::wstring path)
    {
        while (!path.empty() && path.back() == L'\\') path.pop_back();
        return path;
    }

    // Relative path of 'path' below 'root' (both absolute), "." for the root, or nullopt if outside.
    std::optional<std::wstring> RelativeTo(const std::wstring& root, const std::wstring& path)
    {
        if (root.empty()) return TrimSlashes(path);
        const std::wstring base = TrimSlashes(root);
        const std::wstring full = TrimSlashes(path);
        if (_wcsicmp(base.c_str(), full.c_str()) == 0) return std::wstring(rootRelative);
        if (full.size() <= base.size() + 1 || _wcsnicmp(base.c_str(), full.c_str(), base.size()) != 0 ||
            full[base.size()] != L'\\') return std::nullopt;
        return full.substr(base.size() + 1);
    }

    bool IsSameOrBelow(const std::wstring& ancestor, const std::wstring& path)
    {
        return !ancestor.empty() && !path.empty() && RelativeTo(ancestor, path).has_value();
    }

    void SortRows(std::vector<Ledger::Row>& rows)
    {
        std::ranges::sort(rows, [](const Ledger::Row& a, const Ledger::Row& b)
        {
            if (a.relative == rootRelative || b.relative == rootRelative) return a.relative == rootRelative && b.relative != rootRelative;
            return _wcsicmp(a.relative.c_str(), b.relative.c_str()) < 0;
        });
    }

    std::string ToUtf8(const std::wstring_view text)
    {
        if (text.empty()) return {};
        const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
        std::string out(static_cast<size_t>(size), '\0');
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size, nullptr, nullptr);
        return out;
    }

    std::wstring FromUtf8(const std::string_view text)
    {
        if (text.empty()) return {};
        const int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
        std::wstring out(static_cast<size_t>(size), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), size);
        return out;
    }

    std::wstring Quote(const std::wstring_view text)
    {
        std::wstring out = L"\"";
        for (const wchar_t c : text)
        {
            if (c == L'"') out += L'"';
            out += c;
        }
        return out + L'"';
    }

    // Splits one CSV line; handles quoted fields and doubled quotes.
    std::vector<std::wstring> SplitCsv(const std::wstring_view line)
    {
        std::vector<std::wstring> fields(1);
        bool quoted = false;
        for (size_t i = 0; i < line.size(); ++i)
        {
            const wchar_t c = line[i];
            if (quoted)
            {
                if (c == L'"' && i + 1 < line.size() && line[i + 1] == L'"') { fields.back() += L'"'; ++i; }
                else if (c == L'"') quoted = false;
                else fields.back() += c;
            }
            else if (c == L'"') quoted = true;
            else if (c == L',') fields.emplace_back();
            else fields.back() += c;
        }
        return fields;
    }

    bool ParseNumber(const std::wstring& text, ULONGLONG& value)
    {
        if (text.empty() || !std::ranges::all_of(text, [](const wchar_t c) { return c >= L'0' && c <= L'9'; })) return false;
        try { value = std::stoull(text); }
        catch (const std::out_of_range&) { return false; }
        return true;
    }

    bool WriteFile(const std::wstring& path, const std::wstring& text)
    {
        std::ofstream out(std::filesystem::path(path), std::ios::binary | std::ios::trunc);
        if (!out.is_open()) return false;
        out << "\xEF\xBB\xBF" << ToUtf8(text); // BOM so Excel opens non-ASCII paths correctly
        out.flush();
        return out.good();
    }

    // Keeps only rows at or below 'root' and re-expresses them relative to it.
    Ledger::Snapshot Rebase(const Ledger::Snapshot& snapshot, const std::wstring& root)
    {
        Ledger::Snapshot rebased{ .source = snapshot.source, .root = root };
        for (const auto& row : snapshot.rows)
        {
            if (auto relative = RelativeTo(root, row.path); relative)
                rebased.rows.push_back({ .path = row.path, .relative = std::move(*relative),
                    .size = row.size, .files = row.files, .folders = row.folders });
        }
        SortRows(rebased.rows);
        return rebased;
    }
}

bool Ledger::IsLedgerPath(const std::wstring& path)
{
    return path.size() > ledgerSuffix.size() &&
        _wcsicmp(path.c_str() + path.size() - ledgerSuffix.size(), ledgerSuffix.data()) == 0;
}

Ledger::Snapshot Ledger::FromScan(const CItem* root)
{
    Snapshot snapshot;
    if (root == nullptr) return snapshot;

    // A multi-drive scan has a synthetic "My Computer" root: keep absolute paths as the key.
    const bool multiRoot = root->GetItemType() == IT_MYCOMPUTER;
    snapshot.root = multiRoot ? std::wstring() : root->GetPath();

    for (std::vector stack{ root }; !stack.empty();)
    {
        const CItem* item = stack.back();
        stack.pop_back();

        const ITEMTYPE type = item->GetItemType();
        if (type != IT_MYCOMPUTER && type != IT_DRIVE && type != IT_DIRECTORY) continue;

        Row row{ .size = item->GetSizeLogical(), .files = item->GetFilesCount(), .folders = item->GetFoldersCount() };
        if (item == root)
        {
            row.path = multiRoot ? item->GetName() : item->GetPath();
            row.relative = rootRelative;
        }
        else
        {
            row.path = item->GetPath();
            row.relative = RelativeTo(snapshot.root, row.path).value_or(TrimSlashes(row.path));
        }
        snapshot.rows.push_back(std::move(row));

        if (item->HasChildren())
            for (const CItem* child : item->GetChildren()) stack.push_back(child);
    }

    SortRows(snapshot.rows);
    return snapshot;
}

bool Ledger::Save(const std::wstring& path, const Snapshot& snapshot)
{
    std::wstring text(ledgerHeader);
    for (const auto& row : snapshot.rows)
        text += std::format(L"\r\n{},{},{},{},{}", Quote(row.path), Quote(row.relative), row.size, row.files, row.folders);
    return WriteFile(path, text);
}

std::optional<Ledger::Snapshot> Ledger::Load(const std::wstring& path, std::wstring& error)
{
    std::ifstream in(std::filesystem::path(path), std::ios::binary);
    if (!in.is_open())
    {
        error = TranslateError();
        return std::nullopt;
    }
    std::string bytes((std::istreambuf_iterator(in)), std::istreambuf_iterator<char>());
    if (bytes.starts_with("\xEF\xBB\xBF")) bytes.erase(0, 3);
    const std::wstring text = FromUtf8(bytes);

    Snapshot snapshot{ .source = path };
    std::wistringstream lines(text);
    std::wstring line;
    bool sawHeader = false;
    size_t lineNumber = 0;
    while (std::getline(lines, line))
    {
        ++lineNumber;
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (line.empty()) continue;
        if (!sawHeader)
        {
            if (line != ledgerHeader)
            {
                error = L"Unrecognized header (expected \"" + std::wstring(ledgerHeader) + L"\")";
                return std::nullopt;
            }
            sawHeader = true;
            continue;
        }

        auto fields = SplitCsv(line);
        Row row;
        if (fields.size() != 5 || fields[1].empty() || !ParseNumber(fields[2], row.size) ||
            !ParseNumber(fields[3], row.files) || !ParseNumber(fields[4], row.folders))
        {
            error = std::format(L"Malformed row on line {}", lineNumber);
            return std::nullopt;
        }
        row.path = std::move(fields[0]);
        row.relative = std::move(fields[1]);
        if (row.relative == rootRelative) snapshot.root = row.path;
        snapshot.rows.push_back(std::move(row));
    }

    if (!sawHeader)
    {
        error = L"The file is empty";
        return std::nullopt;
    }
    SortRows(snapshot.rows);
    return snapshot;
}

std::vector<Ledger::DiffRow> Ledger::Compare(Snapshot& baseline, Snapshot& current, Summary& summary)
{
    // If one side scanned a parent of the other's root (e.g. baseline of C:\Data, current scan of C:\),
    // narrow the broader side to the common root so folders line up. Different, unrelated roots
    // (e.g. D:\Data vs E:\Backup\Data) are matched by relative path, which compares copies.
    if (!baseline.root.empty() && !current.root.empty() &&
        _wcsicmp(TrimSlashes(baseline.root).c_str(), TrimSlashes(current.root).c_str()) != 0)
    {
        if (IsSameOrBelow(current.root, baseline.root)) current = Rebase(current, baseline.root);
        else if (IsSameOrBelow(baseline.root, current.root)) baseline = Rebase(baseline, current.root);
    }

    std::unordered_map<std::wstring, const Row*> currentByKey;
    currentByKey.reserve(current.rows.size());
    for (const auto& row : current.rows) currentByKey.emplace(Key(row.relative), &row);

    std::vector<DiffRow> diff;
    diff.reserve(std::max(baseline.rows.size(), current.rows.size()));
    for (const auto& before : baseline.rows)
    {
        DiffRow row{ .relative = before.relative, .before = &before };
        if (const auto it = currentByKey.find(Key(before.relative)); it != currentByKey.end())
        {
            row.after = it->second;
            currentByKey.erase(it);
        }
        diff.push_back(std::move(row));
    }
    for (const auto& after : current.rows)
        if (currentByKey.contains(Key(after.relative)))
            diff.push_back({ .relative = after.relative, .after = &after });

    summary = {};
    for (auto& row : diff)
    {
        const ULONGLONG beforeSize = row.before ? row.before->size : 0, afterSize = row.after ? row.after->size : 0;
        const ULONGLONG beforeFiles = row.before ? row.before->files : 0, afterFiles = row.after ? row.after->files : 0;
        row.sizeDelta = static_cast<LONGLONG>(afterSize) - static_cast<LONGLONG>(beforeSize);
        row.filesDelta = static_cast<LONGLONG>(afterFiles) - static_cast<LONGLONG>(beforeFiles);
        if (!row.before) row.change = Change::Added;
        else if (!row.after) row.change = Change::Removed;
        else if (row.sizeDelta > 0) row.change = Change::Grown;
        else if (row.sizeDelta < 0) row.change = Change::Shrunk;
        else if (row.filesDelta != 0 || row.before->folders != row.after->folders) row.change = Change::FilesChanged;
        else row.change = Change::Unchanged;

        ++summary.counts[std::to_underlying(row.change)];
        if (row.relative == rootRelative)
        {
            summary.sizeDelta = row.sizeDelta;
            summary.filesDelta = row.filesDelta;
        }
    }

    std::ranges::sort(diff, [](const DiffRow& a, const DiffRow& b)
    {
        if (a.relative == rootRelative || b.relative == rootRelative) return a.relative == rootRelative && b.relative != rootRelative;
        return _wcsicmp(a.relative.c_str(), b.relative.c_str()) < 0;
    });
    return diff;
}

std::wstring Ledger::ChangeName(const Change change)
{
    switch (change)
    {
    case Change::Added:        return Localization::Lookup(IDS_LEDGER_ADDED);
    case Change::Removed:      return Localization::Lookup(IDS_LEDGER_REMOVED);
    case Change::Grown:        return Localization::Lookup(IDS_LEDGER_GROWN);
    case Change::Shrunk:       return Localization::Lookup(IDS_LEDGER_SHRUNK);
    case Change::FilesChanged: return Localization::Lookup(IDS_LEDGER_FILES_CHANGED);
    default:                   return Localization::Lookup(IDS_LEDGER_UNCHANGED);
    }
}

bool Ledger::SaveComparison(const std::wstring& path, const std::vector<DiffRow>& rows)
{
    std::wstring text = L"Folder,Change,Baseline Size (bytes),Current Size (bytes),Size Change (bytes),"
        L"Baseline Files,Current Files,Files Change";
    const auto value = [](const Row* row, ULONGLONG Row::* field) { return row ? std::to_wstring(row->*field) : std::wstring(); };
    for (const auto& row : rows)
    {
        text += std::format(L"\r\n{},{},{},{},{},{},{},{}", Quote(row.relative), Quote(ChangeName(row.change)),
            value(row.before, &Row::size), value(row.after, &Row::size), row.sizeDelta,
            value(row.before, &Row::files), value(row.after, &Row::files), row.filesDelta);
    }
    return WriteFile(path, text);
}
