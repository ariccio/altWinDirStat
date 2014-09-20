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
	}

_Must_inspect_result_ _Success_( return != NULL )CMyImageList *CDirstatApp::GetMyImageList( ) {
	m_myImageList.Initialize( );
	return &m_myImageList;
	}

void CDirstatApp::UpdateRamUsage( ) {
	CWinThread::OnIdle(0);
	}

void CDirstatApp::PeriodicalUpdateRamUsage( ) {
	if ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate > RAM_USAGE_UPDATE_INTERVAL ) {
		UpdateRamUsage( );
		m_lastPeriodicalRamUsageUpdate = GetTickCount64( );
		}
	}

bool CDirstatApp::b_PeriodicalUpdateRamUsage( ) {
	/*
	  Wrapper for async launch
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
		auto a = GetAppFileName( );
		auto b = GetLastErrorAsFormattedMessage( );
		s.FormatMessage( IDS_CREATEPROCESSsFAILEDs, a, b );
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

bool CDirstatApp::IsMountPoint( _In_ CString path ) const {
	return m_mountPoints.IsMountPoint( path );
	}

bool CDirstatApp::IsJunctionPoint( _In_ CString path ) const {
	return m_mountPoints.IsJunctionPoint( path );
	}

bool CDirstatApp::IsJunctionPoint( _In_ CString path, _In_ DWORD fAttributes ) const {
	return m_mountPoints.IsJunctionPoint( path, fAttributes );
	}

// Get the alternative colors for compressed and encrypted files/folders. This function uses either the value defined in the Explorer configuration or the default color values.
_Success_( return != clrDefault ) COLORREF CDirstatApp::GetAlternativeColor( _In_ COLORREF clrDefault, _In_z_  LPCTSTR which ) {
	COLORREF x;
	DWORD cbValue = sizeof( x );
	CRegKey key;

	// Open the explorer key
	key.Open( HKEY_CURRENT_USER, _T( "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer" ), KEY_READ );

	// Try to read the REG_BINARY value
	if ( ERROR_SUCCESS == key.QueryBinaryValue( which, &x, &cbValue ) ) {
		return x;
		}
	return clrDefault;
	}

COLORREF CDirstatApp::AltEncryptionColor( ) {
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
	CString n = ( _T( "RAM Usage: %s" ), FormatBytes( m_workingSet ) );
	m_MemUsageCache = n;
	return n;
	}

bool CDirstatApp::UpdateMemoryInfo( ) {
	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	pmc.cb = sizeof( pmc );

	if ( !GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
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
	//Program entry point
	auto flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n" ), flag, ( flag & _CRTDBG_ALLOC_MEM_DF ), ( flag & _CRTDBG_CHECK_CRT_DF ), ( flag & _CRTDBG_LEAK_CHECK_DF ) );
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	auto flag2 = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n" ), flag2, ( flag2 & _CRTDBG_ALLOC_MEM_DF ), ( flag2 & _CRTDBG_CHECK_CRT_DF ), ( flag2 & _CRTDBG_LEAK_CHECK_DF ) );
	CWinApp::InitInstance();
	InitCommonControls( );			// InitCommonControls() is necessary for Windows XP.
	VERIFY( AfxOleInit( ) );		// For SHBrowseForFolder()
	AfxEnableControlContainer( );	// For our rich edit controls in the about dialog
	//Do we need to init RichEdit here?
	VERIFY( AfxInitRichEdit( ) );	// Rich edit control in out about box
	VERIFY( AfxInitRichEdit2( ) );	// On NT, this helps.

	SetRegistryKey( _T( "Seifert" ) );
	LoadStdProfileSettings( 4 );

	GetOptions( )->LoadFromRegistry( );
	
	m_pDocTemplate = new CSingleDocTemplate { IDR_MAINFRAME, RUNTIME_CLASS( CDirstatDoc ), RUNTIME_CLASS( CMainFrame ), RUNTIME_CLASS( CGraphView ) };
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

INT CDirstatApp::ExitInstance( ) {
	return CWinApp::ExitInstance( );
	}

void CDirstatApp::OnAppAbout( ) {
	StartAboutDialog( );
	}

void CDirstatApp::OnFileOpen( ) {
	CSelectDrivesDlg dlg;
	if ( IDOK == dlg.DoModal( ) ) {
		CString path = EncodeSelection( ( RADIO ) dlg.m_radio, dlg.m_folderName, dlg.m_drives );
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
			std::future<bool> fut = std::async( std::launch::async, [] {return ( GetApp( )->b_PeriodicalUpdateRamUsage( ) ); } );
			}
		else {
			more = CWinThread::OnIdle( 0 );
			}
		}
	return more;
	}

void CDirstatApp::DoContextHelp( _In_ DWORD topic ) const {
	UNREFERENCED_PARAMETER( topic );
	AfxMessageBox( _T( "Help is currently disabled. It will be reintroduced in a future build." ) );
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
