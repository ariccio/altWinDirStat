// windirstat.cpp	- Implementation of CDirstatApp and some globals
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2005 Bernhard Seifert
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: bseifert@users.sourceforge.net, bseifert@daccord.net
//
// Last modified: $Date$

#include "stdafx.h"
#include "windirstat.h"
#include "mainframe.h"
#include "selectdrivesdlg.h"
#include "aboutdlg.h"
//#include "reportbugdlg.h"
//#include "modalsendmail.h"
#include "dirstatdoc.h"
#include "graphview.h"
#include <windows.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMainFrame *GetMainFrame( ) {
	// Not: return (CMainFrame *)AfxGetMainWnd();
	// because CWinApp::m_pMainWnd is set too late.
	return CMainFrame::GetTheFrame();
	}

CDirstatApp *GetApp( ) {
	return ( CDirstatApp * ) AfxGetApp( );
	}

CMyImageList *GetMyImageList( ) {
	return GetApp( )->GetMyImageList( );
	}

// CDirstatApp

BEGIN_MESSAGE_MAP(CDirstatApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


CDirstatApp _theApp;

CDirstatApp::CDirstatApp( ) {
	m_workingSet                   = 0;
	m_pageFaults                   = 0;
	m_lastPeriodicalRamUsageUpdate = GetTickCount64();
	m_altEncryptionColor           = GetAlternativeColor(RGB(0x00, 0x80, 0x00), _T("AltEncryptionColor"));
	m_altColor                     = GetAlternativeColor(RGB(0x00, 0x00, 0xFF), _T("AltColor"));

	#ifdef _DEBUG

	#endif
	}

_Must_inspect_result_ CMyImageList *CDirstatApp::GetMyImageList( ) {
	m_myImageList.Initialize( );
	return &m_myImageList;
	}

void CDirstatApp::UpdateRamUsage( ) {
	CWinThread::OnIdle(0);
	}

void CDirstatApp::PeriodicalUpdateRamUsage()
{
	if ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate > RAM_USAGE_UPDATE_INTERVAL ) {
		UpdateRamUsage( );
		m_lastPeriodicalRamUsageUpdate = GetTickCount64( );
		}
}

bool CDirstatApp::b_PeriodicalUpdateRamUsage( ) {
	/*
	  Wrapper around PeriodicalUpdateRamUsage for async launch
	*/
	PeriodicalUpdateRamUsage();
	return true;
	}

void CDirstatApp::RestartApplication( ) {
	// First, try to create the suspended process
	
	auto si = zeroInitSTARTUPINFO( );
	si.cb = sizeof( si );

	auto pi = zeroInitPROCESS_INFORMATION( );
	

	BOOL success = CreateProcess( GetAppFileName( ), NULL, NULL, NULL, false, CREATE_SUSPENDED, NULL, NULL, &si, &pi );
	if (!success) {
		CString s;
		std::wstring a = GetAppFileName( );
		std::wstring b = MdGetWinerrorText( GetLastError( ) );
		s.FormatMessage( IDS_CREATEPROCESSsFAILEDs, a.c_str( ), b.c_str( ) );
		AfxMessageBox( s );
		return;
		}

	// We _send_ the WM_CLOSE here to ensure that all CPersistence-Settings like column widths an so on are saved before the new instance is resumed.
	// This will post a WM_QUIT message.
	GetMainFrame( )->SendMessage( WM_CLOSE );

	DWORD dw = ::ResumeThread( pi.hThread );
	if ( dw != 1 ) {
		TRACE( _T( "ResumeThread() didn't return 1\r\n" ) );
		}
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	}

void CDirstatApp::ReReadMountPoints( ) {
	m_mountPoints.Initialize( );
	}

bool CDirstatApp::IsMountPoint( _In_ CString path ) {
	return m_mountPoints.IsMountPoint(path);
	}

bool CDirstatApp::IsJunctionPoint( _In_ CString path ) {
	return m_mountPoints.IsJunctionPoint( path );
	}

