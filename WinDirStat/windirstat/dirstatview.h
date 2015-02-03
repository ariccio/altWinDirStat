// dirstatview.h	- Declaration of CMyTreeListControl and CDirstatView
//
// see `file_header_text.txt` for licensing & contact info.


#pragma once

#include "stdafx.h"

#ifndef WDS_DIRSTATVIEW_H
#define WDS_DIRSTATVIEW_H




#include "TreeListControl.h"

class CDirstatView;
class CDirstatDoc;
class CItemBranch;

// CDirstatView. The upper left view, which consists of the TreeList.
class CDirstatView final : public CView {
protected:
	CDirstatView( );
	CDirstatView( const CDirstatView& in ) = delete;

	DECLARE_DYNCREATE( CDirstatView )

public:
	virtual ~CDirstatView( ) final = default;

	CDirstatView& operator=( const CDirstatView& in ) = delete;

	void SysColorChanged( );
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

#else


#endif
