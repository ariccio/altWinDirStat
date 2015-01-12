// graphview.h	- Declaration of CGraphView (the Treemap view)
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

#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H
#else
#error ass
#endif



#pragma once


#include "stdafx.h"
#include "treemap.h"

class CDirstatDoc;
class CItemBranch;
class CGraphView;


// CGraphView. The treemap window.
class CGraphView final : public CView {
protected:
	CGraphView( ) : m_recalculationSuspended( false ), m_showTreemap( true ), m_timer( 0 ) {
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

	void EmptyView( ) {
		if ( m_bitmap.m_hObject != NULL ) {
			m_bitmap.DeleteObject( );
			}
		if ( m_dimmed.m_hObject != NULL ) {
			m_dimmed.DeleteObject( );
			}
		}

	void RenderHighlightRectangle  ( _In_ CDC& pdc, _In_       CRect& rc                           ) const;
	void DrawEmptyView             ( _In_ CDC& pScreen_Device_Context                                                 );
	//void DrawZoomFrame             ( _In_ CDC& pdc, _In_       CRect& rc                           );
	void DrawHighlights            ( _In_ CDC& pdc                                                 ) const;
	void DrawHighlightExtension    ( _In_ CDC& pdc                                                 ) const;
	void TweakSizeOfRectangleForHightlight( _In_ CRect& rc, _In_ CRect& rcClient ) const;
	void RecurseHighlightExtension ( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const;
	void DrawSelection             ( _In_ CDC& pdc) const;
	void DoDraw( _In_ CDC& pDC, _In_ CDC& offscreen_buffer, _In_ CRect& rc );
	void DrawViewNotEmpty( _In_ CDC& Screen_Device_Context );

	void RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const;

public:
	bool m_recalculationSuspended : 1; // True while the user is resizing the window.	
	bool m_showTreemap            : 1; // False, if the user switched off the treemap (by F9).

protected:
	WTL::CSize    m_size;				// Current size of view
	CTreemap m_treemap;				// Treemap generator
	CBitmap  m_bitmap;				// Cached view. If m_hObject is NULL, the view must be recalculated.
	WTL::CSize    m_dimmedSize;			// Size of bitmap m_dimmed
	CBitmap  m_dimmed;				// Dimmed view. Used during refresh to avoid the ooops-effect.
	UINT_PTR m_timer;				// We need a timer to realize when the mouse left our window.

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

//#ifndef _DEBUG  // Debugversion in graphview.cpp
//_Must_inspect_result_ inline CDirstatDoc* CGraphView::GetDocument( ) {
//	return reinterpret_cast< CDirstatDoc* >( m_pDocument );
//	}
//#endif


// $Log$
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
