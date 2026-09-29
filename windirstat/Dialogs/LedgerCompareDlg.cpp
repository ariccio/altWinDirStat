// altWinDirStat - side-by-side folder ledger comparison (fork-owned; not part of upstream WinDirStat)
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
#include "LedgerCompareDlg.h"

namespace
{
    enum Column : int { ColFolder, ColChange, ColBaseSize, ColCurSize, ColSizeDelta, ColBaseFiles, ColCurFiles, ColFilesDelta, ColCount };

    // Numeric sort key; a missing side sorts below every real value.
    LONGLONG SortValue(const Ledger::DiffRow& row, const int column)
    {
        switch (column)
        {
        case ColChange:     return std::to_underlying(row.change);
        case ColBaseSize:   return row.before ? static_cast<LONGLONG>(row.before->size) : -1;
        case ColCurSize:    return row.after ? static_cast<LONGLONG>(row.after->size) : -1;
        case ColSizeDelta:  return row.sizeDelta;
        case ColBaseFiles:  return row.before ? static_cast<LONGLONG>(row.before->files) : -1;
        case ColCurFiles:   return row.after ? static_cast<LONGLONG>(row.after->files) : -1;
        case ColFilesDelta: return row.filesDelta;
        default:            return 0;
        }
    }
}

LedgerCompareDlg::LedgerCompareDlg(CWnd* parent, Ledger::Snapshot baseline, Ledger::Snapshot current)
    : MessageTarget(IDD_LEDGER_COMPARE, nullptr, parent)
    , m_baseline(std::move(baseline))
    , m_current(std::move(current))
{
    // The snapshots are members, so the diff rows' pointers into them stay valid for the dialog's life.
    m_rows = Ledger::Compare(m_baseline, m_current, m_summary);
}

bool LedgerCompareDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    Localization::UpdateDialogs(*this);
    DarkMode::AdjustControls(Handle());

    m_list.SubclassDlgItem(IDC_LEDGER_LIST, this);
    m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP | LVS_EX_GRIDLINES);
    const std::array columns{ IDS_LEDGER_COL_FOLDER, IDS_LEDGER_COL_CHANGE, IDS_LEDGER_COL_BASE_SIZE, IDS_LEDGER_COL_CUR_SIZE,
        IDS_LEDGER_COL_SIZE_DELTA, IDS_LEDGER_COL_BASE_FILES, IDS_LEDGER_COL_CUR_FILES, IDS_LEDGER_COL_FILES_DELTA };
    const std::array widths{ 300, 90, 90, 90, 90, 80, 80, 80 };
    for (int i = 0; i < ColCount; ++i)
        m_list.InsertColumn(i, Localization::Lookup(columns[i]), i <= ColChange ? LVCFMT_LEFT : LVCFMT_RIGHT, ScaleForDpi(widths[i]));
    m_list.SetBkColor(DarkMode::SystemColor(COLOR_WINDOW));
    m_list.SetTextBkColor(DarkMode::SystemColor(COLOR_WINDOW));
    m_list.SetTextColor(DarkMode::SystemColor(COLOR_WINDOWTEXT));

    m_layout.AddControl(IDC_LEDGER_SUMMARY, 0, 0, 1, 0);
    m_layout.AddControl(IDC_LEDGER_FILTER, 0, 0, 1, 0);
    m_layout.AddControl(IDC_LEDGER_SHOW_UNCHANGED, 1, 0, 0, 0);
    m_layout.AddControl(IDC_LEDGER_LIST, 0, 0, 1, 1);
    m_layout.AddControl(IDC_LEDGER_LEGEND, 0, 1, 1, 0);
    m_layout.AddControl(IDC_LEDGER_EXPORT, 1, 1, 0, 0);
    m_layout.AddControl(IDCANCEL, 1, 1, 0, 0);
    m_layout.OnInitDialog(true);

    UpdateSummary();
    UpdateList();
    return true;
}

void LedgerCompareDlg::UpdateSummary()
{
    const auto describe = [](const Ledger::Snapshot& snapshot)
    {
        const std::wstring source = snapshot.source.empty() ? Localization::Lookup(IDS_LEDGER_CURRENT_SCAN) : snapshot.source;
        return snapshot.root.empty() ? source : std::format(L"{}  ({})", snapshot.root, source);
    };
    const auto& counts = m_summary.counts;
    const auto count = [&](const Ledger::Change change) { return FormatCount(counts[std::to_underlying(change)]); };

    SetText(IDC_LEDGER_SUMMARY, std::format(L"{}: {}\r\n{}: {}\r\n{}",
        Localization::Lookup(IDS_LEDGER_BASELINE), describe(m_baseline),
        Localization::Lookup(IDS_LEDGER_CURRENT), describe(m_current),
        Localization::Format(IDS_LEDGER_SUMMARY_COUNTS, count(Ledger::Change::Added), count(Ledger::Change::Removed),
            count(Ledger::Change::Grown), count(Ledger::Change::Shrunk), count(Ledger::Change::FilesChanged),
            Ledger::SignedBytes(m_summary.sizeDelta), Ledger::SignedCount(m_summary.filesDelta))));

    SetText(IDC_LEDGER_LEGEND, std::format(L"{} ● {} ● {} ● {} ● {}",
        Ledger::ChangeName(Ledger::Change::Added), Ledger::ChangeName(Ledger::Change::Removed),
        Ledger::ChangeName(Ledger::Change::Grown), Ledger::ChangeName(Ledger::Change::Shrunk),
        Ledger::ChangeName(Ledger::Change::FilesChanged)));
}

