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
#include "windirstat.h"

#include "graphview.h"
#include "dirstatview.h"
#include "typeview.h"
#include "dirstatdoc.h"
#include "osspecific.h"
#include "item.h"
//#include "modalsendmail.h"

#include "pagetreelist.h"
#include "pagetreemap.h"
#include "pagegeneral.h"

#include ".\mainframe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
	// This must be synchronized with the IDR_MAINFRAME menu
	enum TOPLEVELMENU
	{
		TLM_FILE,
		TLM_EDIT,
		TLM_CLEANUP,
		TLM_TREEMAP,
		//TLM_REPORT,
		TLM_VIEW,
		TLM_HELP
	};

	enum
	{
		// This is the position of the first "User defined cleanup" menu item in the "Cleanup" menu.
		// !!! MUST BE SYNCHRONIZED WITH THE MENU RESOURCE !!!
		MAINMENU_USERDEFINEDCLEANUP_POSITION = 11
	};

	enum
	{
		IDC_SUSPEND = 4712,	// ID of "Suspend"-Button
		IDC_DEADFOCUS		// ID of dead-focus window
	};

	// Clipboard-Opener
	class COpenClipboard
	{
	public:
		COpenClipboard(CWnd *owner, bool empty =true) 
		{ 
			m_open = owner->OpenClipboard( );
			if ( !m_open ) {
				MdThrowStringException( IDS_CANNOTOPENCLIPBOARD );
				}
			if (empty) {
				if ( !EmptyClipboard( ) ) {
					MdThrowStringException( IDS_CANNOTEMTPYCLIPBOARD );
					}
				}
		}
		~COpenClipboard()
		{
			if ( m_open ) {
				CloseClipboard( );
				}
		}
	private:
		BOOL m_open;
	};

	
}


/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(COptionsPropertySheet, CPropertySheet)

COptionsPropertySheet::COptionsPropertySheet() : CPropertySheet(IDS_WINDIRSTAT_SETTINGS) {
	m_restartApplication = false;
	m_languageChanged    = false;
	m_alreadyAsked       = false;
	}

void COptionsPropertySheet::SetLanguageChanged(_In_ const bool changed) {
	m_languageChanged = changed;
	}

BOOL COptionsPropertySheet::OnInitDialog() {
	BOOL bResult= CPropertySheet::OnInitDialog();
	
	CRect rc;
	GetWindowRect( rc );
	CPoint pt = rc.TopLeft( );
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

	INT cmd = LOWORD( wParam );
	if ( cmd == IDOK || cmd == ID_APPLY_NOW ) {
		if ( m_languageChanged && ( cmd == IDOK || !m_alreadyAsked ) ) {
			INT r = AfxMessageBox( IDS_LANGUAGERESTARTNOW, MB_YESNOCANCEL );
			if ( r == IDCANCEL ) {
				return true;	// "Message handled". Don't proceed.
				}
			else if ( r == IDNO ) {
				m_alreadyAsked = true; // Don't ask twice.
				}
			else {
				ASSERT( r == IDYES );
				m_restartApplication = true;

				if ( cmd == ID_APPLY_NOW ) {
					// This _posts_ a message...
					EndDialog( IDOK );
					// ... so after returning from this function, the OnOK()-handlers of the pages will be called, before the sheet is closed.
					}
				}
			}
		}
	return CPropertySheet::OnCommand( wParam, lParam );
	}

/////////////////////////////////////////////////////////////////////////////

CMySplitterWnd::CMySplitterWnd(LPCTSTR name) : m_persistenceName(name), m_splitterPos(0.5), m_wasTrackedByUser(false), m_userSplitterPos(0.5) {
	//m_splitterPos = 0.5;
	CPersistence::GetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
	}

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
	
			if ( ( rcClient.right - rcClient.left) > 0 ) {
				m_splitterPos = ( DOUBLE ) cxLeft / ( rcClient.right - rcClient.left );
				}
			}
		else {
			INT dummy = 0;
			INT cyUpper = 0;
			GetRowInfo( 0, cyUpper, dummy );
	
			if ( ( rcClient.bottom - rcClient.top ) > 0 ) {
				m_splitterPos = ( DOUBLE ) cyUpper / ( rcClient.bottom - rcClient.top );
				}
			}
		m_wasTrackedByUser = true;
		m_userSplitterPos  = m_splitterPos;
		}
	}

