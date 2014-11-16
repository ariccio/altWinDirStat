// mainframe.cpp	- Implementation of CMySplitterWnd, CPacmanControl and CMainFrame
//
// WinDirStat - Directory Statistics
// Copyright (C) 2003-2004 Bernhard Seifert
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

#include "graphview.h"
#include "dirstatview.h"
#include "typeview.h"
//#include "osspecific.h"
//#include "item.h"
//#include "pagetreelist.h"
#include "pagetreemap.h"
//#include "pagegeneral.h"
#include ".\mainframe.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//class CPageTreemap;

namespace
{
	// This must be synchronized with the IDR_MAINFRAME menu
	enum TOPLEVELMENU {
		TLM_FILE,
		TLM_EDIT,
		//TLM_CLEANUP,
		TLM_TREEMAP,
		//TLM_REPORT,
		TLM_VIEW,
		TLM_HELP
		};

	enum {
		IDC_DEADFOCUS		// ID of dead-focus window
		};

	class COpenClipboard {
		public:
		COpenClipboard( CWnd* owner, bool empty = true ) {
			m_open = owner->OpenClipboard( );
			if ( !m_open ) {
				displayWindowsMsgBoxWithError( );
				displayWindowsMsgBoxWithMessage( _T( "Cannot open the clipboard." ) );
				TRACE( _T( "Cannot open the clipboard!\r\n" ) );
				}
			if ( empty ) {
				if ( !EmptyClipboard( ) ) {
					displayWindowsMsgBoxWithError( );
					displayWindowsMsgBoxWithMessage( _T( "Cannot empty the clipboard." ) );
					TRACE( _T( "Cannot empty the clipboard!\r\n" ) );
					}
				}
			}
		~COpenClipboard( ) {
			if ( m_open ) {
				CloseClipboard( );
				}
			}
		private:
		BOOL m_open;
		};

	
}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COptionsPropertySheet, CPropertySheet )

//COptionsPropertySheet::COptionsPropertySheet( ) : CPropertySheet( IDS_WINDIRSTAT_SETTINGS ), m_alreadyAsked( false ) { }


BOOL COptionsPropertySheet::OnInitDialog() {
	BOOL bResult = CPropertySheet::OnInitDialog( );
	
	CRect rc;
	GetWindowRect( rc );
	auto pt = rc.TopLeft( );
	CPersistence::GetConfigPosition( pt );
	CRect rc2( pt, rc.Size( ) );
	MoveWindow( rc2 );

	SetActivePage( CPersistence::GetConfigPage( GetPageCount( ) - 1 ) );
	return bResult;
	}

BOOL COptionsPropertySheet::OnCommand( _In_ WPARAM wParam, _In_ LPARAM lParam ) {
	CPersistence::SetConfigPage( GetActiveIndex( ) );

	CRect rc;
	GetWindowRect( rc );
	CPersistence::SetConfigPosition( rc.TopLeft( ) );

	//INT cmd = LOWORD( wParam );
	return CPropertySheet::OnCommand( wParam, lParam );
	}

/////////////////////////////////////////////////////////////////////////////




BEGIN_MESSAGE_MAP(CMySplitterWnd, CSplitterWnd)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CMySplitterWnd::StopTracking(_In_ BOOL bAccept) {
	CSplitterWnd::StopTracking( bAccept );

	if ( bAccept ) {
		CRect rcClient;
		GetClientRect( rcClient );

		if ( GetColumnCount( ) > 1 ) {
			INT dummy = 0;
			INT cxLeft = 0;
			GetColumnInfo( 0, cxLeft, dummy );
	
			if ( ( rcClient.Width( ) ) > 0 ) {
				m_splitterPos = ( DOUBLE ) cxLeft / ( rcClient.Width( ) );
				}
			}
		else {
			INT dummy = 0;
			INT cyUpper = 0;
			GetRowInfo( 0, cyUpper, dummy );
	
			if ( ( rcClient.Height( ) ) > 0 ) {
				m_splitterPos = ( DOUBLE ) cyUpper / ( rcClient.Height( ) );
				}
			}
		m_wasTrackedByUser = true;
		m_userSplitterPos  = m_splitterPos;
		}
	}

