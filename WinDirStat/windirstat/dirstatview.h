// dirstatview.h	- Declaration of CMyTreeListControl and CDirstatView
//
// see `file_header_text.txt` for licensing & contact info.


#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTATVIEW_H
#define WDS_DIRSTATVIEW_H

#pragma message( "Including `" __FILE__ "`..." )


//encourage inter-procedural optimization (and class-hierarchy analysis!)
#include "TreeListControl.h"


#include "datastructures.h"

#include "dirstatdoc.h"

class CDirstatView;
class CDirstatDoc;

namespace {
	const UINT _nIdTreeListControl = 4711;
	}


inline void trace_OnEraseBkgnd( );
inline void trace_LoadingPerst( );
inline void trace_ListStyleCha( );
inline void trace_SelectionCha( );

// CDirstatView. The upper left view, which consists of the TreeList.
class CDirstatView final : public CView {
protected:
	CDirstatView( ) : m_treeListControl( ITEM_ROW_HEIGHT, GetDocument( ) ) {// Created by MFC only
		m_treeListControl.SetSorting( column::COL_SUBTREETOTAL, false );
		}
	CDirstatView( const CDirstatView& in ) = delete;

	DECLARE_DYNCREATE( CDirstatView )

public:
	virtual ~CDirstatView( ) final = default;

	CDirstatView& operator=( const CDirstatView& in ) = delete;

	void SysColorChanged( ) {
		m_treeListControl.SysColorChanged( );
		}
	CTreeListControl m_treeListControl;	// The tree list

protected:
	virtual BOOL PreCreateWindow( CREATESTRUCT& cs ) override final {
		return CView::PreCreateWindow( cs );
		}

	virtual void OnInitialUpdate( ) override final {
		CView::OnInitialUpdate( );
		}
	virtual void OnDraw( CDC* pDC ) override final {
		ASSERT_VALID( pDC );
		CView::OnDraw( pDC );
		}
	
	/*
	//return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(_RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(((CRuntimeClass*)(&CDirstatDoc::classCDirstatDoc)), m_pDocument)
	
	*/
	_Must_inspect_result_ CDirstatDoc* GetDocument( ) {
		return STATIC_DOWNCAST( CDirstatDoc, m_pDocument );
		}

	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final {
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

			case UpdateAllViews_ENUM::HINT_LISTSTYLECHANGED:
				return OnUpdateHINT_LISTSTYLECHANGED( );
			case 0:
				return CView::OnUpdate( pSender, lHint, pHint );

			default:
				return;
			}
		}

	void OnUpdateHINT_NEWROOT( ) {
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
		//m_treeListControl.SetRootItem( newRootItem );
		//VERIFY( m_treeListControl.RedrawItems( 0, m_treeListControl.GetItemCount( ) - 1 ) );
		}
	void OnUpdateHINT_SELECTIONCHANGED( ) {
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
	void OnUpdateHINT_SHOWNEWSELECTION( ) {
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
	void OnUpdateHINT_LISTSTYLECHANGED( ) {
		
		trace_ListStyleCha( );
		const auto Options = GetOptions( );
		m_treeListControl.ShowGrid( Options->m_listGrid );
		m_treeListControl.ShowStripes( Options->m_listStripes );
		m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
		}
	//void OnUpdateHINT_SOMEWORKDONE( );
	
	void SetTreeListControlOptions( ) {
		const auto Options = GetOptions( );
		m_treeListControl.ShowGrid            ( Options->m_listGrid             );
		m_treeListControl.ShowStripes         ( Options->m_listStripes          );
		m_treeListControl.ShowFullRowSelection( Options->m_listFullRowSelection );
		return;
		}

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, INT cx, INT cy ) {
		CWnd::OnSize( nType, cx, cy );
		if ( ::IsWindow( m_treeListControl.m_hWnd ) ) {
			const RECT rc = { 0, 0, cx, cy };
			//If [MoveWindow] succeeds, the return value is nonzero.
			VERIFY( ::MoveWindow( m_treeListControl.m_hWnd, rc.left, rc.top, ( rc.right - rc.left ), ( rc.bottom - rc.top ), TRUE ) );
			m_treeListControl.RedrawWindow( );
			}
		}
	afx_msg INT OnCreate( LPCREATESTRUCT lpCreateStruct ) {
		if ( CView::OnCreate( lpCreateStruct ) == -1 ){
			return -1;
			}
		const RECT rect = { 0, 0, 0, 0 };
		VERIFY( m_treeListControl.CreateEx( 0, WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, rect, this, _nIdTreeListControl ) );
		m_treeListControl.AddExtendedStyle( LVS_EX_HEADERDRAGDROP );
		SetTreeListControlOptions( );
		m_treeListControl.InsertColumn( column::COL_NAME,         _T( "Name" ),                   LVCFMT_LEFT,  200, column::COL_NAME );
		m_treeListControl.InsertColumn( column::COL_PERCENTAGE,   _T( "Percentage" ),             LVCFMT_RIGHT,  55, column::COL_PERCENTAGE );
		m_treeListControl.InsertColumn( column::COL_SUBTREETOTAL, _T( "Size" ),                   LVCFMT_RIGHT,  90, column::COL_SUBTREETOTAL );
		m_treeListControl.InsertColumn( column::COL_ITEMS,        _T( "Items" ),                  LVCFMT_RIGHT,  55, column::COL_ITEMS );
		m_treeListControl.InsertColumn( column::COL_NTCOMPRESS,   _T( "NTFS compression ratio" ), LVCFMT_RIGHT, 100, column::COL_NTCOMPRESS );
	  //m_treeListControl.InsertColumn( column::COL_FILES,        _T( "Files" ),                  LVCFMT_RIGHT,  55, column::COL_FILES );
	  //m_treeListControl.InsertColumn( column::COL_SUBDIRS,      _T( "Subdirs" ),                LVCFMT_RIGHT,  55, column::COL_SUBDIRS );
		m_treeListControl.InsertColumn( column::COL_LASTCHANGE,   _T( "Last Change" ),            LVCFMT_LEFT,  120, column::COL_LASTCHANGE );
		m_treeListControl.InsertColumn( column::COL_ATTRIBUTES,   _T( "Attributes" ),             LVCFMT_LEFT,   50, column::COL_ATTRIBUTES );

		
		trace_LoadingPerst( );
		m_treeListControl.OnColumnsInserted( );
		return 0;
		}
	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		trace_OnEraseBkgnd( );
		//UNREFERENCED_PARAMETER( pDC );
		//return TRUE;
		return CWnd::OnEraseBkgnd( pDC );
		}
	afx_msg void OnDestroy( ) {
		CView::OnDestroy( );
		}
	afx_msg void OnSetFocus( CWnd* pOldWnd ) {
		UNREFERENCED_PARAMETER( pOldWnd );
		m_treeListControl.SetFocus( );
		}
	afx_msg void OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult ) {
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
	afx_msg void OnUpdatePopupToggle( _In_ CCmdUI* pCmdUI ) {
		pCmdUI->Enable( m_treeListControl.SelectedItemCanToggle( ) );
		}
	afx_msg void OnPopupToggle( ) {
		m_treeListControl.ToggleSelectedItem( );
		}

public:
	#ifdef _DEBUG
	virtual void AssertValid( ) const {
		CView::AssertValid( );
		}
	virtual void Dump( CDumpContext& dc ) const {
		TRACE( _T( "CDirstatView::Dump\r\n" ) );
		AfxCheckMemory( );
		CView::Dump( dc );
		}
	#endif
	};

#else


#endif
