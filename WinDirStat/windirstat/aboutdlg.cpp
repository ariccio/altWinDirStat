// aboutdlg.cpp		- Implementation of the StartAboutDialog() function
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
//#include ".\aboutdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {
	enum {
		RE_CONTROL = 4711	// Id of the RichEdit Control
		};

	// Tabs
	}

/////////////////////////////////////////////////////////////////////////////

void StartAboutDialog( ) {
	AfxBeginThread( RUNTIME_CLASS( CAboutThread ), NULL );
	}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CAboutThread, CWinThread );

BOOL CAboutThread::InitInstance( ) {
	CWinThread::InitInstance( );

	CAboutDlg dlg;
	dlg.DoModal( );
	return false;
	}

/////////////////////////////////////////////////////////////////////////////

void CAboutDlg::CMyTabControl::Initialize( ) {
	ModifyStyle( 0, WS_CLIPCHILDREN );

	InsertItem( TAB_ABOUT, _T( "About" ) );
	InsertItem( TAB_LICENSE, _T( "License" ) );

	CRect rc;
	GetClientRect( rc );
	CRect rcItem;
	GetItemRect( 0, rcItem );
	rc.top = rcItem.bottom;

	VERIFY( m_text.CreateEx( 0, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_MULTILINE | ES_READONLY, rc, this, RE_CONTROL ) );
	SetPageText( TAB_ABOUT );
	}

void CAboutDlg::CMyTabControl::SetPageText( _In_ _Pre_satisfies_( ( tab == TAB_ABOUT ) || ( tab == TAB_LICENSE ) ) INT tab ) {
	USES_CONVERSION;

	CString text, translators;
	DWORD newStyle = ES_CENTER;

	if ( tab == TAB_ABOUT ) {
		text.FormatMessage( IDS_ABOUT_ABOUTTEXTss, _T( "Author's email was here" ), _T( "WDS homepage was here" ) );
		}

	else if ( tab == TAB_LICENSE ) {
		text = GPLtext;
		newStyle = ES_LEFT;
		}
	CRect rc;
	m_text.GetWindowRect( rc );
	ScreenToClient( rc );

	auto style = m_text.GetStyle( );
	style &= ~ES_CENTER;
	style |= newStyle;
	style |= WS_VSCROLL;

	auto exstyle = m_text.GetExStyle( );

	m_text.DestroyWindow( );

	m_text.CreateEx( exstyle, style, rc, this, RE_CONTROL );

	m_text.SetEventMask( ENM_LINK | ENM_KEYEVENTS );
	m_text.SetFont( GetFont( ) );

	m_text.SetWindowText( text );

	m_text.HideCaret( );
	}

BEGIN_MESSAGE_MAP(CAboutDlg::CMyTabControl, CTabCtrl) 
	ON_NOTIFY(EN_MSGFILTER, RE_CONTROL, OnEnMsgFilter)
	ON_WM_SIZE()
END_MESSAGE_MAP()

void CAboutDlg::CMyTabControl::OnEnMsgFilter( NMHDR *pNMHDR, LRESULT *pResult ) {
	auto mf = reinterpret_cast<MSGFILTER *>(pNMHDR);
	*pResult = 0;

	if ( mf->msg == WM_KEYDOWN && ( mf->wParam == VK_ESCAPE || mf->wParam == VK_TAB ) ) {
		// Move the focus back to the Tab control
		SetFocus( );

		// If we didn't ignore VK_ESCAPE here, strange things happen: both m_text and the Tab control would disappear.
		*pResult = 1;
		}
	}


void CAboutDlg::CMyTabControl::OnSize( UINT nType, INT cx, INT cy ) {
	CTabCtrl::OnSize( nType, cx, cy );

	if ( IsWindow( m_text.m_hWnd ) ) {
		CRect rc;
		GetClientRect( rc );

		CRect rcItem;
		GetItemRect( 0, rcItem );

		rc.top = rcItem.bottom;
		m_text.MoveWindow( rc );
		}
	}


////////////////////////////////////////////////////////////////////////////

CAboutDlg::CAboutDlg( ) : CDialog( CAboutDlg::IDD ), m_layout( this, _T( "aboutdlg" ) ) { }

void CAboutDlg::DoDataExchange( CDataExchange* pDX ) {
	CDialog::DoDataExchange( pDX );
	DDX_Control( pDX, IDC_CAPTION, m_caption );
	DDX_Control( pDX, IDC_TAB, m_tab );
	}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnTcnSelchangeTab)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog( ) {
	CDialog::OnInitDialog();
	
	m_layout.AddControl( IDC_CAPTION, 0.5, 0, 0, 0 );
	m_layout.AddControl( IDC_TAB,     0,   0, 1, 1 );
	m_layout.AddControl( IDOK,        0.5, 1, 0, 0 );

	m_layout.OnInitDialog( true );

	m_tab.Initialize( );
	m_caption.SetWindowText( _T( "(alt)WinDirStat" ) );

	return true;
	}

void CAboutDlg::OnTcnSelchangeTab( NMHDR * /* pNMHDR */, LRESULT *pResult ) {
	*pResult = 0;
	m_tab.SetPageText( m_tab.GetCurSel( ) );
	}

void CAboutDlg::OnSize( UINT nType, INT cx, INT cy ) {
	CDialog::OnSize( nType, cx, cy );
	TRACE( _T( "Resizing about dialog!\r\n" ) );
	m_layout.OnSize( );
	}

void CAboutDlg::OnGetMinMaxInfo( MINMAXINFO* mmi ) {
	m_layout.OnGetMinMaxInfo( mmi );
	CDialog::OnGetMinMaxInfo( mmi );
	}

void CAboutDlg::OnDestroy( ) {
	m_layout.OnDestroy();
	CDialog::OnDestroy();
	}

// $Log$
// Revision 1.21  2005/04/17 20:45:19  assarbad
// - Now the list of translators is shared among all translations. See changelog for details.
//
// Revision 1.20  2005/04/17 18:13:46  assarbad
// - Moved some "static" resource strings into the respective *.rc2 files
// - Corrected typo in Russian DLL
// - Modified behavior of VERSIONINFO for DLLs. "version.h" has changed therefore
// ... for details as usual, see the changelog.
//
// Revision 1.19  2005/04/17 12:27:21  assarbad
// - For details see changelog of 2005-04-17
//
// Revision 1.18  2005/04/10 16:49:30  assarbad
// - Some smaller fixes including moving the resource string version into the rc2 files
//
// Revision 1.17  2005/01/29 07:08:18  bseifert
// Added Hungarian resource dll.
//
// Revision 1.16  2004/12/19 10:52:39  bseifert
// Minor fixes.
//
// Revision 1.15  2004/12/12 13:40:51  bseifert
// Improved image coloring. Junction point image now with awxlink overlay.
//
// Revision 1.14  2004/12/12 08:34:59  bseifert
// Aboutbox: added Authors-Tab. Removed license.txt from resource dlls (saves 16 KB per dll).
//
// Revision 1.13  2004/11/23 06:37:04  bseifert
// Fixed bug in AboutDlg: Esc-key made controls disappear.
//
// Revision 1.12  2004/11/13 08:17:07  bseifert
// Remove blanks in Unicode Configuration names.
//
// Revision 1.11  2004/11/12 22:14:16  bseifert
// Eliminated CLR_NONE. Minor corrections.
//
// Revision 1.10  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
