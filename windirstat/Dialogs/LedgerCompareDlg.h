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

#pragma once

#include "pch.h"
#include "Layout.h"
#include "Ledger.h"
#include "ForkResource.h"

class LedgerCompareDlg final : public MessageTarget<LedgerCompareDlg, CLayoutDialog>
{
public:
    LedgerCompareDlg(CWnd* parent, Ledger::Snapshot baseline, Ledger::Snapshot current);
    static std::span<const RouteEntry> Routes();

protected:
    bool OnInitDialog() override;
    void OnOK() override {}

private:
    class ResultsList final : public CListCtrl
    {
        bool OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* result) override;
    };

    void OnFilter();
    void OnExport();
    void OnDisplayInfo(NMHDR* header, LRESULT* result);
    void OnColumnClick(NMHDR* header, LRESULT* result);
    void OnCustomDraw(NMHDR* header, LRESULT* result);
    void OnDoubleClick(NMHDR* header, LRESULT* result);
    void UpdateList();
    void UpdateSummary();
    std::wstring CellText(const Ledger::DiffRow& row, int column) const;

    ResultsList m_list;
    Ledger::Snapshot m_baseline;
    Ledger::Snapshot m_current;
    Ledger::Summary m_summary;
    std::vector<Ledger::DiffRow> m_rows;
    std::vector<size_t> m_visible;
    std::wstring m_cell;
    int m_sortColumn = 0;
    bool m_descending = false;
};

inline std::span<const RouteEntry> LedgerCompareDlg::Routes()
{
    static constexpr std::array entries
    {
        Route::Control<&OnFilter>(EN_CHANGE, IDC_LEDGER_FILTER),
        Route::Control<&OnFilter>(BN_CLICKED, IDC_LEDGER_SHOW_UNCHANGED),
        Route::Control<&OnExport>(BN_CLICKED, IDC_LEDGER_EXPORT),
        Route::Notify<&OnDisplayInfo>(LVN_GETDISPINFO, IDC_LEDGER_LIST),
        Route::Notify<&OnColumnClick>(LVN_COLUMNCLICK, IDC_LEDGER_LIST),
        Route::Notify<&OnCustomDraw>(NM_CUSTOMDRAW, IDC_LEDGER_LIST),
        Route::Notify<&OnDoubleClick>(NM_DBLCLK, IDC_LEDGER_LIST),
    };
    return entries;
}
