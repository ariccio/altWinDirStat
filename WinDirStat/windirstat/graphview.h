// graphview.h	- Declaration of CGraphView (the Treemap view)
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_H
#define WDS_GRAPHVIEW_H

#include "treemap.h"
#include "windirstat.h"

class CDirstatDoc;
class CItemBranch;
class CGraphView;


// CGraphView. The treemap window.
class CGraphView final : public CView {
protected:
	CGraphView( ) : m_recalculationSuspended( false ), m_showTreemap( true ), m_timer( 0 ), m_frameptr( GetMainFrame( ) ) {
		m_size.cx = 0;
		m_size.cy = 0;
		m_dimmedSize.cx = 0;
		m_dimmedSize.cy = 0;
		}
	DECLARE_DYNCREATE(CGraphView)

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
	
	virtual void OnDraw( CDC* pScreen_Device_Context ) override final;
	
	bool IsDrawn( ) const {
		return ( m_bitmap.m_hObject != NULL );
		}

	void Inactivate( );
	void reset_timer_if_zero( );
	void EmptyView( ) {
		if ( m_bitmap.m_hObject != NULL ) {
			m_bitmap.DeleteObject( );
			}
		if ( m_dimmed.m_hObject != NULL ) {
			m_dimmed.DeleteObject( );
			}
		}

	void cause_OnIdle_to_be_called_once( ) const;
	void RenderHighlightRectangle  ( _In_ CDC& pdc, _In_       RECT rc_                           ) const;
	void DrawEmptyView             ( _In_ CDC& pScreen_Device_Context                                                 );
	//void DrawZoomFrame             ( _In_ CDC& pdc, _In_       CRect& rc                           );
	void DrawHighlights            ( _In_ CDC& pdc                                                 ) const;
	void DrawHighlightExtension    ( _In_ CDC& pdc                                                 ) const;
	void TweakSizeOfRectangleForHightlight( _Inout_ RECT& rc, _Inout_ RECT& rcClient ) const;
	void RecurseHighlightExtension ( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const;
	void DrawSelection             ( _In_ CDC& pdc) const;
	void DoDraw( _In_ CDC& pDC, _In_ CDC& offscreen_buffer, _In_ RECT& rc );
	void DrawViewNotEmpty( _In_ CDC& Screen_Device_Context );

	//only called from one place
	inline void RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const;

public:
	bool m_recalculationSuspended : 1; // True while the user is resizing the window.	
	//C4820: 'CGraphView' : '3' bytes padding added after data member 'CGraphView::m_showTreemap'
	bool m_showTreemap            : 1; // False, if the user switched off the treemap (by F9).

protected:
	//C4820: 'CGraphView' : '4' bytes padding added after data member 'CGraphView::m_size'
	WTL::CSize    m_size;				// Current size of view
	CTreemap m_treemap;				// Treemap generator
	CBitmap  m_bitmap;				// Cached view. If m_hObject is NULL, the view must be recalculated.
	WTL::CSize    m_dimmedSize;			// Size of bitmap m_dimmed
	CBitmap  m_dimmed;				// Dimmed view. Used during refresh to avoid the ooops-effect.
	UINT_PTR m_timer;				// We need a timer to realize when the mouse left our window.
	CMainFrame* const m_frameptr;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	virtual void OnUpdate(CView * pSender, LPARAM lHint, CObject *pHint) override final;
	afx_msg void OnContextMenu(CWnd *pWnd, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

public:
	#ifdef _DEBUG
		virtual void AssertValid() const;
		virtual void Dump(CDumpContext& dc) const;
	#endif
		//afx_msg void OnPopupCancel();
	};

#else

#endif
