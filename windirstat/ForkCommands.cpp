// altWinDirStat - fork-only menu commands (fork-owned; not part of upstream WinDirStat)
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
#include "LedgerCompareDlg.h"
#include "ProgressDlg.h"

namespace
{
    std::wstring LedgerFilter()
    {
        return std::format(L"{} (*.ledger.csv)|*.ledger.csv|CSV (*.csv)|*.csv|{} (*.*)|*.*||",
            Localization::Lookup(IDS_LEDGER_FILE_FILTER), Localization::Lookup(IDS_ALL_FILES));
    }

    // Loads a ledger behind a progress dialog; reports errors itself.
    std::optional<Ledger::Snapshot> LoadWithProgress(const std::wstring& path)
    {
        std::optional<Ledger::Snapshot> snapshot;
        std::wstring error;
        CProgressDlg(0, CProgressDlg::Flags::NoCancel, GetMainWindow(), [&](CProgressDlg*)
        {
            snapshot = Ledger::Load(path, error);
        }).ShowModal();
        if (!snapshot) DisplayError(Localization::Format(IDS_LEDGER_INVALID, path, error));
        return snapshot;
    }
}

void CWinDirStatModel::OnUpdateFolderLedgerExport(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(HasRootItem() && IsScanSettled());
}

void CWinDirStatModel::OnFolderLedgerExport()
{
    if (!HasRootItem() || !IsScanSettled()) return;
    const auto path = CDialog::PickFile(CDialog::FilePickerMode::Save, LedgerFilter());
    if (!path) return;

    bool saved = false;
    CProgressDlg(0, CProgressDlg::Flags::NoCancel, GetMainWindow(), [&](CProgressDlg*)
    {
        saved = Ledger::Save(*path, Ledger::FromScan(GetRootItem()));
    }).ShowModal();
    if (!saved) DisplayError(Localization::Format(IDS_LEDGER_SAVE_FAILED, *path));
}

void CWinDirStatModel::OnFolderLedgerCompare()
{
    const auto baselinePath = CDialog::PickFile(CDialog::FilePickerMode::Open, LedgerFilter());
    if (!baselinePath) return;
    auto baseline = LoadWithProgress(*baselinePath);
    if (!baseline) return;

    // Compare against the finished scan in the window, or else against a second ledger.
    std::optional<Ledger::Snapshot> current;
    if (HasRootItem() && IsScanSettled())
    {
        CProgressDlg(0, CProgressDlg::Flags::NoCancel, GetMainWindow(), [&](CProgressDlg*)
        {
            current = Ledger::FromScan(GetRootItem());
        }).ShowModal();
    }
    else
    {
        if (CMessageBoxDlg::Show(Localization::Lookup(IDS_LEDGER_PICK_CURRENT), MB_OKCANCEL | MB_ICONINFORMATION) != IDOK) return;
        const auto currentPath = CDialog::PickFile(CDialog::FilePickerMode::Open, LedgerFilter());
        if (!currentPath) return;
        current = LoadWithProgress(*currentPath);
        if (!current) return;
    }

    LedgerCompareDlg(GetMainWindow(), std::move(*baseline), std::move(*current)).ShowModal();
}
