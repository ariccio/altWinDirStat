// graphview.cpp: Implementation of CGraphView
//
// see `file_header_text.txt` for licensing & contact info.

#pragma once

#include "stdafx.h"

#ifndef WDS_GRAPHVIEW_CPP
#define WDS_GRAPHVIEW_CPP

#include "dirstatview.h"


//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "ownerdrawnlistcontrol.h"
#include "TreeListControl.h"
#include "item.h"
#include "typeview.h"


#include "windirstat.h"
#include "dirstatdoc.h"
#include "graphview.h"
#include "options.h"
#include "mainframe.h"


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

void CGraphView::DrawEmptyView( _In_ CDC& pScreen_Device_Context ) {
	const COLORREF gray = RGB( 160, 160, 160 );
	Inactivate( );

	CRect rc;
	GetClientRect( rc );

	if ( m_dimmed.m_hObject == NULL ) {
		return pScreen_Device_Context.FillSolidRect( rc, gray );
		}
	CDC offscreen_buffer;
	VERIFY( offscreen_buffer.CreateCompatibleDC( &pScreen_Device_Context ) );
	CSelectObject sobmp( offscreen_buffer, m_dimmed );
	VERIFY( pScreen_Device_Context.BitBlt( rc.left, rc.top, m_dimmedSize.cx, m_dimmedSize.cy, &offscreen_buffer, 0, 0, SRCCOPY ) );

	if ( rc.Width( ) > m_dimmedSize.cx ) {
		CRect r = rc;
		r.left = r.left + m_dimmedSize.cx;
		pScreen_Device_Context.FillSolidRect( r, gray );
		}

	if ( rc.Height( ) > m_dimmedSize.cy ) {
		CRect r = rc;
		r.top = r.top + m_dimmedSize.cy;
		pScreen_Device_Context.FillSolidRect( r, gray );
		}
	//VERIFY( dcmem.DeleteDC( ) );
	}

void CGraphView::DoDraw( _In_ CDC& pDC, _In_ CDC& offscreen_buffer, _In_ RECT& rc ) {
	WTL::CWaitCursor wc;

	VERIFY( m_bitmap.CreateCompatibleBitmap( &pDC, m_size.cx, m_size.cy ) );

	CSelectObject sobmp( offscreen_buffer, m_bitmap );
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto Options = GetOptions( );
		const auto rootItem = Document->m_rootItem.get( );
		ASSERT( rootItem != NULL );
		if ( rootItem != NULL ) {
			m_treemap.DrawTreemap( offscreen_buffer, rc, rootItem, &( Options->m_treemapOptions ) );
			}
#ifdef _DEBUG
		if ( rootItem != NULL ) {
			m_treemap.RecurseCheckTree( rootItem );
			}
#endif
		}
	ASSERT( Document != NULL );
	// Cause OnIdle() to be called once.
	PostAppMessageW( GetCurrentThreadId( ), WM_NULL, 0, 0 );
	}

void CGraphView::DrawViewNotEmpty( _In_ CDC& Screen_Device_Context ) {
	CRect rc;
	GetClientRect( rc );
	ASSERT( rc.TopLeft( ) == WTL::CPoint( 0, 0 ) );

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

void CGraphView::OnDraw( CDC* pScreen_Device_Context ) {
	ASSERT_VALID( pScreen_Device_Context );
	const auto aDocument = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( aDocument != NULL ) {
		const auto root = aDocument->m_rootItem.get( );
		if ( root != NULL && root->m_attr.m_done ) {
			if ( m_recalculationSuspended || !m_showTreemap ) {
				// TODO: draw something interesting, e.g. outline of the first level.
				DrawEmptyView( *pScreen_Device_Context );
				}
			else {
				DrawViewNotEmpty( *pScreen_Device_Context );
				}
			}
		else {
			DrawEmptyView( *pScreen_Device_Context );
			}
		}
	ASSERT( aDocument != NULL );
	}


void CGraphView::DrawHighlights( _In_ CDC& pdc ) const {
	ASSERT( m_frameptr == GetMainFrame( ) );
	const auto logicalFocus = m_frameptr->m_logicalFocus;
	if ( logicalFocus == LOGICAL_FOCUS::LF_DIRECTORYLIST ) {
		DrawSelection( pdc );
		}
	if ( logicalFocus == LOGICAL_FOCUS::LF_EXTENSIONLIST ) {
		DrawHighlightExtension( pdc );
		}

	GetApp( )->PeriodicalUpdateRamUsage( );
	}

void CGraphView::DrawHighlightExtension( _In_ CDC& pdc ) const {
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

void CGraphView::RecurseHighlightExtension( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
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

void CGraphView::TweakSizeOfRectangleForHightlight( _Inout_ RECT& rc, _Inout_ RECT& rcClient ) const {
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
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto item = Document->m_selectedItem;
		if ( item == NULL ) {//no selection to draw.
			return;
			}
		CRect rcClient;
		GetClientRect( rcClient );

		RECT rc = item->TmiGetRectangle( );

		TweakSizeOfRectangleForHightlight( rc, rcClient );

		CSelectStockObject sobrush( pdc, NULL_BRUSH );
		const auto Options = GetOptions( );
		CPen pen( PS_SOLID, 1, Options->m_treemapHighlightColor );
		CSelectObject sopen( pdc, pen );
		RenderHighlightRectangle( pdc, rc );
		}
	ASSERT( Document != NULL );
	}

void CGraphView::RenderHighlightRectangle( _In_ CDC& pdc, _In_ RECT rc_ ) const {
	/*
	  The documentation of CDC::Rectangle() says that the width and height must be greater than 2. Experiment says that it must be greater than 1. We follow the documentation.
	  A pen and the null brush must be selected.
	  */

	auto rc = CRect( rc_ );

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
		const auto Options = GetOptions( );
		return pdc.FillSolidRect( rc, Options->m_treemapHighlightColor );
		}
	}

#ifdef _DEBUG
void CGraphView::AssertValid( ) const {
	CView::AssertValid( );
	}

void CGraphView::Dump( CDumpContext& dc ) const {
	TRACE( _T( "CGraphView::Dump\r\n" ) );
	CView::Dump( dc );
	}
#endif

void CGraphView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize( nType, cx, cy );
	WTL::CSize sz( cx, cy );
	if ( sz != m_size ) {
		Inactivate( );
		m_size = sz;
		}
	}

