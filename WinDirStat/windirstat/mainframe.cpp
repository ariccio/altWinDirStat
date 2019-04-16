// mainframe.cpp	- Implementation of WDSSplitterWnd, CPacmanControl and CMainFrame
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once

#ifndef WDS_MAINFRAME_CPP
#define WDS_MAINFRAME_CPP

WDS_FILE_INCLUDE_MESSAGE

#include "graphview.h"
#include "dirstatview.h"
#include "datastructures.h"
#include "macros_that_scare_small_children.h"


//encourage inter-procedural optimization (and class-hierarchy analysis!)
//#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "typeview.h"

#include "pagetreemap.h"
#include "pagegeneral.h"
#include "mainframe.h"
#include "dirstatdoc.h"
#include "options.h"
#include "windirstat.h"


#include "globalhelpers.h"
#include "ScopeGuard.h"

#include "stringformatting.h"

namespace {
	// This must be synchronized with the IDR_MAINFRAME menu
	enum TOPLEVELMENU {
		TLM_FILE,
		TLM_EDIT,
		TLM_TREEMAP,
		TLM_VIEW,
		TLM_HELP
		};



	const UINT indicators[ ] = { ID_SEPARATOR, ID_INDICATOR_MEMORYUSAGE };
	
	template<size_t count>
	void SetIndicators( CStatusBar* const status_bar, const UINT( &indicators_array )[ count ] ) {
		static_assert( sizeof( indicators_array ) == ( count * sizeof( UINT ) ), "Bad SetIndicators argument!" );
		VERIFY( status_bar->SetIndicators( indicators_array, count ) );
		}

	const rsize_t debug_str_size = 100u;
	
	void debug_output_searching_time( _In_ const double searchingTime ) {
		if ( searchingTime == DBL_MAX ) {
			::OutputDebugStringA( "WDS: searching time is not yet initialized!\r\n" );
			return;
			}
		//OutputDebugStringW converts to ASCII internally, so we'll just use char.
		_Null_terminated_ char searching_done_str[ debug_str_size ] = { 0 };
		const auto printf_res_1 = _snprintf_s( searching_done_str, debug_str_size, _TRUNCATE, "WDS: searching time: %f\r\n", searchingTime );
		ASSERT( printf_res_1 != -1 );
		::OutputDebugStringA( searching_done_str );

#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_1 );
#endif
		}
	
	void debug_output_frequency( _In_ const std::int64_t m_frequency ) {
		//OutputDebugStringW converts to ASCII internally, so we'll just use char.
		_Null_terminated_ char freq_str[ debug_str_size ] = { 0 };
		const auto printf_res_3 = _snprintf_s( freq_str, debug_str_size, _TRUNCATE, "WDS: timing frequency: %lld\r\n", m_frequency );
		ASSERT( printf_res_3 != -1 );
		::OutputDebugStringA( freq_str );
#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_3 );
#endif
		}

	void debug_output_time_to_draw_empty_window( _In_ const double timeToDrawEmptyWindow ) {
		//OutputDebugStringW converts to ASCII internally, so we'll just use char.
		_Null_terminated_ char drawing_done_str[ debug_str_size ] = { 0 };
		const auto printf_res_4 = _snprintf_s( drawing_done_str, debug_str_size, _TRUNCATE, "WDS: time to draw window:   %f\r\n", timeToDrawEmptyWindow );
		ASSERT( printf_res_4 != -1 );
		::OutputDebugStringA( drawing_done_str );
#ifndef DEBUG
		UNREFERENCED_PARAMETER( printf_res_4 );
#endif
		}

	void output_debugging_info( _In_ const double searchingTime, _In_ const std::int64_t frequency, _In_ const double timeToDrawEmptyWindow ) {
		debug_output_searching_time( searchingTime );
		debug_output_frequency( frequency );
		debug_output_time_to_draw_empty_window( timeToDrawEmptyWindow );
		}

	}

/*
From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:688:
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL, class_init }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); }

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:704:
#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL, NULL)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:598:
#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:594:
#define _RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))

Sooo...
	IMPLEMENT_DYNAMIC(WDSOptionsPropertySheet, CPropertySheet)
	--becomes--
	IMPLEMENT_RUNTIMECLASS(WDSOptionsPropertySheet, CPropertySheet, 0xFFFF, NULL, NULL)
	--becomes--
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass WDSOptionsPropertySheet::classWDSOptionsPropertySheet = { \
		"WDSOptionsPropertySheet", sizeof(class WDSOptionsPropertySheet), 0xFFFF, NULL, \
			RUNTIME_CLASS(CPropertySheet), NULL, NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(WDSOptionsPropertySheet); }

And...
	RUNTIME_CLASS(CPropertySheet)
	--becomes--
	_RUNTIME_CLASS(CPropertySheet)
	--becomes--
	((CRuntimeClass*)(&CPropertySheet::classCPropertySheet))

And...
	RUNTIME_CLASS(WDSOptionsPropertySheet)
	--becomes--
	_RUNTIME_CLASS(WDSOptionsPropertySheet)
	--becomes--
	((CRuntimeClass*)(&WDSOptionsPropertySheet::classWDSOptionsPropertySheet))
*/
//IMPLEMENT_DYNAMIC(WDSOptionsPropertySheet, CPropertySheet)
AFX_COMDAT const CRuntimeClass WDSOptionsPropertySheet::classWDSOptionsPropertySheet = {
		"WDSOptionsPropertySheet" /*m_lpszClassName*/,
		sizeof(struct WDSOptionsPropertySheet) /*m_nObjectSize*/,
		0xFFFF /*wSchema*/,
		NULL /*pfnNew*/,
		(const_cast<CRuntimeClass*>(&CPropertySheet::classCPropertySheet)) /*RUNTIME_CLASS(CPropertySheet)*/ /*m_pBaseClass*/,
		NULL,
		NULL };

