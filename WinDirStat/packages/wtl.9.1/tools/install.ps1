param($installPath, $toolsPath, $package, $project)

$ErrorActionPreference = "Stop";

try {
    # Avoid race conditions when checking the file.

    $mutex = New-Object System.Threading.Mutex($false, "Global¥WTL_AppWizInstaller");
    $mutex.WaitOne() | Out-Null;

    # Determine the VS version and edition.

    $vsVersion = "";
    $vsEdition = "";
    try {
        $dte2 = Get-Interface $dte ([EnvDTE80.DTE2]);
        $vsVersion = $dte2."Version";	# "10.0", "11.0" or "12.0"
        $vsEdition = $dte2."Edition";	# "Professional", "Desktop Express" etc.
    }
    catch [Exception] {
    }
    if ($vsVersion -eq "" -or $vsEdition -eq "") {
        echo "install.ps1: Failed to determine the VS version.";
        exit;
    }

    # Determine where VC is installed.

    $regPath;
    $vszDir;
    $jsParam;
    if ($vsEdition.Contains("Express")) {
        $regPath = "Microsoft\VCExpress\" + $vsVersion + "\Setup\VC"
        $vszDir  = "vcprojects_WDExpress";
        $jsParam = "/ver:" + $vsVersion.Substring(0, 2) + "E";
    }
    else {
        $regPath = "Microsoft\VisualStudio\" + $vsVersion + "\Setup\VC"
        $vszDir  = "vcprojects";
        $jsParam = "/ver:" + $vsVersion.Substring(0, 2);
    }

    $jsParam += " /copyfiles";

    $vcDir = "";
    try {
        $regItem = Get-ItemProperty ("HKLM:Software\" + $regPath);
        $vcDir = $regItem."ProductDir";
    }
    catch [Exception] {
        try {
            $regItem = Get-ItemProperty ("HKLM:Software\Wow6432Node\" + $regPath);
            $vcDir = $regItem."ProductDir";
        }
        catch [Exception] {
        }
    }
    if ($vcDir -eq "") {
        echo "install.ps1: Failed to determine where VC is installed.";
        exit;
    }

    # Check if the AppWizard is already installed.

    $vszFile = $vcDir + $vszDir + "\WTLAppWiz.vsz";
    if (Test-Path $vszFile) {
        echo "install.ps1: AppWizard is already installed.";
        exit;
    }

    # Ask to install the AppWizard.

    $verTitle;
    if ($vsVersion -eq "10.0") {
        $verTitle = "2010";
    }
    elseif ($vsVersion -eq "11.0") {
        $verTitle = "2012";
    }
    elseif ($vsVersion -eq "12.0") {
        $verTitle = "2013";
    }
    elseif ($vsVersion -eq "14.0") {
        $verTitle = "2015";
    }
    else {
        echo "install.ps1: Unsupported Visual Studio version.";
        exit;
    }
    if ($vsEdition.Contains("Express")) {
        $verTitle += " Express";
    }

    [Void][Reflection.Assembly]::LoadWithPartialName("System.Windows.Forms");
    $ret = [Windows.Forms.MessageBox]::Show(
        "WTL AppWizard for Visual Studio " + $verTitle + " is not found.`r`nDo you want to install?",
        "NuGet Package for WTL",
        [System.Windows.Forms.MessageBoxButtons]::YesNo,
        [System.Windows.Forms.MessageBoxIcon]::Information);
    if ($ret -eq 'No') {
        exit;
    }

    # Install the AppWizard only for current VS version.

    Start-Process wscript.exe ("""$toolsPath\AppWiz\setup.js""", $jsParam) -Wait;
}
finally {
    $mutex.ReleaseMutex();
    $mutex.Close();
}
