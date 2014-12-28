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
#include "item.h"

#include "pagetreemap.h"
#include "pagegeneral.h"
#include "mainframe.h"
#include "dirstatdoc.h"
#include "options.h"
#include "windirstat.h"


#include "globalhelpers.h"

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
		COpenClipboard( const COpenClipboard& in ) = delete;

		COpenClipboard& operator=( const COpenClipboard& in ) = delete;

		COpenClipboard( CWnd* const owner, const bool empty = true ) : m_open( owner->OpenClipboard( ) ) {
			//m_open = owner->OpenClipboard( );
			if ( !m_open ) {
				displayWindowsMsgBoxWithError( );
				displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"Cannot open the clipboard." ) ) );
				TRACE( _T( "Cannot open the clipboard!\r\n" ) );
				}
			if ( empty ) {
				if ( !EmptyClipboard( ) ) {
					displayWindowsMsgBoxWithError( );
					displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"Cannot empty the clipboard." ) ) );
					TRACE( _T( "Cannot empty the clipboard!\r\n" ) );
					}
				}
			}
		~COpenClipboard( ) {
			if ( m_open ) {
				VERIFY( CloseClipboard( ) );
				}
			}
		private:
		const BOOL m_open;
		};

	
}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC( COptionsPropertySheet, CPropertySheet )

BOOL COptionsPropertySheet::OnInitDialog() {
	const BOOL bResult = CPropertySheet::OnInitDialog( );
	
	CRect rc;
	GetWindowRect( rc );
	auto pt = rc.TopLeft( );
	CPersistence::GetConfigPosition( pt );
	CRect rc2( pt, rc.Size( ) );
	MoveWindow( rc2 );

	VERIFY( SetActivePage( CPersistence::GetConfigPage( GetPageCount( ) - 1 ) ) );
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

CMySplitterWnd::CMySplitterWnd( _In_z_ PCWSTR name ) : m_persistenceName( name ), m_splitterPos( 0.5 ), m_wasTrackedByUser( false ), m_userSplitterPos( 0.5 ) {
	CPersistence::GetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
	}


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
				m_splitterPos = static_cast< DOUBLE >( cxLeft ) / static_cast< DOUBLE >( rcClient.Width( ) );
				}
			}
		else {
			INT dummy = 0;
			INT cyUpper = 0;
			GetRowInfo( 0, cyUpper, dummy );
	
			if ( ( rcClient.Height( ) ) > 0 ) {
				m_splitterPos = static_cast< DOUBLE >( cyUpper ) / static_cast< DOUBLE >( rcClient.Height( ) );
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
			auto cxLeft = static_cast<INT>( pos * ( rcClient.Width( ) ) );
			if ( cxLeft >= 0 ) {
				SetColumnInfo( 0, cxLeft, 0 );
				RecalcLayout( );
				}
			}
		}
	else {
		ASSERT( m_pRowInfo != NULL );
		if ( m_pRowInfo != NULL ) {
			auto cyUpper = static_cast<INT>( pos * ( rcClient.Height( ) ) );
			if ( cyUpper >= 0 ) {
				SetRowInfo( 0, cyUpper, 0 );
				RecalcLayout( );
				}
			}
		}
	}

void CMySplitterWnd::OnDestroy( ) {
	CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
	CSplitterWnd::OnDestroy( );
	}


void CMySplitterWnd::RestoreSplitterPos(_In_ const DOUBLE posIfVirgin) {
	SetSplitterPos( ( m_wasTrackedByUser ) ? m_userSplitterPos : posIfVirgin );
	}

void CMySplitterWnd::OnSize( const UINT nType, const INT cx, const INT cy ) {
	if ( GetColumnCount( ) > 1 ) {
		INT cxLeft = static_cast< INT >( cx * m_splitterPos );
		if ( cxLeft > 0 ) {
			SetColumnInfo( 0, cxLeft, 0 );
			}
		}
	else {
		INT cyUpper = static_cast<INT>( cy * m_splitterPos );
		if ( cyUpper > 0 ) {
			SetRowInfo( 0, cyUpper, 0 );
			}
		}
	CSplitterWnd::OnSize( nType, cx, cy );
	}

