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
/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

    ceutil.h

Abstract:

    Declaration and implementation of Service helper functions

    Contains all registry manipulation functions for the Windows CE Services.
    Note: The Windows CE Services path is always
        "Software\\Microsoft\\Windows CE Services"
    under HKCU or HKLM


Environment:

    WIN32

--*/
#ifndef _INC_CEREG_H
#define _INC_CEREG_H

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

typedef HKEY  HCESVC;
typedef PHKEY PHCESVC;
typedef DWORD DEVICEID;

#define DEVICE_GUEST    (DEVICEID)-1
#define DEVICE_INVALID  (DEVICEID)0

enum {
    CESVC_ROOT_COMMON=0,
    CESVC_ROOT_MACHINE = CESVC_ROOT_COMMON,
    CESVC_ROOT_USER,
    CESVC_DEVICES,
    CESVC_DEVICEX,
    CESVC_DEVICE,
    CESVC_DEVICE_SELECTED,
    CESVC_SERVICES,
    CESVC_SERVICES_USER = CESVC_SERVICES,
    CESVC_SERVICES_COMMON,
    CESVC_SYNC,
    CESVC_SYNC_COMMON,
    CESVC_FILTERS,
    CESVC_SPECIAL_DEFAULTS,
    CESVC_CUSTOM_MENUS,
    CESVC_SYNCX };

#define SVC_FLAG_GUEST            0x0001
#define SVC_FLAG_CURRENT_PROFILE  0x0002
#define SVC_FLAG_PROFILE          0x0004  // specify profile id
#define SVC_FLAG_ALL_PROFILES     0x0008  // all ids
#define SVC_FLAG_COMMON           0x0010  // shared crud
#define SVC_FLAG_ALL              0x001F

typedef struct {
    DWORD   cbSize;
    DWORD   Flags;
    DWORD   ProfileId;
    BOOL    Enabled;
} SVCINFO_GENERIC;

typedef struct {
    DWORD   cbSize;
    DWORD   Flags;
    DWORD   ProfileId;
    BOOL    Enabled;
    LPSTR   DisplayName;        // sync app name
    LPSTR   ProgId;
} SVCINFO_SYNC;

typedef struct {
    DWORD   cbSize;
    DWORD   Flags;
    DWORD   ProfileId;
    BOOL    Enabled;
    LPWSTR  DisplayName;        // sync app name
    LPWSTR  ProgId;
} SVCINFO_SYNCW;


//
// Prototypes:
//

//
//  Obsolete APIs 
//
HRESULT __stdcall CeSvcAdd( LPSTR pszSvcName, LPSTR pszSvcClass, LPVOID pSvcInfo );
HRESULT __stdcall CeSvcRemove(LPSTR pszSvcName, LPSTR pszSvcClass, DWORD dwSvcFlags );
HRESULT __stdcall CeSvcQueryInfo( LPSTR pszSvcName, LPSTR pszSvcClass, LPVOID pSvcInfo, DWORD cbBuffer );
HRESULT __stdcall CeSvcDelete( HCESVC hSvc );
//
//  End of obsolete APIs
//


//
// legacy entry points
//
HRESULT __stdcall CeSvcOpen( UINT uSvc, LPSTR pszPath, BOOL fCreate, PHCESVC phSvc );
HRESULT __stdcall CeSvcOpenEx( HCESVC hSvcRoot, LPSTR pszPath, BOOL fCreate, PHCESVC phSvc );
HRESULT __stdcall CeSvcClose( HCESVC hSvc );