void CMySplitterWnd::SetSplitterPos(_In_ const DOUBLE pos) {
	m_splitterPos = pos;

	CRect rcClient;
	GetClientRect( &rcClient );

	if ( GetColumnCount( ) > 1 ) {
		ASSERT( m_pColInfo != NULL );
		if ( m_pColInfo != NULL ) {
			auto cxLeft = INT( pos * ( rcClient.Width( ) ) );
			if ( cxLeft >= 0 ) {
				SetColumnInfo( 0, cxLeft, 0 );
				RecalcLayout( );
				}
			}
		}
	else {
		ASSERT( m_pRowInfo != NULL );
		if ( m_pRowInfo != NULL ) {
			auto cyUpper = INT( pos * ( rcClient.Height( ) ) );
			if ( cyUpper >= 0 ) {
				SetRowInfo( 0, cyUpper, 0 );
				RecalcLayout( );
				}
			}
		}
	}

void CMySplitterWnd::RestoreSplitterPos(_In_ const DOUBLE posIfVirgin) {
	if ( m_wasTrackedByUser ) {
		SetSplitterPos( m_userSplitterPos );
		}
	else {
		SetSplitterPos( posIfVirgin );
		}
	}

void CMySplitterWnd::OnSize( const UINT nType, const INT cx, const INT cy ) {
	if ( GetColumnCount( ) > 1 ) {
		INT cxLeft = ( INT ) ( cx * m_splitterPos );
		if ( cxLeft > 0 ) {
			SetColumnInfo( 0, cxLeft, 0 );
			}
		}
	else {
		INT cyUpper = ( INT ) ( cy * m_splitterPos );
		if ( cyUpper > 0 ) {
			SetRowInfo( 0, cyUpper, 0 );
			}
		}
	CSplitterWnd::OnSize( nType, cx, cy );
	}

void CDeadFocusWnd::Create(_In_ CWnd *parent) {
	CRect rc( 0, 0, 0, 0 );
	VERIFY( CWnd::Create( AfxRegisterWndClass( 0, 0, 0, 0 ), _T( "_deadfocus" ), WS_CHILD, rc, parent, IDC_DEADFOCUS ) );
	}

BEGIN_MESSAGE_MAP(CDeadFocusWnd, CWnd)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CDeadFocusWnd::OnKeyDown( const UINT nChar, const UINT /* nRepCnt */, const UINT /* nFlags */ ) {
	if ( nChar == VK_TAB ) {
		GetMainFrame( )->MoveFocus( LF_DIRECTORYLIST );
		}
	}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	ON_WM_CLOSE()
	ON_WM_INITMENUPOPUP()
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MEMORYUSAGE, OnUpdateMemoryUsage)
	ON_WM_SIZE()
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWTREEMAP, OnUpdateViewShowtreemap)
	ON_COMMAND(ID_VIEW_SHOWTREEMAP, OnViewShowtreemap)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHOWFILETYPES, OnUpdateViewShowfiletypes)
	ON_COMMAND(ID_VIEW_SHOWFILETYPES, OnViewShowfiletypes)
	ON_COMMAND(ID_CONFIGURE, OnConfigure)
	ON_WM_DESTROY()
	//ON_COMMAND(ID_TREEMAP_HELPABOUTTREEMAPS, OnTreemapHelpabouttreemaps)
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

CMainFrame* CMainFrame::_theFrame;

CMainFrame* CMainFrame::GetTheFrame( ) {
	return _theFrame;
	}

INT CMainFrame::OnCreate(const LPCREATESTRUCT lpCreateStruct) {
	/*
	Initializes the MAIN frame - wherein the rectangular layout, the list of files, and the list of file types are.
	Initializes a few related things, such as the memory display.
	*/
	
	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}
	
	VERIFY( m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC ) );

	UINT indicators[ INDICATORS_NUMBER ] = { ID_SEPARATOR, ID_INDICATOR_MEMORYUSAGE };


	VERIFY( m_wndStatusBar.Create( this ) );
	VERIFY( m_wndStatusBar.SetIndicators( indicators, INDICATORS_NUMBER ) );
	
	m_wndDeadFocus.Create( this );

	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
	EnableDocking( CBRS_ALIGN_ANY );
	DockControlBar( &m_wndToolBar );

	LoadBarState( CPersistence::GetBarStateSection( ) );
	ShowControlBar( &m_wndStatusBar, CPersistence::GetShowStatusbar( ), false );
	return 0;
	}