void CDeadFocusWnd::Create(_In_ CWnd* parent) {
	CRect rc( 0, 0, 0, 0 );
	VERIFY( CWnd::Create( AfxRegisterWndClass( 0, 0, 0, 0 ), _T( "_deadfocus" ), WS_CHILD, rc, parent, IDC_DEADFOCUS ) );
	}

BEGIN_MESSAGE_MAP(CDeadFocusWnd, CWnd)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CDeadFocusWnd::OnKeyDown( const UINT nChar, const UINT /* nRepCnt */, const UINT /* nFlags */ ) {
	if ( nChar == VK_TAB ) {
		GetMainFrame( )->MoveFocus( focus::LOGICAL_FOCUS::LF_DIRECTORYLIST );
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

_Ret_maybenull_
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
	
	//VERIFY( m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC ) );

	UINT indicators[ INDICATORS_NUMBER ] = { ID_SEPARATOR, ID_INDICATOR_MEMORYUSAGE };


	VERIFY( m_wndStatusBar.Create( this ) );
	VERIFY( m_wndStatusBar.SetIndicators( indicators, INDICATORS_NUMBER ) );
	
	m_wndDeadFocus.Create( this );

	//m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
	EnableDocking( CBRS_ALIGN_ANY );
	//DockControlBar( &m_wndToolBar );

	LoadBarState( CPersistence::GetBarStateSection( ) );
	ShowControlBar( &m_wndStatusBar, CPersistence::GetShowStatusbar( ), false );
	TRACE( _T( "sizeof CItemBranch: %I64u\r\n" ), static_cast< std::uint64_t >( sizeof( CItemBranch ) ) );

	return 0;
	}

void CMainFrame::InitialShowWindow() {
	WINDOWPLACEMENT wp;
	wp.length = sizeof( wp );
	VERIFY( GetWindowPlacement( &wp ) );
	CPersistence::GetMainWindowPlacement( wp );
	//MakeSaneShowCmd( wp.showCmd );
	if ( wp.showCmd != SW_SHOWMAXIMIZED ) {
		wp.showCmd = SW_SHOWNORMAL;
		}
	VERIFY( SetWindowPlacement( &wp ) );
	}

void CMainFrame::OnClose() {
	const auto qpc_1 = help_QueryPerformanceCounter( );
	CWaitCursor wc;

	// It's too late, to do this in OnDestroy(). Because the toolbar, if undocked, is already destroyed in OnDestroy(). So we must save the toolbar state here in OnClose().
	SaveBarState( CPersistence::GetBarStateSection( ) );
	CPersistence::SetShowStatusbar( ( m_wndStatusBar.GetStyle( ) bitand WS_VISIBLE ) != 0 );

#ifdef _DEBUG
	// avoid memory leaks and show hourglass while deleting the tree
	VERIFY( GetDocument( )->OnNewDocument( ) );
#endif

	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		Document->ForgetItemTree( );
		}
	CFrameWnd::OnClose( );
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast<double>( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast<double>( 1.0 ) / static_cast<double>( qpf.QuadPart ) ) );
	ASSERT( timing != 0 );
#ifndef DEBUG
			UNREFERENCED_PARAMETER( timing );
#endif
	TRACE( _T( "OnClose timing: %f\r\n" ), timing );
	auto pmc = zeroInitPROCESS_MEMORY_COUNTERS( );
	pmc.cb = sizeof( pmc );

	if ( GetProcessMemoryInfo( GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		TRACE( _T( "GetProcessMemoryInfo: %I64u\r\n" ), pmc.WorkingSetSize );
		}
	else {
		TRACE( _T( "GetProcessMemoryInfo failed!!\r\n" ) );
		}
	}

