// dirstatview.cpp : Implementation of CDirstatView
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
#include "item.h"
#include "options.h"
#include "dirstatview.h"

#include "dirstatdoc.h"
#include "windirstat.h"
#include "mainframe.h"
#include "globalhelpers.h"


namespace {
	const UINT _nIdTreeListControl = 4711;
	}

//BEGIN_MESSAGE_MAP(CMyTreeListControl, CTreeListControl)
//END_MESSAGE_MAP()
//
//CMyTreeListControl::CMyTreeListControl( ) : CTreeListControl( ITEM_ROW_HEIGHT ) { }

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE( CDirstatView, CView )

BEGIN_MESSAGE_MAP(CDirstatView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_ITEMCHANGED, _nIdTreeListControl, OnLvnItemchanged)
	ON_UPDATE_COMMAND_UI(ID_POPUP_TOGGLE, OnUpdatePopupToggle)
	ON_COMMAND(ID_POPUP_TOGGLE, OnPopupToggle)
END_MESSAGE_MAP()

void CDirstatView::OnSize( UINT nType, INT cx, INT cy ) {
	CView::OnSize( nType, cx, cy );
	if ( IsWindow( m_treeListControl.m_hWnd ) ) {
		CRect rc( 0, 0, cx, cy );
		m_treeListControl.MoveWindow( rc );
		}
	}

void CDirstatView::SetTreeListControlOptions( ) {
	auto Options = GetOptions( );
	       m_treeListControl.ShowGrid            ( Options->m_listGrid             );
	       m_treeListControl.ShowStripes         ( Options->m_listStripes          );
	return m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
	}

INT CDirstatView::OnCreate( LPCREATESTRUCT lpCreateStruct ) {
	if ( CView::OnCreate( lpCreateStruct ) == -1 ){
		return -1;
		}
	RECT rect = { 0, 0, 0, 0 };
	VERIFY( m_treeListControl.CreateEx( 0, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, rect, this, _nIdTreeListControl ) );
	m_treeListControl.AddExtendedStyle( LVS_EX_HEADERDRAGDROP );
	SetTreeListControlOptions( );
	m_treeListControl.InsertColumn( column::COL_NAME,              _T( "Name" ),               LVCFMT_LEFT,  200, column::COL_NAME );
	m_treeListControl.InsertColumn( column::COL_PERCENTAGE,        _T( "Percentage" ),         LVCFMT_RIGHT,  55, column::COL_PERCENTAGE );
	m_treeListControl.InsertColumn( column::COL_SUBTREETOTAL,      _T( "Size" ),               LVCFMT_RIGHT,  90, column::COL_SUBTREETOTAL );
	m_treeListControl.InsertColumn( column::COL_ITEMS,             _T( "Items" ),              LVCFMT_RIGHT,  55, column::COL_ITEMS );
	m_treeListControl.InsertColumn( column::COL_FILES,             _T( "Files" ),              LVCFMT_RIGHT,  55, column::COL_FILES );
  //m_treeListControl.InsertColumn( column::COL_SUBDIRS,           _T( "Subdirs" ),            LVCFMT_RIGHT,  55, column::COL_SUBDIRS );
	m_treeListControl.InsertColumn( column::COL_LASTCHANGE,        _T( "Last Change" ),        LVCFMT_LEFT,  120, column::COL_LASTCHANGE );
	m_treeListControl.InsertColumn( column::COL_ATTRIBUTES,        _T( "Attributes" ),         LVCFMT_LEFT,   50, column::COL_ATTRIBUTES );

	m_treeListControl.OnColumnsInserted( );
	return 0;
	}

void CDirstatView::OnUpdatePopupToggle( _In_ CCmdUI* pCmdUI ) {
	pCmdUI->Enable( m_treeListControl.SelectedItemCanToggle( ) );
	}

void CDirstatView::OnPopupToggle( ) {
	m_treeListControl.ToggleSelectedItem( );
	}

void CDirstatView::OnSetFocus( CWnd* pOldWnd ) {
	UNREFERENCED_PARAMETER( pOldWnd );
	m_treeListControl.SetFocus( );
	}

void CDirstatView::OnDestroy( ) {
	CView::OnDestroy();
	}

CDirstatView::CDirstatView( ) : m_treeListControl( ITEM_ROW_HEIGHT ) {// Created by MFC only
	m_treeListControl.SetSorting( column::COL_SUBTREETOTAL, false );
	}

void CDirstatView::SysColorChanged( ) {
	m_treeListControl.SysColorChanged( );
	}


BOOL CDirstatView::OnEraseBkgnd( CDC* pDC ) {
	UNREFERENCED_PARAMETER( pDC );
	return TRUE;
	}



void CDirstatView::OnLvnItemchanged( NMHDR *pNMHDR, LRESULT *pResult ) {
	auto pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	( pResult != NULL ) ? ( *pResult = 0 ) : ASSERT( false );
	if ( ( pNMLV->uChanged & LVIF_STATE ) != 0 ) {
		if ( pNMLV->iItem == -1 ) {
			ASSERT( false ); // mal gucken //'watch times'?
			return;
			}
		// This is not true (don't know why): ASSERT(m_treeListControl.GetItemState(pNMLV->iItem, LVIS_SELECTED) == pNMLV->uNewState);
		bool selected = ( ( m_treeListControl.GetItemState( pNMLV->iItem, LVIS_SELECTED ) & LVIS_SELECTED ) != 0 );
		auto item = static_cast< CItemBranch * >( m_treeListControl.GetItem( pNMLV->iItem ) );
		if ( item != NULL ) {
			if ( selected ) {
				auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
				if ( Document != NULL ) {
					ASSERT( item != NULL );
					Document->SetSelection( *item );
					ASSERT( Document == m_pDocument );
					return m_pDocument->UpdateAllViews( this, UpdateAllViews_ENUM::HINT_SELECTIONCHANGED );
					}
				TRACE( _T( "I'm told that the selection has changed in a NULL document?!?? This can't be right.\r\n" ) );
				ASSERT( Document != NULL );
				}
			}
		ASSERT( item != NULL );//We got a NULL item??!? WTF
		}
	}

