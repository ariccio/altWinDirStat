// graphview.cpp: Implementation of CGraphView
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
#include "dirstatview.h"
#include "item.h"
#include "windirstat.h"
#include "dirstatdoc.h"
#include "graphview.h"
#include "options.h"
#include "mainframe.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CGraphView, CView)

BEGIN_MESSAGE_MAP(CGraphView, CView)
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_WM_MOUSEMOVE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//ON_COMMAND(ID_POPUP_CANCEL, OnPopupCancel)
END_MESSAGE_MAP()

void CGraphView::DrawEmptyView( _In_ CDC& pDC ) {
	//ASSERT_VALID( pDC );
	const COLORREF gray = RGB( 160, 160, 160 );
	Inactivate( );

	CRect rc;
	GetClientRect( rc );

	if ( m_dimmed.m_hObject == NULL ) {
		return pDC.FillSolidRect( rc, gray );
		}
	CDC dcmem;
	dcmem.CreateCompatibleDC( &pDC );
	CSelectObject sobmp( dcmem, m_dimmed );
	pDC.BitBlt( rc.left, rc.top, m_dimmedSize.cx, m_dimmedSize.cy, &dcmem, 0, 0, SRCCOPY );

	if ( rc.Width( ) > m_dimmedSize.cx ) {
		CRect r = rc;
		r.left = r.left + m_dimmedSize.cx;
		pDC.FillSolidRect( r, gray );
		}

	if ( rc.Height( ) > m_dimmedSize.cy ) {
		CRect r = rc;
		r.top = r.top + m_dimmedSize.cy;
		pDC.FillSolidRect( r, gray );
		}
	}

void CGraphView::DoDraw( _In_ CDC& pDC, _In_ CDC& dcmem, _In_ CRect& rc ) {
	//LockWindowUpdate( );
	CWaitCursor wc;

	m_bitmap.CreateCompatibleBitmap( &pDC, m_size.cx, m_size.cy );

	CSelectObject sobmp( dcmem, m_bitmap );
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		
		
		//if ( Document->IsZoomed( ) ) {
		//	DrawZoomFrame( dcmem, rc );
		//	}
		auto Options = GetOptions( );
		if ( Options != NULL ) {
			//const auto zoomItem = Document->GetZoomItem( );
			//if ( zoomItem != NULL ) {
				//m_treemap.DrawTreemap( dcmem, rc, zoomItem, &( Options->m_treemapOptions ) );
				//}
			//else {
				//auto rootItem = Document->GetRootItem( );
				//ASSERT( rootItem != NULL );
				//if ( rootItem != NULL ) {
				//	m_treemap.DrawTreemap( dcmem, rc, rootItem, &( Options->m_treemapOptions ) );
				//	}
				//}

			auto rootItem = Document->m_rootItem.get( );
			ASSERT( rootItem != NULL );
			if ( rootItem != NULL ) {
				m_treemap.DrawTreemap( dcmem, rc, rootItem, &( Options->m_treemapOptions ) );
				}

#ifdef _DEBUG
				{
					if ( rootItem != NULL ) {
						m_treemap.RecurseCheckTree( rootItem );
						}
				}
#endif
			}
		ASSERT( Options != NULL ); //fall back to default options?
		}
	ASSERT( Document != NULL );
	//UnlockWindowUpdate( );
	// Cause OnIdle() to be called once.
	PostAppMessageW( GetCurrentThreadId( ), WM_NULL, 0, 0 );
	}

void CGraphView::DrawViewNotEmpty( _In_ CDC& pDC ) {
	CRect rc;
	GetClientRect( rc );
	ASSERT( m_size == rc.Size( ) );
	ASSERT( rc.TopLeft( ) == CPoint( 0, 0 ) );

	CDC dcmem;
	dcmem.CreateCompatibleDC( &pDC );

	if ( !IsDrawn( ) ) {
		DoDraw( pDC, dcmem, rc );
		}

	CSelectObject sobmp2( dcmem, m_bitmap );
	pDC.BitBlt( 0, 0, m_size.cx, m_size.cy, &dcmem, 0, 0, SRCCOPY );

	DrawHighlights( pDC );
	
	}

void CGraphView::OnDraw( CDC* pDC ) {
	ASSERT_VALID( pDC );
	auto aDocument = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( aDocument != NULL ) {
		auto root = aDocument->m_rootItem.get( );
		if ( root != NULL && root->IsTreeDone( ) ) {
			if ( m_recalculationSuspended || !m_showTreemap ) {
				// TODO: draw something interesting, e.g. outline of the first level.
				DrawEmptyView( *pDC );
				}
			else {
				DrawViewNotEmpty( *pDC );
				}
			}
		else {
			DrawEmptyView( *pDC );
			}
		}
	ASSERT( aDocument != NULL );
	}