void CMainFrame::InitialShowWindow() {
	WINDOWPLACEMENT wp;
	wp.length = sizeof( wp );
	GetWindowPlacement( &wp );
	CPersistence::GetMainWindowPlacement( wp );
	MakeSaneShowCmd( wp.showCmd );
	SetWindowPlacement( &wp );
	}

void CMainFrame::OnClose() {
	CWaitCursor wc;

	// It's too late, to do this in OnDestroy(). Because the toolbar, if undocked, is already destroyed in OnDestroy(). So we must save the toolbar state here in OnClose().
	SaveBarState( CPersistence::GetBarStateSection( ) );
	CPersistence::SetShowStatusbar( ( m_wndStatusBar.GetStyle( ) & WS_VISIBLE ) != 0 );

#ifdef _DEBUG
	// avoid memory leaks and show hourglass while deleting the tree
	GetDocument()->OnNewDocument();
#endif
	auto Document = GetDocument( );
	if ( Document != NULL ) {
		Document->ForgetItemTree( );
		}
	CFrameWnd::OnClose( );
	}

void CMainFrame::OnDestroy() {
	auto wp = zeroInitWINDOWPLACEMENT( );
	GetWindowPlacement( &wp );
	CPersistence::SetMainWindowPlacement( wp );
	auto TypeView = GetTypeView( );
	auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		CPersistence::SetShowFileTypes( TypeView->m_showTypes );
		}
	if ( GraphView != NULL ) {
		CPersistence::SetShowTreemap( GraphView->m_showTreemap );
		}
	CFrameWnd::OnDestroy( );
	}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext) {
	VERIFY( m_wndSplitter.CreateStatic( this, 2, 1 ) );
	VERIFY( m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS( CGraphView ), CSize( 100, 100 ), pContext ) );
	VERIFY( m_wndSubSplitter.CreateStatic( &m_wndSplitter, INT( 1 ), INT( 2 ), WS_CHILD | WS_VISIBLE | WS_BORDER, UINT( m_wndSplitter.IdFromRowCol( 0, 0 ) ) ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 0, RUNTIME_CLASS( CDirstatView ), CSize( 700, 500 ), pContext ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTypeView ), CSize( 100, 500 ), pContext ) );

	MinimizeGraphView( );
	MinimizeTypeView ( );

	auto TypeView = GetTypeView( );
	auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		TypeView->ShowTypes( CPersistence::GetShowFileTypes( ) );
		}
	if ( GraphView != NULL ) {
		GraphView->m_showTreemap = CPersistence::GetShowTreemap( );
		}
	return TRUE;
	}

void CMainFrame::RestoreTypeView() {
	auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		if ( thisTypeView->m_showTypes ) {
			m_wndSubSplitter.RestoreSplitterPos( 0.72 );
			thisTypeView->RedrawWindow( );
			}
		}
	}


