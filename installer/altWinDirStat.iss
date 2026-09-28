; Inno Setup 6 script for altWinDirStat.
; Built by CI:  ISCC /DAppVersion=1.2.3 /DArch=<x64|Win32|ARM64> /DSourceDir=<dir with altWinDirStat.exe> altWinDirStat.iss
; Same AppId as the legacy 0.1.x installer, so installing this upgrades an old altWinDirStat in place.

#ifndef AppVersion
  #define AppVersion "0.0.0"
#endif
#ifndef Arch
  #define Arch "x64"
#endif
#ifndef SourceDir
  #define SourceDir "..\dist\altWinDirStat-" + Arch
#endif

#define AppName "altWinDirStat"
#define AppExe  "altWinDirStat.exe"

[Setup]
AppId={{6F3C2B7E-9A41-4C8D-B2E5-1D7A0C9E4F21}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher=altWinDirStat contributors
AppPublisherURL=https://github.com/Demonad112/altWinDirStat
DefaultDirName={autopf}\{#AppName}
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
LicenseFile={#SourceDir}\LICENSE.txt
OutputBaseFilename={#AppName}-{#AppVersion}-{#Arch}-Setup
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
; Lets the user pick "install for me only" (no admin) or "all users".
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog commandline
UninstallDisplayIcon={app}\{#AppExe}
MinVersion=6.1
#if Arch == "x64"
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
#elif Arch == "ARM64"
ArchitecturesAllowed=arm64
ArchitecturesInstallIn64BitMode=arm64
#endif

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "contextmenu"; Description: "Add ""Analyze with altWinDirStat"" to the Explorer right-click menu for folders and drives"; GroupDescription: "Integration:"

[Files]
Source: "{#SourceDir}\{#AppExe}";   DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceDir}\README.md";   DestDir: "{app}"; Flags: ignoreversion isreadme

[Icons]
Name: "{autoprograms}\{#AppName}"; Filename: "{app}\{#AppExe}"
Name: "{autodesktop}\{#AppName}";  Filename: "{app}\{#AppExe}"; Tasks: desktopicon

[Registry]
; HKA = HKLM for all-users installs, HKCU for per-user installs.
Root: HKA; Subkey: "Software\Classes\Directory\shell\altWinDirStat";         ValueType: string; ValueName: "";     ValueData: "Analyze with altWinDirStat"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Directory\shell\altWinDirStat";         ValueType: string; ValueName: "Icon"; ValueData: """{app}\{#AppExe}"",0";    Tasks: contextmenu
Root: HKA; Subkey: "Software\Classes\Directory\shell\altWinDirStat\command"; ValueType: string; ValueName: "";     ValueData: """{app}\{#AppExe}"" ""%1""";  Tasks: contextmenu
Root: HKA; Subkey: "Software\Classes\Drive\shell\altWinDirStat";             ValueType: string; ValueName: "";     ValueData: "Analyze with altWinDirStat"; Tasks: contextmenu; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\Drive\shell\altWinDirStat";             ValueType: string; ValueName: "Icon"; ValueData: """{app}\{#AppExe}"",0";    Tasks: contextmenu
Root: HKA; Subkey: "Software\Classes\Drive\shell\altWinDirStat\command";     ValueType: string; ValueName: "";     ValueData: """{app}\{#AppExe}"" ""%1""";  Tasks: contextmenu

[Run]
Filename: "{app}\{#AppExe}"; Description: "{cm:LaunchProgram,{#AppName}}"; Flags: nowait postinstall skipifsilent
