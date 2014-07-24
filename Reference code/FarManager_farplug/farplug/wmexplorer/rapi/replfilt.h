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
/****************************************************************************
*                                                                           *
* replfilt.h -- Pegasus filter procedure declarations, structures,          *
*               constant definitions and macros                             *
*                                                                           *
* Copyright (c) Microsoft Corporation. All rights reserved.		            *
*                                                                           *
****************************************************************************/

// {6C5C05E0-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_ICeFileFilterSite, 
0x6c5c05e0, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
// {6C5C05E1-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_ICeFileFilter, 
0x6c5c05e1, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);
// {6C5C05E2-97A2-11cf-8011-00A0C90A8F78}
DEFINE_GUID(IID_ICeFileFilterOptions, 
0x6c5c05e2, 0x97a2, 0x11cf, 0x80, 0x11, 0x0, 0xa0, 0xc9, 0xa, 0x8f, 0x78);

#ifndef _REPLFILT_
#define _REPLFILT_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef LONG CF_ERROR;

#define HRESULT_TO_CFERROR(_hr, _def) \
    (SUCCEEDED(_hr) ? ERROR_SUCCESS : (HRESULT_FACILITY(_hr)==FACILITY_WIN32 ? HRESULT_CODE(_hr) : (_def)))

//
// ICeFileFilterSite interface provided by Windows CE Services
//
#undef  INTERFACE
#define INTERFACE   ICeFileFilterSite

DECLARE_INTERFACE_(ICeFileFilterSite, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** ICeFileFilterSite methods ***
	STDMETHOD(OpenSourceFile) (THIS_
		int nHowToOpenFile,
		LPVOID *ppObj
	) PURE; 
	STDMETHOD(OpenDestinationFile) (THIS_
		int nHowToOpenFile,
        LPCTSTR pszFullpath,
		LPVOID *ppObj
	) PURE; 
	STDMETHOD(CloseSourceFile) (THIS_
		LPUNKNOWN pObj
	) PURE; 
	STDMETHOD(CloseDestinationFile) (THIS_
    	BOOL bKeepFile,
		LPUNKNOWN pObj
	) PURE; 
	STDMETHOD(ReportProgress) (THIS_
	    UINT nPercent
	) PURE; 
	STDMETHOD(ReportLoss) (THIS_
	    DWORD dw,
    	LPCTSTR psz,
    	va_list args
	) PURE; 
};


//
// Structures passed to ICeFileFilter methods
//
typedef struct tagCFF_CONVERTINFO
{
    BOOL bImport;
    HWND hwndParent;
    BOOL bYesToAll;
	ICeFileFilterSite *pffs;
} CFF_CONVERTINFO; 

typedef struct tagCFF_DESTINATIONFILE
{
    TCHAR szFullpath[_MAX_PATH];
    TCHAR szPath[_MAX_PATH];
    TCHAR szFilename[_MAX_FNAME];
    TCHAR szExtension[_MAX_EXT];
} CFF_DESTINATIONFILE; 

typedef struct tagCFF_SOURCEFILE
{
    TCHAR szFullpath[_MAX_PATH];
    TCHAR szPath[_MAX_PATH];
    TCHAR szFilename[_MAX_FNAME];
    TCHAR szExtension[_MAX_EXT];
    DWORD cbSize;
    FILETIME ftCreated;
    FILETIME ftModified;
} CFF_SOURCEFILE; 


//
// ICeFileFilter interface to be implemented by a file filter
//
#undef  INTERFACE
#define INTERFACE   ICeFileFilter

DECLARE_INTERFACE_(ICeFileFilter, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** ICeFileFilter methods ***
	STDMETHOD(NextConvertFile) (THIS_
        int nConversion,
	    CFF_CONVERTINFO* pci,
		CFF_SOURCEFILE* psf,
	    CFF_DESTINATIONFILE* pdf,
	    volatile BOOL *pbCancel,
        CF_ERROR *perr
	) PURE;
	STDMETHOD(FilterOptions) (THIS_
	    HWND hwndParent
	) PURE; 
    STDMETHOD(FormatMessage) (THIS_
        DWORD  dwFlags,
        DWORD  dwMessageId,
        DWORD  dwLanguageId,
        LPTSTR  lpBuffer,
        DWORD  nSize,
        va_list *  Arguments,
        DWORD  *pcb
    ) PURE;
};


/*
 * Flags for how to open files
 */
#define CF_OPENFLAT      0
#define CF_OPENCOMPOUND  1
#define CF_OPENDONT      2
#define CF_OPENASKMEHOW  3

//
// Structures passed to ICeFileFilterOptions methods
//
typedef struct tagCFF_CONVERTOPTIONS
{
    ULONG   cbSize;
    BOOL    bNoModalUI;
} CFF_CONVERTOPTIONS; 


//
// ICeFileFilterOptions interface to be implemented by a v2 file filter
//
#undef  INTERFACE
#define INTERFACE   ICeFileFilterOptions

DECLARE_INTERFACE_(ICeFileFilterOptions, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** ICeFileFilterOptions methods ***
	STDMETHOD(SetFilterOptions) (THIS_
	    CFF_CONVERTOPTIONS* pco
	) PURE; 
};

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#include <CeFltMap.h>
#endif /* !_REPLFILT_ */