//void CGraphView::DrawZoomFrame( _In_ CDC& pdc, _In_ CRect& rc ) {
//	//ASSERT_VALID( pdc );
//	const INT w = 4;
//	CRect r;
//	
//	r = rc;
//	r.bottom = r.top + w;
//	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
//	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
//	if ( Document != NULL ) {
//		pdc.FillSolidRect( r, RGB( 0, 0, 255 ) );
//
//		r = rc;
//		r.top = r.bottom - w;
//		pdc.FillSolidRect( r, RGB( 0, 0, 255 ) );
//
//		r = rc;
//		r.right = r.left + w;
//		pdc.FillSolidRect( r, RGB( 0, 0, 255 ) );
//
//		r = rc;
//		r.left = r.right - w;
//		pdc.FillSolidRect( r, RGB( 0, 0, 255 ) );
//
//		rc.DeflateRect( w, w );
//		}
//	else {
//		ASSERT( Document != NULL );
//		//Fall back to some sane defaults?
//		r = rc;
//		r.top = r.bottom - w;
//
//		r = rc;
//		r.right = r.left + w;
//
//		r = rc;
//		r.left = r.right - w;
//
//		rc.DeflateRect( w, w );
//
//		}
//	}

void CGraphView::DrawHighlights( _In_ CDC& pdc ) const {
	//ASSERT_VALID( pdc );
	const auto logicalFocus = GetMainFrame( )->m_logicalFocus;
	if ( logicalFocus == focus::LF_DIRECTORYLIST ) {
		DrawSelection( pdc );
		}
	if ( logicalFocus == focus::LF_EXTENSIONLIST ) {
		DrawHighlightExtension( pdc );
		}

	GetApp( )->PeriodicalUpdateRamUsage( );
	}

void CGraphView::DrawHighlightExtension( _In_ CDC& pdc ) const {
	//ASSERT_VALID( pdc );
	CWaitCursor wc;

	CPen pen( PS_SOLID, 1, GetOptions( )->m_treemapHighlightColor );
	CSelectObject sopen( pdc, pen );
	CSelectStockObject sobrush( pdc, NULL_BRUSH );
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );;
	const auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document == NULL ) {
		ASSERT( Document != NULL );
		return;
		}
	//const auto zItem = Document->GetZoomItem( );
	//if ( zItem != NULL ) {
	//	//RecurseHighlightExtension( pdc, ( *zItem ), Document->GetHighlightExtension( ) );
	//	}
	//else {
	//	const auto rItem = Document->GetRootItem( );
	//	if ( rItem != NULL ) {
	//		RecurseHighlightExtension( pdc, ( *rItem ), Document->GetHighlightExtension( ) );
	//		}
	//	}
	const auto rItem = Document->m_rootItem.get( );
	if ( rItem != NULL ) {
		RecurseHighlightExtension( pdc, ( *rItem ), Document->m_highlightExtension );
		}
	}

void CGraphView::RecurseHighlightExtension( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
	//ASSERT_VALID( pdc );
	auto rc = item.m_rect;
	if ( ( rc.right - rc.left ) <= 0 || ( rc.bottom - rc.top ) <= 0 ) {
		return;
		}
	
	if ( item.m_type == IT_FILE ) {
		auto extensionStrPtr = item.CStyle_GetExtensionStrPtr( );
		auto scmp = wcscmp( extensionStrPtr, ext.c_str( ) );
		if ( scmp == 0 ) {
			auto rcc = item.TmiGetRectangle( );
			return RenderHighlightRectangle( pdc, rcc );
			}
		return;
		}

	RecurseHighlightChildren( pdc, item, ext );
	}

void CGraphView::TweakSizeOfRectangleForHightlight( _In_ CRect& rc, _In_ CRect& rcClient ) const {
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

void CGraphView::DrawSelection( _In_ CDC& pdc ) const {
	//ASSERT_VALID( pdc );
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto item = Document->m_selectedItem;
		if ( item == NULL ) {//no selection to draw.
			return;
			}
		CRect rcClient;
		GetClientRect( rcClient );

		auto rc = item->TmiGetRectangle( );

		TweakSizeOfRectangleForHightlight( rc, rcClient );

		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		auto Options = GetOptions( );
		if ( Options != NULL ) {
			CPen pen( PS_SOLID, 1, Options->m_treemapHighlightColor );
			CSelectObject sopen( pdc, pen );
			}
		ASSERT( Options != NULL );
		RenderHighlightRectangle( pdc, rc );
		}
	ASSERT( Document != NULL );
	}

