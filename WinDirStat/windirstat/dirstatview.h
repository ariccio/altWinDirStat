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
#include "TreeListControl.h"

class CDirstatView;
class CDirstatDoc;
class CItemBranch;
class CMyTreeListControl;


// CMyTreeListControl. I had to derive from CTreeListControl because
// CTreeListControl doesn't know about the column constants (COL_***).
class CMyTreeListControl : public CTreeListControl {
public:
	CMyTreeListControl( _In_ CDirstatView* const dirstatView );
	CMyTreeListControl& operator=( const CMyTreeListControl& in ) = delete;
	CMyTreeListControl( const CMyTreeListControl& in ) = delete;

private:
	virtual bool GetAscendingDefault( _In_ const column::ENUM_COL column ) const override final;
protected:
	virtual void OnItemDoubleClick( _In_ _In_range_( 0, INT_MAX ) const INT i ) override final;

	void PrepareDefaultMenu( _Out_ CMenu* const menu, _In_ const CItemBranch* const item );

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnSetFocus( _In_ CWnd* pOldWnd );
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	
	CDirstatView* m_dirstatView;
	};


//
// CDirstatView. The upper left view, which consists of the TreeList.
//
class CDirstatView : public CView {
protected:
	CDirstatView( );
	CDirstatView( const CDirstatView& in ) = delete;

	DECLARE_DYNCREATE( CDirstatView )

public:
	virtual ~CDirstatView( ) { }

	CDirstatView& operator=( const CDirstatView& in ) = delete;

	void SysColorChanged( );
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
	
	/*
	//return DYNAMIC_DOWNCAST( CDirstatDoc, m_pDocument );
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(_RUNTIME_CLASS(CDirstatDoc), m_pDocument)
	//--becomes--
	//(CDirstatDoc*)AfxDynamicDownCast(((CRuntimeClass*)(&CDirstatDoc::classCDirstatDoc)), m_pDocument)
	
	*/
	_Must_inspect_result_ CDirstatDoc* GetDocument( );

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
	afx_msg BOOL OnEraseBkgnd( CDC* pDC );
	afx_msg void OnDestroy( );
	afx_msg void OnSetFocus( CWnd* pOldWnd );
	afx_msg void OnLvnItemchanged( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnUpdatePopupToggle( _In_ CCmdUI* pCmdUI );
	afx_msg void OnPopupToggle( );

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


//#ifndef _DEBUG  // Debug version in dirstatview.cpp
//_Must_inspect_result_ inline CDirstatDoc* CDirstatView::GetDocument( ) {
//	return reinterpret_cast< CDirstatDoc* >( m_pDocument );
//	}
//#endif


// $Log$
// Revision 1.4  2004/11/05 16:53:07  assarbad
// Added Date and History tag where appropriate.
//