CRuntimeClass* WDSOptionsPropertySheet::GetRuntimeClass() const {
	return (const_cast<CRuntimeClass*>(&WDSOptionsPropertySheet::classWDSOptionsPropertySheet)); // RUNTIME_CLASS(WDSOptionsPropertySheet);
	}


BOOL WDSOptionsPropertySheet::OnInitDialog( ) {
	const BOOL bResult = CPropertySheet::OnInitDialog( );

	RECT rc = { 0 };

	//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	ASSERT( ::IsWindow( m_hWnd ) );

	//GetWindowRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633519.aspx
	//If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	VERIFY( ::GetWindowRect( m_hWnd, &rc ) );

	WTL::CPoint pt = CRect( rc ).TopLeft( );
	
	CPersistence::GetConfigPosition( &pt );
	CRect rc2( pt, CRect( rc ).Size( ) );

	ASSERT( m_pCtrlSite == NULL );

	//MoveWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633534.aspx
	//If [MoveWindow] succeeds, the return value is nonzero.
	//If the function fails, the return value is zero. To get extended error information, call GetLastError.
	//If [MoveWindow] succeeds, the return value is nonzero.
	VERIFY( ::MoveWindow( m_hWnd, rc2.left, rc2.top, ( rc2.right - rc2.left ), ( rc2.bottom - rc2.top ), TRUE ) );

	VERIFY( CPropertySheet::SetActivePage( CPersistence::GetConfigPage( GetPageCount( ) - 1 ) ) );
	return bResult;
	}

BOOL WDSOptionsPropertySheet::OnCommand( _In_ WPARAM wParam, _In_ LPARAM lParam ) {
	CPersistence::SetConfigPage( CPropertySheet::GetActiveIndex( ) );

	RECT rc = { 0 };

	//GetWindowRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633519.aspx
	//If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	VERIFY( ::GetWindowRect( m_hWnd, &rc ) );

	CPersistence::SetConfigPosition( CRect(rc).TopLeft( ) );
	ASSERT( m_pCtrlSite == NULL );
	//INT cmd = LOWORD( wParam );
	return CPropertySheet::OnCommand( wParam, lParam );
	}

/////////////////////////////////////////////////////////////////////////////


WDSSplitterWnd::WDSSplitterWnd( _In_z_ PCWSTR const name ) : m_persistenceName( name ) {
	CPersistence::GetSplitterPos( m_persistenceName, &m_wasTrackedByUser, &m_userSplitterPos );
	}


BEGIN_MESSAGE_MAP( WDSSplitterWnd, CSplitterWnd )
	ON_WM_SIZE( )
END_MESSAGE_MAP( )


void WDSSplitterWnd::StopTracking( _In_ BOOL bAccept ) {
	CSplitterWnd::StopTracking( bAccept );
	if ( !bAccept ) {
		return;
		}
	RECT rcClient = { 0, 0, 0, 0 };
	//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	ASSERT( ::IsWindow( m_hWnd ) );
	
	//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
	//Return value: If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	VERIFY( ::GetClientRect( m_hWnd, &rcClient ) );
	
	auto guard = WDS_SCOPEGUARD_INSTANCE( [ &] { CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); } );

	INT dummy = 0;
	if ( CSplitterWnd::GetColumnCount( ) > 1 ) {
		INT cxLeft = 0;
		CSplitterWnd::GetColumnInfo( 0, cxLeft, dummy );

		if ( ( rcClient.right - rcClient.left ) > 0 ) {
			m_splitterPos = static_cast< DOUBLE >( cxLeft ) / static_cast< DOUBLE >( rcClient.right - rcClient.left );
			}
		m_wasTrackedByUser = true;
		m_userSplitterPos = m_splitterPos;
		return;
		}
	INT cyUpper = 0;
	CSplitterWnd::GetRowInfo( 0, cyUpper, dummy );
	if ( ( rcClient.bottom - rcClient.top ) > 0 ) {
		m_splitterPos = static_cast< DOUBLE >( cyUpper ) / static_cast< DOUBLE >( rcClient.bottom - rcClient.top );
		}
	m_wasTrackedByUser = true;
	m_userSplitterPos = m_splitterPos;
	}

