#Requires -Version 7.2
<#
.SYNOPSIS
    altWinDirStat large-scan stress test: the UI must never hang while scanning huge trees.

.DESCRIPTION
    1. Generates a synthetic tree (default ~700k files): a wide/deep folder grid, one flat folder with
       100k files, and a chain of folders deeper than MAX_PATH.
    2. Headless: runs "/saveto <csv> <root>" and checks every generated file is in the export (scan
       correctness at scale), and reports throughput.
    3. GUI: scans the synthetic tree (generic multithreaded finder) and then all of C:\ (NTFS MFT
       finder when elevated, as on CI runners). While each scan runs and for a while after it
       finishes, it probes the window every 250 ms with SendMessageTimeout(WM_NULL), the same kind of
       check Windows uses to mark a window "Not Responding" (5 s). It also resizes the window after
       the scan to force a full treemap re-layout of the finished tree.

    Fails if any probe takes longer than -HangThresholdMs, a scan does not finish within
    -ScanTimeoutSec, the app exits, or the export misses files. Writes a Markdown summary to
    $env:GITHUB_STEP_SUMMARY when present.
#>
param(
    [Parameter(Mandatory)] [string] $ExePath,
    [string] $WorkRoot = (Join-Path ($env:RUNNER_TEMP ?? [IO.Path]::GetTempPath()) 'awds-stress'),
    [int] $GridTop = 200,          # top-level folders
    [int] $GridSub = 30,           # subfolders per top-level folder
    [int] $GridFiles = 100,        # files per subfolder
    [int] $FlatFiles = 100000,     # files in a single folder
    [int] $DeepLevels = 120,       # nested folder depth (well past MAX_PATH)
    [int] $ScanTimeoutSec = 900,
    [int] $SettleSec = 15,         # keep probing after the scan finishes (treemap render, sorting)
    [int] $HangThresholdMs = 5000,
    [int] $StallWarnMs = 1000,
    [switch] $SkipDriveScan
)

$ErrorActionPreference = 'Stop'
$ExePath = (Resolve-Path $ExePath).Path

Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

public static class AwdsStress
{
    [DllImport("user32.dll", SetLastError = true)]
    static extern IntPtr SendMessageTimeoutW(IntPtr hWnd, uint msg, IntPtr wParam, IntPtr lParam,
        uint flags, uint timeout, out IntPtr result);
    [DllImport("user32.dll")] public static extern bool IsWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr hWnd, IntPtr after,
        int x, int y, int cx, int cy, uint flags);
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr hWnd, int cmd);

    // Milliseconds the window's thread took to answer WM_NULL; -1 if it did not answer within timeoutMs.
    public static long Probe(IntPtr hwnd, uint timeoutMs)
    {
        var sw = System.Diagnostics.Stopwatch.StartNew();
        IntPtr r;
        IntPtr ok = SendMessageTimeoutW(hwnd, 0 /*WM_NULL*/, IntPtr.Zero, IntPtr.Zero, 0 /*SMTO_NORMAL*/, timeoutMs, out r);
        return ok == IntPtr.Zero ? -1 : sw.ElapsedMilliseconds;
    }

    // Creates top*sub folders with files each, a flat folder and a deep chain; returns {files, bytes}.
    public static long[] Generate(string root, int top, int sub, int files, int flat, int deep)
    {
        long count = 0, bytes = 0;
        Parallel.For(0, top, t =>
        {
            long local = 0, localBytes = 0;
            for (int s = 0; s < sub; s++)
            {
                string dir = Path.Combine(root, "grid", "t" + t, "s" + s);
                Directory.CreateDirectory(dir);
                for (int f = 0; f < files; f++)
                {
                    string ext = (f % 7) switch { 0 => ".log", 1 => ".txt", 2 => ".bin", 3 => ".dat", 4 => ".jpg", 5 => ".dll", _ => ".tmp" };
                    using var fs = new FileStream(Path.Combine(dir, "f" + f + ext), FileMode.Create, FileAccess.Write);
                    long len = (t * 7919L + s * 104729L + f * 31L) % 8192;
                    fs.SetLength(len);
                    local++; localBytes += len;
                }
            }
            System.Threading.Interlocked.Add(ref count, local);
            System.Threading.Interlocked.Add(ref bytes, localBytes);
        });

        string flatDir = Path.Combine(root, "flat");
        Directory.CreateDirectory(flatDir);
        Parallel.For(0, flat, f =>
        {
            using var fs = new FileStream(Path.Combine(flatDir, "item" + f.ToString("D6") + ".dat"), FileMode.Create, FileAccess.Write);
            fs.SetLength(f % 4096);
        });
        count += flat;
        for (long f = 0; f < flat; f++) bytes += f % 4096;

        string deepDir = Path.Combine(root, "deep");
        for (int d = 0; d < deep; d++) deepDir = Path.Combine(deepDir, "level" + d.ToString("D3"));
        Directory.CreateDirectory(deepDir);
        File.WriteAllBytes(Path.Combine(deepDir, "bottom.txt"), new byte[1234]);
        count += 1; bytes += 1234;
        return new long[] { count, bytes };
    }
}
'@