void CGraphView::RenderHighlightRectangle( _In_ CDC& pdc, _In_ CRect& rc ) const {
	/*
	  The documentation of CDC::Rectangle() says that the width and height must be greater than 2. Experiment says that it must be greater than 1. We follow the documentation.
	  A pen and the null brush must be selected.
	  */

	//ASSERT_VALID( pdc );
	ASSERT( rc.Width( ) >= 0 );
	ASSERT( rc.Height( ) >= 0 );

	if ( rc.Width( ) >= 7 && rc.Height( ) >= 7 ) {
		pdc.Rectangle( rc );		// w = 7
		rc.DeflateRect( 1, 1 );
		pdc.Rectangle( rc );		// w = 5
		rc.DeflateRect( 1, 1 );
		pdc.Rectangle( rc );		// w = 3
		}
	else {
		auto Options = GetOptions( );
		ASSERT( Options != NULL );
		if ( Options != NULL ) {
			return pdc.FillSolidRect( rc, Options->m_treemapHighlightColor );
			}
		pdc.FillSolidRect( rc, RGB( 64, 64, 140 ) );//Fall back to some value
		}
	}

#ifdef _DEBUG
void CGraphView::AssertValid( ) const {
	CView::AssertValid( );
	}

void CGraphView::Dump( CDumpContext& dc ) const {
	CView::Dump( dc );
	}
#endif

void CGraphView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize( nType, cx, cy );
	CSize sz( cx, cy );
	if ( sz != m_size ) {
		Inactivate( );
		m_size = sz;
		}
	}

void CGraphView::OnLButtonDown( UINT nFlags, CPoint point ) {
	//auto Document = GetDocument( );
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto root = Document->m_rootItem.get( );
		if ( root != NULL && root->IsTreeDone( ) && IsDrawn( ) ) {
			//const auto zoomItem = Document->GetZoomItem( );
			//CItemBranch* item = { NULL };
			//ASSERT( zoomItem != NULL );
			//if ( zoomItem != NULL ) {
			//	item = static_cast< CItemBranch* >( m_treemap.FindItemByPoint( zoomItem, point ) );
			//	}
			//else {
			//	item = static_cast< CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
			//	}
			const auto item = static_cast< CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
			if ( item == NULL ) {
				goto noItemOrDocument;
				}
			ASSERT( item != NULL );
			if ( item != NULL ) {
				Document->SetSelection( *item );
				}
			Document->UpdateAllViews( NULL, UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION );
			}
		}
	else {
		TRACE( _T( "User clicked on nothing. User CAN click on nothing. That's a sane case.\r\n" ) );
		}
noItemOrDocument://Yeah, I hate it, but goto CAN be the cleanest solution in certain low-level cases.
	CView::OnLButtonDown( nFlags, point );
	}

void CGraphView::Inactivate( ) {
	//TODO: this function gets called waaay too much. Why are we REsetting every pixel to RGB( 100, 100, 100 ) on every update?? 
	if ( m_bitmap.m_hObject != NULL ) {
		// Move the old bitmap to m_dimmed
		m_dimmed.DeleteObject( );
		m_dimmed.Attach( m_bitmap.Detach( ) );
		m_dimmedSize = m_size;

		// Dimm m_inactive
		CClientDC dc( this );
		CDC dcmem;
		dcmem.CreateCompatibleDC( &dc );
		CSelectObject sobmp( dcmem, m_dimmed );
		for ( INT x = 0; x < m_dimmedSize.cx; x += 2 ) {
			for ( INT y = 0; y < m_dimmedSize.cy; y += 2 ) {
				ASSERT( ( x % 2 ) == 0 );
				ASSERT( ( y % 2 ) == 0 );
				dcmem.SetPixel( x, y, RGB( 100, 100, 100 ) );
				}
			}
		}
	}

void CGraphView::OnSetFocus(CWnd* /*pOldWnd*/) {
	auto MainFrame = GetMainFrame( );
	if ( MainFrame != NULL ) {
		auto DirstatView = MainFrame->GetDirstatView( );
		if ( DirstatView != NULL ) {
			auto junk = DirstatView->SetFocus( );
			if ( junk != NULL ) {
				junk = { NULL };//Don't use return CWnd* right now.
				}
			else if ( junk == NULL ) {
				TRACE( _T( "I'm told I set focus to NULL. That's weird.\r\n" ) );
				}
			}
		ASSERT( DirstatView != NULL );
		}
	ASSERT( MainFrame != NULL );
	}