void CMainFrame::RestoreGraphView() {
	auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		if ( thisGraphView->m_showTreemap ) {
			m_wndSplitter.RestoreSplitterPos( 0.4 );
#ifdef PERF_DEBUG_SLEEP
			Sleep( 1000 );
#endif
			TRACE( _T( "Drawing Empty view...\r\n" ) );

#ifdef DEBUG
			auto emptyViewTiming_1 = help_QueryPerformanceCounter( );
			thisGraphView->DrawEmptyView( );
			auto emptyViewTiming_2 = help_QueryPerformanceCounter( );
#endif
			LARGE_INTEGER timingFrequency = help_QueryPerformanceFrequency( );
			const DOUBLE adjustedTimingFrequency = ( ( DOUBLE ) 1.00 ) / timingFrequency.QuadPart;

#ifdef DEBUG
			DOUBLE timeToDrawEmptyWindow = ( emptyViewTiming_2.QuadPart - emptyViewTiming_1.QuadPart ) * adjustedTimingFrequency;
			TRACE( _T( "Done drawing empty view. Timing: %f\r\n" ), timeToDrawEmptyWindow );
#endif


			TRACE( _T( "Drawing treemap...\r\n" ) );
			auto startDrawTime = help_QueryPerformanceCounter( );

			thisGraphView->RedrawWindow( );
			auto endDrawTime = help_QueryPerformanceCounter( );
			
			DOUBLE timeToDrawWindow = ( endDrawTime.QuadPart - startDrawTime.QuadPart ) * adjustedTimingFrequency;
			TRACE( _T( "Finished drawing treemap! Timing:: %f\r\n" ), timeToDrawWindow );
#ifdef PERF_DEBUG_SLEEP
			Sleep( 1000 );
#endif
			ASSERT( timeToDrawWindow != 0 );
			if ( m_lastSearchTime == -1 ) {
				auto searchingTime = GetDocument( )->m_searchTime;
				m_lastSearchTime = searchingTime;
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, GetDocument( )->GetNameLength( ) );//else the search time compounds whenever the time is written to the status bar
				}
			else {
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, GetDocument( )->GetNameLength( ) );
				}
			}
		}
	}

_Must_inspect_result_ _Success_( return != NULL ) CDirstatView* CMainFrame::GetDirstatView() {
	auto pWnd = m_wndSubSplitter.GetPane( 0, 0 );
	auto pView = DYNAMIC_DOWNCAST( CDirstatView, pWnd );
	return pView;
	}

_Must_inspect_result_ _Success_( return != NULL ) CGraphView* CMainFrame::GetGraphView() {
	auto pWnd = m_wndSplitter.GetPane( 1, 0 );
	auto pView = DYNAMIC_DOWNCAST( CGraphView, pWnd );
	return pView;
	}

_Must_inspect_result_ _Success_( return != NULL ) CTypeView* CMainFrame::GetTypeView() {
	auto pWnd = m_wndSubSplitter.GetPane( 0, 1 );
	auto pView = DYNAMIC_DOWNCAST( CTypeView, pWnd );
	return pView;
	}

LRESULT CMainFrame::OnEnterSizeMove( const WPARAM, const LPARAM ) {
	auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( true );
		}
	return 0;
	}

LRESULT CMainFrame::OnExitSizeMove( const WPARAM, const LPARAM ) {
	auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( false );
		}
	return 0;
	}

void CMainFrame::CopyToClipboard( _In_z_ _In_reads_( strLen ) const PCWSTR psz, rsize_t strLen ) {
	COpenClipboard clipboard(this);
	rsize_t strSizeInBytes = ( strLen + 1 ) * sizeof( TCHAR );

	HGLOBAL h = GlobalAlloc( GMEM_MOVEABLE, strSizeInBytes );
	if ( h == NULL ) {
		displayWindowsMsgBoxWithError( );
		displayWindowsMsgBoxWithMessage( _T( "GlobalAlloc failed! Cannot copy to clipboard!" ) );
		TRACE( _T( "GlobalAlloc failed! Cannot copy to clipboard!\r\n" ) );
		return;
		//throw new CMdStringException( _T( "GlobalAlloc failed." ) );
		}

	auto lp = GlobalLock( h );
	if ( lp == NULL ) {
		displayWindowsMsgBoxWithMessage( _T( "GlobalLock failed!" ) );
		displayWindowsMsgBoxWithError( );
		return;
		}

	auto strP = static_cast< PWSTR >( lp );

	HRESULT strCopyRes = StringCchCopyW( strP, strLen, psz );
	if ( !SUCCEEDED( strCopyRes ) ) {
		if ( strCopyRes == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( _T( "StringCchCopyW failed! (STRSAFE_E_INVALID_PARAMETER)" ) );
			}
		if ( strCopyRes == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( _T( "StringCchCopyW failed! (STRSAFE_E_INSUFFICIENT_BUFFER)" ) );
			}
		else {
			displayWindowsMsgBoxWithMessage( _T( "StringCchCopyW failed!" ) );
			}
		displayWindowsMsgBoxWithError( );
		}

	if ( GlobalUnlock( h ) == 0 ) {
		auto err = GetLastError( );
		if ( err != NO_ERROR ) {
			displayWindowsMsgBoxWithMessage( _T( "GlobalUnlock failed!" ) );
			displayWindowsMsgBoxWithError( );
			return;
			}
		}
  

	//wtf is going on here?
	UINT uFormat = CF_TEXT;
	uFormat = CF_UNICODETEXT;
		
	if ( NULL == SetClipboardData( uFormat, h ) ) {
		//throw new CMdStringException( _T( "Cannot set clipboard data." ) );
		displayWindowsMsgBoxWithError( );
		displayWindowsMsgBoxWithMessage( _T( "Cannot set clipboard data! Cannot copy to clipboard!" ) );
		TRACE( _T( "Cannot set clipboard data! Cannot copy to clipboard!\r\n" ) );
		return;
		}
	}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) {
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
	}


