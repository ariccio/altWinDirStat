// graphview.h	- Declaration of CGraphView (the Treemap view)
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_H
#define WDS_GRAPHVIEW_H

#pragma message( "Including `" __FILE__ "`..." )

#include "datastructures.h"

//#include "macros_that_scare_small_children.h"
//#include "ScopeGuard.h"
#include "treemap.h"
//#include "windirstat.h"
#include "options.h"
//#include "globalhelpers.h"
#include "dirstatdoc.h"
#include "mainframe.h"
//#include "dirstatview.h"

class CDirstatDoc;
class CGraphView;


#ifdef DEBUG
void trace_empty_view_graphview( );
void trace_call_onidle( );
void trace_mouse_left( );
void trace_focused_mouspos( _In_ const LONG x, _In_ const LONG y, _In_z_ PCWSTR const path );
#endif

//TweakSizeOfRectangleForHightlight is called once, unconditionally.
inline void TweakSizeOfRectangleForHightlight( _Inout_ RECT& rc, _Inout_ RECT& rcClient, _In_ const bool grid );

// CGraphView. The treemap window.
class CGraphView final : public CView {
protected:
	CGraphView( );
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
			CWnd::Invalidate( );
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
		VERIFY( ::GetClassInfoW( ::AfxGetInstanceHandle( ), cs.lpszClass, &wc ) );
		wc.hbrBackground = { NULL };
		wc.lpszClassName = _T( "windirstat_graphview_class" );
		cs.lpszClass = reinterpret_cast<PCWSTR>( ::RegisterClassW( &wc ) );
	
		return TRUE;

		}
	
	virtual void OnInitialUpdate( ) override final {
		CView::OnInitialUpdate( );
		}
	
	virtual void OnDraw( CDC* pScreen_Device_Context ) override final;
	
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
		PostAppMessageW( ::GetCurrentThreadId( ), WM_NULL, 0, 0 );
		}

public:
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

protected:
	void DrawEmptyView( _In_ CDC& pScreen_Device_Context ) {
#ifdef DEBUG
		trace_empty_view_graphview( );
#endif
		const COLORREF gray = RGB( 160, 160, 160 );
		CGraphView::Inactivate( );

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


	void RecurseHighlightExtension( _In_ CDC& pdc, _In_ const CTreeListItem& item, _In_ const std::wstring& ext ) const;

	void DrawSelection( _In_ CDC& pdc ) const;
	
	void DoDraw( _In_ CDC& pDC, _In_ CDC& offscreen_buffer, _In_ RECT& rc );
	
	
	void DrawViewNotEmpty( _In_ CDC& Screen_Device_Context ) {
		RECT rc;

		ASSERT( ::IsWindow( m_hWnd ) );
		//::GetClientRect(m_hWnd, lpRect);
		VERIFY( ::GetClientRect( m_hWnd, &rc ) );
		//GetClientRect( &rc );

		CDC offscreen_buffer;
		VERIFY( offscreen_buffer.CreateCompatibleDC( &Screen_Device_Context ) );

		if ( !CGraphView::IsDrawn( ) ) {
			DoDraw( Screen_Device_Context, offscreen_buffer, rc );
			}

		CSelectObject sobmp2( offscreen_buffer, m_bitmap );
		VERIFY( Screen_Device_Context.BitBlt( 0, 0, m_size.cx, m_size.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

		DrawHighlights( Screen_Device_Context );
		//VERIFY( dcmem.DeleteDC( ) );
		}

public:

protected:
	
	void RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CTreeListItem& item, _In_ const std::wstring& ext ) const;

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
		CWnd::OnSize( nType, cx, cy );
		WTL::CSize sz( cx, cy );
		if ( sz != m_size ) {
			CGraphView::Inactivate( );
			m_size = sz;
			}
		}


	afx_msg void OnLButtonDown( UINT nFlags, CPoint point );

	afx_msg void OnSetFocus( CWnd* /*pOldWnd*/ );

	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final {
		if ( !( STATIC_DOWNCAST( CDirstatDoc, m_pDocument ) )->IsRootDone( ) ) {
			CGraphView::Inactivate( );
			}

		switch ( lHint ) {
				case UpdateAllViews_ENUM::HINT_NEWROOT:
					CGraphView::EmptyView( );
					return CView::OnUpdate( pSender, lHint, pHint );

				case 0:
				case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
				case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
				case UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED:
				case UpdateAllViews_ENUM::HINT_EXTENSIONSELECTIONCHANGED:
					return CView::OnUpdate( pSender, lHint, pHint );


				case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
					VERIFY( CWnd::RedrawWindow( ) );
					return;

				case UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED:
					CGraphView::Inactivate( );
					return CView::OnUpdate( pSender, lHint, pHint );

				default:
					return;
			}
		}
	
	afx_msg void OnContextMenu( CWnd* /*pWnd*/, CPoint ptscreen );

	afx_msg void OnMouseMove( UINT /*nFlags*/, CPoint point );

	afx_msg void OnDestroy( ) {
		if ( m_timer != 0 ) {
			VERIFY( CWnd::KillTimer( m_timer ) );
			}
		m_timer = 0;
		CView::OnDestroy( );
		}
	
	afx_msg void OnTimer( UINT_PTR /*nIDEvent*/ ) {
		WTL::CPoint point;
		VERIFY( ::GetCursorPos( &point ) );
		CWnd::ScreenToClient( &point );

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
			VERIFY( CWnd::KillTimer( m_timer ) );
			m_timer = 0;
			}
		}
	};

#else

#endif
