// graphview.h	- Declaration of CGraphView (the Treemap view)
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_H
#define WDS_GRAPHVIEW_H

#include "treemap.h"
#include "windirstat.h"
#include "options.h"
#include "globalhelpers.h"
#include "dirstatdoc.h"
#include "item.h"
#include "mainframe.h"
#include "windirstat.h"
#include "dirstatview.h"

class CDirstatDoc;
class CItemBranch;
class CGraphView;


#ifdef DEBUG
inline void trace_empty_view_graphview( );
inline void trace_call_onidle( );
inline void trace_mouse_left( );
inline void trace_focused_mouspos( _In_ const LONG x, _In_ const LONG y, _In_z_ PCWSTR const path );
#endif

// CGraphView. The treemap window.
class CGraphView final : public CView {
protected:
	CGraphView( ) : m_recalculationSuspended( false ), m_showTreemap( true ), m_timer( 0 ), m_frameptr( GetMainFrame( ) ), m_appptr( GetApp( ) ) {
		m_size.cx = 0;
		m_size.cy = 0;
		m_dimmedSize.cx = 0;
		m_dimmedSize.cy = 0;
		m_showTreemap = CPersistence::GetShowTreemap( );
		}

	/*
#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define DECLARE_DYNAMIC(class_name) \
public: \
	static const CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

----------------------------------------

public:
	static const CRuntimeClass classCGraphView;
	virtual CRuntimeClass* GetRuntimeClass( ) const;
	static CObject* PASCAL CreateObject( );

	*/

public:
	static const CRuntimeClass classCGraphView;
	
	virtual CRuntimeClass* GetRuntimeClass( ) const final {
		return const_cast<CRuntimeClass*>( &CGraphView::classCGraphView );
		}

	static CObject* PASCAL CreateObject( ) {
		return new CGraphView;
		}

	//DECLARE_DYNCREATE(CGraphView)

public:
	//virtual ~CGraphView();

	CGraphView& operator=( const CGraphView& in ) = delete;
	CGraphView( const CGraphView& in ) = delete;

	void SuspendRecalculation( _In_ const bool suspend ) {
		m_recalculationSuspended = suspend;
		if ( !suspend ) {
			Invalidate( );
			}
		}

