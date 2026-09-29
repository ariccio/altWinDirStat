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
