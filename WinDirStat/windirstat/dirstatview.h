// dirstatview.h	- Declaration of CMyTreeListControl and CDirstatView
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

#ifndef DIRSTATVIEW_H
#define DIRSTATVIEW_H
#else
#error ass
#endif

#pragma once

#include "stdafx.h"

//#include "TreeListControl.h"

class CDirstatView;
class CDirstatDoc;
class CItemBranch;


// CMyTreeListControl. I had to derive from CTreeListControl because
// CTreeListControl doesn't know about the column constants (COL_***).
class CMyTreeListControl : public CTreeListControl {
public:
	CMyTreeListControl( _In_ CDirstatView* dirstatView ) : CTreeListControl( ITEM_ROW_HEIGHT ), m_dirstatView( dirstatView ) { }
	virtual bool GetAscendingDefault( _In_ const INT column ) const override final {
		return ( column == column::COL_NAME || column == column::COL_LASTCHANGE );
		}

protected:
	virtual void OnItemDoubleClick(_In_ _In_range_( 0, INT_MAX ) const INT i) override final;

	void PrepareDefaultMenu( _Out_ CMenu* const menu, _In_ const CItemBranch* const item );

	CDirstatView* m_dirstatView;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSetFocus( _In_ CWnd* pOldWnd ) {
		CTreeListControl::OnSetFocus( pOldWnd );
		GetMainFrame( )->SetLogicalFocus( LF_DIRECTORYLIST );
		}
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	};


//
// CDirstatView. The upper left view, which consists of the TreeList.
//
class CDirstatView : public CView {
	protected:
	CDirstatView( ) : m_treeListControl( this ) {// Created by MFC only
		m_treeListControl.SetSorting( column::COL_SUBTREETOTAL, false );
		}
	DECLARE_DYNCREATE(CDirstatView)

public:
	virtual ~CDirstatView( ) { }

	void SysColorChanged( ) {
		m_treeListControl.SysColorChanged( );
		}
	CMyTreeListControl m_treeListControl;	// The tree list
	

	//bool DoSort( );
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
	_Must_inspect_result_ CDirstatDoc* GetDocument( ) {
		return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
		}

	virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint ) override final;

	void OnUpdateHINT_NEWROOT( );
	void OnUpdateHINT_SELECTIONCHANGED( );
	void OnUpdateHINT_SHOWNEWSELECTION( );
	void OnUpdateHINT_LISTSTYLECHANGED( );
	void OnUpdateHINT_SOMEWORKDONE( );
	
	void SetTreeListControlOptions( );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize( UINT nType, INT cx, INT cy );
	afx_msg INT OnCreate( LPCREATESTRUCT lpCreateStruct );
	afx_msg BOOL OnEraseBkgnd( CDC* pDC ) {
		UNREFERENCED_PARAMETER( pDC );
		return TRUE;
		}
	afx_msg void OnDestroy( ) {
#ifdef DRAW_ICONS
		m_treeListControl.MySetImageList( NULL );
#endif
		CView::OnDestroy();
		}
	afx_msg void OnSetFocus( CWnd* pOldWnd ) {
		UNREFERENCED_PARAMETER( pOldWnd );
		m_treeListControl.SetFocus( );
		}
	afx_msg void OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );
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
		AfxCheckMemory( );
		CView::Dump( dc );
		}
	#endif
	};


//#ifndef _DEBUG  // Debug version in dirstatview.cpp
//_Must_inspect_result_ inline CDirstatDoc* CDirstatView::GetDocument( ) {
//	return reinterpret_cast< CDirstatDoc* >( m_pDocument );
//	}
//#endif


// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