DOUBLE CMySplitterWnd::GetSplitterPos() const {
	return m_splitterPos;
	}

void CMySplitterWnd::SetSplitterPos(_In_ const DOUBLE pos) {
	m_splitterPos = pos;

	CRect rcClient;
	GetClientRect( &rcClient );

	if ( GetColumnCount( ) > 1 ) {
		if ( m_pColInfo != NULL ) {
			INT cxLeft = ( INT ) ( pos * ( rcClient.right - rcClient.left ) );
			if ( cxLeft >= 0 ) {
				SetColumnInfo( 0, cxLeft, 0 );
				RecalcLayout( );
				}
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			throw;
			}
		}
	else {
		if ( m_pRowInfo != NULL ) {
			INT cyUpper = ( INT ) ( pos * ( rcClient.bottom - rcClient.top ) );
			if ( cyUpper >= 0 ) {
				SetRowInfo( 0, cyUpper, 0 );
				RecalcLayout( );
				}
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			throw;
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

void CMySplitterWnd::OnDestroy() {
	CPersistence::SetSplitterPos( m_persistenceName, m_wasTrackedByUser, m_userSplitterPos );
	CSplitterWnd::OnDestroy( );
	}


/////////////////////////////////////////////////////////////////////////////

CPacmanControl::CPacmanControl() {
	m_pacman.SetBackgroundColor( GetSysColor( COLOR_BTNFACE ) );
	//m_pacman.SetSpeed( 0.00005 );
	}

void CPacmanControl::Drive( _In_ const LONGLONG readJobs ) {
	if ( IsWindow( m_hWnd ) && m_pacman.Drive( readJobs ) ) {
		RedrawWindow( );
		}
	}

void CPacmanControl::Start( _In_ const bool start ) {
	m_pacman.Start( start );
	}

BEGIN_MESSAGE_MAP(CPacmanControl, CStatic)
	ON_WM_PAINT()
	ON_WM_CREATE()
END_MESSAGE_MAP()

INT CPacmanControl::OnCreate( const LPCREATESTRUCT lpCreateStruct ) {
	if ( CStatic::OnCreate( lpCreateStruct ) == -1 ) {
		return -1;
		}

	m_pacman.Reset( );
	m_pacman.Start( true );
	return 0;
	}

void CPacmanControl::OnPaint() {
	CPaintDC dc( this );
	CRect rc;
	GetClientRect( rc );
#ifdef DRAW_PACMAN
	m_pacman.Draw( &dc, rc );
#endif
	}

/////////////////////////////////////////////////////////////////////////////

CDeadFocusWnd::CDeadFocusWnd() {
	AfxCheckMemory( );
	}

void CDeadFocusWnd::Create(_In_ CWnd *parent) {
	CRect rc( 0, 0, 0, 0 );
	VERIFY( CWnd::Create( AfxRegisterWndClass( 0, 0, 0, 0 ), _T( "_deadfocus" ), WS_CHILD, rc, parent, IDC_DEADFOCUS ) );
	}

CDeadFocusWnd::~CDeadFocusWnd() {
	DestroyWindow( );
	AfxCheckMemory( );
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
	ON_COMMAND(ID_TREEMAP_HELPABOUTTREEMAPS, OnTreemapHelpabouttreemaps)
	ON_BN_CLICKED(IDC_SUSPEND, OnBnClickedSuspend)
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,
	ID_INDICATOR_MEMORYUSAGE,
};

CMainFrame *CMainFrame::_theFrame;

CMainFrame *CMainFrame::GetTheFrame( ) {
	return _theFrame;
	}

CMainFrame::CMainFrame( ) : m_wndSplitter( _T( "main" ) ), m_wndSubSplitter( _T( "sub" ) ), m_progressVisible( false ), m_progressRange( 100 ), m_progressPos( 100 ), /*m_rbLastKnownbytes( NULL ), m_rbLastKnownItems( NULL ),*/ m_lastSearchTime( -1 ) {
	AfxCheckMemory( );
	_theFrame = this;
	m_logicalFocus = LF_NONE;
	}

CMainFrame::~CMainFrame() {
	//Can I `delete _theFrame`?
	//delete _theFrame;//NO - infinite recursion.
	_theFrame = NULL;
	AfxCheckMemory( );
	}

void CMainFrame::ShowProgress(_In_ LONGLONG range) {
	/*
	  A range of 0 means that we have no range.
	  In this case we display Pacman.
	*/
	HideProgress( );
	auto thisOptions = GetOptions( );
	if ( thisOptions != NULL ) {
		if ( thisOptions->IsFollowMountPoints( ) || thisOptions->IsFollowJunctionPoints( ) ) {
			range = 0;
			}
		}
	m_progressRange   = range;
	m_progressPos     = 0;
	m_progressVisible = true;
	if ( range > 0 ) {
		CreateStatusProgress( );
		}
	else {
		CreatePacmanProgress( );
		}
	//UpdateProgress();
	}

void CMainFrame::HideProgress() {
	DestroyProgress( );
	if ( m_progressVisible ) {
		m_progressVisible = false;
		if ( IsWindow( *GetMainFrame( ) ) ) {
			GetDocument( )->SetTitlePrefix( _T( "" ) );
			SetMessageText( AFX_IDS_IDLEMESSAGE );
			}
		}
	}

void CMainFrame::SetProgressPos(_In_ LONGLONG pos) {
	if ( m_progressRange > 0 && pos > m_progressRange ) {
		pos = m_progressRange;
		}

	m_progressPos = pos;
	UpdateProgress( );
	}

void CMainFrame::SetProgressPos100() {
	if ( m_progressRange > 0 ) {
		SetProgressPos( m_progressRange );
		}
	}

bool CMainFrame::IsProgressSuspended() {
	if ( !IsWindow( m_suspendButton.m_hWnd ) ) {
		return false;
		}
	bool checked = ( m_suspendButton.GetState( ) & 0x3 ) != 0;
	return checked;
	}

//void CMainFrame::DrivePacman() {
//	return;
//	m_pacman.Drive( GetDocument( )->GetWorkingItemReadJobs( ) );
//	}

void CMainFrame::UpdateProgress() {
	if ( m_progressVisible ) {
		CString titlePrefix;
		CString suspended;

		if ( IsProgressSuspended( ) ) {
			auto ret = suspended.LoadString( IDS_SUSPENDED_ );//TODO
			if ( ret == 0 ) {
				exit( 666 );
				}
			}

		if ( m_progressRange > 0 ) {
			INT pos = ( INT ) ( ( DOUBLE ) m_progressPos * 100 / m_progressRange );
			m_progress.SetPos( pos );
			titlePrefix.Format( _T( "%d%% %s" ), pos, suspended.GetString( ) );
			}
		else {
			titlePrefix = LoadString( IDS_SCANNING_ ) + suspended;
			}
		//GetDocument( )->SetTitlePrefix( titlePrefix );//gets called far too often. TODO: 
		}
	}

void CMainFrame::FirstUpdateProgress( ) {
	if ( m_progressVisible ) {
		CString titlePrefix;
		CString suspended;

		if ( IsProgressSuspended( ) ) {
			auto ret = suspended.LoadString( IDS_SUSPENDED_ );//TODO
			if ( ret == 0 ) {
				AfxCheckMemory( );
				exit( 666 );
				}
			}
			titlePrefix = LoadString( IDS_SCANNING_ ) + suspended;
		GetDocument( )->SetTitlePrefix( titlePrefix );//gets called far too often. TODO: 
		}
	}

void CMainFrame::CreateStatusProgress() {
	if ( m_progress.m_hWnd == NULL ) {
		CRect rc;
		m_wndStatusBar.GetItemRect( 0, rc );
		CreateSuspendButton( rc );
		m_progress.Create( WS_CHILD | WS_VISIBLE, rc, &m_wndStatusBar, 4711 );
		m_progress.ModifyStyle( WS_BORDER, 0 ); // Doesn't help with XP-style control.
		}
	}

void CMainFrame::CreatePacmanProgress() {
	if ( m_pacman.m_hWnd == NULL ) {
		CRect rc;
		m_wndStatusBar.GetItemRect( 0, rc );
		CreateSuspendButton( rc );
		m_pacman.Create( _T( "" ), WS_CHILD | WS_VISIBLE, rc, &m_wndStatusBar, 4711 );
		}
	}

void CMainFrame::CreateSuspendButton(_Inout_ CRect& rc) {
	/*
	  rc [in]: Rect of status pane
	  rc [out]: Rest for progress/pacman-control
	*/
	ASSERT( rc.IsRectEmpty( ) == 0 );
	ASSERT( rc.IsRectNull( ) == 0 );
	CRect rcButton = rc;
	rcButton.right = rcButton.left + 80;

	VERIFY( m_suspendButton.Create( LoadString( IDS_SUSPEND ), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_PUSHLIKE, rcButton, &m_wndStatusBar, IDC_SUSPEND ) );
	auto DirstatView = GetDirstatView( );
	if ( DirstatView != NULL ) {
		m_suspendButton.SetFont( DirstatView->GetSmallFont( ) );
		}
	rc.left = rcButton.right;
	}

void CMainFrame::DestroyProgress() {
	if ( IsWindow( m_progress.m_hWnd ) ) {
		m_progress.DestroyWindow( );
		m_progress.m_hWnd = NULL;
		}
	else if ( IsWindow( m_pacman.m_hWnd ) ) {
		m_pacman.DestroyWindow( );
		m_pacman.m_hWnd = NULL;
		}
	if ( IsWindow( m_suspendButton.m_hWnd ) ) {
		m_suspendButton.DestroyWindow( );
		m_suspendButton.m_hWnd = NULL;
		}
	AfxCheckMemory( );
	}

void CMainFrame::OnBnClickedSuspend() {
	m_pacman.Start( !IsProgressSuspended( ) );
	UpdateProgress( );
	}

INT CMainFrame::OnCreate(const LPCREATESTRUCT lpCreateStruct) {
	/*
	Initializes the MAIN frame - wherein the rectangular layout, the list of files, and the list of file types are.
	Initializes a few related things, such as the memory display.
	*/
	
	if ( CFrameWnd::OnCreate( lpCreateStruct ) == -1 ) {
		AfxCheckMemory( );
		return -1;
		}
	
	VERIFY( m_wndToolBar.CreateEx( this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC ) );
	//VERIFY( m_wndToolBar.LoadToolBar( IDR_MAINFRAME ) );

	UINT *indic = indicators;
	UINT size = countof( indicators );

	VERIFY( m_wndStatusBar.Create( this ) );
	VERIFY( m_wndStatusBar.SetIndicators( indic, size ) );
	m_wndDeadFocus.Create( this );

	m_wndToolBar.EnableDocking( CBRS_ALIGN_ANY );
	EnableDocking( CBRS_ALIGN_ANY );
	DockControlBar( &m_wndToolBar );

	LoadBarState( CPersistence::GetBarStateSection( ) );
	ShowControlBar( &m_wndToolBar, CPersistence::GetShowToolbar( ), false );
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

void CMainFrame::MakeSaneShowCmd(_Inout_ UINT& u) {
	switch (u)
	{
		default:
		case SW_HIDE:
		case SW_MINIMIZE:
		case SW_SHOWMINNOACTIVE:
		case SW_SHOWNA:
		case SW_SHOWMINIMIZED:
		case SW_SHOWNOACTIVATE:
		case SW_RESTORE:
		case SW_FORCEMINIMIZE:
		case SW_SHOWDEFAULT:
		case SW_SHOW:
		case SW_SHOWNORMAL:
			u = SW_SHOWNORMAL;
			break;
		case SW_SHOWMAXIMIZED:
			break;
	}
	}

void CMainFrame::OnClose() {
	CWaitCursor wc;

	// It's too late, to do this in OnDestroy(). Because the toolbar, if undocked, is already destroyed in OnDestroy(). So we must save the toolbar state here in OnClose().
	SaveBarState( CPersistence::GetBarStateSection( ) );
	CPersistence::SetShowToolbar( ( m_wndToolBar.GetStyle( ) & WS_VISIBLE ) != 0 );
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
	AfxCheckMemory( );
	}

void CMainFrame::OnDestroy() {
	auto wp = zeroInitWINDOWPLACEMENT( );
	GetWindowPlacement( &wp );
	CPersistence::SetMainWindowPlacement( wp );
	auto TypeView = GetTypeView( );
	auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		CPersistence::SetShowFileTypes( TypeView->IsShowTypes( ) );
		}
	if ( GraphView != NULL ) {
		CPersistence::SetShowTreemap( GraphView->IsShowTreemap( ) );
		}
	CFrameWnd::OnDestroy( );
	AfxCheckMemory( );
	}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/, CCreateContext* pContext) {
	VERIFY( m_wndSplitter.CreateStatic( this, 2, 1 ) );
	VERIFY( m_wndSplitter.CreateView( 1, 0, RUNTIME_CLASS( CGraphView ), CSize( 100, 100 ), pContext ) );
	VERIFY( m_wndSubSplitter.CreateStatic( &m_wndSplitter, 1, 2, WS_CHILD | WS_VISIBLE | WS_BORDER, m_wndSplitter.IdFromRowCol( 0, 0 ) ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 0, RUNTIME_CLASS( CDirstatView ), CSize( 700, 500 ), pContext ) );
	VERIFY( m_wndSubSplitter.CreateView( 0, 1, RUNTIME_CLASS( CTypeView ), CSize( 100, 500 ), pContext ) );

	MinimizeGraphView( );
	MinimizeTypeView ( );

	auto TypeView = GetTypeView( );
	auto GraphView = GetGraphView( );
	if ( TypeView != NULL ) {
		TypeView->ShowTypes  ( CPersistence::GetShowFileTypes( ) );
		}
	if ( GraphView != NULL ) {
		GraphView->ShowTreemap( CPersistence::GetShowTreemap  ( ) );
		}
	return TRUE;
	}

BOOL CMainFrame::PreCreateWindow( CREATESTRUCT& cs) {
	if ( !CFrameWnd::PreCreateWindow( cs ) ) {
		return FALSE;
		}
	return TRUE;
	}


// CMainFrame Diagnose

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

void CMainFrame::MinimizeTypeView() {
	m_wndSubSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::RestoreTypeView() {
	auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		if ( thisTypeView->IsShowTypes( ) ) {
			m_wndSubSplitter.RestoreSplitterPos( 0.72 );
			thisTypeView->RedrawWindow( );
			}
		}
	}

void CMainFrame::MinimizeGraphView() {
	m_wndSplitter.SetSplitterPos( 1.0 );
	}

void CMainFrame::RestoreGraphView() {
	auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		if ( thisGraphView->IsShowTreemap( ) ) {
			m_wndSplitter.RestoreSplitterPos( 0.4 );
			thisGraphView->DrawEmptyView( );

			LARGE_INTEGER startDrawTime;
			LARGE_INTEGER endDrawTime;
			LARGE_INTEGER timingFrequency;

			BOOL res1 = QueryPerformanceFrequency( &timingFrequency );
			const DOUBLE adjustedTimingFrequency = ( ( DOUBLE ) 1.00 ) / timingFrequency.QuadPart;
			BOOL res2 = QueryPerformanceCounter( &startDrawTime );


			thisGraphView->RedrawWindow( );
			BOOL res3 = QueryPerformanceCounter( &endDrawTime );

			DOUBLE timeToDrawWindow = 0;
			if ( ( !res2 ) || ( !res1 ) || ( !res3 ) ) {
				timeToDrawWindow = -1;
				}
			else {
				timeToDrawWindow = ( endDrawTime.QuadPart - startDrawTime.QuadPart ) * adjustedTimingFrequency;
				}
			//auto locSearchTime = GetDocument( )->m_searchTime;
			if ( m_lastSearchTime == -1 ) {
				DOUBLE searchingTime = GetDocument( )->m_searchTime;
				m_lastSearchTime = searchingTime;
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime );//else the search time compounds whenever the time is written to the status bar
				}
			else {
				WriteTimeToStatusBar( timeToDrawWindow, m_lastSearchTime );
				}
			}
		}
	}