void CMainFrame::OnDestroy() {
	auto wp = zeroInitWINDOWPLACEMENT( );
	GetWindowPlacement( &wp );
	CPersistence::SetMainWindowPlacement( wp );
	const auto TypeView  = GetTypeView( );
	const auto GraphView = GetGraphView( );
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
	VERIFY( m_wndSubSplitter.CreateStatic( &m_wndSplitter, static_cast<INT>( 1 ), static_cast<INT>( 2 ), WS_CHILD | WS_VISIBLE | WS_BORDER, static_cast<UINT>( m_wndSplitter.IdFromRowCol( 0, 0 ) ) ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 0, RUNTIME_CLASS( CDirstatView ), CSize( 700, 500 ), pContext ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTypeView ), CSize( 100, 500 ), pContext ) );

	//MinimizeGraphView( );
	m_wndSplitter.SetSplitterPos( 1.0 );
	//MinimizeTypeView ( );
	m_wndSubSplitter.SetSplitterPos( 1.0 );

	const auto TypeView  = GetTypeView( );
	const auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		TypeView->ShowTypes( CPersistence::GetShowFileTypes( ) );
		}
	if ( GraphView != NULL ) {
		GraphView->m_showTreemap = CPersistence::GetShowTreemap( );
		}
	return TRUE;
	}

void CMainFrame::RestoreTypeView() {
	const auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		if ( thisTypeView->m_showTypes ) {
			m_wndSubSplitter.RestoreSplitterPos( 0.72 );
			VERIFY( thisTypeView->RedrawWindow( ) );
			}
		}
	}


void CMainFrame::RestoreGraphView() {
	const auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		if ( thisGraphView->m_showTreemap ) {
			m_wndSplitter.RestoreSplitterPos( 0.4 );
#ifdef PERF_DEBUG_SLEEP
			Sleep( 1000 );
#endif
			TRACE( _T( "Drawing Empty view...\r\n" ) );

#ifdef DEBUG
			const auto emptyViewTiming_1 = help_QueryPerformanceCounter( );
			thisGraphView->DrawEmptyView( );
			const auto emptyViewTiming_2 = help_QueryPerformanceCounter( );
#endif
			const LARGE_INTEGER timingFrequency = help_QueryPerformanceFrequency( );
			const DOUBLE adjustedTimingFrequency = ( ( DOUBLE ) 1.00 ) / timingFrequency.QuadPart;

#ifdef DEBUG
			const DOUBLE timeToDrawEmptyWindow = ( emptyViewTiming_2.QuadPart - emptyViewTiming_1.QuadPart ) * adjustedTimingFrequency;
			TRACE( _T( "Done drawing empty view. Timing: %f\r\n" ), timeToDrawEmptyWindow );
#endif


			TRACE( _T( "Drawing treemap...\r\n" ) );
			const auto startDrawTime = help_QueryPerformanceCounter( );

			VERIFY( thisGraphView->RedrawWindow( ) );
			const auto endDrawTime = help_QueryPerformanceCounter( );
			
			const DOUBLE timeToDrawWindow = ( endDrawTime.QuadPart - startDrawTime.QuadPart ) * adjustedTimingFrequency;
			TRACE( _T( "Finished drawing treemap! Timing:: %f\r\n" ), timeToDrawWindow );
#ifdef PERF_DEBUG_SLEEP
			Sleep( 1000 );
#endif

			const auto searchingTime = GetDocument( )->m_searchTime;

			const rsize_t debug_str_size = 100;
			wchar_t searching_done_str[ debug_str_size ] = { 0 };
			const auto printf_res_1 = _snwprintf_s( searching_done_str, debug_str_size, L"WDS: searching time: %f\r\n", searchingTime );
			ASSERT( printf_res_1 != -1 );

#ifndef DEBUG
			UNREFERENCED_PARAMETER( printf_res_1 );
#endif

			wchar_t drawing_start_str[ debug_str_size ] = { 0 };
			const auto printf_res_2 = _snwprintf_s( drawing_start_str, debug_str_size, L"WDS: startDrawTime: %lld\r\n", startDrawTime.QuadPart );
			ASSERT( printf_res_2 != -1 );

#ifndef DEBUG
			UNREFERENCED_PARAMETER( printf_res_2 );
#endif


			wchar_t freq_str[ debug_str_size ] = { 0 };
			const auto printf_res_3 = _snwprintf_s( freq_str, debug_str_size, L"WDS: timingFrequency: %lld\r\n", timingFrequency.QuadPart );
			ASSERT( printf_res_3 != -1 );

#ifndef DEBUG
			UNREFERENCED_PARAMETER( printf_res_3 );
#endif

			wchar_t drawing_done_str[ debug_str_size ] = { 0 };
			const auto printf_res_4 = _snwprintf_s( drawing_done_str, debug_str_size, L"WDS: endDrawTime:   %lld\r\n", endDrawTime.QuadPart );
			ASSERT( printf_res_4 != -1 );

#ifndef DEBUG
			UNREFERENCED_PARAMETER( printf_res_4 );
#endif


			OutputDebugStringW( searching_done_str );
			OutputDebugStringW( drawing_start_str );
			OutputDebugStringW( drawing_done_str );
			OutputDebugStringW( freq_str );

			
			
			const auto avg_name_leng = GetDocument( )->m_rootItem->averageNameLength( );
			ASSERT( timeToDrawWindow != 0 );
			if ( m_lastSearchTime == -1 ) {	
				m_lastSearchTime = searchingTime;
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng );//else the search time compounds whenever the time is written to the status bar
				}
			else {
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng );
				}
			}
		}
	}