HRESULT __stdcall CeSvcGetString( HCESVC hSvc, LPCSTR pszValName, LPSTR pszVal, DWORD cbVal );
HRESULT __stdcall CeSvcSetString( HCESVC hSvc, LPCSTR pszValName, LPCSTR pszVal );
HRESULT __stdcall CeSvcGetDword( HCESVC hSvc, LPCSTR pszValName, LPDWORD pdwVal );
HRESULT __stdcall CeSvcSetDword( HCESVC hSvc, LPCSTR pszValName, DWORD dwVal );
HRESULT __stdcall CeSvcGetBinary( HCESVC hSvc, LPCSTR pszValName, LPBYTE pszVal, LPDWORD pcbVal );
HRESULT __stdcall CeSvcSetBinary( HCESVC hSvc, LPCSTR pszValName, LPBYTE pszVal, DWORD cbVal );
HRESULT __stdcall CeSvcDeleteVal( HCESVC hSvc, LPCSTR pszValName );

DEVICEID __stdcall CeGetDeviceId( void );
DEVICEID __stdcall CeGetSelectedDeviceId( void );

HRESULT __stdcall CeSvcEnumProfiles(PHCESVC phSvc, DWORD lProfileIndex, PDWORD plProfile);


//
// Unicode
//
HRESULT __stdcall CeSvcOpenW( UINT uSvc, LPWSTR pszPath, BOOL fCreate, PHCESVC phSvc );
HRESULT __stdcall CeSvcOpenExW( HCESVC hSvcRoot, LPWSTR pszPath, BOOL fCreate, PHCESVC phSvc );

HRESULT __stdcall CeSvcGetStringW( HCESVC hSvc, LPCWSTR pszValName, LPWSTR pszVal, DWORD cbVal );
HRESULT __stdcall CeSvcSetStringW( HCESVC hSvc, LPCWSTR pszValName, LPCWSTR pszVal );
HRESULT __stdcall CeSvcGetDwordW( HCESVC hSvc, LPCWSTR pszValName, LPDWORD pdwVal );
HRESULT __stdcall CeSvcSetDwordW( HCESVC hSvc, LPCWSTR pszValName, DWORD dwVal );
HRESULT __stdcall CeSvcGetBinaryW( HCESVC hSvc, LPCWSTR pszValName, LPBYTE pszVal, LPDWORD pcbVal );
HRESULT __stdcall CeSvcSetBinaryW( HCESVC hSvc, LPCWSTR pszValName, LPBYTE pszVal, DWORD cbVal );
HRESULT __stdcall CeSvcDeleteValW( HCESVC hSvc, LPCWSTR pszValName );

#ifdef __cplusplus
}       /* End of extern "C" { */
#endif /* __cplusplus */

#ifdef UNICODE
#ifndef CEUTIL_IMPL

#define SVCINFO_SYNC    SVCINFO_SYNCW
#define CeSvcOpen(uSvc, pszPath, fCreate, phSvc)            CeSvcOpenW(uSvc, pszPath, fCreate, phSvc)
#define CeSvcOpenEx(hSvcRoot, pszPath, fCreate, phSvc)      CeSvcOpenExW(hSvcRoot, pszPath, fCreate, phSvc)
#define CeSvcGetString(hSvc, pszValName, pszVal, cbVal)     CeSvcGetStringW(hSvc, pszValName, pszVal, cbVal)
#define CeSvcSetString(hSvc, pszValName, pszVal)            CeSvcSetStringW(hSvc, pszValName, pszVal)
#define CeSvcGetDword(hSvc, pszValName, pdwVal)             CeSvcGetDwordW(hSvc, pszValName, pdwVal)
#define CeSvcSetDword(hSvc, pszValName, dwVal)              CeSvcSetDwordW(hSvc, pszValName, dwVal)
#define CeSvcGetBinary(hSvc, pszValName, pszVal, pcbVal)    CeSvcGetBinaryW(hSvc, pszValName, pszVal, pcbVal)
#define CeSvcSetBinary(hSvc, pszValName, pszVal, cbVal)     CeSvcSetBinaryW(hSvc, pszValName, pszVal, cbVal)
#define CeSvcDeleteVal(hSvc, pszValName)                    CeSvcDeleteValW(hSvc, pszValName)

#endif
#endif

#endif  // _INC_CEREG_H