_Must_inspect_result_ _Success_(return != NULL) CDirstatView* CMainFrame::GetDirstatView() {
	CWnd* pWnd = m_wndSubSplitter.GetPane( 0, 0 );
	CDirstatView* pView = DYNAMIC_DOWNCAST( CDirstatView, pWnd );
	return pView;
	}

_Must_inspect_result_ _Success_(return != NULL) CGraphView* CMainFrame::GetGraphView() {
	CWnd *pWnd = m_wndSplitter.GetPane( 1, 0 );
	CGraphView *pView = DYNAMIC_DOWNCAST( CGraphView, pWnd );
	return pView;
	}

_Must_inspect_result_ _Success_(return != NULL) CTypeView* CMainFrame::GetTypeView() {
	CWnd *pWnd = m_wndSubSplitter.GetPane( 0, 1 );
	CTypeView *pView = DYNAMIC_DOWNCAST( CTypeView, pWnd );
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

void CMainFrame::CopyToClipboard( _In_ const LPCTSTR psz ) {
	try
	{
		COpenClipboard clipboard(this);

		HGLOBAL h = GlobalAlloc( GMEM_MOVEABLE, ( lstrlen( psz ) + 1 ) * sizeof( TCHAR ) );
		if ( h == NULL ) {
			MdThrowStringException( _T( "GlobalAlloc failed." ) );
			}

		LPVOID lp = GlobalLock( h );
		ASSERT( lp != NULL );

		lstrcpy( ( LPTSTR ) lp, psz );
	
		GlobalUnlock( h );
  

		//wtf is going on here?
		UINT uFormat = CF_TEXT;
		uFormat = CF_UNICODETEXT;
		
		if ( NULL == SetClipboardData( uFormat, h ) ) {
			MdThrowStringException( IDS_CANNOTSETCLIPBAORDDATA );
			}
	}
	catch (CException *pe)
	{
		pe->ReportError( );
		pe->Delete( );
	}
	}

void CMainFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) {
	CFrameWnd::OnInitMenuPopup( pPopupMenu, nIndex, bSysMenu );

	if ( !bSysMenu ) {
		switch ( nIndex )
		{
		case TLM_CLEANUP:
			//UpdateCleanupMenu( pPopupMenu );
			break;
		}
		}
	}