_Must_inspect_result_ _Ret_maybenull_ CDirstatView* CMainFrame::GetDirstatView( ) const {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 0 );
	return DYNAMIC_DOWNCAST( CDirstatView, pWnd );
	}

_Must_inspect_result_ _Ret_maybenull_ CGraphView* CMainFrame::GetGraphView( ) const {
	const auto pWnd = m_wndSplitter.GetPane( 1, 0 );
	return DYNAMIC_DOWNCAST( CGraphView, pWnd );
	}

_Must_inspect_result_ _Ret_maybenull_ CTypeView* CMainFrame::GetTypeView( ) const {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 1 );
	return DYNAMIC_DOWNCAST( CTypeView, pWnd );
	}

LRESULT CMainFrame::OnEnterSizeMove( const WPARAM, const LPARAM ) {
	const auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( true );
		}
	return 0;
	}

LRESULT CMainFrame::OnExitSizeMove( const WPARAM, const LPARAM ) {
	const auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		GraphView->SuspendRecalculation( false );
		}
	return 0;
	}

void CMainFrame::CopyToClipboard( _In_ const std::wstring psz ) const {
	COpenClipboard clipboard( const_cast<CMainFrame*>( this ) );
	const rsize_t strSizeInBytes = ( ( psz.length( ) + 1 ) * sizeof( WCHAR ) );

	const HGLOBAL h = GlobalAlloc( GMEM_MOVEABLE bitand GMEM_ZEROINIT, strSizeInBytes );
	if ( h == NULL ) {
		displayWindowsMsgBoxWithMessage( global_strings::global_alloc_failed );
		TRACE( L"%s\r\n", global_strings::global_alloc_failed );
		return;
		}

	const auto lp = GlobalLock( h );
	if ( lp == NULL ) {
		displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"GlobalLock failed!" ) ) );
		return;
		}

	auto strP = static_cast< PWSTR >( lp );

	const HRESULT strCopyRes = StringCchCopyW( strP, ( psz.length( ) + 1 ), psz.c_str( ) );
	if ( !SUCCEEDED( strCopyRes ) ) {
		if ( strCopyRes == STRSAFE_E_INVALID_PARAMETER ) {
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( global_strings::string_cch_copy_failed ) + std::wstring( L"(STRSAFE_E_INVALID_PARAMETER)" ) ) );
			}
		if ( strCopyRes == STRSAFE_E_INSUFFICIENT_BUFFER ) {
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( global_strings::string_cch_copy_failed ) + std::wstring( L"(STRSAFE_E_INSUFFICIENT_BUFFER)" ) ) );
			}
		else {
			displayWindowsMsgBoxWithMessage( global_strings::string_cch_copy_failed );
			}
		const BOOL unlock_res = GlobalUnlock( h );
		strP = NULL;
		const auto last_err = GetLastError( );
		if ( unlock_res == 0 ) {
			ASSERT( last_err == NO_ERROR );
#ifndef DEBUG
			UNREFERENCED_PARAMETER( last_err );
#endif

			}
		return;
		}

	if ( GlobalUnlock( h ) == 0 ) {
		const auto err = GetLastError( );
		if ( err != NO_ERROR ) {
			displayWindowsMsgBoxWithMessage( std::move( std::wstring( L"GlobalUnlock failed!" ) ) );
			return;
			}
		}
	//wtf is going on here?
	UINT uFormat = CF_TEXT;
	uFormat = CF_UNICODETEXT;
		
	if ( NULL == SetClipboardData( uFormat, h ) ) {
		displayWindowsMsgBoxWithMessage( global_strings::cannot_set_clipboard_data );
		TRACE( L"%s\r\n", global_strings::cannot_set_clipboard_data );
		return;
		}
	}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) {
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
	}


