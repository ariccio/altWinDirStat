//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this sample source code is subject to the terms of the Microsoft
// license agreement under which you licensed this sample source code. If
// you did not accept the terms of the license agreement, you are not
// authorized to use this sample source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the LICENSE.RTF on your install media or the root of your tools installation.
// THE SAMPLE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
//
// CE_SETUP.H
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This public header file specifies function prototypes that WCELOAD.EXE will call
// in the ISV application's "SETUP.DLL", as well as the supported return values.

#ifdef __cplusplus
extern "C" {
#endif

//
// Install_Init
//
// @comm    Called before any part of the application is installed
//
typedef enum
{
    codeINSTALL_INIT_CONTINUE  = 0,     // @comm Continue with the installation
    codeINSTALL_INIT_CANCEL             // @comm Immediately cancel the installation
}
codeINSTALL_INIT;

codeINSTALL_INIT
Install_Init(
    HWND        hwndParent,
    BOOL        fFirstCall,     // is this the first time this function is being called?
    BOOL        fPreviouslyInstalled,
    LPCTSTR     pszInstallDir
);

typedef codeINSTALL_INIT (*pfnINSTALL_INIT)( HWND, BOOL, BOOL, LPCTSTR );
const TCHAR szINSTALL_INIT[]    = TEXT("Install_Init");

//
// Install_Exit
//
// @comm    Called after the application is installed
//
typedef enum
{
    codeINSTALL_EXIT_DONE       = 0,    // @comm Exit the installation successfully
    codeINSTALL_EXIT_UNINSTALL          // @comm Uninstall the application before exiting the installation
}
codeINSTALL_EXIT;

codeINSTALL_EXIT
Install_Exit(
    HWND    hwndParent,
    LPCTSTR pszInstallDir,      // final install directory
    WORD    cFailedDirs,
    WORD    cFailedFiles,
    WORD    cFailedRegKeys,
    WORD    cFailedRegVals,
    WORD    cFailedShortcuts
);

typedef codeINSTALL_EXIT (*pfnINSTALL_EXIT)( HWND, LPCTSTR, WORD, WORD, WORD, WORD, WORD );
const TCHAR szINSTALL_EXIT[]    = TEXT("Install_Exit");

//
// Uninstall_Init
//
// @comm    Called before the application is uninstalled
//
typedef enum
{
    codeUNINSTALL_INIT_CONTINUE = 0,    // @comm Continue with the uninstallation
    codeUNINSTALL_INIT_CANCEL           // @comm Immediately cancel the uninstallation
}
codeUNINSTALL_INIT;

codeUNINSTALL_INIT
Uninstall_Init(
    HWND        hwndParent,
    LPCTSTR     pszInstallDir
);

typedef codeUNINSTALL_INIT (*pfnUNINSTALL_INIT)( HWND, LPCTSTR );
const TCHAR szUNINSTALL_INIT[]  = TEXT("Uninstall_Init");

//
// Uninstall_Exit
//
// @comm    Called after the application is uninstalled
//
typedef enum
{
    codeUNINSTALL_EXIT_DONE     = 0     // @comm Exit the uninstallation successfully
}
codeUNINSTALL_EXIT;

codeUNINSTALL_EXIT
Uninstall_Exit(
    HWND    hwndParent
);

typedef codeUNINSTALL_EXIT (*pfnUNINSTALL_EXIT)( HWND );
const TCHAR szUNINSTALL_EXIT[]  = TEXT("Uninstall_Exit");

#ifdef __cplusplus
}       // extern "C"
#endif