bool CDirstatApp::IsJunctionPoint( _In_ CString path, _In_ DWORD fAttributes ) {
	return m_mountPoints.IsJunctionPoint( path, fAttributes );
	}

// Get the alternative colors for compressed and encrypted files/folders.
// This function uses either the value defined in the Explorer configuration or the default color values.
COLORREF CDirstatApp::GetAlternativeColor( _In_ COLORREF clrDefault, _In_ LPCTSTR which ) {
	const LPCTSTR explorerKey = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer");
	COLORREF x;
	DWORD cbValue = sizeof(x);
	CRegKey key;

	// Open the explorer key
	key.Open( HKEY_CURRENT_USER, explorerKey, KEY_READ );

	// Try to read the REG_BINARY value
	if ( ERROR_SUCCESS == key.QueryBinaryValue( which, &x, &cbValue ) ) {
		// Return the read value upon success
		return x;
		}
	else {
		// Return the default upon failure
		return clrDefault;
		}
	}

COLORREF CDirstatApp::AltColor()
{
	// Return property value
	return m_altColor;
}

COLORREF CDirstatApp::AltEncryptionColor()
{
	// Return property value
	return m_altEncryptionColor;
}

CString CDirstatApp::GetCurrentProcessMemoryInfo( ) {
	auto workingSetBefore = m_workingSet;

	UpdateMemoryInfo( );
	auto difference = m_workingSet - workingSetBefore;
	if ( m_workingSet == workingSetBefore && ( m_MemUsageCache != _T( "" ) ) ) {
		return m_MemUsageCache;
		}
	else if ( abs( difference ) < ( m_workingSet * 0.01 ) && ( m_MemUsageCache != _T( "" ) ) ) {
		return m_MemUsageCache;
		}
	else if ( m_workingSet == 0 ) {
		return _T( "" );
		}

	else {
		CString n = ( _T( "RAM Usage: %s" ), FormatBytes( m_workingSet ) );
		m_MemUsageCache = n;
		return n;
		}
	}

//_Must_inspect_result_ CGetCompressedFileSizeApi *CDirstatApp::GetComprSizeApi( ) {
//	return &m_comprSize;
//	}

bool CDirstatApp::UpdateMemoryInfo( ) {
	if ( !m_psapi.IsSupported( ) ) {
		return false;
		}

	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	pmc.cb = sizeof( pmc );

	if ( !m_psapi.GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		return false;
		}

	m_workingSet = pmc.WorkingSetSize;

	bool ret = false;
	if ( pmc.PageFaultCount > m_pageFaults + 500 ) {
		ret = true;
		}

	m_pageFaults = pmc.PageFaultCount;

	return ret;
	}

BOOL CDirstatApp::InitInstance( ) {
	CWinApp::InitInstance();
	InitCommonControls();			// InitCommonControls() is necessary for Windows XP.
	VERIFY(AfxOleInit());			// For SHBrowseForFolder()
	AfxEnableControlContainer();	// For our rich edit controls in the about dialog
	//Do we need to init RichEdit here?
	VERIFY(AfxInitRichEdit());		// Rich edit control in out about box
	VERIFY(AfxInitRichEdit2());		// On NT, this helps.
	//EnableHtmlHelp();

	SetRegistryKey(_T("Seifert"));
	LoadStdProfileSettings(4);

	GetOptions( )->LoadFromRegistry( );
	
	m_pDocTemplate = new CSingleDocTemplate( IDR_MAINFRAME, RUNTIME_CLASS( CDirstatDoc ), RUNTIME_CLASS( CMainFrame ), RUNTIME_CLASS( CGraphView ) );
	if ( !m_pDocTemplate ) {
		return FALSE;
		}
	AddDocTemplate( m_pDocTemplate );
	
	CCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );

	m_nCmdShow = SW_HIDE;
	if ( !ProcessShellCommand( cmdInfo ) ) {
		return FALSE;
		}

	GetMainFrame( )->InitialShowWindow( );
	m_pMainWnd->UpdateWindow( );

	// When called by setup.exe, windirstat remained in the background, so we do a
	m_pMainWnd->BringWindowToTop( );
	m_pMainWnd->SetForegroundWindow( );
	if ( cmdInfo.m_nShellCommand != CCommandLineInfo::FileOpen ) {
		OnFileOpen( );
		}

	return TRUE;
	}

