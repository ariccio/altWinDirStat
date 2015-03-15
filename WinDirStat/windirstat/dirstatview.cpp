// dirstatview.cpp : Implementation of CDirstatView
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.

#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTATVIEW_CPP
#define WDS_DIRSTATVIEW_CPP

WDS_FILE_INCLUDE_MESSAGE

//
#include "dirstatview.h"
#include "dirstatdoc.h"

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_OnEraseBkgnd( ) {
	TRACE( _T( "CDirstatView::OnEraseBkgnd!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_LoadingPerst( ) {
	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_ListStyleCha( ) {
	TRACE( _T( "List style has changed, redrawing!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_SelectionCha( ) {
	TRACE( _T( "CDirstatView::OnUpdateHINT_SELECTIONCHANGED\r\n" ) );
	}

IMPLEMENT_DYNCREATE( CDirstatView, CView )

BEGIN_MESSAGE_MAP(CDirstatView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_ITEMCHANGED, _nIdTreeListControl, &( CDirstatView::OnLvnItemchanged ) )
	ON_UPDATE_COMMAND_UI(ID_POPUP_TOGGLE, &( CDirstatView::OnUpdatePopupToggle ) )
	ON_COMMAND(ID_POPUP_TOGGLE, &( CDirstatView::OnPopupToggle ) )
END_MESSAGE_MAP()

_Must_inspect_result_ CDirstatDoc* CDirstatView::GetDocument( ) {
	return STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	}

void CDirstatView::OnUpdateHINT_NEWROOT( ) {
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	ASSERT( Document != NULL );//The document is NULL??!? WTF
	if ( Document == NULL ) {
		return;
		}
	const auto newRootItem = Document->m_rootItem.get( );
	if ( newRootItem != NULL ) {
		m_treeListControl.SetRootItem( newRootItem );
		VERIFY( m_treeListControl.RedrawItems( 0, m_treeListControl.GetItemCount( ) - 1 ) );
		return;
		}
	}

void CDirstatView::OnUpdateHINT_SELECTIONCHANGED( ) {
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	ASSERT( Document != NULL );//The document is NULL??!? WTF
	if ( Document == NULL ) {
		TRACE( _T( "Document is NULL, CDirstatView::OnUpdateHINT_SELECTIONCHANGED can't do jack shit.\r\n" ) );
		return;
		}
	trace_SelectionCha( );
		
	const auto Selection = Document->m_selectedItem;
	ASSERT( Selection != NULL );
	if ( Selection == NULL ) {
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		return;
		}
	m_treeListControl.SelectAndShowItem( Selection, false );
	}


void CDirstatView::OnUpdateHINT_SHOWNEWSELECTION( ) {
	const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
	ASSERT( Document != NULL );//The document is NULL??!? WTF
	if ( Document == NULL ) {
		return;
		}
	const auto Selection = Document->m_selectedItem;
	ASSERT( Selection != NULL );
	if ( Selection == NULL ) {
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		return;
		}
	TRACE( _T( "New item selected! item: %s\r\n" ), Selection->GetPath( ).c_str( ) );
	m_treeListControl.SelectAndShowItem( Selection, true );
	}


void CDirstatView::OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult ) {
	const auto pNMLV = reinterpret_cast< LPNMLISTVIEW >( pNMHDR );
	//( pResult != NULL ) ? ( *pResult = 0 ) : ASSERT( false );//WTF
	ASSERT( pResult != NULL );
	if ( pResult != NULL ) {
		*pResult = 0;
		}
	if ( ( pNMLV->uChanged & LVIF_STATE ) == 0 ) {
		return;
		}
	if ( pNMLV->iItem == -1 ) {
		ASSERT( false ); // mal gucken //'watch times'?
		return;
		}
	// This is not true (don't know why): ASSERT(m_treeListControl.GetItemState(pNMLV->iItem, LVIS_SELECTED) == pNMLV->uNewState);
	const bool selected = ( ( m_treeListControl.GetItemState( pNMLV->iItem, LVIS_SELECTED ) & LVIS_SELECTED ) != 0 );
	const auto item = static_cast< CTreeListItem * >( m_treeListControl.GetItem( pNMLV->iItem ) );
	ASSERT( item != NULL );//We got a NULL item??!? WTF
	if ( item == NULL ) {
		return;
		}
	if ( selected ) {
		const auto Document = STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		ASSERT( Document != NULL );
		if ( Document == NULL ) {
			TRACE( _T( "I'm told that the selection has changed in a NULL document?!?? This can't be right.\r\n" ) );
			return;
			}
		Document->SetSelection( *item );
		ASSERT( Document == m_pDocument );
		return m_pDocument->UpdateAllViews( this, UpdateAllViews_ENUM::HINT_SELECTIONCHANGED );
		}
	}

#else

#endif