	void DrawEmptyView( ) {
		CClientDC dc( this );
		DrawEmptyView( dc );
		}

protected:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs ) override final {
		// We don't want a background brush
		VERIFY( CView::PreCreateWindow( cs ) ); // this registers a wndclass
	
		WNDCLASS wc;
		VERIFY( GetClassInfoW( AfxGetInstanceHandle( ), cs.lpszClass, &wc ) );
		wc.hbrBackground = { NULL };
		wc.lpszClassName = _T( "windirstat_graphview_class" );
		cs.lpszClass = reinterpret_cast<PCWSTR>( RegisterClassW( &wc ) );
	
		return TRUE;

		}
	
	virtual void OnInitialUpdate( ) override final {
		CView::OnInitialUpdate( );
		}
	
	virtual void OnDraw( CDC* pScreen_Device_Context ) override final {
		ASSERT_VALID( pScreen_Device_Context );
		const auto aDocument = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		ASSERT( aDocument != NULL );
		if ( aDocument == NULL ) {
			return;
			}
		const auto root = aDocument->m_rootItem.get( );
		if ( root == NULL ) {
			return;
			}
		ASSERT( root->m_attr.m_done );

		if ( !( root->m_attr.m_done ) ) {
			displayWindowsMsgBoxWithMessage( L"CGraphView::OnLButtonDown: root item is NOT done! This should never happen!" );
			std::terminate( );
			}
		if ( m_recalculationSuspended || ( !m_showTreemap ) ) {
			// TODO: draw something interesting, e.g. outline of the first level.
			return DrawEmptyView( *pScreen_Device_Context );
			}
		DrawViewNotEmpty( *pScreen_Device_Context );
		}
	
	//this is just a comparison, it should be inlined.
	inline bool IsDrawn( ) const {
		return ( m_bitmap.m_hObject != NULL );
		}

	void Inactivate( ) {
		if ( m_bitmap.m_hObject != NULL ) {
			m_dimmed.Detach( );
			VERIFY( m_dimmed.Attach( m_bitmap.Detach( ) ) );
			m_dimmedSize = m_size;
			}
		}
	
	void reset_timer_if_zero( ) {
		if ( m_timer == 0 ) {
#ifdef DEBUG
			trace_mouse_left( );
#endif
			m_timer = SetTimer( 4711, 100, NULL );//TODO: figure out what the hell this does.//if value is increased ( argument 2 ), program execution will take longer to reach `TRACE( _T( "Mouse has left the tree map area!\r\n" ) );` after mouse has left tree map area.
			}
		}

	void EmptyView( ) {
		if ( m_bitmap.m_hObject != NULL ) {
			m_bitmap.DeleteObject( );
			}
		if ( m_dimmed.m_hObject != NULL ) {
			m_dimmed.DeleteObject( );
			}
		}

	void cause_OnIdle_to_be_called_once( ) const {
		// Cause OnIdle() to be called once.
#ifdef DEBUG
		trace_call_onidle( );
#endif
		PostAppMessageW( GetCurrentThreadId( ), WM_NULL, 0, 0 );
		}


	void RenderHighlightRectangle( _In_ CDC& pdc, _In_ RECT rc_ ) const {
		/*
		  The documentation of CDC::Rectangle() says that the width and height must be greater than 2. Experiment says that it must be greater than 1. We follow the documentation.
		  A pen and the null brush must be selected.
		  */

		auto rc = rc_;

		ASSERT( ( rc.right - rc.left ) >= 0 );
		ASSERT( ( rc.bottom - rc.top ) >= 0 );

		//TODO: BUGBUG: why 7?
		if ( ( ( rc.right - rc.left ) >= 7 ) && ( ( rc.bottom - rc.top ) >= 7 ) ) {

			VERIFY( pdc.Rectangle( &rc ) );		// w = 7

			VERIFY( ::InflateRect( &rc, -( 1 ), -( 1 ) ) );
			//rc.DeflateRect( 1, 1 );


			VERIFY( pdc.Rectangle( &rc ) );		// w = 5



			VERIFY( ::InflateRect( &rc, -( 1 ), -( 1 ) ) );
			//rc.DeflateRect( 1, 1 );


			VERIFY( pdc.Rectangle( &rc ) );		// w = 3
			}
		else {
			const auto Options = GetOptions( );
			return pdc.FillSolidRect( &rc, Options->m_treemapHighlightColor );
			}
		}


	void DrawEmptyView( _In_ CDC& pScreen_Device_Context ) {
#ifdef DEBUG
		trace_empty_view_graphview( );
#endif
		const COLORREF gray = RGB( 160, 160, 160 );
		Inactivate( );

		RECT rc;

		ASSERT( ::IsWindow( m_hWnd ) );
		//::GetClientRect(m_hWnd, lpRect);
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );
		//GetClientRect( &rc );

		if ( m_dimmed.m_hObject == NULL ) {
			return pScreen_Device_Context.FillSolidRect( &rc, gray );
			}
		CDC offscreen_buffer;
		VERIFY( offscreen_buffer.CreateCompatibleDC( &pScreen_Device_Context ) );
		CSelectObject sobmp( offscreen_buffer, m_dimmed );
		VERIFY( pScreen_Device_Context.BitBlt( rc.left, rc.top, m_dimmedSize.cx, m_dimmedSize.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

		if ( ( rc.right - rc.left ) > m_dimmedSize.cx ) {
			RECT r = rc;
			r.left = r.left + m_dimmedSize.cx;
			pScreen_Device_Context.FillSolidRect( &r, gray );
			}

		if ( ( rc.bottom - rc.top ) > m_dimmedSize.cy ) {
			RECT r = rc;
			r.top = r.top + m_dimmedSize.cy;
			pScreen_Device_Context.FillSolidRect( &r, gray );
			}
		//VERIFY( dcmem.DeleteDC( ) );
		}


	//void DrawZoomFrame             ( _In_ CDC& pdc, _In_       CRect& rc                           );
	
	void DrawHighlightExtension( _In_ CDC& pdc ) const {
		WTL::CWaitCursor wc;

		CPen pen( PS_SOLID, 1, GetOptions( )->m_treemapHighlightColor );
		CSelectObject sopen( pdc, pen );
		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		//auto Document = static_cast< CDirstatDoc* >( m_pDocument );;
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		if ( Document == NULL ) {
			ASSERT( Document != NULL );
			return;
			}
		const auto rItem = Document->m_rootItem.get( );
		if ( rItem != NULL ) {
			RecurseHighlightExtension( pdc, ( *rItem ), Document->m_highlightExtension );
			}

		}


	void RecurseHighlightExtension( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
		const auto rc = item.m_rect;
		if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
			return;
			}

		//if ( item.m_type == IT_FILE ) {
		if ( item.m_children == nullptr ) {
			const auto extensionStrPtr = item.CStyle_GetExtensionStrPtr( );
			const auto scmp = wcscmp( extensionStrPtr, ext.c_str( ) );
			if ( scmp == 0 ) {
				auto rcc = item.TmiGetRectangle( );
				return RenderHighlightRectangle( pdc, rcc );
				}
			return;
			}

		RecurseHighlightChildren( pdc, item, ext );
		}
	
	void DrawSelection( _In_ CDC& pdc ) const {
		//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		ASSERT( Document != NULL );
		if ( Document == NULL ) {
			return;
			}
		const auto item = Document->m_selectedItem;
		if ( item == NULL ) {//no selection to draw.
			return;
			}
		RECT rcClient;

		ASSERT( ::IsWindow( m_hWnd ) );
		//::GetClientRect(m_hWnd, lpRect);
		VERIFY( ::GetClientRect( m_hWnd, &rcClient ) );
		//GetClientRect( &rcClient );


		RECT rc = item->TmiGetRectangle( );

		TweakSizeOfRectangleForHightlight( rc, rcClient );

		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		const auto Options = GetOptions( );
		CPen pen( PS_SOLID, 1, Options->m_treemapHighlightColor );
		CSelectObject sopen( pdc, pen );
		RenderHighlightRectangle( pdc, rc );
		}
	
	
	void DoDraw( _In_ CDC& pDC, _In_ CDC& offscreen_buffer, _In_ RECT& rc ) {
		WTL::CWaitCursor wc;

		VERIFY( m_bitmap.CreateCompatibleBitmap( &pDC, m_size.cx, m_size.cy ) );
		auto guard = WDS_SCOPEGUARD_INSTANCE( [&] { cause_OnIdle_to_be_called_once( ); } );

		CSelectObject sobmp( offscreen_buffer, m_bitmap );
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		ASSERT( Document != NULL );
		if ( Document == NULL ) {
			//cause_OnIdle_to_be_called_once( );
			return;
			}
		const auto Options = GetOptions( );
		const auto rootItem = Document->m_rootItem.get( );
		ASSERT( rootItem != NULL );
		if ( rootItem == NULL ) {
			//cause_OnIdle_to_be_called_once( );
			return;
			}
		m_treemap.DrawTreemap( offscreen_buffer, rc, rootItem, Options->m_treemapOptions );
	#ifdef _DEBUG
		m_treemap.RecurseCheckTree( rootItem );
	#endif
		//cause_OnIdle_to_be_called_once( );
		}
	
	
	void DrawViewNotEmpty( _In_ CDC& Screen_Device_Context ) {
		RECT rc;

		ASSERT( ::IsWindow( m_hWnd ) );
		//::GetClientRect(m_hWnd, lpRect);
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );
		//GetClientRect( &rc );

		CDC offscreen_buffer;
		VERIFY( offscreen_buffer.CreateCompatibleDC( &Screen_Device_Context ) );

		if ( !IsDrawn( ) ) {
			DoDraw( Screen_Device_Context, offscreen_buffer, rc );
			}

		CSelectObject sobmp2( offscreen_buffer, m_bitmap );
		VERIFY( Screen_Device_Context.BitBlt( 0, 0, m_size.cx, m_size.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

		DrawHighlights( Screen_Device_Context );
		//VERIFY( dcmem.DeleteDC( ) );
		}


	//TODO: put in anonymous namespace
	void TweakSizeOfRectangleForHightlight( _Inout_ RECT& rc, _Inout_ RECT& rcClient ) const {
		if ( m_treemap.m_options.grid ) {
			rc.right++;
			rc.bottom++;
			}
		if ( rcClient.left < rc.left ) {
			rc.left--;
			}
		if ( rcClient.top < rc.top ) {
			rc.top--;
			}
		if ( rc.right < rcClient.right ) {
			rc.right++;
			}
		if ( rc.bottom < rcClient.bottom ) {
			rc.bottom++;
			}
		}


	//only called from one place
	inline void RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
		const auto childCount = item.m_childCount;
		const auto item_m_children = item.m_children.get( );

		//Not vectorized: 1200, loop contains data dependencies
		for ( size_t i = 0; i < childCount; ++i ) {
			RecurseHighlightExtension( pdc, *( item_m_children + i ), ext );
			}
		}

	//only called from one place, unconditionally.
	inline void DrawHighlights( _In_ CDC& pdc ) const {
		const auto logicalFocus = m_frameptr->m_logicalFocus;
		if ( logicalFocus == LOGICAL_FOCUS::LF_DIRECTORYLIST ) {
			DrawSelection( pdc );
			}
		if ( logicalFocus == LOGICAL_FOCUS::LF_EXTENSIONLIST ) {
			DrawHighlightExtension( pdc );
			}
		m_appptr->PeriodicalUpdateRamUsage( );
		}