void WDSSplitterWnd::SetSplitterPos( _In_ const DOUBLE pos ) noexcept {
	m_splitterPos = pos;
	RECT rcClient = { 0, 0, 0, 0 };

	//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	ASSERT( ::IsWindow( m_hWnd ) );
	
	//GetClientRect function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633503.aspx
	//Return value: If the function succeeds, the return value is nonzero.
	//If the function fails, the return value is zero.
	//To get extended error information, call GetLastError.
	VERIFY( ::GetClientRect( m_hWnd, &rcClient ) );

	//auto splitter_persist = scopeGuard( [&]{ CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); }, __FILE__, __FUNCSIG__, __LINE__ );
	//WDS_SCOPEGUARD_INSTANCE

	//TODO: BUGBUG: THE FOLLOWING LINE SHOULD NOT BE COMMENTED OUT!
	auto splitter_persist = WDS_SCOPEGUARD_INSTANCE( [&]{ CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos ); } );

	if ( CSplitterWnd::GetColumnCount( ) > 1 ) {
		ASSERT( m_pColInfo != NULL );
		if ( m_pColInfo == NULL ) {
			return;
			}
		const auto cxLeft = static_cast< INT >( pos * ( rcClient.right - rcClient.left ) );
		if ( cxLeft >= 0 ) {
			CSplitterWnd::SetColumnInfo( 0, cxLeft, 0 );
			CSplitterWnd::RecalcLayout( );
			return;
			}
		return;
		}
	ASSERT( m_pRowInfo != NULL );
	if ( m_pRowInfo == NULL ) {
		return;
		}
	const auto cyUpper = static_cast< INT >( pos * ( rcClient.bottom - rcClient.top ) );
	if ( cyUpper >= 0 ) {
		CSplitterWnd::SetRowInfo( 0, cyUpper, 0 );
		CSplitterWnd::RecalcLayout( );
		return;
		}
	}

void WDSSplitterWnd::OnSize( const UINT nType, const INT cx, const INT cy ) {
	auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ CSplitterWnd::OnSize( nType, cx, cy ); } );
	if ( CSplitterWnd::GetColumnCount( ) > 1 ) {
		const INT cxLeft = static_cast< INT >( cx * m_splitterPos );
		if ( cxLeft > 0 ) {
			CSplitterWnd::SetColumnInfo( 0, cxLeft, 0 );
			return;
			}
		return;
		}
	const INT cyUpper = static_cast< INT >( cy * m_splitterPos );
	if ( cyUpper > 0 ) {
		CSplitterWnd::SetRowInfo( 0, cyUpper, 0 );
		return;
		}
	}