$summary = [System.Collections.Generic.List[string]]::new()
$failures = [System.Collections.Generic.List[string]]::new()
function Note([string] $line) { Write-Host $line; $summary.Add($line) }

# --- 1. Synthetic tree ------------------------------------------------------------------------------
$tree = Join-Path $WorkRoot 'tree'
if (Test-Path $WorkRoot) { Remove-Item $WorkRoot -Recurse -Force }
New-Item -ItemType Directory -Force $tree | Out-Null
$sw = [Diagnostics.Stopwatch]::StartNew()
$gen = [AwdsStress]::Generate($tree, $GridTop, $GridSub, $GridFiles, $FlatFiles, $DeepLevels)
$fileCount, $byteCount = $gen[0], $gen[1]
Note ("Generated {0:N0} files ({1:N0} bytes) in {2:N0}s" -f $fileCount, $byteCount, $sw.Elapsed.TotalSeconds)

# --- 2. Headless export: correctness and throughput ------------------------------------------------
$csv = Join-Path $WorkRoot 'scan.csv'
$sw.Restart()
$p = Start-Process -FilePath $ExePath -ArgumentList "/saveto `"$csv`" `"$tree`"" -PassThru
if (-not $p.WaitForExit($ScanTimeoutSec * 1000)) { Stop-Process -Id $p.Id -Force; $failures.Add("Headless /saveto did not finish in $ScanTimeoutSec s") }
elseif ($p.ExitCode -ne 0) { $failures.Add("Headless /saveto exited with code $($p.ExitCode)") }
else {
    $secs = $sw.Elapsed.TotalSeconds
    # One row per item: "path",files,folders,logicalSize,... Count the generated files' rows and read
    # the root row's aggregated totals.
    $rows = 0; $fileRows = 0; $rootFiles = $null; $rootBytes = $null
    $rootPrefix = '"' + $tree.TrimEnd('\') + '"'
    foreach ($line in [IO.File]::ReadLines($csv)) {
        $rows++
        if ($line -match '\.(log|txt|bin|dat|jpg|dll|tmp)",') { $fileRows++ }
        else {
            # The root row's path may be written with or without a trailing backslash.
            $prefixLen = if ($line.StartsWith($rootPrefix + ',', [StringComparison]::OrdinalIgnoreCase)) { $rootPrefix.Length }
                elseif ($line.StartsWith($rootPrefix.TrimEnd('"') + '\",', [StringComparison]::OrdinalIgnoreCase)) { $rootPrefix.Length + 1 }
                else { 0 }
            if ($prefixLen -eq 0) { continue }
            $cols = $line.Substring($prefixLen + 1).Split(',')
            $rootFiles = [long]$cols[0]; $rootBytes = [long]$cols[2]
        }
    }
    Note ("Headless scan: {0:N0} items exported in {1:N1}s ({2:N0} items/s)" -f $rows, $secs, ($rows / [math]::Max($secs, 0.001)))
    if ($fileRows -ne $fileCount) { $failures.Add("Export has $fileRows file rows; $fileCount files were generated") }
    elseif ($null -eq $rootFiles) { $failures.Add("Export has no row for the scan root $tree") }
    elseif ($rootFiles -ne $fileCount -or $rootBytes -ne $byteCount) {
        $failures.Add("Root totals wrong: $rootFiles files / $rootBytes bytes; expected $fileCount / $byteCount") }
    else { Note ("Export lists all {0:N0} files and the root totals match exactly ({1:N0} bytes)" -f $fileCount, $byteCount) }
}

# --- 2b. Folder ledger export (altWinDirStat): "/saveto x.ledger.csv" writes folders only --------------
$ledger = Join-Path $WorkRoot 'tree.ledger.csv'
$sw.Restart()
$p = Start-Process -FilePath $ExePath -ArgumentList "/saveto `"$ledger`" `"$tree`"" -PassThru
if (-not $p.WaitForExit($ScanTimeoutSec * 1000)) { Stop-Process -Id $p.Id -Force; $failures.Add("Ledger export did not finish in $ScanTimeoutSec s") }
elseif ($p.ExitCode -ne 0) { $failures.Add("Ledger export exited with code $($p.ExitCode)") }
else {
    $secs = $sw.Elapsed.TotalSeconds
    $lines = [IO.File]::ReadAllLines($ledger)
    # root + grid + flat + deep + top-level folders + their subfolders + the deep chain
    $expectedFolders = 4 + $GridTop + $GridTop * $GridSub + $DeepLevels
    $rootRow = $lines | Where-Object { $_ -match '^"[^"]*","\.",' } | Select-Object -First 1
    $rootCols = if ($rootRow) { $rootRow.Substring($rootRow.IndexOf('".",') + 4).Split(',') } else { @() }
    if ($lines[0].TrimStart([char]0xFEFF) -ne 'Path,Relative Path,Size (bytes),Files,Subfolders') { $failures.Add("Ledger header wrong: '$($lines[0])'") }
    elseif ($lines.Count - 1 -ne $expectedFolders) { $failures.Add("Ledger has $($lines.Count - 1) folder rows; expected $expectedFolders") }
    elseif ($rootCols.Count -lt 3 -or [long]$rootCols[0] -ne $byteCount -or [long]$rootCols[1] -ne $fileCount -or [long]$rootCols[2] -ne $expectedFolders - 1) {
        $failures.Add("Ledger root row wrong: '$rootRow'; expected $byteCount bytes, $fileCount files, $($expectedFolders - 1) subfolders") }
    else { Note ("Folder ledger: {0:N0} folder rows in {1:N1}s ({2:N0} KB); root totals match" -f $expectedFolders, $secs, ((Get-Item $ledger).Length / 1KB)) }
}

# --- 3. GUI scans with hang probes -----------------------------------------------------------------
function Measure-GuiScan([string] $target, [string] $label) {
    $proc = Start-Process -FilePath $ExePath -ArgumentList "`"$target`"" -PassThru
    $sw = [Diagnostics.Stopwatch]::StartNew()
    while ($proc.MainWindowHandle -eq 0) {
        if ($proc.HasExited) { $failures.Add("[$label] app exited before showing a window (0x$('{0:X8}' -f $proc.ExitCode))"); return }
        if ($sw.Elapsed.TotalSeconds -gt 60) { $failures.Add("[$label] no main window after 60 s"); Stop-Process -Id $proc.Id -Force; return }
        Start-Sleep -Milliseconds 200; $proc.Refresh()
    }
    $hwnd = $proc.MainWindowHandle

    $lat = [System.Collections.Generic.List[long]]::new()
    $stalls = 0; $hangs = 0; $worst = 0L
    $sawProgress = $false; $doneAt = $null; $resized = $false
    while ($true) {
        if ($proc.HasExited) { $failures.Add("[$label] app exited during the scan (0x$('{0:X8}' -f $proc.ExitCode))"); return }
        $ms = [AwdsStress]::Probe($hwnd, [uint32]($HangThresholdMs * 2))
        if ($ms -lt 0) { $ms = $HangThresholdMs * 2 }
        $lat.Add($ms); $worst = [math]::Max($worst, $ms)
        if ($ms -ge $StallWarnMs) { $stalls++ }
        if ($ms -ge $HangThresholdMs) { $hangs++ }

        # While a scan runs the title reads "altWinDirStat <ver> [(Administrator)] - NN% <target>" (or the
        # localized "Scanning" text before a percentage is known); the prefix is dropped when it finishes.
        # GetWindowText on another process's window reads the cached caption and never blocks on it.
        $proc.Refresh()
        $title = $proc.MainWindowTitle
        $scanning = $title -match ' - \d+% ' -or $title -match ' - Scanning'
        if ($scanning) { $sawProgress = $true; $doneAt = $null }
        elseif ($null -eq $doneAt -and ($sawProgress -or $sw.Elapsed.TotalSeconds -gt 30)) {
            if (-not $sawProgress) { $failures.Add("[$label] never saw a scan-progress title (last: '$title')") }
            $doneAt = $sw.Elapsed.TotalSeconds
            Write-Host ("[{0}] scan finished after {1:N1}s; title '{2}'" -f $label, $doneAt, $title)
        }
        if ($null -ne $doneAt) {
            if (-not $resized -and $sw.Elapsed.TotalSeconds -gt $doneAt + 3) {
                # Force a full re-layout of the finished tree: restore, resize, maximize.
                [AwdsStress]::ShowWindow($hwnd, 9) | Out-Null
                [AwdsStress]::SetWindowPos($hwnd, [IntPtr]::Zero, 20, 20, 900, 700, 0x0014) | Out-Null
                [AwdsStress]::ShowWindow($hwnd, 3) | Out-Null
                $resized = $true
            }
            if ($sw.Elapsed.TotalSeconds -gt $doneAt + $SettleSec) { break }
        }
        if ($sw.Elapsed.TotalSeconds -gt $ScanTimeoutSec) { $failures.Add("[$label] scan did not finish within $ScanTimeoutSec s (title '$title')"); break }
        Start-Sleep -Milliseconds 250
    }

    $ws = [math]::Round($proc.PeakWorkingSet64 / 1MB)
    Stop-Process -Id $proc.Id -Force -ErrorAction SilentlyContinue
    $sorted = $lat | Sort-Object
    $p99 = $sorted[[math]::Min($sorted.Count - 1, [int][math]::Floor($sorted.Count * 0.99))]
    Note ("[{0}] scan {1:N1}s | UI probes {2:N0} | worst {3:N0} ms | p99 {4:N0} ms | stalls >= {5} ms: {6} | peak memory {7:N0} MB" -f `
        $label, $doneAt, $lat.Count, $worst, $p99, $StallWarnMs, $stalls, $ws)
    if ($hangs -gt 0) { $failures.Add("[$label] UI did not respond for >= $HangThresholdMs ms on $hangs probe(s); worst $worst ms") }
}

Measure-GuiScan $tree 'synthetic-tree'
if (-not $SkipDriveScan) { Measure-GuiScan 'C:\' 'drive-C' }

Remove-Item $WorkRoot -Recurse -Force -ErrorAction SilentlyContinue

# --- Report ----------------------------------------------------------------------------------------
if ($env:GITHUB_STEP_SUMMARY) {
    $md = @('### Large-scan stress test', '') + ($summary | ForEach-Object { "- $_" })
    if ($failures.Count) { $md += @('', '**Failures:**') + ($failures | ForEach-Object { "- $_" }) }
    $md | Out-File $env:GITHUB_STEP_SUMMARY -Append -Encoding utf8
}
if ($failures.Count) {
    $failures | ForEach-Object { Write-Host "::error::$_" }
    exit 1
}
Write-Host 'Stress test passed.'