_At_( lf, _Pre_satisfies_( ( lf == focus::LOGICAL_FOCUS::LF_NONE ) || ( lf == focus::LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == focus::LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::SetLogicalFocus(_In_ const focus::LOGICAL_FOCUS lf) {
	if ( lf != m_logicalFocus ) {
		m_logicalFocus = lf;
		SetSelectionMessageText( );

		//reinterpret_cast< CDocument* >( GetDocument( ) )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED );
		}
	}
_At_( lf, _Pre_satisfies_( ( lf == focus::LOGICAL_FOCUS::LF_NONE ) || ( lf == focus::LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == focus::LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::MoveFocus( _In_ const focus::LOGICAL_FOCUS lf ) {
	if ( lf == focus::LOGICAL_FOCUS::LF_NONE ) {
		SetLogicalFocus( focus::LOGICAL_FOCUS::LF_NONE );
		m_wndDeadFocus.SetFocus( );
		}
	else if ( lf == focus::LOGICAL_FOCUS::LF_DIRECTORYLIST ) {
		const auto DirstatView = GetDirstatView( );
		if ( DirstatView != NULL ) {
			DirstatView->SetFocus( );
			}
		}
	else if ( lf == focus::LOGICAL_FOCUS::LF_EXTENSIONLIST ) {
		const auto TypeView = GetTypeView( );
		if ( TypeView != NULL ) {
			TypeView->SetFocus( );
			}
		}
	}

size_t CMainFrame::getExtDataSize( ) const {
	const auto Document = GetDocument( );
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
	DOUBLE populateTiming = -1;
	DOUBLE averageExtLeng = -1;
	const auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		populateTiming = TypeView->m_extensionListControl.adjustedTiming;
		averageExtLeng = TypeView->m_extensionListControl.averageExtensionNameLength;
		}
	
	const auto extDataSize = getExtDataSize( );
	
		if ( ( searchTiming > 0.00 ) && ( drawTiming > 0.00 ) && ( populateTiming > 0.00 ) ) {
			timeText.Format( _T( "Finding files took %.3f sec. Drawing took %.3f sec. Populating 'file types' took %.3f sec. Total time: %.4f sec. # of file types: %u. Avg name length: %.2f. Avg extension length: %.2f. SSO threshold: %u" ), searchTiming, drawTiming, populateTiming, ( searchTiming + drawTiming + populateTiming ), unsigned( extDataSize ), fileNameLength, averageExtLeng, unsigned( SSO_THRESHOLD_BUF_SIZE ) );
			}
		else {
			timeText.Format( _T( "I had trouble with QueryPerformanceCounter, and can't provide timing. The number of file types: %u. Avg name length: %.2f. Avg extension length: %.2f. SSO threshold: %u" ), unsigned( extDataSize ), fileNameLength, averageExtLeng, unsigned( SSO_THRESHOLD_BUF_SIZE ) );
			}
	SetMessageText( timeText );
	m_drawTiming = std::wstring( timeText.GetString( ) );
	}

void CMainFrame::SetSelectionMessageText() {
	switch ( m_logicalFocus )
	{
		case focus::LOGICAL_FOCUS::LF_NONE:
			SetMessageText( m_drawTiming.c_str( ) );
			break;
		case focus::LOGICAL_FOCUS::LF_DIRECTORYLIST:
			{
			auto Document = GetDocument( );
			if ( Document != NULL ) {
				const auto Selection = Document->m_selectedItem;
				if ( Selection != NULL ) {
					SetMessageText( Selection->GetPath( ).c_str( ) );
					}
				else {
					//SetMessageText(L"are we?");
					SetMessageText( m_drawTiming.c_str( ) );
					}
				}
			else {
				ASSERT( false );
				SetMessageText( _T( "No document?" ) );
				}
			}
			break;
		case focus::LOGICAL_FOCUS::LF_EXTENSIONLIST:
			SetMessageText( _T("*") + CString( GetDocument( )->m_highlightExtension.c_str( ) ) );
			break;
	}
	}

void CMainFrame::OnUpdateMemoryUsage( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( true );
	const rsize_t ramUsageStrBufferSize = 50;
	wchar_t ramUsageStr[ ramUsageStrBufferSize ] = { 0 };
	
	const HRESULT res = GetApp( )->GetCurrentProcessMemoryInfo( ramUsageStr, ramUsageStrBufferSize );
	if ( !SUCCEEDED( res ) ) {
		rsize_t chars_written = 0;
		write_BAD_FMT( ramUsageStr, chars_written );
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
	const auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		pCmdUI->SetCheck( GraphView->m_showTreemap );
		}
	ASSERT( GraphView != NULL );
	}

void CMainFrame::OnViewShowtreemap() {
	const auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		thisGraphView->m_showTreemap = !thisGraphView->m_showTreemap;
		if ( thisGraphView->m_showTreemap ) {
			RestoreGraphView( );
			}
		else {
			//MinimizeGraphView( );
			m_wndSplitter.SetSplitterPos( 1.0 );
			}
		}
	ASSERT( thisGraphView != NULL );
	}

void CMainFrame::OnUpdateViewShowfiletypes(CCmdUI *pCmdUI) {
	const auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		pCmdUI->SetCheck( TypeView->m_showTypes );
		}
	ASSERT( TypeView != NULL );
	}

void CMainFrame::OnViewShowfiletypes() {
	const auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		thisTypeView->ShowTypes( !thisTypeView->m_showTypes );
		if ( thisTypeView->m_showTypes ) {
			RestoreTypeView( );
			}
		else {
			//MinimizeTypeView( );
			m_wndSubSplitter.SetSplitterPos( 1.0 );
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
	const auto Options = GetOptions( );
	Options->SaveToRegistry( );
	}

void CMainFrame::OnSysColorChange() {
	CFrameWnd::OnSysColorChange( );
	const auto DirstatView = GetDirstatView( );
	if ( DirstatView != NULL ) {
		DirstatView->SysColorChanged( );
		}
	ASSERT( DirstatView != NULL );
	const auto TypeView = GetTypeView( );
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