void CMainFrame::SetLogicalFocus(_In_ const LOGICAL_FOCUS lf) {
	if ( lf != m_logicalFocus ) {
		m_logicalFocus = lf;
		SetSelectionMessageText( );

		GetDocument( )->UpdateAllViews( NULL, HINT_SELECTIONSTYLECHANGED );
		}
	}

LOGICAL_FOCUS CMainFrame::GetLogicalFocus( ) const {
	return m_logicalFocus;
	}

void CMainFrame::MoveFocus(_In_ const LOGICAL_FOCUS lf) {
	switch (lf)
	{
		case LF_NONE:
			SetLogicalFocus( LF_NONE );
			m_wndDeadFocus.SetFocus( );
			break;
		case LF_DIRECTORYLIST:
			{
			auto DirstatView = GetDirstatView( );
			if ( DirstatView != NULL ) {
				DirstatView->SetFocus( );
				}
			}
			break;
		case LF_EXTENSIONLIST:
			{
			auto TypeView = GetTypeView( );
			if ( TypeView != NULL ) {
				TypeView->SetFocus( );
				}
			}
			break;
	}
	}

size_t CMainFrame::getExtDataSize( ) {
	auto Document = GetDocument( );
	//std::map<CString, SExtensionRecord>* stdExtensionDataPtr = NULL;
	size_t extDataSize = 0;
	if ( Document != NULL ) {
		auto stdExtensionDataPtr = Document->GetExtensionRecords( );
		if ( stdExtensionDataPtr != NULL ) { 
			extDataSize = stdExtensionDataPtr->size( );
			}
		}
	return extDataSize;
	}

