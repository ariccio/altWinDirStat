// altWinDirStat - fork-only headless command lines (fork-owned; not part of upstream WinDirStat)
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
