// dirstatview.cpp : Implementation of CDirstatView
//
// see `file_header_text.txt` for licensing & contact info. If you can't find that file, then assume you're NOT allowed to do whatever you wanted to do.
#include "stdafx.h"

#pragma once


#ifndef WDS_DIRSTATVIEW_CPP
#define WDS_DIRSTATVIEW_CPP

WDS_FILE_INCLUDE_MESSAGE


//DO NOT MAKE THESE CONSTEXPR, VS2019 lies.
const UINT _WDS_nIdTreeListControl = 4711u;
const UINT ITEM_ROW_HEIGHT = 20u;

//
#include "dirstatview.h"
#include "dirstatdoc.h"

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_OnEraseBkgnd( ) noexcept {
	TRACE( _T( "CDirstatView::OnEraseBkgnd!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_LoadingPerst( ) noexcept {
	TRACE( _T( "Loading persistent attributes....\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_ListStyleCha( ) noexcept {
	TRACE( _T( "List style has changed, redrawing!\r\n" ) );
	}

//this function exists for the singular purpose of tracing to console, as doing so from a .cpp is cleaner.
inline void trace_SelectionCha( ) noexcept {
	TRACE( _T( "CDirstatView::OnUpdateHINT_SELECTIONCHANGED\r\n" ) );
	}

//IMPLEMENT_DYNCREATE( CDirstatView, CView )
CObject* PASCAL CDirstatView::CreateObject() {
	return new CDirstatView;
	}
AFX_COMDAT const CRuntimeClass CDirstatView::classCDirstatView = {
	"CDirstatView" /*m_lpszClassName*/,
	sizeof(class CDirstatView) /*m_nObjectSize*/,
	0xFFFF /*wSchema*/,
	CDirstatView::CreateObject /*pfnNew*/,
	(const_cast<CRuntimeClass*>(&CView::classCView)) /*RUNTIME_CLASS(CView)*/ /*m_pBaseClass*/,
	nullptr /*m_pNextClass*/,
	nullptr /*class_init*/
	};

CRuntimeClass* CDirstatView::GetRuntimeClass() const {
	return (const_cast<CRuntimeClass*>(&CDirstatView::classCDirstatView)) /*RUNTIME_CLASS(CDirstatView)*/;
	}



BEGIN_MESSAGE_MAP(CDirstatView, CView)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_ITEMCHANGED, _WDS_nIdTreeListControl, &( CDirstatView::OnLvnItemchanged ) )
	ON_UPDATE_COMMAND_UI(ID_POPUP_TOGGLE, &( CDirstatView::OnUpdatePopupToggle ) )
	ON_COMMAND(ID_POPUP_TOGGLE, &( CDirstatView::OnPopupToggle ) )
END_MESSAGE_MAP()

_Must_inspect_result_ CDirstatDoc* CDirstatView::GetDocument( ) noexcept {
	return static_cast<CDirstatDoc*>( m_pDocument );
	}

void CDirstatView::OnUpdateHINT_NEWROOT( ) noexcept {
	const CDirstatDoc* const Document = static_cast<CDirstatDoc*>( m_pDocument );
	ASSERT( Document != nullptr );//The document is NULL??!? WTF
	if ( Document == nullptr ) {
		return;
		}
	const auto newRootItem = Document->m_rootItem.get( );
	if ( newRootItem != nullptr ) {
		m_treeListControl.SetRootItem( newRootItem );
		VERIFY( m_treeListControl.RedrawItems( 0, m_treeListControl.GetItemCount( ) - 1 ) );
		return;
		}
	}

void CDirstatView::OnUpdateHINT_SELECTIONCHANGED( ) {
	const CDirstatDoc* const Document = static_cast<CDirstatDoc*>( m_pDocument );
	ASSERT( Document != nullptr );//The document is NULL??!? WTF
	if ( Document == nullptr ) {
		TRACE( _T( "Document is NULL, CDirstatView::OnUpdateHINT_SELECTIONCHANGED can't do jack shit.\r\n" ) );
		return;
		}
	trace_SelectionCha( );
		
	const auto Selection = Document->m_selectedItem;
	ASSERT( Selection != nullptr );
	if ( Selection == nullptr) {
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		return;
		}
	TRACE( _T( "Selection changed to: %s\r\n" ), Selection->GetPath( ).c_str( ) );
	m_treeListControl.SelectAndShowItem( Selection, false );
	}


void CDirstatView::OnUpdateHINT_SHOWNEWSELECTION( ) {
	const CDirstatDoc* const Document = static_cast<CDirstatDoc*>( m_pDocument );
	ASSERT( Document != nullptr );//The document is NULL??!? WTF
	if ( Document == nullptr ) {
		return;
		}
	const auto Selection = Document->m_selectedItem;
	ASSERT( Selection != nullptr );
	if ( Selection == nullptr ) {
		TRACE( _T( "I was told that the selection changed, but found a NULL selection. I can neither select nor show NULL - What would that even mean??\r\n" ) );
		return;
		}
	TRACE( _T( "New item selected! item: %s\r\n" ), Selection->GetPath( ).c_str( ) );
	m_treeListControl.SelectAndShowItem( Selection, true );

	hwnd::RedrawWindow(m_hWnd);
	//CWnd::RedrawWindow( );
	}


void CDirstatView::OnLvnItemchanged( NMHDR* const pNMHDR, LRESULT* const pResult ) {
	const NMLISTVIEW* const pNMLV = reinterpret_cast< const LPNMLISTVIEW >( pNMHDR );
	//( pResult != NULL ) ? ( *pResult = 0 ) : ASSERT( false );//WTF
	ASSERT( pResult != nullptr );
	if ( pResult != nullptr ) {
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
	
	//BUGBUG why?
	//const CTreeListItem* const item = static_cast< CTreeListItem* >( m_treeListControl.GetItem( pNMLV->iItem ) );
	const CTreeListItem* const item = m_treeListControl.GetItem(pNMLV->iItem);
	
	ASSERT( item != nullptr );//We got a NULL item??!? WTF
	if ( item == nullptr ) {
		return;
		}
	if ( selected ) {
		const auto Document = static_cast<CDirstatDoc*>( m_pDocument );
		ASSERT( Document != nullptr );
		if ( Document == nullptr ) {
			TRACE( _T( "I'm told that the selection has changed in a NULL document?!?? This can't be right.\r\n" ) );
			return;
			}
		Document->SetSelection( *item );
		ASSERT( Document == m_pDocument );
		return m_pDocument->UpdateAllViews( this, UpdateAllViews_ENUM::HINT_SELECTIONCHANGED );
		}
	}

#endif