LRESULT CDeadFocusWnd::OnKeyDown( UINT /*nMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled ) {
	const UINT nChar = static_cast<UINT>( wParam );
	if ( nChar == VK_TAB ) {
		m_frameptr->MoveFocus( LOGICAL_FOCUS::LF_DIRECTORYLIST );
		bHandled = TRUE;
		return 0;
		}
	bHandled = FALSE;
	return 0;
	}

/////////////////////////////////////////////////////////////////////////////

/*
From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:598:
#define RUNTIME_CLASS(class_name) _RUNTIME_CLASS(class_name)

From C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:594:
#define _RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))

C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:707:
#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject, NULL)

C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.20.27508\atlmfc\include\afx.h:688:
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew, class_init) \
	AFX_COMDAT const CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL, class_init }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); }


Sooo...
	IMPLEMENT_DYNCREATE( CMainFrame, CFrameWnd )
		--becomes--
	CObject* PASCAL CMainFrame::CreateObject() \
		{ return new CMainFrame; } \
	IMPLEMENT_RUNTIMECLASS(CMainFrame, CFrameWnd, 0xFFFF, \
		CMainFrame::CreateObject, NULL)
		--becomes--
	CObject* PASCAL CMainFrame::CreateObject() \
		{ return new CMainFrame; } \
	AFX_COMDAT const CRuntimeClass CMainFrame::classCMainFrame = { \
		"CMainFrame", sizeof(class CMainFrame), 0xFFFF, CMainFrame::CreateObject, \
			RUNTIME_CLASS(CFrameWnd), NULL, NULL }; \
	CRuntimeClass* CMainFrame::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(CMainFrame); }

And...
	RUNTIME_CLASS(CFrameWnd)
		--becomes--
	_RUNTIME_CLASS(CFrameWnd)
		--becomes--
	((CRuntimeClass*)(&CFrameWnd::classCFrameWnd))

And...
	RUNTIME_CLASS(CMainFrame)
		--becomes--
	_RUNTIME_CLASS(CMainFrame)
		--becomes--
	((CRuntimeClass*)(&CMainFrame::classCMainFrame))
*/

//IMPLEMENT_DYNCREATE( CMainFrame, CFrameWnd )

CObject* PASCAL CMainFrame::CreateObject() noexcept {
	return new CMainFrame;
	}

AFX_COMDAT const CRuntimeClass CMainFrame::classCMainFrame = {
	"CMainFrame" /*m_lpszClassName*/,
	sizeof(class CMainFrame) /*m_nObjectSize*/,
	0xFFFF /*wSchema*/,
	CMainFrame::CreateObject /*pfnNew*/,
	(const_cast<CRuntimeClass*>(&CFrameWnd::classCFrameWnd)) /*RUNTIME_CLASS(CFrameWnd)*/ /*m_pBaseClass*/,
	NULL /*m_pNextClass*/,
	NULL /*class_init*/
	};

CRuntimeClass* CMainFrame::GetRuntimeClass() const {
	return (const_cast<CRuntimeClass*>(&CMainFrame::classCMainFrame)) /*RUNTIME_CLASS(CMainFrame)*/;
	}



BEGIN_MESSAGE_MAP( CMainFrame, CFrameWnd )
	ON_WM_CREATE( )
	ON_MESSAGE( WM_ENTERSIZEMOVE, &( CMainFrame::OnEnterSizeMove ) )
	ON_MESSAGE( WM_EXITSIZEMOVE, &( CMainFrame::OnExitSizeMove ) )
	ON_WM_CLOSE( )
	ON_WM_INITMENUPOPUP( )
	ON_UPDATE_COMMAND_UI( ID_INDICATOR_MEMORYUSAGE, &( CMainFrame::OnUpdateMemoryUsage ) )
	ON_WM_SIZE( )
	ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWTREEMAP, &( CMainFrame::OnUpdateViewShowtreemap ) )
	ON_COMMAND( ID_VIEW_SHOWTREEMAP, &( CMainFrame::OnViewShowtreemap ) )
	ON_UPDATE_COMMAND_UI( ID_VIEW_SHOWFILETYPES, &( CMainFrame::OnUpdateViewShowfiletypes ) )
	ON_COMMAND( ID_VIEW_SHOWFILETYPES, &( CMainFrame::OnViewShowfiletypes ) )
	ON_COMMAND( ID_CONFIGURE, &( CMainFrame::OnConfigure ) )
	ON_COMMAND( ID_SHOW_GRID_NAME_HACK, &( CMainFrame::OnViewShowGrid ) )
	ON_UPDATE_COMMAND_UI( ID_SHOW_GRID_NAME_HACK, &( CMainFrame::OnUpdateViewShowGrid ) )
	ON_WM_DESTROY( )
	//ON_COMMAND(ID_TREEMAP_HELPABOUTTREEMAPS, OnTreemapHelpabouttreemaps)
	ON_WM_SYSCOLORCHANGE( )
END_MESSAGE_MAP( )

CMainFrame* CMainFrame::_theFrame;

_Ret_maybenull_
CMainFrame* CMainFrame::GetTheFrame( ) {
	return _theFrame;
	}

#pragma warning( push )
#pragma warning( disable: 4355 )

CMainFrame::CMainFrame( ) : m_wndSubSplitter( global_strings::sub_split ), m_wndSplitter(global_strings::main_split), m_logicalFocus( LOGICAL_FOCUS::LF_NONE ), m_lastSearchTime(-1), m_wndDeadFocus( this ), m_appptr(nullptr) {// Created by MFC only
	_theFrame = this;
	}
#pragma warning( pop )



INT CMainFrame::OnCreate( const LPCREATESTRUCT lpCreateStruct ) {
	/*
	Initializes the MAIN frame - wherein the rectangular layout, the list of files, and the list of file types are.
	Initializes a few related things, such as the memory display.
	*/

	_theFrame = this;
	m_appptr = GetApp( );
	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}


	VERIFY( m_wndStatusBar.Create( this ) );
	SetIndicators( &m_wndStatusBar, indicators );

	RECT rc = { 0, 0, 0, 0 };

	m_wndDeadFocus.Create( m_hWnd, rc, _T( "_deadfocus" ), WS_CHILD, 0, dead_focus_wnd::IDC_DEADFOCUS, NULL );
	//m_wndDeadFocus.Create( this );

	CFrameWnd::EnableDocking( CBRS_ALIGN_ANY );

	CFrameWnd::LoadBarState( CPersistence::GetBarStateSection( ) );
	CFrameWnd::ShowControlBar( &m_wndStatusBar, CPersistence::GetShowStatusbar( ), false );
	
	TRACE( _T( "sizeof CTreeListItem: %I64u\r\n" ), static_cast< std::uint64_t >( sizeof( CTreeListItem ) ) );
#ifdef DISPLAY_FINAL_CITEMBRANCH_SIZE
#ifndef DEBUG
	if ( IsDebuggerPresent( ) ) {
		const auto size_citembranch = std::to_wstring( sizeof( CTreeListItem ) );
		const std::wstring size_text( L"sizeof CTreeListItem: " + size_citembranch );
		displayWindowsMsgBoxWithMessage( size_text.c_str( ) );
		}
#endif
#endif
	return 0;
	}

