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

CMainFrame* GetMainFrame( ) {
	// Not: return (CMainFrame *)AfxGetMainWnd();
	// because CWinApp::m_pMainWnd is set too late.
	return CMainFrame::GetTheFrame( );
	}

CDirstatApp* GetApp( ) {
	return static_cast< CDirstatApp* >( AfxGetApp( ) );
	}

#ifdef DRAW_ICONS
CMyImageList* GetMyImageList( ) {
	return GetApp( )->GetMyImageList( );
	}
#endif

// CDirstatApp

BEGIN_MESSAGE_MAP(CDirstatApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
END_MESSAGE_MAP()


CDirstatApp _theApp;

#ifdef DRAW_ICONS
_Must_inspect_result_ _Success_( return != NULL )CMyImageList* CDirstatApp::GetMyImageList( ) {
	m_myImageList.Initialize( );
	return &m_myImageList;
	}
#endif

void CDirstatApp::UpdateRamUsage( ) {
	CWinThread::OnIdle( 0 );
	}

void CDirstatApp::PeriodicalUpdateRamUsage( ) {
	if ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate > RAM_USAGE_UPDATE_INTERVAL ) {
		UpdateRamUsage( );
		m_lastPeriodicalRamUsageUpdate = GetTickCount64( );
		}
	}

// Get the alternative colors for compressed and encrypted files/folders. This function uses either the value defined in the Explorer configuration or the default color values.
_Success_( return != clrDefault ) COLORREF CDirstatApp::GetAlternativeColor( _In_ COLORREF clrDefault, _In_z_  PCWSTR which ) {
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

_Success_( SUCCEEDED( return ) ) HRESULT CDirstatApp::GetCurrentProcessMemoryInfo( _Out_writes_z_( strSize ) _Pre_writable_size_( strSize ) PWSTR psz_formatted_usage, _In_range_( 20, 64 ) rsize_t strSize ) {
	auto workingSetBefore = m_workingSet;
	auto Memres = UpdateMemoryInfo( );
	if ( !Memres ) {
		psz_formatted_usage[ 0  ] = 'M';
		psz_formatted_usage[ 1  ] = 'E';
		psz_formatted_usage[ 2  ] = 'M';
		psz_formatted_usage[ 3  ] = '_';
		psz_formatted_usage[ 4  ] = 'I';
		psz_formatted_usage[ 5  ] = 'N';
		psz_formatted_usage[ 6  ] = 'F';
		psz_formatted_usage[ 7  ] = 'O';
		psz_formatted_usage[ 8  ] = '_';
		psz_formatted_usage[ 9  ] = 'E';
		psz_formatted_usage[ 10 ] = 'R';
		psz_formatted_usage[ 11 ] = 'R';
		psz_formatted_usage[ 12 ] =  0;
		return STRSAFE_E_INVALID_PARAMETER;
		}
	const rsize_t ramUsageBytesStrBufferSize = 21;
	wchar_t ramUsageBytesStrBuffer[ ramUsageBytesStrBufferSize ] = { 0 };

	//const rsize_t strSize = 34;
	//wchar_t psz_formatted_usage[ strSize ] = { 0 };


	HRESULT res = FormatBytes( m_workingSet, ramUsageBytesStrBuffer, ramUsageBytesStrBufferSize );
	if ( !SUCCEEDED( res ) ) {
		return StringCchPrintfW( psz_formatted_usage, strSize, L"RAM Usage: %s", FormatBytes( m_workingSet ).GetString( ) );
		}


	HRESULT res2 = StringCchPrintfW( psz_formatted_usage, strSize, L"RAM Usage: %s", ramUsageBytesStrBuffer );
	if ( !SUCCEEDED( res2 ) ) {
		CString n = ( _T( "RAM Usage: %s" ), ramUsageBytesStrBuffer );
		PCWSTR buf = n.GetBuffer( static_cast<int>( strSize ) );
		HRESULT res3 = StringCchCopy( psz_formatted_usage, strSize, buf );
		return res3;
		}

	return res2;
	}

_Success_( return == true ) bool CDirstatApp::UpdateMemoryInfo( ) {
	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	pmc.cb = sizeof( pmc );

	if ( !GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		return false;
		}	

	m_workingSet = pmc.WorkingSetSize;

	return true;
	}

BOOL CDirstatApp::InitInstance( ) {
	//Program entry point
	if ( !SUCCEEDED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ) ) {
		AfxMessageBox( _T( "CoInitializeExFailed!" ) );
		return FALSE;
		}
	auto flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n" ), flag, ( flag & _CRTDBG_ALLOC_MEM_DF ), ( flag & _CRTDBG_CHECK_CRT_DF ), ( flag & _CRTDBG_LEAK_CHECK_DF ) );
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	auto flag2 = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
	TRACE( _T( "CrtDbg state: %i\r\n\t_CRTDBG_ALLOC_MEM_DF: %i\r\n\t_CRTDBG_CHECK_CRT_DF: %i\r\n\t_CRTDBG_LEAK_CHECK_DF: %i\r\n" ), flag2, ( flag2 & _CRTDBG_ALLOC_MEM_DF ), ( flag2 & _CRTDBG_CHECK_CRT_DF ), ( flag2 & _CRTDBG_LEAK_CHECK_DF ) );
	CWinApp::InitInstance();
	InitCommonControls( );			// InitCommonControls() is necessary for Windows XP.
	VERIFY( AfxOleInit( ) );		// For SHBrowseForFolder()
	//AfxEnableControlContainer( );	// For our rich edit controls in the about dialog
	//Do we need to init RichEdit here?
	//VERIFY( AfxInitRichEdit( ) );	// Rich edit control in out about box
	//VERIFY( AfxInitRichEdit2( ) );	// On NT, this helps.
	SetRegistryKey( _T( "Seifert" ) );
	//LoadStdProfileSettings( 4 );

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
	CString text;
	text.FormatMessage( IDS_ABOUT_ABOUTTEXTss );
	displayWindowsMsgBoxWithMessage( text );
	//StartAboutDialog( );
	}

void CDirstatApp::OnFileOpen( ) {
	CSelectDrivesDlg dlg;
	if ( IDOK == dlg.DoModal( ) ) {
		CString path = EncodeSelection( RADIO( dlg.m_radio ), dlg.m_folderName, dlg.m_drives );
		if ( path.Find( '|' ) == -1 ) {
			m_pDocTemplate->OpenDocumentFile( path, true );
			}
		}
	}

BOOL CDirstatApp::OnIdle( _In_ LONG lCount ) {
	BOOL more = FALSE;
	ASSERT( lCount >= 0 );
	auto ramDiff = ( GetTickCount64( ) - m_lastPeriodicalRamUsageUpdate );
	auto doc = GetDocument( );
	if ( doc != NULL ) {
		if ( !doc->Work( ) ) {
			ASSERT( doc->m_workingItem != NULL );
			//Sleep( 10 );//HACK//BUGBUG//TODO//FIXME
			more = TRUE;
			}
		else {
			//more |= CWinThread::OnIdle( 0 );
			}
		}
	
	if ( ramDiff > RAM_USAGE_UPDATE_INTERVAL ) {
		more = CWinApp::OnIdle( lCount );
		if ( !more ) {
			GetApp( )->PeriodicalUpdateRamUsage( );
			}
		else {
			more = CWinThread::OnIdle( 0 );
			}
		}
	return more;
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