void CMainFrame::WriteTimeToStatusBar( _In_ const double drawTiming, _In_ const DOUBLE searchTiming ) {
	CString timeText;
	/*
	  CString::Format reference: http://msdn.microsoft.com/en-us/library/tcxf1dw6.aspx
	  Negative values are assumed to be erroneous.
	*/
	auto populateTiming = GetTypeView( )->getPopulateTiming( );
	auto extDataSize = getExtDataSize( );
		if ( ( searchTiming > 0.00 ) && ( drawTiming > 0.00 ) && ( populateTiming > 0.00 ) ) {
			timeText.Format( _T( "Finding files took %f seconds, Drawing took %f seconds. Populating the list of file types took %f seconds. Number of file types: %u. Drawing is a function of window size (an MFC limitation)." ), searchTiming, drawTiming, populateTiming, ( UINT ) extDataSize );
			}
		else {
			timeText.Format( _T("I had trouble with QueryPerformanceCounter, and can't provide timing for searching or drawing. The number of file types: %u"), (UINT)extDataSize );
			}
	SetMessageText( timeText );
	m_drawTiming = timeText;
	}

void CMainFrame::WriteTimeToStatusBar( ) {
	if ( m_drawTiming != "" ) {
		SetMessageText( m_drawTiming );
		}
	else {
		CString temp;
		temp = "Eeek! No timing info!";
		SetMessageText( temp );
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
					SetMessageText( Selection->GetPath( ) );
					}
				else {
					//SetMessageText(L"are we?");
					SetMessageText( m_drawTiming );
					}
				}
			else {
				AfxCheckMemory( );
				ASSERT( false );
				SetMessageText( _T( "No document?" ) );
				}
			}
			break;
		case LF_EXTENSIONLIST:
			SetMessageText(_T("*") + GetDocument()->GetHighlightExtension());
			break;
	}
	}