void CMainFrame::InitialShowWindow( ) noexcept {
	WINDOWPLACEMENT wp = { 0 };
	wp.length = sizeof( wp );
	VERIFY( CWnd::GetWindowPlacement( &wp ) );
	CPersistence::GetMainWindowPlacement( &wp );
	//MakeSaneShowCmd( wp.showCmd );
	if ( wp.showCmd != SW_SHOWMAXIMIZED ) {
		wp.showCmd = SW_SHOWNORMAL;
		}
	VERIFY( CWnd::SetWindowPlacement( &wp ) );
	}

void CMainFrame::OnClose( ) {
	TRACE( _T( "CMainFrame::OnClose!\r\n" ) );
	const auto qpc_1 = help_QueryPerformanceCounter( );
	//WTL::CWaitCursor wc;

	// It's too late, to do this in OnDestroy(). Because the toolbar, if undocked, is already destroyed in OnDestroy(). So we must save the toolbar state here in OnClose().
	CFrameWnd::SaveBarState( CPersistence::GetBarStateSection( ) );
	CPersistence::SetShowStatusbar( ( m_wndStatusBar.GetStyle( ) bitand WS_VISIBLE ) != 0 );

	const auto Options = GetOptions( );
	Options->SaveToRegistry( );

#ifdef _DEBUG
	// avoid memory leaks and show hourglass while deleting the tree
	//VERIFY( GetDocument( )->OnNewDocument( ) );
#endif

	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		//Document->ForgetItemTree( );
		Document->DeleteContents( );
		}
	CFrameWnd::OnClose( );
	const auto qpc_2 = help_QueryPerformanceCounter( );
	const auto qpf = help_QueryPerformanceFrequency( );
	const auto timing = ( static_cast< double >( qpc_2.QuadPart - qpc_1.QuadPart ) * ( static_cast< double >( 1.0 ) / static_cast< double >( qpf.QuadPart ) ) );
	ASSERT( timing != 0 );
#ifndef DEBUG
	UNREFERENCED_PARAMETER( timing );
#endif
	TRACE( _T( "OnClose timing: %f\r\n" ), timing );
	auto pmc = zero_init_struct<PROCESS_MEMORY_COUNTERS>( );
	pmc.cb = sizeof( pmc );

	if ( ::GetProcessMemoryInfo( ::GetCurrentProcess( ), &pmc, sizeof( pmc ) ) ) {
		TRACE( _T( "GetProcessMemoryInfo: %I64u\r\n" ), static_cast< std::uint64_t >( pmc.WorkingSetSize ) );
		}
	else {
		TRACE( _T( "GetProcessMemoryInfo failed!!\r\n" ) );
		}
	}

void CMainFrame::OnDestroy( ) {
	//auto wp = zeroInitWINDOWPLACEMENT( );
	auto wp = zero_init_struct<WINDOWPLACEMENT>( );
	CWnd::GetWindowPlacement( &wp );
	CPersistence::SetMainWindowPlacement( wp );
	const auto TypeView = GetTypeView( );
	const auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		CPersistence::SetShowFileTypes( TypeView->m_showTypes );
		}
	if ( GraphView != NULL ) {
		CPersistence::SetShowTreemap( GraphView->m_showTreemap );
		}
	CFrameWnd::OnDestroy( );
	}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext ) {
	const SIZE GraphView_size = { 100, 100 };
	const SIZE DirstatView_size = { 700, 500 };
	const SIZE TypeView_size = { 100, 500 };

	VERIFY( m_wndSplitter.CreateStatic( this, 2, 1 ) );
	VERIFY( m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS( CGraphView ), GraphView_size, pContext ) );

	VERIFY( m_wndSubSplitter.CreateStatic( &m_wndSplitter, 1, 2, ( WS_CHILD | WS_VISIBLE | WS_BORDER ), static_cast< UINT >( m_wndSplitter.IdFromRowCol( 0, 0 ) ) ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 0, RUNTIME_CLASS( CDirstatView ), DirstatView_size, pContext ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTypeView ), TypeView_size, pContext ) );

	//MinimizeGraphView( );
	m_wndSplitter.SetSplitterPos( 1.0 );
	//MinimizeTypeView ( );
	m_wndSubSplitter.SetSplitterPos( 1.0 );

	const auto TypeView = GetTypeView( );
	const auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		TypeView->ShowTypes( CPersistence::GetShowFileTypes( ) );
		}
	if ( GraphView != NULL ) {
		GraphView->m_showTreemap = CPersistence::GetShowTreemap( );
		}
	return TRUE;
	}

void CMainFrame::RestoreTypeView( ) noexcept {
	const auto thisTypeView = GetTypeView( );
	if ( thisTypeView == NULL ) {
		return;
		}
	if ( thisTypeView->m_showTypes ) {
		m_wndSubSplitter.RestoreSplitterPos( 0.72 );
		VERIFY( thisTypeView->RedrawWindow( ) );
		}
	}