void CMainFrame::SetLogicalFocus(_In_ const LOGICAL_FOCUS lf) {
	if ( lf != m_logicalFocus ) {
		m_logicalFocus = lf;
		SetSelectionMessageText( );

		//reinterpret_cast< CDocument* >( GetDocument( ) )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		GetDocument( )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		}
	}

void CMainFrame::MoveFocus( _In_ _Pre_satisfies_( ( lf == LF_NONE ) || ( lf == LF_DIRECTORYLIST ) || ( lf == LF_EXTENSIONLIST ) ) const LOGICAL_FOCUS lf ) {
	if ( lf == LF_NONE ) {
		SetLogicalFocus( LF_NONE );
		m_wndDeadFocus.SetFocus( );
		}
	else if ( lf == LF_DIRECTORYLIST ) {
		auto DirstatView = GetDirstatView( );
		if ( DirstatView != NULL ) {
			DirstatView->SetFocus( );
			}
		}
	else if ( lf == LF_EXTENSIONLIST ) {
		auto TypeView = GetTypeView( );
		if ( TypeView != NULL ) {
			TypeView->SetFocus( );
			}
		}
	}

size_t CMainFrame::getExtDataSize( ) {
	auto Document = GetDocument( );
	if ( Document != NULL ) {
		return Document->GetExtensionRecords( )->size( );
		}
	return 0;
	}

void CMainFrame::WriteTimeToStatusBar( _In_ const double drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength ) {
	CString timeText;
	/*
	  CString::Format reference: http://msdn.microsoft.com/en-us/library/tcxf1dw6.aspx
	  Negative values are assumed to be erroneous.
	*/
	DOUBLE populateTiming = 0;
	DOUBLE averageExtLeng = 0;
	auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		populateTiming = TypeView->getPopulateTiming( );
		averageExtLeng = TypeView->getExtensionNameLength( );
		}
	
	auto extDataSize = getExtDataSize( );
	
		if ( ( searchTiming > 0.00 ) && ( drawTiming > 0.00 ) && ( populateTiming > 0.00 ) ) {
			timeText.Format( _T( "Finding files took %.3f sec. Drawing took %.3f sec. Populating 'file types' took %.3f sec. Total time: %.4f sec. # of file types: %u. Avg name length: %.2f. Avg extension length: %.2f. SSO threshold: %u" ), searchTiming, drawTiming, populateTiming, ( searchTiming + drawTiming + populateTiming ), unsigned( extDataSize ), fileNameLength, averageExtLeng, unsigned( SSO_THRESHOLD_BUF_SIZE ) );
			}
		else {
			timeText.Format( _T( "I had trouble with QueryPerformanceCounter, and can't provide timing. The number of file types: %u. Avg name length: %.2f. Avg extension length: %.2f. SSO threshold: %u" ), unsigned( extDataSize ), fileNameLength, averageExtLeng, unsigned( SSO_THRESHOLD_BUF_SIZE ) );
			}
	SetMessageText( timeText );
	m_drawTiming = timeText;
	}