void CGraphView::OnLButtonDown( UINT nFlags, CPoint point ) {
	//auto Document = static_cast< CDirstatDoc* >( m_pDocument );
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto root = Document->m_rootItem.get( );
		if ( root != NULL && root->m_attr.m_done && IsDrawn( ) ) {
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
	if ( m_bitmap.m_hObject != NULL ) {
		m_dimmed.Detach( );
		VERIFY( m_dimmed.Attach( m_bitmap.Detach( ) ) );
		m_dimmedSize = m_size;
		}
	}

void CGraphView::OnSetFocus(CWnd* /*pOldWnd*/) {
	ASSERT( m_frameptr == GetMainFrame( ) );
	if ( m_frameptr != NULL ) {
		const auto DirstatView = m_frameptr->GetDirstatView( );
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
	ASSERT( m_frameptr != NULL );
	}

void CGraphView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) {
	if ( !( STATIC_DOWNCAST( CDirstatDoc, m_pDocument ) )->IsRootDone( ) ) {
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
			VERIFY( RedrawWindow( ) );
			return;

		//case UpdateAllViews_ENUM::HINT_ZOOMCHANGED:
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
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto root = Document->m_rootItem.get( );
		if ( root != NULL ) {
			if ( root->m_attr.m_done ) {
				CMenu menu;
				VERIFY( menu.LoadMenuW( IDR_POPUPGRAPH ) );
				const auto sub = menu.GetSubMenu( 0 );
				if ( sub != NULL ) {
					VERIFY( sub->TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON, ptscreen.x, ptscreen.y, AfxGetMainWnd( ) ) );
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
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		const auto root = Document->m_rootItem.get( );
		if ( root != NULL ) {
			if ( root->m_attr.m_done && IsDrawn( ) ) {
				auto item = static_cast<const CItemBranch* >( m_treemap.FindItemByPoint( root, point ) );
				if ( item != NULL ) {
					ASSERT( m_frameptr != NULL );
					ASSERT( GetMainFrame( ) == m_frameptr );
					if ( m_frameptr != NULL ) {
						TRACE( _T( "focused & Mouse on tree map!(x: %ld, y: %ld), %s\r\n" ), point.x, point.y, item->GetPath( ).c_str( ) );
						m_frameptr->SetMessageText( ( item->GetPath( ).c_str( ) ) );
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
		VERIFY( KillTimer( m_timer ) );
		}
	m_timer = 0;
	CView::OnDestroy( );
	}

void CGraphView::OnTimer( UINT_PTR /*nIDEvent*/ ) {
	WTL::CPoint point;
	VERIFY( GetCursorPos( &point ) );
	ScreenToClient( &point );

	RECT rc;
	GetClientRect( &rc );

	if ( !PtInRect( &rc, point ) ) {
		TRACE( _T( "Mouse has left the tree map area!\r\n" ) );
		ASSERT( GetMainFrame( ) == m_frameptr );
		m_frameptr->SetSelectionMessageText( );
		VERIFY( KillTimer( m_timer ) );
		m_timer = 0;
		}
	}

void CGraphView::RecurseHighlightChildren( _In_ CDC& pdc, _In_ const CItemBranch& item, _In_ const std::wstring& ext ) const {
	const auto childCount = item.m_childCount;
	const auto item_m_children = item.m_children.get( );

	//Not vectorized: 1200, loop contains data dependencies
	for ( size_t i = 0; i < childCount; ++i ) {
		RecurseHighlightExtension( pdc, *( item_m_children + i ), ext );
		}
	}


#else

#endif