void LedgerCompareDlg::UpdateList()
{
    const std::wstring filter = GetText(IDC_LEDGER_FILTER);
    const bool showUnchanged = IsChecked(IDC_LEDGER_SHOW_UNCHANGED);

    m_visible.clear();
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        const auto& row = m_rows[i];
        if (!showUnchanged && row.change == Ledger::Change::Unchanged) continue;
        if (!filter.empty() && StrStrIW(row.relative.c_str(), filter.c_str()) == nullptr) continue;
        m_visible.push_back(i);
    }

    // Folder order keeps parents above their children; other columns sort numerically.
    if (m_sortColumn != ColFolder || m_descending)
    {
        std::ranges::stable_sort(m_visible, [this](const size_t a, const size_t b)
        {
            const auto& left = m_rows[m_descending ? b : a];
            const auto& right = m_rows[m_descending ? a : b];
            if (m_sortColumn == ColFolder) return _wcsicmp(left.relative.c_str(), right.relative.c_str()) < 0;
            return SortValue(left, m_sortColumn) < SortValue(right, m_sortColumn);
        });
    }

    m_list.SetItemCountEx(static_cast<int>(m_visible.size()), LVSICF_NOSCROLL);
    m_list.Invalidate();
    GetDlgItem(IDC_LEDGER_EXPORT)->EnableWindow(!m_visible.empty());
}

std::wstring LedgerCompareDlg::CellText(const Ledger::DiffRow& row, const int column) const
{
    switch (column)
    {
    case ColFolder:     return row.relative;
    case ColChange:     return Ledger::ChangeName(row.change);
    case ColBaseSize:   return row.before ? FormatBytes(row.before->size) : std::wstring();
    case ColCurSize:    return row.after ? FormatBytes(row.after->size) : std::wstring();
    case ColSizeDelta:  return Ledger::SignedBytes(row.sizeDelta);
    case ColBaseFiles:  return row.before ? FormatCount(row.before->files) : std::wstring();
    case ColCurFiles:   return row.after ? FormatCount(row.after->files) : std::wstring();
    case ColFilesDelta: return Ledger::SignedCount(row.filesDelta);
    default:            return {};
    }
}

void LedgerCompareDlg::OnFilter()
{
    if (m_list.m_hWnd != nullptr) UpdateList();
}

void LedgerCompareDlg::OnExport()
{
    const auto path = CDialog::PickFile(CDialog::FilePickerMode::Save,
        std::format(L"CSV (*.csv)|*.csv|{} (*.*)|*.*||", Localization::Lookup(IDS_ALL_FILES)), this);
    if (!path) return;

    std::vector<Ledger::DiffRow> rows;
    rows.reserve(m_visible.size());
    for (const size_t i : m_visible) rows.push_back(m_rows[i]);
    if (!Ledger::SaveComparison(*path, rows))
        DisplayError(Localization::Format(IDS_LEDGER_SAVE_FAILED, *path));
}

void LedgerCompareDlg::OnDisplayInfo(NMHDR* header, LRESULT* result)
{
    *result = 0;
    auto& item = reinterpret_cast<NMLVDISPINFO*>(header)->item;
    if (!(item.mask & LVIF_TEXT) || item.iItem < 0 || static_cast<size_t>(item.iItem) >= m_visible.size()) return;
    // Retain callback text until the list view finishes consuming it.
    m_cell = CellText(m_rows[m_visible[item.iItem]], item.iSubItem);
    item.pszText = m_cell.data();
}

void LedgerCompareDlg::OnColumnClick(NMHDR* header, LRESULT* result)
{
    *result = 0;
    const int column = reinterpret_cast<NMLISTVIEW*>(header)->iSubItem;
    if (column < 0 || column >= ColCount) return;
    // Numeric columns default to largest first; the folder column defaults to A-Z.
    m_descending = column == m_sortColumn ? !m_descending : column != ColFolder;
    m_sortColumn = column;
    UpdateList();
}

void LedgerCompareDlg::OnCustomDraw(NMHDR* header, LRESULT* result)
{
    *result = CDRF_DODEFAULT;
    auto* draw = reinterpret_cast<NMLVCUSTOMDRAW*>(header);
    if (draw->nmcd.dwDrawStage == CDDS_PREPAINT)
    {
        *result = CDRF_NOTIFYITEMDRAW;
    }
    else if (draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT && draw->nmcd.dwItemSpec < m_visible.size())
    {
        draw->clrText = Ledger::ChangeColor(m_rows[m_visible[draw->nmcd.dwItemSpec]].change);
        if (DarkMode::IsDarkModeActive()) draw->clrTextBk = DarkMode::SystemColor(COLOR_WINDOW);
    }
}

void LedgerCompareDlg::OnDoubleClick(NMHDR* header, LRESULT* result)
{
    *result = 0;
    const int item = reinterpret_cast<NMITEMACTIVATE*>(header)->iItem;
    if (item < 0 || static_cast<size_t>(item) >= m_visible.size()) return;

    // Open the folder in Explorer when it still exists on this machine.
    const auto& row = m_rows[m_visible[item]];
    const std::wstring& path = row.after ? row.after->path : row.before->path;
    if (FolderExists(path)) ShellExecuteW(Handle(), L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}
