@call "%VCINSTALLDIR%\vcvarsall.bat" x86

nmake -nologo RELEASE=1 clean
nmake -nologo RELEASE=1 distrib installer
@if errorlevel 1 goto error
@copy Release.x86\*.7z .
@copy Release.x86\*.msi .
nmake -nologo RELEASE=1 clean

@call "%VCINSTALLDIR%\vcvarsall.bat" x86_amd64

nmake -nologo RELEASE=1 PLATFORM=x64 clean
nmake -nologo RELEASE=1 PLATFORM=x64 distrib installer
@if errorlevel 1 goto error
@copy Release.x64\*.7z .
@copy Release.x64\*.msi .
nmake -nologo RELEASE=1 PLATFORM=x64 clean

@goto end

:error
@echo TERMINATED WITH ERRORS

:end