void CMainFrame::RestoreGraphView( ) noexcept {
	const auto thisGraphView = GetGraphView( );
	if ( thisGraphView == NULL ) {
		return;
		}
	if ( !( thisGraphView->m_showTreemap ) ) {
		return;
		}
	m_wndSplitter.RestoreSplitterPos( 0.4 );

	QPC_timer timer_draw_empty_view;
	timer_draw_empty_view.begin( );
	thisGraphView->DrawEmptyView( );
	timer_draw_empty_view.end( );
	const DOUBLE timeToDrawEmptyWindow = timer_draw_empty_view.total_time_elapsed( );
	TRACE( _T( "Done drawing empty view. Timing: %f\r\nDrawing treemap...\r\n" ), timeToDrawEmptyWindow );
	QPC_timer timer_draw_treemap;
	timer_draw_treemap.begin( );
	VERIFY( thisGraphView->RedrawWindow( ) );
	timer_draw_treemap.end( );

	const DOUBLE timeToDrawWindow = timer_draw_treemap.total_time_elapsed( );
	TRACE( _T( "Finished drawing treemap! Timing: %f\r\n" ), timeToDrawWindow );
	const auto comp_file_timing = GetDocument( )->m_compressed_file_timing;
	const auto searchingTime = GetDocument( )->m_searchTime;
	ASSERT( searchingTime > 0 );
	output_debugging_info( searchingTime, timer_draw_treemap.m_frequency, timeToDrawEmptyWindow );
	double average_name_length_scope_holder = -1;
	const auto root_item = GetDocument( )->m_rootItem.get( );
	if ( root_item != nullptr ) {
		average_name_length_scope_holder = GetDocument( )->m_rootItem->averageNameLength( );
		}
	const auto avg_name_leng = average_name_length_scope_holder;
	ASSERT( timeToDrawWindow != 0 );
	m_lastSearchTime = searchingTime;
	if ( m_lastSearchTime == -1 ) {
		m_lastSearchTime = searchingTime;
		ASSERT( m_lastSearchTime >= comp_file_timing );
		WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng, comp_file_timing );//else the search time compounds whenever the time is written to the status bar
		}
	else {
		WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime, avg_name_leng, comp_file_timing );
		}
	}

_Must_inspect_result_ _Ret_maybenull_ CDirstatView* CMainFrame::GetDirstatView( ) const noexcept {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 0 );
	return static_cast<CDirstatView*>( pWnd );
	}

//cannot be defined in header.
_Must_inspect_result_ _Ret_maybenull_ CGraphView* CMainFrame::GetGraphView( ) const noexcept {
	const auto pWnd = m_wndSplitter.GetPane( 1, 0 );
	return static_cast<CGraphView*>( pWnd );
	}

_Must_inspect_result_ _Ret_maybenull_ CTypeView* CMainFrame::GetTypeView( ) const noexcept {
	const auto pWnd = m_wndSubSplitter.GetPane( 0, 1 );
	return static_cast<CTypeView*>( pWnd );
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

void CMainFrame::OnInitMenuPopup( CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu ) {
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );
	}


_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::SetLogicalFocus( _In_ const LOGICAL_FOCUS lf ) noexcept {
	if ( lf != m_logicalFocus ) {
		m_logicalFocus = lf;
		SetSelectionMessageText( );

		//reinterpret_cast< CDocument* >( GetDocument( ) )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED );
		}
	}
_At_( lf, _Pre_satisfies_( ( lf == LOGICAL_FOCUS::LF_NONE ) || ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) || ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) ) )
void CMainFrame::MoveFocus( _In_ const LOGICAL_FOCUS lf ) noexcept {
	if ( lf == LOGICAL_FOCUS::LF_NONE ) {
		SetLogicalFocus( LOGICAL_FOCUS::LF_NONE );
		m_wndDeadFocus.SetFocus( );
		}
	else if ( lf == LOGICAL_FOCUS::LF_DIRECTORYLIST ) {
		const auto DirstatView = GetDirstatView( );
		if ( DirstatView != NULL ) {
			DirstatView->SetFocus( );
			}
		}
	else if ( lf == LOGICAL_FOCUS::LF_EXTENSIONLIST ) {
		const auto TypeView = GetTypeView( );
		if ( TypeView != NULL ) {
			TypeView->SetFocus( );
			}
		}
	}

size_t CMainFrame::getExtDataSize( ) const noexcept {
	const auto Document = GetDocument( );
	if ( Document != NULL ) {
		return Document->GetExtensionRecords( )->size( );
		}
	return 0;
	}

void CMainFrame::valid_timing_to_write( _In_ const double populate_timing, _In_ const double draw_timing, _In_ const double average_extension_length, _In_ const double enum_timing, _In_ const double compressed_file_timing, _In_ const double total_time, _In_ const rsize_t ext_data_size, _In_ const double file_name_length, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) noexcept {
	const HRESULT fmt_res = StringCchPrintfW( buffer_ptr, buffer_size_init, _T( "File enumeration took %.3f sec. NTFS compressed file size processing took: %.3f sec. Drawing took %.3f sec. Populating 'file types' took %.3f sec. Total: %.4f sec. # file types: %u. Avg name len: %.2f. Avg extension len: %.2f." ), enum_timing, compressed_file_timing, draw_timing, populate_timing, total_time, unsigned( ext_data_size ), file_name_length, average_extension_length );
	ASSERT( SUCCEEDED( fmt_res ) );
	if ( SUCCEEDED( fmt_res ) ) {
		CFrameWnd::SetMessageText( buffer_ptr );
		m_drawTiming = buffer_ptr;
		return;
		}
	if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		CFrameWnd::SetMessageText( L"Couldn't set message text: STRSAFE_E_INSUFFICIENT_BUFFER" );
		return;
		}
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfW" );
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
	CFrameWnd::SetMessageText( L"Couldn't set message text, unknown error!" );
	}

