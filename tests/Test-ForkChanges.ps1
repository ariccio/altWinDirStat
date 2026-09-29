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
    Check (($folders -join '|') -eq 'a\b\c|edge|gone|new') 'significant rows are a\b\c, edge, gone, new' "(got '$($folders -join '|')')"
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