_Must_inspect_result_ CDirstatDoc* CDirstatView::GetDocument( ) {
	return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	}


void CDirstatView::OnUpdateHINT_NEWROOT( ) {
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		auto newRootItem = Document->m_rootItem.get( );
		if ( newRootItem != NULL ) {
			m_treeListControl.SetRootItem( newRootItem );
			VERIFY( m_treeListControl.RedrawItems( 0, m_treeListControl.GetItemCount( ) - 1 ) );
			}
		else {
			m_treeListControl.SetRootItem( newRootItem );
			VERIFY( m_treeListControl.RedrawItems( 0, m_treeListControl.GetItemCount( ) - 1 ) );
			}
		}
	ASSERT( Document != NULL );//The document is NULL??!? WTF
	}

void CDirstatView::OnUpdateHINT_SELECTIONCHANGED( ) {
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		auto Selection = Document->m_selectedItem;
		ASSERT( Selection != NULL );
		if ( Selection != NULL ) {
			return m_treeListControl.SelectAndShowItem( Selection, false );
			}
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		}
	ASSERT( Document != NULL );//The Document has a NULL root item??!?
	}

void CDirstatView::OnUpdateHINT_SHOWNEWSELECTION( ) {
	auto Document = DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	if ( Document != NULL ) {
		auto Selection = Document->m_selectedItem;
		if ( Selection != NULL ) {
			TRACE( _T( "New item selected! item: %s\r\n" ), Selection->GetPath( ).c_str( ) );
			return m_treeListControl.SelectAndShowItem( Selection, true );
			}
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		ASSERT( Selection != NULL );
		}
	ASSERT( Document != NULL );//The Document has a NULL root item??!?
	}

void CDirstatView::OnUpdateHINT_LISTSTYLECHANGED( ) {
	TRACE( _T( "List style has changed, redrawing!\r\n" ) );
	auto Options = GetOptions( );
	m_treeListControl.ShowGrid( Options->m_listGrid );
	m_treeListControl.ShowStripes( Options->m_listStripes );
	m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
	}

void CDirstatView::OnUpdateHINT_SOMEWORKDONE( ) {
	MSG msg;
	while ( PeekMessageW( &msg, m_treeListControl, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) ) {//TODO convert to GetMessage? peek message SPINS and PEGS a SINGLE core at 100%
		if ( msg.message == WM_QUIT ) {
			TRACE( _T( "OnUpdate, case HINT_SOMEWORKDONE: received message to quit!!\r\n" ) );

			PostQuitMessage( static_cast<int>( msg.wParam ) );//TODO: BAD IMPLICIT CONVERSION HERE!!! BUGBUG FIXME
			break;
			}
		VERIFY( TranslateMessage( &msg ) );
		DispatchMessageW( &msg );
		}
	}

void CDirstatView::OnUpdate( CView *pSender, LPARAM lHint, CObject *pHint ) {
	switch ( lHint )
	{
		case UpdateAllViews_ENUM::HINT_NEWROOT:
			return OnUpdateHINT_NEWROOT( );

		case UpdateAllViews_ENUM::HINT_SELECTIONCHANGED:
			return OnUpdateHINT_SELECTIONCHANGED( );

		case UpdateAllViews_ENUM::HINT_SHOWNEWSELECTION:
			return OnUpdateHINT_SHOWNEWSELECTION( );

		case UpdateAllViews_ENUM::HINT_REDRAWWINDOW:
			VERIFY( m_treeListControl.RedrawWindow( ) );
			break;

		case UpdateAllViews_ENUM::HINT_ZOOMCHANGED:
			return CView::OnUpdate( pSender, lHint, pHint );

		case UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED:
			return OnUpdateHINT_LISTSTYLECHANGED( );

		case UpdateAllViews_ENUM::HINT_SOMEWORKDONE:
			OnUpdateHINT_SOMEWORKDONE( );
			// fall thru
		case 0:
			CView::OnUpdate( pSender, lHint, pHint );

		default:
			return;
		}
	}

//#ifdef _DEBUG
////void CDirstatView::AssertValid() const {
////	CView::AssertValid( );
////	}
//
//void CDirstatView::Dump(CDumpContext& dc) const{
//	AfxCheckMemory( );
//	CView::Dump( dc );
//	}
//
//#endif //_DEBUG

// $Log$
// Revision 1.13  2004/11/25 21:13:38  assarbad
// - Implemented "attributes" column in the treelist
// - Adopted width in German dialog
// - Provided German, Russian and English version of IDS_TREECOL_ATTRIBUTES
//
// Revision 1.12  2004/11/24 20:28:12  bseifert
// Implemented context menu compromise.
//
// Revision 1.11  2004/11/12 09:27:01  assarbad
// - Implemented ExplorerStyle option which will not be accessible through the options dialog.
//   It handles where the context menu is being shown.
//
// Revision 1.10  2004/11/09 22:30:40  assarbad
// - Context menu at the actual position after right-click in the tree (may be reverted)
//
// Revision 1.9  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