protected:
	//TODO: use plain old SIZE struct
	WTL::CSize    m_size;				// Current size of view

public:
	bool m_recalculationSuspended : 1; // True while the user is resizing the window.	
	//C4820: 'CGraphView' : '3' bytes padding added after data member 'CGraphView::m_showTreemap'
	bool m_showTreemap            : 1; // False, if the user switched off the treemap (by F9).

protected:
	CTreemap m_treemap;				// Treemap generator
	CBitmap  m_bitmap;				// Cached view. If m_hObject is NULL, the view must be recalculated.
	WTL::CSize    m_dimmedSize;			// Size of bitmap m_dimmed
	CBitmap  m_dimmed;				// Dimmed view. Used during refresh to avoid the ooops-effect.
	UINT_PTR m_timer;				// We need a timer to realize when the mouse left our window.
	CMainFrame* const m_frameptr;
	CDirstatApp* m_appptr;

	/*
#define DECLARE_MESSAGE_MAP() \
protected: \
	static const AFX_MSGMAP* PASCAL GetThisMessageMap(); \
	virtual const AFX_MSGMAP* GetMessageMap() const; \
-------------------------------------------------------

	*/


protected:
	static const AFX_MSGMAP* PASCAL GetThisMessageMap( );
	
	virtual const AFX_MSGMAP* GetMessageMap( ) const final {
		return GetThisMessageMap( );
		}


	afx_msg void OnSize( UINT nType, INT cx, INT cy ) {
		CView::OnSize( nType, cx, cy );
		WTL::CSize sz( cx, cy );
		if ( sz != m_size ) {
			Inactivate( );
			m_size = sz;
			}
		}


	afx_msg void OnLButtonDown( UINT nFlags, CPoint point ) {
		//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		auto guard = WDS_SCOPEGUARD_INSTANCE( [=]{ CView::OnLButtonDown( nFlags, point ); } );
		if ( Document == NULL ) {
			TRACE( _T( "User clicked on nothing. User CAN click on nothing. That's a sane case.\r\n" ) );
			//return CView::OnLButtonDown( nFlags, point );
			return;
			}
		const auto root = Document->m_rootItem.get( );
		if ( root == NULL ) {
			//return CView::OnLButtonDown( nFlags, point );
			return;
			}

		ASSERT( root->m_attr.m_done );

		if ( !( root->m_attr.m_done ) ) {
			displayWindowsMsgBoxWithMessage( L"CGraphView::OnLButtonDown: root item is NOT done! This should never happen!" );
			std::terminate( );
			}

		if ( !IsDrawn( ) ) {
			//return CView::OnLButtonDown( nFlags, point );
			return;
			}
		const auto item = static_cast< CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
		if ( item == NULL ) {
			//return CView::OnLButtonDown( nFlags, point );
			return;
			}
		Document->SetSelection( *item );
		Document->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION );
		//CView::OnLButtonDown( nFlags, point );
		return;
		}


	afx_msg void OnSetFocus( CWnd* /*pOldWnd*/ ) {
		ASSERT( m_frameptr != NULL );
		if ( m_frameptr == NULL ) {
			return;
			}
		const auto DirstatView = m_frameptr->GetDirstatView( );
		ASSERT( DirstatView != NULL );
		if ( DirstatView == NULL ) {
			return;
			}

		//TODO: BUGBUG: WTF IS THIS??!?
		auto junk = DirstatView->SetFocus( );
		if ( junk != NULL ) {
			junk = { NULL };//Don't use return CWnd* right now.
			}
		else if ( junk == NULL ) {
			TRACE( _T( "I'm told I set focus to NULL. That's weird.\r\n" ) );
			}
		}

	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final {
		if ( !( STATIC_DOWNCAST( CDirstatDoc, m_pDocument ) )->IsRootDone( ) ) {
			Inactivate( );
			}

		switch ( lHint ) {
				case UpdateAllViews_ENUM::HINT_NEWROOT:
					EmptyView( );
					return CView::OnUpdate( pSender, lHint, pHint );

				case 0:
				case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
				case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
				case UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED:
				case UpdateAllViews_ENUM::HINT_EXTENSIONSELECTIONCHANGED:
					return CView::OnUpdate( pSender, lHint, pHint );


				case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
					VERIFY( RedrawWindow( ) );
					return;

				case UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED:
					Inactivate( );
					return CView::OnUpdate( pSender, lHint, pHint );

				default:
					return;
			}
		}
	
	afx_msg void OnContextMenu( CWnd* /*pWnd*/, CPoint ptscreen ) {
		//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		if ( Document == NULL ) {
			TRACE( _T( "User tried to open a Context Menu, but the Document is NULL. Well, they'll get what they asked for: a (NULL context) menu :)\r\n" ) );//(NULL context) menu == no context menu
			return;
			}
		const auto root = Document->m_rootItem.get( );
		if ( root == NULL ) {
			TRACE( _T( "User tried to open a Context Menu, but there are no items in the Document. Well, they'll get what they asked for: a (NULL context) menu :)\r\n" ) );//(NULL context) menu == no context menu
			return;
			}
		if ( !( root->m_attr.m_done ) ) {
			return;
			}
		CMenu menu;
		VERIFY( menu.LoadMenuW( IDR_POPUPGRAPH ) );
		const auto sub = menu.GetSubMenu( 0 );
		ASSERT( sub != NULL );//How the fuck could we ever get NULL from that???!?
		if ( sub == NULL ) {
			return;
			}
		VERIFY( sub->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, ptscreen.x, ptscreen.y, AfxGetMainWnd( ) ) );
		}


	afx_msg void OnMouseMove( UINT /*nFlags*/, CPoint point ) {
		//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		//Perhaps surprisingly, Document == NULL CAN be a valid condition. We don't have to set the message to anything if there's no document.
		auto guard = WDS_SCOPEGUARD_INSTANCE( [&]{ reset_timer_if_zero( ); } );
		if ( Document == NULL ) {
			//return reset_timer_if_zero( );
			return;
			}
		const auto root = Document->m_rootItem.get( );
		if ( root == NULL ) {
			TRACE( _T( "FindItemByPoint CANNOT find a point when given a NULL root! So let's not try.\r\n" ) );
			//return reset_timer_if_zero( );
			return;
			}

		ASSERT( root->m_attr.m_done );

		if ( !( root->m_attr.m_done ) ) {
			displayWindowsMsgBoxWithMessage( L"CGraphView::OnMouseMove: root item is NOT done! This should never happen!" );
			std::terminate( );
			}

		if ( !IsDrawn( ) ) {
			//return reset_timer_if_zero( );
			return;
			}
		const auto item = static_cast< const CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
		if ( item == NULL ) {
			TRACE( _T( "There's nothing with a path, therefore nothing for which we can set the message text.\r\n" ) );
			//return reset_timer_if_zero( );
			return;
			}
		ASSERT( m_frameptr != NULL );
		if ( m_frameptr == NULL ) {
			//return reset_timer_if_zero( );
			return;
			}
#ifdef DEBUG
		trace_focused_mouspos( point.x, point.y, item->GetPath( ).c_str( ) );
#endif
		//TRACE( _T( "focused & Mouse on tree map!(x: %ld, y: %ld), %s\r\n" ), point.x, point.y, item->GetPath( ).c_str( ) );
		m_frameptr->SetMessageText( item->GetPath( ).c_str( ) );

		reset_timer_if_zero( );
		guard.dismiss( );
		}

	afx_msg void OnDestroy( ) {
		if ( m_timer != 0 ) {
			VERIFY( KillTimer( m_timer ) );
			}
		m_timer = 0;
		CView::OnDestroy( );
		}
	
	afx_msg void OnTimer( UINT_PTR /*nIDEvent*/ ) {
		WTL::CPoint point;
		VERIFY( GetCursorPos( &point ) );
		ScreenToClient( &point );

		RECT rc;
		/*
	_AFXWIN_INLINE void CWnd::GetClientRect(LPRECT lpRect) const
		{ ASSERT(::IsWindow(m_hWnd)); ::GetClientRect(m_hWnd, lpRect); }
		*/
		ASSERT( ::IsWindow( m_hWnd ) );

		//"If [GetClientRect] succeeds, the return value is nonzero. To get extended error information, call GetLastError."
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );

		if ( !PtInRect( &rc, point ) ) {
			TRACE( _T( "Mouse has left the tree map area!\r\n" ) );
			m_frameptr->SetSelectionMessageText( );
			VERIFY( KillTimer( m_timer ) );
			m_timer = 0;
			}
		}

public:
	#ifdef _DEBUG
	virtual void AssertValid( ) const final {
		CView::AssertValid( );
		}
	virtual void Dump( CDumpContext& dc ) const final {
		TRACE( _T( "CGraphView::Dump\r\n" ) );
		CView::Dump( dc );
		}
	#endif
		//afx_msg void OnPopupCancel();
	};

#else

#endif