void CMainFrame::OnUpdateMemoryUsage( CCmdUI *pCmdUI ) {
	pCmdUI->Enable( true );
	pCmdUI->SetText( GetApp( )->GetCurrentProcessMemoryInfo( ) );
	if ( GetApp( )->m_MemUsageThread != NULL ) {
		//pCmdUI->SetText( GetApp()->m_MemUsageThread-> );
		}
	}



void CMainFrame::OnSize( const UINT nType, const INT cx, const INT cy ) {
	CFrameWnd::OnSize( nType, cx, cy );

	if ( !IsWindow( m_wndStatusBar.m_hWnd ) ) {
		return;
		}
	CRect rc;
	rc.bottom = NULL;
	rc.left = NULL;
	rc.right = NULL;
	rc.top = NULL;

	m_wndStatusBar.GetItemRect( 0, rc );

	if ( m_suspendButton.m_hWnd != NULL ) {
		CRect suspend;
		m_suspendButton.GetClientRect( suspend );
		rc.left = suspend.right;
		}
	if ( m_progress.m_hWnd != NULL ) {
		m_progress.MoveWindow( rc );
		}
	else if ( m_pacman.m_hWnd != NULL ) {
		m_pacman.MoveWindow( rc );
		}
	}

void CMainFrame::OnUpdateViewShowtreemap(CCmdUI *pCmdUI) {
	auto GraphView = GetGraphView( );
	if ( GraphView != NULL ) {
		pCmdUI->SetCheck( GraphView->IsShowTreemap( ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CMainFrame::OnViewShowtreemap() {
	auto thisGraphView = GetGraphView( );
	if ( thisGraphView != NULL ) {
		thisGraphView->ShowTreemap( !thisGraphView->IsShowTreemap( ) );
		if ( thisGraphView->IsShowTreemap( ) ) {
			RestoreGraphView( );
			}
		else {
			MinimizeGraphView( );
			}
		}
	else{
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CMainFrame::OnUpdateViewShowfiletypes(CCmdUI *pCmdUI) {
	auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		pCmdUI->SetCheck( TypeView->IsShowTypes( ) );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CMainFrame::OnViewShowfiletypes() {
	auto thisTypeView = GetTypeView( );
	if ( thisTypeView != NULL ) {
		thisTypeView->ShowTypes( !thisTypeView->IsShowTypes( ) );
		if ( thisTypeView->IsShowTypes( ) ) {
			RestoreTypeView( );
			}
		else {
			MinimizeTypeView( );
			}
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

void CMainFrame::OnConfigure() {
	COptionsPropertySheet sheet;

	CPageGeneral  general;
	CPageTreelist treelist;
	CPageTreemap  treemap;

	sheet.AddPage( &general );
	sheet.AddPage( &treelist );
	sheet.AddPage( &treemap );

	sheet.DoModal( );
	auto Options = GetOptions( );
	if ( Options != NULL ) {
		Options->SaveToRegistry( );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	if ( sheet.m_restartApplication ) {
		auto App = GetApp( );
		if ( App != NULL ) {
			App->RestartApplication( );
			}
		else {
			AfxCheckMemory( );
			ASSERT( false );
			}
		}
	}


void CMainFrame::OnTreemapHelpabouttreemaps() {
	GetApp( )->DoContextHelp( IDH_Treemap );
	}


void CMainFrame::OnSysColorChange() {
	CFrameWnd::OnSysColorChange( );
	auto DirstatView = GetDirstatView( );
	if ( DirstatView != NULL ) {
		DirstatView->SysColorChanged( );
		}
	auto TypeView = GetTypeView( );
	if ( TypeView != NULL ) {
		TypeView->SysColorChanged( );
		}
	else {
		AfxCheckMemory( );
		ASSERT( false );
		}
	}

// $Log$
// Revision 1.9  2004/11/27 07:19:37  bseifert
// Unicode/Ansi/Debug-specification in version.h/Aboutbox. Minor fixes.
//
// Revision 1.8  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