INT CDirstatApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}


void CDirstatApp::OnAppAbout()
{
	StartAboutDialog();
}


//UINT workerThread( LPVOID parameter ) {
//	auto doc = ( CDirstatDoc* ) parameter;
//	if ( doc != NULL ) {
//		auto isDone = doc->Work( 1000 );
//		while ( !isDone ) {
//			doc->Work( 1000 );
//			}
//		}
//	}


void CDirstatApp::OnFileOpen()
{
	CSelectDrivesDlg dlg;
	if ( IDOK == dlg.DoModal( ) ) {
		CString path = CDirstatDoc::EncodeSelection( ( RADIO ) dlg.m_radio, dlg.m_folderName, dlg.m_drives );
		m_pDocTemplate->OpenDocumentFile( path, true );
		}
}

BOOL CDirstatApp::OnIdle( _In_ LONG lCount ) {
	BOOL more = false;
	ASSERT( lCount >= 0 );
	
	auto doc = GetDocument( );
	if ( doc != NULL ) {
		if ( !doc->Work( 1000 ) ) {
			more = true;
			}
		}
	
	else if ( ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate ) > RAM_USAGE_UPDATE_INTERVAL ) {
		more = CWinApp::OnIdle( lCount );
		if ( !more ) {
			std::future<bool> fut = std::async( std::launch::async | std::launch::deferred, [] {return ( GetApp( )->b_PeriodicalUpdateRamUsage( ) ); } );
			}
		else {
			more = CWinThread::OnIdle( 0 );
			}
		}
	// The status bar (RAM usage) is updated only when count == 0.
	// That's why we call an extra OnIdle(0) here.
	//if ( CWinThread::OnIdle( 0 ) ) {
	//	more  = true;
	//	}
	return more;
	}

void CDirstatApp::DoContextHelp( _In_ DWORD topic ) {
	( VOID ) topic;
	if ( FileExists( m_pszHelpFilePath ) ) {
		// I want a NULL parent window. So I don't use CWinApp::HtmlHelp().
		//::HtmlHelp( NULL, m_pszHelpFilePath, HH_HELP_CONTEXT, topic );
		}
	else {
		//CString msg;
		//msg.FormatMessage( IDS_HELPFILEsCOULDNOTBEFOUND, _T( "windirstat.chm" ) );
		//AfxMessageBox( msg );
		}
	CString msg = _T( "Help is currently disabled. It will be reintroduced in a future build." );
	AfxMessageBox( msg );
	}


// $Log$
// Revision 1.16  2005/04/17 12:27:21  assarbad
// - For details see changelog of 2005-04-17
//
// Revision 1.15  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.14  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.13  2004/11/28 14:40:06  assarbad
// - Extended CFileFindWDS to replace a global function
// - Now packing/unpacking the file attributes. This even spares a call to find encrypted/compressed files.
//
// Revision 1.12  2004/11/25 11:58:52  assarbad
// - Minor fixes (odd behavior of coloring in ANSI version, caching of the GetCompressedFileSize API)
//   for details see the changelog.txt
//
// Revision 1.11  2004/11/14 08:49:06  bseifert
// Date/Time/Number formatting now uses User-Locale. New option to force old behavior.
//
// Revision 1.10  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.9  2004/11/10 01:03:00  assarbad
// - Style cleaning of the alternative coloring code for compressed/encrypted items
//
// Revision 1.8  2004/11/08 00:46:26  assarbad
// - Added feature to distinguish compressed and encrypted files/folders by color as in the Windows 2000/XP explorer.
//   Same rules apply. (Green = encrypted / Blue = compressed)
//
// Revision 1.7  2004/11/05 16:53:08  assarbad
// Added Date and History tag where appropriate.
//