void CMainFrame::invalid_timing_to_write( _In_ const double average_extension_length, _In_ const rsize_t ext_data_size, _Out_ _Post_z_ _Pre_writable_size_( buffer_size_init ) PWSTR buffer_ptr, const rsize_t buffer_size_init ) noexcept {
	const HRESULT fmt_res = StringCchPrintfW( buffer_ptr, buffer_size_init, _T( "I had trouble with QueryPerformanceCounter, and can't provide timing. # file types: %u. Avg extension len: %.2f." ), unsigned( ext_data_size ), average_extension_length );
	ASSERT( SUCCEEDED( fmt_res ) );
	if ( SUCCEEDED( fmt_res ) ) {
		CFrameWnd::SetMessageText( buffer_ptr );
		m_drawTiming = buffer_ptr;
		return;
		}
	if ( fmt_res == STRSAFE_E_INSUFFICIENT_BUFFER ) {
		CFrameWnd::SetMessageText( L"Couldn't set message text: STRSAFE_E_INSUFFICIENT_BUFFER" );
		return;
		}
	WDS_STRSAFE_E_INVALID_PARAMETER_HANDLER( fmt_res, "StringCchPrintfW" );
	WDS_ASSERT_EXPECTED_STRING_FORMAT_FAILURE_HRESULT( fmt_res );
	CFrameWnd::SetMessageText( L"Couldn't set message text, unknown error!" );
	}


_Pre_satisfies_( searchTiming >= compressed_file_timing )
void CMainFrame::WriteTimeToStatusBar( _In_ const double drawTiming, _In_ const DOUBLE searchTiming, _In_ const DOUBLE fileNameLength, _In_ const DOUBLE compressed_file_timing ) {
	/*
	  Negative values are assumed to be erroneous.
	*/
	ASSERT( searchTiming >= compressed_file_timing );
	const rsize_t buffer_size_init = 512u;

	//TODO: why are we using heap here?
	std::unique_ptr<_Null_terminated_ wchar_t[ ]> buffer_uniq_ptr = std::make_unique<_Null_terminated_ wchar_t[ ]>( buffer_size_init );

	PWSTR buffer_ptr = buffer_uniq_ptr.get( );

	DOUBLE populateTiming_scopeholder = -1;
	DOUBLE averageExtLeng_scopeholder = -1;
	const auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		populateTiming_scopeholder = TypeView->m_extensionListControl.m_adjustedTiming;
		averageExtLeng_scopeholder = TypeView->m_extensionListControl.m_averageExtensionNameLength;
		}
	else {
		//no typeview, no population done!
		populateTiming_scopeholder = 0.00;
		}

	const DOUBLE populateTiming = populateTiming_scopeholder;
	const DOUBLE averageExtLeng = averageExtLeng_scopeholder;
	ASSERT( searchTiming >= compressed_file_timing );
	const auto enum_timing = ( searchTiming - compressed_file_timing );
	ASSERT( searchTiming >= enum_timing );

	const auto extDataSize = getExtDataSize( );
	if ( extDataSize == 0 ) {
		TRACE( _T( "0 extensions, we've probably not enumerated anything. NOT writing to status bar.\r\n" ) );
		return;
		}

	ASSERT( searchTiming > 0.00f );
	ASSERT( drawTiming > 0.00f );
	//ASSERT( populateTiming > 0.00f );//todo
	m_drawTiming.clear( );
	double total_time_scope_holder = 0;
	if ( ( searchTiming >= 0.00 ) && ( drawTiming > 0.00 ) && ( populateTiming > 0.00 ) ) {
		total_time_scope_holder = ( searchTiming + drawTiming + populateTiming );
		}
	const auto total_time = total_time_scope_holder;
	if ( ( searchTiming >= 0.00 ) && ( drawTiming >= 0.00 ) && ( populateTiming >= 0.00 ) && ( enum_timing < DBL_MAX ) && ( searchTiming < DBL_MAX ) ) {
		valid_timing_to_write( populateTiming, drawTiming, averageExtLeng, enum_timing, compressed_file_timing, total_time, extDataSize, fileNameLength, buffer_ptr, buffer_size_init );
		return;
		}

	invalid_timing_to_write( averageExtLeng, extDataSize, buffer_ptr, buffer_size_init );
	return;
	}