void CGraphView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) {
	if ( !( static_cast< CDirstatDoc* >( m_pDocument ) )->IsRootDone( ) ) {
		Inactivate( );
		}

	switch ( lHint )
	{
		case UpdateAllViews_ENUM::HINT_NEWROOT:
			EmptyView( );
			return CView::OnUpdate( pSender, lHint, pHint );

		case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
		case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
		case UpdateAllViews_ENUM::HINT_SELECTIONSTYLECHANGED:
		case UpdateAllViews_ENUM::HINT_EXTENSIONSELECTIONCHANGED:
			return CView::OnUpdate( pSender, lHint, pHint );


		case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
			RedrawWindow( );
			break;

		case UpdateAllViews_ENUM::HINT_ZOOMCHANGED:
		case UpdateAllViews_ENUM::HINT_TREEMAPSTYLECHANGED:
			Inactivate( );
			return CView::OnUpdate( pSender, lHint, pHint );

		case 0:
			return CView::OnUpdate( pSender, lHint, pHint );

		default:
			return;
	}
	}

void CGraphView::OnContextMenu(CWnd* /*pWnd*/, CPoint ptscreen) {
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		auto root = Document->m_rootItem.get( );
		if ( root != NULL ) {
			if ( root->IsTreeDone( ) ) {
				CMenu menu;
				menu.LoadMenu( IDR_POPUPGRAPH );
				auto sub = menu.GetSubMenu( 0 );
				if ( sub != NULL ) {
					sub->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, ptscreen.x, ptscreen.y, AfxGetMainWnd( ) );
					}
				ASSERT( sub != NULL );//How the fuck could we ever get NULL from that???!?
				}
			}
		else {
			TRACE( _T( "User tried to open a Context Menu, but there are no items in the Document. Well, they'll get what they asked for: a (NULL context) menu :)\r\n" ) );//(NULL context) menu == no context menu
			}
		}
	else { 
		TRACE( _T( "User tried to open a Context Menu, but the Document is NULL. Well, they'll get what they asked for: a (NULL context) menu :)\r\n" ) );//(NULL context) menu == no context menu
		}
	}

void CGraphView::OnMouseMove( UINT /*nFlags*/, CPoint point ) {
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		auto root = Document->m_rootItem.get( );
		if ( root != NULL ) {
			if ( root->IsTreeDone( ) && IsDrawn( ) ) {
				//if ( ZoomItem != NULL ) {
				//	}
				//else {
				//	TRACE( _T( "FindItemByPoint CANNOT find a point when given a NULL ZoomItem! So let's not try.\r\n" ) );
				//	}
				auto item = static_cast<const CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
				if ( item != NULL ) {
					auto MainFrame = GetMainFrame( );
					ASSERT( MainFrame != NULL );
					if ( MainFrame != NULL ) {
						TRACE( _T( "Window focused, Mouse over tree map!(x: %ld, y: %ld), Item: %s.\r\n" ), point.x, point.y, item->GetPath( ).c_str( ) );
						MainFrame->SetMessageText( ( item->GetPath( ).c_str( ) ) );
						}
					}
				else {
					TRACE( _T( "There's nothing with a path, therefore nothing for which we can set the message text.\r\n" ) );
					}
				}
			else {
				TRACE( _T( "FindItemByPoint CANNOT find a point when given a NULL root! So let's not try.\r\n" ) );
				}
			}
		}
		//Perhaps surprisingly, Document == NULL CAN be a valid condition. We don't have to set the message to anything if there's no document.
	if ( m_timer == 0 ) {
		TRACE( _T( "Mouse has left the tree map area?\r\n" ) );
		m_timer = SetTimer( 4711, 100, NULL );//TODO: figure out what the hell this does.//if value is increased ( argument 2 ), program execution will take longer to reach `TRACE( _T( "Mouse has left the tree map area!\r\n" ) );` after mouse has left tree map area.
		}
	}

void CGraphView::OnDestroy( ) {
	if ( m_timer != NULL ) {
		KillTimer( m_timer );
		}
	m_timer = 0;
	CView::OnDestroy( );
	}

void CGraphView::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	CPoint point;
	GetCursorPos( &point );
	ScreenToClient( &point );

	CRect rc;
	GetClientRect( rc );

	if ( !rc.PtInRect( point ) ) {
		TRACE( _T( "Mouse has left the tree map area!\r\n" ) );
		GetMainFrame( )->SetSelectionMessageText( );
		KillTimer( m_timer );
		m_timer = 0;
		}
	}

void CGraphView::RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
#ifdef ARRAYTEST
	for ( size_t i = 0; i < childSizeCount( ( &item ) ); ++i ) {
		RecurseHighlightExtension( pdc, *( item.m_children + ( i ) ), ext );
		}
#else
	for ( const auto& child : item.m_children ) {
		ASSERT( child != NULL );
		if ( child != NULL ) {
			RecurseHighlightExtension( pdc, ( *child ), ext );
			}
		}
#endif

	}


//void CGraphView::OnPopupCancel( ) { }

// $Log$
// Revision 1.6  2004/11/09 22:31:59  assarbad
// - Removed obsolete code lines
//
// Revision 1.5  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