void CMainFrame::WriteTimeToStatusBar( ) {
	if ( m_drawTiming != "" ) {
		SetMessageText( m_drawTiming );
		}
	else {
		SetMessageText( _T( "Eeek! No timing info!" ) );
		}
	}


void CMainFrame::SetSelectionMessageText() {
	switch ( GetLogicalFocus( ) )
	{
		case LF_NONE:
			SetMessageText( m_drawTiming );
			break;
		case LF_DIRECTORYLIST:
			{
			auto Document = GetDocument( );
			if ( Document != NULL ) {
				auto Selection = Document->GetSelection( );
				if ( Selection != NULL ) {
					SetMessageText( Selection->GetPath( ).c_str( ) );
					}
				else {
					//SetMessageText(L"are we?");
					SetMessageText( m_drawTiming );
					}
				}
			else {
				ASSERT( false );
				SetMessageText( _T( "No document?" ) );
				}
			}
			break;
		case LF_EXTENSIONLIST:
			SetMessageText( _T("*") + CString( GetDocument( )->GetHighlightExtension( ).c_str( ) ) );
			break;
	}
	}

void CMainFrame::OnUpdateMemoryUsage( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( true );
	const rsize_t ramUsageStrBufferSize = 34;
	wchar_t ramUsageStr[ ramUsageStrBufferSize ] = { 0 };
	
	HRESULT res = GetApp( )->GetCurrentProcessMemoryInfo( ramUsageStr, ramUsageStrBufferSize );
	if ( !SUCCEEDED( res ) ) {
		write_BAD_FMT( ramUsageStr );
		}
	pCmdUI->SetText( ramUsageStr );
	}



void CMainFrame::OnSize( const UINT nType, const INT cx, const INT cy ) {
	CFrameWnd::OnSize( nType, cx, cy );

	if ( !IsWindow( m_wndStatusBar.m_hWnd ) ) {
		return;
		}
	CRect rc;
	m_wndStatusBar.GetItemRect( 0, rc );
	}

void CMainFrame::OnUpdateViewShowtreemap(CCmdUI *pCmdUI) {
	auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		pCmdUI->SetCheck( GraphView->m_showTreemap );
		}
	ASSERT( GraphView != NULL );
	}

void CMainFrame::OnViewShowtreemap() {
	auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		thisGraphView->m_showTreemap = !thisGraphView->m_showTreemap;
		if ( thisGraphView->m_showTreemap ) {
			RestoreGraphView( );
			}
		else {
			MinimizeGraphView( );
			}
		}
	ASSERT( thisGraphView != NULL );
	}

void CMainFrame::OnUpdateViewShowfiletypes(CCmdUI *pCmdUI) {
	auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		pCmdUI->SetCheck( TypeView->m_showTypes );
		}
	ASSERT( TypeView != NULL );
	}

void CMainFrame::OnViewShowfiletypes() {
	auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		thisTypeView->ShowTypes( !thisTypeView->m_showTypes );
		if ( thisTypeView->m_showTypes ) {
			RestoreTypeView( );
			}
		else {
			MinimizeTypeView( );
			}
		}
	ASSERT( thisTypeView != NULL );
	}

void CMainFrame::OnConfigure() {
	COptionsPropertySheet sheet;

	CPageGeneral  general;
	CPageTreemap  treemap;

	sheet.AddPage( &general );
	sheet.AddPage( &treemap );

	sheet.DoModal( );
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		Options->SaveToRegistry( );
		}
	}

void CMainFrame::OnSysColorChange() {
	CFrameWnd::OnSysColorChange( );
	auto DirstatView = GetDirstatView( );
	if ( DirstatView != NULL ) {
		DirstatView->SysColorChanged( );
		}
	ASSERT( DirstatView != NULL );
	auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		TypeView->SysColorChanged( );
		}
	ASSERT( TypeView != NULL );
	}

// $Log$
// Revision 1.9  2004/11/27 07:19:37  bseifert
// Unicode/Ansi/Debug-specification in version.h/Aboutbox. Minor fixes.
//
// Revision 1.8  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