void CMainFrame::SetSelectionMessageText( ) noexcept {
	switch ( m_logicalFocus ) {
		case LOGICAL_FOCUS::LF_NONE:
			return CFrameWnd::SetMessageText( m_drawTiming.c_str( ) );
		case LOGICAL_FOCUS::LF_DIRECTORYLIST:
			{
			const auto Document = GetDocument( );
			ASSERT( Document != NULL );
			if ( Document == NULL ) {
				CFrameWnd::SetMessageText( _T( "No document?" ) );
				return;
				}
			const auto Selection = Document->m_selectedItem;
			if ( Selection == NULL ) {
				CFrameWnd::SetMessageText( m_drawTiming.c_str( ) );
				return;
				}
			CFrameWnd::SetMessageText( Selection->GetPath( ).c_str( ) );
			return;
			}
			return;
		case LOGICAL_FOCUS::LF_EXTENSIONLIST:
			return CFrameWnd::SetMessageText( std::wstring( L'*' + GetDocument( )->m_highlightExtension ).c_str( ) );
		}
	}

void CMainFrame::OnUpdateMemoryUsage( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( true );
	const rsize_t ramUsageStrBufferSize = 50;
	_Null_terminated_ wchar_t ramUsageStr[ ramUsageStrBufferSize ] = { 0 };
	const HRESULT res = m_appptr->GetCurrentProcessMemoryInfo( ramUsageStr, ramUsageStrBufferSize );
	if ( !SUCCEEDED( res ) ) {
		rsize_t chars_written = 0;
		wds_fmt::write_BAD_FMT( ramUsageStr, chars_written );
		}
	pCmdUI->SetText( ramUsageStr );
	}



void CMainFrame::OnSize( const UINT nType, const INT cx, const INT cy ) {
	CFrameWnd::OnSize( nType, cx, cy );

	//IsWindow function: https://msdn.microsoft.com/en-us/library/windows/desktop/ms633528.aspx
	//If the window handle identifies an existing window, the return value is nonzero.
	//If the window handle does not identify an existing window, the return value is zero.
	if ( !( ::IsWindow( m_wndStatusBar.m_hWnd ) ) ) {
		return;
		}
	CRect rc;
	m_wndStatusBar.GetItemRect( 0, rc );
	}

void CMainFrame::OnUpdateViewShowtreemap( CCmdUI *pCmdUI ) {
	const auto GraphView = GetGraphView( );
	ASSERT( GraphView != NULL );
	if ( GraphView == NULL ) {
		return;
		}
	pCmdUI->SetCheck( GraphView->m_showTreemap );
	}

void CMainFrame::OnViewShowtreemap( ) {
	const auto thisGraphView = GetGraphView( );
	ASSERT( thisGraphView != NULL );
	if ( thisGraphView == NULL ) {
		return;
		}
	thisGraphView->m_showTreemap = !thisGraphView->m_showTreemap;
	if ( thisGraphView->m_showTreemap ) {
		return RestoreGraphView( );
		}
	//MinimizeGraphView( );
	m_wndSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::OnUpdateViewShowfiletypes( CCmdUI *pCmdUI ) {
	const auto TypeView = GetTypeView( );
	ASSERT( TypeView != NULL );
	if ( TypeView == NULL ) {
		return;
		}
	pCmdUI->SetCheck( TypeView->m_showTypes );
	}

void CMainFrame::OnViewShowGrid( ) {
	const auto Options = GetOptions( );
	if ( Options == NULL ) {
		return;
		}
	Options->m_treemapOptions.grid = ( !( Options->m_treemapOptions.grid ) );
	GetDocument( )->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED );
	}

void CMainFrame::OnUpdateViewShowGrid( CCmdUI *pCmdUI ) {
	const auto Options = GetOptions( );
	if ( Options == NULL ) {
		return;
		}
	pCmdUI->SetCheck( Options->m_treemapOptions.grid ? 1 : 0 );
	}

void CMainFrame::OnViewShowfiletypes( ) {
	const auto thisTypeView = GetTypeView( );
	ASSERT( thisTypeView != NULL );
	if ( thisTypeView == NULL ) {
		return;
		}
	thisTypeView->ShowTypes( !thisTypeView->m_showTypes );
	if ( thisTypeView->m_showTypes ) {
		return RestoreTypeView( );
		}
	//MinimizeTypeView( );
	m_wndSubSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::OnConfigure( ) {
	WDSOptionsPropertySheet sheet;

	CPageGeneral  general( m_appptr );
	CPageTreemap  treemap;

	sheet.AddPage( &general );
	sheet.AddPage( &treemap );

	sheet.DoModal( );
	const auto Options = GetOptions( );
	Options->SaveToRegistry( );
	}

void CMainFrame::OnSysColorChange( ) {
	CFrameWnd::OnSysColorChange( );
	const auto DirstatView = GetDirstatView( );
	ASSERT( DirstatView != NULL );
	if ( DirstatView != NULL ) {
		DirstatView->SysColorChanged( );
		}
	
	const auto TypeView = GetTypeView( );
	ASSERT( TypeView != NULL );
	if ( TypeView != NULL ) {
		TypeView->SysColorChanged( );
		}
	
	}

#endif
